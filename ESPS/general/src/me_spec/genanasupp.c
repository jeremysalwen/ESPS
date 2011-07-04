/* This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing
 * by Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright 1988 Entropic Speech, Inc. All Rights Reserved."
 *
 * Module: genanasupp.c
 *
 * Written by:  Rodney Johnson
 *
 * This module contains support routines for general analysis FEA records
 */
#ifndef lint
    static char *sccs_id = "@(#)genanasupp.c	1.3	10/20/93	ESI";
#endif


/*
 * include files
 */

#include <stdio.h>
#include <assert.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include "genana.h"

/*
 * external system functions
 */

#ifndef DEC_ALPHA
char	*calloc();
#endif

/*
 * external functions
 */


/*
 * This function allocates a record
 */

#define GETPTR(member,type,field) \
        {ana_rec->member = (type *) get_fea_ptr(fea_rec, field, hd);}

struct genana *
allo_genana_rec(hd, fana, s_p_fld, pwr_fld)
    struct header   *hd;
    int		    fana;
    char	    *s_p_fld, *pwr_fld;
{
    struct fea_data *fea_rec;
    struct genana   *ana_rec;

    spsassert(hd, "allo_genana_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "allo_genana_rec: file not FEA");

    ana_rec = (struct genana *) calloc(1, sizeof (struct genana));
    spsassert(ana_rec, "allo_genana_rec: calloc failed on ana_rec");

    ana_rec->fea_rec = fea_rec = allo_fea_rec(hd);
    ana_rec->tag = (hd->common.tag) ? &fea_rec->tag : NULL;

    if (fana)
    {
	GETPTR(frame_len, long, "frame_len")
	GETPTR(num_pulses, long, "num_pulses")
	GETPTR(frame_type, short, "frame_type")
	GETPTR(p_pulse_len, float, "p_pulse_len")
    }
    else
    {
	ana_rec->frame_len = NULL;
	ana_rec->num_pulses = NULL;
	ana_rec->frame_type = NULL;
	ana_rec->p_pulse_len = NULL;
    }

    GETPTR(raw_power, float, pwr_fld)
    GETPTR(spec_param, float, s_p_fld)

    return ana_rec;
}

/*
 * This routine reads a record
 */

int
get_genana_rec(ana_rec, hd, file)
    struct genana   *ana_rec;
    struct header   *hd;
    FILE	    *file;
{
    spsassert(hd != NULL,"get_genana_rec: hd is NULL");
    spsassert(ana_rec != NULL,"get_genana_rec: ana_rec is NULL");
    spsassert(file != NULL,"get_genana_rec: file is NULL");
    spsassert(hd->common.type == FT_FEA, "get_genana_rec: file not FEA");

    return get_fea_rec(ana_rec->fea_rec, hd, file);
}
