/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This program computes the FFT_CEPSTRUM of SD & FEA_SD files to FEA files
 */

static char *sccs_id = "@(#)fftcep.c	1.9	1/22/97	ESI/ERL";

char  *Version = "1.9";
char  *Date = "1/22/97";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/limits.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/window.h>
#include <esps/constants.h>
#include <math.h>

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		 ProgName, text); (void) exit(1);}

#define SYNTAX_ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		 ProgName, text); SYNTAX;}

#define Fprintf (void)fprintf
#define Sprintf (void)sprintf

#define SYNTAX \
USAGE("fftcep [-P param] [-r range] [-l frame_len] [-S step]\n [-w window_type] [-o order] [-F] [-R] [-e element_range] [-z zeroing_range] [-x debug_level] sd_file fea_file")

#define WT_PREFIX "WT_"

#define DEFAULT_ORDER 10

 /*
  * global variables
  */
 char	*ProgName = "fftcep";
 int	debug_level = 0;         /* debug level */
 void	get_range();
 void	pr_farray();
 void	pr_carray();
 /*  
  * external functions
  */
 char	 *get_cmd_line();
 void	 lrange_switch();
 long    *grange_switch();
 char    *eopen();
 void    fft_cepstrum();
 void    fft_cepstrum_r();
 void    fft_ccepstrum();
 void    fft_ccepstrum_r();
 long	 atol();

 /*
  * main program
  */
 main(argc, argv)
      int  argc;
      char **argv;
 {
     extern int
	     optind;
     extern char
	     *optarg;

     char    *param_file = NULL;  /*parameter file name*/
     FILE    *ifile = stdin,	/* input and output file streams */
	     *ofile = stdout;
     char    *iname, *oname;	/* input and output file names */

     struct header
	     *ih, *oh;		/* input and output file headers */
     struct fea_data
	     *cep_rec;		/* record for cepstral spectral data */
     struct feasd
	     *sd_rec;		/* record for input data */

     int     ch;
     long    order;		/* order of fft */
     long    tr_length;         /* transform length (2^order) */
     long    field_length;      /* number of elements in output field */

     char    *erange = NULL;	/* string for element range specification (-e) */
     char    *prange = NULL;	/* string for range specification (-p) */
     char    *zrange = NULL;     /* string for zeroing range spec (-z) */
     int     oflag = 0;		/* flag for order option */
     int     pflag = 0;		/* flag for range option */
     int     wflag = 0;		/* flag for window option*/
     int     lflag = 0;		/* flag for -l frame_len option */
     int     Sflag = 0;		/* flag for -S option (step size)*/
     int     Rflag = 0;          /* flag for -R option (cepstrum) */
     int     Fflag = 0;          /* flag for -F option (output has float data type) */
     int     eflag = 0;          /* flag for -e option (element range) */
     int     zflag = 0;          /* flag for -z option (set elements to zero) */
     char    *window_type;	/* name of type of window to apply to data */
     char    *pref_w_type;	/* window type name with added prefix */
     long    i, j, k;
     long    frame_len = -1;	/* number of sampled data points */
     float   sf;
     int     actsize;		/* actual number points read */
     long    nframes = 1;	/* number fixed length frames to process */
     float   *x_r, *w_r, *w_i;  /* arrays for data, windowed data, and cepstrum */
     int     more = 1;		/* flag to indicate more sampled data */
     long    position;		/* current position in SD file */
     long    step;         	/* step size for shifting frames */
     long    start = 1;
     long    nan = 0; 
     int     win;   		/* window type code */
     extern char
	     *window_types[];	/* standard window type names */

     int     complex_input;      /* NO or YES for data type od input*/
     int     in_type;            /* input data type */
     int     num_channels;       /* number of input data channels */
     float   **fdata;            /* pointer to data in fea_sd record */

     float_cplx
	     **cdata;		/* pointer to data in fea_sd record */
     float_cplx
	     *x_c, *w_c;	/* pointers to input and windowed data, if complex */
     long    *e_array,           /* params for parsing element options */
	     e_array_len, *z_array, z_array_len;
     char    fieldname[20];
     float   **orec_fdata_p;	/* pointer to array of output fields - float */
     float_cplx
	     **orec_cdata_p;	/* pointer to array of output fields - float_cplx */

     static char		/* params for method used */
	     *cplx_cepstrum_codes[] = {"YES", "NO", NULL};
     static char		/* params for data type of output */
	     *data_type_codes[] = {"FLOAT_CPLX", "FLOAT", NULL};


     /*
      * process command line options
      */
     while ((ch = getopt(argc, argv, "l:o:r:w:x:P:S:FRe:z:")) != EOF)
	 switch (ch)  {
	     case 'l':
	     frame_len = atoi(optarg);
	     lflag++;
	     break;

	     case 'o':
	     order = atoi(optarg);
	     tr_length = (1 << order); 
	     oflag++;
	     break;

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

	     case 'P':
	     param_file = optarg;
	     break;

	     case 'S':
	     step = atol(optarg);
	     Sflag++;
	     break;

	     case 'F':
	     Fflag++;
	     break;

	     case 'R':
	     Rflag++;
	     break;

	     case 'e':
	     erange = optarg;
	     eflag++;
	     break;

	     case 'z':
	     zrange = optarg;
	     zflag++;
	     break;

	     default: 
	     SYNTAX;
	     break;
	 }

     /*
      * check number of range specifications
      */
     if(pflag > 1)
	 ERROR_EXIT("range should only be specified once");
     if(eflag > 1)
	 ERROR_EXIT("element range should only be specified once");

     /*
      * process file arguments
      */
     if (optind < argc)
	 iname = eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_SD, &ih, &ifile);
     else 
	 SYNTAX_ERROR_EXIT("no input sampled data file specified");
     if (debug_level)
	 Fprintf(stderr, "Input file is %s\n", iname);

     /* 
      * check if input file is multichannel
      */
     num_channels = get_fea_siz("samples", ih, (short *) NULL, (long **) NULL);
     if (debug_level)
	 Fprintf(stderr, "Channels in input: %d\n", num_channels);

     /*
      * Check if input data is complex_input
      */
     in_type = get_fea_type("samples", ih);
     switch (in_type)
	 {
	     case DOUBLE:
	     case FLOAT:
	     case LONG:
	     case SHORT:
	     case BYTE:
	     complex_input = NO;
	     break;

	     case DOUBLE_CPLX:
	     case FLOAT_CPLX:
	     case LONG_CPLX:
	     case SHORT_CPLX:
	     case BYTE_CPLX:
	     complex_input = YES;
	     break;

	     default:
	     Fprintf(stderr, "%s: bad type code (%d) in input file header.\n",
		     ProgName, in_type);
	     exit(1);
	     break;
	 }

     /*
      * set up output fea file
      */
     if (optind < argc)
	 oname = eopen(ProgName, argv[optind++], "w", NONE, NONE, &oh, &ofile);
     else 
	 SYNTAX_ERROR_EXIT("no output file specified");  
     if (debug_level)
	 Fprintf(stderr, "Output file is %s\n", oname);

     /*
      * process parameters
      */
     if (strcmp(iname, "<stdin>") != 0)
	 (void) read_params(param_file, SC_CHECK_FILE, iname);
     else
	 (void) read_params(param_file, SC_NOCOMMON, iname);

     if (!wflag)
	 window_type =(symtype("window_type") != ST_UNDEF) ? getsym_s("window_type") : "RECT";

     pref_w_type = 
	 malloc((unsigned)((int)strlen(WT_PREFIX) + (int)strlen(window_type) + 1));
     spsassert(pref_w_type, "can't allocate space for window type name");
     (void) strcpy(pref_w_type, WT_PREFIX);
     (void) strcat(pref_w_type, window_type);

     win = lin_search(window_types, pref_w_type);
     spsassert(win > -1, "window type not recognized");

     if (debug_level)
	 Fprintf(stderr, "%s: window_type = %s\n",
		 ProgName, window_type);

     get_range(&start, &nan, &frame_len, &step, &nframes, &order, 
	       &tr_length, prange, pflag, lflag, Sflag, oflag, ih);


     if (!eflag) 
	 if ( symtype("element_range") != ST_UNDEF ) {
	     eflag++;
	     erange = getsym_s("element_range");
	 }
     if (eflag)
	 e_array = grange_switch( erange, &e_array_len);  
     else {
	 e_array_len = tr_length;
	 erange = malloc(15);
	 Sprintf( erange, "%d:%d\n", 0, tr_length-1);
     }

     if (!zflag) 
	 if ( symtype("zeroing_range") != ST_UNDEF ) {
	     zflag++;
	     zrange = getsym_s("zeroing_range");
	 }

     /* check that e opt is sensible and z opt is compat with e opt */
     if (eflag) 
	 for (i=0; i < e_array_len; i++)
	     spsassert( e_array[i] >= 0 && e_array[i] < tr_length, 
		       "element derivation options yields impossible ranges.");
     if (zflag) {
	 z_array = grange_switch( zrange, &z_array_len);  
	 if ( z_array_len > e_array_len) 
	     ERROR_EXIT("zeroing option and element derivation options yield incompatible ranges.");
	 for (i=0; i< z_array_len; i++)
	     spsassert( z_array[i] >= 0 && z_array[i] < e_array_len, 
		       "zeroing option and element derivation options yield incompatible ranges.");
     }

     if ( !Rflag && symtype("method") != ST_UNDEF ) 
	 if ( strcmp("CEPSTRUM", getsym_s("method")) == 0 )
	     Rflag++;

     if ( !Fflag && symtype("data_type") != ST_UNDEF ) 
	 if ( strcmp("FLOAT", getsym_s("data_type")) == 0 )
	     Fflag++;

     if (debug_level) {
	 Fprintf(stderr,
		 "%s: start = %ld, nan = %ld, frame size = %ld, step = %ld nframes = %ld\n",
		 ProgName, start, nan, frame_len, step, nframes);
	 Fprintf(stderr, "%s: order = %d, tr_length = %d\n", 
		 ProgName, order, tr_length);
	 Fprintf(stderr, "%s: %d output field(s) of length %d.\n", ProgName, num_channels,
		 tr_length);
	 if (eflag)
	     Fprintf(stderr, "%s: forming output field(s) frome elements %s.\n", ProgName, erange);
	 else
	     Fprintf(stderr, "%s: -e option not used.\n", ProgName);
	 if (Fflag)
	     Fprintf(stderr, "%s: Output field data type float.\n", ProgName);
	 else
	     Fprintf(stderr, "%s: Output field data type float_cplx.\n", ProgName);
	 if (Rflag)
	     Fprintf(stderr, "%s: Computing real cepstrum.\n", ProgName);
	 else
	     Fprintf(stderr, "%s: Computing complex cepstrum.\n", ProgName);
	 if (zflag)
	     Fprintf(stderr, "%s: zeroing elements %s in output field.\n", ProgName, zrange); 
     }

     /*
      * Allocate data arrays
      */
     if (complex_input) {
	 x_c = (float_cplx *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float_cplx));
	 w_c = (float_cplx *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float_cplx));
	 spsassert( x_c != NULL && w_c != NULL, "fftcep: can't allocate enough memory");
     }
     else {
	 x_r = (float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float));
	 w_r = (float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float)); 
	 w_i = (float *) calloc((unsigned) MAX(frame_len, tr_length), sizeof(float)); 
	 spsassert(w_r != NULL && w_i != NULL && x_r != NULL,
		   "fftcep: couldn't allocate enough memory");
     }

     if (start < 1)
	 ERROR_EXIT("can't have negative starting point");

     /*
      * only read as many points as length of transform
      */
     if (tr_length < frame_len) {
	 Fprintf(stderr, "fftcep: WARNING - transform length %ld less than frame length %ld\n",
		 tr_length, frame_len);
	 Fprintf(stderr,"\t\t(framing determined by step size)\n");
     }	
     if (tr_length > frame_len) {
	 Fprintf(stderr, "fftcep: WARNING - transform length (%ld) greater than frame length (%ld)\n",
		 tr_length, frame_len);
	 Fprintf(stderr, "\t %ld zeros padded for each frame\n", tr_length-frame_len);
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

     sf = get_genhd_val("record_freq", ih, (double)1.0);

     *add_genhd_l("start", (long *) NULL, 1, oh) = start;
     *add_genhd_l("nan", (long *) NULL, 1, oh) = nan;
     *add_genhd_l("step", (long *) NULL, 1, oh) = step;
     *add_genhd_l("frmlen", (long *) NULL, 1, oh) = frame_len;
     *add_genhd_l("order", (long *) NULL, 1, oh) = order;
     *add_genhd_f("src_sf", (float *) NULL, 1, oh) = sf;
     (void) add_genhd_c("element_range", erange, (int) 0, oh);
     if (zflag)
	 (void) add_genhd_c("zeroing_range", zrange, (int) 0, oh);
     *add_genhd_e("window_type", (short *) NULL, 1, window_types, oh) = win;
     *add_genhd_e("cplx_cepstrum", (short *) NULL, 1, cplx_cepstrum_codes, oh) = Rflag;
     *add_genhd_e("data_type", (short *) NULL, 1, data_type_codes, oh) = Fflag;

     if (eflag)
	 field_length = e_array_len;
     else
	 field_length = tr_length;

     for (ch=0; ch<num_channels; ch++) {
	 Sprintf( fieldname, "%s%d", "cepstrum_", ch);
	 if (Fflag) 
	     (void) add_fea_fld(fieldname, (long) field_length,
				(short) 1, (long *) NULL, FLOAT, (char **) NULL, oh);
	 else 
	     (void) add_fea_fld(fieldname, (long) field_length,
				(short) 1, (long *) NULL, FLOAT_CPLX, (char **) NULL, oh);
     }

     write_header(oh, ofile);

     cep_rec = allo_fea_rec(oh);

     if (Fflag)
	 orec_fdata_p = (float **) calloc((unsigned) num_channels, sizeof(float *));
     else
	 orec_cdata_p = (float_cplx **) calloc((unsigned) num_channels, sizeof(float_cplx *));
     for (ch=0; ch<num_channels; ch++) {
	 Sprintf( fieldname, "%s%d", "cepstrum_", ch);
	 if (Fflag)
	     orec_fdata_p[ch] = (float *) get_fea_ptr(cep_rec, fieldname, oh);
	 else
	     orec_cdata_p[ch] = (float_cplx *) get_fea_ptr(cep_rec, fieldname, oh);
     }

     /*
      * allocate fea record and move to starting position
      */

     if(complex_input){
	 sd_rec = allo_feasd_recs(ih, FLOAT_CPLX, frame_len, (char *) NULL, YES);
	 cdata = (float_cplx **) sd_rec->ptrs;
     }
     else{
	 sd_rec = allo_feasd_recs(ih, FLOAT, frame_len, (char *) NULL, YES);
	 fdata = (float **) sd_rec->ptrs;
     }
     spsassert(sd_rec != NULL, "fftcep: can't allocate output record memory.");

     if (start > 1) 
	 fea_skiprec(ifile, (long) (start - 1), ih);


     /*
      * main loop
      */
     position = start;
     for (i = 0; i < nframes && more; i++) {

	 /*
	  * get sampled data
	  */
	 if (i == 0) 
	     actsize = get_feasd_recs(sd_rec, 0L, frame_len, ih, ifile);  
	 else
	     actsize = get_feasd_orecs(sd_rec, 0L, frame_len, step, ih, ifile);

	 if (actsize < frame_len)
	     Fprintf(stderr, "%s: WARNING - got fewer points than frame_len in frame %d\n",
		     ProgName, i + 1);

	 if (actsize == 0) 
	     break;
	 if (debug_level > 1)
	     Fprintf(stderr, "%s: got %d records\n", ProgName, actsize);

	 if (debug_level > 2) {
	     if (!complex_input)
		 pr_farray("frame from get_feasd_orecs", (long) frame_len, fdata[ch]);
	}
	more = (actsize == frame_len);    	

	for (ch=0; ch<num_channels; ch++) {
	    if (debug_level > 1)
		Fprintf(stderr, "%s: frame %d\tchannel: %d\n", ProgName, i + 1, ch);

	    for(j=0; j<actsize; j++){
		if (complex_input)
		    x_c[j] = cdata[j][ch]; 
		else {
		    x_r[j] = fdata[j][ch];
		    w_i[j] = 0.0;
		}
	    }
	    if (tr_length > actsize) {
		if (debug_level > 1) 
		    Fprintf(stderr, "%s: padding zeros to frame\n", ProgName);
		for (j = actsize; j < tr_length; j++) 
		    if (complex_input)
			w_c[j].imag = w_c[j].real = 0.0;
		    else
			w_i[j] = w_r[j] = 0.0;
	    }

	    /* Window data */
	    if (complex_input) 
		(void) windowcf(frame_len, x_c, w_c, win, (float_cplx *) NULL); 
	    else 
		(void) window(frame_len, x_r, w_r, win, (double *) NULL);

	    if (debug_level > 2) {
		if (complex_input)
		    pr_carray("windowed frame, complex input to fft_cepstrum", 
			      (long) MAX(frame_len, tr_length), w_c);
		else {
		    pr_farray("windowed frame, real input to fft_cepstrum", 
			      (long) MAX(frame_len, tr_length), w_r);
		    pr_farray("windowed frame, imaginary input to fft_cepstrum", 
			      (long) MAX(frame_len, tr_length), w_i);
		}
	    }
	    
	    /*
	     * compute fft_cepstrum
	     */
	    if (complex_input) {
		if (Rflag) 
		    fft_cepstrum(w_c, order);
		else
		    fft_ccepstrum(w_c, order);
	    }
	    else {
		if (Rflag)
		    fft_cepstrum_r(w_r, w_i, order);
		else 
		    fft_ccepstrum_r(w_r, w_i, order);
	    }

	    if (debug_level > 2) {
		if (complex_input) {
		    Fprintf(stderr, "fft_cepstrum: real, imag:\n");
		    for (j = 0; j < tr_length; j++) 
			Fprintf(stderr, "%f\t%f\n", w_c[j].real, w_c[j].imag);
		}
		else {
		    Fprintf(stderr, "fft_cepstrum: real, imag:\n");
		    for (j = 0; j < tr_length; j++) 
			Fprintf(stderr, "%f\t%f\n", w_r[j], w_i[j]);
		}
	    }
	    
	    /*
	     * fill in cepstral data
	     */
	    if (complex_input) {
		if (eflag) {
		    for (j=0; j<e_array_len; j++) {
			k = e_array[j];
			if (Fflag) 
			    orec_fdata_p[ch][j] = w_c[k].real;
			else 
			    orec_cdata_p[ch][j] = w_c[k];
		    }
		}
		else {
   		    for (j=0; j<tr_length; j++) {
			if (Fflag) 
			    orec_fdata_p[ch][j] = w_c[j].real;
			else 
			    orec_cdata_p[ch][j] = w_c[j];
		    }
		}
	    }
	    else  {
		if (eflag) {
		    for (j=0; j<e_array_len; j++) {
			k = e_array[j];
			if (Fflag) 
			    orec_fdata_p[ch][j] = w_r[k];
			else {
			    orec_cdata_p[ch][j].real = w_r[k];
			    orec_cdata_p[ch][j].imag = w_i[k];
			}
		    }
		}
		else {
		    for (j=0; j<tr_length; j++) {
			if (Fflag) 
			    orec_fdata_p[ch][j] = w_r[j];
			else {
			    orec_cdata_p[ch][j].real = w_r[j];
			    orec_cdata_p[ch][j].imag = w_i[j];
			}
		    }
		}
	    }

	    if (zflag) {
		for (j=0; j<z_array_len; j++) {
		    k = z_array[j];
		    if (Fflag)
			orec_fdata_p[ch][k] = 0.0;
		    else {
			orec_cdata_p[ch][k].real = 0.0;
			orec_cdata_p[ch][k].imag = 0.0;
		    }
		}
	    }


	}
	/*
	 *  write out cepstral data
	 */

	cep_rec->tag = position;
	position += step;
	    
	put_fea_rec(cep_rec, oh, ofile);
    }    
    
    /*
     * put info in ESPS common
     */
if (strcmp(oname, "<stdout>") != 0 && strcmp(iname, "<stdin>") != 0) {
	(void) putsym_s("filename", iname);
	(void) putsym_s("prog", ProgName);
	(void) putsym_i("start", (int) start);
	(void) putsym_i("nan", (int) (nframes * frame_len));
    }    
    exit(0);
    /*NOTREACHED*/
}


