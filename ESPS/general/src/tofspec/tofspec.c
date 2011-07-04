/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1988-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore, Entropic Speech, Inc.
 * Checked by:
 * Revised by:
 *
 * This program takes the data from a given FEA field in an input file
 * and represents it as a "power spectrum" in an output FEA_SPEC
 * file.  The main purpose of this program is to allow waves+ users
 * to display arbitrary FEA data as spectrograms.
 *
 */

static char *sccs_id = "@(#)tofspec.c	1.12	9/5/95	ESI/ERL";

#define VERSION "1.12"
#define DATE "9/5/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feaspec.h>

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", ProgName, (text)); \
  exit(1);}
#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define SYNTAX \
USAGE("tofspec [-d] [-f fea_field] [-i input_range] [-o output_range]\n\t[-r record_range] [-s sf] [-v freqs] [-x debug_level] [-F freq_format]\n\t[-P params] [-R] [-S] input output");

/* default upper and lower limits; only use 128-13 values 
   since top 13 levels of color map are used for things other than
   data values in waves+*/

#define HIGH_OUT 50
#define LOW_OUT -64.0

double          log10();
double          getsym_d();
char		*get_cmd_line();
void		lrange_switch();
void		copy_fea_fields();
void		copy_fea_rec(), frange_switch();
char            *type_convert();

static float	*flist_switch();


