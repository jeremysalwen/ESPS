/*  refl_to_auto.c - convert reflection coefficients to auto-correlation
 *		     coefficients

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
 *
 * 				
 *  Module Name:  refl_to_auto.c
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *  Checked By:  Alan Parker
 *
 *  Purpose:  convert reflection coefficients to auto-correlation
 *	      coefficients
 *
 *
 *  
 */

#ifndef lint
	static char *sccs_id = "@(#)refl_to_auto.c	1.8 2/21/96 ESI";
#endif

#include  <stdio.h>
#include  <math.h>
#include  <esps/esps.h>
#define	  SQUARE(x)	(x) * (x)


void
refl_to_auto (ref_coeff, pfe, auto_corr, order)
float	*ref_coeff;	/* input reflection coefficients */
float	pfe;		/* prediction filter error or LPA gain */
float	*auto_corr;	/* conversion to auto-correlation coefficients */
int	order;		/* number of parameters which uniquely specify H(z) */
{

/* 
 * This module was extracted from:
 *
 * 	"PROGRAMS FOR DIGITAL SIGNAL PROCESSING," edited by the 
 *	Digital Signal Processing Committee, IEEE Acoustics, Speech,
 *	and Signal Processing Society, Chapter 4.3, pp. 4.3-1 to 4.3-7.
 *
 *
 * This module converts order number of reflection coefficients into
 *	order + 1 number of auto-correlation coefficients.
 *
 * Inputs -
 *
 *	ref_coeff - Reflection Coefficient array. Notice: only
 *	             ref_coeff[1]...ref_coeff[order] are processed
 *		     (ref_coeff[0] is ignored)
 *	       pfe - The prediction filter error or Linear Predictive
 *		     Analysis Gain (also known as ALPHA)
 *	     order - number of reflection coefficients to convert.
 *
 * Output -
 *
 *	 auto_corr - Corresponding auto-correlation coefficients.
 *		     Note: auto_corr[0] is ignored and
 *			   auto_corr[1] = input_energy (first auto_corr coef.).
 *				.
 *				.
 *				.
 *			   auto_corr[order+1] = last coef.
 */

/* The following are used as indices in loops and arrays */

	int	i;
	int	j, mh;
	int	k, kb;
	int	l, lb;
	

	float	*sa;	/* temporary storage for filter coefficients */

/* temporary variables */

	float	at;
	float	q;
	float	sum;
	float	input_energy;


	/* allocate adequate memory for sa */

	if((sa = (float *)calloc((unsigned)order+1, sizeof (float))) == NULL ) {
	   (void) fprintf (stderr,
	   "refl_to_auto: cannot allocate storage for sa.\n");
	   exit (1);
	}

	/* Compute r(0) or R(1) from ALPHA (which is equal to pfe) */
	
	input_energy = pfe;
	for (i = 1; i <= order; i++) {
	    sa[i] = -ref_coeff[i];
	    input_energy *= 1 / (1 - SQUARE (ref_coeff[i]));
	}

	auto_corr[1] = input_energy;
	auto_corr[2] = ref_coeff[1] * auto_corr[1];

	for (j = 2; j <= order; j++) {
	    mh = j / 2;
	    q = -ref_coeff[j];

	    for (k = 1; k <= mh; k++) {
		kb = j - k;
		at = sa[k] + q * sa[kb];
		sa[kb] += q * sa[k];
		sa[k] = at;
	    }

	    sum = 0.0;

	    for (l = 1; l <= j; l++) {
		lb = j + 1 - l;
		sum = sum + sa[l] * auto_corr[lb];
	    }

	    auto_corr[j + 1] = -sum;
	}

	free ((char *) sa);
}
