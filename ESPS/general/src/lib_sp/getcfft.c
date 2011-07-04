/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Written by:  David Burton
 * Modified by: Bill Byrne 
 *  				
 * Module:	getcfft.c
 *
 * This module contains various fast fourier transform functions:
 *
 * get_cfft:    computes the complex DFT of float_cplx data
 * get_cfftd:   computes the complex DFT of double_cplx data
 * get_cfft_inv: computes the complex IDFT using float_cplx data
 * get_cfftd_inv: computes the complex IDFT using double_cplx data
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#ifndef lint
static char *sccs_id = "@(#)getcfft.c	1.2 10/19/90 ESI";
#endif

double pow();

/* Fourier transform of array data[2**log_fft_size] */
void
get_cfft(data, log_fft_size)
float_cplx   *data;      /* data array - holds input and output*/
int          log_fft_size;  /* just like it says*/
{

    static float   *x_real = NULL, *x_imag = NULL;
    static int      old_log_fft_size = 0;
    static int	    fft_size;
    int             i;
/* 
 *Initialization 
*/

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (x_real != NULL) free((char *) x_real);
	x_real = (float *) malloc(fft_size * sizeof *x_real);
	spsassert(x_real,
	    "get_cfft: couldn't allocate dynamic memory for real array\n");

	if (x_imag != NULL) free((char *) x_imag);
	x_imag = (float *) malloc(fft_size  * sizeof *x_imag);
	spsassert(x_imag,
           "get_cfft: couldn't allocate dynamic memory for imaginary array\n");
      }

      for(i = 0; i < fft_size; i++){
	x_real[i] = data[i].real;
	x_imag[i] = data[i].imag;
      }
/*
 * Call the standard get_fft function
*/

	get_fft(x_real, x_imag, log_fft_size);

/*
 * Pack it back into a float_cplx data array
*/

	for(i = 0; i < fft_size; i++){
	  data[i].real = x_real[i];
	  data[i].imag = x_imag[i];
	}
  }

/* Fourier transform of double_cplx array data[2**log_fft_size] */
void
get_cfftd(data, log_fft_size)
double_cplx   *data;
int     log_fft_size;
{
  static int      old_log_fft_size = 0;
  static double  *x_real = NULL, *x_imag = NULL;
  static int      fft_size;
  int             i;
/* 
* Initialization 
*/

    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (x_real != NULL) free((char *) x_real);
	x_real = (double *) malloc(fft_size * sizeof *x_real);
	spsassert(x_real,
	   "get_cfftd: couldn't allocate dynamic memory for real array\n");

	if (x_imag != NULL) free((char *) x_imag);
	x_imag = (double *) malloc(fft_size  * sizeof *x_imag);
	spsassert(x_imag,
          "get_cfftd: couldn't allocate dynamic memory for imaginary array\n");
      }
	
      for(i = 0; i < fft_size; i++){
	x_real[i] = data[i].real;
	x_imag[i] = data[i].imag;
      }

/*
 * Call the standard get_fftd function
*/

      get_fftd(x_real, x_imag, log_fft_size);

/*
 * Pack it back into a double_cplx data array
*/

	for(i = 0; i < fft_size; i++){
	  data[i].real = x_real[i];
	  data[i].imag = x_imag[i];
	}
}

/* Inverse Fourier transform of float_cplx array data[2**log_fft_size] */
void
get_cfft_inv(data, log_fft_size)
float_cplx   *data;
int     log_fft_size;
{
  static int     old_log_fft_size = 0;
  static float  *x_real = NULL, *x_imag = NULL;
  static int     fft_size;
  int            i;
/*
* Initialization
*/
    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (x_real != NULL) free((char *) x_real);
	x_real = (float *) malloc(fft_size * sizeof *x_real);
	spsassert(x_real,
	    "get_cfft_inv: couldn't allocate dynamic memory for real array\n");

	if (x_imag != NULL) free((char *) x_imag);
	x_imag = (float *) malloc(fft_size  * sizeof *x_imag);
	spsassert(x_imag,
           "get_cfft_inv: couldn't allocate dynamic memory for imaginary array\n");
      }

      for(i = 0; i < fft_size; i++){
	x_real[i] = data[i].real;
	x_imag[i] = data[i].imag;
      }

/*
 * Call the standard get_fft_inv function
*/

	get_fft_inv(x_real, x_imag, log_fft_size);

/*
 * Pack it back into a float_cplx data array
*/

	for(i = 0; i < fft_size; i++){
	  data[i].real = x_real[i];
	  data[i].imag = x_imag[i];
	}
}


/* Inverse Fourier transform of double_cplx array data[2**log_fft_size] */
void
get_cfftd_inv(data, log_fft_size)
double_cplx   *data;
int     log_fft_size;
{
  static int     old_log_fft_size;
  static double *x_real = NULL, *x_imag = NULL;
  static int     fft_size;
  int            i;
/*
* Initialization
*/
    if (log_fft_size != old_log_fft_size)
    {
	fft_size = ROUND(pow(2.0, (double) log_fft_size));
	old_log_fft_size = log_fft_size;

	if (x_real != NULL) free((char *) x_real);
	x_real = (double *) malloc(fft_size * sizeof *x_real);
	spsassert(x_real,
	    "get_cfft_inv: couldn't allocate dynamic memory for real array\n");

	if (x_imag != NULL) free((char *) x_imag);
	x_imag = (double *) malloc(fft_size  * sizeof *x_imag);
	spsassert(x_imag,
           "get_cfft_inv: couldn't allocate dynamic memory for imaginary array\n");
      }

      for(i = 0; i < fft_size; i++){
	x_real[i] = data[i].real;
	x_imag[i] = data[i].imag;
      }
/*
 * Call the standard get_fftd_inv function
*/

	get_fftd_inv(x_real, x_imag, log_fft_size);

/*
 * Pack it back into a double_cplx data array
*/

	for(i = 0; i < fft_size; i++){
	  data[i].real = x_real[i];
	  data[i].imag = x_imag[i];
	}
}


