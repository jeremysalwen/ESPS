/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *   compute Bark-scaled critical-band spectra
 */

static char *sccs_id = "@(#)barkspec.c	1.3\t9/2/98\tERL";


/* INCLUDE FILES */

#include <stdlib.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>

/* LOCAL CONSTANTS */

#define VERSION	"1.3"		/* must be 7 char or less */
#define DATE	"9/2/98"	/* must be 25 char or less */
#define PROG	"barkspec"	/* must be 15 char or less */

#if !defined(HP700)
#define ASINH_AVAILABLE		/* Define this if implementation provides
				 * non-ANSI function "asinh"; else undefine
				 * to get homebrew definition further down.
				 */
#endif

#define LOG10_BY_10 0.230258509299404568402
				/* log(10.0)/10.0 */

#define LO_3DB	(-0.590f)	/* low edge of -3 dB band of filter */
#define HI_3DB	( 0.410f)	/* high edge of -3 dB band of filter */
#define LO_LIM	(-6.465f)	/* low cutoff (-60dB) of filter */
#define HI_LIM	( 2.865f)	/* high cutoff (-60dB) of filter */

/* LOCAL MACROS */

#define SYNTAX \
USAGE("barkspec [-a add_const] [-m mult_const] [-n num_freqs] [-r range]\n" \
      "\t[-x debug_level] [-B  bark_range] [-H  freq_range]\n" \
      "\t[-P  param_file] [-S spec_type] [-X] input.fspec output.fspec")

#define ERROR(text) \
{(void) fprintf(stderr, "%s: %s - exiting\n", PROG, (text)); exit(1);}

#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define TRYALLOC(type, num, var, txt) \
{if (((var) = (type *) malloc((num)*sizeof(type))) == NULL) \
 {(void) fprintf(stderr, "%s: can't allocate memory--%s - exiting\n", \
		 PROG, (txt)); exit(1);}}

#define STRCMP(a, op, b) (strcmp((a), (b)) op 0)

#define ABS(x) (((x) < 0) ? -(x) : (x))

/* SYSTEM FUNCTIONS AND VARIABLES */

extern int	getopt();		/* parse command options */
extern int	optind;			/* used by getopt */
extern char	*optarg;		/* used by getopt */

/* ESPS FUNCTIONS AND VARIABLES */

int		debug_level = 0;
extern char	*get_cmd_line(int argc, char **argv);

/* LOCAL TYPEDEFS AND STRUCTURES */

struct filt {
    long    start, end;			/* limits of range of summation */
    double  *weights;			/* filter weights */
};

/* LOCAL FUNCTION DECLARATIONS */

static double	bark_to_hz(double b);
static double	hz_to_bark(double h);
static double	filter_weight(double b);
#ifndef ASINH_AVAILABLE
static double	asinh(double x);
#endif

/* STATIC (LOCAL) GLOBAL VARIABLES */


/* MAIN PROGRAM */

