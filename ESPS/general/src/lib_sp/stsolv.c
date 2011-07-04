/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988 Entropic Speech, Inc.  All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: stsolv.c							|
|									|
|  This function solves a real symmetric Toeplitz system of linear	|
|  equations by Levinson's method.					|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)stsolv.c	1.1	6/20/88	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

/* 
 * stsolv
 *
 * This function solves the system of equations
 * 
 *	Rw = v
 * 
 * where  R  is a Toeplitz matrix with coefficients given by
 * 
 *	R   = r
 *	 ij    |i-j|
 * The input vectors  r  and  v  and the output vector  w  are of
 * length  order + 1  with indices running from 0 to  order .  In
 * the intended applications,  R  is an autocorrelation matrix; it
 * should be positive definite.  Levinson's method is used--
 * reflection coefficients are computed as intermediate results,
 * and the computation halts if a reflecton coefficient with
 * magnitude 1 or greater is computed.  In that case  w  will be
 * the solution of a system of smaller order, indicated by the
 * return value of the function.  The return value upon normal
 * completion is equal to  order , and -1 indicates that  r[0]
 * equals 0.
 */

int
stsolv(r, v, order, w)
    double  *r, *v;
    int	    order;
    double  *w;
{
    double  e, k, g;
    static double
	    *a;
    static int
	    asize = 0;
    int	    m, i;

#ifdef UE
    spsassert(r, "stsolv: NULL input argument - r");
    spsassert(v, "stsolv: NULL input argument - v");
    spsassert(w, "stsolv: NULL output argument - w");
#endif

    if (asize == 0)
    {
	asize = order + 1;
	a = malloc_d((unsigned) asize);
	spsassert(a, "stsolv: can't allocate space for temporary array");
    }
    else if (asize < order + 1)
    {
	asize = order + 1;
	a = (double *) realloc((char *) a, (unsigned) asize*sizeof(double));
	spsassert(a, "stsolv: can't reallocate space for temporary array");
    }

    if (r[0] <= 0.0) return -1;
    e = r[0];
    a[0] = -1;
    w[0] = v[0]/e;

    for (m = 1; m <= order; m++)
    {
	k = 0;
	g = v[m];
	for (i = 0; i < m; i++)
	{
	    k -= a[i]*r[m-i];
	    g -= w[i]*r[m-i];
	}
	k /= e;
	if (k >= 1.0 || k <= -1.0) return m-1;
	e *= 1 - k*k;
	g /= e;
	a[m] = k;
	for (i = 1; i <= m/2; i++)
	{
	    double  t = a[i] - k*a[m-i];
	    a[m-i] -= k*a[i];
	    a[i] = t;
	}

	w[m] = g;
	for (i = 0; i < m; i++)
	    w[i] -= g*a[m-i];
    }

    return order;
}
