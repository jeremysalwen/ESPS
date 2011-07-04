/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

#ifndef lint
	static char *sccs_id = "@(#)downsample.c	1.4 1/26/93		ATT, ESI ";
#endif

/* downsample.c */
/* a quick and dirty downsampler */
#include <Objects.h>

#define PI 3.1415927

/* ----------------------------------------------------------------------- */
Signal *downsample(s,freq2)
     double freq2;
     Signal *s;
{
  short	*bufin, **bufout;
  static double	beta = 0.0, b[256];
  double	ratio_t, maxi, maxc, ratio, beta_new, freq1;
  static int	ncoeff = 127, ncoefft = 0, nbits = 15;
  static short	ic[256];
  int	insert, decimate, out_samps, smin, smax;
  Signal *so;

  register int i, j, k, l;
  register short	*bufp;
  char *cp, temp[200];

  if(s) {
    freq1 = s->freq;
  cp = new_ext(s->name,"ds");
  if((bufout = (short**)malloc(sizeof(short*))) &&
     (so=new_signal(cp,SIG_UNKNOWN,dup_header(s->header),bufout,s->buff_size,
		    s->freq,s->dim))) {
    so->start_time = s->start_time + (((double)s->start_samp)/s->freq);
    bufin = ((short**)s->data)[0];
    ratio = freq2/freq1;
    ratprx(ratio,&insert,&decimate,10);
    ratio_t = ((double)insert)/((double)decimate);

    if(ratio_t > .99) return(s);
  
    freq2 = ratio_t * freq1;
    beta_new = (.5 * freq2)/(insert * freq1);

    if(beta != beta_new){
      beta = beta_new;
      if( !lc_lin_fir(beta,&ncoeff,b)) {
	printf("\nProblems computing interpolation filter\n");
	return(FALSE);
      }
      maxi = (1 << nbits) - 1;
      j = (ncoeff/2) + 1;
      for(ncoefft = 0, i=0; i < j; i++){
	ic[i] = 0.5 + (maxi * b[i]);
	if(ic[i]) ncoefft = i+1;
      }
    }				/*  endif new coefficients need to be computed */

    if(dwnsamp(bufin,s->buff_size,bufout,&out_samps,insert,decimate,ncoefft,ic,
	       &smin,&smax)){
      double fmin = smin, fmax = smax;
      so->buff_size = so->file_size = out_samps;
      so->freq = freq2;
      so->band = freq2/2.0;
      so->version += 1;
      head_printf(so->header,"version",&(so->version));
      sprintf(temp,"downsample: ncoefft %d window Hanning signal %s",
	      ncoefft, s->name);
      head_printf(so->header,"operation",temp);
      head_printf(so->header,"samples",&(so->buff_size));
      head_printf(so->header,"frequency",&(so->freq));
      head_printf(so->header,"bandwidth",&(so->band));
      head_printf(so->header,"minimum",&fmin);
      head_printf(so->header,"maximum",&fmax);
      head_printf(so->header,"time",get_date());
      return(so);
    } else
      printf("Problems in dwnsamp() in downsample()\n");
  } else
       printf("Can't create a new Signal in downsample()\n");
  } else
    printf("Null Signal passed to downsample()\n");
  
  return(NULL);
}
  

/* ******************************************************************** */
get_abs_maximum(d,n)
     register short *d;
     register int n;
{
  register int i;
  register short amax, t;

  if((t = *d++) >= 0) amax = t;
  else amax = -t;
  
  for(i = n-1; i-- > 0; ) {
    if((t = *d++) > amax) amax = t;
    else {
      if(-t > amax) amax = -t;
    }
  }
  return((int)amax);
}

/* ******************************************************************** */
dwnsamp(buf,in_samps,buf2,out_samps,insert,decimate,ncoef,ic,smin,smax)
     short	*buf, **buf2;
     int	in_samps, *out_samps, insert, decimate, ncoef, *smin, *smax;
     short ic[];
{
  register short  *bufp, *bufp2;
  short	*buft;
  register int i, j, k, l, m;
  int imax, imin;
    
  if(!(*buf2 = buft = (short*)malloc(sizeof(short)*insert*in_samps))) {
    perror("malloc() in dwnsamp()");
    return(FALSE);
  }
    
  k = imax = get_abs_maximum(buf,in_samps);
  if(insert > 1) k = (32767 * 32767)/k;	/*  prepare to scale data */
  else k = (16384 * 32767)/k;
  l = 16384;
  m = 15;
    

  /* Insert zero samples to boost the sampling frequency and scale the
     signal to maintain maximum precision. */
  for(i=0, bufp=buft, bufp2=buf; i < in_samps; i++) {
    *bufp++ = ((k * (*bufp2++)) + l) >> m ; 
    for(j=1; j < insert; j++) *bufp++ = 0;
  }
    
  do_fir(buft,in_samps*insert,buft,ncoef,ic,0);
    
  /*	Finally, decimate and return the downsampled signal. */
  *out_samps = j = (in_samps * insert)/decimate;
  k = decimate;
  for(i=0, bufp=buft, imax = imin = *bufp; i < j; bufp += k,i++) {
    *buft++ = *bufp;
    if(imax < *bufp) imax = *bufp;
    else
      if(imin > *bufp) imin = *bufp;
  }
  *smin = imin;
  *smax = imax;
  *buf2 = (short*)realloc(*buf2,sizeof(short) * (*out_samps));
  return(TRUE);
}


