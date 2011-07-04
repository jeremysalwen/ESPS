/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)func.h	1.5 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS programs that use arr_func(3-ESPS).
 *
 */

#ifndef func_H
#define func_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Codes for functions of real or complex scalars.
 * Keep consistent with strings in array function_types (see below).
 */

#define FN_NONE  0
#define FN_ABS   1
#define FN_ARG   2
#define FN_ATAN  3
#define FN_CONJ  4
#define FN_COS   5
#define FN_EXP   6
#define FN_IM    7
#define FN_LOG   8
#define FN_LOG10 9
#define FN_RE    10
#define FN_RECIP 11
#define FN_SGN   12
#define FN_SIN   13
#define FN_SQR   14
#define FN_SQRT  15
#define FN_TAN   16
#define FN_EXP10 17

/*
 * Strings corresponding to symbol names without the "FN_" prefix.
 * Defined in arr_func.c.
 */

extern char *function_types[];

/*
 * Definition shared by functions arr_func and arr_op:
 * value returned for LOG (and LOG10) for zero argument.
 */

#define FN_LOG_ZERO	(-DBL_MAX)

/*
 * Function declaration.
 */

char *
arr_func ARGS((int func, long int num,
	       char *src, int src_type, char *dest, int dest_type));

#ifdef __cplusplus
}
#endif

#endif /* func_H */
