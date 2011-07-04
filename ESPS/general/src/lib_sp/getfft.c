/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Written by:  Shankar Narayan
 * Partly rewritten by: David Talkin
 * (should be completely rewritten)
 * Module:	getfft.c
 *
 * This module contains various fast fourier transform functions:
 *
 * get_fft:     computes the complex DFT of floats using FFT algorithm
 * get_fftd:    computes the complex DFT of doubles using FFT algorithm
 * get_rfft:    computes the FFT from real data FFT algorithm (float)
 * get_rfftd:   computes the FFT from real data FFT algorithm (double)
 * get_fft_inv: computes the complex IDFT using FFT algorithm
 * get_fftd_inv: computes the complex IDFT using FFT algorithm
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#ifndef lint
static char *sccs_id = "@(#)getfft.c	1.11 2/3/92 ESI";
#endif

double	sin(), cos(), pow();

/* Fourier transform of array x[2**log_fft_size] */
void
get_fft(x_real, x_imag, log_fft_size)
register float   x_real[], x_imag[];
int     log_fft_size;
{
    register float  t_real, t_imag, w_imag, w_real;
    double  	    ang, twopibyn;
    int		    l, stage, two_power_stage, ij;
    register int    i, s, sby2, p, q;
    static float    *cos_tab = NULL, *sin_tab = NULL;
    static int      old_log_fft_size = 0;
    static int	    *map = NULL;
    static int	    fft_size;

	/* Initialization */

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (sin_tab != NULL) free((char *) sin_tab);
	sin_tab = (float *) malloc(fft_size / 2 * sizeof(float));
	spsassert(sin_tab,
	    "get_fft: couldn't allocate dynamic memory for array - sin_tab\n");

	if (cos_tab != NULL) free((char *) cos_tab);
	cos_tab = (float *) malloc(fft_size / 2 * sizeof(float));
	spsassert(cos_tab,
	    "get_fft: couldn't allocate dynamic memory for array - cos_tab\n");

	if (map != NULL) free((char *) map);
	map = (int *) malloc(fft_size * sizeof(int));
	spsassert(map,
	    "get_fft: couldn't allocate dynamic memory for array - map\n");

	/* Generate bit-reversal table */
	for (i = 0; i < fft_size; i++)
	    map[i] = 0;
	for (i = 0; i < fft_size; i++)
	{
	    p = i;
	    q = 0;
	    for (l = 1; l <= log_fft_size; l++)
	    {
		q = 2 * q + (p % 2);
		p = p / 2;
	    }
	    if (i < q)
		map[i] = q;
	}

	/* Compute twiddle factors */

	twopibyn = 6.283185307 / fft_size;
	for (i = 0; i < fft_size / 2; i++)
	{
	    ang = i * twopibyn;
	    cos_tab[i] = cos(ang);
	    sin_tab[i] = -sin(ang);
	}
    }

    sby2 = fft_size;
    two_power_stage = 1;
    for (stage = 1; stage <= log_fft_size; stage++)
    {
	ij = 0;
	s = sby2;
	sby2 = s / 2;
	for (l = 0; l < sby2; l++)
	{
	    w_real = cos_tab[ij];
	    w_imag = sin_tab[ij];

	    p = l;
	    q = p + sby2;
	    for (i = fft_size; i > 0; i -= s)
	    {
		t_real = x_real[p];
		t_imag = x_imag[p];
		x_real[p] = t_real + x_real[q];
		x_imag[p] = t_imag + x_imag[q];

		t_real -= x_real[q];
		t_imag -= x_imag[q];
		x_real[q] = t_real * w_real - t_imag * w_imag;
		x_imag[q] = t_real * w_imag + t_imag * w_real;

		p += s;
		q = p + sby2;
	    }
	    ij += two_power_stage;
	}
	two_power_stage *= 2;
    }
    /* perform bit-reversal operation */
    {
      register float *fip, *frp;
      register int *mp;
      for (frp = x_real, fip = x_imag, mp = map, i = fft_size; i--; frp++, fip++)
	{
	  if((q = *mp++))	{
	    t_real = *frp;
	    *frp = x_real[q];
	    x_real[q] = t_real;

	    t_imag = *fip;
	    *fip = x_imag[q];
	    x_imag[q] = t_imag;
	  }
	}
    }
}


