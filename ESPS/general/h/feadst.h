/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)feadst.h	1.3 2/20/96 ERL
 *
 * Written by:  Alan parker
 * Checked by:
 * Revised by:
 *
 * Brief description: definitions needed for fea_dst file type
 *
 */

#ifndef feadst_H
#define feadst_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/* defines for section_mthds
*/

#define SM_NONE			0
#define SM_ONE 			1
#define SM_EQUAL_LENGTH		2
#define SM_EQUAL_ENERGY		3
#define SM_EQUAL_DISTORTION	4

extern char *section_mthds[];


/* FEA_DST record structure
*/

struct feadst {
	short	*data_sect_mthd;	/* input data sectioning method */
	short	*data_sect_num;		/* number of input sections */
	short	*cbk_sect_mthd;		/* cbk design sectioning method */
	short	*cbk_sect_num;		/* number of cbk sections */
	char 	*quant_field;		/* field quantized */
	char 	*quant_field_rep;	/* type of field quantized */
	short	*cbk_struct;		/* codebook struct */
	short	*cbk_type;		/* codebook type */
        float	*cbk_train_dst;	        /* final design distortion(s) */
 	short	*dsgn_dst;		/* codebook design distortion */
	short	*encde_dst;		/* quantization distortion */
	short	*cbk_sect_size;		/* size of each section cbk */
	char 	*cbk_name;		/* name of input cbk */
	long	*cbk_rec;		/* cbk record number */
	char	*data_name;		/* name of input file */
	char	*cbk_source;		/* name of codebook's source */
	char	*source_type;		/* type of source */
	char	*cbk_signal;		/* name of codebook's signal */
        char	*input_source;	        /* name of input source */
        char	*input_signal;	        /* name of input signal */
        short	*in_rep_number;	        /* repetition # of input */
 	float	*data_sect_dst;		/* quantization distortion per sect */
	short	*data_sect_size;	/* num of input vectors per sect */
	float	*average_dst;		/* total average distortion */
	struct fea_data *fea_rec;	/* ptr to corresponding fea rec */
};

int
init_feadst_hd ARGS((struct header *hd, long int max_num_sects));

int
get_feadst_rec ARGS((struct feadst *dst_rec, struct header *hd, FILE *file));

void
put_feadst_rec ARGS((struct feadst *dst_rec, struct header *hd, FILE *file));

struct feadst *
allo_feadst_rec ARGS((struct header *hd));


#ifdef __cplusplus
}
#endif

#endif /* feadst_H */
