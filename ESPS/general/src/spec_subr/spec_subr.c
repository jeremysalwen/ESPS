/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Select subrange of frequencies in FEA_SPEC file and output as ARB_FIXED
 *
 */

static char *sccs_id = "@(#)spec_subr.c	1.2	8/31/95	ERL";
int    debug_level = 0;


/* INCLUDE FILES */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feaspec.h>

/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "8/31/95"
#define EC_SCCS_VERSION "1.2"

/* LOCAL TYPEDEFS AND STRUCTURES */

/* LOCAL MACROS */

#define SYNTAX \
USAGE("spec_subr -b freq_range [-r range] [-x debug_level] in.spec out.spec")

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", ProgName, (text)); \
  exit(1);}
#define REQUIRE(test, text) {if (!(test)) ERROR(text)}


/* ESPS FUNCTION DECLARATIONS */

char	*get_cmd_line();
void	frange_switch(), lrange_switch();


/* STATIC (LOCAL) GLOBAL VARIABLES */

static char *ProgName = "spec_subr";
static char *Version = EC_SCCS_VERSION;
static char *Date = EC_SCCS_DATE;

 
/* MAIN PROGRAM */

int
main(argc, argv)
    int			argc;
    char		**argv;
{
    extern int		optind;
    extern char		*optarg;
    int			ch;

    int			rflag = NO;
    char 		*rrange;
    long		start_rec, end_rec, num_recs, num_read;

    int			bflag = NO;
    char 		*brange;
    double		band_low, band_high, band;

    char		*inname;
    FILE		*infile;
    struct header	*inhd;
    struct feaspec	*inrec;

    char		*outname;
    FILE		*outfile;
    struct header	*outhd;
    struct feaspec	*outrec;

    int			def_tot_power;
    int			freq_format_in;
    int			spec_type;
    int			contin;
    long		num_freqs_in, num_freqs_out;
    int			frame_meth;
    float		*freqs_in, *freqs_out, freq;

    long		frmlen;
    int			re_spec_format;

    long		*indices;
    long		i;

    /*
     *  Process command-line options.
     */

    while ((ch = getopt(argc, argv, "b:r:x:")) != EOF)
        switch (ch)
	{
	case 'b':
	    brange = optarg;
	    bflag++;
	    break;
	case 'r':
	    rrange = optarg;
	    rflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	default:
	    SYNTAX;
	    break;
	}

    /*
     * Process file arguments.
     */

    if (argc - optind < 2)
    {
	Fprintf(stderr, "%s: Too few file names.\n", ProgName);
	SYNTAX;
    }
    if (argc - optind > 2)
    {
	Fprintf(stderr, "%s: Too many file names.\n", ProgName);
	SYNTAX;
    }

    inname = argv[optind++];
    outname = argv[optind++];

    inname = eopen(ProgName, inname, "r", FT_FEA, FEA_SPEC, &inhd, &infile);

    inrec = allo_feaspec_rec(inhd, FLOAT);

    /*
     *  Process parameters and options.
     */

    start_rec = 1;
    num_recs = 0;
    end_rec = LONG_MAX;

    if (rflag)
    {
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	num_recs =
	    (end_rec == LONG_MAX) ? 0 : end_rec - start_rec + 1;
	REQUIRE(start_rec > 0, "Can't start before beginning of file");
	REQUIRE(start_rec <= end_rec, "empty range of records specified");
    }

    if (!bflag)
    {
	ERROR("No frequency range specified.");
    }
    else
    {
	band_low = DBL_MAX;
	band_high = -DBL_MAX;
	frange_switch(brange, &band_low, &band_high);
	band = band_high - band_low;
	REQUIRE(band > 0, "Empty frequency band specified");
    }

    /*
     *  Create output-file header.
     */

    outhd = new_header(FT_FEA);

    def_tot_power = (get_fea_type("tot_power", inhd) == FLOAT);

    freq_format_in = *get_genhd_s("freq_format", inhd);
    spec_type = *get_genhd_s("spec_type", inhd);
    contin = *get_genhd_s("contin", inhd);
    num_freqs_in = *get_genhd_l("num_freqs", inhd);
    frame_meth = *get_genhd_s("frame_meth", inhd);

    REQUIRE(freq_format_in != SPFMT_ARB_VAR, "ARB_VAR not yet supported");

    freqs_in = inrec->frqs;

    frmlen =
	(frame_meth == SPFRM_FIXED)
	    ? *get_genhd_l("frmlen", inhd)
		: 0;		/* arbitrary */

    re_spec_format = get_fea_type("re_spec_val", inhd);

    indices = (long *) malloc(sizeof(long) * num_freqs_in);
    num_freqs_out = 0;
    for (i = 0; i < num_freqs_in; i++)
    {
	freq = freqs_in[i];

	if (band_low <= freq && freq <= band_high)
	    indices[num_freqs_out++] = i;
    }

    REQUIRE(num_freqs_out > 0,
	    "No frequencies in input file in specified band");

    freqs_out = (float *) malloc(sizeof(float) * num_freqs_out);
    for (i = 0; i < num_freqs_out; i++)
	freqs_out[i] = freqs_in[indices[i]];

    (void) init_feaspec_hd(outhd,
			   def_tot_power,
			   ARB_FIXED,
			   spec_type,
			   contin,
			   num_freqs_out,
			   frame_meth,
			   freqs_out,
			   0.0,		/* arbitrary */
			   frmlen,
			   re_spec_format);

    add_source_file(outhd, inname, inhd);

    outhd->common.tag = inhd->common.tag;
    if (inhd->common.tag
	&& !copy_genhd(outhd, inhd, "src_sf")
	&& !copy_genhd(outhd, inhd, "sf"))
    {
	Fprintf(stderr,
		"%s: Warning: no \"src_sf\" or \"sf\" in tagged input file.\n",
		ProgName);
    }

    (void) strcpy(outhd->common.prog, ProgName);
    (void) strcpy(outhd->common.vers, Version);
    (void) strcpy(outhd->common.progdate, Date);
    add_comment(outhd, get_cmd_line(argc, argv));
    outhd->variable.refer = inhd->variable.refer;

    *add_genhd_d("band_low", (double *) NULL, 1, outhd) = band_low;
    *add_genhd_d("band", (double *) NULL, 1, outhd) = band;
    *add_genhd_l("start", (long *) NULL, 1, outhd) = start_rec;
    *add_genhd_l("nan", (long *) NULL, 1, outhd) = num_recs;

    update_waves_gen(inhd, outhd, (float) start_rec, (float) 1.0);

    outname = eopen(ProgName, outname, "w", NONE, NONE, &outhd, &outfile);
    write_header(outhd, outfile);
    outrec = allo_feaspec_rec(outhd, FLOAT);

    /*
     *  Main loop.
     */

    num_read = start_rec - 1;
    fea_skiprec(infile, num_read, inhd);

    while (num_read++ < end_rec && get_feaspec_rec(inrec, inhd, infile) != EOF)
    {
	if (inhd->common.tag)
	    *outrec->tag = *inrec->tag;

	for (i = 0; i < num_freqs_out; i++)
	    outrec->re_spec_val[i] = inrec->re_spec_val[indices[i]];

	if (spec_type == SPTYP_CPLX)
	    for (i - 0; i < num_freqs_out; i++)
		outrec->im_spec_val[i] = inrec->im_spec_val[indices[i]];
	
	if (def_tot_power)
	    *outrec->tot_power = *inrec->tot_power;

	if (frame_meth == SPFRM_VARIABLE)
	    *outrec->frame_len = *inrec->frame_len;

	(void) put_feaspec_rec(outrec, outhd, outfile);
    }

    exit(0);
    /*NOTREACHED*/
}
