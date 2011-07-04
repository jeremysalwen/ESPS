/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: vq.c	
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This program is used to vector quantize a single feature vector 
 * using a full search VQ codebook.  
 */

#ifndef lint
static char *sccs_id = "@(#)vqencode.c	1.6	8/11/91 EPI";
#endif

#include <stdio.h>
#include <assert.h>
#include <esps/esps.h>
#define BIGDIST 1E20

long
vqencode(fea, fea_dim, codebook, cbk_size, cbk_dim, dist_val, distort, dist_err)
float	*fea;		/*feature vector to be encoded*/
long	fea_dim;	/*dimension of feature vector*/
float	**codebook;	/*the full search vq codebook*/
long	cbk_size;	/*number of codewords in codebook*/
long	cbk_dim;	/*dimension of each codeword*/
double	*dist_val;	/*distortion between fea and selected codeword*/
double	(*distort)();	/*routine to compute distortions*/
int	*dist_err;	/*for passing back errors from distort*/
/*
 *This routine encodes a single feature vector with respect to a full-search
 *VQ codebook.  The codebook is passed as a matrix (e.g., one allocated 
 *by f_mat_alloc(3-SPS), and usually will be the codebook item in a 
 *vqcbk structure.  The routine returns the index of the closest codeword.
 *If distort == NULL, a mean-square-error distortion is used.  Otherwise
 *the pointer is used to call a distortion routine.  
 */
{
    long j;
    double newdist;	  
    long min_j;		/*index of minimum distortion codeword*/
    *dist_val = BIGDIST;

    if (distort == NULL) {	/*use mean-square-error distortion*/
	double diff;
	double msedist;
	long n;
	float *codeword, *fea_vec;

	spsassert(fea_dim == cbk_dim, 
		  "feature and codebook dimensions differ (MSE)");

	for(j = 0; j < cbk_size; j++) {
	    n = fea_dim;
	    msedist = 0.0;
	    fea_vec = fea;
	    codeword = codebook[j];
	    while (n-- != 0) {
		diff = *fea_vec++ - *codeword++;
		msedist += diff * diff;
	    }
	    msedist /= cbk_dim;
	    if (msedist < *dist_val) {
		*dist_val = msedist;
		min_j = j;
	    }
	}
    }
    else { /*encode using externally supplied distort function
	    */
        for(j = 0; j < cbk_size; j++) {
	    newdist = distort(fea, fea_dim, codebook[j], cbk_dim, dist_err);
	    if (newdist < *dist_val) {
		*dist_val = newdist;
		min_j = j;
	    }
	}
    }
    return min_j;
}

