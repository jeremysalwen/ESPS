/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				
  @(#)sd.h	1.7 2/20/96 ESI

*/

#ifndef sd_H
#define sd_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* declare the sd support functions */

int
get_sd_type ARGS((struct header *hd));

int
get_sd_recsize ARGS((struct header *hd));

int
get_sd_recd ARGS((double *dbuf, int nsmpls, struct header *hd, FILE *stream));

int
get_sd_recf ARGS((float *fbuf, int nsmpls, struct header *hd, FILE *stream));

int
get_sd_recs ARGS((short int *sbuf,
		  int nsmpls, struct header *hd, FILE *stream));

int
get_sd_orecd ARGS((double *dbuf, int framelen, int nmove,
		   int first, struct header *hd, FILE *file));

int
get_sd_orecf ARGS((float *fbuf, int framelen, int nmove,
		   int first, struct header *hd, FILE *file));

int
get_sd_orecs ARGS((short int *sbuf, int framelen, int nmove,
		   int first, struct header *hd, FILE *file));

void
put_sd_recd ARGS((double *dbuf, int nsmpls, struct header *hd, FILE *stream));

void
put_sd_recf ARGS((float *fbuf, int nsmpls, struct header *hd, FILE *stream));

void
put_sd_recs ARGS((short int *sbuf,
		  int nsmpls, struct header *hd, FILE *stream));

void
set_sd_type ARGS((struct header *hd, int type));


#ifdef __cplusplus
}
#endif

#endif /* sd_H */
