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
 * Brief description:  real cepstrum and phase factors from sample data
 */

static char *sccs_id = "@(#)cepanal.c	1.1	29 Apr 1997	ERL";

#define VERSION	"1.1"
#define DATE	"29 Apr 1997"
#define PROG	"cepanal"

int    debug_level = 0;

/* INCLUDE FILES */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/window.h>
#include <esps/func.h>
#include <esps/op.h>
#include <esps/types.h>
#include <esps/range_switch.h>
#include <esps/array.h>
#include <esps/get_fft.h>

/* LOCAL CONSTANTS */

#define DEF_ORDER   10			/* default order of FFT */

/* LOCAL MACROS */

#define SYNTAX \
USAGE(PROG " [-l frame_len] [-o order] [-r range] [-w window_type]\n" \
      "\t[-x debug_level] [-S step] input.sd phase_out.fea cepst_out.fea")

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", PROG, (text)); \
  exit(1);}

#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define STRCMP(a, op, b) (strcmp((a), (b)) op 0)

/* SYSTEM FUNCTIONS AND VARIABLES */

extern int	getopt();			/* parse command options */
extern int	optind;				/* used by getopt */
extern char	*optarg;			/* used by getopt */

/* ESPS FUNCTIONS AND VARIABLES */

extern char	*get_cmd_line(int argc, char **argv);

/* LOCAL TYPEDEFS AND STRUCTURES */

/* LOCAL FUNCTION DECLARATIONS */

static void	pr_farray(char *text, long n, float *arr);
static void	pr_fcarray(char *text, long n, float_cplx *arr);

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
/* MAIN PROGRAM */