int
main(int   argc,
     char  **argv)
{
    int		    ch;			/* command-line option letter */

    long	    i, j, k, n;		/* loop counters and array indices */

    char	    *a_arg = NULL;	/* a-option argument */
    double	    add_const;		/* constant to be added
					 * to output values */

    char	    *m_arg = NULL;	/* m-option argument */
    double	    mult_const;		/* constant by which output values
					 * are to be multiplied */

    char	    *n_arg = NULL;	/* n-option argument */
    long	    out_nfreqs;		/* number of Bark values at which
					 * to compute spectral values */

    char	    *r_arg = NULL;	/* r-option argument */
    long	    startrec;		/* first record to process */
    long	    nan;		/* number of records to process,
					 * but 0 means range extends to
					 * end of file */

    char	    *B_arg = NULL;	/* B-option argument */
    double	    bark_low;		/* low limit of range of Bark-
					 * scale values to be covered */
    double	    bark_high;		/* high limit of range of Bark-
					 * scale values to be covered */

    char	    *H_arg = NULL;	/* H-option argument */
    double	    band_low;		/* linear-scale frequency
					 * equivalent to bark_low */
    double	    band_high;		/* linear-scale frequency
					 * equivalent to bark_high */

    char	    *param_name = NULL;	/* parameter file name */

    char	    *S_arg = NULL;	/* S-option argument */
    char	    *sptypsym;		/* param file entry for spec_type */
    int		    out_sptyp;		/* SPTYPE_PWR or SPTYPE_DB: ouput
					 * spectral values directly or
					 * convert to dB? */

    int		    X_specified = NO;	/* output table of filter band
					 * edges and peak freqs? */

    char	    *inname;		/* input file name */
    FILE	    *infile;		/* input stream */
    struct header   *inhd;		/* input file header */
    struct feaspec  *inrec;		/* input FEA_SPEC record */
    float	    *inbuf;		/* input data buffer */
    float	    *inbufim;		/* input data buffer (imag part) */
    int		    in_sptyp;		/* input file spec_type */
    long	    in_nfreqs;		/* number of frequencies in input
					 * record */
    int		    frame_meth;		/* input file frame_meth */
    long	    frmlen;		/* input file frame length */
    int		    tot_power_def; 	/* tot_power field defined? */
    double	    record_freq;	/* input file record frequency */

    char	    *outname;		/* output file name */
    FILE	    *outfile;		/* output stream */
    struct header   *outhd;		/* output file header */
    struct feaspec  *outrec;		/* output FEA_SPEC record */
    float	    *outbuf;		/* output data buffer */
    float	    *bark_freqs;	/* bark_scale filter peak
					 * frequencies */
    double	    chan_sp;		/* spacing between values
					 * in bark_freqs */
    float	    *freqs;		/* linear-scale equivalents of
					 * values in bark_freqs */

    float	    sf;			/* sf/2 = upper limit of input
					 * frequency range */
    double	    df;			/* interval between frequencies
					 * in input */

    struct filt	    *filters;		/* filter structures */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "a:m:n:r:x:B:H:P:S:X")) != EOF)
	switch (ch) {

	case 'a':
	    a_arg = optarg;
	    break;
	case 'm':
	    m_arg = optarg;
	    break;
	case 'n':
	    n_arg = optarg;
	    break;
	case 'r':
	    r_arg = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'B':
	    B_arg = optarg;
	    break;
	case 'H':
	    H_arg = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'S':
	    S_arg = optarg;
	    break;
	case 'X':
	    X_specified = YES;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    /*
     * PROCESS FILE NAMES.
     */

    if (argc - optind > 2) {

	fprintf(stderr, "%s: too many file names.\n", PROG);
	SYNTAX;
    }

    if (argc - optind < 2) {

	fprintf(stderr, "%s: not enough file names.\n", PROG);
	SYNTAX;
    }

    inname = argv[optind++];
    outname = argv[optind++];

    REQUIRE(STRCMP(inname, ==, "-") || STRCMP(inname, != , outname),
	    "output file same as input");

    /*
     * OPEN AND CHECK INPUT FILE.
     */

    inname = eopen(PROG, inname, "r", FT_FEA, FEA_SPEC, &inhd, &infile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: input file = \"%s\".\n", PROG, inname);

    REQUIRE(genhd_type("spec_type", NULL, inhd) == CODED,
	    "header item \"spec_type\" undefined or wrong type");
    in_sptyp = *get_genhd_s("spec_type", inhd);

    REQUIRE(genhd_type("freq_format", NULL, inhd) == CODED,
	    "header item \"freq_format\" undefined or wrong type");
    REQUIRE(*get_genhd_s("freq_format", inhd) == SPFMT_SYM_EDGE,
	    "input file freq_format is not SYM_EDGE");

    REQUIRE(genhd_type("sf", NULL, inhd) == FLOAT,
	    "header item \"sf\" undefined or wrong type");
    sf = *get_genhd_f("sf", inhd);

    REQUIRE(genhd_type("num_freqs", NULL, inhd) == LONG,
	    "input header item \"num_freqs\" undefined or wrong type");
    in_nfreqs = *get_genhd_l("num_freqs", inhd);

    REQUIRE(genhd_type("frame_meth", NULL, inhd) == CODED,
	    "header item \"frame_meth\" undefined or wrong type");
    frame_meth = *get_genhd_s("frame_meth", inhd);

    if (frame_meth == SPFRM_FIXED) {

	REQUIRE(genhd_type("frmlen", NULL, inhd) == LONG,
		"header item \"frmlen\" undefined or wrong type "
		"in file with FIXED frame_meth");
	frmlen = *get_genhd_l("frmlen", inhd);

    } else {
	frmlen = 0;
    }

    tot_power_def = (get_fea_type("tot_power", inhd) != UNDEF);

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: input spec_type = %d, sf = %g, num_freqs = %ld\n"
		"%s: frame_meth = %d, frmlen = %ld, tot_power_def = %s\n",
		PROG, in_sptyp, sf, in_nfreqs,
		PROG, frame_meth, frmlen, (tot_power_def) ? "YES" : "NO");

    /*
     * READ PARAMETER FILE.
     */

    (void) read_params(param_name, SC_NOCOMMON, inname);

    /*
     * PROCESS OPTIONS.
     */

    /* RECORD RANGE */

    startrec = 1;
    nan = 0;

    if (r_arg) {

	long	    endrec;		/* last record to process */

	endrec = LONG_MAX;
	lrange_switch(r_arg, &startrec, &endrec, NO);
	REQUIRE(endrec >= startrec,
		"empty range of records specified");

	if (endrec != LONG_MAX)
	    nan = endrec - startrec + 1;

    } else {

	switch (symtype("start")) {

	case ST_INT:
	    startrec = getsym_i("start");
	    break;
	case ST_UNDEF:
	    break;
	default:
	    ERROR("type of symbol \"start\" is not INT");
	}

	switch (symtype("nan")) {

	case ST_INT:
	    nan = getsym_i("nan");
	    break;
	case ST_UNDEF:
	    break;
	default:
	    ERROR("type of symbol \"nan\" is not INT");
	}
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: start = %ld, nan = %ld\n", PROG, startrec, nan);
	
    /* FREQUENCY RANGE */

    REQUIRE(B_arg == NULL || H_arg == NULL,
	    "both B and H options specified");

    bark_low = 0.0;		/* default */
    bark_high = 0.0;		/* 0 implies use default */
    band_low = 0.0;		/* default */
    band_high = 0.0;		/* 0 implies use default */

    if (B_arg != NULL) {

	frange_switch(B_arg, &bark_low, &bark_high);
	band_low = bark_to_hz(bark_low);
	band_high = bark_to_hz(bark_high);

    } else if (H_arg != NULL) {

	frange_switch(H_arg, &band_low, &band_high);
	bark_low = hz_to_bark(band_low);
	bark_high = hz_to_bark(band_high);

    } else {

	if (symtype("bark_high") != ST_UNDEF) {

	    REQUIRE(symtype("bark_high") == ST_FLOAT,
		    "type of symbol \"bark_high\" is not FLOAT");
	    bark_high = getsym_d("bark_high");
	    band_high = bark_to_hz(bark_high);

	} else if (symtype("band_high") != ST_UNDEF) {

	    REQUIRE(symtype("band_high") == ST_FLOAT,
		    "type of symbol \"band_high\" is not FLOAT");
	    band_high = getsym_d("band_high");
	    bark_high = hz_to_bark(band_high);
	}

	if (symtype("bark_low") != ST_UNDEF) {

	    REQUIRE(symtype("bark_low") == ST_FLOAT,
		    "type of symbol \"bark_low\" is not FLOAT");
	    bark_low = getsym_d("bark_low");
	    band_low = bark_to_hz(bark_low);

	} else if (symtype("band_low") != ST_UNDEF) {

	    REQUIRE(symtype("band_low") == ST_FLOAT,
		    "type of symbol \"band_low\" is not FLOAT");
	    band_low = getsym_d("band_low");
	    bark_low = hz_to_bark(band_low);
	}
    }

    if (band_high == 0.0) {

	/* If the initial 0 values haven't been changed by option or
	 * param-file entry, or if an explicit 0 was specified, assign 
	 * the default values to band_high and bark_high.  We could
	 * have tested either one.  This depends on the fact that
	 * bark_to_hz(0.0) == 0.0 and hz_to_bark(0.0) == 0.0.
	 */
	band_high = sf/2.0;
	bark_high = hz_to_bark(band_high);
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: bark_low = %g, bark_high = %g,\n"
		"\tband_low = %g, band_high = %g\n",
		PROG, bark_low, bark_high, band_low, band_high);

    REQUIRE(bark_high - bark_low >= 1.0,
	    "Bark-scale range less than 1 bark");
    REQUIRE(bark_low >= LO_3DB,
	    "frequency range extends too far below 0");
    REQUIRE(bark_high <= HI_3DB + hz_to_bark(sf/2.0),
	    "frequency range extends too far above Nyquist");

    /* NUMBER AND SPACING OF CHANNELS */

    out_nfreqs = 0;			/* 0 implies use default */

    if (n_arg != NULL) {

	out_nfreqs = atol(n_arg);

    } else if (symtype("num_freqs") != ST_UNDEF) {

	REQUIRE(symtype("num_freqs") == ST_INT,
		"type of symbol \"num_freqs\" is not INT");
	out_nfreqs = getsym_i("num_freqs");
    }

    if (out_nfreqs == 0) {		/* use default */
	out_nfreqs = LROUND(bark_high - bark_low);
    }

    if (out_nfreqs > 1) {

	chan_sp = (bark_high - bark_low - 1.0)/(out_nfreqs - 1);

	REQUIRE(chan_sp > 0.0,
		"spacing between channels is not positive");

    } else {
	chan_sp = 1.0;			/* arbitrary valid numeric value */
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: output num_freqs = %ld, channel spacing = %g\n",
		PROG, out_nfreqs, chan_sp);

    /* SPEC TYPE */

    if (S_arg != NULL) {

	out_sptyp = lin_search(sptyp_names, S_arg);

    } else if (symtype("spec_type") != ST_UNDEF) {

	sptypsym = getsym_s("spec_type");
	
	REQUIRE(sptypsym != NULL,
		"type of symbol \"spec_type\" is not STRING");
	out_sptyp = lin_search(sptyp_names, sptypsym);

    } else {
	out_sptyp = ST_DB;
    }

    REQUIRE(out_sptyp == ST_DB || out_sptyp == ST_PWR,
	    "spec_type is not DB or PWR");

    if (debug_level >= 1)
	fprintf(stderr, "%s: output spec_type = \"%s\"\n",
		PROG, sptyp_names[out_sptyp]);

    /* ADDITIVE AND MULTIPLICATIVE CONSTANTS */

    if (a_arg != NULL) {

	add_const = atof(a_arg);

    } else if (symtype("add_const") != ST_UNDEF) {

	REQUIRE(symtype("add_const") == ST_FLOAT,
		"type of symbol \"add_const\" is not FLOAT");
	add_const = getsym_d("add_const");

    } else {
	add_const = 0;
    }

    if (m_arg != NULL) {

	mult_const = atof(m_arg);

    } else if (symtype("mult_const") != ST_UNDEF) {

	REQUIRE(symtype("mult_const") == ST_FLOAT,
		"type of symbol \"mult_const\" is not FLOAT");
	mult_const = getsym_d("mult_const");

    } else {
	mult_const = 1;
    }

    if (debug_level >= 1)
	fprintf(stderr, "%s: add_const = %g, mult_const = %g\n",
		PROG, add_const, mult_const);

    /*
     * PREPARE FILTERS
     */

    /* FILTER PEAK FREQS */

    TRYALLOC(float, out_nfreqs, bark_freqs, "bark_freqs");
    TRYALLOC(float, out_nfreqs, freqs, "freqs");

    for (i = 0; i < out_nfreqs; i++) {

	bark_freqs[i] = bark_low - LO_3DB + i*chan_sp;
	freqs[i] = bark_to_hz(bark_freqs[i]);
    }

    if (X_specified || debug_level >= 2) {

	fprintf(stderr,
		"-------------bark_freqs------------  "
		"---------------freqs---------------\n");
	fprintf(stderr,
		"  low edge      peak     high edge   "
		"  low edge      peak     high edge \n");
	for (i = 0; i < out_nfreqs; i++)
	    fprintf(stderr,
		    "%11.3f %11.3f %11.3f  %11.1f %11.1f %11.1f\n",
		    bark_freqs[i] + LO_3DB,
		    bark_freqs[i],
		    bark_freqs[i] + HI_3DB,
		    bark_to_hz(bark_freqs[i] + LO_3DB),
		    freqs[i],
		    bark_to_hz(bark_freqs[i] + HI_3DB));
    }

    /* ALLOCATE STORAGE */

    TRYALLOC(struct filt, out_nfreqs, filters, "filters");
    for (i = 0; i < out_nfreqs; i++)
	TRYALLOC(double, in_nfreqs, filters[i].weights, "filter weights");

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: allocated space for filter structs and weights\n",
		PROG);

    /* COMPUTE SUMMATION LIMITS AND WEIGHTS */

    {
	long	N = in_nfreqs - 1;	/* input spectral points are
					 * indexed from 0 thru N.
					 * Indices wrap mod 2N so that
    					 * -N is equivalent to N.
					 * E.g. for spectra from a 256-point
					 * FFT, we would have in_nfreqs = 129,
					 * N = 128 */
	df = sf/(2*N);			/* interval between frequencies
					 * in input; Nyquist is N*df */

	for (i = 0; i < out_nfreqs; i++) {

	    long    lo, hi;		/* indices of input frequencies
					 * spanning range from low filter
					 * cutoff to high cutoff */
	    long    start, end;		/* indexing limits (for filt
					 * structure) */
	    double  peak;		/* peak Bark-scale frequency
					 * of filter */

	    if (debug_level >= 3)
		fprintf(stderr,
			"%s: computing filter for channel %ld\n",
			PROG, i);

	    for (k = 0; k <= N; k++)
		filters[i].weights[k] = 0.0;

	    peak = bark_freqs[i];
	    lo = ceil(bark_to_hz(peak + LO_LIM)/df);
	    hi = floor(bark_to_hz(peak + HI_LIM)/df);

	    if (debug_level >= 3)
		fprintf(stderr, "%s: low index = %ld, high index = %ld\n",
			PROG, lo, hi);

	    start = LONG_MAX;
	    end = LONG_MIN;
	    for (j = lo; j <= hi; j++) {

		/* wrap index into range 0..N using (1) symmetry about 0
		 * and (2) periodicity mod 2N */

		k = ABS(j);
		k = (k + N)%(2*N) - N;
		k = ABS(k);

		if (k < start)
		    start = k;
		if (k > end)
		    end = k;

		filters[i].weights[k] +=
		    filter_weight(hz_to_bark(j*df) - peak);

		if (debug_level >= 5)
		    fprintf(stderr, "%s: index = %ld; weight[%ld] = %g\n",
			    PROG, j, k, filters[i].weights[k]);
	    } /* end for (j ... ) */

	    filters[i].start = start;
	    filters[i].end = end;

	    if (debug_level >= 3)
		fprintf(stderr, "%s: start index = %ld, end index = %ld\n",
			PROG, start, end);
	    if (debug_level >= 4) {

		fprintf(stderr, "%s: channel %ld weights ...", PROG, i);

		for (j = start; j <= end; j++) {

		    if ((j - start)%5 == 0)
			fprintf(stderr, "\n [%ld]\t", j);
		    fprintf(stderr, "%14.6g", filters[i].weights[j]);
		}
		fprintf(stderr, "\n");
	    }
	} /* end for (i ... ) */
    } /* end "COMPUTE SUMMATION LIMITS ..." */

    /*
     * CREATE OUTPUT FILE
     */

    /* CREATE HEADER */

    if (debug_level >= 1)
	fprintf(stderr, "%s: creating output header\n", PROG);

    outhd = new_header(FT_FEA);
    add_source_file(outhd, inname, inhd);
    add_comment(outhd, get_cmd_line(argc, argv));
    (void) strcpy(outhd->common.prog, PROG);
    (void) strcpy(outhd->common.vers, VERSION);
    (void) strcpy(outhd->common.progdate, DATE);
    outhd->variable.refer = inhd->variable.refer;

    init_feaspec_hd(outhd, tot_power_def, SPFMT_ARB_FIXED, out_sptyp, NO,
		    out_nfreqs, frame_meth, freqs, sf, frmlen, FLOAT);

    outhd->common.tag = inhd->common.tag;

    if (outhd->common.tag) {

	double		src_sf;		/* value for output file header
					 * item src_sf */

	src_sf = get_genhd_val("src_sf", inhd, -1.0);

	if (src_sf == -1.0)
	    src_sf = sf;

	*add_genhd_d("src_sf", NULL, 1, outhd) = src_sf;

	if (debug_level >= 1)
	    fprintf(stderr, "%s: file is tagged; src_sf = %g\n",
		    PROG, src_sf);

    } else {

	if (debug_level >= 1)
	    fprintf(stderr, "%s: file is not tagged\n", PROG);
    }

    record_freq = get_genhd_val("record_freq", inhd, 0.0);

    if (record_freq != 0.0) {

	update_waves_gen(inhd, outhd, (float) startrec, 1.0f);

	if (debug_level >= 1)
	    fprintf(stderr, "%s: record_freq defined; value is %g\n",
		    PROG, record_freq);

    } else if (!outhd->common.tag) {

	fprintf(stderr,
		"%s: WARNING: file is not tagged "
		"and \"record_freq\" is undefined.\n",
		PROG);
    }

    *add_genhd_l("start", NULL, 1, outhd) = startrec;
    *add_genhd_l("nan", NULL, 1, outhd) = nan;
    *add_genhd_d("add_const", NULL, 1, outhd) = add_const;
    *add_genhd_d("mult_const", NULL, 1, outhd) = mult_const;
    *add_genhd_d("band_low", NULL, 1, outhd) = band_low;
    *add_genhd_d("band_high", NULL, 1, outhd) = band_high;
    *add_genhd_d("bark_low", NULL, 1, outhd) = bark_low;
    *add_genhd_d("bark_high", NULL, 1, outhd) = bark_high;

    (void) add_genhd_f("bark_freqs", bark_freqs, (int) out_nfreqs, outhd);

    /* OPEN FILE */

    if (debug_level >= 1)
	fprintf(stderr, "%s: opening output file\n", PROG);

    outname = eopen(PROG, outname, "w", NONE, NONE, NULL, &outfile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: output file = \"%s\".\n", PROG, outname);

    /* WRITE HEADER */

    if (debug_level >= 1)
	fprintf(stderr, "%s: writing header\n", PROG);

    write_header(outhd, outfile);

    /*
     * SET UP I/O RECORD STRUCTS AND DATA BUFFERS
     */

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: allocating input and output record structures\n",
		PROG);

    inrec = allo_feaspec_rec(inhd, FLOAT);
    inbuf = inrec->re_spec_val;
    inbufim = inrec->im_spec_val;

    outrec = allo_feaspec_rec(outhd, FLOAT);
    outbuf = outrec->re_spec_val;

    /*
     * MAIN READ-WRITE LOOP
     */

    fea_skiprec(infile, startrec - 1, inhd);

    if (debug_level >= 1)
	fprintf(stderr, "%s: reading and processing data\n", PROG);

    for (n = 0;
	 (nan == 0 || n < nan) && get_feaspec_rec(inrec, inhd, infile) != EOF;
	 n++)
    {
	if (debug_level >= 3)
	    fprintf(stderr, "%s: processing record %ld\n",
		    PROG, n + 1);

	if (debug_level >= 4) {

	    fprintf(stderr, "%s: record %ld input data ...", PROG, n + 1);

	    for (j = 0; j < in_nfreqs; j++) {

		if (j%5 == 0)
		    fprintf(stderr, "\n [%ld]\t", j);
		fprintf(stderr, "%14.6g", inbuf[j]);
	    }
	    fprintf(stderr, "\n");
	}

	/* CONVERT TO PWR */

	switch (in_sptyp) {

	case SPTYP_PWR:
	    /* nothing to do */
	    break;
	case SPTYP_DB:
	    for (j = 0; j < in_nfreqs; j++)
		inbuf[j] = exp(LOG10_BY_10 * inbuf[j]);
	    break;
	case SPTYP_REAL:
	    for (j = 0; j < in_nfreqs; j++)
		inbuf[j] = inbuf[j] * inbuf[j];
	    break;
	case SPTYP_CPLX:
	    for (j = 0; j < in_nfreqs; j++)
		inbuf[j] = inbuf[j]*inbuf[j] + inbufim[j]*inbufim[j];
	    break;
	default:
	    ERROR("unrecognized input spec_type");
	}

	if (debug_level >= 4) {

	    fprintf(stderr,
		    "%s: record %ld input data (PWR) ...", PROG, n + 1);

	    for (j = 0; j < in_nfreqs; j++) {

		if (j%5 == 0)
		    fprintf(stderr, "\n [%ld]\t", j);
		fprintf(stderr, "%14.6g", inbuf[j]);
	    }
	    fprintf(stderr, "\n");
	}

	/* COMPUTE WEIGHTED SUMS */

	for (i = 0; i < out_nfreqs; i++) {

	    long    start, end;
	    double  *weights;
	    double  sum;

	    start = filters[i].start;
	    end = filters[i].end;
	    weights = filters[i].weights;
	    sum = 0.0;
	    
	    for (j = start; j <= end; j++)
		sum += weights[j] * inbuf[j];

	    outbuf[i] = sum * df;

	    /* Input "power" is assumed to represent a power spectral
	     * density with units of (unit power)/Hz, such as fft
	     * produces.  The factor df converts the weighted sums into
	     * integrated powers with units of (unit power).  (df is
	     * defined under "COMPUTE SUMMATION LIMITS AND WEIGHTS"
	     * above.) */
	}

	if (debug_level >= 4) {

	    fprintf(stderr, "%s: record %ld output data ...", PROG, n + 1);

	    for (i = 0; i < out_nfreqs; i++) {

		if (i%5 == 0)
		    fprintf(stderr, "\n [%ld]\t", i);
		fprintf(stderr, "%14.6g", outbuf[i]);
	    }
	    fprintf(stderr, "\n");
	}

	/* CONVERT TO OUTPUT SPEC_TYPE */

	switch (out_sptyp) {

	case SPTYP_PWR:
	    /* nothing to do */
	    break;
	case SPTYP_DB:
	    for (i = 0; i < out_nfreqs; i++)
		outbuf[i] = 10*log10(outbuf[i]);
	    break;
	case SPTYP_REAL: /* FALL THROUGH */
	case SPTYP_CPLX:
	    ERROR("unsupported output spec_type");
	    break;
	default:
	    ERROR("unrecognized output spec_type");
	}

	if (debug_level >= 4) {

	    fprintf(stderr, "%s: record %ld output data (%s) ...",
		    PROG, n + 1, sptyp_names[out_sptyp]);

	    for (i = 0; i < out_nfreqs; i++) {

		if (i%5 == 0)
		    fprintf(stderr, "\n [%ld]\t", i);
		fprintf(stderr, "%14.6g", outbuf[i]);
	    }
	    fprintf(stderr, "\n");
	}

	/* OPIONAL LINEAR SCALING OF OUTPUT DATA */

	if (mult_const != 1.0)
	    for (i = 0; i < out_nfreqs; i++)
		outbuf[i] = outbuf[i] * mult_const;

	if (add_const != 0.0)
	    for (i = 0; i < out_nfreqs; i++)
		outbuf[i] = outbuf[i] + add_const;

	if (debug_level >= 4) {

	    fprintf(stderr,
		    "%s: record %ld scaled output data ...", PROG, n + 1);

	    for (i = 0; i < out_nfreqs; i++) {

		if (i%5 == 0)
		    fprintf(stderr, "\n [%ld]\t", i);
		fprintf(stderr, "%14.6g", outbuf[i]);
	    }
	    fprintf(stderr, "\n");
	}

	/* TOT_POWER, FRAME_LEN, AND TAG */

	if (tot_power_def) {

	    *outrec->tot_power = *inrec->tot_power;

	    if (debug_level >= 3)
		fprintf(stderr, "%s: tot_power = %g\n",
			PROG, *inrec->tot_power);
	}

	if (frame_meth == SPFRM_VARIABLE) {

	    *outrec->frame_len = *inrec->frame_len;

	    if (debug_level >= 3)
		fprintf(stderr, "%s: frame_len = %ld\n",
			PROG, *inrec->frame_len);
	}

	if (outhd->common.tag) {

	    *outrec->tag = *inrec->tag;

	    if (debug_level >= 3)
		fprintf(stderr, "%s: tag = %ld\n",
			PROG, *inrec->tag);
	}

	/* WRITE RECORD */

	if (debug_level >= 3)
	    fprintf(stderr, "%s: writing output record %d\n", PROG, n + 1);

	put_feaspec_rec(outrec, outhd, outfile);
    }

    /*
     * WRAP UP
     */

    if (debug_level >= 1)
	fprintf(stderr, "%s: wrote %ld records\n", PROG, n);

    if (n < nan)
	fprintf(stderr,
		"%s: WARNING: premature end of file; "
		"only %d records read\n",
		PROG, n);

    return 0;	/* equivalent to exit(0); */
}