char	*ProgName = "tofspec";
char	*Version = VERSION;
char	*Date = DATE;
int	debug_level = 0;	/* debug level, global for library*/

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
    char	    *field_name; /* name of input field to transform */
    int		    field_type;	/* data type of input field */
    int		    dflag = NO;	/* -d option specified? */
    int             fflag = NO; /* -f option specified? */
    int 	    iflag = NO;	/* -l option specified? */
    int 	    oflag = NO;	/* -o option specified? */
    int		    rflag = NO;	/* -r option specified? */
    int             sflag = NO; /* -s option specified? */
    int             vflag = NO; /* -v option specified? */
    int             Fflag = NO; /* -F option specified? */
    int 	    Rflag = NO;	/* -R option specified? */
    int 	    Sflag = NO;	/* -S option specified? */
    int  determine_limits = NO; /* true if display limits from file */
    char	    *rec_range;	/* arguments of -r option */
    char            *in_range;	/* data range for full output range */
    char            *out_range;	/* output range */
    char	    *fmt_name;	/* string naming freq_format */
    int		    freq_format; /* FEA_SPEC frequenc format:
				    SYM_EDGE, ARB_FIXED, etc. */
    char	    *freq_list;	/* string giving frequeny values */
    float	    *freqs = NULL; /* frequency values */
    long	    num_freqs;	/* number of frequency values */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				   (0 means all up to end of file) */
    long	    num_read;	/* number of records actually read */

    char	    *param_name = NULL; /* parameter file name */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */
    char	    *f_ptr;	/* pointer (untyped) to data
				   field in input record */
    long	    size;	/* size of input data field */
    short           rank = 0;	/* rank of input data field */
    float	    *data;	/* input data as doubles */
    float           dtemp;	/* temp location for one input value */
    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct feaspec  *orec;	/* output record */
    double          low_in = FLT_MAX; /*input range limits*/
    double          high_in = -FLT_MAX; 
    double          log_low_in, log_high_in; /*log input limits*/
    double	    high_out = HIGH_OUT; /*output range limits*/
    double          low_out = LOW_OUT;
    char            *high_in_s = NULL; /*strings for param file input*/
    char            *low_in_s = NULL; 

    double  datamin, datamax;	/* max and min data values */
    int		    j,k;	/* loop indices */
    double 	    sf;		/* sampling frequency */
    float   	    scale;	/* scaling constant */

    /*
     * Parse command-line options.
     */

    while ((ch = getopt(argc, argv, "df:i:o:r:s:v:x:F:P:RS")) != EOF)
        switch (ch)
	{
	case 'd':
	    dflag = YES;
	    break;
	case 'f':
	    field_name = optarg;
	    fflag++;
	    break;
	case 'i':
	    iflag = YES;
	    in_range = optarg;
	    break;
	case 'o':
	    oflag = YES;
	    out_range = optarg;
	    break;
	case 'r':
	    rflag = YES;
	    rec_range = optarg;
	    break;
	case 's':
	    sflag = YES;
	    sf = atof(optarg);
	    break;
	case 'v':
	    vflag = YES;
	    freq_list = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'F':
	    Fflag = YES;
	    fmt_name = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'R':
	    Rflag = YES;
	    break;
	case 'S':
	    Sflag = YES;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    if (debug_level && dflag) 
	Fprintf(stderr, "tofspec: -d option specified, will take logs\n");

    if (debug_level && iflag) 
	Fprintf(stderr, "tofspec:  limits determined from -i option\n");

    if (iflag && Sflag) 
	Fprintf(stderr, "tofspec: -i option ignored since -S option given\n");

    /*
     * Process file names and open files.
     */

    if (argc - optind > 2)
    {
	Fprintf(stderr,
		"%s: too many file names specified.\n", ProgName);
	SYNTAX;
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr,
		"%s: too few file names specified.\n", ProgName);
	SYNTAX;
    }

    iname = eopen(ProgName,
		  argv[optind++], "r", FT_FEA, NONE, &ihd, &ifile);

    oname = eopen(ProgName,
		  argv[optind++], "w", NONE, NONE, &ohd, &ofile);

    if (debug_level)
	Fprintf(stderr, "tofspec: Input file: %s\nOutput file: %s\n",
		iname, oname);

    /*
     * Get parameter values.
     */

    if (ifile != stdin)
	(void) read_params(param_name, SC_CHECK_FILE, iname);
    else
	(void) read_params(param_name, SC_NOCOMMON, iname);

    /* input range */

    if (iflag)
	frange_switch(in_range, &low_in, &high_in);

    else if (!Sflag) {
	if (symtype("low_input") != ST_UNDEF) {
	    low_in_s = getsym_s("low_input");
	    if (strcmp(low_in_s, "determine from file") != 0)
		low_in = atof(low_in_s);
	}
	if (symtype("high_input") != ST_UNDEF) {
	    high_in_s = getsym_s("high_input");
	    if (strcmp(high_in_s, "determine from file") != 0)
		high_in = atof(high_in_s);
	}
    }

    determine_limits = (low_in == FLT_MAX || high_in == -FLT_MAX);

    if (debug_level) 
	Fprintf(stderr, "tofspec: determine_limits = %d\n", determine_limits);

    if (determine_limits && (strcmp(iname, "<stdin>") == 0)) {
	Fprintf(stderr, 
		"tofspec: can't use stdin when input range determined from file\n");
	exit(1);
    }

    /* output limits */

    if (oflag)
	frange_switch(out_range, &low_out, &high_out);
    else
    {
	if (symtype("low_output") != ST_UNDEF) 
	    low_out = getsym_d("low_output");
	if (symtype("high_output") != ST_UNDEF) 
	    high_out = getsym_d("high_output");
    }

    if (low_out < -64.0) {
	Fprintf(stderr, 
		"tofspec: WARNING - output data type is BYTE, so low_out limited to -64.0\n");
	low_out = -64.0;
    }

    if (high_out > 63.0) {
	Fprintf(stderr, 
		"tofspec: WARNING - output data type is BYTE, so high_out limited to 63.0\n");
	high_out = 63.0;
    }

    if (debug_level)
	Fprintf(stderr, "tofspec: output range is %g, %g.\n", 
		low_out, high_out);    

    /* field name */

    if (!fflag)
	field_name =
	    (symtype("field_name") != ST_UNDEF)
		? getsym_s("field_name")
		    : "sd";

    if (debug_level)
	Fprintf(stderr,
		"  field_name is \"%s\".\n", field_name);

    /* range of input records */

    if (rflag)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rec_range, &start_rec, &end_rec, 0);
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

    if (debug_level)
	Fprintf(stderr, "start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    /*
     * Get info on input field
     */

    field_type = get_fea_type(field_name, ihd);

    REQUIRE(field_type != UNDEF, "specified field not in input file");

    size = get_fea_siz(field_name, ihd, &rank, (long **) NULL);

    if (rank != 1) 
	Fprintf(stderr, 
		"tofspec: WARNING, field %s not a vector.\n", field_name);

    REQUIRE(size != 0,
	    "size == 0, bad data field definition in input file");

    /* sf */
    /* If -s is used, we have a forced setting for sf; otherwise
       we fake up as element numbers. */

    if (!sflag)
	sf =
	    (symtype("sf") != ST_UNDEF)
		? getsym_d("sf")
		    : 0;	/* means fake up element numbers as frequencies*/

    if (sf == 0) 
	sf = 2*(size - 1);	/* fake up element numbers as frequencies*/

    /* freq_format */

    if (!Fflag)
    {
	fmt_name =
	    (symtype("freq_format") != ST_UNDEF)
		? getsym_s("freq_format")
		    : "SYM_EDGE";
	REQUIRE(fmt_name, "Parameter \"freq_format\" not STRING");
    }

    switch (freq_format = lin_search(spfmt_names, fmt_name))
    {
    case SPFMT_ARB_FIXED:
	if (vflag)
	{
	    freqs = flist_switch(freq_list, &num_freqs);
	    REQUIRE(num_freqs == size,
		    "wrong number of specified frequencies");
	}
	else
	{
	    REQUIRE(symtype("freqs") == ST_FARRAY,
		    "Paramter \"freqs\" undefined or not float array");
	    num_freqs = symsize("freqs");
	    REQUIRE(num_freqs == size,
		    "wrong number of specified frequencies");
	    freqs = (float *) malloc(sizeof(float) * size);
	    REQUIRE(freqs, "couldn't allocate storage for freqs array");
	    (void) getsym_fa("freqs", freqs, (int) num_freqs);
	}

	break;
    case SPFMT_SYM_EDGE:
	break;
    case SPFMT_ASYM_EDGE:
    case SPFMT_SYM_CTR:
    case SPFMT_ASYM_CTR:
    case SPFMT_ARB_VAR:
	ERROR("freq format not yet supported");
	break;
    default:
	ERROR("freq format not recognized");
	break;
    }

    /* possible loop to determine data limits*/

    irec = allo_fea_rec(ihd);
    f_ptr = get_fea_ptr(irec, field_name, ihd);

    REQUIRE(f_ptr, "NULL pointer returned from get_fea_ptr");

    data = calloc_f((unsigned) size);

    REQUIRE(data,
	    "can't allocate space for array of data as doubles");

    if (determine_limits) {

	if (debug_level) 
	    Fprintf(stderr, "tofspec: computing display limits from data\n");
      
	datamin = FLT_MAX;
	datamax = -FLT_MAX;
      
	num_read = start_rec - 1,
	fea_skiprec(ifile, num_read, ihd);
      
	for ( ;
	     num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	     num_read++
	     )
	{
	    if (debug_level > 2)
		Fprintf(stderr, 
			"Record number %ld read.\n", num_read + 1);
	  
	    (void) type_convert(size, f_ptr, field_type, 
				(char *) data, FLOAT, (void (*)()) NULL);

	    for (j = 0; j < size; j++) {
		if (data[j] < datamin) datamin = data[j];
		if (data[j] > datamax) datamax = data[j];
	    }
	}
      
	if (num_read < end_rec && num_recs != 0)
	    Fprintf(stderr, "tofspec: Only %ld records read.\n", 
		    num_read - start_rec + 1);

	if (low_in == FLT_MAX) low_in = datamin;
	if (high_in == -FLT_MAX) high_in = datamax;

	rewind(ifile);
	ihd = read_header(ifile);
    }

    if (debug_level)
	Fprintf(stderr, "tofspec: input range is %g, %g.\n", 
		low_in, high_in);

    if (dflag) {
	if (low_in <= 0) {
	    Fprintf(stderr, 
		    "tofspec: can't use -d option with negative or zero data\n");
	    exit(1);
	}
	else {
	    log_low_in = log10(low_in);
	    log_high_in = log10(high_in);
	    if (debug_level) 
		Fprintf(stderr, 
			"tofspec: log limits on data values: %g, %g.\n", 
			log_low_in, log_high_in);
	}
    }

    /*set scale values for data */

    if (dflag) 
	scale = (high_out - low_out) / (log_high_in - log_low_in);
    else 
	scale = (high_out - low_out) / (high_in - low_in);

    if (debug_level) Fprintf(stderr, "tofspec: scale = %g\n", scale);

    /*
     * Create and write output-file header
     */

    ohd = new_header(FT_FEA);

    if (init_feaspec_hd(ohd, NO, freq_format, SPTYP_DB,
			YES, size, SPFRM_NONE, freqs, sf, (long) 0,
			BYTE) != 0) {
	Fprintf(stderr, "tofspec: Error filling FEA_SPEC header\n");
	exit(1);
    }

    ohd->common.tag = ihd->common.tag;
    if (ihd->common.tag
	&& !copy_genhd(ohd, ihd, "src_sf"))
    {
	if (genhd_type("sf", (int *) NULL, ihd) == ST_UNDEF)
	    Fprintf(stderr,
		    "tofspec: no \"src_sf\" or \"sf\" in tagged file.\n");
	else
	    *add_genhd_f("src_sf", (float *) NULL, 1, ohd) =
		get_genhd_val("sf", ihd, 1.0);
    }

    ohd->variable.refer = ihd->variable.refer;
    ohd->variable.refhd = ihd->variable.refhd;

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));

    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec;
    *add_genhd_f("low_input", (float *) NULL, 1, ohd) = low_in;
    *add_genhd_f("high_input", (float *) NULL, 1, ohd) = high_in;
    *add_genhd_f("low_output", (float *) NULL, 1, ohd) = low_out;
    *add_genhd_f("high_output", (float *) NULL, 1, ohd) = high_out;

    if (!Sflag) 
	*add_genhd_f("scale_factor", (float *) NULL, 1, ohd) = scale;

    if (dflag) {
	*add_genhd_f("log_low_input", (float *) NULL, 1, ohd) = log_low_in;
	*add_genhd_f("log_high_input", (float *) NULL, 1, ohd) = log_high_in;
    }

    update_waves_gen(ihd, ohd, (float)start_rec, 1.0);

    if (debug_level)
	Fprintf(stderr, "tofspec: Writing output header to file.\n");

    write_header(ohd, ofile);

    orec = allo_feaspec_rec(ohd, FLOAT);

    /*
     * Main read-write loop
     */

    /* now read/write loop for putting out FEA_SPEC file*/

    num_read = start_rec - 1,
    fea_skiprec(ifile, num_read, ihd);

    for ( ;
	 num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	 num_read++
	 )
    {
	if (debug_level > 1)
	    Fprintf(stderr, "Record number %ld read.\n", num_read + 1);

	(void) type_convert(size, f_ptr, field_type, 
			    (char *) data, FLOAT, (void (*)()) NULL);

	for (j = 0; j < size; j++) {

	    if (Rflag)		/*reverse elements*/
		k = size - 1 - j;
	    else		/*element order unchanged*/
		k = j;

	    if (Sflag) {	/*no scaling*/
		if (dflag) 
		    dtemp = log10(data[k]);
		else
		    dtemp = data[k];
		if (dtemp < low_out)
		    orec->re_spec_val[j] = low_out;
		else if (dtemp > high_out)
		    orec->re_spec_val[j] = high_out;
		else 
		    orec->re_spec_val[j] = dtemp;
	    }
	    else {		/*scale the data to fit*/
		if (data[k] <= low_in)
		    orec->re_spec_val[j] = low_out;
		else if (data[k] >= high_in)
		    orec->re_spec_val[j] = high_out;
		else {
		    if (dflag)
			orec->re_spec_val[j] = 
			    low_out + scale * (log10(data[k]) - log_low_in);
		    else
			orec->re_spec_val[j] = 
			    low_out + scale * (data[k] - low_in);
		}
	    }
	}
	
	if (ohd->common.tag == YES)
	    *orec->tag = irec->tag;

	(void) put_feaspec_rec(orec, ohd, ofile);
    }

    if (num_read < end_rec && num_recs != 0)
	Fprintf(stderr, "tofspec: Only %ld records read.\n", 
		num_read - start_rec + 1);
 
    /*
     * Write common
     */

    if (strcmp(oname, "<stdout>") != 0 && strcmp(iname, "<stdin>") != 0) {
	(void) putsym_s("filename", iname);
	(void) putsym_s("prog", ProgName);
	(void) putsym_i("start", (int) start_rec);
	(void) putsym_i("nan", (int) num_recs);
    }

    exit(0);
    /*NOTREACHED*/
}


#define INIT_SIZE   200
#define DELIM       ", "

char	*savestring();

static float *
flist_switch(text, len)
    char    *text;
    long    *len;
{
    char    *list, *token;
    long    alloc_len;
    float   *vals;
    long    num;

    spsassert(text, "flist_switch: NULL text");
    list = savestring(text);
    alloc_len = INIT_SIZE;
    vals = (float *) malloc(sizeof(float) * alloc_len);
    if (!vals)
	return NULL;
    for (token = strtok(list, DELIM), num = 0;
	 token;
	 token = strtok((char *) NULL, DELIM), num++)
    {
	if (num == alloc_len)
	{
	    alloc_len += alloc_len/2;
	    vals =
		(float *) realloc((char *) vals, sizeof(float) * alloc_len);
	    if (!vals)
		return NULL;
	}

	vals[num] = atof(token);
    }

    free((char *) list);
    if (len)
	*len = num;
    return (float *) realloc((char *) vals, sizeof(float) * num);
}