int
main(int   argc,
     char  **argv)
{
    int		    ch;			/* command-line option letter */

    long	    i;			/* loop index */

    char	    *o_arg = NULL;	/* -o option argument */
    int		    order;		/* order of FFT */
    long	    translen;		/* FFT length */

    char	    *l_arg = NULL;	/* -l option argument */
    long	    framelen;		/* frame length */
    long	    maxlen;		/* translen or framelen, whichever is
					 * greater (length of temp arrays) */

    char	    *S_arg = NULL;	/* -S option argument */
    long	    step;		/* increment between frame endpoints */

    char	    *r_arg = NULL;	/* -r option argument */ 
    long	    startrec;		/* first sample to process */
    long	    endrec;		/* last sample to process */
    long	    nrec;		/* number of samples to process */
    long	    nan;		/* like nrec, but 0 means range extends
					 * from startrec to the end of file */

    char	    *w_arg = NULL;	/* -w option argument */
    int		    win;		/* window type code */

    long	    nfrm;		/* number of frames to process */
    long	    ifrm;		/* index of current frame */

    char	    *sdname;		/* input file name (sampled data) */
    FILE	    *sdfile;		/* input file */
    struct header   *sdhd;		/* input file header */
    struct feasd    *sdrec;		/* input data record structure */
    float	    *sddata;		/* input data array */
    int		    nchan;		/* number of input data channels */
    double	    sf;			/* sampling frequency */

    char	    *phname;		/* first (phase) output file name */
    FILE	    *phfile;		/* phase output file */
    struct header   *phhd;		/* phase file header */
    long	    phlen;		/* length of phase output field */
    struct fea_data *phrec;		/* phase output data record */
    float	    *phdata;		/* phase output data array */

    char	    *cepname;		/* second output file name (cepst) */
    FILE	    *cepfile;		/* cepstrum output file */
    struct header   *cephd;		/* cepstrum file header */
    long	    ceplen;		/* length of cepstrum output field */
    struct fea_data *ceprec;		/* cepstrum output data record */
    float	    *cepdata;		/* cepstrum output data array */

    long	    nvalid;		/* number of valid samples in
					 * buffer (returned by get_feasd
					 * functions) */
    long	    minvalid;		/* value returned by get_feasd_orecs
					 * when called at end of file */
    int		    eof;		/* end of input file encountered? */
    float	    *x, *y;		/* temp storage for FFT and inverse
					 * FFT computation (real and
					 * imaginary parts) */
    float_cplx	    *z;			/* FFT result (complex form) */
    float_cplx	    *logz;		/* Complex log of FFT */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "l:o:r:w:x:S:")) != EOF)
        switch (ch)
	{
	case 'l':
	    l_arg = optarg;
	    break;
	case 'o':
	    o_arg = optarg;
	    break;
	case 'r':
	    r_arg = optarg;
	    break;
	case 'w':
	    w_arg = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'S':
	    S_arg = optarg;
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

    sdname = argv[optind++];
    phname = argv[optind++];
    cepname = argv[optind++];

    REQUIRE(STRCMP(sdname, ==, "-")
	    || STRCMP(sdname, !=, phname) && STRCMP(sdname, !=, cepname),
	    "Output file cannot be same as input file");

    REQUIRE(STRCMP(phname, !=, cepname),
	    "Output files cannot be the same");

    /*
     * OPEN AND CHECK INPUT FILE.
     */

    sdname = eopen(PROG, sdname, "r", FT_FEA, FEA_SD, &sdhd, &sdfile);

    if (debug_level >= 1)
	fprintf(stderr, "%s: input file = \"%s\".\n", PROG, sdname);

    nchan = (int) get_fea_siz("samples", sdhd, NULL, NULL);

    REQUIRE(nchan == 1,
	    "Multichannel data not supported");
    REQUIRE(!is_field_complex(sdhd, "samples"),
	    "Complex input data not supported");

    sf = *get_genhd_d("record_freq", sdhd);

    /*
     * PROCESS OPTIONS.
     */

    /* ORDER, FRAMELEN, STEP */

    order =
	(o_arg != NULL) ? atoi(o_arg) : DEF_ORDER;

    REQUIRE(order >= 0,
	    "Order of FFT can't be negative");

    translen = LROUND(pow(2.0, (double) order));

    framelen =
	(l_arg != NULL) ? atol(l_arg) : translen;
    if (framelen == 0)
	framelen = translen;

    step =
	(S_arg != NULL) ? atol(S_arg) : framelen;
    if (step == 0)
	step = framelen;

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: order = %d, translen = %ld, "
		"framelen = %ld, step = %ld.\n",
		PROG, order, translen, framelen, step);

    /* RANGE */

    startrec = 1;
    endrec = LONG_MAX;
    if (r_arg != NULL)
	lrange_switch(r_arg, &startrec, &endrec, NO);

    REQUIRE(startrec >= 1,
	    "Can't start before beginning of file");
    if (endrec == LONG_MAX)
    {
	nrec = LONG_MAX;
	nfrm = LONG_MAX;
    }
    else
    {
	REQUIRE(endrec >= startrec,
		"Empty range of records specified");

	nrec = endrec - startrec + 1; /* number of samples in range */

	/* Find the number of frames corresponding to "nrec" samples.
	 * For the first frame, "framelen" samples are read.
	 * For each additional frame, if any, "step" additional samples
	 * are read.
	 * Partial frames are rounded _up_ to the nearest whole frame.
	 */
	nfrm =
	    (nrec <= framelen) ? 1
		: (step <= framelen) ? 2 + (nrec - framelen - 1) / step
		    : 1 + (nrec - 1) / step;

	if (framelen + (nfrm - 1) * step > nrec)
	    fprintf(stderr,
		    "%s: Last frame will overrun range by %ld points.\n",
		    PROG, framelen + (nfrm - 1) * step - nrec);
    }

    nan = (nrec == LONG_MAX) ? 0 : nrec;

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: startrec = %ld, endrec = %ld,\n"
		"\tnrec = %ld, nan = %ld, nfrm = %ld.\n",
		PROG, startrec, endrec, nrec, nan, nfrm);

    /* WINDOW */

    win =
	(w_arg != NULL) ? win_type_from_name(w_arg) : WT_RECT;
    REQUIRE(win != WT_NONE, "Invalid window type");
    REQUIRE(win != WT_KAISER, "Sorry, KAISER window not supported");
    REQUIRE(win != WT_ARB, "Sorry, ARB window not supported");

    if (debug_level >= 1)
	fprintf(stderr, "%s: win = %s.\n", PROG, window_types[win]);

    /*
     * SET UP OUTPUT FILES
     */

    /* PHASE FILE */

    phname = eopen(PROG, phname, "w", NONE, NONE, NULL, &phfile);

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: output file for phase = \"%s\".\n", PROG, phname);

    phhd = new_header(FT_FEA);
    add_source_file(phhd, sdname, sdhd);
    (void) strcpy(phhd->common.prog, PROG);
    (void) strcpy(phhd->common.vers, VERSION);
    (void) strcpy(phhd->common.progdate, DATE);
    add_comment(phhd, get_cmd_line(argc, argv));
    update_waves_gen(sdhd, phhd,
		     (float) (startrec + framelen/2), (float) step);
    /* framelen/2 is integer division
     * (for compatibility with fft & fftcep) */

    *add_genhd_l("start", NULL, 1, phhd) = startrec;
    *add_genhd_l("nan", NULL, 1, phhd) = nan;
    *add_genhd_d("sf", NULL, 1, phhd) = sf;
    *add_genhd_l("fft_length", NULL, 1, phhd) = translen;
    *add_genhd_l("frmlen", NULL, 1, phhd) = framelen;
    *add_genhd_l("step", NULL, 1, phhd) = step;
    *add_genhd_e("window_type", NULL, 1, window_types, phhd) = win;
    phlen = translen/2 + 1;
    (void) add_fea_fld("phase", phlen, 1, NULL, FLOAT, NULL, phhd);

    if (debug_level >= 1)
	fprintf(stderr, "%s: size of phase field is %ld.\n", PROG, phlen);
    if (debug_level >= 1)
	fprintf(stderr, "%s: writing phase file header.\n", PROG);

    write_header(phhd, phfile);
    
    /* CEPSTRUM FILE */

    cepname = eopen(PROG, cepname, "w", NONE, NONE, NULL, &cepfile);

    if (debug_level >= 1)
	fprintf(stderr,
		"%s: output file for cepstrum = \"%s\".\n", PROG, cepname);

    cephd = new_header(FT_FEA);
    add_source_file(cephd, sdname, sdhd);
    (void) strcpy(cephd->common.prog, PROG);
    (void) strcpy(cephd->common.vers, VERSION);
    (void) strcpy(cephd->common.progdate, DATE);
    add_comment(cephd, get_cmd_line(argc, argv));
    update_waves_gen(sdhd, cephd,
		     (float) (startrec + framelen/2), (float) step);
    /* framelen/2 is integer division
     * (for compatibility with fft & fftcep) */

    *add_genhd_l("start", NULL, 1, cephd) = startrec;
    *add_genhd_l("nan", NULL, 1, cephd) = nan;
    *add_genhd_d("sf", NULL, 1, cephd) = sf;
    *add_genhd_l("fft_length", NULL, 1, cephd) = translen;
    *add_genhd_l("frmlen", NULL, 1, cephd) = framelen;
    *add_genhd_l("step", NULL, 1, cephd) = step;
    *add_genhd_e("window_type", NULL, 1, window_types, cephd) = win;
    ceplen = translen/2 + 1;
    (void) add_fea_fld("cepstrum", ceplen, 1, NULL, FLOAT, NULL, cephd);

    if (debug_level >= 1)
	fprintf(stderr, "%s: size of cepstrum field is %ld.\n", PROG, ceplen);
    if (debug_level >= 1)
	fprintf(stderr, "%s: writing cepstrum file header.\n", PROG);

    write_header(cephd, cepfile);

    /*
     * ALLOCATE RECORD STRUCTURES AND STORAGE
     */

    if (debug_level >= 1)
	fprintf(stderr, "%s: allocating storage.\n", PROG);

    sdrec = allo_feasd_recs(sdhd, FLOAT, framelen, NULL, NO);
    sddata = (float *) sdrec->data;

    maxlen = MAX(framelen, translen);
    x = (float *) arr_alloc(1, &maxlen, FLOAT, 0);
    y = (float *) arr_alloc(1, &maxlen, FLOAT, 0);
    z = NULL;			/* first call of arr_op will allocate */
    logz = NULL;		/* first call of arr_func will allocate */

    phrec = allo_fea_rec(phhd);
    phdata = (float *) get_fea_ptr(phrec, "phase", phhd);

    ceprec = allo_fea_rec(cephd);
    cepdata = (float *) get_fea_ptr(ceprec, "cepstrum", cephd);

    /*
     * MAIN LOOP
     */

    /* Set up for eof test.  In the case of overlapping frames
     * (framelen > step), the function get_feasd_orecs will return
     * framelen - step when it fails to read any new samples, since
     * it obtains that many valid samples by shifting samples already
     * in the buffer.
     */
    minvalid = (framelen <= step) ? 0 : framelen - step;
    eof = NO;

    /* Get first frame. */

    fea_skiprec(sdfile, startrec - 1, sdhd);
    nvalid = get_feasd_recs(sdrec, 0, framelen, sdhd, sdfile);
    REQUIRE(nvalid > 0,
	    "No samples read from input");
    ifrm = 1;

    if (debug_level >= 1)
	fprintf(stderr, "%s: beginning of main loop.\n", PROG);

    /* Loop, processing a frame, writing the results, and reading a new
     * frame, until we reach either the end of the specified range or the
     * end of the file.
     */

    do
    {
	if (debug_level >= 2)
	    fprintf(stderr, "\n%s: FRAME %ld\n", PROG, ifrm);

	if (nvalid < framelen)
	{
	    eof = YES;
	    fprintf(stderr,
		    "%s: EOF in frame %ld; only %ld points read.\n",
		    PROG, ifrm, nvalid);
	}

	if (debug_level >= 2)
	    pr_farray("input sampled data",
		      (debug_level == 2) ? MIN(framelen, 10) : framelen,
		      sddata);

	/* WINDOW INPUT DATA */

	window(framelen, sddata, x, win, NULL); /* when win is WT_RECT,
						 * this just copies */
	for (i = 0; i < framelen; i++)
	{
	    y[i] = 0.0;		/* supply imag part */
	}
	for (i = framelen; i < translen; i++)
	{
	    x[i] = 0.0;
	    y[i] = 0.0;
	}

	/* GET FFT OF WINDOWED DATA */

	if (debug_level >= 2)
	    pr_farray("windowed sampled data",
		      (debug_level == 2) ? MIN(framelen, 10) : framelen,
		      sddata);

	get_rfft(x, y, order);

	if (debug_level >= 2)
	{
	    pr_farray("FFT of input (real part)",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      x);
	    pr_farray("FFT of input (imag part)",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      y);
	}

	/* TAKE COMPLEX LOG OF FFT */

	z = (float_cplx *) arr_op(OP_CPLX, translen,
				  (char *) x, FLOAT, (char *) y, FLOAT,
				  (char *) z, FLOAT_CPLX);
	logz = (float_cplx *) arr_func(FN_LOG, translen,
				       (char *) z, FLOAT_CPLX,
				       (char *) logz, FLOAT_CPLX);

	if (debug_level >= 2)
	    pr_fcarray("complex log of FFT",
		       (debug_level == 2) ? MIN(framelen, 10) : framelen,
		       logz);

	/* GET PHASE AND WRITE IT OUT */

	(void) arr_func(FN_IM, phlen,
			(char *) logz, FLOAT_CPLX, (char *) phdata, FLOAT);
	put_fea_rec(phrec, phhd, phfile);

	if (debug_level >= 2)
	    pr_farray("phase",
		      (debug_level == 2) ? MIN(phlen, 10) : phlen,
		      phdata);

	/* GET CEPSTRUM AND WRITE IT OUT */

	(void) arr_func(FN_RE, translen,
			(char *) logz, FLOAT_CPLX, (char *) x, FLOAT);
	for (i = 0; i < translen; i++)
	    y[i] = 0.0;

	if (debug_level >= 2)
	    pr_farray("log of magnitude of FFT",
		      (debug_level == 2) ? MIN(translen, 10) : translen,
		      x);

	get_fft_inv(x, y, order);

	for (i = 0; i < ceplen; i++)
	    cepdata[i] = x[i];
	put_fea_rec(ceprec, cephd, cepfile);

	if (debug_level >= 2)
	{
	    pr_farray("cepstrum (Re)",
		      (debug_level == 2) ? MIN(ceplen, 10) : ceplen,
		      x);
	    pr_farray("cepstrum (Im)",
		      (debug_level == 2) ? MIN(ceplen, 10) : ceplen,
		      y);
	}

	/* We go back to process a new frame as long as
	 * (1) we haven't yet processed the number of frames required for
	 *     the specified range,
	 * (2) we didn't hit the end of file while reading the current
	 *     frame, and
	 * (3) we get at least one new sample when we try to read the new
	 *     frame (i.e. nvalid > minvalid; see comment above on
	 *     initialization of minvalid).
	 */
    } while (++ifrm <= nfrm
	     && !eof
	     && ((nvalid = get_feasd_orecs(sdrec, 0,
					   framelen, step, sdhd, sdfile))
		 > minvalid));

    if (debug_level >= 2)
	fprintf(stderr, "\n");
    if (debug_level >= 1)
	fprintf(stderr, "%s: end of main loop; ifrm = %ld.\n",
		PROG, ifrm);

    if (nfrm != LONG_MAX && ifrm <= nfrm)
	fprintf(stderr, "%s: Premature EOF; only %ld frames read.\n",
		PROG, ifrm - 1);

    /*
     * WRITE COMMON AND EXIT
     */

    if (phfile != stdout && cepfile != stdout)
    {
	REQUIRE(startrec <= INT_MAX
		&& putsym_i("start", (int) startrec) != -1,
		"Can't write \"start\" to Common.");
	REQUIRE(nan <= INT_MAX
		&& putsym_i("nan", (int) nan) != -1,
		"Can't write \"nan\" to Common.");
	REQUIRE(putsym_s("prog", PROG) != -1,
		"Can't write \"prog\" to Common.");
	REQUIRE(putsym_s("filename", sdname) != -1,
		"Can't write \"filename\" to Common.");
    }

    exit(0);
    return 0;
}


/* LOCAL FUNCTION DEFINITIONS */

/*
 * For debug printout of float arrays
 */

static void
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

static void
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

