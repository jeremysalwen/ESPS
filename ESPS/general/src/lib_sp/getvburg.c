/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Shankar Narayan (I think - js) 
 * Checked by:
 * Revised by:
 *
 * Brief description:	Computation of reflection coefficients and 
 * inverse Filter coefficients using VBURG Method
 *
 */

/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

*/ 

static char *sccs_id = "@(#)getvburg.c	1.3	10/19/93	ESI/ERL";

void exit(), free();

static void rc_to_c ();

#include <stdio.h>
#include <esps/esps.h>

void get_vburg (x, lnt, r, order, c, rc, pgain, matsiz)
float   x[], c[], rc[], *pgain;
double  r[];
int     order, lnt, matsiz;
{
/*
input:
	x - input data array of size "lnt"

parameters:
	order - No. of reflection coefficients
	matsiz - sample covariance matrix size

output:
	rc - "order" reflection coefficients
	r[0] - signal power
	c - lpc filter coefficients
	*pgain - filter gain
	
*/
    register double num, den, dtemp;
    register float  stemp;
    double  f0Tf0, b0Tb0, f0Tb0;
    float  *ptr1, *f, *b;
    int     i, n, stage, vctsiz = lnt - matsiz + 1;
#ifndef DEC_ALPHA
    char   *malloc ();
#endif
#define rcstage	num

/* Allocate memory for f and b arrays */
    f = malloc_f (lnt * sizeof *f);
    if (f == NULL)
    {
	Fprintf (stderr, "getvburg: couldn't allocate dynamic memory for array - f\n");
	exit (1);
    }

    b = malloc_f (lnt * sizeof *b);
    if (b == NULL)
    {
	Fprintf (stderr, "getvburg: couldn't allocate dynamic memory for array - b\n");
	exit (1);
    }

    for (n = 0; n < lnt; n++)
	f[n] = b[n] = x[n];

    /* Compute signal energy */

    for (dtemp = 0.0, n = 0; n < vctsiz; n++)
	dtemp += x[n] * x[n];

    b0Tb0 = den = dtemp;
    f0Tf0 = den + x[vctsiz] * x[vctsiz] - x[0] * x[0];

    for (ptr1 = &x[vctsiz], n = 0; n < matsiz - 1; n++, ptr1++)
    {
	dtemp += *ptr1 * *ptr1 - x[n] * x[n];
	den += dtemp;
    }
    r[0] = den / matsiz;
    den = 2 * den - b0Tb0 - dtemp;

    for (stage = 1; stage <= order; stage++)
    {

	/* Compute first inner product */

	for (ptr1 = &f[stage], dtemp = 0.0, n = 0; n < vctsiz; n++)
	    dtemp += *ptr1++ * b[n];

	/*  Compute recursively the inner product of other vectors */

	f0Tb0 = num = dtemp;
	for (n = stage; n < matsiz - 1; n++)
	{
	    i = vctsiz + n;
	    dtemp += f[i] * b[i - stage] - f[n] * b[n - stage];
	    num += dtemp;
	}

	/*  Compute reflection coefficient and the residuals */

	rc[stage] = rcstage = 2.0 * num / den;
	for (ptr1 = b, n = stage; n < lnt; n++)
	{
	    stemp = *ptr1;
	    *ptr1++ = stemp - rcstage * f[n];
	    f[n] -= rcstage * stemp;
	}

	/* Update b0Tb0 and compute of bnTbn */

	dtemp = b0Tb0 + rcstage * rcstage * f0Tf0 - 2.0 * rcstage * f0Tb0;
	f0Tf0 = f0Tf0 + rcstage * rcstage * b0Tb0 - 2.0 * rcstage * f0Tb0;
	i = matsiz - stage - 1;
	for (b0Tb0 = dtemp, n = 0; n < i; n++)
	{
	    stemp = b[n + vctsiz];
	    dtemp += stemp * stemp - b[n] * b[n];
	}

	/* Update den */

	den = den * (1.0 - rcstage * rcstage) - f0Tf0 - dtemp;

	/* Update f0Tf0 for next stage */

	stemp = f[vctsiz + stage];
	f0Tf0 += stemp * stemp - f[stage] * f[stage];
    }
    rc_to_c (rc, order, c, pgain);
    free ((char *) f);
    free ((char *) b);
}

 /* convert reflection coefficients to filter coefficients */
 /* copied from local.epi/src/lib/spectran.c */

static void
rc_to_c (rc, order, c, pgain)
float   rc[], c[], *pgain;
int     order;
{
    float   tmp, ctmp[1000], gain = 1.0;
    int     i, j;

    c[0] = -1.0;
    for (i = 1; i <= order; i++)
    {
	tmp = rc[i];
	gain = gain * (1.0 - tmp * tmp);

	if (i != 1)
	{
	    for (j = 1; j < i; j++)
		ctmp[j] = c[j] - tmp * c[i - j];
	}
	ctmp[i] = tmp;
	for (j = 1; j <= i; j++)
	    c[j] = ctmp[j];
    }
    *pgain = gain;
}