/* Inverse Fourier transform of array x[2**log_fft_size] */
void
get_fft_inv(x_real, x_imag, log_fft_size)
float   x_real[], x_imag[];
int     log_fft_size;
{
    register float  t_real, t_imag, w_imag, w_real;
    register        p, q;
    double  	    ang, twopibyn;
    int     	    i, l, s, sby2, stage, two_power_stage, ij;
    static float    *cos_tab = NULL, *sin_tab = NULL;
    static int	    old_log_fft_size = 0;
    static int	    *map = NULL;
    static int	    fft_size;

    /* Initialization */

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (sin_tab != NULL) free((char *) sin_tab);
	sin_tab = (float *) malloc(fft_size / 2 * sizeof(float));
	spsassert(sin_tab,
	    "get_fft_inv: couldn't allocate dynamic memory for array - sin_tab\n");

	if (cos_tab != NULL) free((char *) cos_tab);
	cos_tab = (float *) malloc(fft_size / 2 * sizeof(float));
	spsassert(cos_tab,
	    "get_fft_inv: couldn't allocate dynamic memory for array - cos_tab\n");

	if (map != NULL) free((char *) map);
	map = (int *) malloc(fft_size * sizeof(int));
	spsassert(map,
	    "get_fft_inv: couldn't allocate dynamic memory for array - map\n");

	/* Generate bit-reversal table */
	for (i = 0; i < fft_size; i++)
	    map[i] = 0;
	for (i = 0; i < fft_size; i++)
	{
	    p = i;
	    q = 0;
	    for (l = 1; l <= log_fft_size; l++)
	    {
		q = 2 * q + (p % 2);
		p = p / 2;
	    }
	    if (i < q)
		map[i] = q;
	}

	/* Compute twiddle factors */

	twopibyn = 6.283185307 / fft_size;
	for (i = 0; i < fft_size / 2; i++)
	{
	    ang = i * twopibyn;
	    cos_tab[i] = cos(ang);
	    sin_tab[i] = -sin(ang);
	}
    }

/* Complex Conjugation of input data */
    for (i = 0; i < fft_size; i++)
	x_imag[i] = -x_imag[i];

    sby2 = fft_size;
    two_power_stage = 1;
    for (stage = 1; stage <= log_fft_size; stage++)
    {
	ij = 0;
	s = sby2;
	sby2 = s / 2;
	for (l = 0; l < sby2; l++)
	{
	    w_real = cos_tab[ij];
	    w_imag = sin_tab[ij];

	    p = l;
	    q = p + sby2;
	    for (i = 0; i < fft_size; i += s)
	    {
		t_real = x_real[p];
		t_imag = x_imag[p];
		x_real[p] = t_real + x_real[q];
		x_imag[p] = t_imag + x_imag[q];

		t_real -= x_real[q];
		t_imag -= x_imag[q];
		x_real[q] = t_real * w_real - t_imag * w_imag;
		x_imag[q] = t_real * w_imag + t_imag * w_real;

		p += s;
		q = p + sby2;
	    }
	    ij += two_power_stage;
	}
	two_power_stage *= 2;
    }
    /* perform bit-reversal operation */
    for (i = 0; i < fft_size; i++)
    {
	q = map[i];
	if (q)
	{
	    t_real = x_real[i];
	    x_real[i] = x_real[q];
	    x_real[q] = t_real;

	    t_imag = x_imag[i];
	    x_imag[i] = x_imag[q];
	    x_imag[q] = t_imag;
	}
    }
/* Complex Conjugation of Output Data and scaling by factor fft_size */
    for (i = 0; i < fft_size; i++)
    {
	x_real[i] = x_real[i] / fft_size;
	x_imag[i] = -x_imag[i] / fft_size;
    }
}

