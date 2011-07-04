/* include file for feature file subtype VQ
#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987-1990 Entropic Speech, Inc.
#       Copyright (c) 1990-1995 Entropic Research Laboratroy, Inc.
#                     All rights reserved."
#
  		@(#)vq.h	1.8 2/20/96 ESI
*/

#ifndef vq_H
#define vq_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 *defines for vqdesign parameter init
 */

#define INIT_CLUSTER 	0
#define INIT_NOCLUSTER	1
extern char *vq_init[];

/*
 *defines for vqdesign parameter split_crit
 */

#define SPLIT_POP	0
#define SPLIT_DIST	1

/*
 *defines for vqdesign return values
 */
#define VQ_OK		0
#define VQ_NOCONVG	1	
#define VQ_GC_ERR	2
#define VQ_VECWD_ERR	3
#define VQ_DIST_ERR	4
#define VQ_SPLIT_ERR	5
#define VQ_INPUT_ERR	6
extern char *vq_returns[];

/*
 *defines for distortion types
 */
#define MSE		1
#define MSE_LAR		2
#define IS		3
#define GOIS		4
#define GNIS		5
extern char *dist_types[];

/*
 *defines for the cbk_struct
 */
#define FULL_SEARCH	1
#define BINARY_TREE	2
extern char *cbk_structs[];

/*
 *defines for codebook type
 */
#define MISC		0
#define	RC_VQCBK	1
#define RC_VCD_VQCBK	2
#define RC_UNVCD_VQCBK	3
#define LSF_VQCBK	4
#define LSF_VCD_VQCBK	5
#define LSF_UNVCD_VQCBK	6
#define LM_VQCBK	7
#define LM_VCD_VQCBK	8
#define LM_UNVCD_VQCBK	9
#define LAR_VQCBK	10
#define LAR_VCD_VQCBK	11
#define LAR_UNVCD_VQCBK	12
#define CEP_VQCBK	13
#define CEP_VCD_VQCBK	14
#define CEP_UNVCD_VQCBK	15
#define	VOICED_VQCBK	16
#define UNVOICED_VQCBK	17
extern char *vq_cbk_types[];

/*
 *defines for types of checkpoints
 */
#define CHK_SPLIT	0	/*checkpoint right before split*/
#define CHK_ENCODE	2	/*checkpoint right after
				  training sequence encoded*/

/*structure definition for VQ full search codebooks*/
struct vqcbk {
	double	*conv_ratio;	/*fractional distortion convergence threshold*/
	double	*final_dist;	/*average distortion of current codebook*/
	float	**codebook;	/*codeword matrix; codeword[j] points 
				  to jth codeword*/
	float	*clusterdist;	/*distortion of cluster corresponding 
				  to each codeword*/
	long	*clustersize;	/*size of the cluster corresponding to 
				  each codeword*/
	long	*train_length;	/*length of training sequence used*/
	long	*design_size;	/*design goal for number of codewords*/
	long	*current_size;	/*current number of codewords in codebook*/
	long	*dimen;		/*dimension of codewords*/
	long	*num_iter;	/*total number of cluster iterations*/
	long	*num_empty;	/*total number of times empty cell found*/
	short	*cbk_type;	/*codebook type, e.g, RC_VQCBK;
				  see definitions above */
	short	*dist_type;	/*distortion type, e.g, MSE; see
				  definitions above */
	short	*cbk_struct;	/*codebook structure, e.g., FULL_SEARCH;
				  see definitions above */
	char	*field_rep;	/*spec_param value in FEA_ANA files,
				  if appropriate */
	char 	*field;		/*field represented by codebook */
	char 	*source_name;	/*name of the signal's source */
	char 	*signal_name;	/*type of signal */
	char	*source_type;	/*subcategory of source */
	struct fea_data *fea_rec;/*pointer to corresponding FEA record*/
};

/*The items in the vqcbk structures have the following sizes:
 *
 *	Item		Size
 *
 *	conv_ratio	scalar
 *	final_dist	scalar
 *	codebook	matrix (design_size rows by dimen columns)
 *	clusterdist	vector (length design_size)
 *	clustersize	vector (length design_size)
 *	train_length	scalar
 *	design_size	scalar
 *	current_size	scalar
 *	dimen		scalar
 *	num_iter	scalar
 *	num_empty	scalar
 *	cbk_type	scalar
 *	dist_type	scalar
 *	cbk_struct	scalar
 *	field_rep	character 16
 *	field		character 16
 *	source_name	character 16
 *	signal_name	character 32
 *	source_type	character 16
 *	
 *
 *The pointer fea_rec does not correspond to actual data that is part
 *of the vqfea record.  
 */

/* declare support functions
*/

int
init_vqfea_hd ARGS((struct header *hd, long int rows, long int cols));

struct vqcbk *
allo_vqfea_rec ARGS((struct header *hd));

void
put_vqfea_rec ARGS((struct vqcbk *cdbk, struct header *hd, FILE *strm));

int
get_vqfea_rec ARGS((struct vqcbk *cdbk, struct header *hd, FILE *strm));


#ifdef __cplusplus
}
#endif

#endif /* vq_H */
