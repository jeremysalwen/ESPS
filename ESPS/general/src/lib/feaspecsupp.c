/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright 1988 Entropic Speech, Inc. All Rights Reserved."
 *
 * Program: feaspecsupp.c
 *
 * Written by:  Rodney Johnson
 * Checked by:  
 *
 * This program contains support routines for FEA file subtype FEA_SPEC
 */

#ifndef lint
    static char *sccs_id = "@(#)feaspecsupp.c	1.6	7/2/90	 ESI";
#endif

/*
 * Include files.
 */

#include <stdio.h>
#include <esps/spsassert.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/fea.h>
#include <esps/feaspec.h> 


char *savestring();
char **get_feaspec_xfields();
void free_feaspec_xfields();

/*
 * String array definitions.
 * Keep consistent with constant symbols in esps/feaspec.h.
 */

char	*spfmt_names[] = {"NONE", "SYM_CTR", "SYM_EDGE", "ASYM_CTR",
			    "ASYM_EDGE", "ARB_VAR", "ARB_FIXED", NULL};

char	*sptyp_names[] = {"NONE", "PWR", "DB", "REAL", "CPLX",  NULL};

char	*spfrm_names[] = {"NONE", "FIXED", "VARIABLE", NULL};


/*
 * This function fills in the header of a FEA file to make it a
 * file of subtype FEA_SPEC.
 */

#define ADDFLD(name,size,rank,dimen,type,enums) \
        {if (add_fea_fld((name),(long)(size),(rank),(long*)(dimen), \
                type,(char**)(enums),hd) == -1) return 1;}