void
get_range(srec, nan, fsize, step, nfrm, order, 
	  tlen, rng, rflag, lenflag, sizflag, oflag, inhd)
    long *srec;			/* starting point */
    long *nan;			/* number of points */
    long *fsize;		/* frame size */
    long *step;			/* step size */
    long *nfrm;			/* number of frames */
    long *order;                /* transform order */
    long *tlen;			/* transform length */
    char *rng;			/* range string from range option */
    int rflag;			/* flag for whether range option used */
    int lenflag;		/* flag for whether frame_len option used */
    int sizflag;		/* flag for whether step option used */
    int oflag;                  /* flag for whether order option used */
    struct header *inhd;	/* input file header */
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
	if (last != LONG_MAX)
	    Fprintf(stderr, 
		    "fft: WARNING - not enough points in SD file\n");
	last = inhd->common.ndrec;
    }

    *nan = last - *srec + 1;

    if (!oflag)
    {
	*order =
	    (symtype("order") != ST_UNDEF)
		? getsym_i("order")
		    : DEFAULT_ORDER;

	*tlen = (1 << *order);
    }

    /* frame length: if not set or if set to 0, use transform length */

    if (!lenflag) 
	*fsize = 
	    (symtype("frame_len") != ST_UNDEF)
		? getsym_i("frame_len")
		    : *tlen;

    if (*fsize == 0) 
	*fsize = *tlen;

    /* step size: if not set or if set to 0, use frame length */

    if (!sizflag)
	*step =
	    (symtype("step") != ST_UNDEF)
		? getsym_i("step")
		    : *fsize;

    if (*step == 0)
	*step = *fsize;

    *nfrm = (*nan <= *fsize) ? 1 : 2 + (*nan - *fsize - 1) / *step;

    if (*fsize > *nan)
    {
	Fprintf(stderr, 
		"%s: WARNING - frame_len %ld larger than range %ld;\n",
		ProgName, *fsize, *nan);
	Fprintf(stderr, "\t single frame will overrun range.\n");
    }
    else
    {
	if ((*fsize + (*nfrm - 1) * *step) > *nan)
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

void pr_carray(text, n, arr)
  char    *text;
  long    n;
  float_cplx  *arr;
{
    int	    i;
    
    Fprintf(stderr, "%s: %s -- %d points:\n", ProgName, text, n);
    for (i = 0; i < n; i++)
	{
	    Fprintf(stderr, "%f %f  ", arr[i].real, arr[i].imag);
	    if (i%3 == 2 || i == n - 1) Fprintf(stderr,  "\n");
	}
}
