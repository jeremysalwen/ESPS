/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * History:  pre-SDS version by Shankar Narayan
 *	     SDS version by Joe Buck
 *	     SPS version by Rod Johnson
 *	     converted to use FEA_ANA files and to avoid ndrec by John Shore
 *	     non-RC spec_reps, non-FEA_ANA files added by Rod Johnson
 *
 * Brief description: makes spectral records from analysis feature records.
 *
 */

static char *sccs_id = "@(#)me_spec.c	1.14	8/31/95	ESI/ERL";

extern char *ProgName, *Version, *Date;		/* Defined in version.c. */
 
#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/anafea.h>
#include "genana.h"
#include <esps/feaspec.h>

#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num), sizeof(type))) == NULL) \
    {Fprintf(stderr, "%s: can't allocate memory--%s", ProgName, (msg)); \
    exit(1);}}

#define SYNTAX \
USAGE("me_spec [-n num_freqs] [-o max_order] [-r range] [-x debug_level]\n [-G] [-P params] file.fea file.spec") \
 ;


/* GLOBAL ARRAYS*/
char *yesno[] = {"NO", "YES", NULL};

/* External ESPS functions. */

char    *get_cmd_line();
char    *eopen();
struct zfunc
	*get_genzfunc();
long	*fld_range_switch();
void	lrange_switch();
int	reps_rc();
void	rctoc();
void	get_arspec();

/* local ESPS function. */

double  avg_raw_pwr();

int     debug_level = 0;	/* Specified with -x option. */

void
main(argc, argv)
    int     argc;
    char    **argv;
{
    int	    c;			/* Command-line option letter. */
    extern  optind;		/* Used by getopt for command-line parsing. */
    extern char
            *optarg;		/* Used by getopt for command-line parsing. */

    int	    nflag = NO;		/* -n option specified? */
    int     num_freqs;		/* Number of frequencies in output spectra. */

    int	    oflag = NO;		/* max_order defined? */
    int	    max_order;		/* Upper bound on order of output spectra. */

    int	    rflag = NO;		/* -r option specified? */
    char    *range;		/* Range of records to process */
    long    startrec, endrec;	/* Starting and ending input records. */
    long    nrecs;		/* Number of input records to process. */

    int	    Gflag = NO;		/* -G option specified? */
    int	    fana;		/* special FEA_ANA processing? */

    char    *params = NULL;	/* Parameter-file name. */

    char    *spec_param_field;	/* Field name for spectral parameters. */

    short   spec_rep;		/* Numerical code for type of spec. params. */

    char    *power_field;	/* Name of field containing spectral power. */
    double  def_power;		/* Used if power not in input records. */
    float   total_power;        
    char    *samp_freq_name;	/* Name of header item for sampling freq. */
    double  samp_freq;		/* Sampling frequency. */
    double src_sf;              /* sampling freq. of source sampled data file*/
    struct header
            *ihd, *ohd;		/* Input and output file headers. */
    char    *iname;		/* Input file name. */
    FILE    *ifile,		/* Input stream. */
            *ofile;		/* Output stream. */
    struct genana
            *ana_rec;		/* Input analysis feature record. */
    float   *spec_params;
    struct feaspec
            *spec_rec;		/* Output spectrum record. */
    float   *lpcfilter;		/* LPC filter coefficients. */
    float   *reflcoef;		/* Reflection coefficients. */
    float   gain;		/* Filter gain. */
    double  raw_pwr;		/* Raw power or average raw power. */
    int     i;			/* Loop index */
    int	    input_order;
    long    fld_len;
    long    *elements;
    int     order;		/* Filter order */
    long    order_vcd;		/* Order of voiced frames in FEA_ANA file. */
    long    order_unvcd;	/* Order of unvoiced frames in FEA_ANA file. */
    char    *cmd_line;		/* Command line saved for output header. */
    int	    first = YES;	/* First time through main loop? */
    int     frame_meth;         /* holds frame type information */
    long    frlen;

    cmd_line = get_cmd_line(argc, argv); /* Store copy of command line. */

/* Process command-line options. */

    while ((c = getopt(argc, argv, "n:o:r:x:GP:")) != EOF)
    switch (c)
    {
    case 'n':
	nflag = YES;
        num_freqs = atoi(optarg);
        break;
    case 'o':
	oflag = YES;
	max_order = atoi(optarg);
	break;
    case 'r': 
	rflag = YES;
        range = optarg;
        break;
    case 'x': 
        debug_level = atoi(optarg);
        break;
    case 'G':
	Gflag = YES;
	break;
    case 'P':
	params = optarg;
	break;
    default: 
        SYNTAX;
        break;
    }

/* Process file names and open files. */

    if (argc - optind > 2)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }

    if (argc - optind < 2)
    {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }

    iname = eopen(ProgName, argv[optind++],
			    "r", FT_FEA, NONE, &ihd, &ifile);

    (void) eopen(ProgName, argv[optind++],
			    "w", FT_FEA, NONE, &ohd, &ofile);

