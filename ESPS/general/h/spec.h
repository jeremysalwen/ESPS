/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				

  @(#)spec.h	1.7 2/20/96 ESI
*/

#ifndef spec_H
#define spec_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* spec data structure */

struct spec_data {
	long tag;		/* position tag */
	float tot_power;	/* total power */
	float *re_spec_val;	/* real part of spectral value */
	float *im_spec_val;	/* imaginary part of spectral value */
	float *frqs;		/* frequencies */
	long n_frqs;		/* number of frequencies */
	short frame_len;	/* number of samples in analysis window */
	short voiced;		/* YES for voiced frame */
};

/* declare support functions */

struct spec_data *
allo_spec_rec ARGS((struct header *hd));

int
get_spec_rec ARGS((struct spec_data *p, struct header *hd, FILE *file));

void
put_spec_rec ARGS((struct spec_data *p, struct header *hd, FILE *file));

void
print_spec_rec ARGS((struct spec_data *p, struct header *hd, FILE *file));

#ifdef __cplusplus
}
#endif

#endif /* spec_H */
