/********************************************************
*
*  This material contains propriety software of Entropic
*  Speech, Inc.  Any reproduction, distribution, or
*  publication without the prior written permission of
*  Entropic Speech, Inc. is strictly prohibited.  Any
*  public distribution of copies of this work authorized
*  in writing by Entropic Speech, Inc. must bear the
*  notice "Copyright 1987 Entropic Speech, Inc."
*
*  Module Name: fft_filter.c
*
*  Written By:   Brian Sublett   
*  Modified by David Burton for ESPS 3.0
*  Modified for FEAFILT by Bill Byrne, 5/24/91
*
*  Purpose:  This program reads data from an ESPS speech data
*	     file and filters it with an filter of
*	     arbitrary length.  The resulting output data
*	     is then printed to another ESPS data file.
*
*  Secrets:  
*
*********************************************************/


static char *sccs_id = "@(#)fft_filter.c	3.18 7/10/98 ESI";

/* 
 * System Includes
*/
# include <stdio.h>

/*
 * ESPS Includes
*/
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feasd.h>
# include <esps/feafilt.h>
# include <esps/unix.h>

/*
 * Defines
*/
# define FILNAMSIZE 40 /* Maximum file name size is  40 characters.    */
# define MAX_FFT_SIZE 16384   /* Maximum FFT taken. */
# define SYNTAX USAGE ("filter [-P parfile] [-f filter name] [-F filt_file] \n[-x debug_level] [-i up/down] [-{pr} range] [-z] in_file out_file");

/*
 * ESPS Functions
*/
void lrange_switch();
char *get_cmd_line();
void get_fft();
void put_sd_recf();
void add_genzfunc();
double log();
int debug_level = 0;

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE  *fpin = stdin, *fpout = stdout; /*input and output stream pntrs*/
    FILE *fpco;				  /* FILT file stream pointer*/
    int i, nsiz, nn, up = 1, down = 1;
    int co_num = 1;
    struct feafilt *filt_rec;		    /*data structure for FEAFILT files*/
    char co_source = 'p';
    char *in_file = NULL, *out_file = "<stdout>"; /*in & out file name*/
    char *filt_file;			    /* FILT file name*/
    char na_name[FILNAMSIZE];    /*holds name of parameter entry for coeffs*/
    char nsiz_name[FILNAMSIZE];	 /*holds name of param entry for # coeffs.*/
    short fflag = NO, interp = NO;

    long start = 1, nan = LONG_MAX, start_p = 0, end = LONG_MAX;
    struct header *ih, *oh, *fh = NULL;	/* header ptrs for in out and FILT*/
    struct zfunc *pzfunc, dummyzfunc;
    char *param_file = NULL, *filter_name = "filter";

    int num_of_files = 0;
    extern char *optarg;
    extern optind;

    char *Version = "3.18";
    char *Date = "7/10/98";

    register float hr, hi;
    static float  data_real[MAX_FFT_SIZE], data_imag[MAX_FFT_SIZE],
	   filt_coef_real[MAX_FFT_SIZE], filt_coef_imag[MAX_FFT_SIZE],
	   inp[MAX_FFT_SIZE];	/*arrays for fft results*/
    float  *fptr1, *fptr2;

    double t;
    static double temp[MAX_FFT_SIZE];

    int     N, c, block_size;
    int range_flag = 0;
    char *range;
    int total_pts = 0;		    /* count total points written */
    int     fft_size, log_fft_size;
    int check_common;		    /* read_params flag */
    int Fflag = 0;
    int zflag = 0;
    long num_channels;
    double start_time = 0;
    double record_freq = 0;

    short  input_field_type;        /* input field type */
    struct feasd *sdrec;            /* pointer to output record */

