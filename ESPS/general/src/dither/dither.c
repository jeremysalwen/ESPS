
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
 * Written by: Joe Buck
 * Checked by:
 * Revised by:
 *
 * Brief description: dithers spectrograms 
 *
 */

static char *sccs_id = "@(#)dither.c	1.5	8/31/95	ESI/ERL";
int debug_level = 0;

/*----------------------------------------------------------------------+
|									|
|  Program: dither.c							|
|									|
|  This program dithers spectrograms in a FEA_SPEC or other FEA file	|
|  for display on a single-plane black-and-white display.  The output	|
|  contains copies of the input records with a new field containing	|
|  the dithered image.							|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/

#define VERSION "1.5"
#define DATE "8/31/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX USAGE("dither [-x debug_level] [-P params] input.fea output.fea")


char	*get_cmd_line();
char	*arr_alloc();
double	pow();

char	*ProgName = "dither";
char	*Version = VERSION;
char	*Date = DATE;
char	sbuf[256];		/* to hold comment */

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

    char	    *spec_name;	/* field name for input spectral data */
    char	    *dspec_name;/* field name for dithered spectral data */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				    (0 means all up to end of file) */
    long	    num_read;	/* number of records actually read */

    char	    *param_name = "params";
				/* parameter file name */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */
    char	    *data;	/* input data */
    long	    size;	/* number of input data points per record */

    FILE    	    *tempfile;	/* file for temporary copy of input records */
    double  	    max;	/* maximum input value */

    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    char    	    *ddata;	/* output data */

    long	    bufdim[2];	/* dimensions of buf */
    double	    **buf;	/* data buffer for dithering algorithm */
				/* error diffusion coefficients */
    double	    alpha =	7.0/16.0,
    	    	    beta =	3.0/16.0,
    	    	    gamma =	5.0/16.0,
		    delta =	1.0/16.0;
    double	    err;	/* error term */

    int		    i, j;	/* loop indices */
    int		    spec_type;

/*
 * Parse command-line options.
 */

    while ((ch = getopt(argc, argv, "x:P:")) != EOF)
        switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	default:
	    SYNTAX
	    ;
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
	;
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
	;
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

    spec_name =
	(symtype("spec_field_name") != ST_UNDEF)
	? getsym_s("spec_field_name")
	: "re_spec_val";


    dspec_name =
	(symtype("dspec_field_name") != ST_UNDEF)
	? getsym_s("dspec_field_name")
	: "dith_spec_val";

    if (debug_level)
	Fprintf(stderr,
	    "spec_field_name: \"%s\".  dspec_field_name: \"%s\".\n",
	    spec_name, dspec_name);

    REQUIRE(get_fea_type(dspec_name, ihd) == UNDEF,
		"dspec_field_name already exists in input file");

    start_rec =
	(symtype("start") != ST_UNDEF)
	? getsym_i("start")
	: 1;
    num_recs =
	(symtype("nan") != ST_UNDEF)
	? getsym_i("nan")
	: 0;
    end_rec =
	(num_recs != 0)
	? start_rec - 1 + num_recs
	: LONG_MAX;

    if (debug_level)
	Fprintf(stderr, "start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    irec = allo_fea_rec(ihd);
    REQUIRE(irec, "can't allocate memory for input record");

    data =  get_fea_ptr(irec, spec_name, ihd);
    REQUIRE(data, "can't find spectral-data field in input record");

    spec_type = get_fea_type(spec_name, ihd);
    REQUIRE(spec_type == BYTE || spec_type == FLOAT, 
	"bad type of spectral-data field in input file; it must be BYTE or FLOAT");

    size = get_fea_siz(spec_name, ihd, (short *) NULL, (long **) NULL);
    REQUIRE(size != -1,
	"bad spectral-data field definition in input file");

    if (spec_type == BYTE)
    	for (i = 0; i < size; i++) data[i] = 0;
    else
    	for (i = 0; i < size; i++) ((float *)data)[i] = 0;

/*
 * Create and write output-file header
 */

    ohd = copy_header(ihd);

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));
    (void) sprintf(sbuf, "dithered spectral data field %s added by dither\n",
		   dspec_name);
    add_comment (ohd, sbuf);

    REQUIRE(add_fea_fld(dspec_name, size, 1,
			(long *) NULL, BYTE, (char **) NULL, ohd) != -1,
	    "can't create dspec_field_name field in output file header" );

    if (debug_level)
	Fprintf(stderr, "Writing output header to file.\n");
    write_header(ohd, ofile);

/*
 * Allocate records and set up pointers
 */

    orec = allo_fea_rec(ohd);
    REQUIRE(orec, "can't allocate memory for output record");

    ddata = (char *) get_fea_ptr(orec, dspec_name, ohd);
    REQUIRE(ddata, "can't locate dithered-data field in output record");

    for (i = 0; i < size; i++) ddata[i] = 0.0;

    bufdim[0] = 2;  bufdim[1] = size + 2;
    buf = (double **) arr_alloc(2, bufdim, DOUBLE, 0);
    for (i = 0; i < 2; i++)
	for (j = 0; j < size + 2; j++)
	    buf[i][j] = 0.0;

/*
 * Copy records to tmp file and find max value.
 */

    tempfile = tmpfile();
    max = -FLT_MAX;
    num_read = start_rec - 1,
    fea_skiprec(ifile, num_read, ihd);

    for ( ;
	 num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	 num_read++
	)
    {
	if (debug_level > 2)
	    Fprintf(stderr, "Record number %ld read.\n", num_read + 1);

	if (spec_type == BYTE) {
	  for (j = 0; j < size; j++)
	    if (data[j] > max) max = data[j];
	}
	else {
	  for (j = 0; j < size; j++)
	    if (((float *)data)[j] > max) max = ((float *)data)[j];
	}

	put_fea_rec(irec, ihd, tempfile);
    }

    rewind(tempfile);

/* 
 * Main read-write loop.
 */

    while (get_fea_rec(irec, ihd, tempfile) != EOF)
    {
	double	hilim, lolim, scale;

	hilim = max - 7.0;
	lolim = hilim - 40.0;
	scale = 1.0/(hilim - lolim);

	copy_fea_rec(irec, ihd, orec, ohd,
		     (char **) NULL, (short **) NULL);

	for (j = 0; j < size; j++)
	{
	    double  val;

	    if (spec_type == BYTE)
	    	val = data[j];
	    else
	    	val = ((float *)data)[j];

	    val = (val < lolim) ? 0.0
	    	: (val > hilim) ? 1.0
		: scale*(val - lolim);

	    buf[0][j+1] = buf[1][j+1] + val*val;
	}
	buf[1][1] = 0.0;
	for (j = 0; j < size; j++)
	{
	    ddata[j] = (buf[0][j+1] < 0.5) ? 0 : 1;
	    err = buf[0][j+1] - ddata[j];
	    buf[0][j+2]	+= err*alpha;
	    buf[1][j+2]	 = err*delta; /* Yes, '=' here, not '+='. */
	    buf[1][j+1]	+= err*gamma;
	    buf[1][j]	+= err*beta;
	}

	put_fea_rec(orec, ohd, ofile);
    }

    if (num_read < end_rec && num_recs != 0)
	Fprintf(stderr, "Only %ld records read.\n", num_read);

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