/* Get parameter values. */

    (void) read_params(params, SC_NOCOMMON, (char *) NULL);

    if (!nflag)
	num_freqs =
	    (symtype("num_freqs") == ST_INT)
	    ? getsym_i("num_freqs")
	    : 513;

    if (num_freqs < 2)
    {
        Fprintf(stderr,
	    "%s: number of frequencies %d is less than 2.\n",
	    ProgName, num_freqs);
        exit(1);                    
    }
    
    if (debug_level)
	Fprintf(stderr, "num_freqs: %d\n", num_freqs);

    if (!oflag && symtype("max_order") == ST_INT)
    {
	max_order = getsym_i("max_order");
	oflag = YES;
    }

    if (debug_level)
	if (oflag)
	    Fprintf(stderr, "max_order: %d\n", max_order);
	else
	    Fprintf(stderr, "no max_order\n");

    if (rflag)
    {
	startrec = 1;
	endrec = LONG_MAX;
	lrange_switch(range, &startrec, &endrec, 0);
	nrecs =
	    (endrec != LONG_MAX)
	    ? endrec - startrec + 1
	    : 0;
    }
    else
    {
	startrec =
	    (symtype("start") == ST_INT)
	    ? getsym_i("start")
	    : 1;
	nrecs =
	    (symtype("nan") == ST_INT)
	    ? getsym_i("nan")
	    : 0;
	endrec = (nrecs != 0)
	    ? startrec - 1 + nrecs
	    : LONG_MAX;
    }

    if (startrec < 1)
    {
	Fprintf(stderr, "%s: Can't start before beginning of file.\n",
	    ProgName);
	exit(1);
    }

    if (endrec < startrec)
    {
	Fprintf(stderr, "%s: Last record precedes first.\n", ProgName);
	exit(1);
    }

    nrecs = endrec - (startrec - 1);

    if (debug_level)
	Fprintf(stderr, "start: %ld.  endrec: %ld.  nan: %ld.\n",
	    startrec, endrec, nrecs);

    fana = ihd->hd.fea->fea_type == FEA_ANA && !Gflag;
    
    if (symtype("spec_param_field") == ST_STRING)
	spec_param_field = getsym_s("spec_param_field");
    else if (fana)
	spec_param_field = "spec_param";
    else
    {
	Fprintf(stderr, "Name of spec_param field not specified.\n");
	exit(1);
    }

    if (debug_level)
	Fprintf(stderr, "spec_param_field: \"%s\"\n", spec_param_field);

    if (symtype("spec_rep") == ST_STRING)
    {
	spec_rep = lin_search(spec_reps, getsym_s("spec_rep"));
	if (spec_rep < 0)
	{
	    Fprintf(stderr,
		"%s: Unrecognized spectral representation %s.\n",
		 ProgName, getsym_s("spec_rep"));
	    exit(1);
	}
    }
    else if (fana)
	spec_rep = *get_genhd_s("spec_rep", ihd);
    else
    {
	Fprintf(stderr, "%s: Spectral representation not specified.\n",
	    ProgName);
	exit(1);
    }

    if ( symtype("power_field") == ST_STRING
	|| symtype("power") == ST_FLOAT )
    {
	if (symtype("power_field") != ST_STRING)
	    power_field = NULL;
	else
	{
	    power_field = getsym_s("power_field");
	    if (get_fea_type(power_field, ihd) != FLOAT)
		if (symtype("power") == ST_FLOAT)
		    power_field = NULL;
		else
		{
		    Fprintf(stderr,
			"%s: Field \"%s\" and symbol \"power\" undefined or not float.\n",
			ProgName, power_field);
		    exit(1);
		}
	}

	if (power_field == NULL)
	    def_power = getsym_d("power");
    }
    else if (fana)
	power_field = "raw_power";
    else
    {
	Fprintf(stderr,
	    "%s: Neither field nor default value for power specified.\n",
	    ProgName);
	exit(1);
    }

    if ( symtype("samp_freq_name") == ST_STRING
	|| symtype("samp_freq") == ST_FLOAT )
    {
	if (symtype("samp_freq_name") != ST_STRING)
	    samp_freq = getsym_d("samp_freq");
	else
	{
	    samp_freq_name = getsym_s("samp_freq_name");
	    if (genhd_type(samp_freq_name, (int *) NULL, ihd) == FLOAT)
		samp_freq = get_genhd_val(samp_freq_name, ihd, (double)1);
	    else if (symtype("samp_freq") == ST_FLOAT)
		samp_freq = getsym_d("samp_freq");
	    else
	    {
		Fprintf(stderr,
		    "%s: Header item \"%s\" and symbol \"samp_freq\" undefined or not float.\n",
		    ProgName, samp_freq_name);
		exit(1);
	    }
	}
    }
    else if (fana)
	samp_freq = get_genhd_val("src_sf", ihd, (double)(1));
    else
    {
	Fprintf(stderr,
	    "%s: No header-item name or default value for sampling frequency.\n",
	    ProgName);
	exit(1);
    }

