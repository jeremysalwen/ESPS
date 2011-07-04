/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988, 1990 Entropic Speech, Inc.  All rights reserved."
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: window.c							|
|									|
|  This program windows the data in sampled-data records in a FEA	|
|  file.  The output is contains copies of the input records with a	|
|  field containing the windowed data.					|
|									|
|  John Shore, Entropic Speech, Inc.			         	|
|  based on power.c by Rodney Johnson					|
|  User-defined window added by David Burton				|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)window.c	1.10	8/31/95	ESI";
#endif
#define VERSION "1.10"
#define DATE "8/31/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/window.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX USAGE("window [-f sd_field [-f winsd_field]] [-l window_length]\n [-r range] [-x debug_level] [-w window_type] [-P params]\n input.fea output.fea") ;

#define WT_PREFIX "WT_"


void		pr_farray();
char		*get_cmd_line();
void		lrange_switch();
char            *type_convert();
char            *atoarray();
char	*ProgName = "window";
char	*Version = VERSION;
char	*Date = DATE;
int	debug_level = 0;	/* debug level, global for library*/

/*array of field names to copy over from input to output (these are for 
 *copying over the segment labelling*/

char   *seg_fields[] = {"source_file", "segment_start", "segment_length", NULL};


/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int	    optind;	/* for use of getopt() */
    extern char	    *optarg;	/* for use of getopt() */
    int		    ch;		/* command-line option letter */

    int		    fflag = 0;	/* -f option specified 0, 1, or 2 times? */
    char	    *sd_name;	/* field name for input sampled data */
    int		    sd_type;	/* data type of sampled-data field */
    char	    *wsd_name;	/* field name for windowed sampled-data */
    char	    *window_type = NULL;	
				/* name of type of window to apply to data */
    char	    *pref_w_type;	
				/* window type name with added prefix */
    int		    win;    	/* window type code */
    int		    wflag = 0;	/* flag for window option*/
    int		    rflag = NO;	/* -r option specified? */
    int		    lflag = NO; /* -l option specified? */
    char	    *rrange;	/* arguments of -r option */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				    (0 means all up to end of file) */
    long	    num_read;	/* number of records actually read */

    char	    *param_name = NULL;
				/* parameter file name */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    FILE   *instrm = stdin;     /* for ASCII data*/
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */
    char	    *sd_ptr;	/* pointer (untyped) to sampled-data
				    field in input record */
    long	    *frame_len;	/* pointer to frame size
				    in input header or record */
    long	    size;	/* size of input sampled-data field */
    long	    win_len;   	/* size of window to apply */
    float	    *data;	/* input data as float */
    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    float	    *wsd_ptr;	/* pointer to windowed 
				    sampled data field in output record */

    int		    i;		/* loop index */
    int		    seglenwarn = 0;	
				/* warning flag for segment lengths less
				    than window length*/
    short           tagged=0;	/* flag to indicate input is tagged */
    short           seglab=0;	/* to indicate segmenta labelled input */
    short           file_ind;	/* index for location of source_file field */
    extern char
	*window_types[];	/* standard window type names */

    double          *udefined;  /*pointer for user defined window coeffs*/

/*
 * Parse command-line options.
 */

    while ((ch = getopt(argc, argv, "f:r:x:P:l:w:")) != EOF)
        switch (ch)
	{
	case 'f':
	    switch (fflag)
    	    {
	    case 0:
		sd_name	= optarg;
		fflag++;
		break;
	    case 1:
		wsd_name = optarg;
		fflag++;
		break;
	    default:		
		Fprintf(stderr, 
		    "%s: -f option may be specified at most twice.\n",
		    ProgName);
		exit(1);
	    }
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'l':
	    win_len = atoi(optarg);
	    lflag = YES;
	    break;
	case 'w':
	    window_type = optarg;
	    wflag++;
	    break;
	default:
	    SYNTAX
	    break;
	}

/*
 * Process file names and open files.
 */

    if (argc - optind > 2)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }

    iname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, NONE, &ihd, &ifile);

    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);

    if (debug_level)
	Fprintf(stderr, "Input file: %s\nOutput file: %s\n",
	    iname, oname);

