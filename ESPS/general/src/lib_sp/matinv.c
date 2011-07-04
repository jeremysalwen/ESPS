/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  matinv.c								|
|									|
|  Rodney Johnson.							|
|	matrix_inv() based on C and Fortran code by Ajaipal Virdy.	|
|	decomp() and solve() based on Fortran routines in:		|
|	    Forsythe, G. E., M. A. Malcolm, C. B. Moler (1977),		|
|	    "Computer Methods for Mathematical Computations",		|
|	    New Jersey: Prentice-Hall, Inc., 51-55.			|
|  Derek Lin.                                                           |
|       added matrix_d()                                                |
|									|
|  matrix_inv() computes the inverse of a matrix of floats.		|
|  determ() computes the determinant of a matrix of floats.		|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)matinv.c	1.9	5/27/93	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define LARGE_COND_F 1000000.0
#define LARGE_COND_D 100000000000000.0

char		*arr_alloc();
void		arr_free();

static void	decomp(), decomp_d(), solve(), solve_d();
double		matrix_inv(), matrix_inv_d();
double		determ(), determ_d();

double
matrix_inv (mat_in, inv_out, size)
    float   **mat_in;
    float   **inv_out;
    int	    size;
{
    long    dim[2];
    float   **a;
    long    *pivot;
    float   *work;
    double  cond;
    int	    i, j;

    dim[0] = dim[1] = size;
    a = (float **) arr_alloc(2, dim, FLOAT, 0);
    pivot = (long *) arr_alloc(1, dim, LONG, 0);
    work = (float *) arr_alloc(1, dim, FLOAT, 0);

    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) a[i][j] = mat_in[i][j];

    decomp(size, a, &cond, pivot, work);

    if (cond >= LARGE_COND_F )
    {
	cond = -1.0;
    }
    else
	for (j = 0; j < size; j++)
	{
	    for (i = 0; i < size; i++) work[i] = 0.0;
	    work[j] = 1.0;

	    solve(size, a, work, pivot);

	    for (i = 0; i < size; i++) inv_out[i][j] = work[i];
	}

    arr_free((char *) a, 2, FLOAT, 0);
    arr_free((char *) pivot, 1, LONG, 0);
    arr_free((char *) work, 1, FLOAT, 0);

    return (cond);
}


static void
decomp(n, a, cond, pivot, work)
    int	    n;
    float   **a;
    double  *cond;
    long    *pivot;
    float   *work;
{
    double  ek, t, anorm, ynorm, znorm;
    int	    i, j, k, m;

    pivot[n-1] = 1;

    anorm = 0.0;
    for (j = 0; j < n; j++)
    {
	t = 0.0;
	for (i = 0; i < n; i++) t += ABS(a[i][j]);
	if (t > anorm) anorm = t;
    }

    for (k = 0; k < n-1; k++)
    {
	m = k;
	for (i = k+1; i < n; i++)
	    if (ABS(a[i][k]) > ABS(a[m][k])) m = i;

	pivot[k] = m;
	if (m != k) pivot[n-1] = -pivot[n-1];

	t = a[m][k];
	a[m][k] = a[k][k];
	a[k][k] = t;

	if (t != 0.0)
	{
	    for (i = k+1; i < n; i++) a[i][k] = -a[i][k]/t;

	    for (j = k+1; j < n; j++)
	    {
		t = a[m][j];
		a[m][j] = a[k][j];
		a[k][j] = t;

		if (t != 0.0)
		    for (i = k+1; i < n; i++) a[i][j] += a[i][k]*t;
	    }
	}
    }

    for (k = 0; k < n; k++)
    {
	t = 0.0;
	for (i = 0; i < k; i++) t += a[i][k]*work[i];
	ek = (t < 0.0) ? -1.0 : 1.0;

	if (a[k][k] == 0.0)
	{
	    *cond = FLT_MAX;
	    return;
	}

	work[k] = - (ek + t)/a[k][k];
    }

    for (k = n-2; k >= 0; k--)
    {
	t = 0.0;
	for (i = k+1; i < n; i++) t += a[i][k]*work[k];

	work[k] = t;
	m = pivot[k];
	if (m != k)
	{
	    t = work[m];
	    work[m] = work[k];
	    work[k] = t;
	}
    }

    ynorm = 0.0;
    for (i = 0; i < n; i++) ynorm += ABS(work[i]);

    solve(n, a, work, pivot);

    znorm = 0.0;
    for (i = 0; i < n; i++) znorm += ABS(work[i]);

    *cond = anorm*znorm/ynorm;
    if (*cond < 1.0) *cond = 1.0;
}


static void
solve(n, a, b, pivot)
    int	    n;
    float   **a, *b;
    long    *pivot;
{
    int	    i, k, m;
    double  t;

    for (k = 0; k < n-1; k++)
    {
	m = pivot[k];

	t = b[m];
	b[m] = b[k];
	b[k] = t;

	for (i = k+1; i < n; i++) b[i] += a[i][k]*t;
    }

    for (k = n-1; k >= 0; k--)
    {
	b[k] /= a[k][k];
	t = -b[k];
	for (i = 0; i < k; i++) b[i] += a[i][k]*t;
    }
}


double
determ(mat, size)
    float   **mat;
    int	    size;
{
    long    dim[2];
    float   **a;
    long    *pivot;
    float   *work;
    double  cond;
    int	    i, j;
    double  det;

#ifdef UE
    spsassert(mat != NULL, "determ: mat is NULL");
#endif

    dim[0] = dim[1] = size;
    a = (float **) arr_alloc(2, dim, FLOAT, 0);
    pivot = (long *) arr_alloc(1, dim, LONG, 0);
    work = (float *) arr_alloc(1, dim, FLOAT, 0);

    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) a[i][j] = mat[i][j];

    decomp(size, a, &cond, pivot, work);

    if (cond >= LARGE_COND_F)
    {
	det = 0.0;
    }
    else
    {
	det = pivot[size-1];
	for (i = 0; i < size; i++)
	    det *= a[i][i];
    }

    arr_free((char *) a, 2, FLOAT, 0);
    arr_free((char *) pivot, 1, LONG, 0);
    arr_free((char *) work, 1, FLOAT, 0);

    return det;
}


