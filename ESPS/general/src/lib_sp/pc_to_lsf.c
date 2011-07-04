 /*
  * This material contains proprietary software of Entropic Speech, Inc.
  * Any reproduction, distribution, or publication without the the prior
  * written permission of Entropic Speech, Inc. is strictly prohibited.
  * Any public distribution of copies of this work authorized in writing by
  * Entropic Speech, Inc. must bear the notice
  *
  *     "Copyright (c) 1987 Entropic Speech, Inc."; All Rights Reserved
  *
  * Function: pc_to_lsf
  *
  * Written by: Shankar Narayan
  * Checked by: David Burton
  *
  * pc_to_lsf -- transform autoregressive filter coefficients to line spectral
  *		frequencies
  */

#ifndef lint
static char *sccs_id = "@(#)pc_to_lsf.c	1.18 9/18/96 ESI";
#endif

/*
 * system include files
*/
#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/unix.h>


#ifndef FLT_MIN
#define FLT_MIN  1e-37
#endif

static void get_frep();

int
pc_to_lsf (pc, lsf, order, bwidth, frq_res)
int     order;			/* Filter order */
float   pc[],			/* Prediction filter */
        lsf[],			/* LSF array */
        bwidth,			/* Bandwidth = sf / 2 */
        frq_res;		/* Frequency Resolution for search grid
				  --- bwidth / 200 is a good number. As
				  interpolation is performed on the frequency
			  response prior to null detection, higher value
				  than this does not seem to help much. */
{
    float   *e_filter, *o_filter, *c;
    int     i, j, e_order, o_order;
    float  *e_fresp, *o_fresp, prev, *ptr;
    int     npts = (int) (bwidth / frq_res);


    if (order % 2 == 0)
	e_order = o_order = order / 2;
    else
    {
	e_order = (order + 1) / 2;
	o_order = (order - 1) / 2;
    }
/* Allocate memory for even and odd filters and their frequency responses */

    e_filter = (float *) calloc ((unsigned) (order + 2), sizeof (float));
    spsassert(e_filter != NULL,
	      "pc_to_lsf: Couldn't calloc memory for e_filter");
    o_filter = (float *) calloc ((unsigned) (order + 2), sizeof (float));
    spsassert(o_filter != NULL,
	      "pc_to_lsf: Couldn't calloc memory for o_filter");
    e_fresp = (float *) calloc ((unsigned) (npts+1), sizeof (float));
    spsassert(e_fresp != NULL,
	      "pc_to_lsf: Couldn't calloc memory for e_fresp");
    o_fresp = (float *) calloc ((unsigned) (npts+1), sizeof (float));
    spsassert(o_fresp != NULL,
	      "pc_to_lsf: Couldn't calloc memory for o_fresp");

    c = (float *) calloc ((unsigned) (order + 2), sizeof (float));
    spsassert(c != NULL,
	      "pc_to_lsf: Couldn't calloc memory for c");

    c[0] = 1.0;
    for (i = 1; i <= order; i++)
	c[i] = -pc[i - 1];
    c[order + 1] = 0.0;

    /* Form e and o polynomials */

    e_filter[0] = o_filter[0] = 1.0;
    if (order % 2 == 0)
    {
/*
	Order = even
	E(z) = [C(z) + z^-(order+1) C(1/z)] / (1 + 1/z)
	O(z) = [C(z) - z^-(order+1) C(1/z)] / (1 - 1/z)
	Note that both the polynomials are even.  Both of same order.
*/
	for (i = 1; i <= order + 1; i++)
	{
	    e_filter[i] = c[i] + c[order + 1 - i];
	    o_filter[i] = c[i] - c[order + 1 - i];
	    e_filter[i] = e_filter[i] - e_filter[i - 1];
	    o_filter[i] = o_filter[i] + o_filter[i - 1];
	}
    }
    else
    {
/*
	Order = odd
	E(z) = C(z) + z^-(order+1) C(1/z)
	O(z) = [C(z) - z^-(order+1) C(1/z)] / (1 - z^-2)
	Note that both E(z) and O(z) are even.  Order of E(z) is one degree
	higher than O(z).
*/
	for (i = 1; i <= order + 1; i++)
	{
	    e_filter[i] = c[i] + c[order + 1 - i];
	    o_filter[i] = c[i] - c[order + 1 - i];
	}

	for (i = 2; i <= order + 1; i++)
	    o_filter[i] = o_filter[i] + o_filter[i - 2];
    }

/*
As the polynomials are even, we can use cosine transforms to obtain
frequency response.
*/
    j = e_order;
    e_filter[0] = e_filter[j++];
    for (i = 1; i <= e_order; i++)
	e_filter[i] = 2.0 * e_filter[j++];


    j = o_order;
    o_filter[0] = o_filter[j++];
    for (i = 1; i <= o_order; i++)
	o_filter[i] = 2.0 * o_filter[j++];

/* Obtain the frequency response of even and odd polynomial */

    get_frep (e_filter, e_order, o_filter, o_order, e_fresp, o_fresp, npts);

/*
Compute LSFs by scanning for zero-crossing locations in the two
frequency responses.  Note that these zero-crossings occur alternatively.
*/

    prev = e_fresp[0];
    ptr = &e_fresp[1];
    j = 0;
    for (i = 1; i < npts; i++)
    {
	if (*ptr == 0)
	{

	    *ptr = FLT_MIN;
	    if (prev < 0)
		*ptr = -FLT_MIN;
	}

	/* Check to make sure that consecutive values are
	   not NaN. We do this to work around machine dependencies
	   on handling NaN and (NaN * NaN). In particular,
	   the DEC Alpha makes the following "< 0" comparison True
	   when both numbers are -NaN.
	*/

	if ( !(isnan((double)prev)) || !(isnan((double)*ptr)) )
        {
	  if (prev * *ptr < 0)
	  {
	    float   freq;
	    freq = i - 1 - prev / (*ptr - prev);
	    freq = bwidth * freq / npts;
	    lsf[j] = freq;
	    j += 2;
	  }
	  prev = *ptr++;
	}
    }

    prev = o_fresp[0];
    ptr = &o_fresp[1];
    j = 1;
    for (i = 1; i < npts; i++)
    {
	if (*ptr == 0)
	{

	    *ptr = FLT_MIN;
	    if (prev < 0)
		*ptr = -FLT_MIN;
	}

	/* Check to make sure that consecutive values are
	   not NaN. We do this to work around machine dependencies
	   on handling NaN and (NaN * NaN). In particular,
	   the DEC Alpha makes the following "< 0" comparison True
	   when both numbers are -NaN.
	*/

	if ( !(isnan((double)prev)) || !(isnan((double)*ptr)) )
        {
	  if (prev * *ptr < 0)
	  {
	    float   freq;
	    freq = i - 1 - prev / (*ptr - prev);
	    freq = bwidth * freq / npts;
	    lsf[j] = freq;
	    j += 2;
	  }
	  prev = *ptr++;
	}
    }

/*
 * ALL Done
*/
free ((char *) e_filter);
free ((char *) o_filter);
free ((char *) e_fresp);
free ((char *) o_fresp);
return (0);
}

