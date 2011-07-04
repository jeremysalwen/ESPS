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
|  classify.c								|
|  									|
|  Rodney Johnson							|
|  									|
|  Maximum-likelihood or maximum-posterior-probability classification	|
|  of feature vector							|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)classify.c	1.4 10/19/93 EPI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/constants.h>


double  exp(), sqrt();
#ifndef DEC_ALPHA
char	*calloc();
void	free();
#endif
double	determ();

int
classify(feavec, nfea, nclas, means, invcovars, priors, posteriors)
    float   *feavec;
    int	    nfea, nclas;
    float   **means;
    float   ***invcovars;
    float   *priors;
    float   *posteriors;
{
    int	    c, cmax;
    double  sum, max;
    double  *vec = (double *) calloc((unsigned) nfea, sizeof (double));
    spsassert(vec != NULL, "classify: calloc failed!");

#ifdef UE
    spsassert(feavec != NULL, "classify: feavec is NULL");
    spsassert(means != NULL, "classify: means is NULL");
    spsassert(posteriors != NULL, "classify: posteriors is NULL");
#endif

    sum = 0;
    max = -1.0;
    for (c = 0; c < nclas; c++)
    {
	int	i, j;
	double	likelihood = 0.0, d1;

	for (i = 0; i < nfea; i++)
	    vec[i] = feavec[i] - means[c][i];

	for (i = 0; i < nfea; i++)
	{
	    double  s = 0.0;

	    for (j = 0; j < nfea; j++)
		s += invcovars[c][i][j]*vec[j];
	    likelihood += vec[i]*s;
	}

	if((d1=determ(invcovars[c], nfea))==0)
	  fprintf(stderr,"WARNING: classify(): class %d inverse covariance has 0 determinant\n",c);
	likelihood = sqrt(d1) * exp(-0.5*likelihood);

	posteriors[c] = (priors) ? priors[c]*likelihood : likelihood;
	if (posteriors[c] > max)
	    max = posteriors[cmax = c];
	sum += posteriors[c];
    }

    free((char *) vec);

    for (c = 0; c < nclas; c++)
	posteriors[c] /= sum;

    return cmax;
}
