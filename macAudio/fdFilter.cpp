/* fdFilter.cc */

/* FdFilter: In-Line Filter class */

/*

This class implements a general-purpose streaming FIR filter that is
applied in the frequency domain for speed/efficiency.  Using
FFT-multiply-IFFT rather than a simple time-domain convolution greatly
speeds up the computations.  The longer the impulse response of the
filter, the greater the advantage of this FFT approach over
time-domain convolution.

This class also supports sample-rate conversion.  For simple
input/output rate ratios, it is also quite efficient, but becomes
rather inefficient for ratios with denominator terms greater than 5.
It therefore limits the maximum denominator value to 10, and thus can
only approximate rate conversions that require larger denominator
terms.  For large denominator ratios, or extreme decimation, indexed
time-domain implementations can be much faster.

All filters implemented with FdFilter are symmetric FIR, and thus have
linear phase response.  The filter is implemented non-causally, so
that there is no signal delay introduced by filtering or downsampling
operations.

See the individual methods for details of use.

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <getopt.h>
#include "fdFilter.h"

// Instantiation when FdFilter is to be used as a high- or low-pass filter
// or as a sample-rate converter.  'inFreq' is the sample rate of the
// input signal in Hz.  If inFreq != cFreq and convertF is true, FdFilter is
// confgured to convert the sample rate to cFreq using a filter of
// length specified by fDur.
//
// If convertF is false, FdFilter is configured to be a high- or low-pass
// filter.  In this case, cFreq is interpreted as the corner frequency
// of the filter, and must be in the range 0 < cFreq < (inFreq/2).  If
// doHp is true, the filter will be high-pass, else, low-pass.  Again,
// fDur determines the filter length.  in all cases, the filter length
// is specified in seconds, and determines the transition band width.
// The band width is approximately 1.0/fDur Hz wide.
/* ******************************************************************** */
FdFilter::FdFilter(float inFreq, float cFreq, int doHp, float fDur, int convertF)
{
  FdFilterInitialize(inFreq, cFreq, doHp, fDur, convertF, NULL, NULL, 0);
}

// Instantiation when FdFilter will be used to implement a filter whose
// spectral magnitude values are listed in the plain text file with
// the name specified by 'specShape'.  inFreq is the sample rate in Hz
// of the input signal.  The file must have the following format:
//
// Line 1:                    pow2 nMags
// Lines 2 through (nMags+1): ind val
//
// nMags must equal ((1 << pow2)/2)+1,
// pow2 is typically in the range 3 < pow2 < 15.

// The lines in the file containing (ind,val) pairs specify the
// magnitude response of the filter uniformly sampling the spectrum
// from 0 Hz to (inFreq/2) Hz.  the 'ind' column is just a line index
// to make the file easily human readable, the 'val' column contains
// (positive) magnitude scaling values.  Note that values > 1.0 cause
// an increase in output signal amplitude re the input signal at the
// corresponding frequency, and have the potential to cause clipping,
// if the input signal is too energetic at those frequencies.
/* ******************************************************************** */
FdFilter::FdFilter(float inFreq, char *specShape)
{
  FdFilterInitialize(inFreq, inFreq, 0, 0.01, 0, specShape, NULL, 0);
}

// Instantiation when FdFilter will be used to implement a filter whose
// 'nMagnitudes' spectral magnitude values are in the array 'specArray'.
// nMagnitudes must equal ((1 << pow2)+1) for some pow2 in the
// range 2 < pow2 < 16.  The values in specArray are interpreted as
// above for the 'val' column in the 'specShape' file.
/* ******************************************************************** */
FdFilter::FdFilter(float inFreq, float *specArray, int nMagnitudes)
{
  FdFilterInitialize(inFreq, inFreq, 0, 0.01, 0, NULL, specArray, nMagnitudes);
}

