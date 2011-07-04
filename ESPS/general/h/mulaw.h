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
 * @(#)mulaw.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS functions that do mu-law compression and expansion.
 *
 */

#ifndef mulaw_H
#define mulaw_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

int
linear_to_mu ARGS((short int *input, char *output, long int size));

int
mu_to_linear ARGS((char *in, short int *out, long int size));


#ifdef __cplusplus
}
#endif

#endif /* mulaw_H */
