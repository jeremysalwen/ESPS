/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by: David Burton, Rod Johnson
 *
 * Brief description: support for fea_dst file type
 *
 */

#ifndef lint
static char *sccs_id = "@(#)feadstsupp.c	1.3 12/18/96 ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feadst.h>
#include <esps/vq.h>

#ifndef DEC_ALPHA
char *calloc();
#endif

/*
 * string array definitions (others used by this file type are in
 * vqsupport.c
 */

char *section_mthds[] = {"NONE", "SM_ONE", "SM_EQUAL_LENGTH", 
	"SM_EQUAL_ENERGY", NULL};

/* 
 * macro to fill in feature fields 
 */

#define ADDFLD( name, size, rank, dimen, type, enums ) \
{ \
    if ( add_fea_fld( (name), (long)(size), (rank), (long *)(dimen), \
      type, (char **)(enums), hd ) == -1 ) \
        return( 1 ); \
}

/*
 * This function creates a feadst header.  It creates one generic header
 * item, and a number of data record fields.
*/
int
init_feadst_hd(hd, max_num_sects)
struct header *hd;
long max_num_sects;
{
	spsassert(hd, "init_feadst_hd: hd is NULL");
	spsassert(hd->common.type == FT_FEA, "init_feadst_hd: hd not FEA");
	spsassert(max_num_sects > 0, "init_feadst_hd: max_num_sects <= 0");

	*add_genhd_l("max_num_sects",NULL,1,hd) = max_num_sects;

	ADDFLD("data_sect_mthd",  1,             0, NULL, CODED, section_mthds)
	ADDFLD("data_sect_num",   1,             0, NULL, SHORT, NULL)
	ADDFLD("cbk_sect_mthd",   1,             0, NULL, CODED, section_mthds)
	ADDFLD("cbk_sect_num",    1,             0, NULL, SHORT, NULL)
	ADDFLD("quant_field",     16,            1, NULL, CHAR,  NULL)
	ADDFLD("quant_field_rep", 16,            1, NULL, CHAR,  NULL)
	ADDFLD("cbk_struct",      1,             0, NULL, CODED, cbk_structs)
	ADDFLD("cbk_type",        1,             0, NULL, CODED, vq_cbk_types)
	ADDFLD("cbk_train_dst",	  max_num_sects, 1, NULL, FLOAT, NULL)
	ADDFLD("dsgn_dst",        1,             0, NULL, CODED, dist_types)
	ADDFLD("encde_dst",       1,             0, NULL, CODED, dist_types)
	ADDFLD("cbk_sect_size",   max_num_sects, 1, NULL, SHORT, NULL)
	ADDFLD("cbk_name",	  16,		 1, NULL, CHAR,  NULL) 
	ADDFLD("cbk_rec",	  1,		 0, NULL, LONG, NULL)
	ADDFLD("data_name",	  16,		 1, NULL, CHAR,  NULL)
	ADDFLD("cbk_source",	  32,		 1, NULL, CHAR,  NULL)
	ADDFLD("source_type",	  16,		 1, NULL, CHAR,  NULL)
	ADDFLD("cbk_signal",	  16,		 1, NULL, CHAR,  NULL)
	ADDFLD("input_source",    32,		 1, NULL, CHAR,  NULL)
	ADDFLD("input_signal",	  16,		 1, NULL, CHAR,  NULL)
	ADDFLD("in_rep_number",    1,		 0, NULL, SHORT, NULL)
	ADDFLD("data_sect_dst",	  max_num_sects, 1, NULL, FLOAT, NULL)
	ADDFLD("data_sect_size",  max_num_sects, 1, NULL, SHORT, NULL)
	ADDFLD("average_dst",	  1, 		 0, NULL, FLOAT, NULL)

	hd->hd.fea->fea_type = FEA_DST;
	return 0;
}

/*
 *This function allocates a record for the FEA file subtype FEA_DST
 */

