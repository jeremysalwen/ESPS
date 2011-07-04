/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *
 * Program: vqcbksupport.c	
 *
 * Written by:  John Shore
 * Checked by:  Alan Parker
 *
 * This program contains support routines for FEA file subtype VQ
 */

#ifndef lint
static char *sccs_id = "@(#)vqsupport.c	1.13	12/1/93 EPI";
#endif

#include <stdio.h>
#include <assert.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>

float **f_mat_alloc();
#ifndef DEC_ALPHA
char *calloc();
#endif

char *marg_index();

/*
 *string array definitions
 */
char *dist_types[] = {"NONE", "MSE", "MSE_LAR", "IS", "GOIS", "GNIS", NULL};
char *cbk_structs[] = {"NONE", "FULL_SEARCH", "BINARY_TREE", NULL};
char *vq_cbk_types[] = {"MISC", "RC_VQCBK", "RC_VCD_VQCBK", "RC_UNVCD_VQCBK", 
	"LSF_VQCBK", "LSF_VCD_VQCBK", "LSF_UNVCD_VQCBK", 
	"LM_VQCBK", "LM_VCD_VQCBK", "LM_UNVCD_VQCBK", 
	"LAR_VQCBK", "LAR_VCD_VQCBK", "LAR_UNVCD_VQCBK", 
	"CEP_VQCBK", "CEP_VCD_VQCBK", "CEP_UNVCD_VQCBK", 
	"VOICED_VQCBK", "UNVOICED_VQCBK", NULL};
char *vq_returns[] = {"VQ_OK", "VQ_NOCONVG", "VQ_GC_ERR", "VQ_VECWD_ERR", 
	"VQ_DIST_ERR", "VQ_SPLIT_ERR", "VQ_INPUT_ERR", NULL};
char *vq_init[] = {"INIT_CLUSTER", "INIT_NOCLUSTER",  NULL};

