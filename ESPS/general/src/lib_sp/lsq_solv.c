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
|  lsq_solv2.c								|
|									|
|  Rodney W. Johnson.  Based on the Fortran procedure by C. L. Lawson	|
|	and R. J. Hanson cited below.					|
|									|
|  lsq_solv() computes minimum least_squares solutions to sets of	|
|	linear equations with vector right-hand sides.			|
|									|
|  lsq_solv2() computes minimum least_squares solutions to sets of	|
|	linear equations with matrix right-hand sides.			|
|									|
|  moore_pen() computes the Moore-Penrose generalized inverse of a	|
|	matrix.								|
|									|
|  Ref:  C. L. Lawson and R. J. Hanson, 'Solving Least Squares		|
|	Problems', Prentice-Hall, 1974.  Subroutines HFTI and H12	|
|	in Appendix C.							|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)lsq_solv.c	1.2	7/26/88	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <math.h>

char	*arr_alloc();
void	arr_free();
char	*marg_index();

static double	sq();


int
lsq_solv2(a, b, m, n, p, x, eps)
    double  **a, **b;
    int	    m, n, p;
    double  **x, eps;
{
    long    dim[2];
    double  **ab;
    double  *g, *h;
    long    *perm;
    double  hmax;
    double  tmp;
    int	    ldiag, jmax, rank;
    int	    i, j, r, k;

    dim[0] = m; dim[1] = n + p;
    ab = (double **) arr_alloc(2, dim, DOUBLE, 0);
    g = (double *) arr_alloc(1, dim+1, DOUBLE, 0);
    h = (double *) arr_alloc(1, dim+1, DOUBLE, 0);
    perm = (long *) arr_alloc(1, dim+1, LONG, 0);

    for (i = 0; i < m; i++)
    {
	for (j = 0; j < n; j++)
	    ab[i][j] = a[i][j];
	for (j = 0; j < p; j++)
	    ab[i][n+j] = b[i][j];
    }

    ldiag = MIN(m, n);

    for (r = 0; r < ldiag; r++)
    {
	if (r != 0)
	{
	    jmax = r;
	    for (j = r; j < n; j++)
	    {
		h[j] -= sq(ab[r-1][j]);
		if (h[j] > h[jmax]) jmax = j;
	    }
	}

	if (r == 0 || hmax + 0.001*h[jmax] > hmax)
	{
	    jmax = r;
	    for (j = r; j < n; j++)
	    {
		h[j] = 0;
		for (i = r; i < m; i++)
		    h[j] += sq(ab[i][j]);
		if (h[j] > h[jmax]) jmax = j;
	    }
	    hmax = h[jmax];
	}

	perm[r] = jmax;
	if (perm[r] != r)
	{
	    for (i = 0; i < m; i++)
	    {
		tmp = ab[i][r];
		ab[i][r] = ab[i][jmax];
		ab[i][jmax] = tmp;
	    }
	    h[jmax] = h[r];
	}

	/*
	 * Householder transformation on columns of a and b to zero the
	 * subdiagonal elements of column r of a.
	 */

	{
	    double  sum, b, scl;

	    scl = fabs(ab[r][r]);
	    for (i = r+1; i < m; i++)
		if (fabs(ab[i][r]) > scl)
		    scl = fabs(ab[i][r]);
	    if (scl > 0)
	    {
		sum = sq(ab[r][r]/scl);
		for (i = r + 1; i < m; i++)
		{
		    sum += sq(ab[i][r]/scl);
		}
		scl = sqrt(sum)*scl;
		if (ab[r][r] > 0.0) scl = -scl;

		h[r] = ab[r][r] - scl;
		ab[r][r] = scl;

		b = h[r]*ab[r][r];
		if (b != 0.0)
		    for (j = r + 1; j < n + p; j++)
		    {
			sum = ab[r][j]*h[r];
			for (i = r + 1; i < m; i++)
			    sum += ab[i][j]*ab[i][r];
			if (sum != 0.0)
			{
			    sum /= b;
			    ab[r][j] += sum*h[r];
			    for (i = r + 1; i < m; i++)
				ab[i][j] += sum*ab[i][r];
			} /* if (sum ...) */
		    } /* for (j ...) */
	    } /* if (scl ...) */
	} /* end Householder trans. on columns */
    } /* for (r ...) */

    if (ldiag == 0 || fabs(ab[0][0]) == 0.0)
    {
	rank = 0;
	for (j = 0; j < n; j++)
	    for (k = 0; k < p; k++)
		x[j][k] = 0.0;
	return rank;
    }

    for (r = 0; r < ldiag && fabs(ab[r][r]) >= eps*fabs(ab[0][0]); r++)
    { }
    rank = r;

    if (rank < n)
	for (r = rank - 1; r >= 0; r--)
	{
	    double  sum, b, scl;

	    /*
	     * Householder transformation on first (rank) rows of a to
	     * zero the elements to the right of column rank-1 in row r.
	     */
	    
	    scl = fabs(ab[r][r]);
	    for (j = rank; j < n; j++)
		if (fabs(ab[r][j]) > scl)
		    scl = fabs(ab[r][j]);
	    if (scl > 0)
	    {
		sum = sq(ab[r][r]/scl);
		for (j = rank; j < n; j++)
		    sum += sq(ab[r][j]/scl);
		scl = sqrt(sum)*scl;
		if (ab[r][r] > 0.0) scl = -scl;

		g[r] = ab[r][r] - scl;
		ab[r][r] = scl;

		b = g[r]*ab[r][r];
		if (b != 0.0)
		    for (i = 0; i < r; i++)
		    {
			sum = ab[i][r]*g[r];
			for (j = rank; j < n; j++)
			    sum += ab[i][j]*ab[r][j];
			if (sum != 0.0)
			{
			    sum /= b;
			    ab[i][r] += sum*g[r];
			    for (j = rank; j < n; j++)
				ab[i][j] += sum*ab[r][j];
			} /* if (sum ...) */
		    } /* for (i ...) */
	    } /* if (scl ...) */
	} /* for (r ...) */
     /* end Householder trans. on rows */

    for (k = 0; k < p; k++)
    {
	for (i = rank - 1; i >= 0; i--)
	{
	    double	sum = 0.0;

	    for (j = i + 1; j < rank; j++)
		sum += ab[i][j]*x[j][k];
	    x[i][k] = (ab[i][n+k] - sum)/ab[i][i];
	}

	for (j = rank; j < n; j++) x[j][k] = 0.0;

	/*
	 * Householder transformations on solution vector to compensate
	 * for transformations on rows of a.
	 */

	if (rank < n)
	    for (r = 0; r < rank; r++)
	    {
		double  sum, b;

		b = g[r]*ab[r][r];
		if (b != 0.0)
		{
		    sum = x[r][k]*g[r];
		    for (j = rank; j < n; j++)
			sum += x[j][k]*ab[r][j];
		    if (sum != 0.0)
		    {
			sum /= b;
			x[r][k] += sum*g[r];
			for (j = rank; j < n; j++)
			    x[j][k] += sum*ab[r][j];
		    } /* if (sum ...) */
		} /* if (b ...) */
	    } /* for (r ...) */
	/* end Householder transformations on solution vector. */

	/*
	 * Permute solution vector to compensate for column interchanges.
	 */

	for (j = ldiag - 1; j >= 0; j--)
	    if (perm[j] != j)
	    {
		tmp = x[perm[j]][k];
		x[perm[j]][k] = x[j][k];
		x[j][k] = tmp;
	    }
    } /* for (k ...) */

    arr_free((char *) ab, 2, DOUBLE, 0);
    arr_free((char *) g, 1, DOUBLE, 0);
    arr_free((char *) h, 1, DOUBLE, 0);
    arr_free((char *) perm, 1, DOUBLE, 0);

    return rank;
}


