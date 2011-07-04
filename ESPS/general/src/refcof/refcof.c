/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * History:  
 *  Originally by Brian Sublett;
 *  Revised by David Burton (added -m option)
 *  Revised by John Shore (added -r and -p options, removed ndrec 
 *          dependence, fixed a bunch of bugs)
 *  Rewritten by John Shore for ESPS 3.0 (added choice of spectrum 
 *          analysis method, used simpler SD buffer method). 
 *  Revised by Shore to have range option specify full range rather than
 *            first frame
 *  Revised by Shore to use overlapping frames and windowing.  
 *  Revised by Shore to add fBURG, VBURG, STRCOV, STRCOV1; and also to 
 *             to create compute_rc;
 * 
 * Purpose:  This program reads data from an ESPS FEA_SD file and computes the
 * reflection coefficients and power for the purposes of plotting the maximum
 * entropy spectrum.  The output is written to an ESPS FEA_ANA file. 
 *
 */

static char *sccs_id = "@(#)refcof.c	3.28	9/9/98	ESI/ERL";

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/anafea.h>
#include <esps/window.h>
#include <esps/ana_methods.h>

/*** User defines go here. ***/

#define WT_PREFIX "WT_"

#define SYNTAX \
USAGE("refcof [-P param] [-{pr} range] [-l frame_len] [-e preemphasis] \n \
 [-S step] [-w window_type] [-m method] [-o order] [-d ] [-Z] \n \
 [-x debug_level] [-s sinc_n] [-c conv_test] [-i max_iter]  \n \
 [-z] file.sd file.rc")

#define DEBUG(n) if (debug_level >= n) Fprintf

#define Fprintf (void)fprintf
#define DEF_ORDER 15
#define DEF_FRAMELEN 1000
#define FILNAMSIZE 40		/* Maximum size of filter name + 4 */

#define ERROR_EXIT(text) {(void) fprintf(stderr, "refcof: %s - exiting\n", text); exit(1);}


long		atol();
char           *eopen();
char           *get_cmd_line();
double          getsym_d();
void            put_sd_recs();
void		pr_farray();
void            symerr_exit();
void            lrange_switch();

int             debug_level = 0;	/* debugging level */
char	       *local_no_yes[] = {"NO", "YES", NULL};
int             initflag = 0; /* needed by bestauto, but I don't know why */
char	       *ProgName = "refcof";
char	       *nomemory = "refcof: couldn't allocate enough memory";

static long	n_rec();
static void     get_range();