/* Check the command line options. */

    while ((c = getopt (argc, argv, "P:f:F:x:i:r:p:z")) != EOF)
	{
	switch (c)
	    {
	    case 'P':
		param_file = optarg;
		break;
	    case 'f':
		filter_name = optarg;
		fflag = YES;
		break;
	    case 'F':
		Fflag++;
		filt_file = optarg;
		co_source = 'f';
		TRYOPEN ("filter", filt_file, "r", fpco);
		break;
	    case 'x':
		if (sscanf(optarg,"%d", &debug_level) != 1)
		    {
		    Fprintf (stderr, "Error reading -x arg. Exitting.\n");
		    exit (1);
		    }
		if (debug_level != 0 && debug_level != 1)
		    {
		    debug_level = 1;
		    }
		break;
	    case 'i':
		if ((nn = sscanf (optarg, "%d/%d", &up, &down)) != 2)
		    {
		    Fprintf (stderr,"fft_filter: interpolation format is up/down\n");
		    Fprintf (stderr,"fft_filter: nn=%d up=%d down=%d\n", nn, up, down);
		    Fprintf(stderr, 
			    "\nfft_filter: -i option not supported yet\n");
		    exit (1);
		    }
		if (up < 0 || up > 10)
		    {
		    Fprintf(stderr,"fft_filter: up = %d is illegal.\n", up);
		    exit (1);
		    }
		if (down < 0 || down > 10)
		    {
		    Fprintf(stderr,"fft_filter: down = %d is illegal.\n", down);
		    exit (1);
		    }
		interp = YES;
		break;
            case 'r':
	    case 'p':
		range_flag = 1;
		range = optarg;
		break;
	    case 'z':
		zflag++;
		break;
	    default:
		USAGE ("filter [-P parfile] [-f filter name] [-F filt_file] \n[-x debug_level] [-{rp} range] [-z] in_file out_file");
	    }
	}

/* Change the filtername default if -F was used and -f wasn't. */

    if (fflag == NO && co_source == 'f')
	{
	filter_name = "1";
	}

/* Get the filenames. */

/* 
 * Get the number of filenames on the command line
*/ 
    if((num_of_files = argc - optind) > 2){
	Fprintf(stderr, "Only two file names allowed\n");
	SYNTAX;
    }
    if(debug_level > 0)
	Fprintf(stderr, "fft_filter: num_of_files = %d\n", num_of_files);

    if(num_of_files == 0){
	Fprintf(stderr, "No output filename specified.\n");
	SYNTAX;
    }

    if(num_of_files == 1){
/*
 * Only output file specified on command line
*/
    	out_file = eopen("fft_filter", argv[optind], "w", NONE, NONE,
			    (struct header **)NULL, &fpout);
	if (debug_level) 
	    {
	    Fprintf (stderr,"Output file is %s\n", out_file);
	    }
	  check_common = SC_CHECK_FILE;
    }


    if (num_of_files == 2)

/*
 * Both input and output file names specified on command line
*/
	{
	in_file = eopen("fft_filter", argv[optind++], "r", FT_FEA, FEA_SD,
			&ih, &fpin);
	if (debug_level) 
		Fprintf (stderr,"fft_filter: Input file is %s\n", in_file);


    	out_file = eopen("fft_filter", argv[optind], "w", NONE, NONE,
			    (struct header **)NULL, &fpout);
	if (debug_level) 
	    {
	    Fprintf (stderr,"fft_filter: Output file is %s\n", out_file);
	    }
	/*
	 * decide whether to check common
	 */
	if(strcmp(in_file, "<stdin>") == 0)
	  check_common = SC_NOCOMMON;
	else
	  check_common = SC_CHECK_FILE;
	}


/* Read the parameter file. */

            if(read_params(param_file, check_common, in_file) == -3){
		Fprintf(stderr, 
		"fft_filt: Can't read params file or Common\n");
		exit(1);    
	    }
	    if(in_file == NULL){
		if(symtype("filename") == ST_UNDEF){
	        Fprintf(stderr, 
		"fft_filter: No input file on command line or in Common\n");
		exit(1);
		}
		
		in_file = eopen("fft_filter", getsym_s("filename"), "r",
    		    FT_FEA, FEA_SD, &ih, &fpin);
		if(debug_level > 0)
		    Fprintf(stderr, "fft_filter: Input file name from common is %s\n",
		    in_file);

	    }
