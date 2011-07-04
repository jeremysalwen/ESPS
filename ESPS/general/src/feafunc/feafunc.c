/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
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
 * Written by:  John Shore
 * Checked by:
 * Revised by:  Rodney Johnson
 *
 * Apply optional function, gain factor, additive constant, and type change
 * to FEA fields
 *
 */

static char *sccs_id = "@(#)feafunc.c	1.11	8/31/95	ESI/ERL";

/*
  Based on window.c.
 */

#define VERSION "1.11"
#define DATE "8/31/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/func.h>

#include <signal.h>

#define REQUIRE(test, text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("feafunc [-d add_constant] [-f input_field [-f output_field]]\n\t[-g gain_factor] [-r range] [-t output_type] [-x debug_level]\n\t[-F function] [-P params] input.fea output.fea") ;

#define ABS(x) ((x) < 0 ? -(x) : (x))


extern char *type_codes[];
extern char *function_types[];


char	    *get_cmd_line();
void	    lrange_switch();
char	    *arr_alloc();
char	    *arr_func();
char	    *type_convert();


static int  fpe_handler();		/* fpe handler routine */
static void clip_warn();		/* used with type_convert to print
					   warning in case of clipping */
static int  numeric();			/* tests whether type is numeric */


char	    *ProgName = "feafunc";
char	    *Version = VERSION;
char	    *Date = DATE;
char	    sbuf[256];			/* to hold comment */
int	    debug_level = 0;		/* debug level, global for library*/

