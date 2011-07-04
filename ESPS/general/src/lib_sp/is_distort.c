/*  is_distort.c - compute the various Itakura-Saito distortion measures
 *
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice			
 *							
 *               "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 * 				
 *  Module Name:  is_distort.c
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *  Checked By:  Alan Parker
 *
 *  Purpose:  compute the various Itakura-Saito distortion measures
 *
 *
 *  
 */

#ifndef lint
	static char *sccs_id = "@(#)is_distort.c	1.3	11/19/87 ESI";
#endif

#include  <math.h>
#include  <stdio.h>
#include <esps/esps.h>

extern int	debug_level;

double
is_dist_td (autocorrn, autocorrn_err, codeword, codeword_err, size)
float	*autocorrn;
float	autocorrn_err;
float	*codeword;
float	codeword_err;
int	size;
{

/*
 *
 *
 *
 *  is_dist_td  computes the Itakura-Saito distortion between an autocorrn
 *  		autocorrelation sequence and a pretored codeword.
 *
 *	inputs:
 *		autocorrn     = float array of size size containing speech 
 *			        autocorrelation values
 *		autocorrn_err = real value equal to the Log LPC gain
 *			        of the autocorrn
 *		codeword      = float array of size size containing a codeword
 *		codeword_err  = real value equal to the Log LPC gain
 *			        of the codeword
 *		size          = integer dimension of codeword and autocorrn
 *
 *	outputs:
 *		dist          = Itakura-Saito distortion between 
 *		       		autocorrn and codeword
 *
 */


    /* Local Variables */

	double	dist;
	float	alpha;

	int	i;

#ifdef UE
	spsassert(autocorrn != NULL, "is_dist_td: autocorrn NULL");
	spsassert(codeword != NULL, "is_dist_td: codeword NULL");
#endif

	dist = 0.0;
	for (i = 2; i <= size; i++)
	    dist = dist + autocorrn[i] * codeword[i];
	    
	alpha = autocorrn[1] * codeword[1] + 2 * dist;

	dist = (alpha / exp ((double) codeword_err)) - autocorrn_err + 
		codeword_err - 1.0;

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "\nis_dist_td: dist = %g.\n", dist);

	return (dist);

}


double
gnis_dist_td (autocorrn, autocorrn_err, codeword, size)
float	*autocorrn;
float	autocorrn_err;
float	*codeword;
int	size;
{

/*
 *
 *
 *
 *  gnis_dist_td  computes the gain normalized distortion between an autocorrn
 *  		  autocorrelation sequence and a pretored codeword.
 *
 *	inputs:
 *		autocorrn     = float array of size size containing speech 
 *			        autocorrelation values
 *
 *		autocorrn_err = real value equal to the Log LPC gain
 *			        of the autocorrn
 *
 *		codeword      = float array of size size containing a codeword
 *
 *		size          = integer dimension of codeword and autocorrn
 *
 *	outputs:
 *		dist          = Gain normalized distortion between 
 *		       		autocorrn and codeword
 *
 */


    /* Local Variables */

	double	dist;

	int	i;

#ifdef UE
	spsassert(autocorrn != NULL, "gnis_dist_td: autocorrn NULL");
	spsassert(codeword != NULL, "gnis_dist_td: codeword NULL");
#endif

	dist = 0.0;
	for (i = 2; i <= size; i++)
	    dist = dist + autocorrn[i] * codeword[i];
	    
	dist = autocorrn[1] * codeword[1] + 2 * dist;

	dist = log (dist) - autocorrn_err;

	dist = exp (dist) - 1.0;

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "\ngnis_dist_td: dist = %g.\n", dist);

	return (dist);

}


double
gois_dist_td (autocorrn, autocorrn_err, codeword, size)
float	*autocorrn;
float	autocorrn_err;
float	*codeword;
int	size;
{

/*
 *
 *
 *
 *  gois_dist_td  computes the gain-optimized distortion between an autocorrn
 *  		  autocorrelation sequence and a pretored codeword.
 *
 *	inputs:
 *		autocorrn     = float array of size size containing speech 
 *			        autocorrelation values
 *
 *		autocorrn_err = real value equal to the Log LPC gain
 *			        of the autocorrn
 *
 *		codeword      = float array of size size containing a codeword
 *
 *		size          = integer dimension of codeword and autocorrn
 *
 *	outputs:
 *		dist          = Gain normalized distortion between 
 *		       		autocorrn and codeword
 *
 */


    /* Local Variables */

	double	dist;

	int	i;

#ifdef UE
	spsassert(autocorrn != NULL, "gois_dist_td: autocorrn NULL");
	spsassert(codeword != NULL, "gois_dist_td: codeword NULL");
#endif

	dist = 0.0;
	for (i = 2; i <= size; i++)
	    dist = dist + autocorrn[i] * codeword[i];
	    
	dist = autocorrn[1] * codeword[1] + 2 * dist;
	dist = log (dist) - autocorrn_err;

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "\ngois_dist_td: dist = %g.\n", dist);

	return (dist);

}