/*
 * abort if mulichannel or complex
*/
    if((num_channels = 
        get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) != 1){
          Fprintf(stderr, 
                  "fft_filter: Multichannel data not supported yet - exiting.\n");
           exit(1);
         }

    if(is_field_complex(ih, "samples") == YES)
      {
           Fprintf(stderr, "fft_filter: Complex data not supported - exiting.\n");
           exit(1);
         }    

    input_field_type = get_fea_type( "samples", ih);
    if (debug_level) 
      switch( input_field_type ) {
	  case DOUBLE: 
	   Fprintf(stderr, "Input field type: double\n");
	   break;
	  case FLOAT:
	   Fprintf(stderr, "Input field type: float\n");
	   break;
	  case LONG: 
	   Fprintf(stderr, "Input field type: long\n");
	   break;
	  case SHORT:
	   Fprintf(stderr, "Input field type: short\n");
	   break;
	  case BYTE:
	   Fprintf(stderr, "Input field type: byte\n");
	   break;
	  default:
	   Fprintf(stderr, "Input field type unrecognized\n");
	   exit(1);
	   break;
	 }
	  
	  
/* Create the output header. */

    oh = new_header (FT_FEA);
    add_comment(oh, get_cmd_line(argc, argv));
    add_source_file (oh, in_file, ih);
    Strcpy (oh->common.prog, "fft_filter");
    Strcpy (oh->common.vers, Version);
    Strcpy (oh->common.progdate, Date);

    if((record_freq = get_genhd_val("record_freq", ih, (double)0.)) == 0){
      Fprintf(stderr,"fft_filter: Input file record_freq == 0 - exiting.\n");
      exit(1);
    }
    if((init_feasd_hd(oh, input_field_type, (int) 1, 
		      &start_time, (int) 0, record_freq)) != 0){
      Fprintf(stderr, 
	      "fft_filter: Couldn't allocate input fea_sd header - exiting\n");
      exit(1);
    }
    /*
     * Get range info, if not on command line
    */
    if (!range_flag){
	if(symtype("start") == ST_UNDEF){
	    if(debug_level > 0)
	    Fprintf(stderr, 
		"fft_filter: No starting point in params or Common\n");
	}
        else
	    start = (long) getsym_i("start");
        start = start - 1;  /* Make start an offset. */
        if (start < 0) start = 0;
	if(symtype("nan") == ST_UNDEF){
	    if(debug_level > 0)
	    Fprintf(stderr, 
	    "fft_filter: No NAN value specified in params file or Common\n");
	}
	else
            nan = (long) getsym_i("nan");
	}

/* Zero the arrays.   */

    for (i=0; i<MAX_FFT_SIZE; i++)
	{
	filt_coef_real[i] = filt_coef_imag[i] = 0.0;
	data_real[i] = data_imag[i] = 0.0;
	inp[i] = 0.0;
	}

/* Get the coefficients. */

    if (co_source == 'p')
	{
	(void)sprintf (nsiz_name,"%s_nsiz", filter_name);
	if(symtype(nsiz_name) == ST_INT)
            nsiz = getsym_i(nsiz_name);
	else{
	    Fprintf(stderr, 
		"fft_filter: The number of coefficients not specified\n");
	    exit(1);
	}
	    
	if (debug_level)
	    {
	    Fprintf (stderr,
	    "fft_filter: filtername is %s, nsiz_name is %s\n", filter_name, nsiz_name);
	    Fprintf (stderr,"fft_filter: from parameter file: nsiz = %d\n", nsiz);
	    }

        (void)sprintf (na_name, "%s_num", filter_name);
	if (debug_level)
	    Fprintf (stderr,"fft_filter: na_name is %s\n", na_name);
        if ((nn=getsym_da(na_name,temp,nsiz)) != nsiz)
            {
            Fprintf (stderr, "Wrong number of numerator coefficients in %s\n", param_file);
            exit (1);
            }

/* Put it into a zfunc.  */

	for (i=0; i<nsiz; i++)
	    {
	    filt_coef_real[i] = temp[i];
	    }
        pzfunc = new_zfunc(nsiz, (int)0, filt_coef_real, (float *)NULL);
	}

    else
	{
	fh = read_header (fpco);
	add_source_file (oh, filt_file, fh);
	co_num = atoi (filter_name);

	if (co_num > 1) fea_skiprec (fpco, (long)(co_num - 1), fh);
	filt_rec = allo_feafilt_rec (fh);
	if(get_feafilt_rec (filt_rec, fh, fpco) == EOF){
	    Fprintf(stderr, 
		"fft_filter: Filt record %d doesn't exist in %s\n",
		 co_num, filt_file);
	    exit(1);
	}

/*	pzfunc = &(filt_rec->filt_rec); */
/*	pzfunc = filtrec_to_zfunc( filt_rec ); */
	dummyzfunc = feafilt_to_zfunc( filt_rec ); 
	pzfunc = &dummyzfunc;

	if (pzfunc->dsiz != 0)
	    {
	    Fprintf (stderr,
		     "fft_filter: denominator size (dsiz) = %d; not = 0.\n",
		     pzfunc->dsiz);
	    Fprintf (stderr,"fft_filter: Only works on FIR filters.\n");
		     
	    exit (1);
	    }
	nsiz = pzfunc->nsiz;
	for (i=0; i<nsiz; i++)
	    {
	    filt_coef_real[i] = pzfunc->zeros[i];
	    }
	}

    if (debug_level == 1)
	{
        Fprintf (stderr,"start-1=%ld nan=%ld\n", start, nan);
	}