long	    num_read;			/* number of records skipped or read */
long	    clip_warn_flag = 0;		/* last record for which clip_warn
					   was called */

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
    int             tflag = 0;  /* -t option specified? */
    int             gflag = 0;  /* -g option specified? */
    int             dflag = 0;  /* -d option specified? */
    double          gain_re;	/* real gain factor */
    double	    gain_im;	/* imaginary part of gain factor */
    double          add_re;	/* real add constant */
    double	    add_im;	/* imaginary part of additive constant */
    char	    *inf_name;	/* field name for input field */
    int		    inf_type;	/* data type of input field */
    char	    *type_code;	/* string specifying output field type */
    int		    outf_type;	/* data type of output field */
    char	    *outf_name;	/* field name for output field */
    int		    warn_if_repl = YES;	/* print warning if output
					   field exists in input file? */
    char            *inf_names[2];  /* need arrays for print_fea_recf */
    char            *outf_names[2]; /* need arrays for print_fea_recf */
    char	    **fld_names;    /* fields to be copied unchanged */
    char	    *function_type; /* name of function to apply to data */
    short           func;	/* function code */
    int		    complex;	/* do complex computations? */
    int		    func_flag = 0;  /* flag for window option */
    int		    rflag = NO;	/* -r option specified? */
    char	    *rrange;	/* arguments of -r option */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				   (0 means all up to end of file) */

    char	    *param_name =  NULL;
				/* parameter file name */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */
    char	    *inf_ptr;	/* pointer (untyped) to 
				   field in input record */
    char	    *outf_ptr;	/* pointer (untyped) to 
				   field in output record*/
    long	    size;	/* size of input data field */
    short	    rank;	/* number of dimensions of input field */
    long	    *dimens;	/* dimensions of input field */
    double	    *data;	/* input real data as double */
    double_cplx	    *cdata;	/* input complex data as double */
    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    int		    i;		/* loop index */


    /*
     * Parse command-line options.
     */

    while ((ch = getopt(argc, argv, "d:f:g:r:t:x:P:F:")) != EOF)
	switch (ch)
	{
	case 'd':
	    dflag++;
	    add_re = add_im = 0.0;
	    sscanf(optarg, "%lf,%lf", &add_re, &add_im);
	    break;
	case 'f':
	    switch (fflag)
	    {
	    case 0:
		inf_name = optarg;
		fflag++;
		break;
	    case 1:
		outf_name = optarg;
		fflag++;
		break;
	    default:		
		Fprintf(stderr, 
			"%s: -f option may be specified at most twice.\n",
			ProgName);
		exit(1);
	    }
	    break;
	case 'g':
	    gflag++;
	    gain_re = gain_im = 0.0;
	    sscanf(optarg, "%lf,%lf", &gain_re, &gain_im);
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 't':
	    tflag++;
	    type_code = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'F':
	    function_type = optarg;
	    func_flag++;
	    break;
	case 'P':
	    param_name = optarg;
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

    if (ifile != stdin)
	REQUIRE(strcmp(iname, argv[optind]),
		"output file same as input");

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

    if (!func_flag)
	function_type =
	    (symtype("function_type") != ST_UNDEF)
		? getsym_s("function_type")
		    : "none";

    func = lin_search(function_types, function_type);
    if (func == -1)
    {
	Fprintf(stderr, "%s: invalid function type %s\n", 
		ProgName, function_type);
	exit(1);
    }

    if (debug_level)
	Fprintf(stderr, "%s: function_type = %s, function code = %d\n",
		ProgName, function_type, func);

    if (fflag < 1)
	inf_name =
	    (symtype("input_field") != ST_UNDEF)
		? getsym_s("input_field")
		    : "samples";

    if (fflag < 2) 
        outf_name = 
	    (symtype("output_field") != ST_UNDEF) 
		? getsym_s("output_field")
		    : NULL;

    if ((outf_name == NULL)
	|| (strcmp(outf_name, "input_name_[function]") == 0)) {
	outf_name =
	    malloc((unsigned)(strlen(inf_name) + strlen(function_type) + 2));
	REQUIRE(outf_name, "can't allocate space for output field name");
	(void) strcpy(outf_name, inf_name);
	(void) strcat(outf_name, "_");
	(void) strcat(outf_name, function_type);
    }
    else
    if (strcmp(outf_name, "-") == 0)
    {
	outf_name = inf_name;
	warn_if_repl = NO;
    }

    inf_names[0] = inf_name;
    outf_names[0] = outf_name;
    inf_names[1] = outf_names[1] = NULL;

    if (!gflag)
    {
        gain_re = 
	    (symtype("gain_real") != ST_UNDEF)
		? getsym_d("gain_real")
		    : 1.0;
        gain_im = 
	    (symtype("gain_imag") != ST_UNDEF)
		? getsym_d("gain_imag")
		    : 0.0;
    }

    if (!dflag)
    {
        add_re = 
	    (symtype("add_real") != ST_UNDEF)
		? getsym_d("add_real")
		    : 0.0;
        add_im = 
	    (symtype("add_imag") != ST_UNDEF)
		? getsym_d("add_imag")
		    : 0.0;
    }

    if (debug_level) {
	Fprintf(stderr,
		"input_field: \"%s\".  output_field: \"%s\".\n",
		inf_name, outf_name);
	Fprintf(stderr,
		"gain: [%g, %g]. add: [%g, %g]\n",
		gain_re, gain_im, add_re, add_im);
    }

    inf_type = get_fea_type(inf_name, ihd);
    if (inf_type == UNDEF) {
	Fprintf(stderr, "%s: input_field %s not in input file\n",
		ProgName, inf_name);
	exit(1);
    }

    if (!tflag)
	type_code =
	    (symtype("output_type") != ST_UNDEF)
		? getsym_s("output_type")
		    : NULL;

    if (type_code == NULL || strcmp(type_code ,"input type") == 0)
	outf_type = inf_type;
    else
    {
	outf_type = lin_search(type_codes, type_code);
	if (outf_type == CHAR) outf_type = BYTE;
	if (!numeric(outf_type))
	{
	    Fprintf(stderr, "%s: invalid output type %s\n",
		    ProgName, type_code);
	    exit(1);
	}
    }

    complex = NO;

    if (is_type_complex(inf_type))
    {
	complex = YES;
        if (debug_level) 
	    Fprintf(stderr, "%s: input field is complex\n", ProgName);
    }

    /* We have just assumed that complex arithmetic is required
       if the input is complex.  Consider special treatment for
       functions ABS, ARG, IM, RE, that map complex to real. */

    if (is_type_complex(outf_type))
    {
	complex = YES;
	if (debug_level)
	    Fprintf(stderr, "%s: output field is complex\n", ProgName);
    }

    if (add_im != 0.0)
    {
	complex = YES;
	if (debug_level)
	    Fprintf(stderr, "%s: additive constant is complex\n", ProgName);
    }

    if (gain_im != 0.0)
    {
	complex = YES;
	if (debug_level)
	    Fprintf(stderr, "%s: gain factor is complex\n", ProgName);
    }

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

    if (debug_level)
	Fprintf(stderr, "start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    irec = allo_fea_rec(ihd);

    REQUIRE(irec != NULL, "can't allocate memory for input record");

    inf_ptr = get_fea_ptr(irec, inf_name, ihd);

    REQUIRE(inf_ptr, "can't locate field in input record");

    size = get_fea_siz(inf_name, ihd, &rank, &dimens);

    REQUIRE(size != 0,
	    "bad field definition in input file");

    /*
     * Create and write output-file header
     */

    ohd = copy_header(ihd);

    if (get_fea_type(outf_name, ihd) == UNDEF)
	fld_names = NULL;	/* Copy all input fields unchanged. */
    else
    {
	char	**out_names;

	if (warn_if_repl)
	    Fprintf(stderr,
		    "%s: output field %s replaces existing field in input.\n",
		    ProgName, inf_name);

	if (debug_level)
	    fprintf(stderr, "%s: removing old field %s.\n",
		    ProgName, outf_name);

	(void) del_fea_fld(outf_name, ohd);

	/* Make list of fields to copy unchanged: all but output field. */

	fld_names =
	    (char **) malloc((1 + ohd->hd.fea->field_count) * sizeof(char *));
	REQUIRE(fld_names, "can't allocate memory for list of field names");
	out_names = ohd->hd.fea->names;
	for (i = 0; fld_names[i] = out_names[i]; i++) { }
    }

    if (debug_level > 1)
    {
	fprintf(stderr, "%s: fields to copy unchanged from input:", ProgName);
	if (!fld_names)
	    fprintf(stderr, " ALL.\n");
	else
	{
	    for (i = 0; fld_names[i]; i++)
		fprintf(stderr, "\t%s", fld_names[i]);
	    fprintf(stderr, "\n");
	}
    }

    if (debug_level)
	fprintf(stderr, "%s: adding new field %s.\n", ProgName, outf_name);

    REQUIRE(add_fea_fld(outf_name, size, rank,
			dimens, outf_type, (char **) NULL, ohd) != -1,
	    "can't create output field in output file header");

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));
    sprintf(sbuf, "function field %s added by feafunc\n", outf_name);
    add_comment(ohd, sbuf);
    (void) copy_genhd(ohd, ihd, (char *) NULL);

    *add_genhd_e("function_type", (short *) NULL, 1, function_types, ohd) = func;
    *add_genhd_d("gain_real", (double *) NULL, 1, ohd) = gain_re;
    *add_genhd_d("gain_imag", (double *) NULL, 1, ohd) = gain_im;
    *add_genhd_d("add_real", (double *) NULL, 1, ohd) = add_re;
    *add_genhd_d("add_imag", (double *) NULL, 1, ohd) = add_im;
    (void) add_genhd_c("input_field", inf_name, 0, ohd);
    (void) add_genhd_c("output_field", outf_name, 0, ohd);
    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec;

    update_waves_gen(ihd, ohd, (float) start_rec, (float) 1.0);

    if (debug_level)
	Fprintf(stderr, "Writing output header to file.\n");

    write_header(ohd, ofile);

    /*
     * Allocate records and set up pointers
     */

    orec = allo_fea_rec(ohd);
    REQUIRE(orec != NULL, "can't allocate memory for output record");

    outf_ptr = get_fea_ptr(orec, outf_name, ohd);

    REQUIRE(outf_ptr, "can't locate data field in output record");

    if (complex) {
	cdata = (double_cplx *) arr_alloc(1, &size, DOUBLE_CPLX, 0);
	REQUIRE(cdata,
		"can't allocate space for array of data as DOUBLE_CPLX");
    }
    else {
	data = (double *) arr_alloc(1, &size, DOUBLE, 0);
	REQUIRE(data,
		"can't allocate space for array of data as DOUBLE");
    }
      
    /*
     * MAIN READ-WRITE LOOP
     */

    num_read = start_rec - 1;

    fea_skiprec(ifile, num_read, ihd);

    (void) signal(SIGFPE, fpe_handler);

    /* This can treat single samples as general FEA records.  Consider
       special handling of FEA_SD files with no extraneous fields,
       using get_feasd_recs instead of get_fea_rec. */

    while (num_read++ < end_rec && get_fea_rec(irec, ihd, ifile) != EOF)
    {
	if (debug_level > 2)
	    Fprintf(stderr, "Record number %ld read.\n", num_read);

	/* Copy fields from input to output. */

	copy_fea_rec(irec, ihd, orec, ohd, fld_names, (short **) NULL);

	/* Convert input_field to double or double_cplx, apply functional form,
	   and store in output_field. */

	if (complex) 
	{
	    (void) arr_func(func, size, inf_ptr, inf_type,
			    (char *) cdata, DOUBLE_CPLX);

	    /* Consider checking for gain 1, add 0 */

	    for (i = 0; i < size; i++)
	    {
		double  x, y;

		x = cdata[i].real;
		y = cdata[i].imag;
		cdata[i].real = gain_re*x - gain_im*y + add_re;
		cdata[i].imag = gain_re*y + gain_im*x + add_im;
	    }

	    /* Could save waste motion when outf_type is DOUBLE_CPLX:
	       set cdata = cast of outf_ptr instead of malloc'ing
	       separate array. */

	    (void) type_convert(size, (char *) cdata, DOUBLE_CPLX,
				outf_ptr, outf_type, clip_warn);
	}
	else
	{
	    (void) arr_func(func, size, inf_ptr, inf_type,
			    (char *) data, DOUBLE);

	    for (i = 0; i < size; i++)
		data[i] = gain_re*data[i] + add_re;

	    (void) type_convert(size, (char *) data, DOUBLE,
				outf_ptr, outf_type, clip_warn);
	}

	if (debug_level > 3) 
	    print_fea_recf(irec, ihd, stderr, inf_names);

    	if (debug_level > 3) 
	    print_fea_recf(orec, ohd, stderr, outf_names);

	put_fea_rec(orec, ohd, ofile);
    }

    if (num_read <= end_rec && num_recs != 0)
	Fprintf(stderr, "feafunc: only %ld records read.\n",
		num_read - start_rec);

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

/* Handle floating-point exception. */

static int
fpe_handler() {
  Fprintf(stderr, "%s: Floating point exception in record %ld.\n", 
	  ProgName, num_read);
  exit(1);
}

/* Print warning once for each record in which clipping occurs. */

static void
clip_warn(i, val)
    long	i;
    double_cplx	val;
{
    if (num_read == clip_warn_flag) return;
    fprintf(stderr, "%s: clipping occurred in record %ld.\n",
	    ProgName, num_read);
    clip_warn_flag = num_read;
}

/* There are separate static copies of "numeric" in atoarray.c, feasdsupp.c,
   and typeconver.c.  Make public. */

/* Is type numeric? */

static int
numeric(type)
    int	    type;
{
    switch (type)
    {
    case BYTE:
    case SHORT:
    case LONG:
    case FLOAT:
    case DOUBLE:
    case BYTE_CPLX:
    case SHORT_CPLX:
    case LONG_CPLX:
    case FLOAT_CPLX:
    case DOUBLE_CPLX:
	return YES;
	break;
    default:
	return NO;
	break;
    }

    /*NOTREACHED*/
}