double
matrix_inv_d (mat_in, inv_out, size)
    double   **mat_in;
    double   **inv_out;
    int	    size;
{
    long    dim[2];
    double   **a;
    long    *pivot;
    double   *work;
    double  cond;
    int	    i, j;

    dim[0] = dim[1] = size;
    a = (double **) arr_alloc(2, dim, DOUBLE, 0);
    pivot = (long *) arr_alloc(1, dim, LONG, 0);
    work = (double *) arr_alloc(1, dim, DOUBLE, 0);

    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) a[i][j] = mat_in[i][j];

    decomp_d(size, a, &cond, pivot, work);

    if (cond >= LARGE_COND_D )
    {
	cond = -1.0;
    }
    else
	for (j = 0; j < size; j++)
	{
	    for (i = 0; i < size; i++) work[i] = 0.0;
	    work[j] = 1.0;

	    solve_d(size, a, work, pivot);

	    for (i = 0; i < size; i++) inv_out[i][j] = work[i];
	}

    arr_free((char *) a, 2, DOUBLE, 0);
    arr_free((char *) pivot, 1, LONG, 0);
    arr_free((char *) work, 1, DOUBLE, 0);

    return (cond);
}


static void
decomp_d(n, a, cond, pivot, work)
    int	    n;
    double   **a;
    double  *cond;
    long    *pivot;
    double   *work;
{
    double  ek, t, anorm, ynorm, znorm;
    int	    i, j, k, m;

    pivot[n-1] = 1;

    anorm = 0.0;
    for (j = 0; j < n; j++)
    {
	t = 0.0;
	for (i = 0; i < n; i++) t += ABS(a[i][j]);
	if (t > anorm) anorm = t;
    }

    for (k = 0; k < n-1; k++)
    {
	m = k;
	for (i = k+1; i < n; i++)
	    if (ABS(a[i][k]) > ABS(a[m][k])) m = i;

	pivot[k] = m;
	if (m != k) pivot[n-1] = -pivot[n-1];

	t = a[m][k];
	a[m][k] = a[k][k];
	a[k][k] = t;

	if (t != 0.0)
	{
	    for (i = k+1; i < n; i++) a[i][k] = -a[i][k]/t;

	    for (j = k+1; j < n; j++)
	    {
		t = a[m][j];
		a[m][j] = a[k][j];
		a[k][j] = t;

		if (t != 0.0)
		    for (i = k+1; i < n; i++) a[i][j] += a[i][k]*t;
	    }
	}
    }

    for (k = 0; k < n; k++)
    {
	t = 0.0;
	for (i = 0; i < k; i++) t += a[i][k]*work[i];
	ek = (t < 0.0) ? -1.0 : 1.0;

	if (a[k][k] == 0.0)
	{
	    *cond = FLT_MAX;
	    return;
	}

	work[k] = - (ek + t)/a[k][k];
    }

    for (k = n-2; k >= 0; k--)
    {
	t = 0.0;
	for (i = k+1; i < n; i++) t += a[i][k]*work[k];

	work[k] = t;
	m = pivot[k];
	if (m != k)
	{
	    t = work[m];
	    work[m] = work[k];
	    work[k] = t;
	}
    }

    ynorm = 0.0;
    for (i = 0; i < n; i++) ynorm += ABS(work[i]);

    solve_d(n, a, work, pivot);

    znorm = 0.0;
    for (i = 0; i < n; i++) znorm += ABS(work[i]);

    *cond = anorm*znorm/ynorm;
    if (*cond < 1.0) *cond = 1.0;
}


static void
solve_d(n, a, b, pivot)
    int	    n;
    double   **a, *b;
    long    *pivot;
{
    int	    i, k, m;
    double  t;

    for (k = 0; k < n-1; k++)
    {
	m = pivot[k];

	t = b[m];
	b[m] = b[k];
	b[k] = t;

	for (i = k+1; i < n; i++) b[i] += a[i][k]*t;
    }

    for (k = n-1; k >= 0; k--)
    {
	b[k] /= a[k][k];
	t = -b[k];
	for (i = 0; i < k; i++) b[i] += a[i][k]*t;
    }
}


double
determ_d(mat, size)
    double   **mat;
    int	    size;
{
    long    dim[2];
    double   **a;
    long    *pivot;
    double   *work;
    double  cond;
    int	    i, j;
    double  det;

#ifdef UE
    spsassert(mat != NULL, "determ_d: mat is NULL");
#endif

    dim[0] = dim[1] = size;
    a = (double **) arr_alloc(2, dim, DOUBLE, 0);
    pivot = (long *) arr_alloc(1, dim, LONG, 0);
    work = (double *) arr_alloc(1, dim, DOUBLE, 0);

    for (i = 0; i < size; i++)
	for (j = 0; j < size; j++) a[i][j] = mat[i][j];

    decomp_d(size, a, &cond, pivot, work);

    if (cond >= LARGE_COND_D)
    {
	det = 0.0;
    }
    else
    {
	det = pivot[size-1];
	for (i = 0; i < size; i++)
	    det *= a[i][i];
    }

    arr_free((char *) a, 2, DOUBLE, 0);
    arr_free((char *) pivot, 1, LONG, 0);
    arr_free((char *) work, 1, DOUBLE, 0);

    return det;
}
