/* This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 *
 * Program: anafeasupp.c	
 *
 * Written by:  John Shore
 * Checked by:  Alan Parker
 *
 * This program contains support routines for FEA file subtype FEA_ANA
 */

#ifndef lint
static char *sccs_id = "@(#)anafeasupp.c	1.9	10/19/93 ESI";
#endif


/*
 * include files
 */
#include <stdio.h>
#include <assert.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/anafea.h>

/*
 * external system functions
 */

/*
 * external functions
 */
#ifndef DEC_ALPHA
char	*calloc();
#endif

/*
 *string array definitions
 */
char *frame_types[] = {"NONE", "UNKNOWN", "VOICED", "UNVOICED", 
    "SILENCE", "TRANSITION", NULL}; 

char *spec_reps[] = {"NONE", "RC", "LAR", "LSF", "AUTO", 
		     "AFC", "CEP", "AF", NULL};

char *noyes[] = {"NO", "YES", NULL};

/*
 *This function fills in the header of a FEA file to make it a
 *file of subtype ANA.
 */
#define ADDFLD(name,size,rank,dimen,type,enums) \
        {if (add_fea_fld((name),(long)(size),(rank),(long*)(dimen), \
                type,(char**)(enums),hd) == -1) return 1;}

int
init_anafea_hd(hd, order_vcd, order_unvcd, maxpulses, maxraw, 
	maxlpc, filt_nsiz, filt_dsiz)
