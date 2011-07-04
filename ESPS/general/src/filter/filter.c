/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   Brian Sublett
 * Checked by:   Dave Burton
 * Revised by:   Bill Byrne (converted to FEA_FILT)
 *
 * This program reads data from an ESPS sampled-data file and
 * filters it with a filter of arbitrary length.  The resulting
 * output data is then written to another ESPS data file.  The
 * program may perform interpolation filtering, where the output
 * sampling rate is different from the input sampling rate.  The
 * program calls the routine block_filter to implement regular
 * filtering.  It uses interp_filt to implement interpolation
 * filtering.
 */

static char *sccs_id = "%W% %G% ERL"

#define Version "3.22"
#define Date "5/22/92"

THIS FILE NEEDS TO EDITED BEFORE 5.2 RELEASE TO FIX SKIPREC.  THIS STATEMENT
IS IN HERE TO CAUSE A COMPILE ERROR SO THAT WE DON'T FORGET THAT.

/* 
 * System Includes
 */
# include <stdio.h>
# include <math.h>

/*
 * ESPS Includes
 */
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feasd.h>
# include <esps/feafilt.h>
# include <esps/unix.h>
# include <esps/limits.h>

# define AR_LIM 2048	/* Store this many outputs before
			    writing to a file.  */

# define FILNAMSIZE 40	/* Maximum filter name size.  */

# define SYNTAX \
USAGE("filter [-d data_type] [-f filter name] [-i up/down] [-{pr} range]\n [-x debug_level] [-z] [-F filt_file] [-P parfile] in_file out_file");

/*
 * ESPS Functions
 */

void lrange_switch();
char *get_cmd_line();
void put_sd_recd();
void add_genzfunc();
char *arr_alloc();


int debug_level = 0;

main(argc, argv)
    int	    argc;
    char    *argv[];
{
    FILE  *fpin = stdin, *fpout = stdout; /* in and out file stream ptrs*/
    FILE *fpco;				  /* strream ptr for FILT file*/
    struct header *ih, *oh, *fh = NULL;	 /* file header ptrs*/
    char *in_file = NULL, *out_file = "<stdout>"; /* In and Out file names*/
    int i, j, nsiz, dsiz, nn, up = 1, down = 1, outflag = 0, coutflag = 0, nout, cplxflag=0;
    int co_num = 1;
    struct feafilt *filt_rec;		/* FEAFILT file data structure */
    double  *x, *y, **state;     	/* state vector info */
    double  *cx, *cy, **cstate;     	/* state vector info */
    long    dim[2];			/* dimensions of state, cstate */
    float *fzeros, *fpoles;		/* to write zfunc */
    double *zeros, *poles;		/* holds zero and pole coeff */
    char co_source = 'p';		/* flag for source of coeffs */

    char *filt_file;			/* holds FILT filename */
    char na_name[FILNAMSIZE], da_name[FILNAMSIZE];  /* holds coeffs */
    char nsiz_name[FILNAMSIZE], dsiz_name[FILNAMSIZE]; /* number of coeff */
    short   type, interp = NO, fflag = NO; 
    int	    order;
    long    c, start = 1, nan = LONG_MAX, start_p = 0, end = LONG_MAX, n;

    struct zfunc *pzfunc, tmp_zfunc;
    char *param_file = NULL, *filter_name = "filter";

    struct feasd *irec;                 /* input record */
    struct feasd *orec;                 /* output record */
    double **dptr;                      /* input data pointer */
    double_cplx **cdptr;                /* input data pointer - complex */
    double **optr;                      /* output data pointer */
    double_cplx **coptr;                /* output data pointer - complex */

    int ch_no;                          /* channel number */
    int total_pts = 0;			/* number of output points*/    
    int num_of_files = 0;		/* number of input files*/
    int rflag = 0;			/* flag for command line range*/
    char *range;			/* holds range arguments*/
    int check_common;			/*flag used by read_params*/
    int dflag = 0;			/*flag for data type*/
    char data_type;			/*holds output data type*/
    long big_count = 0;			/* holds # values > maxvalue*/
    double maxvalue = 0;		/* set if -d option causes 
					   conversion to lower type*/
    int Fflag = 0;			/* indicates filt file input*/
    int zflag = 0;                      /* flag to do filter delay adj */
    double fabs();
    long num_channels;                  /* holds # of sampled data channels*/
    double src_sf;                      /* source and output sampling freq.*/
    extern char *optarg;
    extern optind;


/* Check the command line options. */

    while ((c = getopt(argc, argv, "P:f:F:x:d:i:p:r:z")) != EOF)
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
	case 'd':
	    dflag++;
	    data_type = optarg[0];
	    break;
	case 'F':
	    Fflag++;
	    filt_file = optarg;
	    co_source = 'f';
	    TRYOPEN("filter", filt_file, "r", fpco);
	    break;
	case 'x':
	    if (sscanf(optarg, "%d", &debug_level) != 1)
	    {
		Fprintf (stderr, "Error reading -x arg. Exiting.\n");
		exit(1);
	    }
	    break;
	case 'i':
	    if ((nn = sscanf(optarg, "%d/%d", &up, &down)) != 2)
	    {
		Fprintf(stderr, "filter:interpolation format is up/down\n");
		Fprintf(stderr, "filter: nn=%d up=%d down=%d\n", nn, up, down);
		exit(1);
	    }
	    if (up <= 0)
	    {
	    Fprintf(stderr, "filter: up = %d is illegal; must be > 0.\n", up);
	    exit(1);
	    }
	    if (down <= 0)
	    {
	    Fprintf(stderr, "filter: down = %d is illegal; must be > 0.\n", down);
	    exit(1);
	    }
	    interp = YES;
	    break;
	case 'p':
	case 'r':
	    rflag = 1;
	    range = optarg;
	    break;
        case 'z':
	    zflag++;
	    break;
	default:
	    SYNTAX;
	}
    }