main(argc, argv)
    int             argc;
    char           *argv[];
{
    FILE           *fopen(), *fpin, *fpout;
    char           *in_file, *out_file;
    struct header  *ih, *oh;
    struct anafea  *recp;
    char           *param_file = NULL;
    int             i, k;
    float          *x, *px, *rc, fst;
    double         *r;
    int		    first = 1;	/* flag for get_sd_orec */
    long	    step;	/* step size for shifting frames */
    long            start = 1;
    long	    nan = LONG_MAX; 
    int		    win;	/* window type code */
    long	    frame_len = -1;  /* frame length */
    int		    actsize;
    int		    sincn = 0;
    int             order = DEF_ORDER;
    int             c;
    long            nframes = 1;
    double          atof();
    float           gain;
    char           *cmd_line;	/* to hold command line */
    extern char    *optarg;
    extern          optind;
    char           *analysis;   /*analysis method type */
    int		    method;
    char	    *window_type; /* name of type of window to apply to data */
    char	    *pref_w_type; /* window type name with added prefix */
    float           preemp = 0;
    int		    wflag = 0; /* flag for window option*/
    int		    zflag = 0; /* flag for -z silent option*/
    int		    Zflag = 0; /* flag for -Z don't zero-fill option*/
    int		    dflag = 0; /* flag for dc remove option*/
    int             mflag = 0;	/* flag for analysis method option */
    int             pflag = 0;	/* flag for range option */
    int             lflag = 0;	/* flag for -l frame_len option */
    int		    Sflag = 0;  /* flag for -S option (step size)*/
    int		    sflag = 0; /* flag for -s option*/
    int             oflag = 0;	/* flag for order option */
    int             cflag = 0;	/* flag for -c option */
    int             iflag = 0;	/* flag for -i option */
    int             eflag = 0;  /* flag for -e option */
    char           *prange = NULL;	/* string for range specification
					 * (-p) */
    int             more = 1;	/* flag to indicate more sampled data */
    double         record_freq; /* holds sampling frequency*/
    double         strcov_test = 1e-5; /*convergence test for STRCOV */
    int            strcov_maxiter = 20;   /* max iterations for STRCOV */

    char           *Version = "3.28";
    char           *Date = "9/9/98";

   /* Initialization.    */

    cmd_line = get_cmd_line(argc, argv);	/* store copy of command line */

    /* Check the command line options. */
    while ((c = getopt(argc, argv, "P:p:r:x:m:o:l:s:S:w:c:i:e:dzZ")) != EOF) {
	switch (c) {
	case 'P':
	    param_file = optarg;
	    break;
	case 'p':
	case 'r':
	    prange = optarg;
	    pflag++;
	    break;
	case 'l':
	    frame_len = atoi(optarg);
	    lflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'm':
	    analysis = optarg;
	    mflag++;
	    break;
	case 'o':
	    order = atoi(optarg);
	    oflag++;
	    break;
        case 'e':
	    preemp = atof(optarg);
	    eflag++;
	    break;
	case 'd':
	    dflag++;
	    break;
	case 'z':
	    zflag++;
	    break;
	case 'Z':
	    Zflag++;
	    break;
	case 'w':
	    window_type = optarg;
	    wflag++;
	    break;
	case 'S':
	    step = atol(optarg);
	    Sflag++;
	    break;
	case 's':
	    sincn = atoi(optarg);
	    sflag++;
	    break;
	case 'c':
	    strcov_test = atof(optarg);
	    cflag++;
	    break;
	case 'i':
	    strcov_maxiter = atoi(optarg);
	    iflag++;
	    break;
	default:
	    SYNTAX;
	}
    }
    /*
     * open input and output files
     */
    if (optind < argc)
	in_file = eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_SD, &ih, &fpin);
    else {
	Fprintf(stderr, "%s: no input file specified.\n", ProgName);
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "Input file is %s\n", in_file);
    if (is_field_complex(ih, "samples") == YES){
	ERROR_EXIT("does not support complex input data");
    }
    if (optind < argc)
	out_file = eopen(ProgName, argv[optind++], "w", NONE, NONE, &oh, &fpout);
    else {
	Fprintf(stderr, "%s: no output FEA_ANA file specified.\n", ProgName);
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "Output file is %s\n", out_file);
    /*
     * read the parameter file and process the range
     */
    if (strcmp(in_file, "<stdin>") != 0)
	(void) read_params(param_file, SC_CHECK_FILE, in_file);
    else
        (void) read_params(param_file, SC_NOCOMMON, in_file);

    get_range(&start, &nan, &frame_len, &step, &nframes, 
	prange, pflag, lflag, Sflag, &fpin, ih, zflag, Zflag);

    if (!wflag)
	window_type =
	    (symtype("window_type") != ST_UNDEF)
	    ? getsym_s("window_type")
	    : "RECT";

    pref_w_type = 
	malloc((unsigned)(strlen(WT_PREFIX) + strlen(window_type) + 1));
    spsassert(pref_w_type, "can't allocate space for window type name");
    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);

    win = lin_search(window_types, pref_w_type);

    spsassert(win > -1, "window type not recognized");

    if (debug_level)
	Fprintf(stderr, "%s: window_type = %s, win = %d\n",
	    ProgName, window_type, win);
    if (!oflag)
	if (symtype("order") != ST_UNDEF)
	    order = getsym_i("order");
    if (!eflag)
	if (symtype("preemphasis") != ST_UNDEF)
	    preemp = getsym_d("preemphasis");
    if (debug_level) Fprintf(stderr,
	"%s: start = %ld, nan = %ld, frame size = %ld, step = %ld nframes = %ld\n",
	    ProgName, start, nan, frame_len, step, nframes);
    if (debug_level) 
	Fprintf(stderr, "%s: order = %d, preempahsis = %f\n", ProgName, order, preemp);
    symerr_exit();
    if (start <= 0)
	ERROR_EXIT("negative or zero starting point");
    if (frame_len < 0)
	ERROR_EXIT("can't have negative frame length");
    if (!mflag) {
	if (symtype("method") != ST_UNDEF)
	    analysis = getsym_s("method");
	else
	    analysis = "AUTOC";
    }

    fea_skiprec(fpin, start - 1, ih);

    x = (float *) calloc((unsigned) frame_len, sizeof(float));
    spsassert(x != NULL, nomemory);
    if(preemp){
      px = (float *) calloc((unsigned) frame_len, sizeof(float));
      spsassert(px != NULL, nomemory);
    }
    r = (double *) calloc((unsigned) order + 1, sizeof(double));
    spsassert(r != NULL, nomemory);
    rc = (float *) calloc((unsigned) order + 1, sizeof(float));
    spsassert(rc != NULL, nomemory);
    /*
     *Create the output header
     */
    oh = new_header(FT_FEA);
    if (init_anafea_hd(oh, 0L, (long) order, 1L, 1L, 1L, 0, 0) != 0)
	ERROR_EXIT("error filling FEA_ANA header");
    add_source_file(oh, in_file, ih);
    add_comment(oh, cmd_line);
    (void) strcpy(oh->common.prog, "refcof");
    (void) strcpy(oh->common.vers, Version);
    (void) strcpy(oh->common.progdate, Date);
    oh->common.tag = YES;
    oh->variable.refer = in_file;
    if((record_freq = get_genhd_val("record_freq", ih, (double) -1.)) == -1.){
      Fprintf(stderr, 
	      "refcof: Invalid record frequency in input file - exiting.\n");
      exit(1);
    }
    *get_genhd_f("src_sf", oh) = (float)record_freq;
    *get_genhd_s("spec_rep", oh) = RC;
    *get_genhd_l("frmlen", oh) = frame_len; 
    *get_genhd_l("start", oh) = start;
    *get_genhd_l("nan", oh) =  nan; 
    if (ih->common.ndrec != -1) {
	    /*not a pipe, so we know how many points are out there*/
	    if ((ih->common.ndrec < nan) && !zflag && !Zflag)
		Fprintf(stderr, 
		    "%s: WARNING - not enough points in input file\n", ProgName);
	}
    /*
     * Check analysis method and write generic header item 
     */

    if ((method = lin_search(ana_methods, analysis)) == -1) {
	(void)fprintf(stderr, "%s: analysis method is %s\n", ProgName, analysis);
	ERROR_EXIT("Invalid spectrum analysis method");
    }
    if (debug_level) Fprintf(stderr,
	"%s: analysis method is %s\n", ProgName, ana_methods[method]);
