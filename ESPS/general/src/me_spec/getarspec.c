/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved" |
|									|
/*----------------------------------------------------------------------+
|									|
|  get_arspec -- compute max-entropy spectrum from residual power and	|
|		  filter coefficients					|
|									|
|  by Shankar Narayan, EPI						|
|									|
|  Module:  getarspec.c							|
|									|
|  Accepts an array <zero> of analysis filter coefficients of given	|
|  order <order>, a residual power <res_power>, a sampling frequency	|
|  <sampfreq>, and a number <num_points> of frequencies.  Computes an	|
|  array <outbfr> of (log) spectral power densities in db at		|
|  (1 + <num_points>/2) frequencies starting with 0.0 and equispaced	|
|  at intervals of <sampfreq>/<num_points>.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)getarspec.c	1.2	12/13/88	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/constants.h>

double  sin(), cos(), log10();


void
get_arspec (zero, order, res_power, sampfreq, outbfr, num_points)
float   zero[], res_power, outbfr[];
int     order, num_points;
double	sampfreq;
{
    register float  nr, ni;
    static int	    first = 1;
    static float    *sintab, *costab;
    double  w;
    double  bias;
    int     i, j, ij;

    if (first != num_points)
    {
	if (sintab == NULL)
	    sintab = (float *) malloc((unsigned) num_points * sizeof *sintab);
	else
	    sintab = (float *)
			realloc((char *) sintab,
				(unsigned) num_points * sizeof *sintab);
	if (sintab == NULL)
	{
	    Fprintf (stderr, "getarspec: couldn't allocate dynamic memory for array - sintab\n");
	    exit (1);
	}
	if (costab == NULL)
	    costab = (float *) malloc((unsigned) num_points * sizeof *costab);
	else
	    costab = (float *)
			realloc((char *) costab,
				(unsigned) num_points * sizeof *costab);
	if (costab == NULL)
	{
	    Fprintf (stderr, "getarspec: couldn't allocate dynamic memory for array - costab\n");
	    exit (1);
	}
	for (i = 0; i < num_points; i++)
	{
	    w = (2*PI) * i / num_points;
	    costab[i] = cos (w);
	    sintab[i] = sin (w);
	}
	first = num_points;
    }
    bias = 2.0 * res_power / sampfreq;
    bias = 10.0 * log10 (bias);

    for (i = 0; i <= num_points / 2; i++)
    {
	nr = 1.0;
	ni = 0.0;
	ij = i;
	for (j = 1; j <= order; j++)
	{
	    nr -= zero[j] * costab[ij];
	    ni -= zero[j] * sintab[ij];
	    ij += i;
	    if (ij >= num_points)
		ij -= num_points;
	}
	nr = nr * nr + ni * ni;
	if (nr == 0.0)
	    nr = 10.0 * log10(FLT_MAX);
	else
	    nr = -10.0 * log10(nr);
	outbfr[i] = nr + bias;
    }
}
