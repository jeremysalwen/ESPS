/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				
  @(#)fea.h	1.24 2/20/96 ESI
*/

#ifndef fea_H
#define fea_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


struct fea_data {
    long    tag;
    double *d_data;		/* double data */
    float  *f_data;		/* float data */
    long   *l_data;		/* long data */
    short  *s_data;		/* short data  and enum data */
    char   *b_data;		/* byte data */
    double_cplx *dc_data;	/* double complex data */
    float_cplx  *fc_data;	/* float complex data */
    long_cplx	*lc_data;	/* long complex data */
    short_cplx	*sc_data;	/* short complex data */
    byte_cplx	*bc_data;	/* byte complex data */
};

/* define Feature file subtype codes */

#define FEA_VQ	1		/* vector quant file */
#define FEA_ANA 2		/* ana file */
#define FEA_STAT 3		/* statistics file */
#define FEA_QHIST 4		/* histogram file */
#define FEA_DST	5		/* quantized distortion file */
#define FEA_2KB 6	
#define FEA_SPEC 7		/* spectrum file */
#define FEA_SD 8                /* sampled-data file */
#define FEA_FILT 9              /* filter file */

/* declare functions in feasupport module */

short
fea_encode ARGS((char *str, char *name, struct header *hd));

short
fea_encode_ci ARGS((char *str, char *name, struct header *hd));

char *
get_fea_ptr ARGS((struct fea_data *rec, char *name, struct header *hd));

char *
fea_decode ARGS((int code, char *name, struct header *hd));
/* code is specified as short in pre-ANSI style in feasupport.c;
 * therefore it is passed as int.
 */

char  **
get_fea_deriv ARGS((char *name, struct header *hd));

struct fea_data *
allo_fea_rec ARGS((struct header *hd));

void
put_fea_rec ARGS((struct fea_data *rec, struct header *hd, FILE *file));

void
set_seg_lab ARGS((struct header *hd, char **files));

void
print_fea_rec ARGS((struct fea_data *rec, struct header *hd, FILE *file));

void
print_fea_recf ARGS((struct fea_data *rec,
		     struct header *hd, FILE *file, char **fields));

short
get_fea_type ARGS((char *name, struct header *hd));

long
get_fea_siz ARGS((char *name, struct header *hd,
		  short int *rank, long int **dimen));

int
get_fea_rec ARGS((struct fea_data *rec, struct header *hd, FILE *file));

int
add_fea_fld ARGS((char *name, long int size, int rank, long int *dimen,
		  int type, char **coded, struct header *hd));
/* rank and type are specified as short in pre-ANSI style in feasupport.c;
 * therefore they are passed as int.
 */

int
copy_fea_fld ARGS((struct header *hd1, struct header *hd2, char *name));

int
del_fea_fld ARGS((char *name, struct header *hd));

int
set_fea_deriv ARGS((char *name, char **srcfield, struct header *hd));

void
copy_fea_fields ARGS((struct header *dest, struct header *src));

void
copy_fea_rec ARGS((struct fea_data *irec, struct header *ihd,
		   struct fea_data *orec, struct header *ohd,
		   char **fields, short int **trans));

int
fea_compat ARGS((struct header *ihd, struct header *ohd,
		 char **fields, short int ***trans));

void
free_fea_rec ARGS((struct fea_data *rec));


#ifdef __cplusplus
}
#endif

#endif /* fea_H */
