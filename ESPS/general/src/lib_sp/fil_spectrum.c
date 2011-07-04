/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 *               Original FFT part is written by Brian Sublett and Bill Bryne
 *               for the old filtspec
 * Checked by:
 * Revised by:
 *
 * Brief description: compute magnitude and phase response from a feafilt
 *    data record.
 *
 */

static char *sccs_id = "@(#)fil_spectrum.c	1.2	12/21/92	ERL";

#define SMALL 1.0e-30
#define LOGSMALL -600
#define BIG 1.0e+30

#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <esps/feafilt.h>
#include <esps/unix.h>
#include <esps/constants.h>
#include <esps/spsassert.h>

double sqrt();
extern int debug_level;

/* NOW IIR can use any nsamp, but FIR must be a power of 2*/

int fil_spectrum( spec_rec, mag, phase, filt_rec, fhd, nsamp)
     struct feaspec *spec_rec;
     double *mag;
     double *phase;
     struct feafilt *filt_rec;
     struct header *fhd;
     long nsamp;
{
  char *FuncName="Filt_cplx_spectrum";
  int i,j, code = 0;
  
  if( filt_rec->pole_dim == 0 || filt_rec->poles == NULL) code = 1;

  switch(code){
/*
 * POLE/ZERO SYSTEM
 */
  case 0:
    {
      int psiz=0, zsiz=0;
      register double up, down;
      register double_cplx *poles, *zeros;
      double pslot;
      double magv, phasev;
      register double_cplx *uc;  /* frequencies uni-spaced around unit circle*/
      short cplx_flag = 0;
      double gain=1;

      zeros = filt_rec->zeros;
      zsiz = *filt_rec->zero_dim;
      psiz = *filt_rec->pole_dim;
      poles = filt_rec->poles;	
      
      i=1;
      if( genhd_type("filter_complex", &i, fhd) != HD_UNDEF)
	if( *get_genhd_s("filter_complex", fhd) == YES ) cplx_flag = 1;

      if(filt_rec->re_num_coeff != NULL && *filt_rec->num_size >= 1){
	gain = filt_rec->re_num_coeff[0];
	if(debug_level) Fprintf(stderr,"filter gain %e from re_num_coeff[0]\n"
				, gain);
      }
      else
	if(debug_level) Fprintf(stderr,"IIR filter gain = 1\n");


      uc = (double_cplx *) malloc( nsamp * sizeof(double_cplx));
      spsassert(uc, "fil_spectrum: uc malloc failed");
      pslot = PI / nsamp;
      for (i=0; i<nsamp; i++){
	uc[i].real = cos( pslot * i);
	uc[i].imag = sin( pslot * i);
      }

      if(debug_level){
	if(cplx_flag)
	  Fprintf(stderr,"%s: This is a complex filter\n", FuncName);
	else
	  Fprintf(stderr,"%s: This is a real filter\n", FuncName);
	for(i=0;i<psiz;i++) Fprintf(stderr,"poles[%d]:real %f imag %f\n",
				    i,poles[i].real, poles[i].imag);
	for(i=0;i<zsiz;i++) Fprintf(stderr,"zeros[%d]:real %f imag %f\n",
				    i,zeros[i].real, zeros[i].imag);
      }

/* 
 * Magnitude 
 */
      for (i=0; i<nsamp; i++){
	up = 1;
	down = 1;
	for( j=0; j<psiz; j++ ){
	  down *= (uc[i].real - poles[j].real) * (uc[i].real - poles[j].real) 
	    + (uc[i].imag - poles[j].imag) * (uc[i].imag - poles[j].imag);
	  if( !cplx_flag && poles[j].imag != 0)
	    down *= (uc[i].real - poles[j].real)* (uc[i].real -poles[j].real)
	      + (uc[i].imag + poles[j].imag) * (uc[i].imag + poles[j].imag);
	}
	for( j=0; j<zsiz; j++ ){
	  up *= (uc[i].real - zeros[j].real) * (uc[i].real - zeros[j].real) 
	    + (uc[i].imag - zeros[j].imag) * (uc[i].imag - zeros[j].imag);
	  if( !cplx_flag && zeros[j].imag !=0 )
	    up *= (uc[i].real - zeros[j].real) * (uc[i].real - zeros[j].real) 
	      + (uc[i].imag + zeros[j].imag) * (uc[i].imag + zeros[j].imag);
	}
	magv = gain * sqrt(up/down);
/*
 * Phase
 */
	up = 0;
	down = 0;
	for( j=0; j<psiz; j++){
	  down += atan2(poles[j].imag-uc[i].imag, poles[j].real-uc[i].real);
	  if (!cplx_flag && poles[j].imag !=0)
	    down += atan2(-poles[j].imag-uc[i].imag,poles[j].real-uc[i].real);
	}

	for( j=0; j<zsiz; j++){
	  /* avoid atan2 error, when uc coicide with zero on unit circle */
	  if( zeros[j].imag == uc[i].imag && zeros[j].real == uc[i].real){}
	  else{
	    up += atan2(zeros[j].imag-uc[i].imag, zeros[j].real-uc[i].real);
	    if ( !cplx_flag && zeros[j].imag !=0)
	      up += atan2(-zeros[j].imag-uc[i].imag,zeros[j].real-uc[i].real);
	  }
	}
	phasev = up - down;

	if(spec_rec){
	  spec_rec->re_spec_val[i] = (float) magv * cos(phasev);
	  spec_rec->im_spec_val[i] = (float) magv * sin(phasev);
	}
	if(mag)  mag[i] = magv;
	if(phase) phase[i] = phasev;
      }
      break;
    }
/*
 * FIR or IIR with only num/denom coeffs available 
 */
  case 1:
    {
      double *realn, *reald;
      double *imagn, *imagd;
      double magn, phasen, magd, phased;
      int nsiz=0, dsiz=0;
      int order;
      int fftbins;

      nsiz = *filt_rec->num_size;
      dsiz = *filt_rec->denom_size;

      for(i=1,j=nsamp-1; i<=j; i*=2) {}
      i/=2;
      if(i-j !=0){
	Fprintf(stderr, "%s: number of frequencies must be a power of 2 plus 1\n", FuncName);
	return(1);
      }

      fftbins = (nsamp-1) * 2;
      order = log_2(fftbins);
      if(debug_level) Fprintf(stderr,"order of FFT: %d\n", order);

      realn = (double*) calloc ((unsigned)fftbins, sizeof (double));
      spsassert(realn != NULL, "Couldn't allocate memory for realn");
      imagn = (double*) calloc ((unsigned)fftbins, sizeof (double));
      spsassert(imagn != NULL, "Couldn't allocate memory for imagn");
      reald = (double*) calloc ((unsigned)fftbins, sizeof (double));
      spsassert(reald != NULL, "Couldn't allocate memory for reald");
      imagd = (double*) calloc ((unsigned)fftbins, sizeof (double));
      spsassert(imagd != NULL, "Couldn't allocate memory for imagd");

      if(debug_level)
	Fprintf(stderr,"%s: This is a real filter\n",FuncName);
      if(debug_level>2){
	for(i=0;i<dsiz;i++)
	  Fprintf(stderr,"denom[%d] %e\n", i, filt_rec->re_denom_coeff[i]);
	for(i=0;i<nsiz;i++)
	  Fprintf(stderr,"num[%d] %e\n", i, filt_rec->re_num_coeff[i]);
      }
      /* Get Numerator polynomial and compute amplitude spectrm */
      for (i=0; i<nsiz; i++)
	    {
	      realn[i] = filt_rec->re_num_coeff[i];
	      imagn[i] = 0.0;
	    }
      for (i=nsiz; i<fftbins; i++)
	{
	  realn[i] = imagn[i] = 0.0;
	}
      
      get_fftd (realn, imagn, order);
      if(debug_level > 5){
	for(i=0;i<nsamp;i++)
			Fprintf(stderr, 
				"numerator: real[%d] = %f, imag[%d] = %f\n",
				i, realn[i], i, imagn[i]);
      }
      /* Get denominator polynomial and compute amplitude spectrum */
      if (dsiz > 0)
	{
	  for (i=0; i<dsiz; i++)
	    {
	      reald[i] = filt_rec->re_denom_coeff[i];
	      imagd[i] = 0.0;
	    }
	  for (i=dsiz; i<fftbins; i++)
	    {
	      reald[i] = imagd[i] = 0.0;
	    }

            (void) get_fftd (reald, imagd, order);
	  if(debug_level > 5){
	    for(i=0;i< nsamp;i++)
	      Fprintf(stderr,
		      "denominator: real[%d] = %f, imag[%d] = %f\n",
		      i,reald[i],i,imagd[i]);
	  }
	}
      
      if (dsiz > 0)
	{/* Find magnitude and phase of numerator and denominator*/
	  for (i=0; i<nsamp; i++)
	    {/*First look at numerator*/
	      magn = hypot (realn[i],imagn[i]);
	      
	      if ((fabs (realn[i])) > SMALL 
		  && (fabs (imagn[i])) > SMALL)
		{ /*compute phase angle*/
		  phasen = atan2 (imagn[i], realn[i]);
		}
	      else if (fabs (realn[i]) < SMALL 
		       && fabs (imagn[i]) > SMALL)
		{/* special Cases */
		  if (imagn[i] < 0)
		    {
		      phasen = -PI/2;
		    }
		  else
		    {
		      phasen = PI/2;
		    }
		}
	      else if (fabs (imagn[i]) < SMALL 
		       && fabs (realn[i]) > SMALL)
		{/* More special cases*/
		  if (realn [i] < 0)
		    {
		      phasen = PI;
		    }
		  else
		    {
		      phasen = 0;
		    }
		}
	      else
		{/* Both SMALL */
		  phasen = 0;
		}
	      
	      /* Now look at denominator*/
	      magd = hypot (reald[i],imagd[i]);
	      
	      if ((fabs (reald[i]) > SMALL) 
		  && (fabs (imagd[i]) > SMALL))
		{/* Compute phase */
		  phased = atan2 (imagd[i], reald[i]);
		}
	      else if (fabs (reald[i]) < SMALL 
		       && fabs (imagd[i]) > SMALL)
		{/* Special Cases */
		  if (imagd[i] < 0)
		    phased = -PI/2;
		  else
		    phased = PI/2;
		}
	      else if (fabs (imagd[i]) < SMALL 
		       && fabs (reald[i]) > SMALL)
		{/* More special cases */
		  if (reald [i] < 0)
		    phased = PI;
		  else
		    phased = 0;
		}
	      else
		{/* Both SMALL*/
		  phased = 0;
		}
	      
	      if (magd != 0)
		magn = magn/magd;
	      else
		{
		  if (magn == 0)
		    magn = 1.0;
		  else
		    magn = BIG;
		}
	      phasen = phasen - phased;
	      if(spec_rec){
		spec_rec->re_spec_val[i] = (float)magn*cos (phasen);
		spec_rec->im_spec_val[i] = (float)magn*sin (phasen);
	      }
	      if(mag) mag[i] = magn;
	      if(phase) phase[i] = phasen;
	    }
	}
      else /*Only numerator spectrum*/
	{
	  for (i=0; i<nsamp; i++)
	    {
	      if(spec_rec){
		spec_rec->re_spec_val[i] = (float)realn [i];
		spec_rec->im_spec_val[i] = (float)imagn [i];
	      }
	      if(mag) mag[i] = sqrt(realn[i]*realn[i]+imagn[i]*imagn[i]);
	      if(phase) phase[i] = atan2(imagn[i],realn[i]);
	    }
	}
      break;
    }/*case*/
  }/*switch*/
  return(0);
}


int
log_2 (n)
/*return power of 2 <= n */
int n;
    {
    int p, i=0;
    for (p=1; p<n; p*=2)
	{
	i++;
	}
    return (i);
    }