int
init_vqfea_hd(hd, rows, cols)
struct header *hd;	/*FEA file header*/
long rows;	/*number of rows in codebook*/
long cols;	/*number of columns in codebook*/
/*
 *This function fills in the header of a FEA file to make it a
 *file of subtype VQ.
 */
{
    long *dimens;	/*for dimensions of codebook*/
    dimens = (long *)calloc(2, sizeof (long));
    dimens[0] = rows;
    dimens[1] = cols;
    assert(hd->common.type == FT_FEA);
    /*
     *first, put in the generic header items
     */
    *add_genhd_l("design_size", (long *)NULL, 1, hd) = rows;
    *add_genhd_l("codeword_dimen", (long *)NULL, 1, hd) = cols;
    /*
     *then define the record fields
     */
    if (add_fea_fld("conv_ratio",(long)1,0,(long *)NULL,
		DOUBLE,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("final_dist",(long)1,0,(long *)NULL,
		DOUBLE,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("train_length",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("design_size",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("current_size",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("dimen",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("num_iter",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("num_empty",(long)1,0,(long *)NULL,
		LONG,(char **)NULL,hd) == -1) return 1;

    if (add_fea_fld("cbk_struct",(long)1,0,(long *)NULL,
		CODED,cbk_structs,hd) == -1) return 1;

    if (add_fea_fld("dist_type",(long)1,0,(long *)NULL,
		CODED,dist_types,hd) == -1) return 1;

    if (add_fea_fld("cbk_type",(long)1,0,(long *)NULL,
		CODED,vq_cbk_types,hd) == -1) return 1;

    if (add_fea_fld("codebook", (long) (rows*cols), 2, dimens,
		FLOAT, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("clustersize", (long) rows, 1, (long *)NULL, 
		LONG, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("clusterdist", (long) rows, 1, (long *)NULL, 
		FLOAT, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("field_rep", (long)16, 1, (long *)NULL, 
		CHAR, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("field", (long)16, 1, (long *)NULL, CHAR, 
		(char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("source_name", (long)32, 1, (long *)NULL, 
		CHAR, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("signal_name", (long)16, 1, (long *)NULL, 
		CHAR, (char **)NULL, hd) == -1) return 1;

    if (add_fea_fld("source_type", (long)16, 1, (long *)NULL, 
		CHAR, (char **)NULL, hd) == -1) return 1;

    hd->hd.fea->fea_type = FEA_VQ;
    return 0;
}


struct vqcbk *
allo_vqfea_rec(hd)
struct header *hd;
/*
 *This function allocates a record for the FEA file subtype VQ
 */
{
  static long dims[2];

    struct vqcbk *cdbk = (struct vqcbk *) calloc(1, sizeof(struct vqcbk));
    struct fea_data *fea_rec;
    long rows, cols;
    assert((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_VQ));
    fea_rec = allo_fea_rec(hd);
    cdbk->conv_ratio = (double *) get_fea_ptr(fea_rec, "conv_ratio", hd);
    cdbk->final_dist = (double *) get_fea_ptr(fea_rec, "final_dist", hd);
    cdbk->train_length = (long *) get_fea_ptr(fea_rec, "train_length", hd);
    cdbk->design_size = (long *) get_fea_ptr(fea_rec, "design_size", hd);
    cdbk->current_size = (long *) get_fea_ptr(fea_rec, "current_size", hd);
    cdbk->dimen = (long *) get_fea_ptr(fea_rec, "dimen", hd);
    cdbk->num_iter = (long *) get_fea_ptr(fea_rec, "num_iter", hd);
    cdbk->num_empty = (long *) get_fea_ptr(fea_rec, "num_empty", hd);
    cdbk->cbk_struct = (short *) get_fea_ptr(fea_rec, "cbk_struct", hd);
    cdbk->dist_type = (short *) get_fea_ptr(fea_rec, "dist_type", hd);
    cdbk->cbk_type = (short *) get_fea_ptr(fea_rec, "cbk_type", hd);
    cdbk->clusterdist = (float *) get_fea_ptr(fea_rec, "clusterdist", hd);
    cdbk->clustersize = (long *) get_fea_ptr(fea_rec, "clustersize", hd);
    cdbk->field_rep = (char *) get_fea_ptr(fea_rec, "field_rep", hd);
    cdbk->field = (char *) get_fea_ptr(fea_rec, "field", hd);
    cdbk->source_name = (char *) get_fea_ptr(fea_rec, "source_name", hd);
    cdbk->signal_name = (char *) get_fea_ptr(fea_rec, "signal_name", hd);
    cdbk->source_type = (char *) get_fea_ptr(fea_rec, "source_type", hd);

    /*
     *Now take care of the codebook; we use marg_index to lash up the 
     *pointers properly; this way, we don't have to copy back and forth
     * in get/put_vqfea_ref(). 
     */

    rows = *(long *)get_genhd("design_size", hd);
    cols = *(long *)get_genhd("codeword_dimen", hd);

    dims[0] = rows;
    dims[1] = cols;

    cdbk->codebook = (float **) marg_index(get_fea_ptr(fea_rec, "codebook", hd), 
					   2, dims, FLOAT, 0);
    assert(cdbk->codebook != NULL);

    /*
     *Now must keep track of which FEA record this cbk structure 
     *corresponds to (note that this component of the cbk structure
     *doesn't correspond to anything in the FEA record!)
     */
    cdbk->fea_rec = fea_rec;
    
    return cdbk;
}

void
put_vqfea_rec(cdbk, hd, strm)
struct vqcbk *cdbk;
struct header *hd;
FILE *strm;
{
/*
 *This routine writes a record of the FEA file subtype VQ
 */
    float *fp;
    long i, j;
    assert((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_VQ));

    /*
     *Since all the pointers in cdbk are hooked to the right 
     *space in the FEA record, now all we have to do is write the record.
     */

    put_fea_rec(cdbk->fea_rec, hd, strm);
}

int
get_vqfea_rec(cdbk, hd, strm)
struct vqcbk *cdbk;
struct header *hd;
FILE *strm;
{
/*
 *This routine reads a record of the FEA file subtype VQ
 */
    float *fp;
    int get_val;
    long i, j;
    assert((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_VQ));
    /*
     *read the FEA record that corresponds to the vqcbk record
     */

    get_val = get_fea_rec(cdbk->fea_rec, hd, strm);
    
    /*
     *Since the pointers are hooked up directly, we just return.
     */

    return get_val;
}