/* Store the order and coefficients in the output header. */

    (void)add_genzfunc("fft_filter", pzfunc, oh);

    if (debug_level == 1)
	{
        Fprintf(stderr,"There are %d coefficients.\n", nsiz);
        for (i=0; i<nsiz; i++)
	    {
            Fprintf(stderr,"zeros[%d] = %lf\n", i, pzfunc->zeros[i]);
	    }
	}

    fft_size = 1;
    log_fft_size = 0;
    i = nsiz + 1;
    while (i / 2 > 0)
    {
	i = i / 2;
	fft_size *= 2;
	log_fft_size++;
    }
    if (fft_size > MAX_FFT_SIZE/16)
	{
	if (fft_size > MAX_FFT_SIZE)
	    {
	    Fprintf (stderr,
		     "fft_filter: fft_size=%d too large;reduce filter size\n", 
		     fft_size);
	    exit (1);
	    }
	else {
	    fft_size = MAX_FFT_SIZE;
	    log_fft_size = ROUND(log((double)MAX_FFT_SIZE)/log(2.0));
	  }
	if (nsiz == MAX_FFT_SIZE)
	    {
	    Fprintf (stderr,"fft_filter: nsiz=%d too large\n", nsiz);
	    exit (1);
	    }
	}
    else
	{
        fft_size *= 16;
        log_fft_size += 4;
	}

    for (i=0; i<nsiz; i++) filt_coef_real[i] /= fft_size;

/* Compute the fourier transform of input filter coefficient */

    (void)get_fft (filt_coef_real, filt_coef_imag, log_fft_size);

/* Parse range -- read whole file unless -p option or common overides. */

    if (range_flag)
	{
	lrange_switch (range, &start, &end, 1);
	if (start != 0) start -= 1;  /* start becomes an offset. */
        nan = end - start;
	}
    if (nan == 0) nan = LONG_MAX;
    if (nan < 0)
	{
	Fprintf (stderr,
	"fft_filter: Number of points to process (nan) = %ld less than zero\n",
		 nan);
	exit (1);
	}

    if (debug_level == 1)
	{
        Fprintf(stderr,"fft_filter: start = %ld nan = %ld\n", start+1, nan);
	}


/*
 * Update start_time generic
*/
    {
      long start_adj = start + 1;
      if(zflag && co_source != 'p')
	start_adj = start + 1 - 
	  (long)get_genhd_val("delay_samples", fh, (double)0);

    update_waves_gen(ih, oh, (float)start_adj, (float)1);
    }