/* Change the filtername default if -F was used and -f wasn't. */

    if (fflag == NO && co_source == 'f')
	filter_name = "1";

/* Get the filenames. */

/* 
 * Get the number of filenames on the command line
 */ 
    if ((num_of_files = argc - optind) > 2){
	Fprintf(stderr, "Only two file names allowed\n");
	SYNTAX;
    }
    if (debug_level > 0)
	Fprintf(stderr, "filter: num_of_files = %d\n", num_of_files);

    if (num_of_files == 0){
	Fprintf(stderr, "No output filename specified.\n");
	SYNTAX;
    }


    if (num_of_files == 2) {

/*
 * Both input and output file names specified on command line
 */
	in_file = eopen("filter", argv[optind++], "r", FT_FEA, FEA_SD,
			&ih, &fpin);
	if (debug_level) 
	    Fprintf(stderr, "filter: Input file is %s\n", in_file);


    	out_file = eopen("filter", argv[optind], "w", NONE, NONE,
			    (struct header **)NULL, &fpout);
	if (debug_level) 
	    Fprintf(stderr, "filter: Output file is %s\n", out_file);

	/*
	 * Decide whether to check COMMON or not
	 */
	if (strcmp(in_file, "<stdin>") == 0)
	    check_common = SC_NOCOMMON;
	else
	    check_common = SC_CHECK_FILE;
	
    }

    if ( num_of_files == 1 )
        check_common = SC_CHECK_FILE;

    /* Read the parameter file. */
    (void)read_params(param_file, check_common, in_file);

    if (num_of_files == 1)  {
/*
 * Only output file specified on command line
 */
	if (in_file == NULL){
	    if (symtype("filename") == ST_UNDEF){
	        Fprintf(stderr, 
			"filter: No input file on command line or in Common\n");
		exit(1);
	    }
		
	    in_file = eopen("filter", getsym_s("filename"), "r",
			    FT_FEA, FEA_SD, &ih, &fpin);
	}
	
	if (strcmp( in_file, argv[optind] ) == 0 ) {
	    Fprintf(stderr, "filter: Fatal Error. Identical input and output names\n");
	    exit(1);
	}

	out_file = eopen("filter", argv[optind], "w", NONE, NONE,
			    (struct header **)NULL, &fpout);
	if (debug_level) 
	    Fprintf(stderr, "Output file is %s\n", out_file);
    }
/*
 * Check input types
 */
    if (is_field_complex(ih, "samples")){
	if (debug_level)
	    Fprintf(stderr, "filter: Complex data.\n");
	cplxflag = 1;
    }

    num_channels = get_fea_siz("samples", ih, (short *) NULL, (long **) NULL);
    if (debug_level) 
	Fprintf(stderr, "field samples is dimension %d\n", num_channels);

