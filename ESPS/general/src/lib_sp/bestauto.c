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
 * Revised by:  John Shore (to allow both Wenger and Fraenkel functions)
 *              and to parameterize convergence and max iterations
 *
 * Brief description: accurate estimation of autocorrelation coefficients
 * using generalized burg (i.e., structured covariance method).
 */

/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
*/ 

static char *sccs_id = "@(#)bestauto.c	1.9	10/19/93	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>

#ifndef DEC_ALPHA
char   *malloc ();
#endif
void exit(), free();

extern int  debug_level;

void estimate_covar();

void strcov_auto (data, lnt, R, order, matsiz, window_flag, alg, conv_test, max_iter)
int     lnt, order, matsiz, window_flag;
float   *data;
double  *R;
char    alg;
double  conv_test;
int     max_iter;
/*
A routine for accurate estimation of autocorrelation coefficients
using generalized Burg algorithm (i.e., structured covariance method).
Input:
	data - input data of size "lnt"
	order - The number of auto-correlation coefficents desired.
 	matsiz - Sample covariance matrix size (it should be > order)
 	window_flag - A non-zero value specifies triangular windowing.
	alg - 'w' for Dan Wenger's, 'f' for Bernard Fraenkel's
	conv_test - convergence test for Fraenkel's function
	max_iter - maximum iterations for Fraenkel's function
Output:
	R[0] - signal energy, and
	R[1], ..., r[order] - normalized auto-correlation coefficients.
*/
{
    int     i, j, return_flag = -999;
    int     gmon_flag = (debug_level > 1);
    double *S, *iS, *iR, *Sout, *iSout, pdist;
    double **Smat; 

/* Allocate Memory spaces */
    S = malloc_d (matsiz * matsiz * sizeof *S);
    if (S == NULL)
    {
	Fprintf (stderr, "strcov_auto: couldn't allocate dynamic memory for array - S\n");
	exit (1);
    }

    iS = malloc_d (matsiz * matsiz * sizeof *iS);
    if (iS == NULL)
    {
	Fprintf (stderr, "strcov_auto: couldn't allocate memory for array - iS\n");
	exit (1);
    }

    iR = malloc_d (matsiz * matsiz * sizeof *iR);
    if (iR == NULL)
    {
	Fprintf (stderr, "strcov_auto: couldn't allocate memory for array - iR\n");
	exit (1);
    }

    iSout = malloc_d (matsiz * matsiz * sizeof *iSout);
    if (iSout == NULL)
    {
	Fprintf (stderr, "strcov_auto: couldn't allocate memory for array - iSout\n");
	exit (1);
    }

    Sout = malloc_d (matsiz * matsiz * sizeof *Sout);
    if (Sout == NULL)
    {
	Fprintf (stderr, "strcov_auto: couldn't allocate memory for array - Sout\n");
	exit (1);
    }

    estimate_covar (data, lnt, S, matsiz, window_flag);

    switch (alg) {
	case 'w':
	if (debug_level) 
	  fprintf(stderr, "strcov_auto: calling genburg\n"); 

	genburg (S, iS, &matsiz, &pdist, Sout, iSout, 0, gmon_flag,
		 &return_flag, R, iR, 2, 0);
	break;

	case 'f':
	default:

	Smat = (double **) d_mat_alloc(matsiz,matsiz); 

	for (i = 0; i < matsiz; i++) 
	    for (j = 0; j < matsiz; j++) 
		Smat[i][j] = S[(i*matsiz)+j];
	
	if (debug_level) 
	  fprintf(stderr, "strcov_auto: calling struct_cov\n"); 

	struct_cov(Smat, R, &pdist, matsiz, conv_test, max_iter);

	break;
      }

    if (return_flag == 1 || return_flag == 3 || return_flag == 4 || return_flag == 6)
    {
	for (i = 1; i <= order; R[i++] = 0.0);
    }
    if (debug_level)
	switch (return_flag)
	{
	    case 0: 
		Fprintf (stderr, "No decrease in measure after Maxit attempts\n");
		break;
	    case 1: 
		Fprintf (stderr, "Rinit/Rnew singular or Rinit neg definite\n");
		break;
	    case 2: 
		Fprintf (stderr, "sigma_in or Rinit non-pos-definite\n");
		break;
	    case 3: 
		Fprintf (stderr, "Aij singular\n");
		break;
	    case 4: 
		Fprintf (stderr, "unsuccessful interpolation, no pos-def Rnew\n");
		break;
	    case 5: 
		break;
	    case 6: 
		Fprintf (stderr, "insufficient storage allocation\n");
		break;
	}
    for (i = 1; i <= order; i++)
    {
	R[i] = R[i] / R[0];
    }

    /* 
     Note that all the auto-correlation coefficients are normalized
     and R[0] has energy stored.
     */

    if (debug_level)
	Fprintf (stderr, "pdist = %f\n", pdist);

    free ((char *) S);
    free ((char *) iS);
    free ((char *) iR);
    free ((char *) iSout);
    free ((char *) Sout);
}