// The private method that configures FdFilter to satisfy the requirements
// of the constructor.
/* ----------------------------------------------------------------------- */
void FdFilter::FdFilterInitialize(float inFreq, float cFreq, int doHp, float fDur, int convertF,
			char *specShape, float *specArray, int nMagnitudes)
{
  float	beta = 0.0, *b = NULL;
  float  ratio_t, ratio, freq1=inFreq;
  int pow2, i;

  insert = 1;
  decimate = 1;
  state = 1;
  arrayLeftover = 0;
  arrayIndex = 0;
  arraySamplesUsed = 0;
  trueOutputFreq = inFreq;

  if(specShape || (specArray && (nMagnitudes > 1))) {
    doHp = -1; /* flag that this is an equalization filtering */
  } else {
    if(convertF) {
      freq1 = inFreq;
    } else {			/* it is just a symmetric FIR */
      if(cFreq >= (freq1 = inFreq)/2.0) {
	fprintf(stderr,"Unreasonable corner frequency specified to filter() (%f)\n",cFreq);
      }
    }
  }

  if(specShape) {
    FILE *specStream = fopen(specShape, "r");

    if(specStream) {
      int nSpect, ind;
      char line[500];
      float fs;
      if(fgets(line,500, specStream)) {
	int nItems = sscanf(line,"%d %d %f", &pow2, &nSpect, &fs);
	if((nItems == 3) && (fs != inFreq)) { // This should be a fatal error
	  fprintf(stderr,"Filter spec (%f) does not match input frequency (%f)\n",fs,inFreq);
	  fprintf(stderr,"The filtering results will probably not be what you want!\n");
	}
	b = new float [nSpect];
	nCoeff = (nSpect - 1); /* represents actual filter-kernel length,
				  instead of half filter length
				  when using externsl filter. */
	for(i=0; i < nSpect; i++) {
	  if((! fgets(line,500,specStream)) ||
	     (sscanf(line,"%d %f", &ind, &(b[i])) != 2)) {
	    fprintf(stderr, "Parsing error in spect ratio file %s\n",specShape);
	  }
	}
      } else {
	fprintf(stderr,"Bad format in spectrum file %s\n", specShape);
      }
      fclose(specStream);
    } else {
      fprintf(stderr,"Can't open %s as a spectrum file\n",specShape);
    }
  } else
    if( specArray && (nMagnitudes > 1)) { // Note: nMagnitudes MUST be ((2^k)+1) for k > 1.
      nCoeff = nMagnitudes - 1;
      int nft = nCoeff * 2;
      pow2 = 1;
      while( (1 << pow2) < nft) {
	pow2++;
      }
      b = specArray; // Note: b must not be deleted in this case!
    } else { /* it is not an eq filter */
      if(convertF) {
	/* get a ratio of integers close to desired freq. ratio. */
	ratio = cFreq/freq1;
	ratprx(ratio,&insert,&decimate,FD_MAX_DECIMATE);
	ratio_t = ((float)insert)/((float)decimate);
    
	if(fabs(1.0-ratio_t) < .01) {
	  fprintf(stderr,"Input and output frequencies are essentially equal!\n");
	}
    
	trueOutputFreq = ratio_t * freq1;
	//	if(cFreq != trueOutputFreq) {
	//	  fprintf(stderr,
	//		  "Warning: The output frequency obtained (%f) is not the frequency requested (%f)\n",
	//		  trueOutputFreq, cFreq);
	//	}
	cFreq = trueOutputFreq;
	nCoeff = ((int)(freq1 * insert * fDur)) | 1;
	if(cFreq < freq1)
	  beta = (.5 * cFreq)/(insert * freq1);
	else
	  beta = .5/insert;
      } else {
	beta = cFreq/freq1;
	nCoeff = ((int)(freq1 * fDur)) | 1;
      }

      /* Generate the symmetric FIR filter coefficients. */
      b = new float [1 + nCoeff/2];
      lc_lin_fir(beta,&nCoeff,b);

      if(insert > 1) { /* scale up filter coeffs. to maintain precision in output */
	float fact = (float)insert;
	for(i=nCoeff/2; i>=0; i--) b[i] *= fact;
      }
    } /* end else it is not an eq filter */

  nCoeffBy2 = nCoeff/2;

  if(doHp >= 0) { /* Is it a simple high- or low-pass filter? */
    mirror_filter(b,doHp);
    int nf2 = nCoeff << 1;
    nFFT = 128;
    pow2 = 7;
    while(nf2 > nFFT) {
      nFFT *= 2;
      pow2++;
    }
  } else { /* it is a filter with the magnitude response specified in b. */
    nFFT = nCoeff * 2;
    pow2 = 2;
    while((1 << pow2) < nFFT)
      pow2++;
  }

  nFFTBy2 = nFFT/2;

  x = new float [nFFT];
  y = new float [nFFT];
  xf = new float [nFFT];
  yf = new float [nFFT];

  leftovers = new short [nFFT];
  maxInput = FD_FBUF_SIZE/insert;
  outputDelayed = new float [FD_FBUF_SIZE+nCoeff+nFFT];

  float ftscale = 1.0/(float)nFFT;
  ft = new FFT(pow2);
  if(doHp >= 0) {
    /* position the filter kernel to be symmetric about time=0 */
    /* Note that this assumes an odd number of symmetric filter coefficients. */
    for(i=0; i <= nCoeffBy2; i++) {
      xf[i] = ftscale * fCoeff[i+nCoeffBy2];
      yf[i] = 0.0;
    }
    for( ; i < nCoeff; i++) {
      xf[nFFT - nCoeff + i] = ftscale * fCoeff[i-nCoeffBy2-1];
      yf[nFFT - nCoeff + i] = 0.0;
    }
    for(i=nCoeffBy2 ;i < (nFFT-nCoeffBy2);i++)
      xf[i] = yf[i] = 0.0;
    ft->fft(xf,yf);
  } else { /* Install the magnitude response symmetrically. */
    for(i=0; i <= nCoeff; i++) {
      xf[i] = ftscale * b[i];
      yf[i] = 0.0;
    }
    for( ; i < nFFT; i++) {
      xf[i] = xf[nFFT - i];
      yf[i] = 0.0;
    }
  }
  /* The filter, regardless of origin, is now in the frequency domain. */

  if(specShape && !( specArray && (nMagnitudes > 1)))
    delete b;

  b = NULL;
  filterMode = doHp;
}