int
init_feaspec_hd(hd, def_tot_power, freq_format, spec_type,
		contin, num_freqs, frame_meth, freqs, sf, frmlen, 
		re_spec_format)
    struct header   *hd;	/* FEA file header*/
    int		    def_tot_power;  /* Define tot_power field? */
    int		    freq_format;    /* How set of frequencies defined. */
    int		    spec_type;	/* Are data log power, complex, etc.? */
    int		    contin;	/* Continuous or discrete spectrum? */
    long	    num_freqs;	/* Number of frequencies. */
    int		    frame_meth;	/* How frame length is determined. */
    float	    *freqs;	/* Frequencies (if given in header). */
    double	    sf;		/* Sampling frequency. */
    long	    frmlen;	/* Number of points in analysis frame. */
    int	    	    re_spec_format; /* FLOAT or BYTE */
{
    static char	*no_yes[] = {"NO", "YES", NULL};

    spsassert(hd, "init_feaspec_hd: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "init_feaspec_hd: file not FEA");
    if (spec_type == SPTYP_DB)
    	spsassert(re_spec_format == BYTE || re_spec_format == FLOAT,
	 "init_feaspec_hd: re_spec_format not BYTE or FLOAT");

    /*
     * First, put in the generic header items.
     */

    *add_genhd_e("freq_format", (short *) NULL, 1, spfmt_names, hd) =
	freq_format;
    *add_genhd_e("spec_type", (short *) NULL, 1, sptyp_names, hd) = spec_type;
    *add_genhd_e("contin", (short *) NULL, 1, no_yes, hd) = contin;
    *add_genhd_l("num_freqs", (long *) NULL, 1, hd) = num_freqs;
    *add_genhd_e("frame_meth", (short *) NULL, 1, spfrm_names, hd) = frame_meth;
    if (freq_format == SPFMT_ARB_FIXED)
    {
	float	*ptr;
	int	i;

	spsassert(freqs, "init_feaspec_hd: vector of freqs is NULL");
	ptr = add_genhd_f("freqs", (float *) NULL, (int) num_freqs, hd);
	for (i = 0; i < num_freqs; i++)
	    ptr[i] = freqs[i];
    }
    switch (freq_format)
    {
    case SPFMT_SYM_CTR:
    case SPFMT_SYM_EDGE:
    case SPFMT_ASYM_CTR:
    case SPFMT_ASYM_EDGE:
	*add_genhd_f("sf", (float *) NULL, 1, hd) = sf;
	break;
    default:
	break;
    }
    if (frame_meth == SPFRM_FIXED)
    {
	*add_genhd_l("frmlen", (long *) NULL, 1, hd) = frmlen;
    }

    /*
     * Then define the record fields.
     */

    if (def_tot_power)
	ADDFLD("tot_power", 1, 0, NULL, FLOAT, NULL)
    if (spec_type != SPTYP_DB || 
       (spec_type == SPTYP_DB && re_spec_format == FLOAT))
    	ADDFLD("re_spec_val", num_freqs, 1, NULL, FLOAT, NULL)
    else
    	ADDFLD("re_spec_val", num_freqs, 1, NULL, BYTE, NULL)
    if (spec_type == SPTYP_CPLX)
	ADDFLD("im_spec_val", num_freqs, 1, NULL, FLOAT, NULL)
    if (freq_format == SPFMT_ARB_VAR)
    {
	ADDFLD("n_frqs", 1, 0, NULL, LONG, NULL)
	ADDFLD("frqs", num_freqs, 1, NULL, FLOAT, NULL)
    }
    if (frame_meth == SPFRM_VARIABLE)
	ADDFLD("frame_len", 1, 0, NULL, LONG, NULL)

    hd->hd.fea->fea_type = FEA_SPEC;
    return 0;
}

/*
 * This function allocates a record for the FEA file subtype FEA_SPEC.
 */

#define GETPTR(member,type,field) \
        {spec_rec->member = (type *) get_fea_ptr(fea_rec, field, hd);}

struct feaspec *
allo_feaspec_rec(hd, re_spec_format)
    struct header   *hd;
    int	    	    re_spec_format;
{
    struct fea_data *fea_rec;
    struct feaspec  *spec_rec;
    short	    *freq_format;
    long	    *num_freqs;
    short	    *frame_meth;
    float	    *freqs;
    float	    *sf;
    long	    *frmlen;
    double	    f, step;
    int		    i;
    int	    	    spec_type;


    spsassert(hd, "allo_feaspec_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA
		&& hd->hd.fea->fea_type == FEA_SPEC, 
	"allo_feaspec_rec: file not FEA or not FEA_SPEC");
    spec_type = *get_genhd_s("spec_type", hd);
    spsassert(spec_type, "allo_feaspec_rec: missing header item spec_type");
    if (spec_type == SPTYP_DB) {
    	spsassert(re_spec_format == BYTE || re_spec_format == FLOAT,
	 "allo_feaspec_rec: re_spec_format not BYTE or FLOAT");
    }
    else
	re_spec_format = FLOAT;

    spec_rec = (struct feaspec *) calloc(1, sizeof(struct feaspec));
    spsassert(spec_rec, "allo_feaspec_rec: calloc failed on spec_rec");

    spec_rec->fea_rec = fea_rec = allo_fea_rec(hd);

    if (hd->common.tag)
	spec_rec->tag = &fea_rec->tag;
    else
	spec_rec->tag = NULL;

    num_freqs = get_genhd_l("num_freqs", hd);
    spsassert(num_freqs, "allo_feaspec_rec: num_freqs undefined.")
    spec_rec->re_spec_val_b = NULL;
    spec_rec->re_spec_val = NULL;

    if (re_spec_format == FLOAT && get_fea_type("re_spec_val",hd) == FLOAT)
    	GETPTR(re_spec_val, float, "re_spec_val")
    else if (re_spec_format == BYTE && get_fea_type("re_spec_val",hd) == BYTE)
    	GETPTR(re_spec_val_b, char, "re_spec_val")
    else if (re_spec_format == BYTE) {
	spec_rec->re_spec_val_b = malloc((unsigned) *num_freqs);
	spsassert(spec_rec->re_spec_val_b,
		"allo_feaspec_rec: malloc failed on re_spec_val_b.");
    }
    else if (re_spec_format == FLOAT) {
	spec_rec->re_spec_val = malloc_f((unsigned) *num_freqs);
	spsassert(spec_rec->re_spec_val,
		"allo_feaspec_rec: malloc failed on re_spec_val.")
    }

    GETPTR(tot_power, float, "tot_power")
    GETPTR(im_spec_val, float, "im_spec_val")
    GETPTR(n_frqs, long, "n_frqs")
    GETPTR(frqs, float, "frqs")
    GETPTR(frame_len, long, "frame_len")
    if (!spec_rec->im_spec_val)
    {
	spec_rec->im_spec_val = malloc_f((unsigned) *num_freqs);
	spsassert(spec_rec->im_spec_val,
		"allo_feaspec_rec: malloc failed on im_spec_val.")
	for (i = 0; i < *num_freqs; i++) spec_rec->im_spec_val[i] = 0.0;
    }
    if (!spec_rec->frqs)
    {
	freq_format = get_genhd_s("freq_format", hd);
	spsassert(freq_format, "allo_feaspec_rec: freq_format undefined.")

	spec_rec->n_frqs = malloc_l((unsigned) 1);
	spsassert(spec_rec->n_frqs,
	    "allo_feaspec_rec: malloc failed on n_frqs.")
	*spec_rec->n_frqs = *num_freqs;

	spec_rec->frqs = malloc_f((unsigned) *num_freqs);
	spsassert(spec_rec->frqs, "allo_feaspec_rec: malloc failed on frqs.")

	switch(*freq_format)
	{
	case SPFMT_SYM_CTR:
	case SPFMT_SYM_EDGE:
	case SPFMT_ASYM_CTR:
	case SPFMT_ASYM_EDGE:
	    sf = get_genhd_f("sf", hd);
	    spsassert(sf, "allo_feaspec_rec: sf undefined.")
	    switch (*freq_format)
	    {
	    case SPFMT_SYM_CTR:
		step = *sf / (2 * *num_freqs);
		f = step/2;
		break;
	    case SPFMT_SYM_EDGE:
		step = *sf / (2 * (*num_freqs - 1));
		f = 0.0;
		break;
	    case SPFMT_ASYM_CTR:
		step = *sf / *num_freqs;
		f = (step - *sf)/2;
		break;
	    case SPFMT_ASYM_EDGE:
		step = *sf / (*num_freqs - 1);
		f = - *sf/2;
		break;
	    }
	    for (i = 0; i < *num_freqs; i++, f += step) spec_rec->frqs[i] = f;
	    break;
	case SPFMT_ARB_VAR:
	    Fprintf(stderr,
		"%s: frqs field required by freq_format but not present.\n",
		"allo_feaspec_rec");
	    exit(1);
	    break;
	case SPFMT_ARB_FIXED:
	    freqs = get_genhd_f("freqs", hd);
	    spsassert(freqs,
		"allo_feaspec_rec: header item freqs undefined.")
	    for (i = 0; i < *num_freqs; i++) spec_rec->frqs[i] = freqs[i];
	    break;
	default:
	    break;
	}
    }
    if (!spec_rec->frame_len)
    {
	frame_meth = get_genhd_s("frame_meth", hd);
	spsassert(frame_meth, "allo_feaspec_rec: frame_meth undefined.")

	switch(*frame_meth)
	{
	case SPFRM_NONE:
	    break;
	case SPFRM_FIXED:
	    frmlen = get_genhd_l("frmlen", hd);
	    spsassert(frmlen, "allo_feaspec_rec: frmlen undefined.")
	    spec_rec->frame_len = malloc_l((unsigned) 1);
	    spsassert(spec_rec->frame_len,
		"allo_feaspec_rec: malloc failed on frame_len.")
	    *spec_rec->frame_len = *frmlen;
	    break;
	case SPFRM_VARIABLE:
	    Fprintf(stderr,
		"%s: frame_len field required by frame_meth but not present.\n",
		"allo_feaspec_rec");
	    exit(1);
	    break;
	}
    }

    return spec_rec;
}

/*
 * This routine writes a record of the FEA file subtype FEA_SPEC.
 */

long
put_feaspec_rec(spec_rec, hd, file)
    struct feaspec  *spec_rec;
    struct header   *hd;
    FILE            *file;
{
    int re_spec_type = get_fea_type("re_spec_val", hd);
    long overflow = 0;


    spsassert(hd, "put_feaspec_rec: hd is NULL");
    spsassert(spec_rec, "put_feaspec_rec: spec_rec is NULL");
    spsassert(file, "put_feaspec_rec: file is NULL");
    spsassert(hd->common.type == FT_FEA
		&& hd->hd.fea->fea_type == FEA_SPEC,
	"put_feaspec_rec: file not FEA or not FEA_SPEC");

    if ((spec_rec->re_spec_val_b && re_spec_type == BYTE)
    || (spec_rec->re_spec_val && re_spec_type == FLOAT))
    	put_fea_rec(spec_rec->fea_rec, hd, file);
    else {
	long *num_freqs = get_genhd_l("num_freqs", hd);
	long i;
	spsassert(num_freqs, "put_feaspec_rec: num_freqs undefined");
	if (spec_rec->re_spec_val) { 	/* convert from float to byte */
		char *re_spec_val_b = get_fea_ptr(spec_rec->fea_rec, 
					"re_spec_val", hd);
		for (i=0; i< *num_freqs; i++) {
			if (spec_rec->re_spec_val[i]+64.5 >= 1 + CHAR_MAX) {
				re_spec_val_b[i] = CHAR_MAX;
				overflow++;
			}
			else if (spec_rec->re_spec_val[i]+64.5 < 0) {
				re_spec_val_b[i] = 0;
				overflow++;
			}
			else re_spec_val_b[i] = spec_rec->re_spec_val[i]+64.5;
		}
	}
	else {	/* convert from byte to float */
		float *re_spec_val = (float *)get_fea_ptr(spec_rec->fea_rec,
					"re_spec_val", hd);
		for (i=0; i< *num_freqs; i++) {
			re_spec_val[i] = spec_rec->re_spec_val_b[i]-64;
		}
	}
        put_fea_rec(spec_rec->fea_rec, hd, file);
    }
    return overflow;
}

/*
 * This routine reads a record of the FEA file subtype FEA_SPEC.
 */

long
get_feaspec_rec(spec_rec, hd, file)
    struct feaspec  *spec_rec;
    struct header   *hd;
    FILE            *file;
{
    int re_spec_type;
    long overflow = 0;

    spsassert(hd, "get_feaspec_rec: hd is NULL");
    spsassert(spec_rec, "get_feaspec_rec: spec_rec is NULL");
    spsassert(file, "get_feaspec_rec: file is NULL");
    spsassert(hd->common.type == FT_FEA
		&& hd->hd.fea->fea_type == FEA_SPEC,
	"get_feaspec_rec: file not FEA or not FEA_SPEC");

    re_spec_type = get_fea_type("re_spec_val", hd);
    if ((spec_rec->re_spec_val_b && re_spec_type == BYTE)
    || (spec_rec->re_spec_val && re_spec_type == FLOAT))
    	return (get_fea_rec(spec_rec->fea_rec, hd, file) == EOF) ? EOF : 0;

    else {
	long *num_freqs = get_genhd_l("num_freqs", hd);
	long i;
	spsassert(num_freqs, "put_feaspec_rec: num_freqs undefined");

	if (get_fea_rec(spec_rec->fea_rec, hd, file) == EOF) return EOF;

	if (spec_rec->re_spec_val) { 	/* convert from byte to float */
		char *re_spec_val_b = get_fea_ptr(spec_rec->fea_rec, 
					"re_spec_val", hd);
		for (i=0; i< *num_freqs; i++) {
			spec_rec->re_spec_val[i] = (int)re_spec_val_b[i] - 64;
		}

	} else {			/* convert from float to byte */
		float *re_spec_val = (float *)get_fea_ptr(spec_rec->fea_rec, 
					"re_spec_val", hd);
		for (i=0; i< *num_freqs; i++) {
			if (re_spec_val[i]+64.5 >= 1 + CHAR_MAX) {
				spec_rec->re_spec_val_b[i] = CHAR_MAX;
				overflow++;
			}
			else if (re_spec_val[i]+64.5 < 0) {
				spec_rec->re_spec_val_b[i] = 0;
				overflow++;
			}
			else spec_rec->re_spec_val_b[i] = re_spec_val[i]+64.5;
		}
        }
    }
    return overflow;
}


/* this routine prints a feaspec record (used by psps, etc) */

void
print_feaspec_rec(p, hd, file)
struct feaspec *p;
struct header *hd;
FILE *file;

{
	int	i;
	int	j;
	long	n;
	float	sf;
	double	freqmin;
	short	*freq_format = NULL;
	short	*spec_type = NULL;
	long	*num_freqs = NULL;
	char	*sep = "";
	char	**xfields;

/* check the validity of the input parameters */

	spsassert(hd != NULL, "print_feaspec_rec: hd is NULL");
	spsassert(p != NULL, "print_feaspec_rec: p is NULL");
	spsassert(file != NULL, "print_feaspec_rec: file is NULL");
	spsassert(hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SPEC,
		"print_feaspec_rec: file not FEA_SPEC");
	spsassert(get_genhd_val("num_freqs", hd, -1.0) >= 0, 
		"print_feaspec_rec: num_freqs < 0 or missing");

	if (hd->common.tag == YES) {
		(void) fprintf(file, "tag: %ld", *p->tag);
		sep = ", ";
	}
	if (p->tot_power) {
		(void) fprintf(file, "%stot_power: %g", sep, *p->tot_power);
		sep = ", ";
	}
	if (p->frame_len)
		(void) fprintf(file, "%sframe_len: %d", sep, *p->frame_len);
	(void) fprintf(file, "\n");

/* print any fields that are not part of the standard fea_spec file here */

	xfields = get_feaspec_xfields(hd);
	print_fea_recf(p->fea_rec, hd, file, xfields);
	free_feaspec_xfields(xfields);

/* print spectral information */

	freq_format = get_genhd_s("freq_format", hd);
	spsassert(freq_format, "print_feaspec_rec: freq_format undefined.")
    	num_freqs = get_genhd_l("num_freqs", hd);
    	spsassert(num_freqs, "print_feaspec_rec: num_freqs undefined.")
	spec_type = get_genhd_s("spec_type", hd);
	spsassert(spec_type, "print_feaspec_rec: spec_type undefined.")

	if (*freq_format == SPFMT_ARB_VAR)
		n = *p->n_frqs;
	else
		n = *num_freqs;

	sf = get_genhd_val("sf", hd, 1.0);

	(void)fprintf(file, "freq\t\tre_spec_val\tim_spec_val\n");

	for (i = 0; i < n; i++) {
		(void)fprintf(file, "%e", p->frqs[i]);
		if (p->re_spec_val)
			(void)fprintf(file, "\t%e", p->re_spec_val[i]);
		else if (p->re_spec_val_b)
			(void)fprintf(file, "\t%d", (int)p->re_spec_val_b[i]);
		if (*spec_type == SPTYP_CPLX)
			(void)fprintf(file, "\t%e", p->im_spec_val[i]);
		(void)fprintf(file, "\n");
	}

	(void) fprintf(file, "\n");
	return;
}

static
char *standard_fields[] = {"tag", "tot_power", "re_spec_val", "im_spec_val",
			   "n_frqs", "frqs", "frame_len", NULL};

char **
get_feaspec_xfields(hd)
struct header *hd;
{

char **list;
int  list_size = 1;
long i,j;

	spsassert(hd, "get_feaspec_xfields: hd is NULL");
	spsassert(hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SPEC,
		"get_feaspec_xfields: hd is not FEA or not FEA_SPEC");
	list = (char **)malloc(sizeof(char *));
	list[0] = NULL;

	for (i=0; hd->hd.fea->names[i] != NULL; i++) {
		for (j=0; standard_fields[j] != NULL; j++) {
		   if (strcmp(hd->hd.fea->names[i], standard_fields[j]) == 0)
			break;
		}
		if (standard_fields[j] == NULL) {
		   list = (char **)realloc((char *)list, 
			(list_size+1)*sizeof(char *));
		   list[list_size-1] = savestring(hd->hd.fea->names[i]);
		   list[list_size++] = NULL;
		}
	}
	return list;
}

void
free_feaspec_xfields(list)
    char    **list;
{
    int	    i;

    for (i = 0; list[i]; i++)
	free(list[i]);
    free((char *) list);
}
