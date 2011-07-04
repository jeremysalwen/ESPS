/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
   "Copyright (c) 1987, 1988 Entropic Speech, Inc.; All rights reserved"
 				
 * 				
 *  Module Name:  refl_to_filt.c
 *
 *  Written By:  Ajaipal S. Virdy
 *               based on reflfilt.c by Bernard G. Fraenkel
 *               changed by Rod Johnson to avoid allocating temp storage
 * 	
 *  Checked By: Rod Johnson
 *
 *  Purpose:  convert reflection coefficients to filter coefficients
 *
 *
 *  Secrets:  None
 *  
 */

#ifndef lint
	static char *sccs_id = "@(#)refl_to_filt.c	1.6	9/18/96	ESI";
#endif

/* ---------------------------------------------------------------------------

			refl_to_filter

Subroutine to go from reflection coefficients to filter coefficients 

Parameters:
  - rfl_cof: array of reflection coefficients
  - filt: array of filter coefficients
  - size: size of the array of filter coefficients 

NOTES:
	refl_to_filter returns filt normalized with filt[0] = -1.0

--------------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>

extern int debug_level;

int
refl_to_filter(rfl_cof, filt, size)
    float   *rfl_cof, *filt;
    int     size;

/*
generates the filter 'filt' using the reflection coefficients stored in array 
'rfl_cof'
*/

{
    float  t;
    float  rc, *dptr1, *dptr2;
    register int    j, k;

/* verify that | reflection coeff | <1 */
    for (j = 1; j < size; j++)
	if (fabs (rfl_cof[j]) > 1.0) {
	    if (debug_level)
		(void) fprintf(stderr,
			" reflection coefficient at order %d > 1 ( = %g )",
			j, rfl_cof[j]);
	    return (1);
	}

/*   make the filter filt  */
    *filt = -1.0;
    for (k = 1; k < size; k++) {

	rc = rfl_cof[k];
	filt[k] = rc;

	for (j = 1, dptr1 = filt + 1, dptr2 = filt + k - 1; j <= k/2; j++)
	{
	    t = *dptr1 - rc * *dptr2;
	    *dptr2-- -= rc * *dptr1;
	    *dptr1++ = t;
	}
    }
    return (0);
}

