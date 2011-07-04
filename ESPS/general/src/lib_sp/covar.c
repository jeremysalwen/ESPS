/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  ??
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)covar.c	1.10	10/19/93	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>

void free();

static cnt = 0;

covar (data, lnt, c, order, grc, pgain, window_flag)
float   data[], c[], grc[], *pgain;
int     lnt, order, window_flag;
/*
 Compute lpc filter coefficients using standard covariance method
Input:
data --- input data vector of size "lnt".
order --- order of the desired lpc filter.
window_flag --- A non-zero value would specify triangular windowing
		of the sample data vectors of size "order + 1".
Output:
rc --- generalized reflection coefficient array of "order"
c --- lpc filter coefficient array of size "order"
*pgain --- lpc filter gain.
*/
{
    double *C, *r, *sample_C, Const, inv_pivot, sqrt ();
    double  tmp, t;
    int     matsiz, nsdvct;	/* no. of sample data vectors */
    int     i, j, k, ns, ni, nj;
#ifndef DEC_ALPHA
    char   *malloc ();
#endif
    
    cnt++;
    matsiz = order + 1;
/* Allocate memory space */
    C = (double *) malloc (matsiz * matsiz * sizeof *C);
    if (C == NULL)
    {
Fprintf (stderr, "covar: couldn't allocate dynamic memory for array - C\n");
	exit (1);
    }
    r = (double *) malloc (matsiz * sizeof *C);
    if (r == NULL)
    {
Fprintf (stderr, "covar: couldn't allocate dynamic memory for array - r\n");
	exit (1);
    }

    sample_C = (double *) malloc (matsiz * matsiz * sizeof *sample_C);
    if (sample_C == NULL)
    {
Fprintf (stderr, 
	"covar: couldn't allocate dynamic memory for array - sample_C\n");
	exit (1);
    }

    nsdvct = lnt - order;
    if (window_flag)
    {
	/* Estimate sample covariance of first data vector */
	for (i = 0; i < matsiz; i++)
	{
	    ni = order - i;
	    for (j = i; j < matsiz; j++)
	    {
		nj = order - j;
	  C[i * matsiz + j] = sample_C[i * matsiz + j] = data[ni] * data[nj];
	    }
	}

	for (ns = matsiz; ns < lnt; ns++)
	{
	    /* update sample_covar matrix using shift-invariance property 
	    */
	    for (i = order; i > 0; i--)
		for (j = i; j < matsiz; j++)
	      sample_C[i * matsiz + j] = sample_C[(i - 1) * matsiz + j - 1];
	    t = data[ns];
	    for (i = 0; i < matsiz; i++)
		sample_C[i] = t * data[ns - i];

	    /* accumulate covar matrix with proper weight */

	    t = ns - matsiz + 2;
	    if ((2 * t - 1) > nsdvct)
		t = nsdvct - t + 1;
	    for (i = 0; i < matsiz; i++)
		for (j = i; j < matsiz; j++)
		    C[i * matsiz + j] += t * sample_C[i * matsiz + j];
	}
	t = 2.0 / nsdvct;
    }
    else
    {
	/* Estimate first row of covariance matrix */
	for (i = 0; i < matsiz; i++)
	{
	    t = 0.0;
	    k = order - i;
	    for (j = order; j < lnt; j++)
		t += data[j] * data[k++];
	    C[i] = t;
	}
	for (i = 1; i < matsiz; i++)
	{
	    ni = order - i;
	    for (j = i; j < matsiz; j++)
 C[i * matsiz + j] = C[(i - 1) * matsiz + j - 1] + data[ni] * data[order - j]
		    - data[lnt - i] * data[lnt - j];
	}
	t = 1.0 / nsdvct;
    }
    for (i = 0; i < matsiz; i++)
	for (j = i; j < matsiz; j++)
	{
	    C[i * matsiz + j] *= t;
	    C[j * matsiz + i] = C[i * matsiz + j];
	}

    /* Forward Substitution of Gauss-Seidal Elimination Method */
    for (i = 1; i <= order; i++)
    {
	inv_pivot = 1.0 / C[i * matsiz + i];
	for (j = i + 1; j < matsiz; j++)
	{
	    Const = -C[j * matsiz + i] * inv_pivot;
	    C[matsiz * j] = C[matsiz * j] + Const * C[matsiz * i];
	    for (k = i; k < matsiz; k++)
	    {
	C[j * matsiz + k] = C[j * matsiz + k] + Const * C[i * matsiz + k];
	    }
	}
    }

/* Backward Substitution */
    c[0] = 1.0;
    for (i = order; i > 0; i--)
    {
	tmp = C[matsiz * i];
	for (j = i + 1; j < matsiz; j++)
	    tmp -= C[i * matsiz + j] * c[j];
	c[i] = tmp / C[i * matsiz + i];
    }
    if (ctorc2 (c, grc, order, pgain) == 0)
    {
      (void) fprintf (stderr, 
		      "covar: Unstable Filter in call number %d.\n", cnt);
      (void)fprintf(stderr, "Replaced with modified Burg estimate\n");
      (void)get_burg (data, lnt, r, order, c, grc, pgain, 'm');
    }

    free ((char *) r);
    free ((char *) C);
    free ((char *) sample_C);
}

ctorc2 (c, rc, order, pgain)
float   rc[], c[], *pgain;
int     order;
{
    float   gain, tmp2[100], tmp1[100], t1, t2;
    int     i, j;

/* Compute reflection coefficients from "c" filter */
    tmp2[0] = 1.0;
    for (i = 1; i <= order; i++)
	tmp2[i] = -c[i];

    gain = 1.0;
    for (i = order; i > 0; i--)
    {
	t1 = tmp2[i];
	rc[i] = -t1;

	if (t1 * t1 >= 1.0)
	{
	    *pgain = 0.0;
	    return (0);
	}
	t2 = 1.0 - t1 * t1;
	gain *= t2;
	for (j = 1; j <= i - 1; j++)
	    tmp1[j] = (tmp2[j] - t1 * tmp2[i - j]) / t2;
	for (j = 1; j <= i - 1; j++)
	    tmp2[j] = tmp1[j];
    }
    rc[0] = gain;
    *pgain = gain;
    return (1);
}
