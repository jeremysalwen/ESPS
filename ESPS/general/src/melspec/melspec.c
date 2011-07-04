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
 *   compute mel-scaled spectra
 */

static char *sccs_id = "@(#)melspec.c	1.1\t7/12/98\tERL";


/* INCLUDE FILES */

#include <stdlib.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>

/* LOCAL CONSTANTS */

#define VERSION	"1.1"		/* must be 7 char or less */
#define DATE	"7/12/98"	/* must be 25 char or less */
#define PROG	"melspec"	/* must be 15 char or less */

#define LOG10_BY_10 0.230258509299404568402
				/* log(10.0)/10.0 */

#define MEL_CONST   1127.01048033415743865
				/* 1000.0/log(1700.0/700.0),
				 * chosen to make hz_to_mel(1000.0)
				 * equal to 1000.0.
				 */

/* LOCAL MACROS */

#define SYNTAX \
USAGE("melspec [-a add_const] [-m mult_const] [-n num_freqs] [-r range]\n" \
      "\t[-x debug_level] [-H  freq_range] [-M  mel_range]\n" \
      "\t[-P  param_file] [-S spec_type] [-W channel_width] [-X]\n" \
      "\tinput.fspec output.fspec")

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

static double	mel_to_hz(double m);	/* convert mel scale to linear */
static double	hz_to_mel(double h);	/* convert linear scale to mel */
static double	filter_weight(double m, double h);
					/* compute filter weights */

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
    long	    out_nfreqs;		/* number of mel values at which
					 * to compute spectral values */

    char	    *r_arg = NULL;	/* r-option argument */
    long	    startrec;		/* first record to process */
    long	    nan;		/* number of records to process,
					 * but 0 means range extends to
					 * end of file */

    char	    *H_arg = NULL;	/* H-option argument */
    double	    band_low;		/* linear-scale frequency
					 * equivalent to mel_low */
    double	    band_high;		/* linear-scale frequency
					 * equivalent to mel_high */

    char	    *M_arg = NULL;	/* M-option argument */
    double	    mel_low;		/* low limit of range of mel-
					 * scale values to be covered */
    double	    mel_high;		/* high limit of range of mel-
					 * scale values to be covered */

    char	    *param_name = NULL;	/* parameter file name */

    char	    *S_arg = NULL;	/* S-option argument */
    char	    *sptypsym;		/* param file entry for spec_type */
    int		    out_sptyp;		/* SPTYPE_PWR or SPTYPE_DB: ouput
					 * spectral values directly or
					 * convert to dB? */

    char	    *W_arg = NULL; 	/* W-option argument */
    double	    channel_width;	/* width in mel of triangular
					 * filters */
    double	    half_width;		/* 0.5*channel_width */

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
    float	    *mel_freqs;		/* filter mel-scale center
					 * frequencies */
    double	    chan_sp;		/* spacing between values
					 * in mel_freqs */
    float	    *freqs;		/* linear-scale equivalents of
					 * values in mel_freqs */

    float	    sf;			/* sf/2 = upper limit of input
					 * frequency range */
    double	    df;			/* interval between frequencies
					 * in input */

    struct filt	    *filters;		/* filter structures */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "a:m:n:r:x:H:M:P:S:W:X")) != EOF)
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
	case 'H':
	    H_arg = optarg;
	    break;
	case 'M':
	    M_arg = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'S':
	    S_arg = optarg;
	    break;
	case 'W':
	    W_arg = optarg;
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
	    "input file spec_type is not SYM_EDGE");

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

    REQUIRE(M_arg == NULL || H_arg == NULL,
	    "both M and H options specified");

    mel_low = 0.0;		/* default */
    mel_high = 0.0;		/* 0 implies use default */
    band_low = 0.0;		/* default */
    band_high = 0.0;		/* 0 implies use default */

    if (M_arg != NULL) {

	frange_switch(M_arg, &mel_low, &mel_high);
	band_low = mel_to_hz(mel_low);
	band_high = mel_to_hz(mel_high);

    } else if (H_arg != NULL) {

	frange_switch(H_arg, &band_low, &band_high);
	mel_low = hz_to_mel(band_low);
	mel_high = hz_to_mel(band_high);

    } else {

	if (symtype("mel_high") != ST_UNDEF) {

	    REQUIRE(symtype("mel_high") == ST_FLOAT,
		    "type of symbol \"mel_high\" is not FLOAT");
	    mel_high = getsym_d("mel_high");
	    band_high = mel_to_hz(mel_high);

	} else if (symtype("band_high") != ST_UNDEF) {

	    REQUIRE(symtype("band_high") == ST_FLOAT,
		    "type of symbol \"band_high\" is not FLOAT");
	    band_high = getsym_d("band_high");
	    mel_high = hz_to_mel(band_high);
	}

	if (symtype("mel_low") != ST_UNDEF) {

	    REQUIRE(symtype("mel_low") == ST_FLOAT,
		    "type of symbol \"mel_low\" is not FLOAT");
	    mel_low = getsym_d("mel_low");
	    band_low = mel_to_hz(mel_low);

	} else if (symtype("band_low") != ST_UNDEF) {

	    REQUIRE(symtype("band_low") == ST_FLOAT,
		    "type of symbol \"band_low\" is not FLOAT");
	    band_low = getsym_d("band_low");
	    mel_low = hz_to_mel(band_low);
	}
    }

    if (mel_high == 0.0) {

	/* If the initial 0 values haven't been changed by option or
	 * param-file entry, or if an explicit 0 was specified, assign 
	 * the default values to band_high and mel_high.  We could
	 * have tested either one.  This depends on the fact that
	 * bark_to_hz(0.0) == 0.0 and hz_to_bark(0.0) == 0.0.
	 */
	band_high = sf/2.0;
	mel_high = hz_to_mel(band_high);
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: mel_low = %g, mel_high = %g,\n"
		"\tband_low = %g, band_high = %g\n",
		PROG, mel_low, mel_high, band_low, band_high);

    /* NUMBER AND SPACING OF CHANNELS */

    if (n_arg != NULL) {

	out_nfreqs = atol(n_arg);

    } else if (symtype("num_freqs") != ST_UNDEF) {

	REQUIRE(symtype("num_freqs") == ST_INT,
		"type of symbol \"num_freqs\" is not INT");
	out_nfreqs = getsym_i("num_freqs");

    } else {
	out_nfreqs = 0;			/* invalid value to indicate need
					 * to compute default from
					 * channel_width */
    }

    if (W_arg != NULL) {

	channel_width = atof(W_arg);

    } else if (symtype("channel_width") != ST_UNDEF) {

	REQUIRE(symtype("channel_width") == ST_FLOAT,
		"type of symbol \"channel_width\" is not FLOAT");
	channel_width = getsym_d("channel_width");

    } else {
	channel_width = 0.0;		/* invalid value to indicate need
					 * to compute default from
					 * output num_freqs */
    }

    if (channel_width > 0.0) {

	if (out_nfreqs == 0)
	    out_nfreqs = LROUND(2.0*(mel_high - mel_low)/channel_width) - 1;

    } else if (out_nfreqs > 0) {

	if (channel_width == 0.0)
	    channel_width = 2.0*(mel_high - mel_low)/(out_nfreqs + 1);

    } else {
	ERROR("both num_freqs and channel_width are unspecified or invalid");
    }

    REQUIRE(channel_width > 0.0,
	    "channel_width not positive");
    REQUIRE(mel_high - mel_low >= channel_width,
	    "single channel bandwidth exceeds total mel-scale range");
    REQUIRE(out_nfreqs > 0,
	    "output num_freqs not positive");

    half_width = 0.5*channel_width;

    if (out_nfreqs > 1) {

	chan_sp = (mel_high - mel_low - channel_width)/(out_nfreqs - 1);

	REQUIRE(chan_sp > 0.0,
		"spacing between channels is not positive");

    } else {
	chan_sp = 1.0;			/* arbitrary valid numeric value */
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: %ld channels, width = %g, spacing = %g\n",
		PROG, out_nfreqs, channel_width, chan_sp);

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

    TRYALLOC(float, out_nfreqs, mel_freqs, "mel_freqs");
    TRYALLOC(float, out_nfreqs, freqs, "freqs");

    REQUIRE(mel_low + half_width >= 0.0,
	    "first filter peak frequency is negative");
    REQUIRE(mel_high - half_width <= hz_to_mel(sf/2.0),
	    "last filter peak frequency exceeds Nyquist");

    for (i = 0; i < out_nfreqs; i++) {

	mel_freqs[i] = mel_low + half_width + i*chan_sp;
	freqs[i] = mel_to_hz(mel_freqs[i]);
    }

    if (X_specified || debug_level >= 2) {

	fprintf(stderr,
		"-------------mel_freqs-------------  "
		"---------------freqs---------------\n");
	fprintf(stderr,
		"  low edge      peak     high edge   "
		"  low edge      peak     high edge \n");
	for (i = 0; i < out_nfreqs; i++)
	    fprintf(stderr,
		    "%11.1f %11.1f %11.1f  %11.1f %11.1f %11.1f\n",
		    mel_freqs[i] - half_width,
		    mel_freqs[i],
		    mel_freqs[i] + half_width,
		    mel_to_hz(mel_freqs[i] - half_width),
		    freqs[i],
		    mel_to_hz(mel_freqs[i] + half_width));
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
	    double  peak;		/* peak mel-scale frequency
					 * of filter */

	    if (debug_level >= 3)
		fprintf(stderr,
			"%s: computing filter for channel %ld\n",
			PROG, i);

	    for (k = 0; k <= N; k++)
		filters[i].weights[k] = 0.0;

	    peak = mel_freqs[i];
	    lo = ceil(mel_to_hz(peak - half_width)/df);
	    hi = floor(mel_to_hz(peak + half_width)/df);

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
		    filter_weight(hz_to_mel(j*df) - peak, half_width);

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
    *add_genhd_d("mel_low", NULL, 1, outhd) = mel_low;
    *add_genhd_d("mel_high", NULL, 1, outhd) = mel_high;
    *add_genhd_d("channel_width", NULL, 1, outhd) = channel_width;

    (void) add_genhd_f("mel_freqs", mel_freqs, (int) out_nfreqs, outhd);

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
 * Convert a mel-scale value m to the equivalent linear-scale
 * frequency (in hertz).
 */

static double
mel_to_hz(double m)
{
    return 700.0*(exp(m/MEL_CONST) - 1.0);
}


/*
 * Convert a linear-scale frequency f (in hertz) to the equivalent
 * mel-scale value.
 */

static double
hz_to_mel(double f)
{
    return MEL_CONST*log(1.0 + f/700.0);
}


/*
 * Compute the value of a triangular filter weighting function
 * at a point, given:
 * - the distance m of the point from the peak (in mels)
 * - the half-width h of the filter.
 */

static double
filter_weight(double m, double h)
{
    double  w;

    w = (m < 0.0) ? (1.0 + m/h) : (1.0 - m/h);
    return (w < 0.0) ? 0.0 : w;
}