void
get_rfft(real, imag, log_fft_size)
float   real[], imag[];
int     log_fft_size;
{
  register float *fpri, *fprj, *fpii, *fpij, t;
  double  twopibyn;
  static float *sint=NULL, *cost=NULL;
  static int tab_size = 0;
  register int i, size;
  int log_sizeby2, size_by_2;

  size = ROUND(pow(2.0, (double) log_fft_size));
  size_by_2 = size / 2;
  log_sizeby2 = log_fft_size - 1;
  twopibyn = 2.0 * 3.141592654 / size;
  if(tab_size != size) {
    register int lim;
    if(tab_size < size) {
      if(sint && cost) {
	free(sint);
	free(cost);
	sint = cost = NULL;
      }
      if( (sint = (float*) malloc(sizeof(float)*(size/4+1))) == NULL || 
	 (cost = (float*) malloc(sizeof(float)*(size/4+1))) == NULL) {
	  fprintf(stderr,"Allocation problems in get_rfft()\n");
	  exit(-1);
      }
  }
    for(i = 0, fpri = sint, fpii = cost, lim = size/4; i <= lim;
	i++) {
      *fpii++ = cos((t = twopibyn * i));
      *fpri++ = sin(t);
    }
    tab_size = size;
  }
  if (size == 1) return;
  /* Combine Even and odd sequences to get
     complex data of length = size / 2 */
  for(i= size>>1, fpri = fprj = real, fpii = imag; i--; ) {
    *fpri++ = *fprj++;
    *fpii++ = *fprj++;
  }

  get_fft(real, imag, log_sizeby2);


  /* Separate reven and odd parts  */
  {
    register float   er, ei, or, oi, st, ct, half = 0.5;
    register int smi, smj, j, lim;

    j = size_by_2;
    t = real[0];
    real[0] += imag[0];
    real[size_by_2] = t - imag[0];
    imag[0] = imag[size_by_2] = 0.0;
    for (fpri = real+1, fprj = real + size_by_2 - 1,
	 fpii = imag+1, fpij = imag+size_by_2 - 1, i = 1,
	 lim = size/4; i <= lim; i++)
      {
	j--;
	ct = cost[i];
	st = sint[i];
	er = (*fpri + *fprj)*half;
	ei = (*fpii - *fpij)*half;
	oi = (*fprj - *fpri)*half;
	or = (*fpii + *fpij)*half;
	t = (ct * or) + (st * oi);
	*fpri++ = real[(smi = size - i)] = er + t;
	*fprj-- = real[(smj = size - j)] = er - t;

	t = ct * oi - st * or;
	*fpii = ei + t;
	imag[smj] = ei - t;

	imag[smi] = -(*fpii++);
	*fpij-- = -imag[smj];
      }
  }
}

