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
|  min_rel_ent.c							|
|									|
|  Rodney W. Johnson.  Based on an APL function from the paper		|
|	referenced below and on a Fortran adataption by Joseph T.	|
|	Buck.								|
|									|
|  min_rel_ent() computes a minimum-relatve-entropy probability		|
|	distribution q, given an initial estimate p and a constraint	|
|	matrix c.  The result is a vector of nonnegative numbers that	|
|	minimizes the relative entropy of q with respect to p,		|
|	subject to the constraints that the matrix product cp equal	|
|	zero and the sum of the elements of q be 1.  The solution is	|
|	of the form							|
|	                      m-1					|
|	    q  = alpha p  exp SUM beta  c     (j = 0, . . . , n-1)	|
|	     j          j     i=0     i  ij				|
|	where alpha and the elements of beta are Lagrange multipliers	|
|	chosen to satisfy the constraints.  The value of beta is	|
|	available as an additional output.				|
|									|
|   Ref:  R. Johnson, "Determining Probability Distributions by		|
|	Maximum Entropy and Minimum Cross-Entropy", APL Quote Quad,	|
|	vol. 9, no. 4, June 1979, pp. 24-29.  (APL 79 Conference	|
|	Proceedings).							|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)minrelent.c	1.1	9/2/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <math.h>

char	*arr_alloc();
void	arr_free();

static double	dist();
int		lsq_solv();


double
min_rel_ent(p, c, q, beta, m, n, maxit, thresh)
    double  *p, **c, *q, *beta;
    int	    m, n, maxit;
    double  thresh;
{
    double  *rp, *q0;
    double  **t;
    double  *delta_b;
    double  sum, term, d;
    long    dim[2];
    int	    i, j, iter;

    dim[0] = n;
    rp = (double *) arr_alloc(1, dim, DOUBLE, 0);
    q0 = (double *) arr_alloc(1, dim, DOUBLE, 0);
    dim[1] = m;
    t = (double **) arr_alloc(2, dim, DOUBLE, 0);
    dim[0] = m;
    delta_b = (double *) arr_alloc(1, dim, DOUBLE, 0);

    for (j = 0; j < n; j++)
	q[j] = rp[j] = sqrt(p[j]);

    for (i = 0; i < m; i++)
	beta[i] = 0;

    for (   iter = 0,  d = DBL_MAX;
	    iter < maxit && d >= thresh;
	    iter++,    d = dist(q, q0, n) )
    {
	for (j = 0; j < n; j++)
	    q0[j] = q[j];

	for (i = 0; i < m; i++)
	    for (j = 0; j < n; j++)
		t[j][i] = c[i][j]*q[j];

	(void) lsq_solv(t, q, n, m, delta_b, thresh);

	for (i = 0; i < m; i++)
	    beta[i] -= delta_b[i];

	for (j = 0; j < n; j++)
	{
	    term = 0;

	    for (i = 0; i < m; i++)
		term += beta[i]*c[i][j];

	    q[j] = rp[j] * exp(0.5*term);
	}
    }

    if (d > thresh)
	Fprintf(stderr,
	    "min_rel_ent: convergence failed after %d iterations.\n",
	    maxit);

    sum = 0.0;

    for (j = 0; j < n; j++)
	sum += q[j] *= q[j];

    for (j = 0; j < n; j++)
	q[j] /= sum;

    arr_free((char *) rp, 1, DOUBLE, 0);
    arr_free((char *) q0, 1, DOUBLE, 0);
    arr_free((char *) t, 2, DOUBLE, 0);
    arr_free((char *) delta_b, 1, DOUBLE, 0);

    return d;
}

static double
dist(p, q, n)
    double  *p, *q;
    int	    n;
{
    int	    j;
    double  a, b, c;
    double  d = 0.0;

    for (j = 0; j < n; j++)
    {
	c = p[j] - q[j];
	if (c != 0.0)
	{
	    a = fabs(p[j]);
	    b = fabs(q[j]);
	    c = (a > b) ? fabs(c)/a : fabs(c)/b;
	    if (c > d) d = c;
	}
    }

    return d;
}