/*
 * Convert a Bark-scale value b to the equivalent linear-scale
 * frequency (in hertz).
 */

static double
bark_to_hz(double b)
{
    return 600.0*sinh(b/6.0);
}


/*
 * Convert a linear-scale frequency f (in hertz) to the equivalent
 * Bark-scale value.
 */

static double
hz_to_bark(double f)
{
    return 6.0*asinh(f/600.0);
}


/*
 * Compute the value of a critical-band weighting function at a point,
 * given the distance b of the point from the peak (in barks)
 */

static double
filter_weight(double b)
{
    double  w;			/* return value (dB) */

    b -= 0.210;			/* adjust for peak at 0 bark;
				 * not 0.215; see man page.
				 */
				/* the other magic numbers below come
				 * come from Wang, Sekey, & Gersho [1] and
				 * Sekey & Hanson [2]; see the man page
				 * for full references.
				 */
    w = 7.0 - 7.5*b - 17.5*sqrt(0.196 + b*b);

    return exp(LOG10_BY_10 * w);	/* convert from dB */
}


/*
 * Inverse sinh function, for use when not provided by the math library.
 */

#ifndef ASINH_AVAILABLE
static double
asinh(double x)
{
    if (x > 0.2) {		/* precise value not critical */

	/* theoretically correct, but loses precision
	 * when x is small in magnitude or negative */

	return log(sqrt(1 + x*x) + x);

    } else if (x < -0.2) {	/* precise value not critical */

	/* theoretically correct, but loses precision
	 * when x is small in magnitude or positive */

	return -log(sqrt(1 + x*x) - x);

    } else {

	double  f, n, p, s, s0;

	/* use power-series expansion when |x| is small */
	f = -x*x;
	n = 1.0;
	p = x;
	s = 0.0;		/* will add in first term (x) at end */

	do {
	    s0 = s;
	    p *= f*n/(n+1.0);
	    n += 2.0;
	    s += p/n;
	} while (s != s0);

	return s + x;
    }
}
#endif