/* Get elements, order of spec_param field. */

    elements =
	fld_range_switch(spec_param_field, &spec_param_field, &fld_len, ihd);

    if (elements == NULL)
    {
	Fprintf(stderr, "%s: Input field %s undefined.\n",
	    ProgName, spec_param_field);
	exit(1);
    }

    if (get_fea_type(spec_param_field, ihd) != FLOAT)
    {
	Fprintf(stderr, "%s: Input field %s undefined or not float.\n",
	    ProgName, spec_param_field);
	exit(1);
    }

    if (fana)
    {
	order_vcd = get_genhd_val("order_vcd", ihd, (double) 1);
	order_unvcd = get_genhd_val("order_unvcd", ihd, (double) 1);
	input_order = MAX(order_vcd, order_unvcd);

	if (debug_level) 
	  Fprintf(stderr, "order_vcd = %ld; order_unvcd = %ld\n",
		  order_vcd, order_unvcd);

	if (input_order > fld_len)
	{
	    Fprintf(stderr, "%s: Input field length %d too small.\n",
		ProgName, fld_len);
	}
	if ((order_vcd == 0) || (order_unvcd == 0)) 
	{ 
	    order_vcd = order_unvcd = input_order; 

	    if (debug_level) {
	      Fprintf(stderr, 
		 "%s: WARNING - order_vcd or order_unvcd is 0;\n", ProgName);
	      Fprintf(stderr, "\tperhaps you need the -G option\n"); 
	    }
	}
    }
    else
	input_order = fld_len;

    if (!oflag || max_order > input_order)
	max_order = input_order;
    
    if (debug_level) 
	  Fprintf(stderr, "input_order = %d\n", input_order);

/* Make output header */

    if (debug_level)
	Fprintf(stderr, "Making output header.\n");

    ohd = new_header(FT_FEA);
    add_source_file(ohd, iname, ihd);
    add_comment(ohd, cmd_line);
    ohd->common.tag = ihd->common.tag;
    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    ohd->variable.refer	     = ihd->variable.refer;

/*
 * Check file subtype (and possibly frmlen) to determine
 * frame_meth value - only fea_ana with frmlen != 0 are FIXED type
*/
    frame_meth = SPFRM_NONE;
    frlen = (long)get_genhd_val("frmlen", ihd, (double)0);
    if(fana && frlen != 0)
      frame_meth = SPFRM_FIXED;
      
    

    spsassert(
	      !init_feaspec_hd(ohd, YES, SPFMT_SYM_EDGE, SPTYP_DB, YES,
			       (long)num_freqs, frame_meth, (float *)NULL,
			       samp_freq, frlen, FLOAT),
	      "Error filling FEA_SPEC header.\n");

    if(fana){
      (void)add_genhd_l("order_vcd", &order_vcd, 1, ohd);
      (void)add_genhd_l("order_unvcd", &order_unvcd, 1, ohd);
   (void)add_fea_fld("voiced", 1L, (short) 0, (long *)NULL, CODED, yesno, ohd);
    }

    if((src_sf = get_genhd_val("src_sf", ihd, (double)-1)) != -1)
      (void)add_genhd_d("src_sf", &src_sf, (int)1, ohd);

/*
 * update start_time and record_freq
*/
   update_waves_gen(ihd, ohd, (float)startrec, 1.0);

/* Allocate records */

    if (debug_level)
	Fprintf(stderr, "Allocating records.\n");

    ana_rec = allo_genana_rec(ihd, fana, spec_param_field, power_field);
    spec_rec = allo_feaspec_rec(ohd,FLOAT);
    TRYALLOC(float, input_order, spec_params, "input spectral parameters")
    TRYALLOC(float, max_order, lpcfilter, "lpc filter")

    if (spec_rep == RC)
	reflcoef = spec_params;
    else
	TRYALLOC(float, max_order, reflcoef, "reflection coefficients")

