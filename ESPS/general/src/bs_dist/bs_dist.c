/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *   compute Bark spectral distortion
 *
 * Ref:  S. Wang, A. Sekey, & A. Gersho, IEEE J Select Areas in Comm,
 * v.10, n.5 (June 1992) 819-829.
 */

static char *sccs_id = "@(#)bs_dist.c	1.2	9/2/98	ERL";


/* INCLUDE FILES */

#include <stdlib.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <esps/constants.h>

/* LOCAL CONSTANTS */

#define VERSION	"1.2"		/* must be 7 char or less */
#define DATE	"9/2/98"	/* must be 25 char or less */
#define PROG	"bs_dist"	/* must be 15 char or less */

#define LOG2_OVER_10  (0.0693147180559945309417)    /* log(2)/10.0 */

/* LOCAL MACROS */

#define SYNTAX \
USAGE("bs_dist [-a] [-e threshold] [-m distortionType]\n" \
      "\t[-{pr} range1 [-{pr} range2]] [-x debug_level] [-A] [-P paramFile]\n" \
      "\tinFile1.spec inFile2.spec [outFile.fea]")

#define ERROR(text) \
{(void) fprintf(stderr, "%s: %s - exiting\n", PROG, (text)); exit(1);}

#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define TRYALLOC(type, num, var, txt) \
{if (((var) = (type *) malloc((num)*sizeof(type))) == NULL) \
 {(void) fprintf(stderr, "%s: can't allocate memory--%s - exiting\n", \
		 PROG, (txt)); exit(1);}}

#define GET_GENHD_CODED(name, hd, var, txt) \
{if (genhd_type((name), NULL, (hd)) != CODED) \
 {(void) fprintf(stderr, "%s: header item \"%s\" in %s undefined or" \
		 " wrong type - exiting\n", PROG, (name), (txt)); exit(1);} \
 {(var) = *get_genhd_s((name), (hd));}}

#define STRCMP(a, op, b) (strcmp((a), (b)) op 0)

/* SYSTEM FUNCTIONS AND VARIABLES */

extern int	getopt();		/* parse command options */
extern int	optind;			/* used by getopt */
extern char	*optarg;		/* used by getopt */

/* ESPS FUNCTIONS AND VARIABLES */

char		*get_cmd_line(int argc, char **argv);
int		debug_level = 0;

/* LOCAL TYPEDEFS AND STRUCTURES */

/* This enum type is common to bs_dist and mbs_dist.
 * Keep them consistent. */

typedef enum {
    BSD = 1,    MBSD
}		d_type;			/* cf. d_types below */

/* LOCAL FUNCTION DECLARATIONS */

void	print_farray(int filenum, long recnum,
		     char *text, long numelem, float *data);
					/* for debug printouts */

/* STATIC (LOCAL) GLOBAL VARIABLES */

/* This string array definition is common to bs_dist and mbs_dist.
 * Keep them consistent.
 */

static char	*d_types[] = {
    "NONE", "BSD",  "MBSD",
    NULL
};					/* cf. d_type above */


/* MAIN PROGRAM */

