/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1997 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:  sample data from real cepstrum and phase factors
 */

static char *sccs_id = "@(#)cepsyn.c	1.2	5/14/97	ERL";

#define VERSION	"1.2"
#define DATE	"5/14/97"
#define PROG	"cepsyn"

int    debug_level = 0;

/* INCLUDE FILES */

#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/window.h>
#include <esps/func.h>
#include <esps/op.h>

/* LOCAL CONSTANTS */

/* LOCAL MACROS */

#define SYNTAX \
USAGE(PROG " [-r range1 [-r range2]] [-t output_type] [-x debug_level]\n" \
      "\t[-W] phase_in.fea cepst_in.fea output.sd")

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", PROG, (text)); \
  exit(1);}

#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define STRCMP(a, op, b) (strcmp((a), (b)) op 0)

/* SYSTEM FUNCTIONS AND VARIABLES */

int	getopt();		/* parse command-line options */
int	optind;			/* used by getopt */
char	*optarg;		/* used by getopt */

/* ESPS FUNCTIONS AND VARIABLES */

char	*get_cmd_line(int argc, char **argv);
char	*zero_fill(long num, int type, char *dest);

/* LOCAL TYPEDEFS AND STRUCTURES */

/* LOCAL FUNCTION DECLARATIONS */

int	is_type_numeric(int type);
void	pr_farray(char *text, long n, float *arr);
void	pr_fcarray(char *text, long n, float_cplx *arr);

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
/* MAIN PROGRAM */

