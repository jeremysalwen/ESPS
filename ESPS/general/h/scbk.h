/* record structure for scbk SPS file 

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				

  @(#)scbk.h	1.5 2/20/96 EPI 
*/

#ifndef scbk_H
#define scbk_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

struct qtable_entry {
	float enc;		/* upper limit on encoding range */
	float dec;		/* value to decode */
	unsigned short code;	/* bit pattern */
};

struct scbk_data {
	float final_dist;	/* final distortion */
	float *cdwd_dist;	/* individual codeword distortions */
	long *final_pop;	/* codeword populations */
	struct qtable_entry *qtable;	/* codewords */
};

/* function declarations */

struct scbk_data *
allo_scbk_rec ARGS((struct header *hd));

int
get_scbk_rec ARGS((struct scbk_data *p, struct header *hd, FILE *file));

void 
set_scbk_type ARGS((struct header *hd));

void
put_scbk_rec ARGS((struct scbk_data *p, struct header *hd, FILE *file));

void
print_scbk_rec ARGS((struct scbk_data *p, struct header *hd, FILE *file));


#ifdef __cplusplus
}
#endif

#endif /* scbk_H */
