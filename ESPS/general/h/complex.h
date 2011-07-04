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
 * @(#)complex.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for programs that use complex scalar arithmetic functions.
 *
 */

#ifndef complex_H
#define complex_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * define the COMPLEX data type 
 */

typedef double_cplx COMPLEX;

/*
 * Function declarations.
 */

COMPLEX
cadd ARGS((COMPLEX x, COMPLEX y));

COMPLEX
cdiv ARGS((COMPLEX x, COMPLEX y));

COMPLEX
cmult ARGS((COMPLEX x, COMPLEX y));

COMPLEX
cmultacc ARGS((COMPLEX x, COMPLEX y, COMPLEX r));

COMPLEX
conj ARGS((COMPLEX x));

COMPLEX
csqrt ARGS((COMPLEX x));

COMPLEX
csub ARGS((COMPLEX x, COMPLEX y));

double
modul ARGS((COMPLEX x));

COMPLEX
realadd ARGS((COMPLEX x, double y));

COMPLEX
realmult ARGS((COMPLEX x, double y));


#ifdef __cplusplus
}
#endif

#endif /* complex_H */
