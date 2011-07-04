/* include file for feature file subtype STAT (statistic feature file)

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                 All rights reserved."

  @(#)feastat.h	1.6	2/20/96	EPI
*/

#ifndef feastat_H
#define feastat_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/*
 * structure definition for FEA_STAT records 
 */

struct feastat
{
    short	*class;		/* classification */
    long	*nsamp;		/* number of sample vectors averaged */
    float	*mean;		/* the mean vector */
    float	**covar;	/* covariance matrix */
    float	**invcovar;	/* inverse covariance matrix */
    float	*eigenval;	/* eigenvalue array */
    float	**eigenvec;	/* eigenvector matrix */

    struct fea_data
		*fea_rec;	/* pointer to corresponding FEA record */
};

/*
 * The items in the statistic structure have the following shapes (dimension
 * information refers to generic header items):
 *
 *	Item		Shape	Dimensions
 *
 *	class		scalar
 *	nsamp		scalar
 *	mean		vector	statsize
 *	covar		matrix	{statsize, statsize}
 *	invcovar	matrix	{statsize, statsize}
 *	eigenval	vector	statsize
 *	eigenvec	matrix	{statsize, statsize}
 *
 * The pointer fea_rec does not correspond to actual data that is
 * part of the feastat record.  
 */

/*
 * declarations for support functions
 */

int
init_feastat_hd ARGS((struct header *hd, char *statfield, long int statsize,
		      char **class_type, int covar, int invcovar, int eigen));
/* covar, invcovar, and eigen are specified as short in pre-ANSI style
 * in feastatsupp.c; therefore they are passed as int.
 */

struct feastat *
allo_feastat_rec ARGS((struct header *hd));

int
get_feastat_rec ARGS((struct feastat *stat_rec,
		      struct header *hd, FILE *file));

void
put_feastat_rec ARGS((struct feastat *stat_rec,
		      struct header *hd, FILE *file));


#ifdef __cplusplus
}
#endif

#endif /* feastat_H */