/* Create the output header. */

    oh = new_header(FT_FEA);

    add_source_file(oh, in_file, ih);
    add_comment(oh, get_cmd_line(argc, argv));

    Strcpy(oh->common.prog, "filter");
    Strcpy(oh->common.vers, Version);
    Strcpy(oh->common.progdate, Date);

  /*
   * Find output data type and set maximum value
   */
    if (dflag){
	switch (data_type)
	{
	case 'B':
	case 'b': 
	    maxvalue = CHAR_MAX;
	    if (cplxflag) 
		type = BYTE_CPLX;
	    else
		type = BYTE;
	    break;
	case 'S':
	case 's': 
	    maxvalue = SHRT_MAX;
	    if (cplxflag)
		type = SHORT_CPLX;
	    else
		type = SHORT;
	    break;
	case 'L':
	case 'l': 
	    maxvalue = LONG_MAX;
	    if (cplxflag)
		type = LONG_CPLX;
	    else
		type = LONG;
	    break;
	case 'F':
	case 'f': 
	    maxvalue = FLT_MAX;
	    if (cplxflag)
		type = FLOAT_CPLX;
	    else
		type = FLOAT;
	    break;
	case 'D':
	case 'd': 
	    maxvalue = DBL_MAX; 
	    if (cplxflag)
		type = DOUBLE_CPLX;
	    else
		type = DOUBLE;
	    break;
	default: 
	    Fprintf(stderr, 
		    "filter: -d type \"%c\" unknown - exiting.\n",
		    data_type);
	    exit(1);
	}
    }
    else{
        type = get_sd_type(ih);
	switch (type)
	{
	case CHAR: 
	case BYTE:
	case BYTE_CPLX:
	    maxvalue = CHAR_MAX;
	    break;
	case SHORT: 
	case SHORT_CPLX: 
	    maxvalue = SHRT_MAX;
	    break;
	case LONG: 
	case LONG_CPLX: 
	    maxvalue = LONG_MAX;
	    break;
	case FLOAT: 
	case FLOAT_CPLX: 
	    maxvalue = FLT_MAX;
	    break;
	case DOUBLE: 
	case DOUBLE_CPLX: 
	    maxvalue = DBL_MAX; 
	    break;
	default: 
	    Fprintf(stderr, "filter: input data type unknown\n");
	    exit(1);
	}
    }

    (void) add_genhd_d("max_value", &maxvalue, 1, oh);

/*
 * Initialize output fea_sd header
 */
    {
	double start_time = 0.;    /*this value is overwritten later*/

	src_sf = get_genhd_val("record_freq", ih, (double)0.);
	if (src_sf == 0){
	    Fprintf(stderr,
	        "filter: Input file has invalid \"record_freq\" - exiting.\n");
	    exit(1);
	}

        if (init_feasd_hd(oh, type, (int) num_channels, &start_time, 0, src_sf) 
	    != 0)
	{
	    Fprintf(stderr,
		"filter: Couldn't allocate fea_sd header - exiting\n");
	    exit(1);
	}

    }
       
/*
 * Get range 
 */

    if (!rflag){
	if (symtype("start") == ST_UNDEF){
	    if (debug_level > 0)
	    Fprintf(stderr, 
		"filter: No starting point in params or Common\n");
	}
        else
	    start = (long) getsym_i("start");
        start = start - 1;  /* Make start an offset. */
        if (start < 0) start = 0;
	if (symtype("nan") == ST_UNDEF){
	    if (debug_level > 0)
	    Fprintf(stderr, 
	        "filter: No NAN value specified in params file or Common\n");
	}
	else
            nan = (long) getsym_i("nan");
    }
    else{
	lrange_switch(range, &start, &end, 1);
	if (start != 0) start -= 1;  /* start becomes an offset. */
        nan = end - start;
    }

    if (nan == 0) nan = LONG_MAX;
    if (nan < 0)
    {
	Fprintf(stderr, "filter: nan = %ld less than zero\n", nan);
	exit(1);
    }
    if (debug_level >= 1)
        Fprintf(stderr, "filter: start = %ld nan = %ld\n", start+1, nan);

