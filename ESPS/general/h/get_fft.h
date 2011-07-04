/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)get_fft.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS programs that use various fft functions.
 *
 */

#ifndef get_fft_H
#define get_fft_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

void
get_fft ARGS((register float *x_real,
	      register float *x_imag, int log_fft_size));

void
get_fft_inv ARGS((float *x_real, float *x_imag, int log_fft_size));

void
get_fftd ARGS((register double *x_real,
	       register double *x_imag, int log_fft_size));

void
get_fftd_inv ARGS((double *x_real, double *x_imag, int log_fft_size));

void
get_cfft ARGS((float_cplx *data, int log_fft_size));

void
get_cfft_inv ARGS((float_cplx *data, int log_fft_size));

void
get_cfftd ARGS((double_cplx *data, int log_fft_size));

void
get_cfftd_inv ARGS((double_cplx *data, int log_fft_size));

void
get_rfft ARGS((float *real, float *imag, int log_fft_size));


#ifdef __cplusplus
}
#endif

#endif /* get_fft_H */