int
main(int   argc,
     char  **argv)
{
    int		    ch;			/* command-line option letter */

    char	    *r_arg[2];		/* -r option arguments */
    int		    r_num = 0;		/* number of -r options */
    long	    startrec[2];	/* first record to process
					 * in each input file */
    long	    endrec[2];		/* last record to process
					 * in each input file */
    long	    nrec;		/* number of records to process */
    long	    nan;		/* like nrec, but 0 means continue
					 * processing until end of file */
    long	    irec;		/* index of current record */

    char	    *t_arg = NULL;	/* -t option argument */
    int		    sdtype;		/* code for output data type */

    int		    W_flag = NO;	/* -W option specified? */
    int		    doing_windows;	/* try to normalize overlapped
					 * output sample sums by sums of
					 * window weights? */
    int		    req_overlap;	/* required overlap between frames
					 * for normalization to work */
    int		    wtype;		/* window type */
    float	    *win;		/* window */
    float	    *wt;		/* summed weights (win values) for
					 * normalizing samples */

    long	    translen;		/* Fourier transform length */
    int		    order;		/* Fourier transform order */
    long	    i, j;		/* loop indices */

    char	    *phname;		/* phase input file name */
    struct header   *phhd;		/* phase input file header */
    FILE	    *phfile;		/* phase input file */
    long	    phlen;		/* length of phase field */
    struct fea_data *phrec;		/* phase input data record */
    float	    *phdata;		/* phase input data array */

    char	    *cepname;		/* cepstrum input file name */
    struct header   *cephd;		/* cepstrum input file header */
    FILE	    *cepfile;		/* cepstrum input file */
    long	    ceplen;		/* length of cepstrum field */
    struct fea_data *ceprec;		/* cepstrum input data record */
    float	    *cepdata;		/* cepstrum input data array */
    

    double	    sf;			/* sampling frequency (Hz) */
    long	    framelen;		/* sampled-data frame length */
    double	    record_freq;	/* input record rate */
    long	    step;		/* increment between frame endpoints */
    long	    buflen;		/* length of output sample buffer */
    long	    wrap;		/* circular shift for centering
					 * frame in array of length translen */
    long	    lolim, hilim;	/* limits of region in buffer holding
					 * data to be written */

    char	    *sdname;		/* output sampled-data file name */
    struct header   *sdhd;		/* output sampled-data file header */
    FILE	    *sdfile;		/* output sampled-data file */
    struct feasd    *sdrec;		/* output data record structure */
    float	    *sddata;		/* output data array */
    double	    start_time;		/* output starting time (sec) */

    float	    *x, *y;		/* temp storage for FFT and inverse
					 * FFT computation (real and
					 * imaginary parts) */
    float_cplx	    *z;			/* FFT result (complex form) */
    float_cplx	    *expz;		/* Complex exponential of FFT */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "r:t:x:W")) != EOF)
        switch (ch)
	{
	case 'r':
	    if (r_num < 2)
		r_arg[r_num++] = optarg;
	    else
	    {
		fprintf(stderr, "%s: too many -r options.\n", PROG);
		SYNTAX;
	    }
	    break;
	case 't':
	    t_arg = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'W':
	    W_flag = YES;
	    break;
	default:
	    SYNTAX;
	}

    /*
     * PROCESS FILE NAMES.
     */

    if (argc - optind > 3)
    {
	fprintf(stderr, "%s: too many file names.\n", PROG);
	SYNTAX;
    }
    if (argc - optind < 3)
    {
	fprintf(stderr, "%s: not enough file names.\n", PROG);
	SYNTAX;
    }

    phname = argv[optind++];
    cepname = argv[optind++];
    sdname = argv[optind++];

    REQUIRE(STRCMP(sdname, ==, "-")
	    || STRCMP(sdname, !=, phname) && STRCMP(sdname, !=, cepname),
	    "Output file cannot be same as input file");

    REQUIRE(STRCMP(phname, !=, cepname),
	    "Input files cannot be the same");

    /*
     * OPEN AND CHECK INPUT FILES.
     */

    /* PHASE FILE */

    phname = eopen(PROG, phname, "r", FT_FEA, NONE, &phhd, &phfile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: first input file = \"%s\".\n", PROG, phname);

    REQUIRE(get_fea_type("phase", phhd) == FLOAT,
	    "Field \"phase\" undefined or wrong type in first input file");

    phlen = get_fea_siz("phase", phhd, NULL, NULL);

    if (debug_level >= 1)
	fprintf(stderr, "%s: \"phase\" field length = %ld.\n", PROG, phlen);

    /* CEPSTRUM FILE */

    cepname = eopen(PROG, cepname, "r", FT_FEA, NONE, &cephd, &cepfile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: first input file = \"%s\".\n",
		PROG, cepname);

    REQUIRE(get_fea_type("cepstrum", cephd) == FLOAT,
	    "Field \"cepstrum\" undefined or wrong type in second input file");

    ceplen = get_fea_siz("cepstrum", cephd, NULL, NULL);

    if (debug_level >= 1)
	fprintf(stderr, "%s: \"cepstrum\" field length = %ld.\n",
		PROG, ceplen);

    /* CHECK FIELD SIZES AND GET ORDER */

    REQUIRE(ceplen == phlen,
	    "Fields \"phase\" and \"cepstrum\" must have the same length");
    REQUIRE(ceplen > 0,
	    "Fields \"phase\" and \"cepstrum\" have length 0");

    translen = (ceplen == 1) ? 1 : 2*(ceplen - 1);
    order = (int) (0.5 + log((double) translen) / log(2.0));

    if (debug_level >= 1)
	fprintf(stderr, "%s: translen = %ld, order = %d.\n",
		PROG, translen, order);

    REQUIRE(translen == (long) (0.5 + pow(2.0, (double) order)),
	    "Length of fields \"phase\" and \"cepstrum\" "
	    "doesn't correspond to any integer order");

    /* GET SAMPLING FREQ, FRAME LENGTH, OTHER HEADER ITEMS */

    /* Rely on phase file for these, since the generics added by cepanal
     * are more likely to be intact there than in the cepstrum file.
     * But warn if the files are inconsistent.
     */

    /* Sampling freq */

    sf = get_genhd_val("sf", phhd, 0.0);
    REQUIRE(sf > 0.0,
	    "Header item \"sf\" missing or invalid in phase file");

    if (genhd_type("sf", NULL, cephd) != HD_UNDEF)
    {
	double	cepsf = get_genhd_val("sf", cephd, 0.0);

	if (cepsf > 1.00001*sf || cepsf < 0.99999*sf)
	    fprintf(stderr,
		    "%s: Inconsistent \"sf\" header items in input files.\n",
		    PROG);
    }

    /* Frame length */

    framelen = (long) (0.5 + get_genhd_val("frmlen", phhd, 0.0));
    REQUIRE(framelen > 0,
	    "Header item \"frmlen\" missing or invalid in phase file");

    if (genhd_type("frmlen", NULL, cephd) != HD_UNDEF
	&& framelen != (long) (0.5 + get_genhd_val("frmlen", cephd, 0.0)))
    {
	fprintf(stderr,
		"%s: Inconsistent \"frmlen\" header items "
		"in input files.\n",
		PROG);
    }

    if (framelen > translen)
	fprintf(stderr,
		"%s: Frame length (%ld) exceeds transform length (%sd).\n",
		PROG, framelen, translen);
    
    wrap = MAX(0, (translen - framelen)/2);

    /* Record freq */

    record_freq = get_genhd_val("record_freq", phhd, 0.0);
    REQUIRE(record_freq > 0.0,
	    "Header item \"record_freq\" missing or invalid in phase file");

    if (genhd_type("record_freq", NULL, cephd) != HD_UNDEF
	&& record_freq != get_genhd_val("record_freq", cephd, 0.0))
    {
	fprintf(stderr,
		"%s: Inconsistent \"record_freq\" header items "
		"in input files.\n",
		PROG);
    }

    /* Step size */

    step = (long) (0.5 + get_genhd_val("step", phhd, 0.0));