struct	header	*hd;	/*FEA file header*/
long	order_vcd;	/*order of unvoiced frames*/
long	order_unvcd;	/*order of voiced and transition frames*/
long	maxpulses;	/*maximum number of pitch pulses*/
long	maxraw;		/*maximum number of raw powers*/
long	maxlpc;		/*maximum number of lpc error powers*/
short	filt_nsiz;	/*length of filter numerator polynomial*/
short	filt_dsiz;	/*length of filter denominator polynomial*/
{
    spsassert(hd != NULL, "init_anafea_hd: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "init_anafea_hd: file not FEA");
    /*
     *first, put in the generic header items
     */
    *add_genhd_l("order_vcd", (long *)NULL, 1, hd) = order_vcd;
    *add_genhd_l("order_unvcd", (long *)NULL, 1, hd) = order_unvcd;
    *add_genhd_l("maxpulses", (long *)NULL, 1, hd) = maxpulses;
    *add_genhd_l("maxraw", (long *)NULL, 1, hd) = maxraw;
    *add_genhd_l("maxlpc", (long *)NULL, 1, hd) = maxlpc;
    *add_genhd_l("start", (long *)NULL, 1, hd) = 0;
    *add_genhd_l("nan", (long *)NULL, 1, hd) = 0;
    *add_genhd_l("frmlen", (long *)NULL, 1, hd) = 0;
    if ((filt_nsiz == 0) && (filt_dsiz == 0))
	*add_genhd_e("filters", (short *)NULL, 1, noyes, hd) = NO;
    else 
    {
      *add_genhd_e("filters", (short *)NULL, 1, noyes, hd) = YES;  
      *add_genhd_s("filt_dsiz", (long *)NULL, 1, hd) = filt_dsiz;
      *add_genhd_s("filt_nsiz", (long *)NULL, 1, hd) = filt_nsiz;
    }
    
    /*
     *The following header items are created by this routine,
     *but should be assigned values in the calling program.
     */
    *add_genhd_e("spec_rep", (short *)NULL, 1, spec_reps, hd) = NONE;
    *add_genhd_f("src_sf", (float *)NULL, 1, hd) = 0.0;
    /*
     *then define the record fields
     */
    ADDFLD("frame_len",1,0,NULL,LONG,NULL)
    ADDFLD("num_pulses",1,0,NULL,LONG,NULL)
    ADDFLD("frame_type",1,0,NULL,CODED,frame_types)
    ADDFLD("voiced_fraction",1,0,NULL,FLOAT,NULL)
    ADDFLD("raw_power",maxraw,1,NULL,FLOAT,NULL)
    ADDFLD("lpc_power",maxlpc,1,NULL,FLOAT,NULL)
    ADDFLD("p_pulse_len",maxpulses,1,NULL,FLOAT,NULL)
    ADDFLD("spec_param",MAX(order_vcd, order_unvcd),1,NULL,FLOAT,NULL)
    /*
     *fields for (the filt_zeros and filt_poles are defined only
     *if there will be filters in the record
     */
    if ((*(short *) get_genhd("filters", hd)) == YES) {
	ADDFLD("filt_zeros",filt_nsiz,1,NULL,FLOAT,NULL)
	ADDFLD("filt_poles",filt_dsiz,1,NULL,FLOAT,NULL)	
    }
    hd->hd.fea->fea_type = FEA_ANA;
    return 0;
}

/*
 *This function allocates a record for the FEA file subtype ANA
 */

#define GETPTR(member,type,field) \
        {ana_rec->member = (type *) get_fea_ptr(fea_rec, field, hd);}

struct anafea *
allo_anafea_rec(hd)
struct header *hd;
{
    struct fea_data *fea_rec;
    struct anafea *ana_rec;
    spsassert(hd != NULL,"allo_anafea_rec: hd is NULL");
    spsassert((hd->common.type == FT_FEA)
        && (hd->hd.fea->fea_type == FEA_ANA), 
	"allo_anafea_rec: file not FEA or not FEA_ANA");
    ana_rec = (struct anafea *) calloc(1, sizeof (struct anafea));
    spsassert(ana_rec != NULL,"allo_anafea_rec: calloc failed on ana_rec");
    ana_rec->fea_rec = fea_rec = allo_fea_rec(hd);
    if (hd->common.tag)
	ana_rec->tag = &fea_rec->tag;
    else
	ana_rec->tag = NULL;
    GETPTR(frame_len, long, "frame_len")
    GETPTR(num_pulses, long, "num_pulses")
    GETPTR(frame_type, short, "frame_type")
    GETPTR(voiced_fraction, float, "voiced_fraction")
    GETPTR(raw_power, float, "raw_power")
    GETPTR(lpc_power, float, "lpc_power")
    GETPTR(p_pulse_len, float, "p_pulse_len")
    GETPTR(spec_param, float, "spec_param")
    /*
     *only allocate stuff for filters if header says they are included
     */
    if ((*(short *) get_genhd("filters", hd)) == YES) {
	GETPTR(filt_zeros, float, "filt_zeros")
	GETPTR(filt_poles, float, "filt_poles")
    }
    else {
	ana_rec->filt_zeros = NULL;
	ana_rec->filt_poles = NULL;
    }
    return ana_rec;
}

/*
 *This routine writes a record of the FEA file subtype ANA
 */

void
put_anafea_rec(ana_rec, hd, file)
    struct anafea  *ana_rec;
    struct header   *hd;
    FILE            *file;
{
    spsassert(hd != NULL,"put_anafea_rec: hd is NULL");
    spsassert(ana_rec != NULL,"put_anafea_rec: ana_rec is NULL");
    spsassert(file != NULL,"put_anafea_rec: file is NULL");
    spsassert((hd->common.type == FT_FEA)
        && (hd->hd.fea->fea_type == FEA_ANA),
	"put_anafea_rec: file not FEA or not FEA_ANA");
    put_fea_rec(ana_rec->fea_rec, hd, file);
}

/*
 *This routine reads a record of the FEA file subtype ANA
 */
int
get_anafea_rec(ana_rec, hd, file)
    struct anafea  *ana_rec;
    struct header   *hd;
    FILE            *file;
{
    spsassert(hd != NULL,"get_anafea_rec: hd is NULL");
    spsassert(ana_rec != NULL,"get_anafea_rec: ana_rec is NULL");
    spsassert(file != NULL,"get_anafea_rec: file is NULL");
    spsassert((hd->common.type == FT_FEA)
        && (hd->hd.fea->fea_type == FEA_ANA),
	"get_anafea_rec: file not FEA or not FEA_ANA");
    return get_fea_rec(ana_rec->fea_rec, hd, file);
}
