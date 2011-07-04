/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)lpcfloat.c	1.15	9/26/95	ATT/ERL";

/*   
 *
 *	An implementation of the Le Roux - Gueguen PARCOR computation.
 *	See: IEEE/ASSP June, 1977 pp 257-259.
 *	
 *	Author: David Talkin	Jan., 1984
 *
 */

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <esps/limits.h>
    /* for definition of DBL_MAX */

#define MAXORDER	60	/* maximum permissible LPC order */
#define FALSE 0
#define TRUE 1

#ifndef M_PI
#define M_PI    3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#endif


lgsol(p, r, k, ex)
/*  p	=	The order of the LPC analysis.
 *  r	=	The array of p+1 autocorrelation coefficients.
 *  k	=	The array of PARCOR coefficients returned by lgsol.
 *  ex	=	The normalized prediction residual or "gain."
 *		(Errors are flagged by ex < 0.)
 * All coefficients are returned in "normal" signed format,
 *	i.e. a[0] is assumed to be = 1.
 */

register int p;
register double *r, *k, *ex;

{
register int m, h;
double rl[MAXORDER+1], ep[MAXORDER], en[MAXORDER];
register double *q, *s, temp;

  if( p > MAXORDER ){
	printf("\n Specified lpc order to large in lgsol.\n");
	*ex = -1.;	/* Errors flagged by "ex" less than 0. */
	return;
  }	
  if( *r <= 0.){
	printf("\n Bad autocorelation coefficients in lgsol.\n");
	*ex = -2.;
	return;
  }
  if( *r != 1.){	/* Normalize the autocorrelation coeffs. */
	for( q = rl+1, s = r+1, m = 0; m < p; m++, q++, s++){
		*q = *s / *r;
	}
	*rl = 1.;
 	q = rl;		 /* point to local array of normalized coeffs. */
   
  }
  else  		 /* Point to normalized remote array. */
     		q = r;
   

/* Begin the L-G recursion. */
  for( s = q, m = 0; m < p; m++, s++){
	ep[m] = *(s+1);
	en[m] = *s;
  }
  for( h = 0; h < p; h++){
	k[h] = -ep[h]/en[0];
	*en += k[h]*ep[h];
	if(h == p-1) break;

	ep[p-1] += k[h]*en[p-h-1];
	for( m = h+1; m < p-1; m++){
		temp = ep[m] + k[h]*en[m-h];
		en[m-h] += k[h]*ep[m];
		ep[m] = temp;
	}
  }
  *ex = *en;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
get_window(dout, n, type)
     register double *dout;
     register int n;
{
  static short *din = NULL;
  static int n0 = 0;
  double preemp = 0.0;

  if(n > n0) {
    register short *p;
    register int i;
    
    if(din) free(din);
    din = NULL;
    if(!(din = (short*)malloc(sizeof(short)*n))) {
      printf("Allocation problems in get_window()\n");
      return(FALSE);
    }
    for(i=0, p=din; i++ < n; ) *p++ = 1;
    n0 = n;
  }
  switch(type) {
  case 0:
    rwindow(din, dout, n, preemp);
    break;
  case 1:
    hwindow(din, dout, n, preemp);
    break;
  case 2:
    cwindow(din, dout, n, preemp);
    break;
  case 3:
    hnwindow(din, dout, n, preemp);
    break;
  default:
    printf("Unknown window type (%d) requested in get_window()\n",type);
  }
  return(TRUE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
get_float_window(fout, n, type)
     register float *fout;
     register int n;
{
  static int n0 = 0;
  static double *dout = NULL;

  if(n > n0) {
    if(dout)free(dout);
    dout = NULL;
    if(!(dout = (double*)malloc(sizeof(double)*n))) {
      printf("Allocation problems in get_window()\n");
      return(FALSE);
    }
    n0 = n;
  }
  if(get_window(dout, n, type)) {
    register int i;
    register double *d;

    for(i=0, d = dout; i++ < n; ) *fout++ = *d++;
    return(TRUE);
  }
  return(FALSE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
fwindow(din, dout, n, preemp, type)
     register short *din;
     register float *dout, preemp;
     register int n;
{
  static float *fwind=NULL;
  static int size=0, otype= (-100);
  register int i;
  register float *q;
  register short *p;

  if(size != n) {
    if(fwind) fwind = (float*)realloc(fwind,sizeof(float)*(n+1));
    else fwind =  (float*)malloc(sizeof(float)*(n+1));
    if(!fwind) {
      printf("Allocation problems in fwindow\n");
      return(FALSE);
    }
    otype = -100;
    size = n;
  }
  if(type != otype) {
    get_float_window(fwind, n, type);
    otype = type;
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=fwind; i-- > 0; )
      *dout++ = *q++ * ((float)(*p++) - (preemp * *din++));
  } else {
    for(i=n, q=fwind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}
  
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* same as fwindow() but input is float */
fwindow_f(din, dout, n, preemp, type)
     register float *din;
     register float *dout, preemp;
     register int n;
{
  static float *fwind=NULL;
  static int size=0, otype= (-100);
  register int i;
  register float *q;
  register float *p;

  if(size != n) {
    if(fwind) fwind = (float*)realloc(fwind,sizeof(float)*(n+1));
    else fwind =  (float*)malloc(sizeof(float)*(n+1));
    if(!fwind) {
      printf("Allocation problems in fwindow\n");
      return(FALSE);
    }
    otype = -100;
    size = n;
  }
  if(type != otype) {
    get_float_window(fwind, n, type);
    otype = type;
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=fwind; i-- > 0; )
      *dout++ = *q++ * ((*p++) - (preemp * *din++));
  } else {
    for(i=n, q=fwind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}
  
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* same as fwindow() but I/O is double */
fwindow_d(din, dout, n, preemp, type)
     register double *din;
     register double *dout, preemp;
     register int n;
{
  static float *fwind=NULL;
  static int size=0, otype= (-100);
  register int i;
  register float *q;
  register double *p;

  if(size != n) {
    if(fwind) fwind = (float*)realloc(fwind,sizeof(float)*(n+1));
    else fwind =  (float*)malloc(sizeof(float)*(n+1));
    if(!fwind) {
      printf("Allocation problems in fwindow\n");
      return(FALSE);
    }
    otype = -100;
    size = n;
  }
  if(type != otype) {
    get_float_window(fwind, n, type);
    otype = type;
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=fwind; i-- > 0; )
      *dout++ = *q++ * ((*p++) - (preemp * *din++));
  } else {
    for(i=n, q=fwind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}
  


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
w_window(din, dout, n, preemp, type)
     register short *din;
     register double *dout, preemp;
     register int n;
{
  switch(type) {
  case 0:
    rwindow(din, dout, n, preemp);
    return;
  case 1:
    hwindow(din, dout, n, preemp);
    return;
  case 2:
    cwindow(din, dout, n, preemp);
    return;
  case 3:
    hnwindow(din, dout, n, preemp);
    return;
  default:
    printf("Unknown window type (%d) requested in w_window()\n",type);
  }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
rwindow(din, dout, n, preemp)
     register short *din;
     register double *dout, preemp;
     register int n;
{
  register short *p;
 
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for( p=din+1; n-- > 0; )
      *dout++ = (double)(*p++) - (preemp * *din++);
  } else {
    for( ; n-- > 0; )
      *dout++ =  *din++;
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
cwindow(din, dout, n, preemp)
     register short *din;
     register double *dout, preemp;
     register int n;
{
  register int i, j;
  register short *p;
  static int wsize = 0;
  static double *wind=NULL;
  register double *q, co;
 
  if(wsize != n) {		/* Need to create a new cos**4 window? */
    register double arg, half=0.5;
    
    if(wind) wind = (double*)realloc(wind,n*sizeof(double));
    else wind = (double*)malloc(n*sizeof(double));
    wsize = n;
    for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; ) {
      co = half*(1.0 - cos((half + (double)i++) * arg));
      *q++ = co * co * co * co;
    }
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=wind; i-- > 0; )
      *dout++ = *q++ * ((double)(*p++) - (preemp * *din++));
  } else {
    for(i=n, q=wind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
hwindow(din, dout, n, preemp)
     register short *din;
     register double *dout, preemp;
     register int n;
{
  register int i, j;
  register short *p;
  static int wsize = 0;
  static double *wind=NULL;
  register double *q;

  if(wsize != n) {		/* Need to create a new Hamming window? */
    register double arg, half=0.5;
    
    if(wind) wind = (double*)realloc(wind,n*sizeof(double));
    else wind = (double*)malloc(n*sizeof(double));
    wsize = n;
    for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; )
      *q++ = (.54 - .46 * cos((half + (double)i++) * arg));
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=wind; i-- > 0; )
      *dout++ = *q++ * ((double)(*p++) - (preemp * *din++));
  } else {
    for(i=n, q=wind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
hnwindow(din, dout, n, preemp)
     register short *din;
     register double *dout, preemp;
     register int n;
{
  register int i, j;
  register short *p;
  static int wsize = 0;
  static double *wind=NULL;
  register double *q;

  if(wsize != n) {		/* Need to create a new Hamming window? */
    register double arg, half=0.5;
    
    if(wind) wind = (double*)realloc(wind,n*sizeof(double));
    else wind = (double*)malloc(n*sizeof(double));
    wsize = n;
    for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; )
      *q++ = (half - half * cos((half + (double)i++) * arg));
  }
/* If preemphasis is to be performed,  this assumes that there are n+1 valid
   samples in the input buffer (din). */
  if(preemp != 0.0) {
    for(i=n, p=din+1, q=wind; i-- > 0; )
      *dout++ = *q++ * ((double)(*p++) - (preemp * *din++));
  } else {
    for(i=n, q=wind; i-- > 0; )
      *dout++ = *q++ * *din++;
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
autoc( windowsize, s, p, r, e )
register int windowsize, p;
register double *s, *r, *e;
/*
 * Compute the pp+1 autocorrelation lags of the windowsize samples in s.
 * Return the normalized autocorrelation coefficients in r.
 * The rms is returned in e.
 */
{
  register int i, j;
  register double *q, *t, sum, sum0;

  for ( i=0, q=s, sum0=0.; i< windowsize; q++, i++){
	sum0 += (*q) * (*q);
  }
  *r = 1.;  /* r[0] will always =1. */
  if ( sum0 == 0.){   /* No energy: fake low-energy white noise. */
  	*e = 1.;   /* Arbitrarily assign 1 to rms. */
		   /* Now fake autocorrelation of white noise. */
	for ( i=1; i<=p; i++){
		r[i] = 0.;
	}
	return;
  }
  for( i=1; i <= p; i++){
	for( sum=0., j=0, q=s, t=s+i; j < (windowsize)-i; j++, q++, t++){
		sum += (*q) * (*t);
	}
	*(++r) = sum/sum0;
  }
  if(sum0 < 0.0) printf("lpcfloat:autoc(): sum0 = %f\n",sum0);
  *e = sqrt(sum0/windowsize);
}


k_to_a ( k, a, p )
register int p;
register double *k, *a;
/*
 * Convert the p PARCOR parameters in k to LPC (AR) coefficients in a.
 */
{
    int i,j;
    double b[MAXORDER];

    *a = *k;
    for ( i=1; i < p; i++ ){
	a[i] = k[i];
	for ( j=0; j<=i; j++ ){
		b[j] = a[j];
	}
	for ( j=0; j<i; j++ ){
	    a[j] += k[i]*b[i-j-1];
	}
    }
}
   
durbin ( r, k, a, p, ex)
register int p;
register double *r, *k, *a, *ex;
/*
* Compute the AR and PARCOR coefficients using Durbin's recursion. 
* Note: Durbin returns the coefficients in normal sign format.
*	(i.e. a[0] is assumed to be = +1.)
*/
{
  double b[MAXORDER];
  register int i, j, l;
  register double e, s;

    e = *r;
    *k = -r[1]/e;
    *a = *k;
    e *= (1. - (*k) * (*k));
    for ( i=1; i < p; i++){
	s = 0;
	for ( j=0; j<i; j++){
		s -= a[j] * r[i-j];
	}
	k[i] = ( s - r[i+1] )/e;
	a[i] = k[i];
	for ( j=0; j<=i; j++){
		b[j] = a[j];
	}
	for ( j=0; j<i; j++){
		a[j] += k[i] * b[i-j-1];
	}
	e *= ( 1. - (k[i] * k[i]) );
    }
    *ex = e;
}

a_to_aca ( a, b, c, p )
double *a, *b, *c;
register int p;
/*  Compute the autocorrelations of the p LP coefficients in a. 
 *  (a[0] is assumed to be = 1 and not explicitely accessed.)
 *  The magnitude of a is returned in c.
 *  2* the other autocorrelation coefficients are returned in b.
 */
{
    double		s;
    register short	i, j, pm;
	for ( s=1., i = 0; i < p; i++ ){
		s += (a[i] * a[i]);
	}
	*c = s;
	pm = p-1;
	for ( i = 0; i < p; i++){
		s = a[i];
		for ( j = 0; j < (pm-i); j++){
			s += (a[j] * a[j+i+1]);
		}
		b[i] = 2. * s;
	}

}

double itakura ( p, b, c, r, gain )
register double *b, *c, *r, *gain;
register int p;
/* Compute the Itakura LPC distance between the model represented
 * by the signal autocorrelation (r) and its residual (gain) and
 * the model represented by an LPC autocorrelation (c, b).
 * Both models are of order p.
 * r is assumed normalized and r[0]=1 is not explicitely accessed.
 * Values returned by the function are >= 1.
 */
 {
     double s;
     register int i;
     for( s= *c, i=0; i< p; i++){
	s += r[i] * b[i];
    }
    return (s/ *gain);
}

lpc(lpc_ord,lpc_stabl,wsize,data,lpca,ar,lpck,normerr,rms,preemp,type)
     int lpc_ord, wsize, type;
     double lpc_stabl, *lpca, *ar, *lpck, *normerr, *rms, preemp;
     short *data;
{
  static double *dwind=NULL;
  static int nwind=0;
  double rho[MAXORDER+1], k[MAXORDER], a[MAXORDER+1],*r,*kp,*ap,en,er;
  double wfact = 1.0;

  if((wsize <= 0) || (!data) || (lpc_ord > MAXORDER)) return(FALSE);
  
  if(nwind != wsize) {
    if(dwind) dwind = (double*)realloc(dwind,wsize*sizeof(double));
    else dwind = (double*)malloc(wsize*sizeof(double));
    if(!dwind) {
      printf("Can't allocate scratch memory in lpc()\n");
      return(FALSE);
    }
    nwind = wsize;
  }
  
  w_window(data, dwind, wsize, preemp, type);
  if(!(r = ar)) r = rho;
  if(!(kp = lpck)) kp = k;
  if(!(ap = lpca)) ap = a;
  autoc( wsize, dwind, lpc_ord, r, &en );
  if(lpc_stabl > 1.0) { /* add a little to the diagonal for stability */
    int i;
    double ffact;
    ffact =1.0/(1.0 + exp((-lpc_stabl/20.0) * log(10.0)));
    for(i=1; i <= lpc_ord; i++) rho[i] = ffact * r[i];
    *rho = *r;
    r = rho;
    if(ar)
      for(i=0;i<=lpc_ord; i++) ar[i] = r[i]; /* copy out for possible use later */
  }
  durbin ( r, kp, &ap[1], lpc_ord, &er);

/* After talkin with David T., I decided to take the following correction
out -- if people window, the resulting output (spectrum and power) should
correspond to the windowed data, and we shouldn't try to back-correct
to un-windowed data. */

/*  switch(type) {		/* rms correction for window */
/*   case 0:
    wfact = 1.0;		/* rectangular */
/*    break;
  case 1:
    wfact = .630397;		/* Hamming */
/*    break;
  case 2:
    wfact = .443149;		/* (.5 - .5*cos)^4 */
/*    break;
  case 3:
    wfact = .612372;		/* Hanning */
/*    break;
  }
*/

  *ap = 1.0;
  if(rms) *rms = en/wfact;
  if(normerr) *normerr = er;
  return(TRUE);
}

/* convert reflection (PARCOR) coefficients to areas */
dreflar(c,a,n) double *c,*a; int n;{
/*	refl to area			*/
	register double *pa,*pa1,*pc,*pcl;
	pa = a + 1;	pa1 = a;
	*a = 1.;
	for(pc=c,pcl=c+ n;pc<pcl;pc++)
	*pa++ = *pa1++ * (1+*pc)/(1-*pc);
	}

/* covariance LPC analysis; originally from Markel and Gray */
/* (a translation from the fortran) */

w_covar(xx,m,n,istrt,y,alpha,r0,preemp,w_type)
     double *y, *alpha, *r0, preemp;
     short *xx;
     int *m,n,istrt;
     int w_type;
{
  static double *x=NULL;
  static int nold = 0, ofile = -1;
  static mold = 0;
  static double *b = NULL, *beta = NULL, *grc = NULL, *cc = NULL, gam,s;
 int ibeg, ibeg1, ibeg2, ibegm2, ibegmp, np0, ibegm1, msq, np, np1, mf, jp, ip,
     mp, i, j, minc, n1, n2, n3, npb, msub, mm1, isub, m2;
  int mnew = 0;

  if((n+1) > nold) {
    if(x) free(x);
    x = NULL;
    if(!(x = (double*)calloc((n+1), sizeof(double)))) {
      printf("Allocation failure in w_covar()\n");
      return(FALSE);
    }
    nold = n+1;
  }

  if(*m > mold) {
    if(b) free(b); if(beta) free(beta); if (grc) free(grc); if (cc) free(cc);
    b = beta = grc = cc = NULL;
    mnew = *m;
    
    if(!((b = (double*)malloc(sizeof(double)*((mnew+1)*(mnew+1)/2))) && 
       (beta = (double*)malloc(sizeof(double)*(mnew+3)))  &&
       (grc = (double*)malloc(sizeof(double)*(mnew+3)))  &&
       (cc = (double*)malloc(sizeof(double)*(mnew+3)))))   {
      printf("Allocation failure in w_covar()\n");
      return(FALSE);
    }
    mold = mnew;
  }

  w_window(xx, x, n, preemp, w_type);  

 ibeg = istrt - 1;
 ibeg1 = ibeg + 1;
 mp = *m + 1;
 ibegm1 = ibeg - 1;
 ibeg2 = ibeg + 2;
 ibegmp = ibeg + mp;
 i = *m;
 msq = ( i + i*i)/2;
 for (i=1; i <= msq; i++) b[i] = 0.0;
 *alpha = 0.0;
 cc[1] = 0.0;
 cc[2] = 0.0;
 for(np=mp; np <= n; np++) {
   np1 = np + ibegm1;
   np0 = np + ibeg;
   *alpha += x[np0] * x[np0];
   cc[1] += x[np0] * x[np1];
   cc[2] += x[np1] * x[np1];
 }
 *r0 = *alpha;
 b[1] = 1.0;
 beta[1] = cc[2];
 grc[1] = -cc[1]/cc[2];
 y[0] = 1.0;
 y[1] = grc[1];
 *alpha += grc[1]*cc[1];
 if( *m <= 1) return(FALSE);		/* need to correct indices?? */
 mf = *m;
 for( minc = 2; minc <= mf; minc++) {
   for(j=1; j <= minc; j++) {
     jp = minc + 2 - j;
     n1 = ibeg1 + mp - jp;
     n2 = ibeg1 + n - minc;
     n3 = ibeg2 + n - jp;
     cc[jp] = cc[jp - 1] + x[ibegmp-minc]*x[n1] - x[n2]*x[n3];
   }
   cc[1] = 0.0;
   for(np = mp; np <= n; np++) {
     npb = np + ibeg;
     cc[1] += x[npb-minc]*x[npb];
   }
   msub = (minc*minc - minc)/2;
   mm1 = minc - 1;
   b[msub+minc] = 1.0;
   for(ip=1; ip <= mm1; ip++) {
     isub = (ip*ip - ip)/2;
     if(beta[ip] <= 0.0) {
       *m = minc-1;
       return(TRUE);
     }
     gam = 0.0;
     for(j=1; j <= ip; j++)
       gam += cc[j+1]*b[isub+j];
     gam /= beta[ip];
     for(jp=1; jp <= ip; jp++)
       b[msub+jp] -= gam*b[isub+jp];
   }
   beta[minc] = 0.0;
   for(j=1; j <= minc; j++)
     beta[minc] += cc[j+1]*b[msub+j];
   if(beta[minc] <= 0.0) {
     *m = minc-1;
     return(TRUE);
   }
   s = 0.0;
   for(ip=1; ip <= minc; ip++)
     s += cc[ip]*y[ip-1];
   grc[minc] = -s/beta[minc];
   for(ip=1; ip < minc; ip++) {
     m2 = msub+ip;
     y[ip] += grc[minc]*b[m2];
   }
   y[minc] = grc[minc];
   s = grc[minc]*grc[minc]*beta[minc];
   *alpha -= s;
   if(*alpha <= 0.0) {
     if(minc < *m) *m = minc;
     return(TRUE);
   }
 }
}

/* Same as above, but returns alpha as a function of order. */
covar2(xx,m,n,istrt,y,alpha,r0,preemp)
     double *y, *alpha, *r0, preemp;
     short *xx;
     int *m,n,istrt;
{
  static double *x=NULL;
  static int nold = 0;
  double b[513],beta[33],grc[33],cc[33],gam,s;
  int ibeg, ibeg1, ibeg2,ibegm2, ibegmp, np0, ibegm1, msq, np, np1, mf, jp, ip,
     mp, i, j, minc, n1, n2, n3, npb, msub, mm1, isub, m2;

  if((n+1) > nold) {
    if(x) free(x);
    x = NULL;
    if(!(x = (double*)malloc(sizeof(double)*(n+1)))) {
      printf("Allocation failure in covar2()\n");
      return(FALSE);
    }
    nold = n+1;
  }

 for(i=1; i <= n; i++) x[i] = (double)xx[i] - preemp * xx[i-1];
 ibeg = istrt - 1;
 ibeg1 = ibeg + 1;
 mp = *m + 1;
 ibegm1 = ibeg - 1;
 ibeg2 = ibeg + 2;
 ibegmp = ibeg + mp;
 i = *m;
 msq = ( i + i*i)/2;
 for (i=1; i <= msq; i++) b[i] = 0.0;
 *alpha = 0.0;
 cc[1] = 0.0;
 cc[2] = 0.0;
 for(np=mp; np <= n; np++) {
   np1 = np + ibegm1;
   np0 = np + ibeg;
   *alpha += x[np0] * x[np0];
   cc[1] += x[np0] * x[np1];
   cc[2] += x[np1] * x[np1];
 }
 *r0 = *alpha;
 b[1] = 1.0;
 beta[1] = cc[2];
 grc[1] = -cc[1]/cc[2];
 y[0] = 1.0;
 y[1] = grc[1];
 *alpha += grc[1]*cc[1];
 if( *m <= 1) return;		/* need to correct indices?? */
 mf = *m;
 for( minc = 2; minc <= mf; minc++) {
   for(j=1; j <= minc; j++) {
     jp = minc + 2 - j;
     n1 = ibeg1 + mp - jp;
     n2 = ibeg1 + n - minc;
     n3 = ibeg2 + n - jp;
     cc[jp] = cc[jp - 1] + x[ibegmp-minc]*x[n1] - x[n2]*x[n3];
   }
   cc[1] = 0.0;
   for(np = mp; np <= n; np++) {
     npb = np + ibeg;
     cc[1] += x[npb-minc]*x[npb];
   }
   msub = (minc*minc - minc)/2;
   mm1 = minc - 1;
   b[msub+minc] = 1.0;
   for(ip=1; ip <= mm1; ip++) {
     isub = (ip*ip - ip)/2;
     if(beta[ip] <= 0.0) {
       *m = minc-1;
       return;
     }
     gam = 0.0;
     for(j=1; j <= ip; j++)
       gam += cc[j+1]*b[isub+j];
     gam /= beta[ip];
     for(jp=1; jp <= ip; jp++)
       b[msub+jp] -= gam*b[isub+jp];
   }
   beta[minc] = 0.0;
   for(j=1; j <= minc; j++)
     beta[minc] += cc[j+1]*b[msub+j];
   if(beta[minc] <= 0.0) {
     *m = minc-1;
     return;
   }
   s = 0.0;
   for(ip=1; ip <= minc; ip++)
     s += cc[ip]*y[ip-1];
   grc[minc] = -s/beta[minc];
   for(ip=1; ip < minc; ip++) {
     m2 = msub+ip;
     y[ip] += grc[minc]*b[m2];
   }
   y[minc] = grc[minc];
   s = grc[minc]*grc[minc]*beta[minc];
   alpha[minc-1] = alpha[minc-2] - s;
   if(alpha[minc-1] <= 0.0) {
     if(minc < *m) *m = minc;
     return;
   }
 }
}
   
	 
/*      ----------------------------------------------------------      */
/* Find the roots of the LPC denominator polynomial and convert the z-plane
	zeros to equivalent resonant frequencies and bandwidths.	*/
/* The complex poles are then ordered by frequency.  */
formant(lpc_order,s_freq,lpca,n_form,freq,band,init)
int	lpc_order, /* order of the LP model */
	*n_form,   /* number of COMPLEX roots of the LPC polynomial */
	init; 	   /* preset to true if no root candidates are available */
double	s_freq,    /* the sampling frequency of the speech waveform data */
	*lpca, 	   /* linear predictor coefficients */
	*freq,     /* returned array of candidate formant frequencies */
	*band;     /* returned array of candidate formant bandwidths */
{
  double  x, flo, pi2t, theta;
  static double  rr[MAXORDER], ri[MAXORDER];
  int	i,ii,iscomp1,iscomp2,fc,swit;

  if(init){ /* set up starting points for the root search near unit circle */
    x = M_PI/(lpc_order + 1);
    for(i=0;i<=lpc_order;i++){
      flo = lpc_order - i;
      rr[i] = 2.0 * cos((flo + 0.5) * x);
      ri[i] = 2.0 * sin((flo + 0.5) * x);
    }
  }
  if(! lbpoly(lpca,lpc_order,rr,ri)){ /* find the roots of the LPC polynomial */
    *n_form = 0;		/* was there a problem in the root finder? */
    return(FALSE);
  }

  pi2t = M_PI * 2.0 /s_freq;

  /* convert the z-plane locations to frequencies and bandwidths */
  for(fc=0, ii=0; ii < lpc_order; ii++){
    if((rr[ii] != 0.0)||(ri[ii] != 0.0)){
      theta = atan2(ri[ii],rr[ii]);
      freq[fc] = fabs(theta / pi2t);
      if((band[fc] = 0.5 * s_freq *
	  log(((rr[ii] * rr[ii]) + (ri[ii] * ri[ii])))/M_PI) < 0.0)
	band[fc] = -band[fc];
      fc++;			/* Count the number of real and complex poles. */

      if((rr[ii] == rr[ii+1])&&(ri[ii] == -ri[ii+1]) /* complex pole? */
	 && (ri[ii] != 0.0)) ii++; /* if so, don't duplicate */
    }
  }


  /* Now order the complex poles by frequency.  Always place the (uninteresting)
     real poles at the end of the arrays. 	*/
  theta = s_freq/2.0;		/* temporarily hold the folding frequency. */
  for(i=0; i < fc -1; i++){	/* order the poles by frequency (bubble) */
    for(ii=0; ii < fc -1 -i; ii++){
      /* Force the real poles to the end of the list. */
      iscomp1 = (freq[ii] > 1.0) && (freq[ii] < theta);
      iscomp2 = (freq[ii+1] > 1.0) && (freq[ii+1] < theta);
      swit = (freq[ii] > freq[ii+1]) && iscomp2 ;
      if(swit || (iscomp2 && ! iscomp1)){
	flo = band[ii+1];
	band[ii+1] = band[ii];
	band[ii] = flo;
	flo = freq[ii+1];
	freq[ii+1] = freq[ii];
	freq[ii] = flo;
      }
    }
  }
  /* Now count the complex poles as formant candidates. */
  for(i=0, theta = theta - 1.0, ii=0 ; i < fc; i++)
    if( (freq[i] > 1.0) && (freq[i] < theta) ) ii++;
  *n_form = ii;

  return(TRUE);
}



/*-----------------------------------------------------------------------*/
log_mag(x,y,z,n)
			/* z <- (10 * log10(x^2 + y^2))  for n elements */
double	*x, *y, *z;
int	n;
{
register double	*xp, *yp, *zp, t1, t2, ssq;

    if(x && y && z && n) {
        for(xp=x+n, yp=y+n, zp=z+n; zp > z;) {
	    t1 = *--xp;
	    t2 = *--yp;
	    ssq = (t1*t1)+(t2*t2);
	    *--zp = (ssq > 0.0)? 10.0 * log10(ssq) : -200.0;
	}
	return(TRUE);
    } else {
	return(FALSE);
    }
}

/*-----------------------------------------------------------------------*/
flog_mag(x,y,z,n)
			/* z <- (10 * log10(x^2 + y^2))  for n elements */
float	*x, *y, *z;
int	n;
{
register float	*xp, *yp, *zp, t1, t2, ssq;

    if(x && y && z && n) {
        for(xp=x+n, yp=y+n, zp=z+n; zp > z;) {
	    t1 = *--xp;
	    t2 = *--yp;
	    ssq = (t1*t1)+(t2*t2);
	    *--zp = (ssq > 0.0)? 10.0 * log10((double)ssq) : -200.0;
	}
	return(TRUE);
    } else {
	return(FALSE);
    }
}

#ifdef USE_OLD_FFT
#include	"thetable.c"	/*  table of sines and cosines */
/*-----------------------------------------------------------------------*/
fft ( l, x, y )
int l;
double *x, *y;
/* Compute the discrete Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
register double	c, s,  t1, t2;
register int	j1, j2, li, lix, i;
int	np, lmx, lo, lixnp, lm, j, nv2, k, im, jm;

    for ( np=1, i=1; i <= l; i++) np *= 2;

    if(fft_tablesize < (np/2)) {
	printf("\nPrecomputed trig tables are not big enough in fft().\n");
	return(FALSE);
    }
    k = (fft_tablesize * 2)/np;
        
    for (lmx=np, lo=0; lo < l; lo++, k *= 2) {
	lix = lmx;
	lmx /= 2;
	lixnp = np - lix;
	for (i=0, lm=0; lm<lmx; lm++, i += k ) {
	    c = cosine[i];
	    s = sine[i];
	    for ( li = lixnp+lm, j1 = lm, j2 = lm+lmx; j1<=li;
	    				j1+=lix, j2+=lix ) {
		t1 = x[j1] - x[j2];
		t2 = y[j1] - y[j2];
		x[j1] += x[j2];
		y[j1] += y[j2];
		x[j2] = (c * t1) + (s * t2);
		y[j2] = (c * t2) - (s * t1);
	    }
	}
    }

	/* Now perform the bit reversal. */

    j = 1;
    nv2 = np/2;
    for ( i=1; i < np; i++ ) {
	if ( j < i ) {
	    jm = j-1;
	    im = i-1;
	    t1 = x[jm];
	    t2 = y[jm];
	    x[jm] = x[im];
	    y[jm] = y[im];
	    x[im] = t1;
	    y[im] = t2;
	}
	k = nv2;
	while ( j > k ) {
	    j -= k;
	    k /= 2;
	}
	j += k;
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------*/
ifft ( l, x, y )
int l;
double *x, *y;
/* Compute the discrete inverse Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
register double	c, s,  t1, t2;
register int	j1, j2, li, lix, i;
int	np, lmx, lo, lixnp, lm, j, nv2, k, im, jm;

    for ( np=1, i=1; i <= l; i++) np *= 2;

    if(fft_tablesize < (np/2)) {
	printf("\nPrecomputed trig tables are not big enough in ifft().\n");
	return(FALSE);
    }
    k = (fft_tablesize * 2)/np;
        
    for (lmx=np, lo=0; lo < l; lo++, k *= 2) {
	lix = lmx;
	lmx /= 2;
	lixnp = np - lix;
	for (i=0, lm=0; lm<lmx; lm++, i += k ) {
	    c = cosine[i];
	    s = - sine[i];
	    for ( li = lixnp+lm, j1 = lm, j2 = lm+lmx; j1<=li;
	    				j1+=lix, j2+=lix ) {
		t1 = x[j1] - x[j2];
		t2 = y[j1] - y[j2];
		x[j1] += x[j2];
		y[j1] += y[j2];
		x[j2] = (c * t1) + (s * t2);
		y[j2] = (c * t2) - (s * t1);
	    }
	}
    }

	/* Now perform the bit reversal. */

    j = 1;
    nv2 = np/2;
    for ( i=1; i < np; i++ ) {
	if ( j < i ) {
	    jm = j-1;
	    im = i-1;
	    t1 = x[jm];
	    t2 = y[jm];
	    x[jm] = x[im];
	    y[jm] = y[im];
	    x[im] = t1;
	    y[im] = t2;
	}
	k = nv2;
	while ( j > k ) {
	    j -= k;
	    k /= 2;
	}
	j += k;
    }
    return(TRUE);
}

#include	"theftable.c"	/*  floating table of sines and cosines */
/*-----------------------------------------------------------------------*/
ffft ( l, x, y )
int l;
float *x, *y;
/* Compute the discrete Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
register float	c, s,  t1, t2;
register int	j1, j2, li, lix, i;
int	np, lmx, lo, lixnp, lm, j, nv2, k, im, jm;

    for ( np=1, i=1; i <= l; i++) np *= 2;

    if(fft_ftablesize < (np/2)) {
	printf("\nPrecomputed trig tables are not big enough in fft().\n");
	return(FALSE);
    }
    k = (fft_ftablesize * 2)/np;
        
    for (lmx=np, lo=0; lo < l; lo++, k *= 2) {
	lix = lmx;
	lmx /= 2;
	lixnp = np - lix;
	for (i=0, lm=0; lm<lmx; lm++, i += k ) {
	    c = fcosine[i];
	    s = fsine[i];
	    for ( li = lixnp+lm, j1 = lm, j2 = lm+lmx; j1<=li;
	    				j1+=lix, j2+=lix ) {
		t1 = x[j1] - x[j2];
		t2 = y[j1] - y[j2];
		x[j1] += x[j2];
		y[j1] += y[j2];
		x[j2] = (c * t1) + (s * t2);
		y[j2] = (c * t2) - (s * t1);
	    }
	}
    }

	/* Now perform the bit reversal. */

    j = 1;
    nv2 = np/2;
    for ( i=1; i < np; i++ ) {
	if ( j < i ) {
	    jm = j-1;
	    im = i-1;
	    t1 = x[jm];
	    t2 = y[jm];
	    x[jm] = x[im];
	    y[jm] = y[im];
	    x[im] = t1;
	    y[im] = t2;
	}
	k = nv2;
	while ( j > k ) {
	    j -= k;
	    k /= 2;
	}
	j += k;
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------*/
fifft ( l, x, y )
int l;
float *x, *y;
/* Compute the discrete inverse Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
register float	c, s,  t1, t2;
register int	j1, j2, li, lix, i;
int	np, lmx, lo, lixnp, lm, j, nv2, k, im, jm;

    for ( np=1, i=1; i <= l; i++) np *= 2;

    if(fft_ftablesize < (np/2)) {
	printf("\nPrecomputed trig tables are not big enough in ifft().\n");
	return(FALSE);
    }
    k = (fft_ftablesize * 2)/np;
        
    for (lmx=np, lo=0; lo < l; lo++, k *= 2) {
	lix = lmx;
	lmx /= 2;
	lixnp = np - lix;
	for (i=0, lm=0; lm<lmx; lm++, i += k ) {
	    c = fcosine[i];
	    s = - fsine[i];
	    for ( li = lixnp+lm, j1 = lm, j2 = lm+lmx; j1<=li;
	    				j1+=lix, j2+=lix ) {
		t1 = x[j1] - x[j2];
		t2 = y[j1] - y[j2];
		x[j1] += x[j2];
		y[j1] += y[j2];
		x[j2] = (c * t1) + (s * t2);
		y[j2] = (c * t2) - (s * t1);
	    }
	}
    }

	/* Now perform the bit reversal. */

    j = 1;
    nv2 = np/2;
    for ( i=1; i < np; i++ ) {
	if ( j < i ) {
	    jm = j-1;
	    im = i-1;
	    t1 = x[jm];
	    t2 = y[jm];
	    x[jm] = x[im];
	    y[jm] = y[im];
	    x[im] = t1;
	    y[im] = t2;
	}
	k = nv2;
	while ( j > k ) {
	    j -= k;
	    k /= 2;
	}
	j += k;
    }
    return(TRUE);
}
#endif

/*********************************************************************/
/* Simple-minded real DFT (slooooowww) */
dft(n,x,re,im)
     register int n;
     double *x, *re, *im;
{
  register int m = n/2, i, j, k;
  register double arg, sr, si, a, *rp;

  for(i=0; i <= m; i++) {
    arg = i * 3.1415927/m;
    for(rp=x, j=0, sr=0.0, si=0.0; j < n; j++) {
      sr += cos((a = j*arg))* *rp;
      si += sin(a) * *rp++;
    }
    *re++ = sr;
    *im++ = si;
  }
}

/*		lbpoly.c		*/
/*					*/
/* A polynomial root finder using the Lin-Bairstow method (outlined
	in R.W. Hamming, "Numerical Methods for Scientists and
	Engineers," McGraw-Hill, 1962, pp 356-359.)		*/


#define MAX_ITS	100	/* Max iterations before trying new starts */
#define MAX_TRYS	100	/* Max number of times to try new starts */
#define MAX_ERR		1.e-6	/* Max acceptable error in quad factor */

qquad(a,b,c,r1r,r1i,r2r,r2i) /* find x, where a*x**2 + b*x + c = 0 	*/
double	a, b, c;
double *r1r, *r2r, *r1i, *r2i; /* return real and imag. parts of roots */
{
double  sqrt(), numi;
double  den, y;

	if(a == 0.0){
		if(b == 0){
		   printf("Bad coefficients to _quad().\n");
		   return(FALSE);
		}
		*r1r = -c/b;
		*r1i = *r2r = *r2i = 0;
		return(TRUE);
	}
	numi = b*b - (4.0 * a * c);
	if(numi >= 0.0) {
		/*
		 * Two forms of the quadratic formula:
		 *  -b + sqrt(b^2 - 4ac)           2c
		 *  ------------------- = --------------------
		 *           2a           -b - sqrt(b^2 - 4ac)
		 * The r.h.s. is numerically more accurate when
		 * b and the square root have the same sign and
		 * similar magnitudes.
		 */
		*r1i = *r2i = 0.0;
		if(b < 0.0) {
			y = -b + sqrt(numi);
			*r1r = y / (2.0 * a);
			*r2r = (2.0 * c) / y;
		}
		else {
			y = -b - sqrt(numi);
			*r1r = (2.0 * c) / y;
			*r2r = y / (2.0 * a);
		}
		return(TRUE);
	}
	else {
		den = 2.0 * a;
		*r1i = sqrt( -numi )/den;
		*r2i = -*r1i;
		*r2r = *r1r = -b/den;
		return(TRUE);
	}
}

lbpoly(a, order, rootr, rooti) /* return FALSE on error */
    double  *a;		    /* coeffs. of the polynomial (increasing order) */
    int	    order;	    /* the order of the polynomial */
    double  *rootr, *rooti; /* the real and imag. roots of the polynomial */
    /* Rootr and rooti are assumed to contain starting points for the root
       search on entry to lbpoly(). */
{
    int	    ord, ordm1, ordm2, itcnt, i, j, k, mmk, mmkp2, mmkp1, ntrys;
    double  fabs(), err, p, q, delp, delq, b[MAXORDER], c[MAXORDER], den;
    double  lim0 = 0.5*sqrt(DBL_MAX);

    for(ord = order; ord > 2; ord -= 2){
	ordm1 = ord-1;
	ordm2 = ord-2;
	/* Here is a kluge to prevent UNDERFLOW! (Sometimes the near-zero
	   roots left in rootr and/or rooti cause underflow here...	*/
	if(fabs(rootr[ordm1]) < 1.0e-10) rootr[ordm1] = 0.0;
	if(fabs(rooti[ordm1]) < 1.0e-10) rooti[ordm1] = 0.0;
	p = -2.0 * rootr[ordm1]; /* set initial guesses for quad factor */
	q = (rootr[ordm1] * rootr[ordm1]) + (rooti[ordm1] * rooti[ordm1]);
	for(ntrys = 0; ntrys < MAX_TRYS; ntrys++)
	{
	    int	found = FALSE;

	    for(itcnt = 0; itcnt < MAX_ITS; itcnt++)
	    {
		double	lim = lim0 / (1 + fabs(p) + fabs(q));

		b[ord] = a[ord];
		b[ordm1] = a[ordm1] - (p * b[ord]);
		c[ord] = b[ord];
		c[ordm1] = b[ordm1] - (p * c[ord]);
		for(k = 2; k <= ordm1; k++){
		    mmk = ord - k;
		    mmkp2 = mmk+2;
		    mmkp1 = mmk+1;
		    b[mmk] = a[mmk] - (p* b[mmkp1]) - (q* b[mmkp2]);
		    c[mmk] = b[mmk] - (p* c[mmkp1]) - (q* c[mmkp2]);
		    if (b[mmk] > lim || c[mmk] > lim)
			break;
		}
		if (k > ordm1) { /* normal exit from for(k ... */
		    /* ????	b[0] = a[0] - q * b[2];	*/
		    b[0] = a[0] - p * b[1] - q * b[2];
		    if (b[0] <= lim) k++;
		}
		if (k <= ord)	/* Some coefficient exceeded lim; */
		    break;	/* potential overflow below. */

		err = fabs(b[0]) + fabs(b[1]);

		if(err <= MAX_ERR) {
		    found = TRUE;
		    break;
		}

		den = (c[2] * c[2]) - (c[3] * (c[1] - b[1]));  
		if(den == 0.0)
		    break;

		delp = ((c[2] * b[1]) - (c[3] * b[0]))/den;
		delq = ((c[2] * b[0]) - (b[1] * (c[1] - b[1])))/den;  

		/* printf("\nerr=%f  delp=%f  delq=%f  p=%f  q=%f",
		   err,delp,delq,p,q); */

		p += delp;
		q += delq;

	    } /* for(itcnt... */

	    if (found)		/* we finally found the root! */
		break;
	    else { /* try some new starting values */
		p = ((double)rand() - (1<<30))/(1<<31);
		q = ((double)rand() - (1<<30))/(1<<31);
		/* fprintf(stderr, "\nTried new values: p=%f  q=%f\n",p,q); */
	    }

	} /* for(ntrys... */
	if((itcnt >= MAX_ITS) && (ntrys >= MAX_TRYS)){
	    printf("Exceeded maximum trial count in _lbpoly.\n");
	    return(FALSE);
	}

	if(!qquad(1.0, p, q,
		  &rootr[ordm1], &rooti[ordm1], &rootr[ordm2], &rooti[ordm2]))
	    return(FALSE);

	/* Update the coefficient array with the coeffs. of the
	   reduced polynomial. */
	for( i = 0; i <= ordm2; i++) a[i] = b[i+2];
    }

    if(ord == 2){		/* Is the last factor a quadratic? */
	if(!qquad(a[2], a[1], a[0],
		  &rootr[1], &rooti[1], &rootr[0], &rooti[0]))
	    return(FALSE);
	return(TRUE);
    }
    if(ord < 1) {
	printf("Bad ORDER parameter in _lbpoly()\n");
	return(FALSE);
    }

    if( a[1] != 0.0) rootr[0] = -a[0]/a[1];
    else {
	rootr[0] = 100.0;	/* arbitrary recovery value */
	printf("Numerical problems in lbpoly()\n");
    }
    rooti[0] = 0.0;

    return(TRUE);
}

/*
main()
{
  int i,n;
  int j;
  double a[MAXORDER],rootr[MAXORDER],rooti[MAXORDER];

  while(1){
    printf("\nOrder of the polynomial to be solved: ");
    scanf("%d",&j);
    n = j;
    printf("\nEnter the coefficients of the polynomial in increasing order.\n");
    for(i=0;i <= n;i++){
      printf("a\(%2d\) : ",i);
      scanf("%lf",&a[i]);
      rootr[i] = 22.0; 
      rooti[i] = -10.0;
    }
    if(! lbpoly(a,n,rootr,rooti)){
      printf("\nProblem in root solver.\n");
    } else {
      for(i=0;i<n;i++)printf("\n%d  re %f    im%f",i,rootr[i],rooti[i]);
    }
  }
}
*/


#ifdef STANDALONE
main(ac, av)
     int ac;
     char **av;
{
  int i,j,k, nfft=9;
  FILE *fd, *fopen();
  double x[4096], y[4096];
  char line[500];

  if((fd = fopen(av[1],"r"))) {
    i=0;
    while((i < 512) && fgets(line,500,fd)) {
      sscanf(line,"%lf",&x[i]);
      y[i++] = 0.0;
    }
    for( ; i < 512; i++) {
      x[i] = y[i] = 0.0;
    }
    fft(nfft,x,y);
    log_mag(x,y,x,257);
    for(i=0; i<257; i++) {
      printf("%4d %7.3lf\n",i,x[i]);
    }
  }
}
#endif