static double
sq(x)
    double x;
{
    return x*x;
}


int
lsq_solv(a, b, m, n, x, eps)
    double  **a, *b;
    int	    m, n;
    double  *x, eps;
{
    double  **bb, **xx;
    int	    rank;
    long    dim[2];

    dim[0] = m;	    dim[1] = 1;
    bb = (double **) marg_index((char *) b, 2, dim, DOUBLE, 0);
    dim[0] = n;
    xx = (double **) marg_index((char *) x, 2, dim, DOUBLE, 0);

    rank = lsq_solv2(a, bb, m, n, 1, xx, eps);

    arr_free((char *) bb, 1, DOUBLE, 1);
    arr_free((char *) xx, 1, DOUBLE, 1);

    return rank;
}


int
moore_pen(a, m, n, x, eps)
    double  **a;
    int	    m, n;
    double  **x, eps;
{
    double  **b;
    int	    rank;
    long    dim[2];
    int	    i, j;

    dim[0] = m;	    dim[1] = m;
    b = (double **) arr_alloc(2, dim, DOUBLE, 0);

    for (i = 0; i < m; i++)
    {
	for (j = 0; j < m; j++) b[i][j] = 0;
	b[i][i] = 1;
    }

    rank = lsq_solv2(a, b, m, n, m, x, eps);

    arr_free((char *) b, 2, DOUBLE, 0);

    return rank;
}