/* Get the filter coefficients. */

    if (co_source == 'p')
    {
	Sprintf(nsiz_name, "%s_nsiz", filter_name);
	Sprintf(dsiz_name, "%s_dsiz", filter_name);
	if (symtype(nsiz_name) == ST_INT)
            nsiz = getsym_i(nsiz_name);
	else{
	    Fprintf(stderr, 
	       "filter: No numerator coefficient file specified or \nNumber of numerator coefficients not an integer.\n");
	    exit(1);
	}
	if (symtype(dsiz_name) == ST_INT)
            dsiz = getsym_i(dsiz_name);
	else{
	    Fprintf(stderr, 
	      "filter: Number of denominator coefficients not an integer\nor No denominator coefficient file specified.\n");
	    exit(1);
	}
	if (debug_level)
        {
	    Fprintf(stderr, "filter: filtername is %s, nsiz_name is %s, dsiz_name is %s\n", filter_name, nsiz_name, dsiz_name);
	    Fprintf(stderr, "filter: from parameter file: nsiz = %d dsiz = %d\n", nsiz, dsiz);
        }

        zeros = (double*) calloc((unsigned)nsiz, sizeof(double));
	spsassert(zeros != NULL, 
	    "Memory for zeros could not be allocated\n");
        fzeros = (float*) calloc((unsigned)nsiz, sizeof(float));
	spsassert(fzeros != NULL, 
	    "Memory for fzeros could not be allocated\n");
        poles = (double*) calloc((unsigned)dsiz, sizeof(double));
	spsassert(poles != NULL, 
	    "Memory for poles could not be allocated\n");
        fpoles = (float*) calloc((unsigned)dsiz, sizeof(float));
	spsassert(fpoles != NULL, 
	    "Memory for fpoles could not be allocated\n");

        Sprintf(na_name, "%s_num", filter_name);
	if (debug_level)
	    Fprintf(stderr, "filter: na_name is %s\n", na_name);
        if ((nn=getsym_da(na_name, zeros, nsiz)) != nsiz)
	{
            Fprintf(stderr, "Wrong number of numerator coefficients in %s\n", param_file);
            exit(1);
	}

	if (dsiz > 0)
	{
            Sprintf(da_name, "%s_den", filter_name);
	    if (debug_level)
	        Fprintf(stderr, "filter: da_name is %s\n", da_name);
            if ((nn=getsym_da(da_name, poles, dsiz)) != dsiz)
	    {
	        Fprintf(stderr, "Wrong number of denominator coefficients in %s\n", param_file);
	        exit(1);
	    }
            for (i=0; i<dsiz; i++)
	    {
	        fpoles[i] = poles[i];
	    }
	}

        for (i=0; i<nsiz; i++)
	{
	    fzeros[i] = zeros[i];
	}
/* Store the coefficients in a zfunc. */

        pzfunc = new_zfunc(nsiz, dsiz, fzeros, fpoles);
    }
    else
    {
	fh = read_header(fpco);
	add_source_file(oh, filt_file, fh);
	co_num = atoi(filter_name);
	if (co_num > fh->common.ndrec)
	{
	    Fprintf(stderr, "filter: Not %d records in %s\n", co_num, filt_file);
	    exit(1);
	}
	if (co_num > 1) skiprec(fpco, (long)(co_num - 1), size_rec(fh));
	filt_rec = allo_feafilt_rec(fh);
	(void)get_feafilt_rec(filt_rec, fh, fpco);
	/*
	pzfunc = &(filt_rec->filt_func);
	*/
	tmp_zfunc = feafilt_to_zfunc( filt_rec );
	pzfunc = &tmp_zfunc;

	nsiz = pzfunc->nsiz;
	dsiz = pzfunc->dsiz;
    }

    if (debug_level >= 1)
        Fprintf(stderr, "start-1=%ld nan=%ld\n", start, nan);


/* Calculate the filter order for the state vector. */

    order = MAX(nsiz, dsiz);