// Destructor
/* ******************************************************************** */
FdFilter::~FdFilter()
{
  delete x;
  delete y;
  delete xf;
  delete yf;
  delete fCoeff;
  delete leftovers;
  delete outputDelayed;
  delete ft;
}


// Given the half filter in fc, store the full symmetric kernel in fCoeff.
/* ******************************************************************** */
void FdFilter::mirror_filter(float *fc, int invert)
{
  float *dp1, *dp2, *dp3, sum, integral;
  int i, ncoefb2;

  fCoeff = new float [nCoeff];
  ncoefb2 = 1 + nCoeff/2;
  /* Copy the half-filter and its mirror image into the coefficient array. */
  for(i=ncoefb2-1, dp3=fc+ncoefb2-1, dp2=fCoeff, dp1 = fCoeff+nCoeff-1,
      integral = 0.0; i-- > 0; )
    if(!invert) *dp1-- = *dp2++ = *dp3--;
    else {
      integral += (sum = *dp3--);
      *dp1-- = *dp2++ = -sum;
    }
  if(!invert)  *dp1 = *dp3; /* point of symmetry */
  else {
    integral *= 2;
    integral += *dp3;
    *dp1 = integral - *dp3;
  }
}

// This is a complex vector multiply.  If the seconf vector (r2,i2) is
// known to be real, set 'i2' to NULL for faster computation.  The
// result is returned in (r3,i3).
/* ******************************************************************** */
void FdFilter::cmul(int n, float *r1, float *i1, float *r2, float *i2, float *r3, float *i3)
{
  float tr1, ti1;

  /* This full complex multiply is only necessary for non-symmetric kernels */
  if(i2) { // Only supply the i2 vector if you need to do a full complex multiply.
    while(n--) {
      tr1 = (*r1 * *r2) - (*i1 * *i2);
      ti1 = (*r1++ * *i2++) + (*r2++ * *i1++);
      *i3++ = ti1;
      *r3++ = tr1;
    }
  } else {
    /* Can do this iff the filter is symmetric, zero phase.  */
    while(n--) {
      tr1 = (*r1++ * *r2);
      ti1 = (*r2++ * *i1++);
      *i3++ = ti1;
      *r3++ = tr1;
    } 
  }
}

