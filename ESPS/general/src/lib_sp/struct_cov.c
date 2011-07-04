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
 * Written by:  Bernard Fraenkel
 * Revised by:  John Shore (slightly -- added convergence and iteration
 *              parameters
 */

static char *sccs_id = "@(#)struct_cov.c	1.6	2/21/96	ESI/ERL";

/*---------------------------------------------------------------------------
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 				STRUCT_COV				    

New STRUCTured COVariance algorithm, as invented by John in March 1985 
single channel, single dimension case

Given a sample covariance matrix S, determine the closest Toeplitz matrix R.

Written by: Bernard G. Fraenkel

NOTE: we do NOT assume any symmetry on the sample covariance matrix S
---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#define EPS 1.0e-10 /* floating point zero */

extern int debug_level;

static int levinson();
static int lu_inv();
static int lu_decomp();
static int chk_lu_decomp(); 
static int solv_lu();
static int chk_solv_lu();
static double  tr_s_rinv();

int
struct_cov (S, R, distance, size, cvgtst, max_iter)
double **S, *R, *distance, cvgtst; /* convergence criterion */
int     size;
int     max_iter;
{
    static double **JmJt, **J, **templat, **KmKt, **L, **U, **matrix;
    static double  *F, *rflcof, *Rold, *Y, *dR, *D;
    static int  alloc_siz = 0;

    double  trace, dtemp, **d_mat_alloc (),
            oldtrace, oldcvgr, detS, detR = 1.0, /* for levinson */
	    ddist, oldist,
            cvgr = 1.0e3,	/* intialize to large value */
            alpha = 1.0;
    int     i, k, N = size - 1, center, iter = 0, code;


    if (debug_level > 2)
	fprintf (stderr,"struct_cov: cvg_tst = %lg\n", cvgtst);

/* allocate memory for all matrices */
    if (alloc_siz == 0) {	/* allocate memory for 1st time */
	alloc_siz = size;
	if ((F = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate F", -1);
	if ((rflcof = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate rflcof", -2);
	if ((Rold = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate Rold", -3);
	if ((Y = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate Y", -4);
	if ((dR = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate dR", -5);
	if ((D = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate D", -6);

	if ((JmJt = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate JmJt", -13);
	if ((J = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate J", -14);
	if ((KmKt = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate KmKt", -15);
	if ((L = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate L", -16);
	if ((U = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate U", -17);
	if ((templat = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate templat", -18);
	if ((matrix = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate matrix", -19);
    }
    else if (alloc_siz < size) {/* allocate memory for matrices */
	if ((F = (double *) realloc (F, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate F", -7);
	if ((rflcof = (double *) realloc (rflcof, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate rflcof", -8);
	if ((Rold = (double *) realloc (Rold, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate Rold", -9);
	if ((Y = (double *) realloc (Y, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate Y", -4);
	if ((dR = (double *) realloc (dR, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate dR", -10);
	if ((D = (double *) realloc (D, size * sizeof (double))) == NULL)
	    faterr ("struct_cov", "cannot allocate D", -12);

	d_mat_free (JmJt, alloc_siz);
	if ((JmJt = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate JmJt", -13);
	d_mat_free (J, alloc_siz);
	if ((J = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate J", -14);
	d_mat_free (KmKt, alloc_siz);
	if ((KmKt = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate KmKt", -15);
	d_mat_free (L, alloc_siz);
	if ((L = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate L", -16);
	d_mat_free (U, alloc_siz);
	if ((U = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate U", -17);
	d_mat_free (templat, alloc_siz);
	if ((templat = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate templat", -18);
	d_mat_free (matrix, alloc_siz);
	if ((matrix = d_mat_alloc (size, size)) == NULL)
	    faterr ("struct_cov", "cannot allocate matrix", -19);
	alloc_siz = size;	/* only update at the end since use old
				   one to free */
    }

 /* make S symmetric about both diagonals,  it does not change the result 
 */
    center = N / 2 + 1;
    for (i = 0; i <= center; i++)
	S[i][i] = S[N - i][N - i] = 0.5 * (S[i][i] + S[N - i][N - i]);
    for (i = 0; i < center; i++) {
	for (k = i + 1; k <= N - i; k++) {
	    dtemp = 0.25 * (S[i][k] + S[k][i] + S[N - i][N - k] + S[N - k][N - i]);
	    S[i][k] = S[k][i] = S[N - i][N - k] = S[N - k][N - i] = dtemp;
	}
    }
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "Symmetrized covariance matrix\n");
	print_mat (S, size);
    }

/* compute the LU decomposition of S in order to get its determinant */
    if ((code = lu_decomp (S, size, D, L, U)) != 0)
	fprintf (stderr, "struct_cov:  ludecomp returns : %d for the sample covariance matrix\n", code);
    if (debug_level)
	if ((code = chk_lu_decomp (S, size, D, L, U)) != 0)
	    fprintf (stderr, "struct_cov:  chk_lu_decomp returns : %d for the sample covariance matrix\n", code);
/* compute the determinant */
    for (i = 1, detS = D[0]; i < size; i++)
	detS *= D[i];
    if (detS <= 0.0) {
	if (debug_level)
	    fprintf (stderr, "struct_cov: WARNING : S is not pos. def. : detS = %lg\n", detS);
	detS = 1.0;		/* reset detS so that log (detS) exists
				   and distance>0 */
	if (debug_level)
	    fprintf (stderr, "struct_cov: detS reset to %lg\n", detS);
    }
    detS = log (detS);
    if (debug_level)
	fprintf (stderr, "struct_cov: log(detS) = %lg\n", detS);


/* make the matrix J from the sample covariance matrix S */
    for (k = N; k >= 0; k--)
	J[N][k] = S[N][k];
    for (i = N - 1; i >= 0; i--)
	for (k = N - 1, J[i][N] = S[i][N]; k >= 0; k--)
	    J[i][k] = S[i][k] + J[i + 1][k + 1];
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "matrix J\n");
	print_mat (J, size);
    }

/* make the matrix JmJt: J minus J flipped around & shifted down and right */
/* the first row is equal to that of J */
    for (k = 0; k <= N; k++)
	JmJt[0][k] = J[0][k];
    for (i = 1; i <= N; i++)	/* i is the row number */
	for (k = 1, JmJt[i][0] = J[i][0]; k <= N; k++)
	    JmJt[i][k] = J[i][k] - J[N - k + 1][N - i + 1];
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "JmJt matrix\n");
	print_mat (JmJt, size);
    }

/* make the template matrix for the same operation performed on R */
    for (i = 0; i < size; i++)
	for (k = 0; k < size; k++)
	    templat[i][k] = size - i - k;

    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "template matrix\n");
	print_mat (templat, size);
    }

/* make initial guess : average of the diagonals of S which is SYMMETRIC */
    for (i = 0; i < size; i++) {
	for (k = i, R[i] = 0.0; k < size; k++)
	    R[i] += S[k - i][k];
	R[i] /= (size - i);
    }
 /* make sure R[0] > 0.0 */
    if (*R < 0.0)
	*R = -*R;
    else if (*R == 0.0)
	*R = 1.0;
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "initial guess :\n");
	for (i = 0; i < size; i++)
	    fprintf (stderr, "Rinit[%d] = %lg\n", i, R[i]);
    }
/* get the p.e.f. */
/* if R is not an auto-correlation scale R[0] until it is */
    while (levinson (R, F, rflcof, &detR, size) != 0) {
	R[0] *= 2.0;
	if (debug_level >= 3 && stderr != NULL) {
	    fprintf (stderr, "scaling R[0] because levinson failed\n");
	    fprintf (stderr, "new Rinit[0] = %lg\n", R[0]);
	}
	if (debug_level)
	    fprintf (stderr, "struct_cov: new Rinit[0] = %lg\n", R[0]);
    }
    trace = tr_s_rinv (S, F, rflcof[0], size);
    detR = log (detR);
    *distance = trace - detS + detR - size;
    if (debug_level) {
	fprintf (stderr, "struct_cov: the initial trace is equal to : %-20.15lg\n", trace);
	fprintf (stderr, "struct_cov: the initial distance is equal to : %-20.15lg\n", *distance);
    }

/* start main loop */
    do {
	if (++iter > max_iter) {
	    fprintf (stderr, "struct_cov: iter (%d) > max_iter (%d)\n", iter, max_iter);
	    fprintf (stderr, "struct_cov: PREMATURE EXIT\n");
	    return (1);
	}
	oldcvgr = cvgr;
	oldtrace = trace;
	oldist = *distance;
	for (i = 0; i < size; i++)
	    dR[i] = R[i] - Rold[i];
    /* shift R into Rold */
	for (i = 0; i < size; i++)
	    Rold[i] = R[i];
	alpha = 1.0;

/* compute 'matrix' and 'Y' for the matrix equation until "lu_inv" gives a
   positive definite solution, if not take 1/2 step from Rold 
This 'while' loop is tricky because R represents 2 things: before lu_inv it is
the current tentative previous guess, (tentative because if lu_inv fails, then
R is recomputed from Rold: the estimate 2 iterations before, taking 1/2 step),
as output of lu_inv, R represents
the tentative current guess, (tentative because we have to make sure that it 
is an auto-correlation).  This is why dR has 2 meanings: difference between 
the tentative previous guess and the validated estimate 2 iterations before,
or difference between the current tentative guess and the previous validated 
estimate
*/
	while (1) {
	/* compute the right hand side Y of the matrix equation */
	/* F[0] is assumed to be equal to 1 */
	    for (i = 0; i < size; i++)
		for (k = 1, Y[i] = JmJt[i][0]; k < size; k++)
		    Y[i] += JmJt[i][k] * F[k];

	/* make the matrix part of the matrix equation */
	    if (debug_level >= 3 && stderr != NULL)
		for (i = 0; i < size; i++)
		    fprintf (stderr, "F[%d] = %lg \n", i, F[i]);
	    for (i = 0; i < size; i++)
		for (k = 0; k < size; k++)
		    KmKt[i][k] = templat[i][k] * F[k];
	    if (debug_level >= 3 && stderr != NULL) {
		fprintf (stderr, "KmKt matrix\n");
		print_mat (KmKt, size);
	    }
/* reorder the coefficients properly as factors of R(0)  ... R(N) */
	    for (k = 0; k < size; k++)
		matrix[0][k] = KmKt[0][k];
	    for (i = 1; i < size; i++) {
		matrix[i][0] = KmKt[i][i];
		for (k = 1; k < size; k++) {
		    if (k <= i && k < size-i)
			matrix[i][k] = KmKt[i][i - k] + KmKt[i][i + k];
		    else if (k <= i)
			matrix[i][k] = KmKt[i][i - k];
		    else if (k < size-i)
			matrix[i][k] = KmKt[i][i + k];
		    else
			matrix[i][k] = 0;
		}
	    }
	    if (debug_level >= 3 && stderr != NULL) {
		fprintf (stderr, "matrix matrix\n");
		print_mat (matrix, size);
	    }

	/* solve the matrix equation */
	    if (debug_level >= 3 && stderr != NULL) {
		fprintf (stderr, "Y vector for lu_inv\n");
		for (i = 0; i < size; i++)
		    fprintf (stderr, "Y[%d] = %lg\n", i, Y[i]);
	    }

	/* lu_inv returns != 0 if 'matrix' is singular */
	    if (lu_inv (matrix, R, Y, size) == 0)
		break;		/* R is a potential guess */

	    else {
/* rebuild an estimate of R using Rold, making half a step, making
  sure we still have an auto-correlation
   note that dR is the difference between the  previous 2 guesses 
*/
		if (debug_level >= 3 && stderr != NULL) {
		    fprintf (stderr, "scaling alpha to solve LUINV\n");
		    fprintf (stderr, "alpha = %lg\n", alpha);
		}
		if (debug_level)
		    fprintf (stderr, "struct_cov: alpha = %lg\n", alpha);
		do {
		    if (iter > 1) {
			alpha *= 0.5;
			for (i = 0; i < size; i++)
			    R[i] = Rold[i] + alpha * dR[i];
		    }
		    else {	/* dR is undefined for 1st iteration */
			*R = 2.0 * *Rold;
			for (i = 1; i < size; i++)
			    R[i] = Rold[i];
		    }
		} while (levinson (R, F, rflcof, &detR, size) != 0);
	    }
	}

	if (debug_level)
	    fprintf (stderr, "struct_cov:  final alpha after lu_inv = %lg\n", alpha);

/* dR represents the difference between the tentative guess R and the previous
    solution Rold */
	for (i = 0; i < size; i++)
	    dR[i] = R[i] - Rold[i];
	alpha = 1.0;
    /* compute p.e.f. */
	while (levinson (R, F, rflcof, &detR, size) != 0) {
	    alpha *= 0.5;
	    for (i = 0; i < size; i++)
		R[i] -= alpha * dR[i];
	}
	if (debug_level)
	    fprintf (stderr, "struct_cov:  final alpha after levinson = %lg\n", alpha);

/* compute the convergence criterion */
	for (i = 0, cvgr = 0.0; i < size; i++) {
	    dtemp = Rold[i] - R[i];
	    cvgr += dtemp * dtemp;
	}
	if (debug_level >= 3 && stderr != NULL) {
	    fprintf (stderr, "final R\n");
	    for (i = 0; i < size; i++)
		fprintf (stderr, "R[%d] = %-20.14lg\n", i, R[i]);
	}
	if (R[0] != 0.0)
	    cvgr = sqrt (cvgr) / R[0];
	else
	    fprintf (stderr, "struct_cov: ?! iter %d  R[0] = 0.0 !?\n", iter);

	trace = tr_s_rinv (S, F, rflcof[0], size);
	detR = log (detR);
	*distance = trace - detS + detR - size;
	ddist = oldist - *distance;
	if (debug_level) {
	    fprintf (stderr, " \t\t\tITERATION : %d\n", iter);
	    fprintf (stderr, " trace = %-20.15lg\t", trace);
	    fprintf (stderr, " old trace = %-20.15lg\n", oldtrace);
	    fprintf (stderr, " distance = %-20.15lg\t", *distance);
	    fprintf (stderr, " old distance = %-20.15lg\n", oldist);
	    fprintf (stderr, " \t\tdecrease in distance is: %-20.15lg\n", ddist);
	    if (ddist < 0.0 && iter > 1)
		fprintf (stderr, "struct_cov:  ?! WARNING distance INCREASED !?\n");
	    fprintf (stderr, " cvgr = %-20.15lg\t", cvgr);
	    fprintf (stderr, " old cvgr = %-20.15lg\n", oldcvgr);
	}
    } while (cvgr > cvgtst);

    return (0);
}

/* ----------------------------------------------------------------------- */


/*---------------------------------------------------------------------------

			LEVINSON and INV_LEVINSON

Levinson-Durbin algorithm in the case of real signals (auto-correlation to 
filter coefficients), and inverse Levinson (filter coefficients to auto-
correlation).

"Inverse" Levinson algorithm :    
    Given a minimum phase filter pef, compute the autocorrelation
values autcor corresponding to the inverse filter, and also return the
reflection coefficients.
The returned auto-correlation is normalized such that autcor[0] = gain of the
 filter = rflcof[0]

Parameters:
autcor is a symmetric Toeplitz matrix of size 'size'
pef is the corresponding prediction error filter (returned with pef[0] = 1.0)
rflcof is the array of the reflection coefficients with rflcof[0] = prediction
	error of order (size-1)
*detR is the determinant of autcor which is equal to the product of the 
	prediction errors of the various orders
NOTE: since it is not always desired to compute detR (if 'size' is large, so 
	is detR => possibility of over-flow), 'levinson' will only compute
	detR if its initial value when passed to the subroutine is non zero.
	If detR is desired, one should make sure to make it non-zero before
	calling 'levinson'.

Written by: Bernard G. Fraenkel
----------------------------------------------------------------------------*/
#ifndef APOLLO_68K
#include <stdio.h>
#include <math.h>
#endif
extern int debug_level;

#define L_EPS 1.0e-6
static
levinson (autcor, pef, rflcof, detR, size)
double *autcor, *pef, *rflcof, *detR;
int     size;

{
    static double  *old_pef;	/* p.e.f. of order (current order - 1) */
    static int  alloc_siz = 0;	/* size of storage allocated to old_pef */
    double  rc, pred_err;
    int     detR_flag = 0;	/* detR is not computed unless flag is set
				   */
    register int    i, j, k, order = size - 1;
    register double temp;

 /*   Tests and initialization */

    if (autcor[0] <= 0.0e0) {
	if (debug_level)
	    fprintf (stderr, "levinson: autcor[0] is negative\n");
	return (1);
    }

/* if first time allocate storage for old_pef */
    if (alloc_siz == 0) {
	alloc_siz = size;
	if ((old_pef = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("levinson", "cannot allocate storage for old_pef", -9);
    }
    else if (alloc_siz < size) {/* need to reallocate more storage */
	alloc_siz = size;
	if ((old_pef = (double *) realloc (old_pef, size * sizeof (double))) == NULL)
	    faterr ("levinson", "cannot reallocate storage for old_pef", -10);
    }

/* compute values for orders 0 and 1 */
    pred_err = *autcor;
    if (*detR != 0.0) {		/* => want to compute the value of detR */
	detR_flag = 1;
	*detR = *autcor;
    }
    if (debug_level > 2) {
	if (detR_flag)
	    fprintf (stderr, "levinson: computing detR\n");
	else
	    fprintf (stderr, "levinson: NOT computing detR\n");
    }
    rc = -autcor[1] / autcor[0];
    if (fabs (rc) > 1.0e0) {
	if (debug_level)
	    fprintf (stderr, "levinson : first refl. coeff. is greater than 1\n");
	return (2);
    }
    rflcof[1] = rc;
    pred_err *= (1.0e0 - rc * rc);
    if (detR_flag)
	*detR *= pred_err;
    *pef = 1.0;
    pef[1] = rc;
    if (order <= 1) {
	*rflcof = pred_err;
	return (0);
    }

 /* orders greater than 1 */
    for (i = 2; i < size; i++) {
    /*  shift pef into old_pef */
	for (j = 1; j < i; j++)
	    old_pef[j] = pef[j];

/* compute reflection coefficient */
	for (j = 1, temp = autcor[i]; j < i; j++)
	    temp += autcor[i - j] * old_pef[j];
	rc = -temp / pred_err;
	if (fabs (rc) > 1.0e0) {
	    if (debug_level)
		fprintf (stderr, "levinson : | refl. coeff.[%d] = %lg | > 1\n", i, rc);
	    return (2);
	}
	rflcof[i] = rc;
	pred_err *= (1.0 - rc * rc);
	if (detR_flag)
	    *detR *= pred_err;
/* compute the prediction error filter */
	pef[i] = rc;
	for (j = 1; j < i; j++)
	    pef[j] = old_pef[j] + rc * old_pef[i - j];

/* check if the process is not of order lower than expected */
	if (fabs (rc) == 1.0e0) {
	    if (debug_level)
		fprintf (stderr, "WARNING the process is only %d th order\n", i);
	    if (i < size - 1)
		for (j = i + 1; j < size; j++)
		    pef[j] = 0.0e0;
	    rflcof[0] = pred_err;
	    return (0);
	}
    }
/* assign the total prediction error in rflcof[0] */
    rflcof[0] = pred_err;

    if (debug_level > 2) {	/* Check the prediction error filter
				   equation */
	fprintf (stderr, " levinson check \n");
	for (i = 1, temp = *autcor; i < size; i++)
	    temp += autcor[i] * pef[i];

	if (fabs (temp - pred_err) > L_EPS)
	    fprintf (stderr, " order 0 : temp = %lg     pred_err = %lg \n", temp, pred_err);

	for (i = 1; i < size; i++) {
	    for (j = 1, k = i - 1, temp = autcor[i]; j < size; j++, k--)
		temp += autcor[abs (k)] * pef[j];
	    if (fabs (temp) > L_EPS)
		fprintf (stderr, " order %d : temp = %lg  != 0    \n", i, temp);
	}

    }				/*     End of check */

    return (0);
}

/* ------------------------------------------------------------------ */


static
inv_levinson (pef, autcor, rflcof, size)
double *pef, *rflcof, *autcor;
int     size;

/* ------
"Inverse" Levinson algorithm :    
    Given a minimum phase filter pef, compute the autocorrelation
values autcor corresponding to the inverse filter.
fil contains all the coefficients of the prediction error filters in
increasing order.
At the same time, get the reflection coefficients rflcof.
The returned auto-correlation is normalized such that autcor[0] = gain of the
 filter = rflcof[0]
----- */

{
    static double  *fil;	/* array of all the filter coefficients */
    static int  alloc_siz = 0;
    double  gain, rc, oocsq, dtemp, *dptr, *fil_ptr;
    int     order, sigma;
    register int    k, j, ff, fb;

/*
the coefficients of the successive prediction error filters are 
    	stored, in the array fil, in the following way : 
    	- in increasing order of 'order of the filter'    
    	- for each filter, in increasing order of 'coefficient order'
    	  the coefficient of order 0 is never stored since it is    
    		always equal to 1.    
    	  i.e. fil(1) is the coefficient of the filter of order 1    
    		the second order prediction error filter is equal to   
    		1+ fil(2).Z + fil(3).Z    
*/

/*        INITIALIZATION    */

    order = size - 1;
    sigma = order * size / 2;

/* if first time allocate storage for fil */
    if (alloc_siz == 0) {
	alloc_siz = size;
	k = sigma + 1;		/* watch that "+1" */
	if ((fil = (double *) malloc (k * sizeof (double))) == NULL)
	    faterr ("inv_levinson", "cannot allocate storage for fil", -1);
    }
    else if (alloc_siz < size) {/* need to reallocate more storage */
	alloc_siz = size;
	k = sigma + 1;		/* watch that "+1" */
	if ((fil = (double *) realloc (fil, k * sizeof (double))) == NULL)
	    faterr ("inv_levinson", "cannot reallocate storage for fil", -1);
    }

/* normalize the filter so that the coefficient of order 0
		is equal to 1 */
    if (*pef == 0.0) {
	if (debug_level)
	    fprintf (stderr, " inv_levinson init -> pef[0] = 0.0  \n");
	return (1);
    }
    gain = 1.0 / *pef;
    for (k = order, fil_ptr = fil + sigma, dptr = pef + order; k > 0; k--)
	*fil_ptr-- = *dptr-- * gain;
    gain *= gain;

/*       MAIN LOOP    */

    fil_ptr = fil + sigma - order;
    for (k = order, fb = sigma; k >= 2; k--) {
	rc = fil[fb--];
	if (fabs (rc) >= 1.0) {
	    if (debug_level)
		fprintf (stderr, " | refl coeff(%d) = %-20.15lg | >= 1.0 \n", k, rc);
	    return (2);
	}
	rflcof[k] = rc;
	oocsq = 1.0 / (1.0 - rc * rc);
	gain *= oocsq;
    /* 
     if (fb != k*(k+1)/2-1)
     fprintf (stderr, "tu t''es plante dans ff mon gars\n");
     if (ff  !=  k*(k-1)/2+1 || ff != fb-k+2) 
     fprintf (stderr, "tu t''es plante dans fb mon gars\n");
     */
/*
     - We are here computing the filter of order (k-1). The coefficients
    	are computed in decreasing order, i.e. from j=k-1 to 1
    	fb coefficient j , filter of order k
    	ff coefficient k-j, filter of order k    
*/
	ff = fb - k + 2;
	for (j = k - 1; j >= 1; j--, fb--, ff++)
	    *fil_ptr-- = oocsq * (fil[fb] - rc * fil[ff]);
    }

    rc = fil[1];
    if (fabs (rc) >= 1.0) {
	if (debug_level)
	    fprintf (stderr, " | refl coeff(1) = %-20.15lg | >= 1.0 \n", rc);
	return (2);
    }
    rflcof[1] = rc;
    oocsq = 1.0 / (1.0 - rc * rc);
    gain *= oocsq;
 /* chose to normalize auto-correlation so that autcor[0] = gain of
    filter, which we also store by compatibility with other programs in
    rflcof[0] as well */
    *autcor = *rflcof = gain;

 /* build the auto-correlation using for each order the corresponding pef 
 */
    for (k = 1, fil_ptr = fil + 1; k < size; k++) {
	for (j = k, dptr = autcor + k - 1, dtemp = 0.0; j > 0; j--)
	    dtemp -= *fil_ptr++ * *dptr--;
	autcor[k] = dtemp;
    }

    return (0);
}


/* 				LUINV					 */

/* solve the vectorial equation in X : matrix . X = Y
	where "matrix" is a matrix "size" x "size"
	      X and Y are vectors "size" long
 */


#define CHECK_TST 1.0e-10	/* parameter in the "check" routine
				  sort of a floating point 0 */

static int
lu_inv (matrix, X, Y, size)
double **matrix, *X, *Y;
int     size;
{
    static double **L, **U, *D, *arr;
    static int  alloc_siz = 0;
    double **d_mat_alloc ();
    int     code;

/* allocate memory for all matrices */
    if (alloc_siz == 0) {	/* allocate memory for the first time */
	alloc_siz = size;
	if ((D = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("lu_inv", "cannot allocate D", -1);
	if ((arr = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("lu_inv", "cannot allocate arr", -2);
	if ((L = d_mat_alloc (size, size)) == NULL)
	    faterr ("lu_inv", "cannot allocate L", -5);
	if ((U = d_mat_alloc (size, size)) == NULL)
	    faterr ("lu_inv", "cannot allocate U", -6);
    }
    else if (alloc_siz < size) { /* re-allocate memory */
	if ((D = (double *) realloc (D, size * sizeof (double))) == NULL)
	    faterr ("lu_inv", "cannot reallocate D", -3);
	if ((arr = (double *) realloc (arr, size * sizeof (double))) == NULL)
	    faterr ("lu_inv", "cannot reallocate arr", -4);
	d_mat_free (L, alloc_siz);
	if ((L = d_mat_alloc (size, size)) == NULL)
	    faterr ("lu_inv", "cannot allocate L", -5);
	d_mat_free (U, alloc_siz);
	if ((U = d_mat_alloc (size, size)) == NULL)
	    faterr ("lu_inv", "cannot allocate U", -6);
	alloc_siz = size;	/* only update at the end since use old
				   one to free */
    }
    code = lu_decomp (matrix, size, D, L, U);
    if (debug_level && (code != 0)) {
	fprintf (stderr, " lu_decomp returns : %d\n", code);
	return (code);
    }
    code = chk_lu_decomp (matrix, size, D, L, U);
    if (debug_level && (code != 0)) {
	fprintf (stderr, " chk_lu_decomp returns : %d\n", code);
	return (code);
    }
    code = solv_lu (D, L, U, Y, size, arr, X);
    if (debug_level && (code != 0)) {
	fprintf (stderr, " solv_lu returns : %d\n", code);
	return (code);
    }
    code = chk_solv_lu (matrix, X, Y, size);
    if (debug_level && (code != 0)) {
	fprintf (stderr, " chk_solv_lu returns : %d\n", code);
	return (code);
    }

    return (0);
}

/* ------------------------------------------------------------------------ */


/*				LUDECOMP				    */

/*
LU decomposition of a matrix

matrix = L.D.U where L is lower triangular with 1s on the main diagonal
U is an upper triangular matrix with 1s on the main diagonal
D is diagonal 
*/

static int
lu_decomp (matrix, size, D, L, U)
double **matrix, *D, **L, **U;
int     size;

{
    register int    i, j, k, q;

/* initialization */
    for (i = 0; i < size; i++)
	U[i][i] = L[i][i] = 1.0;

/* i is the row indice ; j the column indice */
    for (q = 0; q < size; q++) {
    /* compute D[q] and check D[q] != 0 */
	for (k = 0, D[q] = matrix[q][q]; k < q; k++)
	    D[q] -= L[q][k] * D[k] * U[k][q];
	if (debug_level && (D[q] == 0.0)) {
	    fprintf (stderr, "lu_decomp : D[%d] = 0.0\n", q);
	    return (1);
	}
	else if (fabs (D[q]) < CHECK_TST)
	    fprintf (stderr, "lu_decomp : WARNING : D[%d] = %-20.15lg\n", q, D[q]);

	for (i = q + 1; i < size; i++) {
	    for (k = 0, L[i][q] = matrix[i][q]; k < q; k++)
		L[i][q] -= L[i][k] * D[k] * U[k][q];
	    L[i][q] /= D[q];
	}

	for (j = q + 1; j < size; j++) {
	    for (k = 0, U[q][j] = matrix[q][j]; k < q; k++)
		U[q][j] -= L[q][k] * D[k] * U[k][j];
	    U[q][j] /= D[q];
	}
    }

    if (debug_level >= 4 && stderr != NULL) {
	fprintf (stderr, " L matrix \n");
	for (i = 0; i < size; i++)
	    for (j = 0; j <= i; j++)
		fprintf (stderr, "%c%-12.8lg", j == 0 ? '\n' : ' ', L[i][j]);
	fprintf (stderr, "\n D matrix \n");
	for (i = 0; i < size; i++)
	    fprintf (stderr, "%-12.8lg\n", D[i]);
	fprintf (stderr, "\nU matrix transpose");
	for (i = 0; i < size; i++)
	    for (j = 0; j <= i; j++)
		fprintf (stderr, "%c%-12.8lg", j == 0 ? '\n' : ' ', U[j][i]);
	fprintf (stderr, "\n");
    }

    return (0);
}

/* ------------------------------------------------------------------------ */

static int
chk_lu_decomp (matrix, size, D, L, U)
double **matrix, *D, **L, **U;
int     size;

/*
check that sampco is equal to
matrix = L.D.U where D is diagonal and L lower triangular 
U is upper triangular
array is a temporary array representing a column of D.U
*/

{
    register int    i, j, k;
    int     not_OK = 0;		/* not_OK=0 <=> everything went fine */
    double  dtemp;
    static double  *array;
    static int alloc_siz = 0;

    if (alloc_siz == 0) {	/* allocate memory */
	alloc_siz = size;
	if ((array = (double *) malloc (size * sizeof (double))) == NULL)
	    faterr ("chk_lu_decomp", "cannot allocate array", -1);
    }
    else if (alloc_siz < size) {/* reallocate memory */
	alloc_siz = size;
	if ((array = (double *) realloc (array, size * sizeof (double))) == NULL)
	    faterr ("chk_lu_decomp", "cannot reallocate array", -1);
    }


/* j refers to the column indice */
    for (j = 0; j < size; j++) {
    /* array is the jth column of D.U */
	for (k = 0; k < j; k++)
	    array[k] = U[k][j] * D[k];
	array[j] = D[j];
	for (k = j + 1; k < size; k++)
	    array[k] = 0.0;
    /* i is the row indice */
	for (i = 0; i < size; i++) {
	    for (k = 0, dtemp = array[i]; k < i; k++)/* L[i][i] = 1 */
		dtemp += L[i][k] * array[k];
	    if (fabs (dtemp - matrix[i][j]) > CHECK_TST) {
		not_OK = 1;
		if (debug_level)
		    fprintf (stderr, " !? lu_decomp/chk_lu_decomp : dtemp = %-20.15lg != matrix[%d][%d] = %-20.15lg\n", dtemp, i, j, matrix[i][j]);
	    }
	}
    }

    return (not_OK);		/* if chk_lu_decomp went fine then not_OK
				   = 0 */
}

/* ------------------------------------------------------------------------ */

static int
solv_lu (D, L, U, P, size, Y, filt)
double *D, **L, **U, *P, *Y, *filt;
int     size;

{
    register int    i, j;

    for (i = 0; i < size; i++)
	for (j = 0, Y[i] = P[i]; j < i; j++)
	    Y[i] -= L[i][j] * Y[j];
    for (i = size-1; i >= 0; i--)
	for (j = i + 1, filt[i] = Y[i] / D[i]; j < size; j++)
	    filt[i] -= U[i][j] * filt[j];

    return (0);
}
/* ------------------------------------------------------------------------ */

static int
chk_solv_lu (matrix, filt, P, size)
double **matrix, *filt, *P;
int     size;

/* check the result of solv_lu */

{
    double  dtemp;
    register int    i, j;
    int     not_OK = 0;		/* return code for the subroutine */

/* assume NO SYMMETRY */
    for (i = 0; i < size; i++) {
	for (j = 0, dtemp = 0.0; j < size; j++)
	    dtemp += filt[j] * matrix[i][j];
	if (fabs (dtemp - P[i]) > CHECK_TST) {
	    not_OK = 1;
	    if (debug_level)
		fprintf (stderr, "chk_solv_lu: !? dtemp = %-20.15lg != P[%d] =%-20.15lg\n", dtemp, i, P[i]);
	}
    }

    return (not_OK);		/* not_OK=0 if everything went fine */
}

/* ------------------------------------------------------------------------ */




/*
			    TR_S_RINV

i.e trace (S.Rinv)
Computes the trace of the product of the 2 matrices S and Rinv,
 where R is a Toeplitz matrix, Rinv its inverse and S a sample covariance
matrix. This  version takes as argument the p.e.f.
corresponding to the Toeplitz matrix R instead of R itself and thus
avoids calling "levinson"
Inputs:
    S: sample covariance matrix
    pef: prediction error filter corresponding to the Toeplitz matrix R
	Rinv is the inverse of R
	IT IS ASSUMED THAT pef[0] = 1.0
    pred_err: is the prediction error corresponding to the matrix R and the
	p.e.f. pef with pef[0] = 1.0
    size: size of the matrices
Outputs:
    tr_s_rinv: returns trace (S.Rinv)

Note: pef and pred_err can be obtained by calling the subroutine Levinson.

Given a sample covariance matrix S, determine the closest Toeplitz matrix R.

Written by: Bernard G. Fraenkel

NOTE: we do NOT assume any symmetry on the sample covariance matrix S
----------------------------------------------------------------------------*/


static
double  tr_s_rinv (S, pef, pred_err, size)
double **S, *pef, pred_err;
int     size;

{
    static double **J, **mat;	/* temporary matrices */
    static int  alloc_siz = 0;	/* storage previously allocated */
    double **d_mat_alloc ();
    double *dptr, trace, dtemp;
    int     N, i, l;

    if (alloc_siz == 0) {	/* allocate memory */
	alloc_siz = size;
	if ((mat = d_mat_alloc (size, size)) == NULL)
	    faterr ("tr_s_rinv", "cannot allocate mat", -1);
	if ((J = d_mat_alloc (size, size)) == NULL)
	    faterr ("tr_s_rinv", "cannot allocate J", -2);

    }
    else if (alloc_siz < size) {/* reallocate memory */
	d_mat_free (mat, alloc_siz);
	if ((mat = d_mat_alloc (size, size)) == NULL)
	    faterr ("tr_s_rinv", "cannot allocate mat", -1);
	d_mat_free (J, alloc_siz);
	if ((J = d_mat_alloc (size, size)) == NULL)
	    faterr ("tr_s_rinv", "cannot allocate J", -2);
	alloc_siz = size;	/* only update at the end since use old
				   one to free */
    }


    N = size - 1;
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "sample covariance matrix\n");
	print_mat (S, size);
    }

    if (*pef != 1.0)
	fprintf (stderr, "tr_s_rinv: !? pef[0] != 1.0 ?!", 1);
    if (pred_err <= 0.0)
	faterr ("tr_s_rinv", "prediction err. is negative", 2);

/* make the matrix J */
    for (l = N; l >= 0; l--)
	J[N][l] = S[N][l];
    for (i = N - 1; i >= 0; i--)
	for (l = N - 1, J[i][N] = S[i][N]; l >= 0; l--)
	    J[i][l] = S[i][l] + J[i + 1][l + 1];
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "matrix J\n");
	print_mat (J, size);
    }

/* the first row is equal to that of J */
    for (l = 0; l <= N; l++)
	mat[0][l] = J[0][l];
    for (i = 1; i <= N; i++)	/* i is the row number */
	for (l = 1, mat[i][0] = J[i][0]; l <= N; l++)
	    mat[i][l] = J[i][l] - J[N - l + 1][N - i + 1];
    if (debug_level >= 3 && stderr != NULL) {
	fprintf (stderr, "mat matrix\n");
	print_mat (mat, size);
    }

/* compute the scalar product trace = pef_transpose . mat . pef */
    trace = 0.0;
    for (i = 0; i <= N; i++) {
	for (l = 0, dptr = pef, dtemp = 0.0; l <= N; l++)
	    dtemp += *dptr++ * mat[i][l];
	trace += dtemp * pef[i];
    }

    trace /= pred_err;

    return (trace);
}

/*  ---------------------------------------------------------------------   */

