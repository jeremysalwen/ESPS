/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1998 Entropic, Inc.  All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rod Johnson, based on code supplied by Wonho Yang
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *   compute modified Bark spectral distortion
 *
 * Ref:  W Yang, M Benbouchta, & R Yantorno, ICASSP '98 Proc
 * (May 1998) 541-544.
 */

static char *sccs_id = "@(#)mbs_dist.c	1.1	9/10/98	ERL";


/* INCLUDE FILES */

#include <stdlib.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>

/* LOCAL CONSTANTS */

#define VERSION	("1.1")		/* must be 7 char or less */
#define DATE	("9/10/98")	/* must be 25 char or less */
#define PROG	("mbs_dist")	/* must be 15 char or less */

#define FRAME	(320)		/* frame size (samples) */
#define BSIZE	(18)		/* number of Bark frequencies */
#define PHSIZE	(13)		/* sizes of arrays used for interpolating
				 * values in phons (see dbtophon) */

/* LOCAL MACROS */

#define SYNTAX \
USAGE("mbs_dist [-a] ] [-{pr} range1 [-{pr} range2]] [-x debug_level] [-A]\n" \
      "\t[-P paramFile] inFile1.spec inFile2.spec [outFile.fea]")

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
long		lseek();		/* used to check for piped input */

/* ESPS FUNCTIONS AND VARIABLES */

char		*get_cmd_line(int argc, char **argv);
int		debug_level = 0;

/* LOCAL TYPEDEFS AND STRUCTURES */

/* This enum type is common to bs_dist and mbs_dist.
 * Keep them consistent.
 */

typedef enum {
    BSD = 1,    MBSD
}		d_type;			/* cf. d_types below */

/* LOCAL FUNCTION DECLARATIONS */

void	spec_to_pwr(int num,
		    float *src, float *src_im, int src_type, double *dest);
double	frame_pwr(int num, double *pwr_spec, double df);
double	*init_freqs(int num, double fmax);
double	*init_spread_func();
double	*init_thresh(int num, double *freqs);
void	normalize(int num, double *pwr_spec, double mean_pwr, double factor);
int	check_frame(double pwr1, double pwr2, double XTHRESH, double YTHRESH);
void	bk_frq(int num, double *freqs, double *pwr_spec, double *bk_spec);
void	spread(double *bk_spec, double *spread_spec);
void	dbtophon(double *spread_spec, double *phon_spec);
void	phontoson(double *phon_spec, double *sone_spec);
double	measure(double *orig, double *dist, double *noise);
double	sfm(int num, double *pwr_spec);
void	thresh2(double alpha, double *spread_spec, double *noise_thr);

void	print_farray(int filenum, long recnum,
		     char *text, long numelem, float *data);
void	print_darray(int filenum, long recnum,
		     char *text, long numelem, double *data);

/* STATIC (LOCAL) GLOBAL VARIABLES */

/* This string array definition is common to bs_dist and mbs_dist.
 * Keep them consistent.
 */

static char	*d_types[] = {
    "NONE", "BSD",  "MBSD",
    NULL
};				/* cf. d_type above */

static char	*noyes[] = {"NO", "YES", NULL};

static double	*FREQ;		/* frequency scale */

static int	BARK[BSIZE+1] = {
    0, 100, 200, 300, 400, 510, 630, 770, 920, 1080,
    1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400
};				/* Bark frequency */

static double	*S;		/* spreading function */

static double	*ABS_TH;	/* absolute hearing threshold */

static double	eqlcon[PHSIZE][BSIZE-3] = {
{12.0,7.0,4.0,1.0,0.0,0.0,0.0,-0.5,-2.0,-3.0,-7.0,-8.0,-8.5,-8.5,-8.5},
{20.0,17.0,14.0,12.0,10.0,9.5,9.0,8.5,7.5,6.5,4.0,3.0,2.5,2.0,2.5},
{29.0,26.0,23.0,21.0,20.0,19.5,19.5,19.0,18.0,17.0,15.0,14.0,13.5,13.0,13.5},
{36.0,34.0,32.0,30.0,29.0,28.5,28.5,28.5,28.0,27.5,26.0,25.0,24.5,24.0,24.5},
{45.0,43.0,41.0,40.0,40.0,40.0,40.0,40.0,40.0,39.5,38.0,37.0,36.5,36.0,36.5},
{53.0,51.0,50.0,49.0,48.5,48.5,49.0,49.0,49.0,49.0,48.0,47.0,46.5,45.5,46.0},
{62.0,60.0,59.0,58.0,58.0,58.5,59.0,59.0,59.0,59.0,58.0,57.5,57.0,56.0,56.0},
{70.0,69.0,68.0,67.5,67.5,68.0,68.0,68.0,68.0,68.0,67.0,66.0,65.5,64.5,64.5},
{79.0,79.0,79.0,79.0,79.0,79.0,79.0,79.0,78.0,77.5,76.0,75.0,74.5,73.0,73.0},
{89.0,89.0,89.0,89.5,90.0,90.0,90.0,89.5,89.0,88.5,87.0,86.0,85.5,84.0,83.5},
{100.0,100.0,100.0,100.0,100.0,99.5,99.0,99.0,98.5,98.0,96.0,95.0,94.5,93.5,93.0},
{112.0,112.0,112.0,112.0,111.0,110.5,109.5,109.0,108.5,108.0,106.0,105.0,104.5,103.0,102.5},
{122.0,122.0,121.0,121.0,120.5,120.0,119.0,118.0,117.0,116.5,114.5,113.5,113.0,111.0,110.5}
};

