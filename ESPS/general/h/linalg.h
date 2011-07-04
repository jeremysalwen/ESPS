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
 * @(#)linalg.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS functions related to linear algebra.
 *
 */

#ifndef linalg_H
#define linalg_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

double
determ ARGS((float **mat, int size));

double
determ_d ARGS((double **mat, int size));

double
matrix_inv ARGS((float **mat_in, float **inv_out, int size));

double
matrix_inv_d ARGS((double **mat_in, double **inv_out, int size));

int
lsq_solv ARGS((double **a, double *b, int m, int n, double *x, double eps));

int
lsq_solv2 ARGS((double **a, double **b,
		int m, int n, int p, double **x, double eps));

int
moore_pen ARGS((double **a, int m, int n, double **x, double eps));

int
stsolv ARGS((double *r, double *v, int order, double *w));


#ifdef __cplusplus
}
#endif

#endif /* linalg_H */
