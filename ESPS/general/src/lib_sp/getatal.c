/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)getatal.c	1.7	2/20/96	ESI/ERL";

#include <stdio.h>

void
get_atal (r, order, c, rc, pgain)
double  r[];
float   rc[], c[], *pgain;
int     order;
{
/*
Compute lpc filter coefficients using Levinson's algorithm.
Input:
	r --- Auto-correlation Function r[1] ... r[order]
Output:
	order reflection coefficients rc[1] to rc[order],
	lpc filter coefficients c[1] to c[order], and
	lpc filter gain *pgain
*/
    double  a0[1000], a1[1000], beta, ref_coeff;
    extern int  debug_level;
    int     i, j;
    double  gain = 1;
    double  alpha = 1.0;

#ifdef UE
    spsassert(r != NULL, "get_atal: r NULL");
    spsassert(rc != NULL, "get_atal: rc NULL");
    spsassert(c != NULL, "get_atal: c NULL");
#endif UE

    for (i = 1; i <= order; i++)
    {
	rc[i] = 0.0;
	c[i] = 0.0;
    }

    for (i = 1; i <= order; i++)
    {
	beta = r[i];
	for (j = 1; j < i; j++)
	    beta = beta - a0[j] * r[i - j];
	ref_coeff = beta / alpha;
	if (ref_coeff >= 1.0 || ref_coeff <= -1.0)
	{
	    if (debug_level >= 1)
		fprintf (stderr,
			"\ninstability noticed in the computation of rc[%d]", i);
	    a0[i] = 0.0;
	    break;
	}
	rc[i] = ref_coeff;
	gain = gain * (1.0 - ref_coeff * ref_coeff);
	alpha = alpha - ref_coeff * beta;

	for (j = 1; j < i; j++)
	{
	    a1[j] = a0[j] - ref_coeff * a0[i - j];
	}
	a1[i] = ref_coeff;
	for (j = 1; j <= i; j++)
	    a0[j] = a1[j];
    }
    for (j = 1; j < i; j++)
	c[j] = a0[j];
    *pgain = gain;
}