/* ******************************************************************** */
// Process samples from 'input' array to 'output' array.  'nIn'
// contains the number of samples available in 'input'.  'maxOut' is
// the maximum number of samples that the caller allows to be
// transferred to 'output' (usually the size of 'output').  Set
// 'first' TRUE if the first sample of a new signal is in 'input'.
// Set 'last' TRUE of the last sample of the signal is in 'input' (to
// cause flushing of processing pipeline).  The simplest setup for use
// of this method requires that the caller use an input buffer no
// larger than the size returned by getMaxInputSize(), and an output
// array twice the size returned by getMaxInputSize().  Then, each
// subsequent call to filterArray will use all 'input' samples, and
// will transfer aoll available samples to 'output'.  Examples of this
// and the other case, of arbitrarily small caller buffers can be seen
// in the test harness at the end of this file.
// This method returns the number of output samples transferred to 'output'.
int FdFilter::filterArray(short *input, int nIn, int first, int last, short *output, int maxOut)
{
  int i, j, nLeft = nIn, nOut = 0, nToGo = maxOut;
  int  toRead, toWrite, available;
  short *p = input, *q, *r = output;

  if(first) {
    state = 1; // indicate start of new signal
    arrayLeftover = 0;
    arrayIndex = 0;
  }

  if(arrayLeftover) { // First, move any output remaining from the previous call.
    int toCopy = arrayLeftover;
    if(toCopy > nToGo)
      toCopy = nToGo;
    for(i=arrayIndex, j = 0; j < toCopy; i++, j++)
      *r++ = outBuffer[i];
    nToGo -= toCopy;
    nOut += toCopy;
    arrayLeftover -= toCopy;
    arrayIndex = i;
  }
  if(nToGo <= 0) {
    arraySamplesUsed = 0; // Can't process any input this time; no room in output array.
    return(maxOut);
  }

  /* Process data from array to array. */
  while(nLeft > 0) {
    toRead = nLeft;
    if(toRead > maxInput)
      toRead = maxInput;
    if(insert > 1) {
      for(q=inBuffer, i=0; i < toRead; i++) {
	*q++ = *p++;
	for(j=1; j < insert; j++)
	  *q++ = 0;
      }
    } else {
      for(q=inBuffer, i=0; i < toRead; i++)
	*q++ = *p++;
    }
    nLeft -= toRead;
    if((nLeft <= 0) && last)
      state |= 2;  // Indicate that end of signal is (also) in this bufferful.
    filterBuffer(toRead*insert, &available);

    state = 0; // Clear the initialization bit, if any for the next iteration.

    toWrite = available;
    if(toWrite > nToGo)
      toWrite = nToGo;

    for(i=0; i < toWrite; i++)
      *r++ = outBuffer[i];
    nOut += toWrite;
    available -= toWrite;
    if(available > 0) { // Ran out of output space; suspend processing
      arrayLeftover = available;  // Save the remaining output samples for the next call.
      arrayIndex = i;
      arraySamplesUsed = nIn - nLeft; // Record the number of input samples actually used.
      return(nOut);
    }
  }
  arraySamplesUsed = nIn;
  return(nOut);
}

// Use after a call to filterArray() to determine the number of input
// samples actually processed.
int FdFilter::getArraySamplesUsed()
{
  return(arraySamplesUsed);
}

// Use after a call to filterArray() to determine how many output
// samples were NOT transferred to caller's output array due to lack
// of space in caller's array.
int FdFilter::getArrayOutputLeftover()
{
  return(arrayLeftover);
}

// Given the input stream 'inStream' and the output stream 'outStream'
// process all samples until EOF is reached on 'inStream'. Returns 1
// on success, 0 on failure.  All processing is done in a single call
// to this method.
/* ******************************************************************** */
int FdFilter::filterStream(FILE *inStream, FILE *outStream)
{
  int i, j;
  int  toread, towrite, nread, rVal = 1, testc;

  toread = maxInput;
  state = 1; // indicate start of new signal
  /* process data from a stream */
  while((nread = fread(inBuffer, sizeof(short), toread, inStream))) {
    testc = getc(inStream);
    if(feof(inStream))
      state |= 2;
    else
      ungetc(testc,inStream);
    if(insert > 1) {
      short *p, *q;
      for(p=inBuffer+nread-1, q= inBuffer+(nread*insert)-1, i=nread; i--; ) {
	for(j=insert-1; j--;) *q-- = 0;
	*q-- = *p--;
      }
    }
    filterBuffer(nread*insert, &towrite);
    if((i = fwrite(outBuffer,sizeof(short),towrite,outStream)) < towrite) {
      fprintf(stderr,"Problems writing output\n");
      rVal = 0;
    }
    state = 0;
  }
  return(rVal);
}

