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
|  Program: image.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imagescale.c	1.2	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

double	    floor(), ceil(), fabs(), log10(), pow();

extern int  debug_level;

void
pscale(low, high, minstep, start, step, n)
    double  low, high, minstep;
    double  *start, *step;
    int	    *n;
{
    static double   carr[] = { 0.5, 1.0, 2.5, 2.0, 5.0, 5.0 };
    static double   darr[] = { 1.0, 2.0, 2.5, 4.0, 5.0, 10.0 };

    double  t, e;
    int	    i;

    if (debug_level >= 3)
	Fprintf(stderr, "pscale: low %g, high %g, %minstep %g\n",
		low, high, minstep);

    t = pow(10.0, floor(log10(minstep)));
    if (10.0*t <= minstep) t = 10.0*t;

    e = DBL_MAX;

    for (i = 0; i < sizeof(darr)/sizeof(double); i++)
    {
	double	c, d, ei;

	d = darr[i]*t;	c = carr[i]*t;
	if (minstep <= d && d <= minstep*2.0)
	{
	    double  ej, s, sj;
	    double	p, q;

	    ei = DBL_MAX;

	    for (   sj = c * ceil((low + minstep)/c),
			sj = (sj - c >= low + minstep) ? sj = sj - c : sj;
		    p = fabs(sj - (low + d)),
			q = fabs( (high - d)
			    - (sj + d * floor((high - minstep - sj)/d)) ),
			ej = MAX(p, q),
			(ej < ei);
		    sj += c )
	    {
		ei = ej;
		s = sj;
	    }

	    if (ei < e)
	    {
		e = ei;
		*start = s;
		*step = d;
	    }
	}
    }

    *n = 1 + floor((high - minstep - *start)/(*step));
    if (*n < 0) *n = 0;

    if (debug_level >= 3)
	Fprintf(stderr, "pscale: start %g, step %g, n %d\n",
		*start, *step, *n);
}
