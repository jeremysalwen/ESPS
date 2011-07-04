/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1988-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne; based on John Shore's program frame.c
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *  This program takes a single channel FEA_SD file, reads frames  	
 *  (possibly overlapping), applies an optional window to the data,      
 *  and produces acoustic feastures based on each frame.
 */

static char *sccs_id = "@(#)acf.c	1.12	1/21/97	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/fea.h>
#include <esps/anafea.h>
#include <esps/feaspec.h>
#include <esps/window.h>
#include <esps/limits.h>
#include <math.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", PROG, text); exit(1);}}

#define SYNTAX {\
fprintf(stderr, "acf [-{r,s} range] [-x debug_level] [-P param] input.sd output.fea\n") ; \
exit(1);}

#define WT_PREFIX "WT_"

#define GPFLAGSET(flagname) \
(symtype(flagname)!=ST_UNDEF&&getsym_i(flagname)==1)

#define GPSTRING(itemname,default) \
symtype(itemname)!=ST_UNDEF ? getsym_s(itemname) : default;

void	lrange_switch();
void    frange_switch();
int	get_sd_orecd();
char	*get_cmd_line();
long    *grange_switch();
double  get_genhd_val();
int     lin_search();

long	n_rec();
void	pr_farray();

char	*PROG = "acf";
char	*Version = "1.12";
char	*Date = "1/21/97";
int     debug_level = 0;	/* debug level; global for library*/

void    fft_cepstrum_r();
void    get_rfft();
void    blt();

double log10();


/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int
	    optind;		/* for use of getopt() */
    extern char
	    *optarg;		/* for use of getopt() */
    int	    ch;			/* command-line option letter */
    long    frame_len;		/* length of each frame */
    int	    rflag = 0;		/* -r option specified? */
    int	    sflag = 0;		/* -s option specified? */
    long    step;		/* shift between successive frame positions */
    char    *rrange;		/* arguments of -r option */
    long    start;		/* starting points in the input files */
    long    nan;		/* total number of samples to analyze */
    double  nan_f;              /* number of samples to analys in param file */
    double  start_f;            /* starting time in the input file */
    double  start_time;         /* generic header item value from input file */
    double  last_f;             /* total number of seconds to analyze */
    long    last;		/* end of range */
    char    *window_type;	/* name of type of window to apply to data */
    char    *pref_w_type;	/* window type name with added prefix */
    int	    win;		/* window type code */
    extern char
	    *window_types[];    /* standard window type names */
    char    *param_name = NULL;
				/* parameter file name */
    char    *iname;		/* input file name */
    FILE    *ifile;		/* input stream */
    struct header
	    *ihd;		/* input file header */
    float   *x;			/* sampled data from the input file */
    char    *oname;		/* output file name */
    FILE    *ofile;		/* output stream */
    struct header
	    *ohd;		/* output file header */
    struct fea_data
	    *fea_rec;		/* fea output record */
    struct  feaspec 
            *spec_rec;          /* feaspec output record */

    short   units=0;            /* range format units=0 => samples;
				                units=1 => seconds */
    static  char *units_val[] = {"SAMPLES", "SECONDS", NULL};
                                /* valid values of units parameter */
    long    n_frames;		/* number of frames to process */
    long    j, k;		/* loop index */
    int	    more = 1;		/* flag to say more frames out there */
    int	    first;		/* flag for initial call of get_sd_orecd() */
    float   src_sf;             /* sampling freq of input */
    float   preemphasis = 0.0;  /* preemphasis value */
    float   dummy;
    int     ptr;
    long    actsize;	        /* actual frame length */

    int     sd_flag=0;          /* store windowed data in FEA rec */
    char    *sd_fname;		/* name for output sd field */
    float   *sd_preemp_ptr;	/* pointer to preemphasized data */
    float   *sd_ptr;		/* pointer to sd field in FEA record */
				/* also used as destination of windowing */

    int     p_flag=0;           /* store frame power in FEA rec */
    char    *p_fname;           /* power field name */
    float   *p_ptr;             /* pointer to power field in FEA rec */
    
    int     zc_flag=0;          /* store frame zero crossings in FEA rec */
    char    *zc_fname;          /* zero crossing field name */
    float   *zc_ptr;            /* ptr to zero crossing field in FEA rec*/
    
    int     ac_flag=0;          /* store autocorrelation in FEA rec*/
    int     ac_yes=0;           /* compute autocorr */
    char    *ac_fname;          /* autocorr field name */
    double  *ac_data;           /* pointer to internal autocorr data */
    float   *ac_ptr;            /* pointer to autocorr data in output record */
    int     ac_order=0;         /* autocorr field dim in FEA rec */

    int     rc_flag=0;          /* store reflection coefficients in FEA rec*/
    int     rc_lpc_yes=0;       /* compute refl coffs */
    char    *rc_fname;          /* refl coeff field name */
    float   *rc_ptr;            /* pointer to refl coeff field in FEA rec */
    float   *rc_data;           /* pointer to refl coeff data; if lpccep_order >
                                   ad_order, padd reflection coeffs with 0's to 
				   satisfy recursion relationship */

    int     lpc_flag=0;         /* store linear prediction coeffs in FEA rec*/
    char    *lpc_fname;         /* lpc field name */
    float   *lpc_ptr;           /* pointer to lpc field in FEA rec */
    float   *lpc_data;           /* pointer to lpc data */
    float   lpc_gain;           /* lpc gain */ 

    int     lsf_flag=0;         /* store line spectral frequencies in FEA rec*/
    char    *lsf_fname;         /* lsf field name */
    float   *lsf_ptr;           /* pointer to lsf field in FEA rec */
    float   lsf_freq_res=10.0;  /* lsf frequency resolution */

    int     lar_flag=0;         /* store log area ratios in FEA rec*/
    char    *lar_fname;         /* lar field name */
    float   *lar_ptr;           /* pointer to lar field in FEA rec */

    int     lpccep_flag=0;      /* store lpc cepstrum in FEA rec*/
    char    *lpccep_fname;      /* lpc cepstrum field name */
    float   *lpccep_ptr;        /* pointer to lpc cepstrum field in FEA rec*/
    int     lpccep_order=0;     /* number of lpccep coeffs computed */
    int     lpccep_deriv_flag=0;     
    char    *lpccep_range; 
    long    *lpccep_index=NULL;
    float   *lpccep_data=NULL;
    long    lpccep_field_len;   /* size of lpc cepstral field in FEA rec */

    int     fft_flag=0;         /* store fft in FEA rec*/
    int     fft_order=0;        /* order of fft */
    int     fft_tlen=0;         /* 1 << fft_order */
    int     hf, num_freqs;      /* fft_rlen/2 and and hf + 1*/
    double  scale;              /* scaling param */
    float   *wr, *wi;           /* dummy vars for fft computation */

    int     fftcep_flag=0;      /* store fft cepstrum in FEA rec */
    char    *fftcep_fname;      /* fft cepstrum field name */
    float   *fftcep_ptr;        /* pointer to fft cepstrum field in FEA rec*/
    int     fftcep_order=0;     /* fftcep field dim in FEA rec */
    float   *fftcep_imag_ptr;   /* pointer for imaginary data in cepstrum */
    float   *fftcep_real_ptr;   /* pointer for real data in cepstrum */
    int     fftcep_tlen=0;      /* 1 << fftcep_order */
    int     fftcep_deriv_flag=0;     
    char    *fftcep_range; 
    long    *fftcep_index=NULL;
    long    fftcep_field_len;   

    float   warp_param=0.0;     /* warping param */

    /* Parse command-line options. */

    while ((ch = getopt(argc, argv, "r:s:x:P:")) != EOF)
        switch (ch)
	{
	case 'r':
	    rrange = optarg;
	    rflag++;
	    break;
        case 's':
	    rrange = optarg;
	    sflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	default:
	    SYNTAX
	    break;
	}
    if ( rflag && sflag ) {
      Fprintf(stderr, "can only specify one of -r or -s\n");
      exit(1);
    }