static double	phons[PHSIZE] = {
0.0,10.0,20.0,30.0,40.0,50.0,60.0,70.0,80.0,90.0,100.0,110.0,120.0
};


/* MAIN PROGRAM */

int
main(int    argc,
     char   **argv)
{
    int		    ch;			/* command-line option letter */

    long	    i, j, k, n;		/* loop counters and array indices */
    long	    num_proc;		/* number of "processed" frames */

    int		    a_specified = NO;	/* a-option specified? (output only
					 * average final distortion with no
					 * output file?) */

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
    struct feaspec  *inrec1;		/* input FEA_SPEC record from
					 * infile1 */
    float	    *inbuf1;		/* input buffer--data from inrec1 */
    float	    *inbufim1;		/* data from inrec1 (imag part) */
    long	    *intag1;		/* pointer to tag in input record */
    int		    freq_format1;	/* freq_format of infile1 */
    int		    spec_type1;		/* spec_type of infile1 */
    int		    contin1;		/* is first input spectrum a
					 * continuous density? */
    double	    src_sf1;		/* src_sf in infile1 */
    double	    sf1;		/* sf in infile1 */
    int		    num_freqs1;		/* num_freqs in infile1 */
    double	    record_freq1;	/* record_freq in infile1 */
    FILE	    *tmpfile1 = NULL;	/* temp file to allow two passes
					 * over data when infile1 is pipe */
    struct header   *tmphd1 = NULL;	/* header for use with tmpfile1 */
    double	    tot_pwr1;		/* power in frame of infile1 */

    char	    *inname2;		/* second input file name */
    FILE	    *infile2;		/* second input stream */
    struct header   *inhd2;		/* second input file header */
    struct feaspec  *inrec2;		/* input FEA_SPEC record from
					 * infile2 */
    float	    *inbuf2;		/* input buffer--data from inrec2 */
    float	    *inbufim2;		/* data from inrec2 (imag part) */
    long	    *intag2;		/* pointer to tag in input record */
    int		    freq_format2;	/* freq_format of infile2 */
    int		    spec_type2;		/* spec_type of infile2 */
    int		    contin2;		/* is second input spectrum a
					 * continuous density? */
    double	    src_sf2;		/* src_sf in infile2 */
    double	    sf2;		/* sf in infile2 */
    int		    num_freqs2;		/* num_freqs in infile2 */
    double	    record_freq2;	/* record_freq in infile2 */
    FILE	    *tmpfile2 = NULL;	/* temp file to allow two passes
					 * over data when infile2 is pipe */
    struct header   *tmphd2 = NULL;	/* header for use with tmpfile2 */
    double	    tot_pwr2;		/* power in frame of infile2 */

    double	    delta_f;		/* interval between frequencies */
    int		    num_freqs;		/* number of frequencies used in
					 * computation */
    double	    fmax;		/* largest frequency used in
					 * computation */
    int		    tag_match;	 	/* are input files tagged files with
					 * the same src_sf? */
    int		    rec_freq_match;	/* do input files have the same
					 * non-zero record_freq? */

    double	    max_xpower; 	/* max frame power in infile1 */
    double	    max_ypower; 	/* max frame power in infile2 */
    double	    mean_xpower;	/* total power in infile1 */
    double	    mean_ypower;	/* total power in infile2 */
    double	    XTHRESHOLD;		/* original-speech threshold for
					 * including frame in overall
					 * average distortion */
    double	    YTHRESHOLD;		/* processed-speech threshold for
					 * including frame in overall
					 * average distortion */
    double	    *PSX;		/* power spectrum from infile1 */
    double	    *PSY;		/* power spectrum from infile2 */
    double	    BX[BSIZE];		/* Bark spectrum from infile1 */
    double	    BY[BSIZE];		/* Bark spectrum from infile2 */
    double	    CX[BSIZE];		/* spread Bark spect from infile1 */
    double	    CY[BSIZE];		/* spread Bark spect from infile2 */
    double	    CNMT[BSIZE];	/* spread Bark spect noise masking
					 * threshold */
    double	    PX[BSIZE-3]; 	/* spread Bark spect from infile1
					 * in phons */
    double	    PY[BSIZE-3]; 	/* spread Bark spect from infile2
					 * in phons */
    double	    PN[BSIZE-3]; 	/* spread Bark spect of noise
					 * in phons */
    double	    SX[BSIZE-3]; 	/* spread Bark spect from infile1
					 * in sones */
    double	    SY[BSIZE-3]; 	/* spread Bark spect from infile2
					 * in sones */
    double	    SN[BSIZE-3]; 	/* spread Bark spect of noise
					 * in sones */

    double	    alpha;		/* noise masking threshold */

    int 	    included;		/* include current frame in overall
					 * average distortion? */

    double	    frame_distortion;	/* distortion for one frame */
    double	    sum_distortion;	/* distortion summed over frames */

    char	    *outname;		/* output file name */
    FILE	    *outfile;		/* output stream */
    struct header   *outhd;		/* output file header */
    struct fea_data *outrec;		/* output file record */
    char	    *distfld;		/* name of frame distortion field */
    char	    *flagfld;		/* name of "included" flag field */
    float	    *outdata;		/* pointer to frame distortion field
					 * in output record */
    short	    *outflag;		/* pointer to "included" flag field
					 * in output record */
    long	    *outtag;		/* pointer to tag in output record */
    long	    *start;		/* pointer to "start" in output hdr */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "ap:r:x:AP:")) != EOF)
	switch (ch) {

	case 'a':
	    a_specified = YES;
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
    REQUIRE(freq_format1 == SPFMT_SYM_EDGE,
	    "freq_format in first input file is not SYM_EDGE");
    GET_GENHD_CODED("freq_format", inhd2, freq_format2, inname2);
    REQUIRE(freq_format1 == SPFMT_SYM_EDGE,
	    "freq_format in second input file is not SYM_EDGE");

    GET_GENHD_CODED("spec_type", inhd1, spec_type1, inname1);
    GET_GENHD_CODED("spec_type", inhd2, spec_type2, inname2);

    GET_GENHD_CODED("contin", inhd1, contin1, inname1);
    GET_GENHD_CODED("contin", inhd2, contin2, inname2);
    REQUIRE((contin1 && contin2) || (!contin1 && !contin2),
	    "one input contains a spectral density, "
	    "and one contains discrete powers");

    src_sf1 = get_genhd_val("src_sf", inhd1, 0.0);
    if (src_sf1 == 0.0)
	src_sf1 = get_genhd_val("sf", inhd1, 0.0);
    src_sf2 = get_genhd_val("src_sf", inhd2, 0.0);
    if (src_sf2 == 0.0)
	src_sf2 = get_genhd_val("sf", inhd2, 0.0);

    sf1 = get_genhd_val("sf", inhd1, 0.0);
    REQUIRE(sf1 > 0.0,
	    "sf missing or not positive in first input file");
    sf2 = get_genhd_val("sf", inhd2, 0.0);
    REQUIRE(sf2 > 0.0,
	    "sf missing or not positive in second input file");

    num_freqs1 = (long) get_genhd_val("num_freqs", inhd1, -1.0);
    REQUIRE(num_freqs1 > 0,
	    "num_freqs missing or not positive in first input file");
    num_freqs2 = (long) get_genhd_val("num_freqs", inhd2, -1.0);
    REQUIRE(num_freqs2 > 0,
	    "num_freqs missing or not positive in second input file");

    delta_f = sf1 / (2.0 * (num_freqs1 - 1));
    REQUIRE(delta_f < 100.0,
	    "frequency spacing not small enough");
    /* 100.0 is the minimum width in Hz of the 1-bark frequency bins
     * (cf. array BARK).  This just assures that no bins turn up empty.
     * Considerably smaller values are preferable in practice---e.g.
     * 8000.0/1024.
     */
    REQUIRE(sf2 / (2.0 * (num_freqs2 - 1)) < 1.0001*delta_f
	    && sf2 /(2.0 * (num_freqs2 - 1)) > 0.9999*delta_f,
	    "inconsistent frequency spacing in the two input files");

    num_freqs = 1 + ROUND(4000.0/delta_f);
    fmax = (num_freqs - 1) * delta_f;

    if (num_freqs > num_freqs1) {

	num_freqs = num_freqs1;
	fmax = 0.5 * sf1;
    }

    if (num_freqs > num_freqs2) {

	num_freqs = num_freqs2;
	fmax = 0.5 * sf2;
    }

    record_freq1 = get_genhd_val("record_freq", inhd1, 0.0);
    record_freq2 = get_genhd_val("record_freq", inhd2, 0.0);

    if (debug_level >= 1)
	fprintf(stderr, "%s: in %s---\n"
		"\tspec_type = \"%s\", contin = %d, src_sf = %g,\n"
		"\tsf = %g, num_freqs = %ld, record_freq = %g,\n"
		"\tfile is%stagged\n",
		PROG, inname1,
		sptyp_names[spec_type1],
		contin1,
		src_sf1,
		sf1,
		num_freqs1,
		record_freq1,
		(inhd1->common.tag) ? " " : " not ");

    if (debug_level >= 1)
	fprintf(stderr, "%s: in %s---\n"
		"\tspec_type = \"%s\", contin = %d, src_sf = %g,\n"
		"\tsf = %g, num_freqs = %ld, record_freq = %g,\n"
		"\tfile is%stagged\n",
		PROG, inname2,
		sptyp_names[spec_type2],
		contin2,
		src_sf2,
		sf2,
		num_freqs2,
		record_freq2,
		(inhd2->common.tag) ? " " : " not ");

    if (debug_level >= 1)
	fprintf(stderr, "%s: num_freqs = %d\n", PROG, num_freqs);

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
		     NULL, 1, d_types, outhd) = MBSD;

	distfld = "MBSD";
	add_fea_fld(distfld, 1, 0, NULL, FLOAT, NULL, outhd);
	flagfld = "MBSD_included";
	add_fea_fld(flagfld, 1, 0, NULL, CODED, noyes, outhd);

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
    inbufim1 = inrec1->im_spec_val;
    intag1 = inrec1->tag;

    inrec2 = allo_feaspec_rec(inhd2, FLOAT);
    inbuf2 = inrec2->re_spec_val;
    inbufim2 = inrec2->im_spec_val;
    intag2 = inrec2->tag;

    if (!a_specified) {

	outrec = allo_fea_rec(outhd);
	outdata = (float *) get_fea_ptr(outrec, distfld, outhd);
	outflag = (short *) get_fea_ptr(outrec, flagfld, outhd);
	outtag = &outrec->tag;
    }

    TRYALLOC(double, num_freqs, PSX, "first power spectrum");
    TRYALLOC(double, num_freqs, PSY, "second power spectrum");

    /*
     * FIRST PASS
     */

    fea_skiprec(infile1, startrec1 - 1, inhd1);
    fea_skiprec(infile2, startrec2 - 1, inhd2);

    if (lseek(fileno(infile1), 0L, SEEK_CUR) < 0) {

	if (debug_level >= 1)
	    fprintf(stderr, "%s: first input is a pipe; making temp file\n",
		    PROG);

	tmpfile1 = tmpfile();
	tmphd1 = copy_header(inhd1);	/* get version without Esignal
					 * header or the like; else we may
					 * get garbage by writing temp
					 * file as FEA and reading back as
					 * another format*/
	write_header(tmphd1, tmpfile1);
    }

    if (lseek(fileno(infile2), 0L, SEEK_CUR) < 0) {

	if (debug_level >= 1)
	    fprintf(stderr, "%s: second input is a pipe; making temp file\n",
		    PROG);

	tmpfile2 = tmpfile();
	tmphd2 = copy_header(inhd2);	/* get version without Esignal
					 * header or the like; else we may
					 * get garbage by writing temp
					 * file as FEA and reading back as
					 * another format*/
	write_header(tmphd2, tmpfile2);
    }

    max_xpower = 0.0;
    max_ypower = 0.0;
    mean_xpower = 0.0;
    mean_ypower = 0.0;

    if (debug_level >= 1)
	 fprintf(stderr, "%s: reading data---first pass\n", PROG);

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
	    fprintf(stderr, "%s: read record %ld\n",
		    PROG, n + 1);

	if (tmpfile1 != NULL) {

	    if (debug_level >= 3)
		fprintf(stderr, "%s: writing record to temp file 1\n",
			PROG);

	    put_fea_rec(inrec1->fea_rec, tmphd1, tmpfile1);
	}

	if (tmpfile2 != NULL) {

	    if (debug_level >= 3)
		fprintf(stderr, "%s: writing record to temp file 2\n",
			PROG);

	    put_fea_rec(inrec2->fea_rec, tmphd2, tmpfile2);
	}

	if (debug_level >= 4) {
	    print_farray(1, n + 1, "input data", num_freqs, inbuf1);
	    print_farray(2, n + 1, "input data", num_freqs, inbuf2);
	}

	/* CONVERT TO PWR */

	spec_to_pwr(num_freqs, inbuf1, inbufim1, spec_type1, PSX);
	spec_to_pwr(num_freqs, inbuf2, inbufim2, spec_type2, PSY);

	if (debug_level >= 4) {
	    print_darray(1, n + 1, "input data (PWR)", num_freqs, PSX);
	    print_darray(2, n + 1, "input data (PWR)", num_freqs, PSY);
	}

	tot_pwr1 = frame_pwr(num_freqs, PSX, delta_f);
	tot_pwr2 = frame_pwr(num_freqs, PSY, delta_f);

	if (debug_level >= 3)
	    fprintf(stderr, "%s: tot_pwr = {%g, %g}\n",
		    PROG, tot_pwr1, tot_pwr2);

	mean_xpower += tot_pwr1;
	mean_ypower += tot_pwr2;

	if (max_xpower < tot_pwr1)
	    max_xpower = tot_pwr1;
	if (max_ypower < tot_pwr2)
	    max_ypower = tot_pwr2;
    }

    if (n != 0) {

	mean_xpower /= n;
	mean_ypower /= n;
    }


    if (debug_level >= 1)
	 fprintf(stderr,
		 "%s: end of first pass; read %ld records\n"
		 "\tmean_xpower = %g, mean_ypower = %g\n"
		 "\tmax_xpower = %g, max_ypower = %g\n",
		 PROG, n,
		 mean_xpower, mean_ypower,
		 max_xpower, max_ypower);

    nan = n;

    if (tmpfile1 != NULL) {

	fclose(infile1);
	infile1 = tmpfile1;
    }

    if (tmpfile2 != NULL) {

	fclose(infile2);
	infile2 = tmpfile2;
    }

    rewind(infile1);
    rewind(infile2);

    /* The factor 375000.0/mean_{x,y}power is the normalizing factor that
     * is applied by the function "normalize".  The factor FRAME converts
     * power to energy for compatibility with program "mbsd".
     */
    XTHRESHOLD =
	max_xpower * FRAME
	    * (375000.0 / mean_xpower) * pow(10.0, -1.5); /* 15 dB below */
    YTHRESHOLD =
	max_ypower * FRAME
	    * (375000.0 / mean_ypower) * pow(10.0, -3.5); /* 35 dB below */
    
    if (debug_level >= 1)
	fprintf(stderr, "%s: XTHRESHOLD = %g, YTHRESHOLD = %g\n",
		PROG, XTHRESHOLD, YTHRESHOLD);

    FREQ = init_freqs(num_freqs, fmax);

    if (debug_level >= 2)
	print_darray(0, 0, "freqs", num_freqs, FREQ);
    
    S = init_spread_func();

    if (debug_level >= 2)
	print_darray(0, 0, "spread function", 2*BSIZE - 1, &S[1 - BSIZE]);

    ABS_TH = init_thresh(num_freqs - 1, FREQ);

    if (debug_level >= 2)
	print_darray(0, 0,
		     "absolute hearing thresh", BSIZE, ABS_TH);

    /*
     * MAIN READ-WRITE LOOP
     */

    inhd1 = read_header(infile1);
    inhd2 = read_header(infile2);

    if (tmpfile1 == NULL) {

	fea_skiprec(infile1, startrec1 - 1, inhd1);
    }

    if (tmpfile2 == NULL) {

	fea_skiprec(infile2, startrec2 - 1, inhd2);
    }

    /* set up to compute overall average distortion */

    sum_distortion = 0.0;
    num_proc = 0;

    if (debug_level >= 1)
	 fprintf(stderr, "%s: reading data---second pass\n", PROG);

    for (n = 0; n < nan; n++) {

	/* READ INPUT FRAMES */

	REQUIRE(get_feaspec_rec(inrec1, inhd1, infile1) != EOF,
		"premature end of file in rereading first input file");
	REQUIRE(get_feaspec_rec(inrec2, inhd2, infile2) != EOF,
		"premature end of file in rereading second input file");

	if (debug_level >= 3)
	    fprintf(stderr, "%s: read record %ld\n",
		    PROG, n + 1);

	if (debug_level >= 4) {
	    print_farray(1, n + 1, "input data", num_freqs, inbuf1);
	    print_farray(2, n + 1, "input data", num_freqs, inbuf2);
	}

	/* CONVERT TO PWR */

	spec_to_pwr(num_freqs, inbuf1, inbufim1, spec_type1, PSX);
	spec_to_pwr(num_freqs, inbuf2, inbufim2, spec_type2, PSY);

	if (debug_level >= 4) {
	    print_darray(1, n + 1, "input data (PWR)", num_freqs, PSX);
	    print_darray(2, n + 1, "input data (PWR)", num_freqs, PSY);
	}

	/* NORMALIZE */

	normalize(num_freqs, PSX, mean_xpower, FRAME*delta_f);
	normalize(num_freqs, PSY, mean_ypower, FRAME*delta_f);

	if (debug_level >= 4) {
	    print_darray(1, n + 1,
			 "normalized power spectrum", num_freqs, PSX);
	    print_darray(2, n + 1,
			 "normalized power spectrum", num_freqs, PSY);
	}

	/* CHECK WHETHER FRAME IS TO BE INCLUDED */

	tot_pwr1 = frame_pwr(num_freqs, PSX, 1.0);
	tot_pwr2 = frame_pwr(num_freqs, PSY, 1.0);
	included = check_frame(tot_pwr1, tot_pwr2, XTHRESHOLD, YTHRESHOLD);

	if (debug_level >= 3)
	    fprintf(stderr,
		    "%s: tot_pwr = {%g, %g}\n"
		    "\tframe%sincluded in overall distortion\n",
		    PROG, tot_pwr1, tot_pwr2,
		    (included) ? " " : " NOT ");

	/* BARK SPECTRUM */

	bk_frq(num_freqs, FREQ, PSX, BX);
	bk_frq(num_freqs, FREQ, PSY, BY);

	if (debug_level >= 4) {
	    print_darray(1, n + 1, "critical-band spectrum", BSIZE, BX);
	    print_darray(2, n + 1, "critical-band spectrum", BSIZE, BY);
	}

	/* SPREAD BARK SPECTRUM */

	spread(BX, CX);
	spread(BY, CY);

	if (debug_level >= 4) {
	    print_darray(1, n + 1,
			 "spread critical-band spectrum", BSIZE, CX);
	    print_darray(2, n + 1,
			 "spread critical-band spectrum", BSIZE, CY);
	}

	/* CONVERT TO PHON */

	dbtophon(CX, PX);
	dbtophon(CY, PY);

	if (debug_level >= 4) {
	    print_darray(1, n + 1,
			 "spread spectrum (phon)", BSIZE - 3, PX);
	    print_darray(2, n + 1,
			 "spread spectrum (phon)", BSIZE - 3, PY);
	}

	/* NOISE MASKING THRESHOLD */

	alpha = sfm(num_freqs, PSX);

	if (debug_level >= 3)
	    fprintf(stderr, "%s: spectral flatness %g\n", PROG, alpha);

	thresh2(alpha, CX, CNMT);

	if (debug_level >= 4)
	    print_darray(0, n + 1, "noise masking threshold", BSIZE, CNMT);

	dbtophon(CNMT, PN);

	if (debug_level >= 4)
	    print_darray(0, n + 1,
			 "noise masking threshold (phon)", BSIZE - 3, PN);

	/* CONVERT TO SONE */

	phontoson(PX, SX);
	phontoson(PY, SY);
	phontoson(PN, SN);

	if (debug_level >= 4) {
	    print_darray(1, n + 1,
			 "spread spectrum (sone)", BSIZE - 3, SX);
	    print_darray(2, n + 1,
			 "spread spectrum (sone)", BSIZE - 3, SY);
	    print_darray(0, 0,
			 "noise spectrum (sone)", BSIZE - 3, SN);
	}

	/* COMPUTE FRAME DISTORTION */

	frame_distortion = measure(SX, SY, SN);

	if (debug_level >= 3)
	    fprintf(stderr, "%s: frame %ld distortion %g\n",
		    PROG, n + 1, frame_distortion);

	/* OVERALL AVERAGE DISTORTION */

	if (included) {

	    num_proc++;
	    sum_distortion += frame_distortion;
	}

	/* WRITE FRAME OUTPUT */

	if (!a_specified) {

	    *outdata = frame_distortion;
	    *outflag = included;

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
	fprintf(stderr,
		"%s: sum of %ld distortions out of %ld processed is %g.\n",
		PROG, num_proc, n, sum_distortion);

    if (a_specified || A_specified) {

	printf("%g\n", (num_proc == 0) ? 0 : sum_distortion/num_proc);
    }

    return 0;	/* equivalent to exit(0); */
}


/*
 * Convert FEA_SPEC data to spec_type PWR
 */

void
spec_to_pwr(int     num,
	    float   *src,
	    float   *src_im,
	    int     src_type,
	    double  *dest)
{
    int     j;

    switch (src_type)
    {
    case SPTYP_PWR:
	for (j = 0; j < num; j++)
	    dest[j] = src[j];
	break;
    case SPTYP_DB:
	for (j = 0; j < num; j++)
	    dest[j] = pow(10.0, 0.1*src[j]);
	break;
    case SPTYP_REAL:
	for (j = 0; j < num; j++)
	    dest[j] = src[j]*src[j];
	break;
    case SPTYP_CPLX:
	for (j = 0; j < num; j++)
	    dest[j] = src[j]*src[j] + src_im[j]*src_im[j];
	break;
    default:
	ERROR("unrecognized spec_type");
    }
}

/*
 * Integrate power spectral density over frame
 */

double
frame_pwr(int       num,
	  double    *pwr_spec,
	  double    df)
{
    int     j;
    double  sum;

    sum = 0.5*(pwr_spec[0] + pwr_spec[num-1]);

    for (j = 1; j < num - 1; j++)
	sum += pwr_spec[j];

    return sum*df;
}

/* Initialize an array of equally spaced frequencies */

double *
init_freqs(int      num,
	   double   fmax)
{
    int     j;
    double  *freqs;
    double  den;

    TRYALLOC(double, num, freqs, "frequencies");

    den = (double) (num - 1);
    for (j = 0; j < num; j++)
	freqs[j] = (j / den) * fmax;

    return freqs;
}

/*
 * Initialize spread function.
 */

double *
init_spread_func()
{
    static double spread_func[2*BSIZE-1];
    double  *S = &spread_func[BSIZE-1];
    int     k;
    double  x;

    for (k = 1 - BSIZE; k < BSIZE; k++) {

	x = (double) k + 0.474;
	x = 15.81 + 7.5*x - 17.5*sqrt(1.0 + x*x);
	S[k] = pow(10.0, 0.1*x);
    }

    return S;
}


/*
 * Initialize absolute hearing threshold in dB by the formula of Terhardt.
 *
 *  thrshld(f) = { 3.64(f/1000)^(-0.8) - 6.5exp[-0.6(f/1000 - 3.3)^2]
 *                 + 0.001(f/1000)^4 }
 *
 * Reference :  Terhardt, E., Stoll, G. and Seewann, M, "Algorithm for
 *        extraction of pitch and pitch salience from complex tonal
 *        signals", J. Acoust. Soc. Am., vol. 71(3), Mar., 1982.
 */

double *
init_thresh(int num, double *freqs)
{
    static double ABS_TH[BSIZE];
    int     i, j, k;
    double  f;
    double  *L;
    double  fsq, t, SUM;

    TRYALLOC(double, num, L, "working storage for \"init_thresh\"");

    L[0] = 0.0;

    for (i = 1; i < num; i++) {

        f = freqs[i]/1000.0;
        fsq = f * f;
        t = f - 3.3;
        L[i] =
	    3.64 / pow(f, 0.8)
		- 6.5 * exp(-0.6 * t * t)
		    + 0.001 * fsq * fsq;
    }

    for (i = 1; i <= BSIZE; i++) {

        SUM = 0.0;
        k = 0;

        for (j = 1; j < num; j++) {

            if (BARK[i-1] <= freqs[j] && freqs[j] < BARK[i]) {

                SUM += L[j];
                k++;
            }
        }

        if (k == 0)
            ABS_TH[i-1] = 0.0;
        else
            ABS_TH[i-1] = SUM / k;
    }

    return ABS_TH;
}


/*
 * Normalize a power spectrum to an overall average power of
 * 375000.0, assuming that "mean_pwr" gives the value before
 * normalization, and "pwr_spec" indicates the current frame.
 * The original "mbsd" normalizes the _unwindowed_ sampled
 * data to an overall RMS of 1000.0.  This is as close as we
 * can come in terms of the mean square of the hanning-windowed
 * signal.  (The mean square value of the hanning window function
 * is 0.375).
 */

void
normalize(int       num,
	  double    *pwr_spec,
	  double    mean_pwr,
	  double    factor)
{
    int     j;
    double  ratio;

    if (mean_pwr == 0)
	return;

    ratio = factor*375000.0/mean_pwr;

    for (j = 0; j < num; j++)
	pwr_spec[j] *= ratio;

    pwr_spec[0] *= 0.5;
}


/*
 * Decide whether to include current frame in computation of overall
 * average distortion.
 */

int
check_frame(double  pwr1,
	    double  pwr2,
	    double  XTHRESH,
	    double  YTHRESH)
{
    return (pwr1 > XTHRESH && pwr2 > YTHRESH);
}


/*
 * Compute critical-band spectrum from power spectrum.
 */

void
bk_frq(int	num,
       double	*freqs,
       double	*pwr_spec,
       double	*bk_spec)
{
    int     i, j;

    for (i = 0; i < BSIZE; i++) {

	bk_spec[i] = 0.0;

	for (j = 0; j < num; j++) {

	    if (BARK[i] <= freqs[j] && freqs[j] < BARK[i+1])
		bk_spec[i] += pwr_spec[j];
	}
    }
}


/*
 * Apply spreading function to critical-band spectrum
 * to compute spread critical-band spectrum.
 */

void
spread(double *bk_spec, double *spread_spec)
{
    int     i, j;

    for (i = 0; i < BSIZE; i++) {

	spread_spec[i] = 0;

	for (j = 0; j < BSIZE; j++)
	    spread_spec[i] += S[i-j] * bk_spec[j];
    }
}


/*
 * Convert spread Bark spectrum to phons.
 */

void
dbtophon(double *spread_spec, double *phon_spec)
{
    int     i, j;
    double  t1;
    double  Ti;

    for (i = 0; i < BSIZE-3; i++) {

	Ti = 10.0 * log10(spread_spec[i+3]);

	for (j = 0; j < PHSIZE; j++) {

	    if (Ti < eqlcon[j][i])
		break;
	}

	if (j == 0) {

	    phon_spec[i] = phons[0];

	} else if (j == PHSIZE) {

	    phon_spec[i] = phons[PHSIZE-1];

	} else {

	    t1 = (Ti - eqlcon[j-1][i]) / (eqlcon[j][i] - eqlcon[j-1][i]);
	    phon_spec[i] = phons[j-1] + t1 * (phons[j] - phons[j-1]);
	}
    }
}


/*
 * Convert spectrum in phons to sones.
 */

void
phontoson(double *phon_spec, double *sone_spec)
{
    int     i;

    for (i = 0; i < BSIZE-3; i++) {

        if (phon_spec[i] >= 40.0) {

	    sone_spec[i] = pow(2.0, 0.1*(phon_spec[i] - 40.0));
	}
	else {

	    sone_spec[i] = pow(phon_spec[i] / 40.0, 2.642);
	}
    }
}


/*
 * Compute frame distortion.
 */

double
measure(double *orig, double *dist, double *noise)
{
    int     i;
    double  d;
    double  t;

    d = 0.0;

    for (i = 0; i < BSIZE - 3; i++) {

	t = fabs(orig[i] - dist[i]) - noise[i];

	if (t > 0.0)
	    d += t;
    }

    return d;
}


/*
 * Spectral flatness measure (based on the ratio of the arithmetic
 * and geometric means).
 */

double
sfm(int num, double *pwr_spec)
{
    double  alpha;
    double  a_mean;		/* arithmetic mean */
    double  g_mean;		/* geometric mean */
    int     i;

    a_mean = 0;
    g_mean = 0;

    for (i = 0; i < num; i++) {

	a_mean += pwr_spec[i];
	g_mean += log(pwr_spec[i]);
    }

    a_mean = a_mean / (double) num;
    g_mean = exp(g_mean / (double) num);

    alpha = 10.0 * log10(g_mean / a_mean) / (-60.0);

    if (alpha > 1.0)
	alpha = 1.0;

    return alpha;
}


/*
 * Noise masking threshold.
 */

void
thresh2(double alpha, double *spread_spec, double *noise_thr)
{
    int     i;
    double  t;

    for (i = 0; i < BSIZE; i++) {

        t = alpha * (14.5 + (double) i + 1.0) + (1.0 - alpha) * 5.5;
	t = 10.0 * log10(spread_spec[i]) - t;

	if (t < ABS_TH[i])
	    t = ABS_TH[i];

	noise_thr[i] = pow(10.0, t/10.0);
    }
}


/*
 * Formatted printout of float array for debugging.
 */

void
print_farray(int    filenum,
	     long   recnum,
	     char   *text,
	     long   numelem,
	     float  *data)
{
    long    j;

    if (filenum == 0)
	fprintf(stderr, "%s: %s ...", PROG, text);
    else
	fprintf(stderr, "%s: file %d record %ld %s ...",
		PROG, filenum, recnum, text);

    for (j = 0; j < numelem; j++) {

	if (j%5 == 0)
	    fprintf(stderr, "\n [%ld]\t", j);
	fprintf(stderr, " %13.6g", data[j]);
    }

    fprintf(stderr, "\n");
}


/*
 * Formatted printout of double array for debugging
 */

void
print_darray(int    filenum,
	     long   recnum,
	     char   *text,
	     long   numelem,
	     double *data)
{
    long    j;

    if (filenum == 0)
	fprintf(stderr, "%s: %s ...", PROG, text);
    else
	fprintf(stderr, "%s: file %d record %ld %s ...",
		PROG, filenum, recnum, text);

    for (j = 0; j < numelem; j++) {

	if (j%5 == 0)
	    fprintf(stderr, "\n [%ld]\t", j);
	fprintf(stderr, " %13.6g", data[j]);
    }

    fprintf(stderr, "\n");
}