// This is a private method that supports filterStream() and
// filterArray().  It assumes that the input samples have been
// transferred to this->inBuffer.  It places the filtered results in
// this->outBuffer, and returns the number of output samples in
// *nOutput.
/* ******************************************************************** */
void FdFilter::filterBuffer(int nInput, int *nOutput)
{
  short *p, *r, *p2;
  float *dp1, *dp2, *q, half=0.5;
  int  i, j, k, npass;
  int totaln;

  if(state & 1) {		/* first call with this signal and filter? */
    firstout = 0;
    to_skip = 0;
    left_over = 0;
    for(i=maxInput+nCoeff, q=outputDelayed; i--; )
    *q++ = 0.0;
  }		  /* end of once-per-filter-invocation initialization */
  
  npass = (nInput + left_over)/nFFT;
  if(!npass && (state&2)) {	/* if it's the end... */
    p=inBuffer;
  } else {
    /* This is the normal non-boundary course of action. */
    for(p=inBuffer, r=leftovers+left_over, i=nFFT-left_over; i--;)
      *r++ = *p++; /* append to leftovers from prev. call */
  }
  if(!npass && !(state&2)) { /* it's not the end, but don't have enough data for a loop */
    left_over += nInput;
    *nOutput = 0;
    firstout |= state&1;		/* flag that start of sig is still here */
    return;
  }
  state |= firstout;
  firstout = 0;

  /* >>>>>>>>>>>>>> Here's the main processing loop. <<<<<<<<<<<<< */
  for(/* p set up above */ q=outputDelayed, i=0; i < npass; i++, q+=nFFT ){
    if(i) {
      for( r = p+nFFTBy2, j=nFFTBy2, dp1=x, dp2=y; j--; ) {
	*dp1++ = *p++;
	*dp2++ = *r++;
      }
      p += nFFTBy2;
    } else {
      for(p2=leftovers, r = p2+nFFTBy2, j=nFFTBy2, dp1=x, dp2=y; j--; ) {
	*dp1++ = *p2++;
	*dp2++ = *r++;
      }
    }
    for(j=nFFTBy2; j--;)
      *dp1++ = *dp2++ = 0.0;

    /* Filtering is done in the frequency domain; transform two real arrays. */
      ft->fft(x,y);
      cmul(nFFT,x,y,xf,NULL,x,y);
      ft->ifft(x,y);

    /* Overlap and add. */
      for(dp2 = q, j=nFFT - nCoeffBy2; j < nFFT; j++)
	*dp2++ += x[j];
      for(j=0, k = nCoeffBy2; j < k; j++)
	*dp2++ += x[j];
      for(j=nCoeffBy2, k = nFFT - nCoeffBy2; j < k; j++)
	*dp2++ = x[j];

      for(dp2 = q+nFFTBy2, j=nFFT - nCoeffBy2; j < nFFT; j++)
	*dp2++ += y[j];
      for(j=0, k = nCoeffBy2; j < k; j++)
	*dp2++ += y[j];
      for(j=nCoeffBy2, k = nFFT - nCoeffBy2; j < k; j++)
	*dp2++ = y[j];
  }				/* end of main processing loop */

  left_over = nInput - (p-inBuffer);

  for(i=left_over, r=leftovers; i--; ) /* save unused input sams. for next call*/
    *r++ = *p++;
  /* If signal end is here, must process any unused input. */
  if(left_over && (state&2)) {	/* must do one more zero-padded pass */
    for(p2 = leftovers+left_over, i = nFFT-left_over; i--; ) *p2++ = 0;
    for(p2=leftovers, r = p2+nFFTBy2, j=nFFTBy2, dp1=x, dp2=y; j--; ) {
      *dp1++ = *p2++;
      *dp2++ = *r++;
    }
    for(j=nFFTBy2; j--;)
      *dp1++ = *dp2++ = 0.0;
    /* Filtering is done in the frequency domain; transform two real arrays. */
      ft->fft(x,y);
      cmul(nFFT,x,y,xf,NULL,x,y);
      ft->ifft(x,y);

    /* Overlap and add. */
      for(dp2 = q, j=nFFT - nCoeffBy2; j < nFFT; j++)
	*dp2++ += x[j];
      for(j=0, k = nCoeffBy2; j < k; j++)
	*dp2++ += x[j];
      for(j=nCoeffBy2, k = nFFT - nCoeffBy2; j < k; j++)
	*dp2++ = x[j];

      for(dp2 = q+nFFTBy2, j=nFFT - nCoeffBy2; j < nFFT; j++)
	*dp2++ += y[j];
      for(j=0, k = nCoeffBy2; j < k; j++)
	*dp2++ += y[j];
      for(j=nCoeffBy2, k = nFFT - nCoeffBy2; j < k; j++)
	*dp2++ = y[j];
  }
  /* total good output samples in ob: */
  totaln = (((npass*nFFT) - to_skip - ((1&state)? nCoeffBy2 : 0))+
	    ((state&2)? left_over+(nCoeffBy2) : 0));
  /* number returned to caller: */
  *nOutput = 1 + (totaln-1)/decimate; /* possible decimation for downsampling */

  /* Round, decimate and output the samples. */
  float fTemp;
  for(j=decimate, i= *nOutput, p=outBuffer, q=(state&1)? outputDelayed+(nCoeffBy2) : outputDelayed+to_skip;
      i-- ; q+=j) {
    if((fTemp = *q) >32767.0)
      fTemp = 32767.0;
    if(fTemp < -32768.0)
      fTemp = -32768.0;
    *p++ = (short)((fTemp > 0.0) ? half + fTemp : fTemp - half);
  }

  for(dp1 = outputDelayed + npass*nFFT, j=nCoeff, dp2=outputDelayed; j--;) /*save mem for next call */
    *dp2++ = *dp1++;
  /* If decimating, number to skip on next call. */
  to_skip = (*nOutput * decimate) - totaln;
}