/*
 * Get parameter values.
 */

    if (ifile != stdin)
	(void) read_params(param_name, SC_CHECK_FILE, iname);
    else
        (void) read_params(param_name, SC_NOCOMMON, iname);

    if (fflag < 1)
	sd_name =
	    (symtype("sd_field_name") != ST_UNDEF)
	    ? getsym_s("sd_field_name")
	    : "sd";

    if (fflag < 2)
	wsd_name =
	    (symtype("wind_field_name") != ST_UNDEF)
	    ? getsym_s("wind_field_name")
	    : "wind_sd";

    if (debug_level)
	Fprintf(stderr,
	    "sd_field_name: \"%s\".  wind_field_name: \"%s\".\n",
	    sd_name, wsd_name);

    sd_type = get_fea_type(sd_name, ihd);
    REQUIRE(sd_type != UNDEF, "sampled-data field not in input file");

    REQUIRE(!is_field_complex(ihd, sd_name), 
	    "sorry, can't deal with complex data"); 

    if (rflag)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	num_recs =
	    (end_rec != LONG_MAX)
	    ? end_rec - start_rec + 1
	    : 0;
    }
    else
    {
	start_rec =
	    (symtype("start") != ST_UNDEF)
	    ? getsym_i("start")
	    : 1;
	num_recs =
	    (symtype("nan") != ST_UNDEF)
	    ? getsym_i("nan")
	    : 0;
	end_rec = (num_recs != 0)
	    ? start_rec - 1 + num_recs
	    : LONG_MAX;
    }

    if (!lflag) 
	win_len =
	    (symtype("window_len") != ST_UNDEF)
	    ? getsym_i("window_len")
	    : 0;
	
    if (debug_level)
	Fprintf(stderr, "start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
	    start_rec, end_rec, num_recs);

    if (debug_level)
	Fprintf(stderr, "window_len: %ld.\n", win_len);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    if (!wflag)
	window_type =
	    (symtype("window_type") != ST_UNDEF)
	    ? getsym_s("window_type")
	    : "HAMMING";

    pref_w_type = 
	malloc((unsigned)(strlen(WT_PREFIX) + strlen(window_type) + 1));
    REQUIRE(pref_w_type, "can't allocate space for window type name");
    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);

    win = lin_search(window_types, pref_w_type);

    if(win == -1){
      Fprintf(stderr, "window: Window type is not recognized.\n\t Assume type is name of ASCII file (or stdin) containing weight coefficients\n");
      /*
       * open file with coefficients
       */
      if (strcmp (window_type, "-") == 0)
        window_type = "<stdin>";
      else 
        TRYOPEN(argv[0], window_type, "r", instrm);

      if(strcmp(window_type, iname) == 0){
	Fprintf(stderr, "window: input window coefficients and signal data cannot both come from standard input - exiting.\n");
	exit(1);
      }

      udefined = (double *) atoarray(instrm, DOUBLE, &win_len, (double *)NULL);
      win = WT_ARB;
     }

    if (debug_level)
	Fprintf(stderr, "%s: window_type = %s, win = %d\n",
	    ProgName, window_type, win);

    irec = allo_fea_rec(ihd);
    REQUIRE(irec != NULL, "can't allocate memory for input record");

    sd_ptr = get_fea_ptr(irec, sd_name, ihd);

    REQUIRE(sd_ptr, "can't locate sampled-data field in input record");

    size = get_fea_siz(sd_name, ihd, (short *) NULL, (long **) NULL);
    REQUIRE(size != -1,
	"bad sampled-data field definition in input file");
    REQUIRE(win_len <= size, "window_len larger than sd field size");

    if (ihd->hd.fea->segment_labeled)
    {
	frame_len = (long *) get_fea_ptr(irec, "segment_length", ihd);
	REQUIRE(frame_len, "no segment_length field in input file header");

	if (debug_level)
	    Fprintf(stderr, "Input file segment_labeled.\n");

	file_ind = lin_search2(ihd->hd.fea->names,"source_file");

	set_seg_lab(ohd, ihd->hd.fea->enums[file_ind]);

	seglab = 1;

    }
    else
    {
	frame_len = &size;

	if (debug_level)
	    Fprintf(stderr,
		"Input file not segment_labeled.  Frame length: %ld.\n",
		*frame_len);
    }

    /* 0 input for window_len (default) means size of sd field*/

    if (win_len == 0) 
	win_len = size;