void
estimate_covar (data, lnt, Sxx, matsiz, window_flag)
int     lnt, matsiz, window_flag;
float   *data;
double  *Sxx;
{
    int     nsdvct;		/* no. of sample data vectors */
    extern int  debug_level;
    int     ni, nj;
    int     i, j, k, ns, ij, order;
    double  samp;
    float   t;
    double *sample_Sxx;

/* Allocate memory space */

    sample_Sxx = malloc_d (matsiz * matsiz * sizeof *sample_Sxx);
    if (sample_Sxx == NULL)
    {
	Fprintf (stderr, "sample_covar: couldn't allocate memory for array - sample_Sxx\n");
	exit (1);
    }

    order = matsiz - 1;
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
		ij = i * matsiz + j;
		Sxx[ij] = sample_Sxx[ij]
		    = data[ni] * data[nj];
	    }
	}
	for (ns = matsiz; ns < lnt; ns++)
	{
	    /* update sample_covar matrix using shift-invariance property 
	    */
	    for (i = order; i > 0; i--)
		for (j = i; j < matsiz; j++)
		    sample_Sxx[i * matsiz + j] =
			sample_Sxx[(i - 1) * matsiz + j - 1];
	    t = data[ns];
	    for (i = 0; i < matsiz; i++)
		sample_Sxx[i] = t * data[ns - i];

	    /* accumulate covar matrix with proper weight */

	    t = ns - matsiz + 2;
	    if ((2 * t - 1) > nsdvct)
		t = nsdvct - t + 1;
	    for (i = 0; i < matsiz; i++)
		for (j = i; j < matsiz; j++)
		{
		    ij = i * matsiz + j;
		    Sxx[ij] += t * sample_Sxx[ij];
		}
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
	    Sxx[i] = t;
	}
	for (i = 1; i < matsiz; i++)
	    for (j = i; j < matsiz; j++)
		Sxx[i * matsiz + j] = Sxx[(i - 1) * matsiz + j - 1]
		    + data[order - i] * data[order - j]
		    - data[lnt - i] * data[lnt - j];

	t = 1.0 / nsdvct;
    }
    for (i = 0; i < matsiz; i++)
	for (j = i; j < matsiz; j++)
	{
	    Sxx[i * matsiz + j] *= t;
	    Sxx[j * matsiz + i] = Sxx[i * matsiz + j];
	}
    if (debug_level)
    {
	Fprintf (stderr, "\n Sample Covariance Matrix\n ");
	for (i = 0; i < matsiz; i++)
	{
	    for (j = 0; j < matsiz; j++)
	    {
		Fprintf (stderr, "%lg, ", Sxx[i * matsiz + j]);
	    }
	    Fprintf (stderr, "\n");
	}
    }
    free ((char *) sample_Sxx);
}
