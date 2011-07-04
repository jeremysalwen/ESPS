/* fft.c */

#include "fft.h"
 

FFT::FFT(int power)
{
  makefttable(power);
}

FFT::~FFT()
{
  delete fsine;
  delete fcosine;
}

/*-----------------------------------------------------------------------*/
/* z <- (10 * log10(x^2 + y^2))  for n elements */
int FFT::flog_mag(float *x, float *y, float *z, int n)
{
float	*xp, *yp, *zp, t1, t2, ssq;

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

/*-----------------------------------------------------------------------*/
int FFT::makefttable(int pow2)
{
  int lmx, lm;
  float *c, *s;
  double	scl, arg;

  fftSize = 1 << pow2;
  fft_ftablesize = lmx = fftSize/2;
  fsine = new float [lmx];
  fcosine = new float [lmx];
  scl = (M_PI*2.0)/fftSize;
  for (s=fsine, c=fcosine, lm=0; lm<lmx; lm++ ) {
    arg = scl * lm;
    *s++ = sin(arg);
    *c++ = cos(arg);
  }
  kbase = (fft_ftablesize * 2)/fftSize;
  power2 = pow2;

  return(fft_ftablesize);
}

/*-----------------------------------------------------------------------*/
void FFT::fft ( float *x, float *y )
/* Compute the discrete Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
  float	c, s,  t1, t2;
  int	j1, j2, li, lix, i;
  int	lmx, lo, lixnp, lm, j, nv2, k=kbase, im, jm, l = power2;


  for (lmx=fftSize, lo=0; lo < l; lo++, k *= 2) {
    lix = lmx;
    lmx /= 2;
    lixnp = fftSize - lix;
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
  nv2 = fftSize/2;
  for ( i=1; i < fftSize; i++ ) {
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
}

/*-----------------------------------------------------------------------*/
void FFT::ifft ( float *x, float *y )
/* Compute the discrete inverse Fourier transform of the 2**l complex sequence
 * in x (real) and y (imaginary).  The DFT is computed in place and the
 * Fourier coefficients are returned in x and y.
 */
{
  float	c, s,  t1, t2;
  int	j1, j2, li, lix, i;
  int	lmx, lo, lixnp, lm, j, nv2, k=kbase, im, jm, l = power2;

  for (lmx=fftSize, lo=0; lo < l; lo++, k *= 2) {
    lix = lmx;
    lmx /= 2;
    lixnp = fftSize - lix;
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
  nv2 = fftSize/2;
  for ( i=1; i < fftSize; i++ ) {
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
}

