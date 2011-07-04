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
 * @(#)fftcepstrum.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS functions for cepstrum computation via FFTs.
 *
 */

#ifndef fftcepstrum_H
#define fftcepstrum_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

void
fft_ccepstrum ARGS((float_cplx *data, long int order));

void
fft_ccepstrum_r ARGS((float *data_r, float *data_i, long int order));

void
fft_cepstrum ARGS((float_cplx *data, long int order));

void
fft_cepstrum_r ARGS((float *data_r, float *data_i, long int order));


#ifdef __cplusplus
}
#endif

#endif /* fftcepstrum_H */