/*
 * check options vs. methods
 */

    if ((method != AM_STRCOV) && (cflag || iflag) && !zflag)
      (void) fprintf(stderr, "%s: method not STRCOV, so -c and -i ignored\n",
		     ProgName);

   if (sflag && (method != AM_AUTOC) 
       && (method != AM_STRCOV) 
       && (method != AM_STRCOV1)
       && !zflag) {

	(void) fprintf(stderr, "%s: method not AUTOC, STRCOV, or STRCOV1.\n",
		     ProgName);
	(void) fprintf(stderr, " -s option ignored\n");
    }

    switch (method) {
    case AM_BURG:
	break;
    case AM_COV:
	break;
    case AM_MBURG:
	break;
    case AM_FBURG:
	break;
    case AM_VBURG:
	break;
    case AM_STRCOV:
	if (!cflag)
	  if (symtype("strcov_test") != ST_UNDEF)
	    strcov_test = getsym_d("strcov_test");
	if (!iflag)
	  if (symtype("strcov_maxiter") != ST_UNDEF)
	    strcov_maxiter = getsym_i("strcov_maxiter");
	break;
    case AM_STRCOV1:
	break;
    case AM_AUTOC:
	break;
    default:
	ERROR_EXIT("Invalid spectrum analysis method");
    }

    *add_genhd_e("method", NULL, 1, ana_methods, oh) = method;
    *add_genhd_s("order", NULL, 1, oh) = order;
    *add_genhd_l("step", (long *) NULL, 1, oh) = step;
    *add_genhd_e("window_type", (short *) NULL, 1, window_types, oh) = win;

    /* add DC_REMOVED info */

    if (dflag) 
	*add_genhd_e("DC_removed", NULL, 1, local_no_yes, oh) = YES;
    else
	*add_genhd_e("DC_removed", NULL, 1, local_no_yes, oh) = NO;

    if (sflag) 
	*add_genhd_s("sinc_n", NULL, 1, oh) = sincn;
    if (preemp) 
	*add_genhd_f("preemphasis", NULL, 1, oh) = preemp;

    update_waves_gen(ih, oh, (float)(start + frame_len/2), (float)step);

    /* Write out header */
    write_header(oh, fpout);

    /* Allocate space to store the analysis records. */

    recp = allo_anafea_rec(oh);
    *recp->frame_len = frame_len;
    *recp->frame_type = NONE;

    /* Begin the spectral analysis.  */

    *recp->tag = (long) start;    

    for (k = 0; k < nframes && more; k++) {
	if (debug_level > 1) 
	    Fprintf(stderr, "%s: frame %d\n", ProgName, k + 1);
	/*
         * get sampled data & do preemphasis
	 */
	if (preemp){
	  register float *din, *dout;
	  if (k == 0) {
	    actsize = get_sd_orecf(px, (int) frame_len, (int) step, 
				   first, ih, fpin);
	    for(x[0] = px[0], din = px+1, dout = x+1, fst = px[actsize-1],
		i=1; i<actsize; din++, i++)
	      *dout++ = *din - preemp * *(din-1);
	  }
	  else{
	    actsize = get_sd_orecf(px, (int) frame_len, (int) step, 
				   !first, ih, fpin);
	    for(x[0] = px[0] - preemp * fst, din = px+1, dout = x+1,
		i=1; i<actsize; din++, i++)
	      *dout++ = *din - preemp * *(din-1);
	    fst = px[actsize-1];
	  }
	}
	else{
	  if (k == 0) 
	    actsize = get_sd_orecf(x, (int) frame_len, (int) step, 
				   first, ih, fpin);
	  else
	    actsize = get_sd_orecf(x, (int) frame_len, (int) step, 
				   !first, ih, fpin);
	}

	if (actsize < frame_len) {
	  
	  if (Zflag) {
	    more = 0;
	    break;
	  }
	  
	  if (!zflag) Fprintf(stderr, 
			      "%s: WARNING - got fewer points than frame_len in frame %d (zero filled)\n",
			      ProgName, k + 1);
	}

	if (debug_level > 3) 
	     pr_farray(x, (long) frame_len,"frame from get_sd_orec");

	if (actsize == 0) break;

	if (debug_level > 1)
	    Fprintf(stderr, "%s: got %d points\n", ProgName, actsize);

	more = (actsize == frame_len);

	if (compute_rc(x, actsize, method, dflag, win, order, sincn, 
		       strcov_maxiter, strcov_test, rc, r, &gain) == -1)
	  ERROR_EXIT("couldn't compute reflection coefficients");

	for (i = 0; i < order; i++)
	    recp->spec_param[i] = rc[i + 1];

	recp->raw_power[0] = r[0];
	recp->lpc_power[0] = r[0] * gain;
	*recp->frame_len = actsize;

	put_anafea_rec(recp, oh, fpout);
	*recp->tag += step;
    }
