/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."

  @(#)filt.h	1.5 2/20/96 ESI
*/

#ifndef filt_H
#define filt_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>
 				

/* Filter Coefficient File record */

#define FDSPARES 20

struct filt_data {
	long	tag;			/* position tag */
	short	spares[FDSPARES];	/* spares */
	struct	zfunc	filt_func;	/* zfunc for the coefficients */
};


/* declare functions */

struct filt_data *
allo_filt_rec ARGS((struct header *hd));

int
get_filt_rec ARGS((struct filt_data *rec, struct header *hd, FILE *file));

void 
set_filt_type ARGS((struct header *hd));

void
put_filt_rec ARGS((struct filt_data *rec, struct header *hd, FILE *file));


#ifndef STARDENT_3000
void
print_filt_rec ARGS((struct filt_data *rec, struct header *ih, FILE *fp));
#endif

#ifdef __cplusplus
}
#endif

#endif /* filt_H */