/* Compute the frequency response of a symmetric polynomial */
static void
get_frep (e_filter, e_order, o_filter, o_order, e_fresp, o_fresp, npts)
float   e_filter[], e_fresp[], o_filter[], o_fresp[];
int     e_order, o_order, npts;
{

    double  cos ();
    float   t, e_accr, o_accr, w, two_cosw, coswn, coswn1;
    int     i, j;

    spsassert(e_filter != NULL, "get_frep: e_filter not OK");
    spsassert(e_fresp != NULL, "get_frep: e_fresp not OK");
    spsassert(o_filter != NULL, "get_frep: o_filter not OK");
    spsassert(o_fresp != NULL, "get_frep: o_fresp not OK");

    w = 3.141592654 / npts;
    for (j = 0; j <= npts; j++)
    {
	e_accr = e_filter[0];
	o_accr = o_filter[0];
	coswn = cos ((double)(j * w));
	coswn1 = 1.0;
	two_cosw = 2.0 * coswn;
	for (i = 1; i <= o_order; i++)
	{
	    e_accr += coswn * e_filter[i];
	    o_accr += coswn * o_filter[i];
	    t = two_cosw * coswn - coswn1;
	    coswn1 = coswn;
	    coswn = t;
	}
	o_fresp[j] = o_accr;
	if (e_order == o_order)
	    e_fresp[j] = e_accr;
	else
	    e_fresp[j] = e_accr + coswn * e_filter[e_order];
    }
}