/* Allocate and initialize the arrays. */

    n = 1 + AR_LIM/num_channels;
    if (n < order) n = order;

    x = (double*) calloc((unsigned)n, sizeof(double));
    spsassert(x != NULL, "Couldn't allocate memory for x");
    /* Allocate an extra space for odd outputs during interpolation filtering. */
    y = (double*) calloc((unsigned)(n*up/down + 1), sizeof(double));
    spsassert(y != NULL, "Couldn't allocate memory for y");

    dim[0] = num_channels;
    dim[1] = up*order;
    state = (double **) arr_alloc(2, dim, DOUBLE, 0);
    spsassert(state != NULL, "Couldn't allocate memory for state");

    if (cplxflag) { 
	cx = (double*) calloc((unsigned)n, sizeof(double));
	spsassert(cx != NULL, "Couldn't allocate memory for x");
	/* Allocate an extra space for odd outputs during interpolation filtering. */
	cy = (double*) calloc((unsigned)(n*up/down + 1), sizeof(double));
	spsassert(cy != NULL, "Couldn't allocate memory for y");

	cstate = (double **) arr_alloc(2, dim, DOUBLE, 0);
	spsassert(cstate != NULL, "Couldn't allocate memory for state");
    }

    /* allocate input record */
    if (!cplxflag) { 
	irec = allo_feasd_recs( ih, DOUBLE, (long)n, (char *)NULL, YES);
	spsassert( irec != NULL, "can't allocate input record.");
	dptr = (double **) irec->ptrs;
	spsassert( dptr != NULL, "can't allocate input pointers.");
    } else {
	irec = allo_feasd_recs( ih, DOUBLE_CPLX, (long)n, (char *)NULL, YES);
	spsassert( irec != NULL, "can't allocate input record.");
	cdptr = (double_cplx **) irec->ptrs;
	spsassert( cdptr != NULL, "can't allocate input pointers.");
    }

    /* allocate output record */
    if (!cplxflag) { 
	orec = allo_feasd_recs( oh, DOUBLE, (long)(n*up/down+1), (char *)NULL, YES);
	spsassert( orec != NULL, "can't allocate output record.");
	optr = (double **) orec->ptrs;
	spsassert( optr != NULL, "can't allocate output pointers.");
    } else {
	orec = allo_feasd_recs( oh, DOUBLE_CPLX, (long)(n*up/down+1), (char *)NULL, YES);
	spsassert( orec != NULL, "can't allocate output record.");
	coptr = (double_cplx **) orec->ptrs;
	spsassert( coptr != NULL, "can't allocate output pointers.");
    }

/* Initialize the data arrays from the data if it is an FIR filter. */

    if (dsiz == 0) {
        start_p = start - (long) order;
        if (start_p > 0)
            skiprec(fpin, start_p, size_rec(ih));

        for (i=0; i<order; i++){
	    if (start_p + i < 0) {
		for (ch_no=0; ch_no<num_channels; ch_no++) {
		    state[ch_no][i*up] = 0.0;
		    if (cplxflag)
			cstate[ch_no][i*up] = 0.0;
		    for (j=1; j<up; j++) {
			state[ch_no][i+j] = 0.0;
			if (cplxflag) 
			    cstate[ch_no][i+j] = 0.0;
		    }
		}
	    }
	    else {
		get_feasd_recs( irec, (long)0, (long)1, ih, fpin);
		for (ch_no=0; ch_no<num_channels; ch_no++) {
		    if (!cplxflag)
			state[ch_no][i*up] = dptr[0][ch_no];
		    else {
			state[ch_no][i*up] = cdptr[0][ch_no].real;
			cstate[ch_no][i*up] = cdptr[0][ch_no].imag;
		    }
		    for (j=1; j<up; j++) {
			state[ch_no][i+j] = 0.0;
			if (cplxflag)
			    cstate[ch_no][i+j] = 0.0;
		    }
		}
	    }
	}
    }

/* For IIR filter, initialize the arrays to zero. */

    else{
	skiprec(fpin, start, size_rec(ih));

	for (i=0; i<up*order; i++) {
	    for (ch_no=0; ch_no<num_channels; ch_no++) {
		state[ch_no][i] = 0;
		if (cplxflag)
	    		state[ch_no][i] = 0;
	    }
	}
    }

/* Store the order and coefficients in the output header. */

    (void)add_genzfunc("filter", pzfunc , oh);

    if (debug_level >= 1)
    {
        Fprintf(stderr, "There are %d and %d coefficients.\n", nsiz, dsiz);
        for (i=0; i<nsiz; i++)
	{
            Fprintf(stderr, "zeros[%d] = %lf\n", i, pzfunc->zeros[i]);
	}
        for (i=0; i<dsiz; i++)
	{
            Fprintf(stderr, "poles[%d] = %lf\n", i, pzfunc->poles[i]);
	}
    }

/*
 * Update start_time generic 
 */

    {
	long start_adj = start + 1;
	float sf_factor = (float)down/(float)up;
	
	if (zflag && co_source != 'p'){

	    /* Get filter delay value and adjust it based on
	       sample frequency at which filter is applied.
	       It is necessary to scale the delay value relative to the 
	       input sample rate for use by update_waves_gen()*/
	       
	    /* The start value is relative to the input sample rate,
	       so it needs no adjustment. */


	    start_adj = start + 1 -
	       (long)get_genhd_val("delay_samples", fh, (double)0)/(float)up;
	    update_waves_gen(ih, oh, (float)start_adj, sf_factor);

	  }
    }