/*      ----------------------------------------------------------      */
do_fir(buf,in_samps,bufo,ncoef,ic,invert)
/* ic contains 1/2 the coefficients of a symmetric FIR filter with unity
    passband gain.  This filter is convolved with the signal in buf.
    The output is placed in buf2.  If invert != 0, the filter magnitude
    response will be inverted. */
short	*buf, *bufo, ic[];
int	in_samps, ncoef, invert;
{
    register short  *buft, *bufp, *bufp2, stem;
    short co[256], mem[256];
    register int i, j, k, l, m, sum, integral;
    int lim;
    
    for(i=ncoef-1, bufp=ic+ncoef-1, bufp2=co, buft = co+((ncoef-1)*2),
	integral = 0; i-- > 0; )
      if(!invert) *buft-- = *bufp2++ = *bufp--;
      else {
	integral += (stem = *bufp--);
	*buft-- = *bufp2++ = -stem;
      }
    if(!invert)  *buft-- = *bufp2++ = *bufp--; /* point of symmetry */
    else {
      integral *= 2;
      integral += *bufp;
      *buft-- = integral - *bufp;
    }
/*         for(i=(ncoef*2)-2; i >= 0; i--) printf("\n%4d%7d",i,co[i]);  */
    for(i=ncoef-1, buft=mem; i-- > 0; ) *buft++ = 0;
    for(i=ncoef; i-- > 0; ) *buft++ = *buf++;
    l = 16384;
    m = 15;
    k = (ncoef << 1) -1;
    for(i=in_samps-ncoef; i-- > 0; ) {
      for(j=k, buft=mem, bufp=co, bufp2=mem+1, sum = 0; j-- > 0;
	  *buft++ = *bufp2++ )
	sum += (((*bufp++ * *buft) + l) >> m);

      *--buft = *buf++;		/* new data to memory */
      *bufo++ = sum; 
    }
    for(i=ncoef; i-- > 0; ) {	/* pad data end with zeros */
      for(j=k, buft=mem, bufp=co, bufp2=mem+1, sum = 0; j-- > 0;
	  *buft++ = *bufp2++ )
	sum += (((*bufp++ * *buft) + l) >> m);
      *--buft = 0;
      *bufo++ = sum; 
    }
}

/*      ----------------------------------------------------------      */
lc_lin_fir(fc,nf,coef)
/* create the coefficients for a symmetric FIR lowpass filter using the
   window technique with a Hanning window. */
register double	fc;
double	coef[];
int	*nf;
{
    register int	i, n, ieo;
    register double	twopi, fn, c;

    if(((*nf % 2) != 1) || (*nf > 127)) {
	if(*nf <= 126) *nf = *nf + 1;
	else *nf = 127;
    }
    n = (*nf + 1)/2;

    /*  compute part of the ideal impulse response */
    twopi = PI * 2.0;
    coef[0] = 2.0 * fc;
    c = PI;
    fn = twopi * fc;
    for(i=1;i < n; i++) coef[i] = sin(i * fn)/(c * i);

    /* Now apply a Hanning window to the (infinite) impulse response. */
    fn = twopi/((double)(*nf - 1));
    for(i=0;i<n;i++) 
	coef[i] *= (.5 + (.5 * cos(fn * ((double)i))));
    
    return(TRUE);
}



/*      ----------------------------------------------------------      */
ratprx(a,k,l,qlim)
double	a;    
int	*l, *k, qlim;
{
    double	aa, af, q, em, qq, pp, ps, e;
    int	ai, ip, i;
    
    aa = fabs(a);
    ai = aa;
/*    af = fmod(aa,1.0); */
    i = aa;
    af = aa - i;
    q = 0;
    em = 1.0;
    while(++q <= qlim) {
	ps = q * af;
	ip = ps + 0.5;
	e = fabs((ps - (double)ip)/q);
	if(e < em) {
	    em = e;
	    pp = ip;
	    qq = q;
	}
    };
    *k = (ai * qq) + pp;
    *k = (a > 0)? *k : -(*k);
    *l = qq;
    return(TRUE);    
}

