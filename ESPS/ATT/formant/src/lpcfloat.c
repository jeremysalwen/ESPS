/*   
 *
 *	An implementation of the Le Roux - Gueguen PARCOR computation.
 *	See: IEEE/ASSP June, 1977 pp 257-259.
 *	
 *	Author: David Talkin	Jan., 1984
 *
 */

#ifndef lint
	static char *sccs_id = "@(#)lpcfloat.c	1.1 12/14/88		ATT, ESI ";
#endif

#include <stdio.h>
#include <math.h>
#include <fcntl.h>

#define MAXORDER	30	/* maximum permissible LPC order */
#define FALSE 0
#define TRUE 1


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
window(din, dout, n, preemp, type)
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
  default:
    printf("Unknown window type (%d) requested in window()\n",type);
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
    register double arg;
    
    if(wind) wind = (double*)realloc(wind,n*sizeof(double));
    else wind = (double*)malloc(n*sizeof(double));
    wsize = n;
    for(i=0, arg=3.1415927*2.0/wsize, q=wind; i < n; ) {
      co = 0.5*(1.0 - cos(((double)i++) * arg));
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
    register double arg;
    
    if(wind) wind = (double*)realloc(wind,n*sizeof(double));
    else wind = (double*)malloc(n*sizeof(double));
    wsize = n;
    for(i=0, arg=3.1415927*2.0/(wsize+1), q=wind; i < n; )
      *q++ = (.54 - .46 * cos(((double)i++) * arg));
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
    register int i, j, l;
    register double b[MAXORDER], e, s;

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
  
  window(data, dwind, wsize, preemp, type);
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
  }
  durbin ( r, kp, &ap[1], lpc_ord, &er);
  *ap = 1.0;
  if(rms) *rms = en;
  if(normerr) *normerr = er;
  return(TRUE);
}

/* covariance LPC analysis; originally from Markel and Gray */
/* (a translation from the fortran) */

covar(xx,m,n,istrt,y,alpha,r0,preemp)
     double *y, *alpha, *r0, preemp;
     short *xx;
     int *m,n,istrt;
{
  static double *x=NULL;
  static int nold = 0;
  double b[513],beta[33],grc[33],cc[33],gam,s;
 int ibeg, ibeg1, ibeg2, ibegm2, ibegmp, np0, ibegm1, msq, np, np1, mf, jp, ip,
     mp, i, j, minc, n1, n2, n3, npb, msub, mm1, isub, m2;

 y--;			/* KLUGE to fortranify the array */

  if((n+1) > nold) {
    if(x) free(x);
    x = NULL;
    if(!(x = (double*)malloc(sizeof(double)*(n+1)))) {
      printf("Allocation failure in covar()\n");
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
 y[1] = 1.0;
 y[2] = grc[1];
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
     s += cc[ip]*y[ip];
   grc[minc] = -s/beta[minc];
   for(ip=2; ip <= minc; ip++) {
     m2 = msub+ip-1;
     y[ip] += grc[minc]*b[m2];
   }
   y[minc+1] = grc[minc];
   s = grc[minc]*grc[minc]*beta[minc];
   *alpha -= s;
   if(*alpha <= 0.0) {
     if(minc < *m) *m = minc;
     return;
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

  y--;			/* KLUGE to fortranify the array */

  if((n+1) > nold) {
    if(x) free(x);
    x = NULL;
    if(!(x = (double*)malloc(sizeof(double)*(n+1)))) {
      printf("Allocation failure in covar()\n");
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
 y[1] = 1.0;
 y[2] = grc[1];
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
     s += cc[ip]*y[ip];
   grc[minc] = -s/beta[minc];
   for(ip=2; ip <= minc; ip++) {
     m2 = msub+ip-1;
     y[ip] += grc[minc]*b[m2];
   }
   y[minc+1] = grc[minc];
   s = grc[minc]*grc[minc]*beta[minc];
   alpha[minc-1] = alpha[minc-2] - s;
   if(alpha[minc-1] <= 0.0) {
     if(minc < *m) *m = minc;
     return;
   }
 }
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
	    *--zp = (ssq)? 10.0 * log10(ssq) : -200.0;
	}
	return(TRUE);
    } else {
	return(FALSE);
    }
}

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

#include	"thetable.c"	/*  table of sines and cosines */

    for ( np=1, i=1; i <= l; i++) np *= 2;

    if(tablesize < (np/2)) {
	printf("\nPrecomputed trig tables are not big enough in fft().\n");
	return(FALSE);
    }
    k = (tablesize * 2)/np;
        
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
