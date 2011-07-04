/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *   "Copyright (c) 1988, 1990 Entropic Speech, Inc. All rights reserved."
 *
 * Program: fft.c (FEA_SPEC version)
 *
 * This program computes FFTs from SD files to FEA_SPEC files
 *
 *  Rodney Johnson, Entropic Speech, Inc.		
 *  Modified by John Shore to add various options, and remove ndrec 
 *  dependence;
 *  Modified for ESPS 3.0 by John Shore
 *  Revised by Shore for overlapping frames and windowing.
 *  Converted from SPEC to FEA_SPEC output by Rod Johnson.
 *  Converted from SD to FEA_SD input by David Burton
 */
#ifndef lint
    static char *sccs_id = "@(#)fft.c	3.30	7/8/96	 ESI";  
#endif
char *Version = "3.30";
char *Date = "7/8/96";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/limits.h>
#include <esps/feaspec.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/window.h>

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}

#define Fprintf (void)fprintf

#define SYNTAX \
USAGE("fft [-c] [-l frame_len] [-o order] [-{pr} range] [-w window_type]\n [-x debug_level] [-z] [-P param] [-S step] [-O freq. offset] sdfile specfile")

#define WT_PREFIX "WT_"

/*
 * global variables
 */

char	*ProgName = "fft";
int	debug_level = 0;		/* debug level */
void	get_range();
void	pr_farray();

/*  
 * external functions
 */
double  log10(), pow(), ceil(), sqrt();
char	*get_cmd_line();
void	lrange_switch();
char    *eopen();
void	get_rfft();
void    get_fft();
long	atol();
/* SDI 26/6/06 added LINUX to avoid compiler error */
#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char    *calloc();
char    *malloc();
#endif
#ifndef DEC_ALPHA
char	*strcat();
#endif
/*
 * main program
 */