/* Position at first record, then fire away */

    fea_skiprec(ifile, startrec - 1, ihd);
    
    if (debug_level)
	Fprintf(stderr, "%ld records.\n", nrecs);

    while (nrecs-- && (get_genana_rec(ana_rec, ihd, ifile) != EOF))
    {
	/* if the file is tagged, we can use the first record to
	    tell us the starting point; if there's only one record
	    being processed, we also know the number of points */
	if (first)
	{
	    first = NO;
	    if (fana && ihd->common.tag)
	    {
		(void)add_genhd_l("start", ana_rec->tag, (int)1, ohd);
		if (nrecs == 0) 
		    (void)add_genhd_l("nan", ana_rec->frame_len, (int)1, ohd);
	    }
	    /*now we go ahead and write the header, before any 
		records are output*/

	    if (debug_level)
		Fprintf(stderr, "Writing header.\n");

	    write_header(ohd, ofile);
	}
        if (ihd->common.tag) *spec_rec->tag = *ana_rec->tag;

	if (!fana)
	    order = input_order;
	else if (*ana_rec->frame_type == VOICED
		|| *ana_rec->frame_type == TRANSITION)
        {
            order = order_vcd;
            *(short *)get_fea_ptr(spec_rec->fea_rec, "voiced", ohd) = YES;
        }
        else /*all other FEA_ANA frame types are treated as unvoiced*/
        {
            order = order_unvcd;
            *(short *)get_fea_ptr(spec_rec->fea_rec, "voiced", ohd) = NO;
        }

	if(frame_meth == SPFRM_VARIABLE){
	frlen = fana ? *ana_rec->frame_len : (long)0;
	*spec_rec->frame_len = frlen;
        }
			/* invalid if ana prog used overlapping */
			/* or noncontiguous frames */

	total_power = raw_pwr =
	    power_field == NULL ? def_power
	    : !fana ? *ana_rec->raw_power
	    : avg_raw_pwr(ana_rec, ihd);
	*spec_rec->tot_power = (float)total_power;

	for (i = 0; i < order; i++)
	    spec_params[i] = ana_rec->spec_param[elements[i]];

	if (debug_level)
	{
	    Fprintf(stderr, "Locn = %ld\n", *ana_rec->tag);
	    Fprintf(stderr, "Raw_power = %g\n", raw_pwr);
	    Fprintf(stderr, "Spectral parameters:\n");
	    for (i = 1; i <= order; i++)
		Fprintf(stderr, "%d\t%f\n", i, spec_params[i-1]);
	}

	if (spec_rep != RC)
	{
	    if ( reps_rc(spec_params, spec_rep, reflcoef, order,
		    0.5*samp_freq) == -1 )
	    {
		Fprintf(stderr,
			"%s: unknown spectral parameter type %d.\n",
			ProgName, spec_rep);
		exit(1);
	    }
	}

	if (debug_level)
	{
	    Fprintf(stderr, "Reflection coefficients:\n");
	    for (i = 1; i <= order; i++)
		Fprintf(stderr, "%d\t%f\n", i, reflcoef[i-1]);
	}

	order = MIN(order, max_order);

	rctoc(reflcoef - 1, order, lpcfilter - 1, &gain);
                                /* rctoc indices start at 1 */
	if (debug_level)
	{
	    Fprintf(stderr, "Filter coefficients:\n");
	    for (i = 1; i <= order; i++)
		Fprintf(stderr, "%d\t%f\n", i, lpcfilter[i-1]);
	    Fprintf(stderr, "Gain = %g\n", gain);
	}

        get_arspec(lpcfilter - 1, order, raw_pwr*gain, samp_freq,
                    spec_rec->re_spec_val, 2*(num_freqs-1));
        (void)put_feaspec_rec(spec_rec, ohd, ofile);
	(void) fflush(ofile);
    }
    exit(0);
    /*NOTREACHED*/
}


double
avg_raw_pwr(ana_rec, h)    /* Average raw power in analysis record */
    struct genana   *ana_rec;
    struct header   *h;
{

    if (ana_rec->p_pulse_len[0] == 0    /* unvoiced */
        || ana_rec->raw_power[1] < 0)   /* only one pwr for frame */
        return ana_rec->raw_power[0];
    else                                /* voiced, one pwr per pulse */
    {
        int     i;
        double  len,
                sum_len = 0,
                sum_energy = 0;

        for (   i = 0; 
                i < *(long *) get_genhd("maxpulses", h)
                    && ana_rec->p_pulse_len[i] != 0;
                i++
            )
	{
            sum_len += len = ana_rec->p_pulse_len[i];
            sum_energy += len * ana_rec->raw_power[i];
        }
        return sum_energy/sum_len;
    }
}