int
main(int   argc,
     char  **argv)
{
    int		    ch;			/* command-line option letter */

    long	    i, j, k, n;		/* loop counters and array indices */

    int		    a_specified = NO;	/* a-option specified? (output only
					 * average final distortion with no
					 * output file?) */

    char	    *e_arg = NULL;	/* e-option argument */
    double	    threshold;		/* frame power threshold */
    int		    check_thresh;	/* check input frame powers against
					 * threshold? */

    char	    *m_arg = NULL;	/* m-option argument */
    char	    *distypesym; 	/* param file entry for
					 * distortion_type */ 
    d_type	    distortion_type;	/* BSD or MBSD */

    char	    *r_arg1 = NULL;	/* first r-option argument */
    char	    *r_arg2 = NULL;	/* second r-option argument */
    long	    startrec1;		/* first record to process from
					 * first input file */
    long	    startrec2;		/* first record to process from
					 * second input file */
    int		    startrecs[2]; 	/* startrec1 and startrec2 as
					 * an array */
    long	    nan;		/* number of records to process,
					 * but 0 means range extends to
					 * end of file */

    int		    A_specified = NO;	/* A-option specified? (output
					 * average final distortion?) */

    char	    *param_name = NULL;	/* parameter file name */

    char	    *inname1;		/* first input file name */
    FILE	    *infile1;		/* first input stream */
    struct header   *inhd1;		/* first input file header */
    struct feaspec  *inrec1;		/* input FEA_SPEC record from first
					 * input file */
    float	    *inbuf1;		/* input buffer--data from inrec1 */
    long	    *intag1;		/* pointer to tag in input record */
    int		    freq_format1;	/* freq_format of first input file */
    int		    spec_type1;		/* spec_type of first input file */
    int		    contin1;		/* is first input spectrum a
					 * continuous density? */
    double	    src_sf1;		/* src_sf in first input file */
    int		    tot_power_defined1;	/* tot_power field defined in
					 * first input file? */
    float	    *tot_power1;	/* pointer to tot_power field
					 * in inrec1 */
    int		    num_freqs1;		/* num_freqs in first input file */
    short	    frame_meth1;	/* frame_meth in first input file */
    double	    record_freq1;	/* record_freq in first input file */
    float	    *freqs1;		/* freqs in first input file header */

    char	    *inname2;		/* second input file name */
    FILE	    *infile2;		/* second input stream */
    struct header   *inhd2;		/* second input file header */
    struct feaspec  *inrec2;		/* input FEA_SPEC record from second
					 * input file */
    float	    *inbuf2;		/* input buffer--data from inrec2 */
    long	    *intag2;		/* pointer to tag in input record */
    int		    freq_format2;	/* freq_format of second input file */
    int		    spec_type2;		/* spec_type of second input file */
    int		    contin2;		/* is second input spectrum a
					 * continuous density? */
    double	    src_sf2;		/* src_sf in second input file */
    int		    tot_power_defined2;	/* tot_power field defined in
					 * second input file? */
    float	    *tot_power2;	/* pointer to tot_power field
					 * in inrec2 */
    int		    num_freqs2;		/* num_freqs in second input file */
    short	    frame_meth2;	/* frame_meth in second input file */
    double	    record_freq2;	/* record_freq in second input file */
    float	    *freqs2;		/* freqs in second input file header */

    int		    num_freqs;		/* number of frequencies used in
					 * computation */

    int		    tag_match;	 	/* are input files tagged files with
					 * the same src_sf? */
    int		    rec_freq_match;	/* do input files have the same
					 * non-zero record_freq? */

    float	    *log_wts;		/* perceptual log weights (in dB) for
					 * converting from dB to phones */
    float	    *p_wts;		/* log_wts values in linear form
					 * rather than logarithmic (dB) */

    double	    frame_distortion;	/* distortion for one frame */
    double	    sum_distortion;	/* distortion summed over frames */
    double	    norm;		/* normalizing factor for
					 * overall average distortion */

    char	    *outname;		/* output file name */
    FILE	    *outfile;		/* output stream */
    struct header   *outhd;		/* output file header */
    struct fea_data *outrec;		/* output file record */
    char	    *outfld;		/* name of frame distortion */
    float	    *outdata;		/* pointer to frame distortion field
					 * in output record */
    long	    *outtag;		/* pointer to tag in output record */
    long	    *start;		/* pointer to "start" in output hdr */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "ae:m:p:r:x:AP:")) != EOF)
	switch (ch) {

	case 'a':
	    a_specified = YES;
	    break;
	case 'e':
	    e_arg = optarg;
	    break;
	case 'm':
	    m_arg = optarg;
	    break;
	case 'p':	/* -p is a synonym for -r */
	    /* FALL THROUGH */
	case 'r':
	    if (r_arg1 == NULL) {
		r_arg1 = optarg;
	    } else if (r_arg2 == NULL) {
		r_arg2 = optarg;
	    } else {
		fprintf(stderr, "%s: too may -r options.\n", PROG);
		SYNTAX;
	    }
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'A':
	    A_specified = YES;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    /*
     * PROCESS FILE NAMES.
     */

    if (argc - optind > (a_specified ? 2 : 3)) {

	fprintf(stderr, "%s: too many file names.\n", PROG);
	SYNTAX;
    }

    if (argc - optind < (a_specified ? 2 : 3)) {

	fprintf(stderr, "%s: not enough file names.\n", PROG);
	SYNTAX;
    }

    inname1 = argv[optind++];
    inname2 = argv[optind++];

    REQUIRE(STRCMP(inname1, !=, "-") || STRCMP(inname2, !=, "-"),
	    "input files both standard input");

    if (!a_specified) {

	outname = argv[optind++];

	if (STRCMP(outname, !=, "-")) {

	    REQUIRE(STRCMP(inname1, !=, outname),
		    "output file same as first input file");
	    REQUIRE(STRCMP(inname2, !=, outname),
		    "output file same as second input file");
	}

	if (A_specified) {

	    REQUIRE(STRCMP(outname, !=, "-"),
		    "-A specified, but output file is standard output");
	}
    }

    /*
     * OPEN AND CHECK INPUT FILES.
     */

    inname1 = eopen(PROG, inname1, "r", FT_FEA, FEA_SPEC, &inhd1, &infile1);
    inname2 = eopen(PROG, inname2, "r", FT_FEA, FEA_SPEC, &inhd2, &infile2);

    if (debug_level >= 1) {

	fprintf(stderr, "%s: first input file = \"%s\"\n", PROG, inname1);
	fprintf(stderr, "%s: second input file = \"%s\"\n", PROG, inname2);
    }

    GET_GENHD_CODED("freq_format", inhd1, freq_format1, inname1);
    REQUIRE(freq_format1 == SPFMT_ARB_FIXED,
	    "freq_format in first input file is not ARB_FIXED");
    GET_GENHD_CODED("freq_format", inhd2, freq_format2, inname2);
    REQUIRE(freq_format1 == SPFMT_ARB_FIXED,
	    "freq_format in second input file is not ARB_FIXED");

    GET_GENHD_CODED("spec_type", inhd1, spec_type1, inname1);
    REQUIRE(spec_type1 == ST_DB || spec_type1 == ST_PWR,
	    "spec_type in first input file is not DB or PWR");
    GET_GENHD_CODED("spec_type", inhd2, spec_type2, inname2);
    REQUIRE(spec_type1 == ST_DB || spec_type1 == ST_PWR,
	    "spec_type in second input file is not DB or PWR");

    tot_power_defined1 = (get_fea_type("tot_power", inhd1) != UNDEF);
    tot_power_defined2 = (get_fea_type("tot_power", inhd2) != UNDEF);

    GET_GENHD_CODED("contin", inhd1, contin1, inname1);
    GET_GENHD_CODED("contin", inhd2, contin2, inname2);
    REQUIRE((contin1 && contin2) || (!contin1 && !contin2),
	    "one input contains a spectral density, "
	    "and one contains discrete powers");

    num_freqs1 = (long) get_genhd_val("num_freqs", inhd1, -1.0);
    REQUIRE(num_freqs1 > 0,
	    "num_freqs missing or not positive in first input file");
    num_freqs2 = (long) get_genhd_val("num_freqs", inhd2, -1.0);
    REQUIRE(num_freqs2 > 0,
	    "num_freqs missing or not positive in second input file");

    GET_GENHD_CODED("frame_meth", inhd1, frame_meth1, inname1);
    GET_GENHD_CODED("frame_meth", inhd2, frame_meth2, inname2);

    record_freq1 = get_genhd_val("record_freq", inhd1, 0.0);
    record_freq2 = get_genhd_val("record_freq", inhd2, 0.0);

    REQUIRE(genhd_type("freqs", NULL, inhd1) == FLOAT,
	    "header item \"freqs\" wrong type or missing "
	    "in first input file");
    freqs1 = get_genhd_f("freqs", inhd1);
    REQUIRE(genhd_type("freqs", NULL, inhd2) == FLOAT,
	    "header item \"freqs\" wrong type or missing "
	    "in second input file");
    freqs2 = get_genhd_f("freqs", inhd2);

    src_sf1 = get_genhd_val("src_sf", inhd1, 0.0);
    if (src_sf1 == 0.0)
	src_sf1 = get_genhd_val("sf", inhd1, 0.0);
    src_sf2 = get_genhd_val("src_sf", inhd2, 0.0);
    if (src_sf2 == 0.0)
	src_sf2 = get_genhd_val("sf", inhd2, 0.0);

    if (debug_level >= 1)
	fprintf(stderr, "%s: in %s---\n"
		"\tspec_type = \"%s\", tot_power is%sdefined, contin = %d,\n"
		"\tnum_freqs = %ld, frame_meth = %d (%s), record_freq = %g,\n"
		"\tfile is%stagged, src_sf = %g\n",
		PROG, inname1,
		sptyp_names[spec_type1],
		tot_power_defined1 ? " " : " not ",
		contin1,
		num_freqs1,
		frame_meth1, (idx_ok(frame_meth1, spfrm_names)
			      ? spfrm_names[frame_meth1] : "bad value"),
		record_freq1,
		(inhd1->common.tag) ? " " : " not ",
		src_sf1);
    if (debug_level >= 2) {
	fprintf(stderr, "\tfreqs = {");
	for (i = 0; i < num_freqs1; i++) {
	    if (i%5 == 0)
		fprintf(stderr, "\n\t [%2ld] ", i);
	    fprintf(stderr, " %11.6g", freqs1[i]);
	}
	fprintf(stderr, "}\n");
    }

    if (debug_level >= 1)
	fprintf(stderr, "%s: in %s---\n"
		"\tspec_type = \"%s\", tot_power is%sdefined, contin = %d,\n"
		"\tnum_freqs = %ld, frame_meth = %d (%s), record_freq = %g,\n"
		"\tfile is%stagged, src_sf = %g\n",
		PROG, inname2,
		sptyp_names[spec_type2],
		tot_power_defined2 ? " " : " not ",
		contin2,
		num_freqs2,
		frame_meth2, (idx_ok(frame_meth2, spfrm_names)
			      ? spfrm_names[frame_meth2] : "bad value"),
		record_freq2,
		(inhd2->common.tag) ? " " : " not ",
		src_sf2);
    if (debug_level >= 2) {
	fprintf(stderr, "\tfreqs = {");
	for (i = 0; i < num_freqs2; i++) {
	    if (i%5 == 0)
		fprintf(stderr, "\n\t [%2ld] ", i);
	    fprintf(stderr, " %11.6g", freqs2[i]);
	}
	fprintf(stderr, "}\n");
    }

    tag_match = (inhd1->common.tag && inhd2->common.tag
		 && src_sf1 > 0.0
		 && src_sf2 > 0.9999*src_sf1
		 && src_sf2 < 1.0001*src_sf1);
    rec_freq_match = (record_freq1 > 0.0
		      && record_freq2 > 0.9999*record_freq1
		      && record_freq2 < 1.0001*record_freq1);

    REQUIRE(tag_match || rec_freq_match,
	    "input files are mismatched in record_freq "
	    "and (if tagged) in src_sf");

    for (i = 0;
	 i < num_freqs1 && i < num_freqs2
	 && freqs1[i] < 4000.0 && freqs2[i] < 4000.0;
	 i++)
    {
	REQUIRE(freqs2[i] > 0.99*freqs1[i] && freqs2[i] < 1.01*freqs1[i],
		"frequencies in the input file headers disagree");
    }
    num_freqs = i;

    if (debug_level >= 1)
	fprintf(stderr, "%s: using %d frequencies\n", PROG, num_freqs);

    /*
     * PROCESS OPTIONS AND PARAMETERS.
     */

    (void) read_params(param_name, SC_NOCOMMON, NULL);

    /* OUTPUT SELECTION */

    REQUIRE(!a_specified || !A_specified,
	    "both -a and -A specified");

    /* RECORD RANGE */

    startrec1 = 1;
    startrec2 = 1;
    nan = 0;

    if (r_arg1 != NULL) {

	long	endrec;

	endrec = LONG_MAX;
	lrange_switch(r_arg1, &startrec1, &endrec, NO);
	REQUIRE(endrec >= startrec1,
		"empty range of records specified");

	if (endrec != LONG_MAX)
	    nan = endrec - startrec1 + 1;

	if (r_arg2 != NULL) {

	    endrec = LONG_MAX;
	    lrange_switch(r_arg2, &startrec2, &endrec, NO);
	    REQUIRE(endrec >= startrec2,
		    "empty range of records specified");

	    if (endrec != LONG_MAX) {

		if (nan == 0)
		    nan = endrec - startrec2 + 1;
		else
		    REQUIRE(nan == endrec - startrec2 + 1,
			    "inconsistent range sizes specified");
	    }

	} else {
	    startrec2 = startrec1;
	}

    } else {

	switch (symtype("start")) {

	case ST_INT:
	    startrec1 = startrec2 = getsym_i("start");
	    break;
	case ST_IARRAY:
	    REQUIRE(getsym_ia("start", startrecs, 2) == 2,
		    "size of array parameter \"start\" is not 2");
	    startrec1 = startrecs[0];
	    startrec2 = startrecs[1];
	    break;
	case ST_UNDEF:
	    break;
	default:
	    ERROR("type of symbol \"start\" is not int or int array");
	}

	switch (symtype("nan")) {

	case ST_INT:
	    nan = getsym_i("nan");
	    break;
	case ST_UNDEF:
	    break;
	default:
	    ERROR("type of symbol \"nan\" is not int");
	}
    }

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: start = {%ld, %ld}, nan = %ld\n",
		PROG, startrec1, startrec2, nan);

    REQUIRE(startrec1 >= 1,
	    "range starts before beginning of first input file");
    REQUIRE(startrec2 >= 1,
	    "range starts before beginning of second input file");

    /* DISTORTION TYPE */

    distortion_type = BSD;
    if (m_arg != NULL) {

	distortion_type = lin_search(d_types, m_arg);

    } else if (symtype("distortion_type") != ST_UNDEF) {

	distypesym = getsym_s("distortion_type");

	REQUIRE(distypesym != NULL,
		"type of symbol \"distortion_type\" is not STRING");
	distortion_type = lin_search(d_types, distypesym);

    } else {
	distortion_type = BSD;
    }

    REQUIRE(distortion_type == BSD,
	    "distortion_type is not BSD");

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: distortion_type = \"%s\"\n",
		PROG, d_types[distortion_type]);

    /* THRESHOLD */

    check_thresh = NO;	/* this becomes YES only if all conditions for
			 * checking input frame powers against the
			 * threshold are met:  we are computing BSD, the
			 * threshold is greater than 0, and the tot_power
			 * field is defined in both input files
			 */

    if (distortion_type == BSD) {

	if (e_arg != NULL) {

	    threshold = atof(e_arg);

	} else if (symtype("threshold") != ST_UNDEF) {

	    REQUIRE(symtype("threshold") == ST_FLOAT,
		    "type of symbol \"threshold\" is not FLOAT");
	    threshold = getsym_d("threshold");

	} else {
	    threshold = 0.0;
	}

	REQUIRE(threshold >= 0.0,
		"threshold is negative");

	if (debug_level >= 1)
	    fprintf(stderr,
		    "%s: threshold = %g\n",
		    PROG, threshold);

	if (threshold > 0.0) {

	    REQUIRE(tot_power_defined1,
		    "non-zero threshold specified, "
		    "but no tot_power field defined in first input file");
	    REQUIRE(tot_power_defined2,
		    "non-zero threshold specified, "
		    "but no tot_power field defined in second input file");

	    check_thresh = YES;
	}

	if (debug_level >= 1)
	    fprintf(stderr,
		    "%s: total power of input frames "
		    "will%sbe compared with threshold\n",
		    PROG, (check_thresh ? " " : " not "));
    }

    /* PERCEPTUAL WEIGHTING FACTORS */

    if (symtype("perceptual_weights") != ST_UNDEF) {

	REQUIRE(symtype("perceptual_weights") == ST_FARRAY
		&& symsize("perceptual_weights") >= num_freqs,
		"parameter \"perceptual_weights\" is too short "
		"or not a FLOAT array");

	TRYALLOC(float, symsize("perceptual_weights"), p_wts, "p_wts");
	TRYALLOC(float, symsize("perceptual_weights"), log_wts, "log_wts");
	getsym_fa("perceptual_weights", p_wts, num_freqs);

	for (i = 0; i < num_freqs; i++) {
	    REQUIRE(p_wts[i] > 0.0,
		    "non-positive value in perceptual_weights array");
	    log_wts[i] = 10 * log10(p_wts[i]);
	}

    } else {

	TRYALLOC(float, num_freqs, p_wts, "p_wts");
	TRYALLOC(float, num_freqs, log_wts, "log_wts");

	for (i = 0; i < num_freqs; i++)
	{
	    double  a = 2.6;	/* Numerator coefficient ... */
	    double  b = 1.6;	/* ... and denominator coefficient of bilinear
				 * preemphasis filter.  These magic numbers are
				 * from section VI.B of Wang, Sekey, & Gersho,
				 * (1992).
				 */
	    double  w0 = (a + 1.0)*(a + 1.0)/((b + 1.0)*(b + 1.0));
	    double  two_x = 2.0 * cos(PI*freqs1[i]/4000.0);
	    double  num = (a + two_x) * a + 1.0;
	    double  den = (b + two_x) * b + 1.0;
	    /* 
	     * numerator = |a+z|^2
	     * denominator = |b+z|^2
	     * where z = exp (2i pi f) and f = freqs1[i]/8000
	     */
	    
	    p_wts[i] = (num/den)/w0;
	    log_wts[i] = 10.0 * log10(p_wts[i]);
	}
    }

    if (debug_level >= 2) {

	fprintf(stderr,
		"%s: log perceptual weighting factors (dB) ...",
		PROG);

	for (i = 0; i < num_freqs; i++) {
	    if (i%5 == 0)
		fprintf(stderr, "\n [%2ld]\t", i);
	    fprintf(stderr, " %13.6g", log_wts[i]);
	}

	fprintf(stderr, "\n");
    }

    /*
     * CREATE OUTPUT FILE
     */

    if (!a_specified) {

	/* CREATE HEADER */

	if (debug_level >= 1)
	    fprintf(stderr, "%s: creating output header\n", PROG);

	outhd = new_header(FT_FEA);
	add_source_file(outhd, inname1, inhd1);
	add_source_file(outhd, inname2, inhd2);
	add_comment(outhd, get_cmd_line(argc, argv));
	(void) strcpy(outhd->common.prog, PROG);
	(void) strcpy(outhd->common.vers, VERSION);
	(void) strcpy(outhd->common.progdate, DATE);
	outhd->variable.refer = inhd1->variable.refer;

	if (tag_match) {

	    outhd->common.tag = YES;
	    *add_genhd_d("src_sf", NULL, 1, outhd) = src_sf1;

	    if (debug_level >= 1)
		fprintf(stderr,
			"%s: output file is tagged; src_sf = %g\n",
			PROG, src_sf1);

	} else {

	    outhd->common.tag = NO;

	    if (debug_level >= 1)
		fprintf(stderr, "%s: output file is not tagged\n",
			PROG);
	}

	if (rec_freq_match) {

	    update_waves_gen(inhd1, outhd, (float) startrec1, 1.0f);

	    if (debug_level >= 1)
		fprintf(stderr, "%s: output record_freq defined; "
			"value is %g\n",
			PROG, record_freq1);

	} else {

	    if (debug_level >= 1)
		fprintf(stderr, "%s: output record_freq undefined\n",
			PROG);
	}

	start = add_genhd_l("start", NULL, 2, outhd);
	start[0] = startrec1;
	start[1] = startrec2;
	*add_genhd_l("nan", NULL, 1, outhd) = nan;
	*add_genhd_e("distortion_type",
		     NULL, 1, d_types, outhd) = distortion_type;

	if (distortion_type == BSD) {
	    *add_genhd_d("threshold", NULL, 1, outhd) = threshold;
	}

	(void) add_genhd_f("perceptual_weights", p_wts, num_freqs, outhd);

	outfld = (distortion_type == BSD) ? "BSD" : "MBSD";

	add_fea_fld(outfld, 1, 0, NULL, FLOAT, NULL, outhd);

	/* OPEN FILE */

	if (debug_level >= 1)
	    fprintf(stderr, "%s: opening output file\n", PROG);

	outname = eopen(PROG, outname, "w", NONE, NONE, NULL, &outfile);

	if (debug_level >= 1)
	    fprintf(stderr, "%s: output file = \"%s\"\n", PROG, outname);

	/* WRITE HEADER */

	if (debug_level >= 1)
	    fprintf(stderr, "%s: writing header\n", PROG);

	write_header(outhd, outfile);

    } /* end if (!a_specified) */

   /*
    * SET UP I/O RECORD STRUCTS AND DATA BUFFERS
    */

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: allocating input and output record structrures\n",
		PROG);

    inrec1 = allo_feaspec_rec(inhd1, FLOAT);
    inbuf1 = inrec1->re_spec_val;
    intag1 = inrec1->tag;

    inrec2 = allo_feaspec_rec(inhd2, FLOAT);
    inbuf2 = inrec2->re_spec_val;
    intag2 = inrec2->tag;

    if (!a_specified) {

	outrec = allo_fea_rec(outhd);
	outdata = (float *) get_fea_ptr(outrec, outfld, outhd);
	outtag = &outrec->tag;
    }

    if (check_thresh) {

	tot_power1 = inrec1->tot_power;
	tot_power2 = inrec2->tot_power;
    }

    /*
     * MAIN READ-WRITE LOOP
     */

    fea_skiprec(infile1, startrec1 - 1, inhd1);
    fea_skiprec(infile2, startrec2 - 1, inhd2);

    if (a_specified || A_specified) {

	/* set up to compute overall average distortion */

	sum_distortion = 0.0;
	norm = 0.0;
    }

    if (debug_level >= 1)
	 fprintf(stderr, "%s: reading and processing data\n", PROG);

    /* If nan == 0, the loop terminates, via a break, when the end of
     * either input file is reached.
     * If nan != 0, the loop terminates after nan records have been
     * read and processed, unless the end of an input file is reached
     * sooner.  In the latter case, a "premature end of file" warning
     * is issued. 
     */

    for (n = 0; nan == 0 || n < nan; n++) {

	/* READ INPUT FRAMES */

	if (get_feaspec_rec(inrec1, inhd1, infile1) == EOF) {

	    if (nan != 0)
		fprintf(stderr,
			"%s: WARNING: premature end of file in \"%s\"; "
			"only %d records read\n",
			PROG, inname1, n);
	    break;
	}

	if (get_feaspec_rec(inrec2, inhd2, infile2) == EOF) {

	    if (nan != 0)
		fprintf(stderr,
			"%s: WARNING: premature end of file in \"%s\"; "
			"only %d records read\n",
			PROG, inname2, n);

	    break;
	}

	if (tag_match && inhd1->common.tag && *intag1 != *intag2) {

	    fprintf(stderr, "%s: WARNING: "
		    "tags mismatch in record %ld. (%ld != %ld)\n",
		    PROG, n + 1, *intag1, *intag2);

	    tag_match = NO;
	}

	if (debug_level >= 3)
	    fprintf(stderr, "%s: processing record %ld\n",
		    PROG, n + 1);

	if (debug_level >= 4) {
	    print_farray(1, n + 1, "input data", num_freqs, inbuf1);
	    print_farray(2, n + 1, "input data", num_freqs, inbuf2);
	}

	/* CONVERT TO DB */

	switch (spec_type1) {

	case SPTYP_PWR:
	    for (j = 0; j < num_freqs; j++)
		inbuf1[j] = 10.0 * log10(inbuf1[j]);
	    break;
	case SPTYP_DB:
	    /* nothing to do */
	    break;
	default:
	    /* Should be impossible; we've already checked spec_type1 */
	    ERROR("ERROR determining spec_type of first input file");
	}

	switch (spec_type2) {

	case SPTYP_PWR:
	    for (j = 0; j < num_freqs; j++)
		inbuf2[j] = 10.0 * log10(inbuf2[j]);
	    break;
	case SPTYP_DB:
	    /* nothing to do */
	    break;
	default:
	    /* Should be impossible; we've already checked spec_type2 */
	    ERROR("ERROR determining spec_type of first input file");
	}

	if (debug_level >= 4) {
	    print_farray(1, n + 1, "input data (DB)", num_freqs, inbuf1);
	    print_farray(2, n + 1, "input data (DB)", num_freqs, inbuf2);
	}

	/* APPLY PERCEPTUAL WEIGHTS */

	for (j = 0; j < num_freqs; j++) {

	    inbuf1[j] += log_wts[j];
	    inbuf2[j] += log_wts[j];
	}

	if (debug_level >= 4) {
	    print_farray(1, n + 1,
			 "input data (phones)", num_freqs, inbuf1);
	    print_farray(2, n + 1,
			 "input data (phones)", num_freqs, inbuf2);
	}

	/* PHONES TO SONES */

	for (j = 0; j < num_freqs; j++) {

	    inbuf1[j] = exp((inbuf1[j] - 40) * LOG2_OVER_10);
	    inbuf2[j] = exp((inbuf2[j] - 40) * LOG2_OVER_10);
	}

	if (debug_level >= 4) {
	    print_farray(1, n + 1,
			 "input data (sones)", num_freqs, inbuf1);
	    print_farray(2, n + 1,
			 "input data (sones)", num_freqs, inbuf2);
	}

	/* SQUARED EUCLIDEAN DISTANCE */

	frame_distortion = 0.0;

	for (j = 0; j < num_freqs; j++) {

	    double  diff = inbuf1[j] - inbuf2[j];

	    frame_distortion += diff * diff;
	}

	if (debug_level >= 3)
	    fprintf(stderr, "%s: frame %ld distortion = %g\n",
		    PROG, n + 1, frame_distortion);

	/* OVERALL AVERAGE DISTORTION */

	if (a_specified || A_specified)
	{
	    if (!check_thresh
		|| (*tot_power1 >= threshold && *tot_power2 >= threshold))
	    {
		sum_distortion += frame_distortion;
	    }
	    else /* (check_thresh
		  *  && (*tot_power1 < thresh || *tot_power2 < thresh) */
	    {
		if (debug_level >= 3)
		    fprintf(stderr,
			    "%s: excluding frame %ld distortion---"
			    "power below threshold\n",
			    PROG, n + 1);
	    }

	    for (j = 0; j < num_freqs; j++) {
		norm += inbuf1[j] * inbuf1[j];
	    }

	    if (debug_level >= 3)
		fprintf(stderr,
			"%s: frame %ld cumulative distortion = %g, "
			"norm factor %g\n",
			PROG, n + 1, sum_distortion, norm);
	}

	/* WRITE FRAME OUTPUT */

	if (!a_specified) {

	    *outdata = frame_distortion;

	    if (outhd->common.tag) {

		*outtag = *intag1;
	    }

	    if (debug_level >= 3)
		fprintf(stderr, "%s: writing output record %ld\n",
			PROG, n + 1);

	    put_fea_rec(outrec, outhd, outfile);
	}

    } /* end for (n = 0; ... ) */
    
    /*
     * WRAP UP
     */

    if (debug_level >= 1)
	fprintf(stderr, "%s: processed %ld frames\n", PROG, n);

    if (a_specified || A_specified) {

	printf("%g\n", sum_distortion/norm);
    }

    return 0;	/* equivalent to exit(0); */
}


/*
 * formatted printout of float array for debugging
 */

void
print_farray(int filenum, long recnum, char *text, long numelem, float *data)
{
    long    j;

    fprintf(stderr, "%s: file %d record %ld %s ...",
	    PROG, filenum, recnum, text);

    for (j = 0; j < numelem; j++) {

	if (j%5 == 0)
	    fprintf(stderr, "\n [%ld]\t", j);
	fprintf(stderr, " %13.6g", data[j]);
    }

    fprintf(stderr, "\n");
}
