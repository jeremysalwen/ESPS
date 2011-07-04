/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *   "Copyright (c) 1990 Entropic Speech, Inc. All rights reserved."
 *
 * Program: sgram.c (FEA_SPEC version)
 *
 * This program computes spectrograms from FEA_SD files to FEA_SPEC files.
 *
 *  Rodney Johnson, Entropic Speech, Inc.		
 *  based on fft.c
 */
#ifndef lint
    static char *sccs_id = "@(#)sgram.c	1.20	8/31/95	 ESI";  
#endif
char *Version = "1.20";
char *Date = "8/31/95";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/limits.h>
#include <esps/feasd.h>
#include <esps/feaspec.h>
#include <esps/fea.h>
#include <esps/window.h>

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}

#define SYNTAX \
USAGE("sgram [-d data_window][-m method][-o fft_order][-{pr} range][-s range]\n\t[-w window_len][-E pre_emphasis][-P param_file][-S step_size] \n\t [-T desired_frames] [-z] input.sd output.fspec")

#define WT_PREFIX "WT_"

/*
 * global variables
 */

char	*ProgName = "sgram";
int	debug_level = 0;		/* debug level */

void	pr_farray();

/*  
 * external functions
 */

double  log10(), pow(), ceil(), sqrt();
char	*get_cmd_line();
void	lrange_switch(), frange_switch();
char    *eopen();
void	get_rfft();

/*
 * main program
 */

