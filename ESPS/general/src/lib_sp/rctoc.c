/*
 * This material contains proprietary software of Entropic Speech, Inc. Any
 * reproduction, distribution, or publication without the prior written
 * permission of Entropic Speech, Inc. is strictly prohibited. Any public
 * distribution of copies of this work authorized in writing by Entropic
 * Speech, Inc. must bear the notice 
 *
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved." 
 *
 */

/* convert reflection coefficients to filter coefficients */
/* this version by Rod Johnson; temp array not used*/

#ifndef lint
static char *sccs_id = "@(#)rctoc.c	1.2	10/20/87 ESI";
#endif

void
rctoc(rc, order, c, pgain)
    float   *rc, *c, *pgain;
    int	    order;
{
    double  tmp, ctmp;
    double  gain = 1.0;
    int	    i, j;

    for (i = 1; i <= order; i++)
    {
	tmp = rc[i];
	gain = gain * (1.0 - tmp*tmp);

	for (j = 1; j <= i/2; j++)
	{
	    ctmp = c[j] - tmp * c[i-j];
	    c[i-j] = c[i-j] - tmp * c[j];
	    c[j] = ctmp;
	}
	c[i] = tmp;
    }
    *pgain = gain;
}