/*!*//* Check consistency */

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: sf = %g, framelen = %ld,\n"
		"\trecord_freq = %g, step = %ld.\n",
		PROG, sf, framelen, record_freq, step);

    /* Window type */

    wtype =
	(genhd_type("window_type", NULL, phhd) != CODED) ? WT_RECT
	    : *get_genhd_s("window_type", phhd);

    /* This may be reset to WT_RECT when the -W option is processed
     * below.
     */

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: wtype = %d (%s).\n",
		PROG, wtype, window_types[wtype]);

    /*
     * PROCESS OPTIONS
     */

    /* RANGE */

    nrec = LONG_MAX;
    startrec[0] = startrec[1] = 1;
    endrec[0] = endrec[1] = LONG_MAX;

    for (i = 0; i < r_num; i++)
    {
	lrange_switch(r_arg[i], &startrec[i], &endrec[i], NO);
	REQUIRE(startrec[i] >= 1,
		"Can't start before beginning of file");
	REQUIRE(endrec[i] >= startrec[i],
		"Empty range of records specified");

	if (endrec[i] != LONG_MAX)
	{
	    if (nrec == LONG_MAX)
		nrec = endrec[i] - startrec[i] + 1;
	    else
		REQUIRE(endrec[i] - startrec[i] + 1 == nrec,
			"Inconsistent range lengths specified");
	}
    }

    if (r_num == 1)
    {
	startrec[1] = startrec[0];
	endrec[1] = endrec[0];
    }

    nan = (nrec == LONG_MAX) ? 0 : nrec;

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: startrec = {%ld, %ld}, endrec = {%ld, %ld},\n"
		"\tnrec = %ld, nan = %ld.\n",
		PROG, startrec[0], startrec[1], endrec[0], endrec[1],
		nrec, nan);

    /* OUTPUT TYPE */

    if (t_arg != NULL)
    {
	sdtype = lin_search(type_codes, t_arg);
	REQUIRE(is_type_numeric(sdtype),
		"Invalid output type");
    }
    else
	sdtype = FLOAT;

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: output type = %d (%s).\n",
		PROG, sdtype, type_codes[sdtype]);

    /* -W FLAG */

    if (W_flag)
	wtype = WT_RECT;

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: -W option specified; wtype reset to %d (WT_RECT).\n",

		PROG, WT_RECT);
    /*
     * SET UP OUTPUT FILE
     */

    sdname = eopen(PROG, sdname, "w", NONE, NONE, NULL, &sdfile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: output file = \"%s\".\n", PROG, sdname);

    sdhd = new_header(FT_FEA);

    add_source_file(sdhd, phname, phhd);
    add_source_file(sdhd, cepname, cephd);
    (void) strcpy(sdhd->common.prog, PROG);
    (void) strcpy(sdhd->common.vers, VERSION);
    (void) strcpy(sdhd->common.progdate, DATE);
    add_comment(sdhd, get_cmd_line(argc, argv));

    /* Get output start_time from start_time in phase file.
     * Check cepstrum file for consistency.
     */

    start_time = (get_genhd_val("start_time", phhd, 0.0)
		  - (double) (framelen/2) / sf
		  + (startrec[0] - 1)/record_freq);

    if (debug_level >= 1)
	fprintf(stderr, "%s: output start_time = %g.\n", PROG, start_time);

    if (genhd_type("start_time", NULL, cephd) != HD_UNDEF
	&& start_time != (get_genhd_val("start_time", cephd, 0.0)
			  - (double) (framelen/2) / sf
			  + (startrec[1] - 1)/record_freq))
    {
	fprintf(stderr,
		"%s: Inconsistent \"start_time\" header items "
		"in input files.\n",
		PROG);
    }

    /* Set up for FEA_SD. */

    init_feasd_hd(sdhd, sdtype, 1, &start_time, NO, sf);

    if (debug_level >= 1)
	fprintf(stderr, "%s: writing output file header.\n", PROG);

    write_header(sdhd, sdfile);

    /*
     * ALLOCATE RECORD STRUCTURES AND STORAGE
     */

    if (debug_level >= 1)
	fprintf(stderr, "%s: allocating storage.\n", PROG);

    phrec = allo_fea_rec(phhd);
    phdata = (float *) get_fea_ptr(phrec, "phase", phhd);

    ceprec = allo_fea_rec(cephd);
    cepdata = (float *) get_fea_ptr(ceprec, "cepstrum", cephd);

    x = (float *) arr_alloc(1, &translen, FLOAT, 0);
    y = (float *) arr_alloc(1, &translen, FLOAT, 0);
    z = NULL;			/* first call of arr_op will allocate */
    expz = NULL;		/* first call of arr_func will allocate */

    buflen = MAX(step, MAX(translen, framelen));
    sdrec = allo_feasd_recs(sdhd, FLOAT, buflen, NULL, NO);
    sddata = (float *) sdrec->data;

    /*
     * COMPUTE SUMMED WEIGHTS
     */


    /* Get the window coefficients by windowing a vector of all 1's.
     * (The window module really ought to provide a way to get the
     * coefficients directly.)
     */
    win = (float *) arr_alloc(1, &framelen, FLOAT, 0);
    for (j = 0; j < framelen; j++)
	win[j] = 1.0;
    window(framelen, win, win, wtype, NULL);
    
    /* The number of zero values at the beginning and end of the
     * window.  We need this much overlap between frames to avoid
     * dividing by zero when normalizing output samples by summed
     * window weights.
     */
    req_overlap = 0;
    for (j = 0; j < framelen/2 && win[j] == 0.0; j++)
	req_overlap += 1;
    for (j = framelen - 1; j >= framelen/2 && win[j] == 0.0; j--)
	req_overlap += 1;
    
    doing_windows = (framelen - step >= req_overlap);
    if (!doing_windows)
    {
	fprintf(stderr, "%s: Insufficient overlap between frames; "
		"can't use summed weights\n"
		"\tto normalize output samples.\n",
		PROG);
    }
    else /* doing windows */
    {
	/* Overlapped sum of window weights with the same overlap and
	 * alignment as the data frames.
	 */
	wt = (float *) arr_alloc(1, &step, FLOAT, 0);
	for (i = 0; i < step; i++)
	{
	    wt[i] = 0.0;
	    j = i - wrap%step;
	    if (j < 0)
		j += step;
	    for ( ; j < framelen; j += step)
		wt[i] += win[j];
	}
	
	if (debug_level >= 1)
	    pr_farray("weights",
		      (debug_level == 1) ? MIN(step, 10) : step,
		      wt);
	}

    /*
     * MAIN LOOP
     */

    if (debug_level >= 1)
	fprintf(stderr, "%s: beginning of main loop.\n", PROG);

    (void) zero_fill(buflen, FLOAT, (char *) sddata);
    lolim = wrap;

    fea_skiprec(phfile, startrec[0] - 1, phhd);
    fea_skiprec(cepfile, startrec[1] - 1, cephd);

    for (irec = 0;
	 irec < nrec && (get_fea_rec(phrec, phhd, phfile) != EOF
			 && get_fea_rec(ceprec, cephd, cepfile) != EOF);
	 irec++)
    {
	if (debug_level >= 2)
	{
	    fprintf(stderr, "\n%s: FRAME %ld\n", PROG, irec + 1);
	    pr_farray("input phase data",
		      (debug_level == 2) ? MIN(phlen, 10) : phlen,
		      phdata);
	    pr_farray("input cepstrum data",
		      (debug_level == 2) ? MIN(ceplen, 10) : ceplen,
		      cepdata);
	}

	/* GET FFT OF CEPSTRUM INPUT */

	x[0] = cepdata[0];
	y[0] = 0.0;
	for (i = 1, j = translen - 1; i < ceplen; i++, j--)
	{
	    x[i] = x[j] = cepdata[i];
	    y[i] = y[j] = 0.0;
	}

	get_rfft(x, y, order);

	if (debug_level >= 2)
	{
	    pr_farray("FFT of input (Re)",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      x);
	    pr_farray("FFT of input (Im)",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      y);
	}

	/* COMBINE FFT WITH PHASE INPUT AND TAKE EXP */

	y[0] = phdata[0];
	for (i = 1, j = translen - 1; i < phlen; i++, j--)
	{
	    y[i] = phdata[i];
	    y[j] = -phdata[i];
	}

	z = (float_cplx *) arr_op(OP_CPLX, translen,
				  (char *) x, FLOAT, (char *) y, FLOAT,
				  (char *) z, FLOAT_CPLX);
	expz = (float_cplx *) arr_func(FN_EXP, translen,
				       (char *) z, FLOAT_CPLX,
				       (char *) expz, FLOAT_CPLX);

	if (debug_level >= 2)
	    pr_fcarray("complex exp of FFT",
		       (debug_level == 2) ? MIN(framelen, 10) : framelen,
		       expz);

	(void) arr_func(FN_RE, translen,
			(char *) expz, FLOAT_CPLX, (char *) x, FLOAT);
	(void) arr_func(FN_IM, translen,
			(char *) expz, FLOAT_CPLX, (char *) y, FLOAT);

	if (debug_level >= 2)
	{
	    pr_farray("real part of exp",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      x);
	    pr_farray("imag part of exp",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      y);
	}

	/* TAKE INVERSE FFT OF EXP */

	get_fft_inv(x, y, order);

	if (debug_level >= 2)
	{
	    pr_farray("real part of inverse transform",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      x);
	    pr_farray("imag part of inverse transform",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      y);
	}

	/* WRITE RESULT */

	for (i = 0, j = wrap; i < translen; i++, j++) {
	    if (j == translen) j = 0;
	    sddata[j] += x[i];
	}

	if (lolim >= step)
	    lolim -= step;
	else
	{
	    if (doing_windows)
		for (i = lolim; i < step; i++)
		    sddata[i] /= wt[i];

	    if (debug_level >= 2)
		fprintf(stderr, "%s: writing frame of output.\n", PROG);
		
	    put_feasd_recs(sdrec, lolim, step - lolim, sdhd, sdfile);
	    lolim = 0;
	}

	for (i = 0, j = step; j < buflen; i++, j++)
	    sddata[i] = sddata[j];
	for (i = buflen - step; i < buflen; i++)
	    sddata[i] = 0.0;
    }

    if (debug_level >= 2)
	fprintf(stderr, "\n");
    if (debug_level >= 1)
	fprintf(stderr, "%s: end of main loop; irec = %ld.\n",
		PROG, irec);

    if (nrec != LONG_MAX && irec < nrec)
	fprintf(stderr, "%s: Premature EOF; only %ld frames read.\n",
		PROG, irec);

    /* FLUSH BUFFER */

    hilim = wrap + framelen - step;
    if (lolim < hilim)
    {
	if (doing_windows)
	for (i = lolim, j = 0; i < hilim; i++, j++)
	{
	    if (j == step)
		j = 0;
	    sddata[i] /= wt[j];
	}

	put_feasd_recs(sdrec, lolim, hilim - lolim, sdhd, sdfile);
    }

    /*
     * WRITE COMMON AND EXIT
     */

    if (sdfile != stdout)
    {
	REQUIRE(putsym_i("start", 1) != -1,
		"Can't write \"start\" to Common.");
	REQUIRE(irec <= INT_MAX
		&& putsym_i("nan", irec) != -1,
		"Can't write \"nan\" to Common.");
	REQUIRE(putsym_s("prog", PROG) != -1,
		"Can't write \"prog\" to Common.");
	REQUIRE(putsym_s("filename", sdname) != -1,
		"can't write \"filename\" to Common");
    }

    exit(0);
}