/*
 * add src_sf if sample rate conversion
 */
    if ((src_sf*up/down) != src_sf){
	(void)add_genhd_d("src_sf", &src_sf, 1, oh);
    }

/* Print the new output header to the output file. */

    write_header(oh, fpout);
    if (debug_level >= 1) 
    {
	Fprintf(stderr, "Header written to %s\n", out_file);
	Fprintf(stderr, "filter: Prior to main filter loop.\n");
    }
    if (debug_level >= 2) 
    {
	for (i=0; i<up*order; i++)
	    for (ch_no=0; ch_no<num_channels; ch_no++)
		Fprintf(stderr, "state[%d][%d]=%lf\n",
			ch_no, i, state[ch_no][i]);
    }

/*
 * Check generic header item samp_freq for consistency with data sf
 */
    if (Fflag > 0)
    {
	double samp_freq;
	samp_freq = get_genhd_val("samp_freq", fh, (double) -1);

	if (samp_freq != src_sf && samp_freq != -1)
	    Fprintf(stderr, 
	"filter: WARNING - Data sampled at %lf samples/sec;\n filter designed for %lf samples/second data.\n",
	    src_sf, samp_freq);
    }

/* Read in and process the data. */

    while ((n = get_feasd_recs(irec, (long)0, (long)n, ih, fpin)) != 0){
	for (ch_no=0; ch_no<num_channels; ch_no++) {
	    for ( j=0; j<n; j++ )
		if ( !cplxflag ) 
		    x[j] = dptr[j][ch_no];
		else {
		    x[j] = cdptr[j][ch_no].real;
		    cx[j] = cdptr[j][ch_no].imag;
		}

	    if (interp) {
		(void)interp_filt((int)n, x, y, pzfunc, state[ch_no],
				  up, down, &outflag, &nout);
		if (cplxflag) 
		    (void)interp_filt((int)n, cx, cy, pzfunc, cstate[ch_no],
				      up, down, &coutflag, &nout);
	    } else {
		(void)block_filter((int)n, x, y, pzfunc, state[ch_no]);
		if (cplxflag)
		    (void)block_filter((int)n, cx, cy, pzfunc, cstate[ch_no]);
		nout = n;
	    }

	    /*
	     * check returned values against maximum size
	     */
	    for (i = 0; i < nout; i++) {
		if (fabs((double)y[i]) > maxvalue) {
		    y[i] = maxvalue;
		    big_count++;
		    if (big_count == 1)
			Fprintf(stderr, "filter: WARNING - output values too big for output type\n");
		}
		if ( cplxflag ) 
		    if (fabs((double)cy[i]) > maxvalue) {
			cy[i] = maxvalue;
			big_count++;
			if (big_count == 1)
			    Fprintf(stderr, "filter: WARNING - output values too big for output type\n");
		    }
		if ( !cplxflag ) 
		    optr[i][ch_no] = y[i]; 
		else { 
		    coptr[i][ch_no].real = y[i];
		    coptr[i][ch_no].imag = cy[i];
		}
	    }
	}
	if (n < nan) {
	    nan -= nout;
	    total_pts += nout;
	    (void)put_feasd_recs(orec, (long)0, (long)nout, oh, fpout);
	} else {
	    nout = nan * up / down;
	    total_pts += nout;
	    (void)put_feasd_recs(orec, (long)0, (long)nout, oh, fpout);
	    break;
	}
    }

/*
 * Write Common, if appropriate
 */
    if (strcmp(out_file, "<stdout>") != 0){
	if (putsym_s("filename", out_file) != 0)
	    Fprintf(stderr, "filter: Could not write ESPS Common\n");
	(void)putsym_s("prog", "filter");
	(void)putsym_i("start", 1);
	(void)putsym_i("nan", total_pts);
    }

/*
 * Summarize total number of output values exceeding maxvalue
 */
    if (big_count > 0)
	Fprintf(stderr, 
	    "filter: %ld values exceed %lf.\n", big_count, maxvalue);

/* Close all of the files. */

    (void)fclose(fpin);
    (void)fclose(fpout);
    exit(0);
    return(0);	/* lint pleasing*/
}