#define GETPTR(member,type,field) \
        {dst_rec->member = (type *) get_fea_ptr(fea_rec, field, hd);}


struct feadst *
allo_feadst_rec(hd)
struct header *hd;
{
	struct fea_data *fea_rec;
	struct feadst *dst_rec;

	spsassert(hd, "allo_feadst_rec: hd is NULL");
	spsassert(hd->common.type == FT_FEA, "allo_feadst_rec: hd not FT_FEA");
	spsassert(hd->hd.fea->fea_type == FEA_DST, 
		"allo_feadst_rec: hd not FEA_DST");

	dst_rec = (struct feadst *)calloc(1, sizeof (struct feadst));
	spsassert(dst_rec, "allo_feadst_rec: calloc failed!");
	dst_rec->fea_rec = fea_rec = allo_fea_rec(hd);
	spsassert(fea_rec, "allo_feadst_rec: allo_fea_rec failed!");

	GETPTR(data_sect_mthd,	short,	"data_sect_mthd")
	GETPTR(data_sect_num,	short,	"data_sect_num")
	GETPTR(cbk_sect_mthd,	short,	"cbk_sect_mthd")
	GETPTR(cbk_sect_num,	short,	"cbk_sect_num")
	GETPTR(quant_field,	char,	"quant_field")
	GETPTR(quant_field_rep,	char,	"quant_field_rep")
	GETPTR(cbk_struct,	short,	"cbk_struct")
	GETPTR(cbk_type,	short,	"cbk_type")
	GETPTR(cbk_train_dst,   float,  "cbk_train_dst")
	GETPTR(dsgn_dst,	short,	"dsgn_dst")
	GETPTR(encde_dst,	short,	"encde_dst")
	GETPTR(cbk_sect_size,	short,	"cbk_sect_size")
	GETPTR(cbk_name,	char,	"cbk_name")
	GETPTR(cbk_rec,		long,	"cbk_rec")
	GETPTR(data_name,	char,	"data_name")
	GETPTR(cbk_source,	char,	"cbk_source")
	GETPTR(source_type,	char,	"source_type")
	GETPTR(cbk_signal,	char,	"cbk_signal")
        GETPTR(input_source,    char,   "input_source")
	GETPTR(input_signal,    char,   "input_signal")
	GETPTR(in_rep_number,   short,  "in_rep_number")
	GETPTR(data_sect_dst,	float,	"data_sect_dst")
	GETPTR(data_sect_size,	short,	"data_sect_size")
	GETPTR(average_dst,	float,	"average_dst")

	return dst_rec;
}

void
put_feadst_rec(dst_rec, hd, file)
struct feadst *dst_rec;
struct header *hd;
FILE *file;

{
	spsassert(dst_rec,"put_feadst_rec: dst_rec is NULL");
	spsassert(hd, "put_feadst_rec: hd is NULL");
	spsassert(hd->common.type == FT_FEA, "put_feadst_rec: hd not FT_FEA");
	spsassert(hd->hd.fea->fea_type == FEA_DST, 
		"put_feadst_rec: hd not FEA_DST");
	spsassert(file, "put_fea_dst_rec: file is NULL");

 	put_fea_rec(dst_rec->fea_rec, hd, file);
}

int
get_feadst_rec(dst_rec, hd, file)
struct feadst *dst_rec;
struct header *hd;
FILE *file;

{
	spsassert(dst_rec,"get_feadst_rec: dst_rec is NULL");
	spsassert(hd, "get_feadst_rec: hd is NULL");
	spsassert(hd->common.type == FT_FEA, "get_feadst_rec: hd not FT_FEA");
	spsassert(hd->hd.fea->fea_type == FEA_DST, 
		"get_feadst_rec: hd not FEA_DST");
	spsassert(file, "get_fea_dst_rec: file is NULL");
	
	return get_fea_rec(dst_rec->fea_rec, hd, file);
}
