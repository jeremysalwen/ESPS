/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 * "Copyright (c) 1987, 1988 Entropic Speech, Inc. All rights reserved."
 *
 * Program: feastatsupp.c	
 *
 * Written by:  Ajaipal S. Virdy
 * Checked by:  Alan Parker
 *
 * This program contains support routines for FEA file subtype FEA_STAT
 */

#ifndef lint
	static char *sccs_id = "@(#)feastatsupp.c	1.1	10/19/93	EPI";
#endif


/*
 * include files
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feastat.h>

/*
 * external system functions
 */

#ifndef DEC_ALPHA
char	*calloc();
int	strlen();
#endif

/*
 * external SPS functions
 */

struct fea_data	*allo_fea_rec();
char		*marg_index();

/*
 * string array definitions:  NONE.
 */

/*
 * This function fills in the header of a FEA file to make it a
 * file of subtype STAT.
 */

#define ADDFLD(name, size, rank, dimen, type, enums) \
        {if (add_fea_fld((name), (long)(size), (rank), (long*)(dimen), \
                type, (char**)(enums), hd) == -1) return 1;}

int
init_feastat_hd(hd, statfield, statsize, class_type, covar, invcovar, eigen)
    struct header   *hd;
    char    *statfield;
    long    statsize;
    char    **class_type;
    short   covar, invcovar, eigen;
{
    long    *dimens;

    spsassert(hd != NULL, "init_feastat_hd: hd is NULL");
    dimens = (long *) calloc(2, sizeof(long));
    spsassert(dimens, "init_feastat_hd: calloc failed!");
    spsassert(hd->common.type == FT_FEA,
		"init_feastat_hd: called on non fea header");

    /*
     * first, put in the generic header items
     */

    (void) add_genhd_c("statfield", statfield, 0, hd);
    *add_genhd_l("statsize", (long *) NULL, 1, hd) = statsize;

    /*
     * Now define the record fields:
     */

    ADDFLD("class", 1, 0, NULL, CODED, class_type)
    ADDFLD("nsamp", 1, 0, NULL, LONG, NULL)
    ADDFLD("mean", statsize, 1, NULL, FLOAT, NULL)

    dimens[0] = dimens[1] = statsize;

    if (covar)
	ADDFLD("covar", statsize * statsize, 2, dimens, FLOAT, NULL)
    if (invcovar)
	ADDFLD("invcovar", statsize * statsize, 2, dimens, FLOAT, NULL)

    if (eigen)
    {
	ADDFLD("eigenval", statsize, 1, NULL, FLOAT, NULL)
	ADDFLD("eigenvec", statsize * statsize, 2, dimens, FLOAT, NULL)
    }

    hd->hd.fea->fea_type = FEA_STAT;
    return 0;
}

/*
 * This function allocates a record for the FEA file subtype STAT
 */

#define GETPTR(member, type, field) \
        {stat_rec->member = (type *) get_fea_ptr(fea_rec, field, hd);}

struct feastat *
allo_feastat_rec(hd)
    struct header   *hd;
{
    struct fea_data	*fea_rec;
    struct feastat	*stat_rec;
    long		dim[2];
    char		*fp;

    spsassert(hd, "allo_feastat_rec: given a NULL hd");
    spsassert((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_STAT),
		"allo_feastat_rec: called on non fea_stat header");

    stat_rec = (struct feastat *) calloc(1, sizeof(struct feastat));
    spsassert(stat_rec, "allo_feastat_rec: calloc failed!");
    stat_rec->fea_rec = fea_rec = allo_fea_rec(hd);

    GETPTR(class, short, "class")
    GETPTR(nsamp, long, "nsamp")
    GETPTR(mean, float, "mean")

    /* Now take care of the matrices; we allocate separate storage as
     * we want to address it as a matrix.  Perhaps later the pointers
     * can be hooked up in a portable way.
     */

    dim[0] = dim[1] = * (long *) get_genhd("statsize", hd);

    fp = get_fea_ptr(fea_rec, "covar", hd);
    if (fp != NULL)
	stat_rec->covar = (float **) marg_index(fp, 2, dim, FLOAT, 0);

    fp = get_fea_ptr(fea_rec, "invcovar", hd);
    if (fp != NULL)
	stat_rec->invcovar = (float **) marg_index(fp, 2, dim, FLOAT, 0);

    fp = get_fea_ptr(fea_rec, "eigenvec", hd);
    if (fp != NULL)
    {
	GETPTR(eigenval, float, "eigenval")
	stat_rec->eigenvec = (float **) marg_index(fp, 2, dim, FLOAT, 0);
    }

    return stat_rec;
}

/*
 * This routine writes a record of the FEA file subtype STAT
 */

void
put_feastat_rec(stat_rec, hd, file)
    struct feastat  *stat_rec;
    struct header   *hd;
    FILE            *file;
{
    spsassert(hd && file && (hd->common.type == FT_FEA)
		&& (hd->hd.fea->fea_type == FEA_STAT),
		"put_feastat_rec: called on non fea_stat file");

    put_fea_rec(stat_rec->fea_rec, hd, file);
}

/*
 * This routine reads a record of the FEA file subtype STAT
 */

int
get_feastat_rec(stat_rec, hd, file)
    struct feastat  *stat_rec;
    struct header   *hd;
    FILE            *file;
{
    spsassert(hd && file && (hd->common.type == FT_FEA)
		&& (hd->hd.fea->fea_type == FEA_STAT),
		"get_feastat_rec: called on non fea_stat file");

    return get_fea_rec(stat_rec->fea_rec, hd, file);
}