main(argc, argv)
    int	    argc;
    char    **argv;
{
    char    	*param_file = NULL; /* parameter file name */
    FILE    	*ifile = stdin,	/* input and output file streams */
            	*ofile = stdout;
    struct header  		/* input and output file headers */
            	*ih, *oh;
    char    	*iname,		/* input and output file names */
	    	*oname;  
	    
    struct feaspec		/* record for spectral data */
            	*spec_rec;

    extern int	optind;
    extern char	*optarg;
    int     	ch;

    short     	zflag = 1;	/* flag for -z (silent) option */
    short    	dflag = 0;	/* flag for data_window option */
    short       mflag = 0;	/* flag for method option -m */
    short       p_mflag = 0;	/* flag for method in parameter file */
    short       Tflag = 0;	/* flag for -T option */
    long        des_frames = 0;	/* number of required frames */
    char    	*window_type;	/* name of type of window to apply to data */
    char    	*pref_w_type;	/* window type name with added prefix */
    char    	*def_window_type; /* default name of window type */

    char    	*method = "wb";	/* method name */
    char        *hd_method = "wb"; /* method name for generic item */
    static char	*methods[] = {"nb", "wb", NULL}; /* valid method names */

    short	    	oflag = 0;	/* flag for fft_order option */
    long     	order;		/* order (exponent of 2) of fft */
    long	def_order;	/* default order */

    short	    	pflag = 0;	/* flag for range (pts.) option*/
    char    	*prange = NULL;	/* string for range specification (-p) */

    short     	sflag = 0;	/* flag for range (sec.) option */
    char    	*srange = NULL;	/* string for range specification (-s) */

    short	wflag = 0;	/* flag for window_len option */
    long	window_size;	/* number of samples in effective window */
    double  	window_len;	/* length of window (ms) */
    double  	def_window_len;	/* default window length */

    short     	Eflag = 0;	/* flag for preemphasis option */
    double  	preemph;	/* preemphasis */
    double  	def_preemph;	/* default preemphasis */

    short     	Sflag = 0;	/* flag for step_size option */
    double  	step_size;	/* step size (ms) */
    double  	def_step_size;	/* default step size */

    long    	i, j;
    long	size;		/* size of generic header item */
    short   	freq_format;
    short   	spec_type;
    long    	num_freqs;
    long	frame_len;	/* number of sampled data points in frame */
    long    	tr_length;	/* number of points in transform */
    unsigned	alloc_len;
    double  	sf;		/* sampling frequency */
    long   	actsize;	/* actual number points read */
    long    	data_length;	/* actual number points transformed */
    long    	nframes;	/* number fixed length frames to process*/
    float   	fpow;		/* power at one frequency */
    float   	tot_power;	/* total power */
    float   	*x, *px;	/* arrays for data and preemphasized data */
    float   	*wx, *y;	/* arrays for windowed data and fft */
    float   	scale;		/* scaling constant */
    short    	more = 1;	/* flag to indicate more sampled data*/
    long    	position;	/* current position in SD file */
    long    	step;		/* step size for shifting frames */
    long    	start;
    long    	end;
    long    	nan; 
    short	win;		/* window type code */
    extern char	*window_types[]; /* standard window type names */
    double  	file_start_time;
    double  	data_start_time;
    double  	data_end_time;
    int	    	zeros = 0;	/* count frequencies with zero power */
    long        num_channels;   /* holds number of input data channels*/

    /*
     * process command line options
     */

    while ((ch = getopt(argc, argv, "d:m:o:p:r:s:w:x:E:P:S:T:z")) != EOF)
        switch (ch)  {
	case 'z':
	    zflag = 1;
	    break;
	case 'd':
	    dflag++;
	    window_type = optarg;
	    hd_method = "other";
	    break;
	case 'm':
	    method = optarg;
	    mflag++;
	    break;
        case 'o':
            order = atoi(optarg);
	    oflag++;
	    hd_method = "other";
	    break;
        case 'p': 
        case 'r': 
            prange = optarg;
	    pflag++;
            break;
        case 's': 
            srange = optarg;
	    sflag++;
            break;
	case 'w':
	    wflag++;
	    window_len = atof(optarg);
	    hd_method = "other";
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'E':
	    preemph = atof(optarg);
	    Eflag = YES;
	    hd_method = "other";
	    break;
	case 'P':
	    param_file = optarg;
	    break;
	case 'S':
	    step_size = atof(optarg);
	    Sflag++;
	    hd_method = "other";
	    break;
	case 'T':
	    des_frames = atoi(optarg);
	    Tflag++;
	    break;
        default: 
            SYNTAX
	    ;
            break;
        }

    if (mflag && (strcmp(hd_method, "other") != 0))
      hd_method = method;

    /*
     * process file arguments
     */

    if (optind < argc)
	iname = eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_SD, &ih, &ifile);
    else {
	Fprintf(stderr, "%s: no input sampled data file specified.\n", ProgName);
	SYNTAX
	;
    }
    if (debug_level)
	Fprintf(stderr, "Input file is %s\n", iname);
    if (optind < argc)
	oname = eopen(ProgName, argv[optind++], "w", NONE, NONE, &oh, &ofile);
    else {
	Fprintf(stderr, "%s: no output FEA_SPEC file specified.\n", ProgName);
	SYNTAX
	;
    }
    if (debug_level)
	Fprintf(stderr, "Output file is %s\n", oname);

    /*
     * Check input file for multichannel and complex data
    */

    if((num_channels = 
        get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) != 1){
     Fprintf(stderr, "sgram: Multichannel data not supported yet - exiting\n");
           exit(1);
         }

    if(is_field_complex(ih, "samples") == YES){
    Fprintf(stderr, "sgram: Complex input data not supported yet - exiting.\n");
      exit(1);
  }
    /*
     * process  parameters
     */

    if (strcmp(iname, "<stdin>") != 0)
	(void) read_params(param_file, SC_CHECK_FILE, iname);
    else
        (void) read_params(param_file, SC_NOCOMMON, iname);

    /* -m method */

     if (!mflag) {
       if (symtype("method") != ST_UNDEF) {
	 method = getsym_s("method");
	 if (strcmp(method, "other") == 0) {
	   /* if parameter file says 'other', defaults are for
	      'wb' but other params may override*/
	   hd_method = "other";
	   method = "wb";
	 }
	 else {
	   /* if param file has specific method (nb or wb), go
	      with that and do not allow parameter overrides*/
	   hd_method = method;
	   p_mflag++;
	 }
       }
     }

    switch (lin_search2(methods, method))
    {
    case 0:			/* "nb" */
	def_window_type = "HANNING";
	def_order = 9;
	def_window_len = 40.0;
	def_preemph = .94;
	def_step_size = 2.0;
	break;
    case 1:			/* "wb" */
	def_window_type = "HANNING";
	def_order = 8;
	def_window_len = 8.0;
	def_preemph = .94;
	def_step_size = 2.0;
	break;
    default:
	ERROR_EXIT("Unrecognized method given with -m option.")
	break;
    }

    /* Based on the method, set the various parameters unless they 
     * have command line overrides 
     */

    if (!dflag) window_type = def_window_type;
    if (!oflag) order = def_order;
    if (!wflag) window_len = def_window_len;
    if (!Eflag) preemph = def_preemph;
    if (!Sflag) step_size = def_step_size;

    /* -d data_window */

    if (!(dflag || p_mflag))
	switch (symtype("data_window"))
	{
	case ST_UNDEF:
	    window_type = def_window_type;
	    break;
	case ST_STRING:
	    window_type = getsym_s("data_window");
	    break;
	default:
	    ERROR_EXIT("Parameter \"data_window\" not of string type.")
	    break;
	}

    pref_w_type = 
	malloc((unsigned)(strlen(WT_PREFIX) + strlen(window_type) + 1));
    spsassert(pref_w_type, "can't allocate space for window type name");
    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);

    win = lin_search(window_types, pref_w_type);

    spsassert(win > -1, "window type not recognized");

    if (debug_level)
	Fprintf(stderr, "window_type = %s, win = %d\n", window_type, win);

    /* -o order */

    if (!(oflag || p_mflag))
	switch (symtype("fft_order"))
	{
	case ST_UNDEF:
	    order = def_order;
	    break;
	case ST_INT:
	    order = getsym_i("fft_order");
	    break;
	default:
	    ERROR_EXIT("Parameter \"fft_order\" not of integer type.")
	    break;
	}

    if ((order > 10) && !zflag)  
      Fprintf(stderr, 
	"%s: WARNING - HEY!!!, order = %ld makes for a huge spectrogram\n",
	ProgName, order);

    tr_length = ROUND(pow(2.0, (double) order));

    if (debug_level)
	Fprintf(stderr, "fft_order = %d, tr_length = %ld\n", order, tr_length);
	
    /* -p range & -s range */

    switch (genhd_type("start_time", &size, ih))
    {
    case HD_UNDEF:
	file_start_time = 0.0;
	break;
    case DOUBLE:
	file_start_time = *get_genhd_d("start_time", ih);
	break;
    default:
	ERROR_EXIT("Header item \"start_time\" not of double type.")
	break;
    }

    if((sf = get_genhd_val("record_freq", ih, (double) -1)) <= 0){
        Fprintf(stderr, "sgram: Record_freq (sf) is not a positive value - exiting.\n");
        exit(1);
      }

    if (debug_level)
	Fprintf(stderr, "file_start_time = %g, sf = %g\n",
		file_start_time, sf);

    if (pflag && sflag)
	ERROR_EXIT("Conflicting options specified: -p and -s.")
    else if (pflag)
    {
	start = 1;
	end = LONG_MAX;
	lrange_switch(prange, &start, &end, 0);
	nan = (end == LONG_MAX) ? 0 : end - start + 1;
	data_start_time = file_start_time + (start - 1) / sf;
    }
    else if (sflag)
    {
	data_start_time = file_start_time;
	data_end_time = FLT_MAX;
	frange_switch(srange, &data_start_time, &data_end_time);
	start = 1 + LROUND((data_start_time - file_start_time) * sf);
	if (data_end_time == FLT_MAX)
	{
	    end = LONG_MAX;
	    nan = 0;
	}
	else
	{
	    end = LROUND((data_end_time - file_start_time) * sf);
	    nan = end - start + 1;
	}
    }
    else
    {
	switch (symtype("start"))
	{
	case ST_UNDEF:
	    start = 1;
	    break;
	case ST_INT:
	    start = getsym_i("start");
	    break;
	default:
	    ERROR_EXIT("Parameter \"start\" not of integer type.")
	    break;
	}

	switch (symtype("nan"))
	{
	case ST_UNDEF:
	    nan = 0;
	    break;
	case ST_INT:
	    nan = getsym_i("nan");
	    break;
	default:
	    ERROR_EXIT("Parameter \"nan\" not of integer type.")
	    break;
	}

	end = (nan == 0) ? LONG_MAX : start + nan - 1;
	data_start_time = file_start_time + (start - 1) / sf;
    }

    if (start < 1) {  /* In case of precision problems in start_time specs... */
         start = 1;
	 if (debug_level){
	   Fprintf(stderr,"Can't start before beginning of file.\n");
	   Fprintf(stderr, "Resetting starting sample to 1.\n");
	 }
    }


    if (debug_level) Fprintf(stderr,
	"start = %ld, nan = %ld, end = %ld, data_start_time = %g\n",
	start, nan, end, data_start_time);

    /* -w window_len */

    if (!(wflag || p_mflag))
	switch (symtype("window_len"))
	{
	case ST_UNDEF:
	    window_len = def_window_len;
	    break;
	case ST_FLOAT:
	    window_len = getsym_d("window_len");
	    break;
	default:
	    ERROR_EXIT("Parameter \"window_len\" not of float type.")
	    break;
	}

    window_size = frame_len = LROUND(window_len * sf / 1000.0);

    if((win == WT_SINC) || (win == WT_SINC_C4))
      frame_len = tr_length;

    if (debug_level)
	Fprintf(stderr, "window_len = %g, frame_len = %ld\n",
		window_len, frame_len);

    /* -E pre_emphasis */

    if (!(Eflag || p_mflag))
	switch (symtype("pre_emphasis"))
	{
	case ST_UNDEF:
	    preemph = def_preemph;
	    break;
	case ST_FLOAT:
	    preemph = getsym_d("pre_emphasis");
	    break;
	default:
	    ERROR_EXIT("Parameter \"pre_emphasis\" not of float type.")
	    break;
	}

    if (debug_level)
	Fprintf(stderr, "preemph = %g\n", preemph);

    if (!(Tflag))
	switch (symtype("desired_frames"))
	{
	case ST_UNDEF:
	    break;
	case ST_INT:
	    des_frames = getsym_i("desired_frames");
	    if (des_frames > 0)
	      Tflag++;
	    break;
	default:
	    ERROR_EXIT("Parameter \"desired_frames\" not of int type.")
	    break;
	}


    /* -S step_size */

    if (!(Sflag || p_mflag || Tflag))
	switch (symtype("step_size"))
	{
	case ST_UNDEF:
	    step_size = def_step_size;
	    break;
	case ST_FLOAT:
	    step_size = getsym_d("step_size");
	    break;
	default:
	    ERROR_EXIT("Parameter \"step_size\" not of float type.")
	    break;
	}

    step = LROUND(step_size * sf / 1000.0);


    /* If -T (TI) option is used, the number of frames (number of ffts) 
     * is forced.  This facilitates looking at a narrow region with 
     * higher resolution.   Here we compute a step size that gives 
     * the (approximate) right number of frames
     */

    if (Tflag) {
	
	if (nan == 0) {
	  Fprintf(stderr, 
	      "%s: can't use -T without specifying a range start\n", ProgName);
	  Fprintf(stderr,"\tand stop with -r or -p\n");
	  exit(1);
	}

	if (Sflag && (debug_level || !zflag))
	  Fprintf(stderr, "%s: WARNING -S option overriden by -T\n", ProgName);

	/* computed fixed step size */

	step = LROUND((nan - frame_len)/((float) (des_frames - 1)));

	if (step <= 0) 
	  step = 1;
      }

    /* compute (adjust, in case of -T)  number of frames to fill 
       specified segment */
    
    if (nan == 0) 
      nan = end - start + 1;

    if (nan < frame_len) 
      nframes = 1;
    else {
      nframes = 1 + (nan - frame_len) / step; 
      /* we add one more frame unless the last one ends exactly at the 
	 end point */
      if (nan > (frame_len + (nframes - 1)*step)) {
	nframes++;
	if (debug_level) 
	  Fprintf(stderr, 
	     "%s: WARNING - last frame will exceed range by %ld points\n", 
	      ProgName, frame_len + (nframes - 1)*step - nan);
      }
    }

    if (debug_level) Fprintf(stderr,
	"start = %ld, nan = %ld, frame size = %ld, step = %ld, nframes = %ld\n",
	start, nan, frame_len, step, nframes);

    /*
     * Allocate buffers
     */

    alloc_len = MAX(frame_len, tr_length);
    x = (float *) calloc(alloc_len + 1, sizeof(float));
    px = (float *) calloc(alloc_len, sizeof(float));	
    wx = (float *) calloc(alloc_len, sizeof(float));	
    y = (float *) calloc(alloc_len, sizeof(float));
    spsassert(x && y && wx, "sgram: couldn't allocate enough memory");

    if (frame_len < 1)
	ERROR_EXIT("window must be at least 1 sample wide");
    if (debug_level) Fprintf(stderr, 
	"frame length %ld, transform length %ld, no. frames %ld\n", 
	    frame_len, tr_length, nframes);

    /*
     * only read as many points as length of transform
     */

    if (tr_length < frame_len && debug_level) {
	Fprintf(stderr, 
	    "%s: WARNING - transform length %ld less than frame length %ld\n",
	    ProgName, tr_length, frame_len);
	Fprintf(stderr,"\t\t(framing determined by step size)\n");
    }	
    if (tr_length > frame_len && debug_level) {
	Fprintf(stderr, 
	    "%s: WARNING - transform length %ld greater than frame length %ld\n",
	    ProgName, tr_length, frame_len);
	Fprintf(stderr,
	    "\t %ld zeros padded for each frame\n", tr_length-frame_len);
    }

    /*
     * create, fill in, and write output file header
     */

    oh = new_header(FT_FEA);
    add_source_file(oh, iname, ih);
    oh->common.tag = YES;
    (void) strcpy(oh->common.prog, ProgName);
    (void) strcpy(oh->common.vers, Version);
    (void) strcpy(oh->common.progdate, Date);
    oh->variable.refer = iname;
    add_comment(oh, get_cmd_line(argc, argv));

    freq_format = SPFMT_SYM_EDGE;
    spec_type = SPTYP_DB;
    num_freqs = tr_length/2 + 1;

    spsassert(
	!init_feaspec_hd(oh, YES, freq_format, spec_type,
	    YES, num_freqs, SPFRM_FIXED, (float *) NULL, sf, frame_len,
	    BYTE),
	"Error filling FEA_SPEC header" )

    *add_genhd_l("start", (long *) NULL, 1, oh) = start;
    *add_genhd_l("nan", (long *) NULL, 1, oh) = nan;
    (void) add_genhd_c("sgram_method", hd_method, strlen(hd_method), oh);
    *add_genhd_l("fft_order", (long *) NULL, 1, oh) = order;
    *add_genhd_l("fft_length", (long *) NULL, 1, oh) = tr_length;
    *add_genhd_l("step", (long *) NULL, 1, oh) = step;
    *add_genhd_e("window_type", (short *) NULL, 1, window_types, oh) = win;
    *add_genhd_d("pre_emphasis", (double *) NULL, 1, oh) = preemph;
    if (Tflag) 
      *add_genhd_l("desired_frames", (long *) NULL, 1, oh) = des_frames;

    (void)update_waves_gen(ih, oh, (float) (start + frame_len/2), (float)step);

    write_header(oh, ofile);

    /*
     * set scale factor to turn into continuous spectral density
     */

    scale = 10 * log10(2.0/(tr_length * sf));

    /*
     * allocate spectral record and move to starting position
     */

    spec_rec = allo_feaspec_rec(oh, FLOAT);

    fea_skiprec(ifile, start - 1, ih);

    /*
     * main loop
     */

    position = start;
    for (i = 0; i < nframes && more; i++) {
	if (debug_level > 1)
	    Fprintf(stderr, "frame %d\n", i + 1);

	/*
	 * get sampled data and preemphasize
	 */

	if (i == 0)
	{
	    x[0] = 0.0;
	    actsize = get_sd_orecf(x + 1, (int) frame_len, (int) step, 
				   YES, ih, ifile);
	}
	else
	{
	    actsize = - 1 + get_sd_orecf(x, (int) frame_len + 1, (int) step, 
					 NO, ih, ifile);
	}
	{
	  register float *ip, *op, old, new, pre = preemph;
	  for (j = frame_len, ip = x, old = *ip++, op = px; j--; ) {
	    *op++ = (new = *ip++) - (old*pre);
	    old = new;
	  }
	}

	if ((actsize < frame_len) && debug_level) 
	  Fprintf(stderr, 
	       "%s: WARNING - got fewer points than frame_len in frame %d\n",
	       ProgName, i + 1);

	if (actsize == 0) break;

	if (debug_level > 1)
	    Fprintf(stderr, "got %d points\n", actsize);
	if (debug_level > 2)
	{
	    pr_farray("frame from get_sd_orec", frame_len + 1, x);
	    pr_farray("preemphasized frame", frame_len, px);
	}

	more = (actsize == frame_len);    	

	/*
	 * Window data.
	 */

	(void) gp_window(window_size, px, wx, win, (double *) NULL, &frame_len);

	if (tr_length > actsize) {
	    if (debug_level > 1) Fprintf(stderr, "padding zeros to frame\n");
	    for (j = actsize; j < tr_length; j++) 
		wx[j] = 0.0;
	}

	if (debug_level > 2) pr_farray("windowed frame, input to fft", 
	    MAX(frame_len, tr_length), wx);

	/*
	 * compute total power
	 */

	tot_power = 0.0;
	{
	  register float ssq, *fp;
	  register int cnt;
	  data_length = (tr_length < actsize ? tr_length : actsize);
	  for (cnt = data_length, ssq = 0.0, fp = wx; cnt-- ; fp++)
	    ssq += *fp * *fp;
	  tot_power = ssq / data_length;
	}
	if (debug_level > 1)
	    Fprintf(stderr, "total power = %g\n", tot_power);

	/*
	 * compute fft
	 */
	{
	  register float *wp = y, zero = 0.0;
	  register int cnt = tr_length;
	  do { *wp++ = zero; } while ( --cnt);
	}

	get_rfft(wx, y, order);

	if (debug_level > 2) {
	    Fprintf(stderr, "real, imag, and power from get_fft:\n");
	    for (j = 0; j < tr_length; j++) 
		Fprintf(stderr, "%f\t%f\t%g\n", wx[j], y[j], 
		    wx[j]*wx[j] + y[j]*y[j]);
	}

	/*
	 * fill in and write out spectral record
	 */

	zeros = 0;
	{
	  register float *op, *rp, *ip, zero = 0.0, rfpow, scl = scale, ten = 10.0;
	  register int cnt;
	  for (cnt = num_freqs, rp = wx, ip = y, op = spec_rec->re_spec_val;
	       cnt-- ; rp++, ip++)
	    {
	      rfpow =  (*ip * *ip) + (*rp * *rp);
	      if (rfpow > zero)
		*op++ = scl + ten * log10(rfpow);
	      else
		{
		  *op++ = scl - ten * log10(FLT_MAX);
		  zeros++;
		}
	    }
	}
	if (zeros && !zflag)
	    Fprintf(stderr,
		    "%s: WARNING - zero power at %d points in frame %ld.\n",
		    ProgName, zeros, i+1);

	if (debug_level > 2) {
	    Fprintf(stderr, "real, imag components in spec_rec:\n");
	    for (j = 0; j < num_freqs; j++) 
		Fprintf(stderr, "%f\t%f\t\n", 
		    spec_rec->re_spec_val[j], spec_rec->im_spec_val[j]);
	}
	*spec_rec->tot_power = tot_power;
	*spec_rec->tag = position;
	position += step;

	(void) put_feaspec_rec(spec_rec, oh, ofile);
    }

/*
 * put info in ESPS common
 */

    if (strcmp(oname, "<stdout>") != 0 && strcmp(iname, "<stdin>") != 0) {
	(void) putsym_s("filename", iname);
	(void) putsym_s("prog", ProgName);
	(void) putsym_i("start", (int) start);
	(void) putsym_i("nan", (int) nan);
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

    Fprintf(stderr, "%s: %s -- %d points:\n", ProgName, text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}