/* LOCAL FUNCTION DEFINITIONS */

/*
 * Return TRUE if "type" is a valid code for a numeric ESPS data type.
 */
/*!*//* Function is in library. */
/*
static int
is_type_numeric(int type)
{
    switch (type)
    {
    case BYTE:
    case SHORT:
    case LONG:
    case FLOAT:
    case DOUBLE:
    case BYTE_CPLX:
    case SHORT_CPLX:
    case LONG_CPLX:
    case FLOAT_CPLX:
    case DOUBLE_CPLX:
        return YES;
    default:
        return NO;
    }
}
*/

/*
 * For debug printout of float arrays
 */

void
pr_farray(char    *text,
	  long    n,
	  float   *arr)
{
    long    i;

    fprintf(stderr, "%s: %s -- %d points:\n", PROG, text, n);
    for (i = 0; i < n; i++)
    {
	fprintf(stderr, "%g ", arr[i]);
	if (i%5 == 4 || i == n - 1)
	    fprintf(stderr, "\n");
    }
}


/*
 * For debug printout of float_cplx arrays
 */

void
pr_fcarray(char		*text,
	   long		n,
	   float_cplx	*arr)
{
    long    i;

    fprintf(stderr, "%s: %s -- %d points:\n", PROG, text, n);
    for (i = 0; i < n; i++)
    {
	fprintf(stderr, "[%g %g] ", arr[i].real, arr[i].imag);
	if (i%2 == 1 || i == n - 1)
	    fprintf(stderr, "\n");
    }
}
