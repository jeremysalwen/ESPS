/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  rel_ent.c								|
|									|
|  Rodney W. Johnson.							|
|									|
|  rel_ent() returns the relative entropy				|
|	    n-1								|
|	    SUM q  log q /p						|
|	    j=0  j      j  j						|
|	of two probability distributions q and p.			|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)relent.c	1.1	8/31/89	ESI";
#endif

#include <math.h>

double
rel_ent(q, p, n)
    double  *q, *p;
    int	    n;
{
    double  sum;
    int	    i;

    sum = 0.0;
    for (i = 0; i < n; i++)
	sum += (q[i] == 0.0) ? 0.0 : q[i]*log(q[i]/p[i]);
    return sum;
}