/* Initialize the data arrays from the data. */

    /*
     * Move forward to starting point in file, but
     * save points for initializing filter
     */
        
    if((start_p = start - nsiz) > 0)
      fea_skiprec(fpin, start_p, ih);
    for (i=0; i<nsiz; i++)
        {
        if (start_p + i < 0) 
	    inp[i] = 0;
        else 
	    if((get_sd_recf (&inp[i], 1, ih, fpin)) == 0){
		Fprintf(stderr, 
		    "fft_filt: Hit end of file initializing filter\n");
		exit(1);
	    }
		
        }


/* Print the new output header to the output file. */

    write_header (oh, fpout);
    sdrec = allo_feasd_recs( oh, FLOAT, (long)MAX_FFT_SIZE, (char *)data_real, NO);
    if ( sdrec == NULL ) {
	Fprintf(stderr, "fft_filter: unable to allocate output record\n");
	exit(1);
      }
    if (debug_level == 1) 
	{
	Fprintf(stderr,"Header written to %s\n", out_file);
	Fprintf (stderr,"output record allocated\n");
	Fprintf (stderr,"fft_filter: Prior to main filter loop.\n");
	}

/*
 * Check generic header item samp_freq for consistency with data sf
*/
    if(Fflag > 0)
    {
	double samp_freq;
	samp_freq = get_genhd_val("samp_freq", fh, (double) 0.);
    	
	if(samp_freq != record_freq)
	    Fprintf(stderr, 
	"fft_filter: WARNING - Data sampled at %g samples/sec;\n filter designed for %g samples/second data.\n",
	    record_freq, samp_freq);
    }

/*
 * Main Loop - filter the data
*/
    block_size = fft_size - nsiz;

    while((N = get_sd_recf(&inp[nsiz], block_size, ih, fpin)) != 0)
    {
	/*pad with zeros if needed*/
	if (N < block_size)
	    for (i = N; i < block_size; i++)
		inp[i + nsiz] = 0.0;

	if (debug_level > 2)
	    Fprintf (stderr, "nan = %d\tread %d samples\n", nan,  N);

	/* Move data to complex array */
	for (i = 0; i < fft_size; i++)
	{
	    data_real[i] = inp[i];
	    data_imag[i] = 0.0;
	}

	/* perform fft of input data */
	(void)get_fft (data_real, data_imag, log_fft_size);

	/*perform convolution in frequency domain  and complex conjugation */
	for (i = 0; i < fft_size; i++)
	{
	    t = data_real[i];
	    hr = filt_coef_real[i];
	    hi = filt_coef_imag[i];
	    data_real[i] = hr * t - hi * data_imag[i];
	    data_imag[i] = - hi * t - hr * data_imag[i];
	}

	/* perform inverse fft of the output */
	get_fft (data_real, data_imag, log_fft_size);

	/* output data */
	if(N < nan){
	    nan -=N;
	    total_pts += N;
	    put_feasd_recs( sdrec, (long)nsiz, N, oh, fpout);
/*	    put_sd_recf (&data_real[nsiz], N, oh, fpout); */
	}
	else{/*hit end of specified range*/
	    total_pts += nan;
	    put_feasd_recs( sdrec, (long)nsiz, (long)nan, oh, fpout); 
/* 	    put_sd_recf (&data_real[nsiz], (int)nan, oh, fpout); */
	    break;
	}


        /*shift the last extra samples to the beginning of the input array */
	if (N == block_size)
	{			/* if not then no more data */
	    fptr1 = inp;
	    fptr2 = inp + N;
	    for (i = 0; i < nsiz; i++)
		*fptr1++ = *fptr2++;/* avoids overwriting */
	    if (debug_level > 2)
		Fprintf (stderr, 
		    "fft_filter: did the shifts for the next block\n");
	}
    }

/*
 * Write Common if appropriate
*/
    if(strcmp(out_file, "<stdout>") != 0){
	if(putsym_s("filename", out_file) != 0)
	    Fprintf(stderr, "fft_filter: Could not write ESPS Common\n");
	(void)putsym_s("prog", "fft_filter");
	(void)putsym_i("start", 1);
	(void)putsym_i("nan", total_pts);
    }
/* Close all of the files. */

    (void)fclose (fpin);
    (void)fclose (fpout);
    exit(0);
    return(0); /*lint pleasing*/
    }