// Return the largest number of samples that can be processed in a
// single call to FdFilter::filterArray().  This can be used to configure
// the caller's buffer sizes to simplify subsequent processing.  The
// simplest use of filterArray() is when the caller sends chunks of
// size getMaxInputSize() (or less) as input, and has an output buffer
// of size >= (2 * getMaxInputSize()).  In this case, no checking is
// required to synchronize input and output buffering.  This, and the
// less ideal case of arbitrary caller buffer sizes are illustrated in
// the test harness at the end of this file.
/* ******************************************************************** */
int FdFilter::getMaxInputSize()
{
  return(maxInput);
}

// When sample-rate conversion is attempted, it is possible that the
// output frequency realizable with the FdFilter configuration does not
// exactly match the requested rate.  getActualOutputFreq() retrieves
// the rate achieved, and allows the caller to decide whether to
// proceed with the filtering or not, and to correctly set the rate og
// the output stream for processes later in the chain.  This method
// may be called immediately after instantiation of the FdFilter, or at any
// later time.
float FdFilter:: getActualOutputFreq()
{
  return(trueOutputFreq);
}

// A private method that finds the closest ratio to the fraction in
// 'a' (0.0 < a < 1.0).  The numerator is returned in 'k', the
// denominator in 'l'.  The largest allowed denominator is specified
// by 'qlim'.
/*      ----------------------------------------------------------      */
void FdFilter::ratprx(float a, int *k, int *l, int qlim)
{
  float	aa, af, q, em, qq=1.0, pp=1.0, ps, e;
  int	ai, ip, i;
    
  aa = fabs(a);
  ai = (int)aa;
  i = (int)aa;
  af = aa - i;
  q = 0;
  em = 1.0;
  while(++q <= qlim) {
    ps = q * af;
    ip = (int)(ps + 0.5);
    e = fabs((ps - (float)ip)/q);
    if(e < em) {
      em = e;
      pp = ip;
      qq = q;
    }
  };
  *k = (int)((ai * qq) + pp);
  *k = (a > 0)? *k : -(*k);
  *l = (int)(qq);
}

