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
 * @(#)array.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for functions that deal with contiguously allocated storage
 *	indexed as a multidimensional array.
 *
 */

#ifndef array_H
#define array_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

char *
arr_alloc ARGS((int rk, long int *dim, int typ, int lvl));

void
arr_free ARGS((char *arr, int rk, int typ, int lvl));

char *
marg_index ARGS((char *arr, int rk, long int *dim, int typ, int lvl));


#ifdef __cplusplus
}
#endif

#endif /* array_H */
