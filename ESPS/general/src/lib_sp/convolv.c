/* ---------------------------------------------------------------------------
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	

			CONVOLV and AUTO_CONVOLV

subroutines to perform convolution on (i.e. product of) 2 polynomials.
	i.e. they perform the polynomial operation: C[Z] = A[Z] * B[Z]

Convolv: is in the case of 2 regular polymial
Auto_convolv is in the case of auto-correlations, or, more precisely, for
	2 symmetric polynomials with negative and positive powers. (only the
	positive powers are stored in the array).
	A[0] A[1] ... A[N] represents:
          A[N]*Z**(-N) + ... + A[1]*Z**(-1) + A[0] + A[1]*Z + ... + A[N]*Z**N

Parameters:
a and b	are the input polynomials
a_siz and b_siz are their respective sizes (i.e. 1 + order of the polynomial)
c	is the resulting polynomial
c_siz	is a pointer to the size of c. *c_siz = a_siz + b_siz - 1

Written by: Bernard G. Fraenkel
--------------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>


#ifndef lint
	static char *sccsid = "@(#)convolv.c	1.3	1/10/90";
#endif




/*    -------------------------------------------------------------   */

convolv (a, a_siz, b, b_siz, c, c_siz)
double *a, *b, *c;
int     a_siz, b_siz;
int    *c_siz;

/*
Convolution of 2 arrays  a and b of finite dimensions a_siz and b_siz
the result is array c of dimension c_siz=a_siz+b_siz-1
*/

{
    register int    k, l;
    double  *dptr, *dptra, *dptrb;


    *c_siz = a_siz + b_siz - 1;

/* multiply B[Z] by A[0] */
    for (k = 0, dptr = c, dptrb = b; k < b_siz; k++)
	*dptr++ = *a * *dptrb++;
    for (k = b_siz, dptr = c + b_siz; k < *c_siz; k++)
    	*dptr++ = 0.0;

/* add to C[Z] the product of B[Z] by A[l]*Z**l */
    for (l = 1, dptr = c, dptra = a + 1; l < a_siz; l++, dptra++) {
	dptr = c + l;
	for (k = 0, dptrb = b;  k < b_siz; k++)
	    *dptr++ += *dptra * *dptrb++;
    }

    return (0);
}

/*    -------------------------------------------------------------   */

auto_convolv (a, a_siz, b, b_siz, c, c_siz)
double *a, *b, *c;
int     a_siz, b_siz;
int    *c_siz;

/*
Convolution of 2 auto-correlation functions  a and b 
of finite dimensions a_siz and b_siz
the result is auto-correlation c of dimension c_siz=a_siz+b_siz-1
*/

{
    register int    k, l;
    double  *dptr, *dptra, *dptrb;
    int     min;

    *c_siz = a_siz + b_siz - 1;

    min = (a_siz <= b_siz ? a_siz : b_siz);
/* positive powers of A[Z] and B[Z], including A[0] and B[0] */
    (void)convolv (a, a_siz, b, b_siz, c, c_siz);
/* positive powers of B[Z], negative powers of A[Z] */
    for (l = 1, dptr = c, dptra = a + 1; l < min ; l++, dptra++) {
	dptr = c;
	for (k = l, dptrb = b + l;  k < b_siz; k++)
	    *dptr++ += *dptra * *dptrb++;
    }
/* positive powers of A[Z], negative powers of B[Z] */
    for (l = 1, dptr = c, dptrb = b + 1; l < min; l++, dptrb++) {
	dptr = c;
	for (k = l, dptra = a + l;  k < a_siz; k++)
	    *dptr++ += *dptrb * *dptra++;
    }

    return (0);
}

/*    -------------------------------------------------------------   */