/*
 * put info in ESPS common
 */
    if (strcmp(out_file, "<stdout>") != 0 && strcmp(in_file, "<stdin>") != 0) {
	(void) putsym_s("filename", in_file);
	(void) putsym_s("prog", "refcof");
	(void) putsym_i("start", (int) start);
	(void) putsym_i("nan", (int) nan);
    }    
    (void)fclose(fpout);
}


static void
get_range(srec, nan, fsize, step, nfrm, rng, rflag, lenflag, sizflag, 
    infile, inhd, zflag, Zflag)
long *srec;			/* starting point */
long *nan;			/* number of points */
long *fsize;			/* frame size */
long *step;			/* step size */
long *nfrm;			/* number of frames */
char *rng;			/* range string from range option */
int rflag;			/* flag for whether range option used */
int lenflag;			/* flag for whether frame_len option used */
int sizflag;			/* flag for whether step option used */
FILE **infile;			/* sampled data stream */
struct header *inhd;		/* input file header */
int zflag;			/* flag for whether -z option used */
int Zflag;			/* flag for whether -Z option used */

{
    long last;
    *srec = 1;
    last = LONG_MAX;

    if (rflag) {
	lrange_switch (rng, srec, &last, 1);	
	if (last == LONG_MAX)
	    last = *srec + DEF_FRAMELEN - 1;
    }
    else {
	if(symtype("start") != ST_UNDEF) 
	    *srec = getsym_i("start");
	if(symtype("nan") != ST_UNDEF) {
	    *nan = getsym_i("nan");
	    if (*nan == 0)
		last = n_rec(infile, inhd);
	    else
		last = *srec + *nan - 1;
	}
	else
	    last = *srec + DEF_FRAMELEN - 1;
    }
    *nan = last - *srec + 1;

    if (!lenflag) 
	*fsize = 
	    (symtype("frame_len") != ST_UNDEF)
	    ? getsym_i("frame_len")
	    : 0;

    /*
     * if no frame_len set or if set to 0, use whole range as frame_len
     */
    if (*fsize == 0) 
	*fsize = *nan;

    if (!sizflag)
	*step =
	    (symtype("step") != ST_UNDEF && getsym_i("step") != 0)
	    ? getsym_i("step")
	    : *fsize;

    /* for -S0 argument */
    if (sizflag && *step==0)
        *step = *fsize;
      
    *nfrm = (*nan <= *fsize) ? 1 : 2 + (*nan - *fsize - 1) / *step;

    if ((*fsize > *nan) && !zflag && !Zflag){
	    Fprintf(stderr, 
	"%s: WARNING - frame_len %ld larger than range %ld;\n",
	    ProgName, *fsize, *nan);
	    Fprintf(stderr, 
		"\t single frame will overrun range.\n");
    }
   else {
	if (((*fsize + (*nfrm - 1) * *step) > *nan) && !zflag) 
	  if (Zflag)
	    *nfrm = *nfrm - 1;
	 else 
	   Fprintf(stderr, 
	    "%s: WARNING - last frame will overrun stated range by %ld points\n", 
		ProgName, *fsize + (*nfrm - 1) * *step - *nan);
    }
}


/*
 * Get number of samples in a sampled-data file.
 * Replace input stream with temporary file if input is a pipe or
 * record size is not fixed.
 */

#define BUFSIZE 1000

static long
n_rec(file, hd)
    FILE **file;
    struct header **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe or has
				     * variable record length. */
    {
	FILE	*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	static double
		buf[BUFSIZE];
	int	num_read;
	long	ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);

	do
	{
	    num_read = get_sd_recd(buf, BUFSIZE, *hd, *file);
	    if (num_read != 0) put_sd_recd(buf, num_read, tmphdr, tmpstrm);
	    ndrec += num_read;
	} while (num_read == BUFSIZE);
	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}
