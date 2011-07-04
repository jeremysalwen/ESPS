/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
*/
#ifndef lint
	static char *sccs_id = "@(#)getburg.c	1.8	2/20/96 ESI";
#endif

#define modified_burg 'm'
#include <stdio.h>
#include <esps/esps.h>

void
get_burg (x, lnt, r, order, c, rc, pgain, method)
float   x[], c[], rc[], *pgain;
double  r[];
int     order, lnt;
char    method;
{
/*
Compute inv filter coefficients using Burg / modified Burg method

input:
	x - input data array of size "lnt"

parameters:
	order - No. of reflection coefficients
	method - char to specify burg ('b') or modified burg method ('m')

output:
	rc - "order" reflection coefficients
	r[0] - signal power
	c - lpc filter coefficients
	*pgain - filter gain
*/

    double  num, den;
    double   btmp, *ptr1;
    double  *f, *b;
    int     n, stage, beg;
#ifndef DEC_ALPHA
    char   *malloc ();
#endif

#define rcstage num

/* Allocate memory for f and b arrays */
    f = (double *) malloc (lnt * sizeof *f);
    spsassert(f != NULL, "getburg: malloc failed");

    b = (double *) malloc (lnt * sizeof *b);
    spsassert(b != NULL, "getburg: malloc failed");

    beg = 0;
    if (method == modified_burg)
	beg = order;

    for (n = 0; n < beg; n++)
	f[n] = b[n] = x[n];
    for (den = 0.0, n = beg; n < lnt; n++)
    {
	f[n] = b[n] = btmp = x[n];
	den += btmp * btmp;
    }
    r[0] = den;

    if (method == modified_burg)
	den = 2 * den - btmp * btmp + x[beg - 1] * x[beg - 1];
    else
	den = 2 * den - btmp * btmp - x[0] * x[0];
    for (stage = 1; stage <= order; stage++)
    {
	num = 0.0;
	beg = stage;
	if (method == modified_burg)
	    beg = order;

	for (ptr1 = &b[beg - stage], n = beg; n < lnt; n++)
	    num += f[n] * *ptr1++;

	rc[stage] = rcstage = 2.0 * num / den;
	for (ptr1 = b, n = stage; n < lnt; n++)
	{
	    btmp = *ptr1;
	    *ptr1++ = btmp - rcstage * f[n];
	    f[n] -= rcstage * btmp;
	}

	den = den * (1.0 - rcstage * rcstage)
	    - b[lnt - stage - 1] * b[lnt - stage - 1];
	/*skip on last pass - j shore; avoids floating exception on 68010*/
	if (stage < order) {
	    if (method == modified_burg)
		den += b[beg - stage - 1] * b[beg - stage - 1];
	    else
		den -= f[stage] * f[stage];
	}
    }
    rctoc (rc, order, c, pgain);

    free ((char *) f);
    free ((char *) b);
}