/* Fourier transform of array x[2**log_fft_size] */
void
get_fftd(x_real, x_imag, log_fft_size)
register double   x_real[], x_imag[];
int     log_fft_size;
{
    register double  t_real, t_imag, w_imag, w_real;
    double  	    ang, twopibyn;
    int		    l, stage, two_power_stage, ij;
    register int    i, s, sby2, p, q;
    static double    *cos_tab = NULL, *sin_tab = NULL;
    static int      old_log_fft_size = 0;
    static int	    *map = NULL;
    static int	    fft_size;

	/* Initialization */

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (sin_tab != NULL) free((char *) sin_tab);
	sin_tab = (double *) malloc(fft_size / 2 * sizeof(double));
	spsassert(sin_tab,
	    "get_fft: couldn't allocate dynamic memory for array - sin_tab\n");

	if (cos_tab != NULL) free((char *) cos_tab);
	cos_tab = (double *) malloc(fft_size / 2 * sizeof(double));
	spsassert(cos_tab,
	    "get_fft: couldn't allocate dynamic memory for array - cos_tab\n");

	if (map != NULL) free((char *) map);
	map = (int *) malloc(fft_size * sizeof(int));
	spsassert(map,
	    "get_fft: couldn't allocate dynamic memory for array - map\n");

	/* Generate bit-reversal table */
	for (i = 0; i < fft_size; i++)
	    map[i] = 0;
	for (i = 0; i < fft_size; i++)
	{
	    p = i;
	    q = 0;
	    for (l = 1; l <= log_fft_size; l++)
	    {
		q = 2 * q + (p % 2);
		p = p / 2;
	    }
	    if (i < q)
		map[i] = q;
	}

	/* Compute twiddle factors */

	twopibyn = 6.283185307 / fft_size;
	for (i = 0; i < fft_size / 2; i++)
	{
	    ang = i * twopibyn;
	    cos_tab[i] = cos(ang);
	    sin_tab[i] = -sin(ang);
	}
    }

    sby2 = fft_size;
    two_power_stage = 1;
    for (stage = 1; stage <= log_fft_size; stage++)
    {
	ij = 0;
	s = sby2;
	sby2 = s / 2;
	for (l = 0; l < sby2; l++)
	{
	    w_real = cos_tab[ij];
	    w_imag = sin_tab[ij];

	    p = l;
	    q = p + sby2;
	    for (i = fft_size; i > 0; i -= s)
	    {
		t_real = x_real[p];
		t_imag = x_imag[p];
		x_real[p] = t_real + x_real[q];
		x_imag[p] = t_imag + x_imag[q];

		t_real -= x_real[q];
		t_imag -= x_imag[q];
		x_real[q] = t_real * w_real - t_imag * w_imag;
		x_imag[q] = t_real * w_imag + t_imag * w_real;

		p += s;
		q = p + sby2;
	    }
	    ij += two_power_stage;
	}
	two_power_stage *= 2;
    }
    /* perform bit-reversal operation */
    {
      register double *fip, *frp;
      register int *mp;
      for (frp = x_real, fip = x_imag, mp = map, i = fft_size; i--; frp++, fip++)
	{
	  if((q = *mp++))	{
	    t_real = *frp;
	    *frp = x_real[q];
	    x_real[q] = t_real;

	    t_imag = *fip;
	    *fip = x_imag[q];
	    x_imag[q] = t_imag;
	  }
	}
    }
}

void
get_rfftd(real, imag, log_fft_size)
double   real[], imag[];
int     log_fft_size;
{
  register double *fpri, *fprj, *fpii, *fpij, t;
  double  twopibyn;
  static double *sint=NULL, *cost=NULL;
  static int tab_size = 0;
  register int i, size;
  int log_sizeby2, size_by_2;

  size = ROUND(pow(2.0, (double) log_fft_size));
  size_by_2 = size / 2;
  log_sizeby2 = log_fft_size - 1;
  twopibyn = 2.0 * 3.141592654 / size;
  if(tab_size != size) {
    register int lim;
    if(tab_size < size) {
      if(sint && cost) {
	free(sint);
	free(cost);
	sint = cost = NULL;
      }
      if(!((sint = (double*)malloc((sizeof(double)*(size/4+1)))) &&
	   (cost = (double*)malloc((sizeof(double)*(size/4+1)))))) {
	fprintf(stderr,"Allocation problems in get_rfft()\n");
	exit(-1);
      }
    }
    for(i = 0, fpri = sint, fpii = cost, lim = size/4; i <= lim; i++) {
      *fpii++ = cos((t = twopibyn * i));
      *fpri++ = sin(t);
    }
    tab_size = size;
  }
  if (size == 1) return;
  /* Combine Even and odd sequences to get
     complex data of length = size / 2 */
  for(i= size>>1, fpri = fprj = real, fpii = imag; i--; ) {
    *fpri++ = *fprj++;
    *fpii++ = *fprj++;
  }

  get_fftd(real, imag, log_sizeby2);


  /* Separate reven and odd parts  */
  {
    register double   er, ei, or, oi, st, ct, half = 0.5;
    register int smi, smj, j, lim;

    j = size_by_2;
    t = real[0];
    real[0] += imag[0];
    real[size_by_2] = t - imag[0];
    imag[0] = imag[size_by_2] = 0.0;
    for (fpri = real+1, fprj = real + size_by_2 - 1,
	 fpii = imag+1, fpij = imag+size_by_2 - 1, i = 1,
	 lim = size/4; i <= lim; i++) {
	j--;
	ct = cost[i];
	st = sint[i];
	er = (*fpri + *fprj)*half;
	ei = (*fpii - *fpij)*half;
	oi = (*fprj - *fpri)*half;
	or = (*fpii + *fpij)*half;
	t = ct * or + st * oi;
	*fpri++ = real[(smi = size - i)] = er + t;
	*fprj-- = real[(smj = size - j)] = er - t;

	t = ct * oi - st * or;
	*fpii = ei + t;
	imag[smj] = ei - t;

	imag[smi] = -(*fpii++);
	*fpij-- = -imag[smj];
      }
  }
}