/* Process file names and open files. */

    if (argc - optind > 2) {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", PROG);
	SYNTAX
    }
    if (argc - optind < 2) {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", PROG);
	SYNTAX
    }
    iname = eopen(PROG,
	    argv[optind++], "r", FT_FEA, FEA_SD, &ihd, &ifile);
    oname = eopen(PROG,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);
    REQUIRE( iname != NULL && oname != NULL,
	    "ERROR: trouble with opening i/o files.\n");

    REQUIRE(get_fea_siz("samples", ihd, (short *) NULL, (long **) NULL) == 1,
	    "sorry, can't deal with multi-channel files");

    REQUIRE(!is_field_complex(ihd, "samples"), 
	    "sorry, can't deal with complex data"); 

    /* Get parameter values. */
    if (ifile != stdin)
	(void) read_params(param_name, SC_CHECK_FILE, iname);
    else
        (void) read_params(param_name, SC_NOCOMMON, iname);

    if (symtype("units") != ST_UNDEF) 
      REQUIRE( (units = lin_search( units_val, getsym_s("units"))) != -1,
	      "invalid specification of parameter units.");
    if (debug_level>1) {
      if (units==0)
	Fprintf(stderr, "%s: units in samples.\n", PROG);
      else
      	Fprintf(stderr, "%s: units in seconds.\n", PROG);
    }

    /* get sample frequency of input */
    src_sf = get_genhd_val("record_freq", ihd, (double) 0.0);
    REQUIRE( !(src_sf == 0.0 && sflag ), 
	    "record_freq not defined in input: can't use -s");
    if (src_sf == 0.0) {
      REQUIRE( units != 0, 
	      "record_freq not in input header: can't specify units as seconds.");
      src_sf = 1.0;  
    }
    start_time = get_genhd_val( "start_time", ihd, 0.0);

    /* if -r option specified, get start and last */
    if (rflag) {
      start = 1;
      last = LONG_MAX;
      lrange_switch(rrange, &start, &last, 0);
    }
    /* if -s option specified, get start and last and convert to samples */
    if (sflag) {
      start_f = start_time;
      last_f = DBL_MAX;
      (void) frange_switch(rrange, &start_f, &last_f);
      REQUIRE(start_f >= start_time, "can't start before beginning of input file");
      REQUIRE(last_f >= start_f, "empty range specified for input file");
      start = (start_f==start_time) ? 1 : (long) ((start_f-start_time)*src_sf+1);
      last = (last_f==DBL_MAX) ? LONG_MAX : (long) (last_f*src_sf+1);
    }
    if (last < LONG_MAX) 
      nan = last - start + 1;

    /* if not -s or -r, get start and nan from param file (as floats) -
       allow for common processing, in which start and nan are ints */
    if (!sflag && !rflag) {
	switch(symtype("start")) {
	    case ST_FLOAT :  /*assume read from param file*/
	         start_f = getsym_d("start");
		 if (start_f==0)
		   start = 1;
		 else
		   start = (units == 0) ? (long) start_f
		       : (long) ((start_f-start_time) * src_sf + 1);
		 break;
            case ST_INT   :  /*assume read from common file*/
		 start = getsym_i("start");
		 break;
            case ST_UNDEF :  /*not defined - start at begining*/
		 start = 1;
		 break;
	    default:
                 Fprintf(stderr, "%s: Invalid type of value start.\n", PROG);
		 exit(1);
	    }
	switch(symtype("nan")) {
	    case ST_FLOAT :  /*assume read from param file*/
	         nan_f = getsym_d("nan");
		 nan = (units == 0) ? (long) nan_f : (long) (nan_f * src_sf);
		 break;
            case ST_INT   :  /*assume read from common file*/
		 nan = getsym_i("nan");
		 break;
            case ST_UNDEF :  /*not defined */
	         nan = 0;
		 break;
	    default:
                 Fprintf(stderr, "%s: Invalid type of value nan.\n", PROG);
		 exit(1);
	    }
	if (nan < LONG_MAX)
	  last = nan + start - 1;
    }
    if ( last == LONG_MAX || nan == 0 ) {
	last = ihd->common.ndrec;
	REQUIRE(last != -1,
		"Must specify endpoint when input is a pipe or has variable record length.");
	nan = last - start + 1;
    }
    REQUIRE(start >= 1, "can't start before beginning of input file");
    REQUIRE(last >= start, "empty range specified for input file");

    if (symtype("frame_len") == ST_UNDEF) 
	frame_len = 0;
    else 
      frame_len = (units==0) ?
	(long) getsym_d("frame_len") : (long) (getsym_d("frame_len")*src_sf);

    if (frame_len == 0) {
	frame_len = nan;
	if (debug_level>1) 
	  Fprintf(stderr, "%s: frame_len changed from 0 to %ld\n", PROG, frame_len);
    }

    if (symtype("step") != ST_UNDEF && getsym_d("step") != 0.0)
      step = (units==0) ? 
	(long) getsym_d("step") : (long)(getsym_d("step")*src_sf);
    else
      step = frame_len;

    n_frames =
      (nan == 0) ? 0
	: (nan <= frame_len) ? 1
	  : 2 + (nan - frame_len - 1) / step;
    
    if (n_frames > 0 && (n_frames - 1) * step + frame_len > nan)
	Fprintf(stderr,
	    "%s: WARNING - last frame will exceed specified range.\n",
	    PROG);

    window_type = (symtype("window_type") != ST_UNDEF) ? 
	getsym_s("window_type") : "RECT";

    pref_w_type =
	malloc((unsigned)(strlen(WT_PREFIX) + strlen(window_type) + 1));
    REQUIRE(pref_w_type, "can't allocate space for window type name");
    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);
    win = lin_search(window_types, pref_w_type);
    if (win <= 0) {
      Fprintf(stderr, "acf: window type not recognized, using RECT");
      win = WT_RECT;
    }

    /* get processing parameters from params file */
    sd_flag = GPFLAGSET("sd_flag") ? 1 : 0;
    if (sd_flag) 
	sd_fname = GPSTRING("sd_fname", "sd");
    
    p_flag = GPFLAGSET("pwr_flag") ? 1 : 0;
    if (p_flag)
	p_fname = GPSTRING("pwr_fname", "power");

    zc_flag = GPFLAGSET("zc_flag") ? 1 : 0;
    if (zc_flag)
	zc_fname = GPSTRING("zc_fname", "zero_crossing");

    ac_flag = GPFLAGSET("ac_flag") ? 1 : 0;
    if (ac_flag) {
      ac_yes = 1;
      ac_fname = GPSTRING("ac_fname", "auto_corr");
    }
    
    rc_flag = GPFLAGSET("rc_flag") ? 1 : 0;
    if (rc_flag) {
      ac_yes = rc_lpc_yes = 1;
      rc_fname = GPSTRING("rc_fname", "refcof");
    }

    lsf_flag = GPFLAGSET("lsf_flag") ? 1 : 0;
    if (lsf_flag) {
      ac_yes = rc_lpc_yes = 1;
      lsf_fname = GPSTRING("lsf_fname", "line_spec_freq");
    }

    lar_flag = GPFLAGSET("lar_flag") ? 1 : 0;
    if (lar_flag) {
      ac_yes = rc_lpc_yes = 1;
      lar_fname = GPSTRING("lar_fname", "log_area_ratio");
    }

    lpc_flag = GPFLAGSET("lpc_flag") ? 1 : 0;
    if (lpc_flag) {
      ac_yes = rc_lpc_yes = 1;
      lpc_fname = GPSTRING("lpc_fname", "lpc_coeffs");
    }
	
    fft_flag = GPFLAGSET("fft_flag") ? 1 : 0;

    lpccep_flag = GPFLAGSET("lpccep_flag") ? 1 : 0;
    if (lpccep_flag) {
      ac_yes = rc_lpc_yes = 1;
      lpccep_fname = GPSTRING("lpccep_fname", "lpc_cepstrum");
    }

    fftcep_flag = GPFLAGSET("fftcep_flag") ? 1 : 0;
    if (fftcep_flag) {
      fftcep_fname = GPSTRING("fftcep_fname", "fft_cepstrum");
    }

    /* get compute params */
    if (ac_yes) {
      if (symtype("ac_order") != ST_UNDEF)
	ac_order = getsym_i("ac_order");
      else {
	Fprintf(stderr,"%s: ac_order undefined in parameter file.\n", PROG);
	exit(1);
      }
      if (ac_order <= 0)
	Fprintf(stderr,"%s: WARNING! Value of 0 or less found for auto corr order.\n", PROG);
    }
    if (lpccep_flag) {
      if (symtype("lpccep_order") == ST_UNDEF)
	  lpccep_order = ac_order;
      else
	  lpccep_order = getsym_i("lpccep_order");
      spsassert(ac_order!=0 || lpccep_order!=0, "both ac_order and lpccep_order are 0.");
	  
      if (lpccep_order < ac_order) {
	Fprintf(stderr, "%s: setting lpc cepstrum order to auto corr order.\n", PROG);
	lpccep_order = ac_order;
      }

      if (ac_order==0) {
	Fprintf(stderr, "%s: ac_order is zero - setting it to lpccep_order (%d).\n", 
		PROG, lpccep_order);
	ac_order = lpccep_order;
      }

      if (symtype("lpccep_deriv")!=ST_UNDEF) {
	  lpccep_range = getsym_s("lpccep_deriv");
	  if ( strlen(lpccep_range) ) { 
	      lpccep_index = grange_switch( lpccep_range, &j);
	      if ( lpccep_index != NULL ) {
		  for (k=0; k<j; k++)
		      spsassert(lpccep_index[k] >= 0 && lpccep_index[k] < lpccep_order, 
				"invalid subrange of lpc_cepstrum elements specified by lpccep_deriv.");
		  lpccep_deriv_flag = 1;
		  lpccep_field_len = j;
	      } 
	  }
      }
      if ( !lpccep_deriv_flag ) {
	  lpccep_range = malloc(15);
	  lpccep_field_len = lpccep_order;
	  Sprintf( lpccep_range, "0:%d", lpccep_order-1);
      }

    }
    if (lsf_flag) 
      if (symtype("lsf_freq_res")!=ST_UNDEF)
	lsf_freq_res = (float) getsym_d("lsf_freq_res");
    if (fftcep_flag) {
      if (symtype("fftcep_order")!=ST_UNDEF)
	fftcep_order = getsym_i("fftcep_order");
      else {
	Fprintf(stderr, "%s: fft cepstrum order undefined.\n", PROG);
	exit(1);
      }
      spsassert( fftcep_order>0, "fftcep_order must be positive.");
      fftcep_tlen = 1 << fftcep_order;
      if (symtype("fftcep_deriv")!=ST_UNDEF) {
	  fftcep_range = getsym_s("fftcep_deriv");
	  if ( strlen(fftcep_range) ) {
	      fftcep_index = grange_switch( fftcep_range, &j);
	      if ( fftcep_index != NULL ) {
		  for (k=0; k<j; k++)
		      spsassert(fftcep_index[k] >= 0 && fftcep_index[k] < fftcep_tlen, 
				"invalid subrange of fft_cepstrum elements specified by fftcep_deriv.");
		  fftcep_deriv_flag = 1;
		  fftcep_field_len = j;
	      } 
	  }
      }
      if ( !fftcep_deriv_flag ) {
	  fftcep_range = malloc(15);
	  fftcep_field_len = fftcep_tlen;
	  Sprintf( fftcep_range, "0:%d", fftcep_tlen-1);
      }
    }


    if ( symtype("preemphasis") != ST_UNDEF )
      preemphasis = (float) getsym_d("preemphasis");
    REQUIRE( preemphasis >=0.0 && preemphasis <=1.0, "preemphasis outside [0.0,1.0]");
    if (fftcep_flag || lpccep_flag) 
	if (symtype("warp_param")!=ST_UNDEF)  
	    warp_param = (float) getsym_d("warp_param");
    REQUIRE(warp_param>-1.0 && warp_param<1.0, "warping parameter outside (-1,1).");
		
    if (fft_flag) {
      if (symtype("fft_order")!=ST_UNDEF) 
	fft_order = getsym_i("fft_order");
      else {
	Fprintf(stderr, "acf: parameter fft_order undefined.\n");
	exit(1);
      }
      fft_tlen = 1 << fft_order;
      hf = fft_tlen / 2;
      num_freqs = hf + 1;
      scale = 10 * log10(2.0/(fft_tlen*src_sf));
    }

    /* Create output-file header */
    ohd = new_header(FT_FEA);
    add_source_file(ohd, iname, ihd);
    ohd->common.tag = YES;
    (void) strcpy(ohd->common.prog, PROG);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);
    ohd->variable.refer = iname;
    add_comment(ohd, get_cmd_line(argc, argv));

    *add_genhd_l("frmlen", (long *) NULL, 1, ohd) = frame_len;
    *add_genhd_l("nan", (long *) NULL, 1, ohd) = nan;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start;
    *add_genhd_l("step", (long *) NULL, 1, ohd) = step;
    *add_genhd_e("window_type", (short *) NULL, 1, window_types, ohd) = win;
    /* processing flags */
    *add_genhd_s("flag_sd", (short *) NULL, 1, ohd) = sd_flag;
    *add_genhd_s("flag_pwr", (short *) NULL, 1, ohd) = p_flag;
    *add_genhd_s("flag_zc", (short *) NULL, 1, ohd) = zc_flag;
    *add_genhd_s("flag_rc", (short *) NULL, 1, ohd) = rc_flag;
    *add_genhd_s("flag_autoc", (short *) NULL, 1, ohd) = ac_flag;
    *add_genhd_s("flag_lsf", (short *) NULL, 1, ohd) = lsf_flag;
    *add_genhd_s("flag_lar", (short *) NULL, 1, ohd) = lar_flag;
    *add_genhd_s("flag_lpc", (short *) NULL, 1, ohd) = lpc_flag;
    *add_genhd_s("flag_fft", (short *) NULL, 1, ohd) = fft_flag;
    *add_genhd_s("flag_lpccep", (short *) NULL, 1, ohd) = lpccep_flag;
    *add_genhd_s("flag_fftcep", (short *) NULL, 1, ohd) = fftcep_flag;
    *add_genhd_f("src_sf", (float *) NULL, 1, ohd) = (float) src_sf;
    if (ac_yes) 
      *add_genhd_s("ac_order", (short *) NULL, 1, ohd) = ac_order;
    if (lpccep_flag) {
      *add_genhd_s("lpccep_order", (short *) NULL, 1, ohd) = lpccep_order;
      (void) add_genhd_c("lpccep_deriv", lpccep_range, (int)0, ohd);
    }
    if (lsf_flag)
      *add_genhd_f("lsf_freq_res", (short *) NULL, 1, ohd) = lsf_freq_res;
    if (fftcep_flag) {
      *add_genhd_s("fftcep_order", (short *) NULL, 1, ohd) = fftcep_order;
      (void) add_genhd_c("fftcep_deriv", fftcep_range, (int)0, ohd);
    }
    *add_genhd_f("warping_param", (float *) NULL, 1, ohd) = warp_param;
    *add_genhd_f("preemphasis", (float *) NULL, 1, ohd) = preemphasis;
    if(debug_level>1)
	Fprintf(stderr, "%s: adding fields.\n", PROG);

    /* create output fields */
    if (sd_flag)
	REQUIRE( add_fea_fld(sd_fname, (long)frame_len, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create sd_field_name field in output file header" );
    if (p_flag)
	REQUIRE( add_fea_fld(p_fname, (long )1, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create power_field_name field in output file header" );
    if (zc_flag)
	REQUIRE( add_fea_fld(zc_fname, (long) 1, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create zc_field_name field in output file header" );
    if (rc_flag)
	REQUIRE( add_fea_fld(rc_fname, (long)ac_order, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create refcof field in output file header" );
    if (ac_flag) 
	REQUIRE( add_fea_fld(ac_fname, (long)(ac_order+1), (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create autocor field in output file header" );
    if (lsf_flag)
	REQUIRE( add_fea_fld(lsf_fname, (long)ac_order, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create lsf field in output file header" );
    if (lar_flag)
	REQUIRE( add_fea_fld(lar_fname, (long)ac_order, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create lar field in output file header" );
    if (lpc_flag)
	REQUIRE( add_fea_fld(lpc_fname, (long)ac_order, (short) 1, (long *) NULL,
			     FLOAT, (char **) NULL, ohd) != -1,
		"can't create lpc field in output file header" );
    if (lpccep_flag)
	REQUIRE( add_fea_fld(lpccep_fname, (long)lpccep_field_len, (short) 1, 
			     (long *) NULL, FLOAT, (char **) NULL, ohd) != -1,
		"can't create lpccep field in output file header" );
    if (fftcep_flag) {
	REQUIRE( add_fea_fld(fftcep_fname, (long)fftcep_field_len, (short) 1, 
			     (long *) NULL, FLOAT, (char **) NULL, ohd) != -1,
		"can't create fftcep field in output file header" );
    }

    update_waves_gen(ihd, ohd, (float) start, (float) step);

    if (debug_level) {
	Fprintf(stderr, "%s: Input file: %s \t Output file: %s\n", PROG,
	    iname, oname);
	Fprintf(stderr, "start: %ld   last: %ld    nan: %ld\n", start, last, nan);
	Fprintf(stderr, "frame_len: %ld    step: %ld   frames: %ld\n", 
		frame_len, step, n_frames);
	Fprintf(stderr, "preemphasis: %f\n", preemphasis);
	Fprintf(stderr, "window_type: %s\n", window_type);
	if (ac_yes) 
	  Fprintf(stderr, "autocorrelation order: %ld\n", ac_order);
	if (rc_lpc_yes)
	  Fprintf(stderr, "lpc and reflection coefs: order %ld\n", ac_order);
	if (lar_flag) 
	  Fprintf(stderr, "log area ratios order: %ld\n", ac_order);
	if (lsf_flag)
	  Fprintf(stderr, "line spectral frequencies order: %ld,   frequency resolution %f\n",
		  ac_order, lsf_freq_res);
	if (lpccep_flag)
	  Fprintf(stderr, "lpc cepstrum: order %ld  formed from elements %s: warping param: %f\n", 
		  lpccep_order, lpccep_range, warp_param);
	if (fftcep_flag)
	  Fprintf(stderr, "fft cepstrum: order %ld (%ld) formed from elements %s; warping param %f\n", 
		  fftcep_order, fftcep_tlen, fftcep_range, warp_param);
	if (fft_flag)
	  Fprintf(stderr, "fft: order %ld (%ld)\n", fft_order, fft_tlen);
    }

    if (fft_flag)
	REQUIRE( 0==init_feaspec_hd(ohd, NO, SPFMT_SYM_EDGE, SPTYP_DB, YES, (long) num_freqs, SPFRM_FIXED, 
				    (float *)NULL,(double) src_sf,(long) frame_len, FLOAT), 
		"can't initialize fea spec header.");

    if ( ohd->hd.fea->field_count == 0  ) {
	Fprintf(stderr, "%s: No output fields defined in parameter file - exiting\n", PROG);
	exit(1);
    }

    if (debug_level>1)
      Fprintf(stderr, "%s:writing output header to file\n", PROG);
    write_header(ohd, ofile);

    /* Allocate buffer and set up output record. */
    x = (float *) calloc((unsigned) frame_len, sizeof(float));
    sd_preemp_ptr = (float *) calloc((unsigned) frame_len, sizeof(float));
    REQUIRE((x != NULL) && (sd_preemp_ptr != NULL), 
	    "can't allocate memory for input frame");

    if (fft_flag)
      spec_rec = allo_feaspec_rec(ohd, FLOAT);
    else
      fea_rec = allo_fea_rec(ohd);
    REQUIRE( (fft_flag && spec_rec != NULL) || (!fft_flag && fea_rec != NULL),
	    "can't allocate output record.");
    
    /* get pointers to fea fields or allocate memory */
    if (sd_flag) {
	if (fft_flag) 
	    sd_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, sd_fname, ohd);
	else
	    sd_ptr = (float *) get_fea_ptr(fea_rec, sd_fname, ohd);
    }
    else
	sd_ptr = (float *) calloc((unsigned)frame_len, sizeof(float));
    REQUIRE( sd_ptr != NULL, "can't allocate memory for windowed data.");

    if (p_flag) {
	if (fft_flag)
	    p_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, p_fname, ohd);
	else
	    p_ptr = (float *) get_fea_ptr(fea_rec, p_fname, ohd);
    }

    if (zc_flag) {
      if (fft_flag) 
	zc_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, zc_fname, ohd);
      else
	zc_ptr = (float *) get_fea_ptr(fea_rec, zc_fname, ohd);
    }

    if (ac_flag) {
      if (fft_flag) 
	ac_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, ac_fname, ohd);
      else
	ac_ptr = (float *) get_fea_ptr(fea_rec, ac_fname, ohd);
    }
    if (ac_yes) 
	ac_data = (double *) calloc((unsigned)(ac_order+1), sizeof(double));
    REQUIRE(!ac_yes  || ac_data!=NULL, 
	    "can't allocate memory for autocorrelation data.");
    REQUIRE(!ac_flag || ac_ptr!=NULL, "can't allocate memory for autocorrelation.");

    if (rc_flag)  {
      if (fft_flag)
	rc_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, rc_fname, ohd);
      else
	rc_ptr = (float *) get_fea_ptr(fea_rec, rc_fname, ohd);
    }
    if (rc_lpc_yes) {
      if (ac_order < lpccep_order) 
	rc_data = (float *) calloc((unsigned)(lpccep_order+1), sizeof(float));
      else
	/*rc_data = rc_ptr;*/
	rc_data  = (float *) calloc((unsigned)(ac_order+1), sizeof(float));
      
      REQUIRE(rc_data!=NULL,"can't allocate memory for refl coeffs.");
    }

    if (lpc_flag) {
	if (fft_flag)
	    lpc_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, lpc_fname, ohd);
	else
	    lpc_ptr = (float *) get_fea_ptr(fea_rec, lpc_fname, ohd);
    }

    if (rc_lpc_yes) {
	lpc_data = (float *) calloc((unsigned)(ac_order+1), sizeof(float));

	REQUIRE(lpc_data!=NULL,"can't allocate memory for lpc coeffs.");
    }

    if (lsf_flag) {
      if (fft_flag) 
	lsf_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, lsf_fname, ohd);
      else
	lsf_ptr = (float *) get_fea_ptr(fea_rec, lsf_fname, ohd);
    }

    if (lar_flag) {
      if (fft_flag)
	lar_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, lar_fname, ohd);
      else
	lar_ptr = (float *) get_fea_ptr(fea_rec, lar_fname, ohd);
    }

    if (lpccep_flag) {
      if (fft_flag)
	lpccep_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, lpccep_fname, ohd);
      else
	lpccep_ptr = (float *) get_fea_ptr(fea_rec, lpccep_fname, ohd);
      if (lpccep_deriv_flag) 
	  lpccep_data = (float *) calloc((unsigned)lpccep_order, sizeof(float));
      else
	  lpccep_data = lpccep_ptr;
      spsassert(lpccep_data!=NULL, "can't allocate memory for lpc data.");
    }

    if (fftcep_flag) {
      if (fft_flag)
	fftcep_ptr = (float *) get_fea_ptr(spec_rec->fea_rec, fftcep_fname, ohd);
      else
	fftcep_ptr = (float *) get_fea_ptr(fea_rec, fftcep_fname, ohd);
      if (fftcep_deriv_flag) 
	  fftcep_real_ptr = (float *) calloc((unsigned)fftcep_tlen, sizeof(float));
      else
	  fftcep_real_ptr = fftcep_ptr;
      fftcep_imag_ptr = (float *) calloc((unsigned)fftcep_tlen, sizeof(float));
      REQUIRE(fftcep_imag_ptr!=NULL && fftcep_real_ptr!=NULL, 
	      "can't allocate fields for fft cepstrum.");
    }

    
    if (fft_flag) {
      wr = (float *) calloc((unsigned)fft_tlen, sizeof(float));
      wi = (float *) calloc((unsigned)fft_tlen, sizeof(float));
      REQUIRE( wr != NULL && wi != NULL, "can't allocate memory for fft.");
    }

    /* Main read-write loop */
    
    fea_skiprec(ifile, (long) (start - 1), ihd);
    first = 1;
    if (fft_flag)
	*spec_rec->tag = start;
    else
	fea_rec->tag = start;

    for (k = 0; k < n_frames && more; k++) 
    {
	if (debug_level>1) {
	    if (fft_flag)
		Fprintf(stderr, "%s: frame %ld at tag %ld\n", PROG, k + 1, 
			*spec_rec->tag);
	    else
		Fprintf(stderr, "%s: frame %ld at tag %ld\n", PROG, k + 1, 
			fea_rec->tag);
	}
	actsize = get_sd_orecf(x, (int) frame_len, (int) step, 
			       first, ihd, ifile);

	first = 0;
	if (actsize == 0) break;
	more = (actsize == frame_len);

	if (actsize < frame_len) {
	  if (fft_flag)
	    Fprintf(stderr, 
		"%s: WARNING - only %ld points in frame %ld at tag %ld (zero filled)\n",
		PROG, actsize, k + 1, *spec_rec->tag);
	  else
	    Fprintf(stderr, 
		"%s: WARNING - only %ld points in frame %ld at tag %ld (zero filled)\n",
		PROG, actsize, k + 1, fea_rec->tag);
	}
	if (debug_level > 2) {
	    pr_farray("frame from input.sd", actsize, x);
	}
	/* preemphasize data ; if preemphasis=0.0, moves to sd_preemp_ptr */
	sd_preemp_ptr[0] = x[0];
	for (ptr=1; ptr<frame_len; ptr++)
	  sd_preemp_ptr[ptr] = x[ptr] - preemphasis * x[ptr-1];

	/* Window data */
	(void) window(frame_len, sd_preemp_ptr, sd_ptr, win, (double *) NULL);
	if (debug_level > 2)
	    pr_farray("windowed frame from input.sd", frame_len, sd_ptr);
	
	/* compute power */
	if (p_flag) {
	    for (*p_ptr=0.0,ptr=0; ptr<frame_len; ptr++)
		*p_ptr += sd_ptr[ptr]*sd_ptr[ptr];
	    *p_ptr = *p_ptr/actsize;
	}

	/* compute zero crossings */
	if (zc_flag) {
	    *zc_ptr = 0.0;
	    dummy = sd_ptr[0];
	    for (ptr=1; ptr<frame_len;ptr++) {
		if ( sd_ptr[ptr] * dummy < 0.0 )
		    (*zc_ptr)++;
		if ( sd_ptr[ptr] != 0.0 )
		    dummy = sd_ptr[ptr];
	    }
	    *zc_ptr = *zc_ptr * src_sf / actsize;
	}

	/* compute auto corr */
	if (ac_yes) 
	  (void) get_auto( sd_ptr, (int)actsize, ac_data, (int)ac_order);
	if (ac_flag)
	  for (ptr=0; ptr<=ac_order; ptr++)
	    ac_ptr[ptr] = (float) ac_data[ptr];
	
	/* compute rc's and lpc's */
	if (rc_lpc_yes) 
	 (void) get_atal(ac_data, (int) ac_order, lpc_data, rc_data, &lpc_gain);
	if (lpc_flag)
	  for (ptr=0; ptr<ac_order; ptr++)
	    lpc_ptr[ptr] = (float) lpc_data[ptr+1];

	if (rc_flag) 
	  for (ptr=0; ptr<ac_order; ptr++)
	    rc_ptr[ptr] = (float) rc_data[ptr+1];

	/* compute lar's */
	if (lar_flag)
	  (void) rc_reps(rc_data+1, lar_ptr, (int)LAR, (int)ac_order, (float)src_sf/2, (float)0);

	/* compute lpc cepstrum */
	if (lpccep_flag) {
	    (void) rc_reps(rc_data+1, lpccep_data, (int)CEP, (int)lpccep_order, (float)src_sf/2, (float)0);

	    if (warp_param != 0.0)
		(void) blt( lpccep_data, (int) lpccep_order, warp_param);

	    if (lpccep_deriv_flag) 
		for (j=0; j<lpccep_field_len; j++)
		    lpccep_ptr[j] = lpccep_data[ lpccep_index[j] ];
	}

	/* compute line spectral frequencies */
	if (lsf_flag)
	  (void) rc_reps(rc_data+1, lsf_ptr, (int)LSF, (int)ac_order, (float)src_sf/2, 
			 (float) lsf_freq_res);

	/* compute fft cepstrum */
	if (fftcep_flag) {
	  for (ptr=0; ptr<fftcep_tlen; ptr++) {
	    fftcep_imag_ptr[ptr] = 0.0;
	    if (ptr<actsize)
	      fftcep_real_ptr[ptr] = sd_ptr[ptr];
	    else
	      fftcep_real_ptr[ptr] = 0.0;
	  }
	  (void) fft_cepstrum_r( fftcep_real_ptr, fftcep_imag_ptr, (long) fftcep_order);
	  if (warp_param != 0.0) 
	    (void) blt( fftcep_real_ptr, (int) fftcep_order, warp_param);
	  if ( fftcep_deriv_flag )
	      for (j=0; j<fftcep_field_len; j++)
		  fftcep_ptr[j] = fftcep_real_ptr[ fftcep_index[j] ];
        }


	/* compute fft */
	if (fft_flag) {

	  for (ptr=0; ptr<fft_tlen; ptr++) {
	    wi[ptr] = 0.0;
	    if (ptr<actsize)
	      wr[ptr] = sd_ptr[ptr];
	    else
	      wr[ptr] = 0.0;
	  }
	  get_rfft(wr, wi, (int) fft_order);
	  for (ptr=0; ptr<num_freqs; ptr++) {
	    dummy = wr[ptr]*wr[ptr] + wi[ptr]*wi[ptr];
	    if (dummy>0)
	      spec_rec->re_spec_val[ptr] = (float )(scale + 10.0 * log10(dummy));
	    else
	      spec_rec->re_spec_val[ptr] = (float) (scale - 10.0 * log10(FLT_MAX));
	  }
	}

	if (fft_flag) {
	  (void) put_feaspec_rec(spec_rec, ohd, ofile);
	  *spec_rec->tag += step;
	} 
	else {
	  put_fea_rec(fea_rec, ohd, ofile);
	  fea_rec->tag += step;
      }
    }

    exit(0);
    /*NOTREACHED*/
}    


/*
 * For debug printout of float arrays
 */

void pr_farray(text, n, arr)
    char    *text;
    long    n;
    float  *arr;
{
    int	    i;

    Fprintf(stderr, "%s -- %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}

#define BUFSIZE 1000

long
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