// A private method to create the coefficients for a symmetric FIR
// lowpass filter using the window technique with a Hanning window.
// Half of the symmetric kernel is returned in ''coef'.  The desired
// number of filter coefficients is in 'nf', but is forced to be odd
// by adding one, if the requesred number is even.  'fc' is the
// normalized corner frequency (0 < fc < 1).
/*      ----------------------------------------------------------      */
void FdFilter::lc_lin_fir(float fc, int *nf, float *coef)
{
  int	i, n;
  double	twopi, fn, c;

  if(((*nf % 2) != 1))
    *nf = *nf + 1;
  n = (*nf + 1)/2;

  /*  Compute part of the ideal impulse response (the sin(x)/x kernel). */
  twopi = M_PI * 2.0;
  coef[0] = 2.0 * fc;
  c = M_PI;
  fn = twopi * fc;
  for(i=1;i < n; i++) coef[i] = sin(i * fn)/(c * i);

  /* Now apply a Hanning window to the (infinite) impulse response. */
  /* (Could use other windows, like Kaiser, Gaussian...) */
  fn = twopi/(double)(*nf);
  for(i=0;i<n;i++) 
    coef[n-i-1] *= (.5 - (.5 * cos(fn * ((double)i + 0.5))));
}


#ifdef FD_FILTER_STANDALONE

// This is a test harness for the FdFilter class.  It reads stdin and
// writes stdout.  It accepts and produces short-integer raw samples
// (no headers).

#define USAGE { \
  fprintf(stderr,"Usage: %s -r<input_sample_rate> \\\n",argv[0]); \
  fprintf(stderr,"   (-S<filter_response_file> | -f<corner_freq.> [-H] | -R<output_sample_rate) \\\n"); \
  fprintf(stderr,"   [-d<filter_dur.>] [-t<mode_switches>] [-h]\n"); \
  fprintf(stderr,"Normally lowpass; -H switches to highpass.\nI/O is via stdin and stdout.\n"); \
  fprintf(stderr,"Implemented as a time-domain, Hanning-windowed, symmetric FIR filter\n	 (non-causal; no delay).\n"); \
  fprintf(stderr,"If -S is used, the filter_response_file is in the form produced by \"equal\".\n"); \
  fprintf(stderr,"If -R is used and output rate != input rate, sample rate conversion occurs.\n\n"); \
  fprintf(stderr,"The -t option allows testing of a variety of modes for using the FdFilter class.\n"); \
  fprintf(stderr,"The argument to -t is a bitwise OR of:\n"); \
  fprintf(stderr," 1 ==> Read the spectral magnitude response from an array instead of from a file.\n"); \
  fprintf(stderr," 2 ==> Process data from buffer-to-buffer, instead of from stream-to-stream\n"); \
  fprintf(stderr," 4 ==> If processing buffer-to-buffer, limit the buffer to a\n"); \
  fprintf(stderr,"   small size, as may be required from some applications.\n"); \
  exit(-1); \
}

  /*      ----------------------------------------------------------      */