/* Inverse Fourier transform of double array x[2**log_fft_size] */
void
get_fftd_inv(x_real, x_imag, log_fft_size)
double   x_real[], x_imag[];
int     log_fft_size;
{
    register double  t_real, t_imag, w_imag, w_real;
    register        p, q;
    double  	    ang, twopibyn;
    int     	    i, l, s, sby2, stage, two_power_stage, ij;
    static double   *cos_tab = NULL, *sin_tab = NULL;
    static int	    old_log_fft_size = 0;
    static int	    *map = NULL;
    static int	    fft_size;

    /* Initialization */

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (sin_tab != NULL) free((char *) sin_tab);
	sin_tab = (double *) malloc(fft_size / 2 * sizeof(double));
	spsassert(sin_tab,
	    "get_fftd_inv: couldn't allocate dynamic memory for array - sin_tab\n");

	if (cos_tab != NULL) free((char *) cos_tab);
	cos_tab = (double *) malloc(fft_size / 2 * sizeof(double));
	spsassert(cos_tab,
	    "get_fftd<_inv: couldn't allocate dynamic memory for array - cos_tab\n");

	if (map != NULL) free((char *) map);
	map = (int *) malloc(fft_size * sizeof(int));
	spsassert(map,
	    "get_fftd_inv: couldn't allocate dynamic memory for array - map\n");

	/* Generate bit-reversal table */
	for (i = 0; i < fft_size; i++)
	    map[i] = 0;
	for (i = 0; i < fft_size; i++)
	{
	    p = i;
	    q = 0;
	    for (l = 1; l <= log_fft_size; l++)
	    {
		q = 2 * q + (p % 2);
		p = p / 2;
	    }
	    if (i < q)
		map[i] = q;
	}

	/* Compute twiddle factors */

	twopibyn = 6.283185307 / fft_size;
	for (i = 0; i < fft_size / 2; i++)
	{
	    ang = i * twopibyn;
	    cos_tab[i] = cos(ang);
	    sin_tab[i] = -sin(ang);
	}
    }

/* Complex Conjugation of input data */
    for (i = 0; i < fft_size; i++)
	x_imag[i] = -x_imag[i];

    sby2 = fft_size;
    two_power_stage = 1;
    for (stage = 1; stage <= log_fft_size; stage++)
    {
	ij = 0;
	s = sby2;
	sby2 = s / 2;
	for (l = 0; l < sby2; l++)
	{
	    w_real = cos_tab[ij];
	    w_imag = sin_tab[ij];

	    p = l;
	    q = p + sby2;
	    for (i = 0; i < fft_size; i += s)
	    {
		t_real = x_real[p];
		t_imag = x_imag[p];
		x_real[p] = t_real + x_real[q];
		x_imag[p] = t_imag + x_imag[q];

		t_real -= x_real[q];
		t_imag -= x_imag[q];
		x_real[q] = t_real * w_real - t_imag * w_imag;
		x_imag[q] = t_real * w_imag + t_imag * w_real;

		p += s;
		q = p + sby2;
	    }
	    ij += two_power_stage;
	}
	two_power_stage *= 2;
    }
    /* perform bit-reversal operation */
    for (i = 0; i < fft_size; i++)
    {
	q = map[i];
	if (q)
	{
	    t_real = x_real[i];
	    x_real[i] = x_real[q];
	    x_real[q] = t_real;

	    t_imag = x_imag[i];
	    x_imag[i] = x_imag[q];
	    x_imag[q] = t_imag;
	}
    }
/* Complex Conjugation of Output Data and scaling by factor fft_size */
    for (i = 0; i < fft_size; i++)
    {
	x_real[i] = x_real[i] / fft_size;
	x_imag[i] = -x_imag[i] / fft_size;
    }
}