/*
 * Create and write output-file header
 */

    ohd = new_header(FT_FEA);

    if (ihd->common.tag == YES) {
	ohd->common.tag = YES;
	ohd->variable.refer = ihd->variable.refer;
	tagged = 1; 
	ohd->variable.refhd = ihd->variable.refhd;
    }

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));

    *add_genhd_e( "window_type", (short *) NULL, 1, window_types, ohd) = win;

    REQUIRE( add_fea_fld(wsd_name, size, (short) 1, (long *) NULL,
			    FLOAT, (char **) NULL, ohd) != -1,
		"can't create wind_field_name field in output file header" );

    *add_genhd_l( "window_len", (long *) NULL, 1, ohd) = win_len;

    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec;

    (void) copy_genhd(ohd, ihd, "src_sf");

    update_waves_gen(ihd, ohd, (float) start_rec, 1.0);

    if (debug_level)
	Fprintf(stderr, "Writing output header to file.\n");

    write_header(ohd, ofile);

/*
 * Allocate records and set up pointers
 */

    orec = allo_fea_rec(ohd);
    REQUIRE(orec != NULL, "can't allocate memory for output record");

    wsd_ptr = (float *) get_fea_ptr(orec, wsd_name, ohd);
    REQUIRE(wsd_ptr, "can't locate wind_samp_data field in output record");

    data = calloc_f((unsigned) size);
    REQUIRE(data,
	"can't allocate space for array of data as floats");

    for (i = 0; i < size; i++) 
	wsd_ptr[i] = 0.0;
/*
 * Main read-write loop
 */

    num_read = start_rec - 1,

    fea_skiprec(ifile, num_read, ihd);

    for (   ;
	    num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	    num_read++
	)
    {
	if (debug_level > 2)
	    Fprintf(stderr, "Record number %ld read.\n", num_read + 1);

	if (tagged)
	  orec->tag = irec->tag;
	else if (seglab)
	  copy_fea_rec(irec, ihd, orec, ohd, seg_fields, (short **) NULL);

	(void) type_convert(win_len, (char *) sd_ptr, sd_type, 
			    (char *) data, FLOAT, (void (*)())NULL);

    	if (debug_level > 3) pr_farray("sampled data input", 
	    win_len, data);

	if (*frame_len < win_len) 
	    seglenwarn++;

	/* window it, with results going directly to output record*/
	if(win != WT_ARB){
	   (void) window(win_len, data, wsd_ptr, win, (double *) NULL);
	   (void)add_genhd_c("window_data_in", window_type, 1, ohd);
	 }
	else
	   (void)window(win_len, data, wsd_ptr, win, udefined);

    	if (debug_level > 3) pr_farray("windowed sampled data", 
	    win_len, wsd_ptr);

	put_fea_rec(orec, ohd, ofile);

    }

    if (seglenwarn) Fprintf(stderr, 
	"%s: WARNING - some segment_length values less than window size\n",
	    ProgName);

    if (num_read < end_rec && num_recs != 0)
	Fprintf(stderr, "window: only %ld records read.\n",
		num_read - start_rec + 1);

/*
 * Write common
 */

    if (ofile != stdout)
    {
	REQUIRE(putsym_i("start", (int) start_rec) != -1,
		"can't write \"start\" to Common");
	REQUIRE(putsym_i("nan", (int) num_recs) != -1,
		"can't write \"nan\" to Common");
	REQUIRE(putsym_s("prog", ProgName) != -1,
		"can't write \"prog\" to Common");
	REQUIRE(putsym_s("filename", iname) != -1,
		"can't write \"filename\" to Common");
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