int main(int argc, char **argv)
{
  int doHp, c, convertF = 0, testLev = 0;
  float corner_f, inRate, outRate, fDur;
  extern char *optarg;
  char *outputname=NULL;
  char *eqSpectrum = NULL;

  if(argc < 3) {
    USAGE
      }
  doHp = 0;
  fDur = .0127;
  corner_f = 0.0;
  inRate = -1.0;
  outRate = -1.0;
  outputname = NULL;
  while((c = getopt(argc,argv,"Hhf:d:R:r:S:t:")) != EOF) {
    switch(c) {
    case 'h':
      USAGE
	case 'H':
	doHp = 1;
      break;
    case 'f':
      if(eqSpectrum) {
	fprintf(stderr,"Corner frequency OR spectral response can be specified, but NOT BOTH\n");
	USAGE
	  }
      corner_f = atof(optarg);
      break;
    case 'd':
      fDur = atof(optarg);
      break;
    case 't':
      testLev = atoi(optarg);
      break;
    case 'S':
      if(corner_f != 0.0) {
	fprintf(stderr,"Corner frequency OR spectral response can be specified, but NOT BOTH\n");
	USAGE
	  }
      eqSpectrum = optarg;
      break;
    case 'R':
      outRate = atof(optarg);
      break;
    case 'r':
      inRate = atof(optarg);
      break;
    }
  }
  if(inRate < 0.0) {
    inRate = outRate;
  }
  if(outRate < 0.0) {
    outRate = inRate;
  }
  if(inRate < 0.0) {
    inRate = outRate = 16000.0;
  }

  if(outRate != inRate) {
    if(eqSpectrum) {
      fprintf(stderr,"When -S is used, the input and output rates must be the same\n");
      USAGE
	}
    convertF = 1;
  }

  if( ! eqSpectrum ) {
    if(convertF) {
      corner_f = outRate;
      doHp = 0;
    }
    if(corner_f <= 0.0) {
      fprintf(stderr,"Corner frequency of filter must be specified (in Hz)\n");
      exit(-1);
    }
  }
  FdFilter *f;
  if(eqSpectrum) {
    if( ! (testLev & 1) ) {
    f = new FdFilter(inRate, eqSpectrum);
    } else {
      float *b;
      int pow2, nc, got, nuttin;
      char line[500];
      FILE *fi = fopen(eqSpectrum,"r");
      if(fi) {
	fgets(line,500,fi);
	got = sscanf(line,"%d %d",&pow2, &nc);
	if(got == 2) {
	  int n = 0;
	  b = new float [nc];
	  while(fgets(line,500,fi) && (n < nc)) {
	    sscanf(line,"%d %f",&nuttin, b+n);
	    n++;
	  }
	  fclose(fi);
	  f = new FdFilter(inRate, b, nc);
	} else {
	  fprintf(stderr,"Bad format in %s\n",eqSpectrum);
	  return(-1);
	}
      } else {
	fprintf(stderr,"Can't open %s as spectrum\n",eqSpectrum);
	return(-1);
      }
    }
  } else
    f = new FdFilter(inRate, corner_f, doHp, fDur, convertF);

#define A_SIZE 1000

  if(testLev & 2) { // Use buffer-to-buffer processing
    if(testLev & 4) { // Use small I/O buffers; messy, but possible
      short input[A_SIZE], output[A_SIZE];
      int moreInput = 1, nLeft = 0, inIndex = 0;
      int first = 1, last = 0;
      int nRead=0, toWrite=0, testc;

      while(moreInput) {
	while(nLeft > 0) {
	  toWrite = f->filterArray(input+inIndex, nLeft, first, last, output, A_SIZE);
	  fwrite(output,sizeof(short),toWrite,stdout);
	  nLeft -= f->getArraySamplesUsed();
	  inIndex += f->getArraySamplesUsed();
	}
	while(f->getArrayOutputLeftover() > 0) {
	  toWrite = f->filterArray(input,0,first,last,output,A_SIZE);
	  fwrite(output,sizeof(short),toWrite,stdout);
	}
	nRead = fread(input,sizeof(short),A_SIZE,stdin);
	
	testc = getc(stdin);
	if(feof(stdin)) {
	  moreInput = 0;
	  last = 1;
	} else
	  ungetc(testc,stdin);

	if(nRead > 0) {
	  toWrite = f->filterArray(input, nRead, first, last, output, A_SIZE);
	  inIndex = f->getArraySamplesUsed();
	  nLeft = nRead - inIndex;
	  fwrite(output,sizeof(short),toWrite,stdout);
	}
	first = 0;
      }
      while(f->getArrayOutputLeftover() > 0) {
	toWrite = f->filterArray(input,0,first,last,output,A_SIZE);
	fwrite(output,sizeof(short),toWrite,stdout);
      }
    } else { // Use recommended buffer size
      int bSize = f->getMaxInputSize();
      short *input = new short [bSize];
      short *output = new short [bSize*2];
      int nRead, toWrite,first=1,last=0, testc;
      while((nRead = fread(input,sizeof(short),bSize,stdin)) > 0) {
	testc = getc(stdin);
	if(feof(stdin))
	  last = 1;
	else
	  ungetc(testc,stdin);
	toWrite = f->filterArray(input,nRead,first,last,output,bSize*2);
	fwrite(output,sizeof(short),toWrite,stdout);
	first = 0;
      }
      delete input;
      delete output;
    }
  } else { // Use the simple stream-to-stream processing
    if( ! f->filterStream(stdin, stdout) )
      if(convertF)
	fprintf(stderr,"Couldn't resample to %fHz.\n", corner_f);
      else
	fprintf(stderr,"Error while filtering\n");
  }
  fclose(stdout);
  delete f;
  return(0);
}

#endif
