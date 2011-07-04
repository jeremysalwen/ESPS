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
 * Brief description: compute inv filter coefficients using 
 * modified Burg method. A covariance type method is used here. 
 * Hence significantly faster than getburg routine.
 */

/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
*/ 

static char *sccs_id = "@(#)getfburg.c	1.2	10/19/93	ESI/ERL";

void exit(), free();
#include <stdio.h>
#include <esps/esps.h>
void get_fburg (x, lnt, a, order, rc, pgain)
float   x[], a[], rc[], *pgain;
int     lnt, order;
{
/*
Compute inv filter coefficients using modified Burg method.
A covariance type method is used here. Hence significantly 
faster than getburg routine.

input:
	x - input data array of size "lnt"

parameters:
	order - No. of reflection coefficients

output:
	rc - "order" reflection coefficients
	a - lpc filter coefficients
	*pgain - filter gain
*/
    double  temp, atSb, btSb, atSa, *S;
    float  *b;
    int     i, j, k, matsiz = order + 1;
#define rcstage temp
#define Sb temp
#ifndef DEC_ALPHA
    char   *malloc ();
#endif

/* Allocate memory space */
    S = malloc_d (matsiz * matsiz * sizeof *S);
    if (S == NULL)
    {
	Fprintf (stderr, "getfburg: couldn't allocate dynamic memory for array - S\n");
	exit (1);
    }

    b = malloc_f (matsiz * sizeof *b);
    if (b == NULL)
    {
	Fprintf (stderr, "getfburg: couldn't allocate dynamic memory for array - b\n");
	exit (1);
    }

    /* Estimate first row of covariance matrix */

    for (i = 0; i < matsiz; i++)
    {
	for (k = order - i, temp = 0.0, j = order; j < lnt; j++)
	    temp += x[j] * x[k++];
	S[i] = temp;
    }
    /* Estimate Other rows of covariance matrix */

    for (i = 1; i < matsiz; i++)
	for (j = i; j < matsiz; j++)
	    S[j * matsiz + i] = S[i * matsiz + j] = S[(i - 1) * matsiz + j - 1]
		+ x[order - i] * x[order - j]
		- x[lnt - i] * x[lnt - j];

    /* Pitch Synchronous Burg Method in Block form */

    for (i = 0; i <= order; i++)
	rc[i] = a[i] = b[i] = 0.0;
    a[0] = b[1] = 1.0;
    atSa = S[0];
    rc[0] = atSa / (lnt - order);
    for (i = 1; i <= order; i++)
    {
	/* Compute btSb and atSb */

	for (atSb = btSb = 0.0, j = 0; j <= i; j++)
	{
	    for (Sb = S[j * matsiz + i], k = 1; k < i; k++)
		Sb += S[j * matsiz + k] * b[k];

	    atSb += a[j] * Sb;
	    btSb += b[j] * Sb;
	}
	rc[i] = rcstage = 2.0 * atSb / (atSa + btSb);
	if (rcstage * rcstage >= 1.0)
	{
	    Fprintf (stderr, "getfburg: Instability noticed in the computation of rc[%d]\n", i);
	    for (i = 1; i < order + 1; i++)
		a[i] = -a[i];
	    return;
	}
	/* Filter Update */

	for (j = i; j > -1; j--)
	{
	    b[j + 1] = b[j] - rcstage * a[j];
	    a[j] = a[j] - rcstage * b[j];
	}
	/* Update atSa */

	atSa = atSa * (1 - rcstage * rcstage);
	*pgain = atSa / S[0];
    }
    for (i = 1; i < order + 1; i++)
	a[i] = -a[i];

    free ((char *) S);
    free ((char *) b);
}