main(argc, argv)
    int	    argc;
    char    **argv;
{
    char    *param_file = NULL;	/* parameter file name */
    FILE    *ifile = stdin,	/* input and output file streams */
            *ofile = stdout;
    struct header  		/* input and output file headers */
            *ih, *oh;
    char    *iname, 		/* input and output file names */
	    *oname;
	    
    struct feaspec *spec_rec;   /* record for spectral data */

    struct feasd *sd_rec;       /* record for input data */

    extern int
            optind;
    extern char
            *optarg;
    int     ch;
    int     order;		/* order of fft */
    char    *prange = NULL;	/* string for range specification (-p) */
    int	    zflag = 0;		/* flag for -z (silent) option */
    int     oflag = 0;		/* flag for order option */
    int	    pflag = 0;		/* flag for range option */
    int	    cflag = 0;		/* flag for complex option */
    int	    wflag = 0;		/* flag for window option */
    int     lflag = 0;		/* flag for -l frame_len option */
    int	    Sflag = 0;		/* flag for -S option (step size) */
    char    *window_type;	/* name of type of window to apply to data */
    char    *pref_w_type;	/* window type name with added prefix */
    long    i, j;
    short   freq_format;
    short   spec_type;
    long    num_freqs;
    long    frame_len = -1;	/* number of sampled data points */
    long    tr_length;		/* number of points in transform */
    long    hf;			/* half number of frequencies */
    float   sf;
    int	    actsize;		/* actual number points read */
    int	    data_length;	/* actual number points transformed */
    long    nframes = 1;	/* number fixed length frames to process */
    float   fpow;		/* power at one frequency */
    float   tot_power;		/* total power */
    float   *x, *wx, *y, *wy;	/* arrays for data, windowed data, and fft */
    float   scale;		/* scaling constant */
    int	    more = 1;		/* flag to indicate more sampled data */
    long    position;		/* current position in SD file */
    long    step;		/* step size for shifting frames */
    long    start = 1;
    long    nan = 0; 
    int	    win;		/* window type code */
    extern char
	    *window_types[];	/* standard window type names */
    int	    init_err;		/* return code from init_sd_rec */
    int	    complex;		/* NO or YES for data type */
    int	    in_type;		/* input data type */
    int	    num_channels;	/* number of input data channels */
    float   *fdata;		/* pointer to data in fea_sd record */
    float_cplx
	    *cdata;		/* pointer to data in fea_sd record */
     double	offset;		/* frequency offset (-O option) */
     int	Oflag = 0;	/* offset was specified via -O */
     float	*freqs = NULL;	/* vector of freqs for freq. offset */
     double	freq;

    /*
     * process command line options
     */
    while ((ch = getopt(argc, argv, "cl:o:p:r:w:x:O:P:S:z")) != EOF)
        switch (ch)  {
	case 'c':
	    cflag++;
	    break;
	case 'l':
	    frame_len = atoi(optarg);
	    lflag++;
	    break;
        case 'o':
            order = atoi(optarg);
	    oflag++;
	    break;
        case 'p': 
        case 'r': 
            prange = optarg;
	    pflag++;
            break;
	case 'w':
	    window_type = optarg;
	    wflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
 	case 'O':
 	    offset = atof(optarg);
 	    Oflag++;
 	    break;
	case 'z':
	    zflag++;
	    break;
	case 'P':
	    param_file = optarg;
	    break;
	case 'S':
	    step = atol(optarg);
	    Sflag++;
	    break;
        default: 
            SYNTAX;
            break;
        }

/*
 * check number of range specifications
 */
    if(pflag > 1){
	Fprintf(stderr,
		"fft: range should only be specified once - exiting.\n");
	exit(1);
    }

/*
 * process file arguments
 */
    if (optind < argc)
	iname = eopen(ProgName,
		      argv[optind++], "r", FT_FEA, FEA_SD, &ih, &ifile);
    else
    {
	Fprintf(stderr, "fft: no input sampled data file specified.\n");
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "Input file is %s\n", iname);
/* 
 * check if input file is multichannel
 */
    if ((num_channels = 
	 get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) != 1)
    {
        Fprintf(stderr, "fft: Multichannel data not supported yet\n");
	exit(1);
    }

/*
 * Check if input data is complex
 */

    in_type = get_fea_type("samples", ih);
    switch (in_type)
    {
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
        complex = NO;
        break;
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
        complex = YES;
        break;
    default:
        Fprintf(stderr, "%s: bad type code (%d) in input file header.\n",
                ProgName, in_type);
        exit(1);
        break;
    }
/*
 * setup output fea file
 */
    if (optind < argc)
	oname = eopen(ProgName, argv[optind++], "w", NONE, NONE, &oh, &ofile);
    else {
	Fprintf(stderr, "%s: no output FEA_SPEC file specified.\n", ProgName);
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "Output file is %s\n", oname);
/*
 * process parameters
 */
    if (strcmp(iname, "<stdin>") != 0)
	(void) read_params(param_file, SC_CHECK_FILE, iname);
    else
        (void) read_params(param_file, SC_NOCOMMON, iname);

    if (!oflag)
	order = 
	    (symtype("order") != ST_UNDEF)
	    ? getsym_i("order")
	    : 10;
    
    if ((order >= 20) && !zflag)
	Fprintf(stderr,
		"%s: WARNING - HEY!!!, order = %ld makes for a huge transform\n",
		ProgName, order);

    tr_length = LROUND(pow(2.0, (double) order));

    get_range(&start, &nan, &frame_len, &step, &nframes, prange, 
	      pflag, lflag, Sflag, tr_length, ih, zflag);

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

    if (debug_level)
	Fprintf(stderr,
		"%s: start = %ld, nan = %ld, order = %d, tr_length = %d\n", 
		ProgName, start, nan, order, tr_length);
    if (debug_level) 
	Fprintf(stderr, "%s: frame size = %ld, step = %ld, nframes = %ld\n",
		ProgName, frame_len, step, nframes);

/*
 * Allocate data arrays
 */
    x = (float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float));
    y = (float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float));
    spsassert(x != NULL && y != NULL, 
	      "fft: couldn't allocate enough memory");
    wx=(float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float)); 
    wy=(float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float)); 
    spsassert(wx != NULL && wy != NULL, 
	      "fft: couldn't allocate enough memory");

    if (start < 1)
	ERROR_EXIT("can't have negative starting point");

    /*
     * only read as many points as length of transform
     */
    if ((tr_length < frame_len) && !zflag) {
	Fprintf(stderr, 
	    "fft: WARNING - transform length %ld less than frame length %ld\n",
	    tr_length, frame_len);
	Fprintf(stderr,"\t\t(framing determined by step size)\n");
    }	
    if ((tr_length > frame_len) && !zflag) {
	Fprintf(stderr, 
		"fft: WARNING - transform length (%ld) greater than frame length (%ld)\n",
		tr_length, frame_len);
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

    update_waves_gen(ih, oh, (float) (start + frame_len/2), (float)step);

    sf = *get_genhd_d("record_freq", ih);

      if (cflag)
	{
	  freq_format = SPFMT_ASYM_EDGE;
	  spec_type = SPTYP_CPLX;
	  num_freqs = tr_length + 1;
	  hf = tr_length/2;
	}
      else if(complex) {
	if (Oflag) {
	  freq_format = SPFMT_ARB_FIXED;
	  spec_type = SPTYP_DB;
	  num_freqs = tr_length + 1;
	  hf = tr_length/2;
	  freqs = (float *) malloc(num_freqs * sizeof(float));
	  /* DEBUG */
	  if (debug_level) fprintf(stderr, "offset %g, sf %g\n", offset, sf);
	  for (freq = offset - sf/2.0, i = 0; i < num_freqs; i++, freq += sf/tr_length)
	    freqs[i] = freq;
	} else {
	  freq_format = SPFMT_ASYM_EDGE;
	  spec_type = SPTYP_DB;
	  num_freqs = tr_length + 1;
	  hf = tr_length/2;
	}
      }
      else
	{
	  freq_format = SPFMT_SYM_EDGE;
	  spec_type = SPTYP_DB;
	  num_freqs = tr_length/2 + 1;
	}

    init_err = init_feaspec_hd(oh, YES, freq_format, spec_type, YES,
			      num_freqs, SPFRM_VARIABLE, freqs, sf,
			      frame_len, FLOAT);
    spsassert(!init_err, "Error filling FEA_SPEC header");

    if (Oflag)
      *add_genhd_f("sf", (float *) NULL, 1, oh) = sf;
    *add_genhd_l("start", (long *) NULL, 1, oh) = start;
    *add_genhd_l("nan", (long *) NULL, 1, oh) = nan;
    *add_genhd_l("fft_length", (long *) NULL, 1, oh) = tr_length;
    *add_genhd_l("step", (long *) NULL, 1, oh) = step;
    *add_genhd_e("window_type", (short *) NULL, 1, window_types, oh) = win;
    (void) add_genhd_c("input_data", (complex) ? "complex" : "real", 0, oh);

    write_header(oh, ofile);

    /*
     * Allocate input record and set up pointer to data
     */

    if(complex)
    {
	sd_rec = allo_feasd_recs(ih, FLOAT_CPLX, frame_len, (char *) NULL, NO);
	cdata = (float_cplx *)sd_rec->data;
    }
    else
    {
	sd_rec = allo_feasd_recs(ih, FLOAT, frame_len, (char *) NULL, NO);
	fdata = (float *)sd_rec->data;
    }

    /*
     * allocate spectral record and move to starting position
     */

    spec_rec = allo_feaspec_rec(oh, FLOAT);

    if (start > 1) fea_skiprec(ifile, start - 1, ih);

    /*
     * main loop
     */

    position = start;
    for (i = 0; i < nframes && more; i++) {
	if (debug_level > 1)
	    Fprintf(stderr, "%s: frame %d\n", ProgName, i + 1);

	tot_power = 0;
	if (complex == NO){
	    /*
	     * initialization; have to zero out y otherwise get_rfft
	     * thinks it's called with non-real data
	     */
	    for (j = 0; j < tr_length; j++) 
		wy[j] = 0.0;
	}
	/*
	 * get sampled data
	 */

	if (i == 0) 
	    actsize = get_feasd_recs(sd_rec, 0L, frame_len, ih, ifile);
	else
	    actsize = get_feasd_orecs(sd_rec, 0L, frame_len, step, ih, ifile);

	if ((actsize < frame_len) && !zflag)
	    Fprintf(stderr, 
		"%s: WARNING - got fewer points than frame_len in frame %d\n",
		ProgName, i + 1);

	if (actsize == 0) break;
	if (debug_level > 1)
	    Fprintf(stderr, "%s: got %d points\n", ProgName, actsize);

	if (debug_level > 2) {
	    if (!complex)
		pr_farray("frame from get_feasd_orecs", frame_len, fdata);
	}
	more = (actsize == frame_len);    	

	/* Window data */
	if (complex){
	    for(j=0; j<frame_len; j++){
		x[j] = cdata[j].real;
		y[j] = cdata[j].imag;
	    }
	    (void) window(frame_len, x, wx, win, (double *) NULL);
	    (void) window(frame_len, y, wy, win, (double *) NULL); 
	}
	else
	    (void) window(frame_len, fdata, wx, win, (double *) NULL);


	if (tr_length > actsize) {
	    if (debug_level > 1)
		Fprintf(stderr, 
			"%s: padding zeros to frame\n", ProgName);
	    for (j = actsize; j < tr_length; j++)
	    {
		wx[j] = 0.0;
		wy[j] = 0.0;
	    }
	}

	if (debug_level > 2) {
	    pr_farray("windowed frame, real input to fft", 
		      MAX(frame_len, tr_length), wx);
	    if(complex)
		pr_farray("windowed frame, imaginary input to fft", 
			  MAX(frame_len, tr_length), wy);
	}
	/* compute total power
	 */
	tot_power = 0.0;
	data_length = (tr_length < actsize ? tr_length : actsize);
	for (j = 0; j < data_length; j++) {
	    if(complex)
		tot_power += wx[j]*wx[j] + wy[j]*wy[j];
	    else
		tot_power += wx[j]*wx[j];
	}
	tot_power = tot_power / data_length;

	if (debug_level > 1)
	    Fprintf(stderr, "fft: total power = %g\n", tot_power);
	/*
	 * compute fft
	 */
        if (complex)
	    get_fft(wx, wy, order);
        else
	    get_rfft(wx, wy, order);

	if (debug_level > 2) {
	    Fprintf(stderr, "fft: real, imag, and power from get_fft:\n");
	    for (j = 0; j < tr_length; j++) 
		Fprintf(stderr, "%f\t%f\t%g\n", wx[j], wy[j], 
			wx[j]*wx[j] + wy[j]*wy[j]);
	}
	/*
	 * fill in and write out spectral record
	 */
	if (cflag) /* complex output */
	{
	    /* ASYM_EDGE format */

	    scale = sqrt(1 / (data_length * sf));
			/* scale factor to make continuous spectral density */

	    if(debug_level > 1)
		Fprintf(stderr, "fft: scale = %f\n", scale);

	    for (j = 0; j < num_freqs; j++)
	    {
		/*  
		 * for ASYM_EDGE stores frequencies (-hf, ..., 0, ..., hf),
		 * hf = tr_length / 2
		 */

		if (j < hf)
		{
		    spec_rec->re_spec_val[j] = scale * wx[j + hf];
		    spec_rec->im_spec_val[j] = scale * wy[j + hf];
		}
		else
		{
		    spec_rec->re_spec_val[j] = scale * wx[j - hf];
		    spec_rec->im_spec_val[j] = scale * wy[j - hf];
		}
	    }
	}
	else if (complex) /* complex input and dB output */
	{
	    /* ASYM_EDGE format */

	    int zeros = 0;

	    scale = 10 * log10(1 / (data_length * sf));
			/* scale factor to make continuous spectral density */

	    if(debug_level > 1)
		Fprintf(stderr, "fft: scale = %f\n", scale);

	    for (j = 0; j < num_freqs; j++)
	    {
		/*  
		 * for ASYM_EDGE stores frequencies (-hf, ..., 0, ..., hf),
		 * hf = tr_length / 2
		 */

		if (j < hf) {
		    fpow = wx[j + hf]*wx[j + hf] + wy[j + hf]*wy[j + hf];
		    if(fpow > 0)
			spec_rec->re_spec_val[j] = scale + 10 * log10(fpow);
		    else
			spec_rec->re_spec_val[j] = scale - 10 * log10(FLT_MAX);
		}
		else {
		    fpow = wx[j - hf]*wx[j - hf] + wy[j - hf]*wy[j - hf];
		    if(fpow > 0)
			spec_rec->re_spec_val[j] = scale + 10 * log10(fpow);
		    else
			spec_rec->re_spec_val[j] = scale - 10 * log10(FLT_MAX);
		}
	    }
	}	    
	else /* real input and dB output */
	{
	    /* SYM_EDGE format */

	    int	zeros = 0;

	    scale = 10 * log10(2.0/(data_length * sf));
			/* scale factor to make continuous spectral density */

	    if(debug_level > 1)
		Fprintf(stderr, "fft: scale = %f\n", scale);

	    for (j = 0; j < num_freqs; j++)
	    {
		fpow =  wx[j]*wx[j] + wy[j]*wy[j];
		if (fpow > 0)
		    spec_rec->re_spec_val[j] = scale + 10 * log10(fpow);
		else
		{
		    spec_rec->re_spec_val[j] = scale - 10 * log10(FLT_MAX);
		    zeros++;
		}
	    }
	    if (zeros && !zflag)
		Fprintf(stderr,
		    "%s: WARNING - zero power at %d points in frame %ld.\n",
		    ProgName, zeros, i+1);
	}
	if (debug_level > 2) {
	    Fprintf(stderr, "fft: real, imag components in spec_rec:\n");
	    for (j = 0; j < num_freqs; j++) 
		Fprintf(stderr, "%f\t%f\t\n", 
			spec_rec->re_spec_val[j], spec_rec->im_spec_val[j]);
	}
	*spec_rec->frame_len = actsize;
	*spec_rec->tot_power = tot_power;
	*spec_rec->tag = position;
	position += step;

	put_feaspec_rec(spec_rec, oh, ofile);
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

void
get_range(srec, nan, fsize, step, nfrm, rng, rflag, lenflag, sizflag, 
	  tlen, inhd, zflag)
    long *srec;			/* starting point */
    long *nan;			/* number of points */
    long *fsize;		/* frame size */
    long *step;			/* step size */
    long *nfrm;			/* number of frames */
    char *rng;			/* range string from range option */
    int rflag;			/* flag for whether range option used */
    int lenflag;		/* flag for whether frame_len option used */
    int sizflag;		/* flag for whether step option used */
    long tlen;			/* transform length */
    struct header *inhd;	/* input file header */
    int zflag;			/* flag for no warnings */
{
    long last;

    *srec = 1;
    last = LONG_MAX;

    if (rflag)
    {
	lrange_switch (rng, srec, &last, 1);	
    }
    else
    {
	if (symtype("start") != ST_UNDEF) 
	    *srec = getsym_i("start");
	if (symtype("nan") != ST_UNDEF)
	{
	    *nan = getsym_i("nan");
	    if (*nan != 0)
		last = *srec + (*nan - 1);
	}
    }

    if (inhd->common.ndrec != -1
		/* not a pipe, so we know how many points are out there */
	&& inhd->common.ndrec < last)
    {
	if (!zflag && last != LONG_MAX)
	    Fprintf(stderr, 
		    "fft: WARNING - not enough points in SD file\n");
	last = inhd->common.ndrec;
    }

    *nan = last - *srec + 1;

    /* frame length: if not set or if set to 0, use transform length */

    if (!lenflag) 
	*fsize = 
	    (symtype("frame_len") != ST_UNDEF)
		? getsym_i("frame_len")
		    : tlen;

    if (*fsize == 0) 
	*fsize = tlen;

    /* step size: if not set or if set to 0, use frame length */

    if (!sizflag)
	*step =
	    (symtype("step") != ST_UNDEF)
		? getsym_i("step")
		    : *fsize;

    if (*step==0)
        *step = *fsize;

    /* number of frames */

    *nfrm = (*nan <= *fsize) ? 1 : 2 + (*nan - *fsize - 1) / *step;

    if ((*fsize > *nan) && !zflag)
    {
	Fprintf(stderr, 
		"%s: WARNING - frame_len %ld larger than range %ld;\n",
		ProgName, *fsize, *nan);
	Fprintf(stderr, 
		"\t single frame will overrun range.\n");
    }
    else
    {
	if (((*fsize + (*nfrm - 1) * *step) > *nan) && !zflag)
	    Fprintf(stderr, 
		"%s: WARNING - last frame will overrun range by %ld points\n", 
		ProgName, *fsize + (*nfrm - 1) * *step - *nan);
    }
}

/*
 * For debug printout of float arrays
 */

void pr_farray(text, n, arr)
    char    *text;
    long    n;
    float   *arr;
{
    int	    i;

    Fprintf(stderr, "%s: %s -- %d points:\n", ProgName, text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}

