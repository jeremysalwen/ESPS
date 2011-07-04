/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Element-by-element binary operations on fields of FEA files.
 *
 */

static char *sccs_id = "@(#)feaop.c	1.3	8/31/95	ERL";


/* INCLUDE FILES */

#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/op.h>


/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "8/31/95"
#define EC_SCCS_VERSION "1.3"

#define DEF_OUT_FLD "input1_[op]_input2"
#define DEF_OUT_TYP "default"


/* LOCAL MACROS */

#define SYNTAX \
USAGE("feaop [-f in_field [-f in_field2 [-f out_field]]] [-g gain_factor]\n\t[-r range [-r range2]] [-t output_type] [-x debug_level] [-z] [-I]\n\t[-O operation] [-P param_file] [-R] file1.in file2.in file3.out")

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", ProgName, (text)); \
  exit(1);}
#define REQUIRE(test, text) {if (!(test)) ERROR(text)}


/* SYSTEM FUNCTIONS AND VARIABLES */

int	getopt();
int	optind;
char	*optarg;


/* ESPS FUNCTIONS AND VARIABLES */

void	    lrange_switch();
char	    *arr_alloc();
short	    cover_type();
char	    *get_cmd_line();
int	    is_type_numeric();
char	    *arr_op();

int	    debug_level = 0;

/* LOCAL FUNCTION DECLARATIONS */

int	    getdata1();

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
static char *ProgName = "feaop";
static char *Version = EC_SCCS_VERSION;
static char *Date = EC_SCCS_DATE;

static char sbuf[256];			/* to hold comment */

static FILE		*ifile1;	/* input stream */
static struct header	*ihd1;		/* input file header */
static struct fea_data	*irec1;		/* input record */
static long		offset;		/* starting position in ifile1 */
static int		Rflag = NO;	/* -R option specified? */


/* MAIN PROGRAM */

int
main(argc, argv)
    int		argc;
    char    	**argv;
{
    int	    	ch;			/* command-line option letter */

    int	    	fflag = 0;		/* -f option used how many times? */
    char    	*in_fld1, *in_fld2;	/* names of input fields */
	    
    char    	*out_fld;		/* name of output field */
    int	    	warn_if_repl = YES;	/* print warning if output
					   field exists in input file? */
    char    	**fld_names;		/* fields to be copied unchanged */

    int	    	gflag = NO;		/* -g option specified? */
    char    	*gain_arg;		/* argument of -g option */
    double_cplx	gain;			/* gain factor */
    int		scale;			/* Boolean: (gain != 1)? */
    char	*gain_ptr;		/* pointer to replicated gain */
    double	*dbl_ptr;		/* gain_ptr as (double *) */
    double_cplx	*cplx_ptr;		/* gain_ptr as (double_cplx *) */
    int	    	rflag = 0;		/* -r option used how many times? */
    char	*rrange1, *rrange2;	/* arguments of -r option */
	    
    long    	start_rec1, start_rec2;	/* starting record numbers in inputs */
    int    	start_recs[2];		/* start_rec1 & start_rec2 as array */
    long    	start_rec3;		/* starting record number in output */
    long    	end_rec1, end_rec2;	/* ending record numbers in inputs */
    long    	end_rec3;		/* ending record number in output */
    long    	num_recs1, num_recs2;	/* number of records (inputs) */
    int    	num_recs[2];		/* num_recs1 & num_recs2 as array */
    long    	num_recs3;		/* number of records (output) */
    long	num_out;		/* number of records written */

    int	    	inf_type1, inf_type2;	/* data types of input fields */
    int		gain_type;		/* data type of gain factor */
    int		scal_type;		/* data type of scaled data */

    int	    	tflag = NO;		/* -t option specified? */
    char    	*type_name;		/* name of output field type */
    int	    	outf_type;		/* data type of output field */

    int	    	zflag = NO;		/* -z option specified? */

    int	    	Iflag = NO;		/* -I option specified? */

    int	    	Oflag = NO;		/* -O option specified? */
    char    	*oper_name;		/* name of operation to perform */
    int	    	oper;			/* numeric code of operation */

    char    	*param_name = NULL;	/* parameter file name */

    char	    *iname1, *iname2;	/* input file names */
    /* ifile1, ihd1, irec1 global for access by getdata1. */
    FILE	    *ifile2;		/* input stream */
    struct header   *ihd2;		/* input file header */
    struct fea_data *irec2;		/* input record */
    char	    *in1_ptr, *in2_ptr;	/* pointers to input data */
    char	    *scal_ptr;		/* pointer to scaled input data */
    long	    size1, size2;	/* sizes of input data fields */
    short	    rank1, rank2;	/* ranks of input fields */
    long	    *dim1, *dim2;	/* dimensions of input fields */
    char	    *oname;		/* output file name */
    FILE	    *ofile;		/* output stream */
    struct header   *ohd;		/* output file header */
    struct fea_data *orec;		/* output record */
    char	    *out_ptr;		/* pointer to output data */
    long	    get1, get2;		/* get-data return flags */
    double	    rec_fr1, rec_fr2;	/* record_freq header item values */
    int		    i;			/* loop index */


    /*
     * Parse command-line options.
     */

    while ((ch = getopt(argc, argv, "f:g:r:t:x:zIO:P:R")) != EOF)
	switch (ch)
	{
	case 'f':
	    switch (fflag)
	    {
	    case 0:
		in_fld1 = optarg;
		fflag++;
		break;
	    case 1:
		in_fld2 = optarg;
		fflag++;
		break;
	    case 2:
		out_fld = optarg;
		fflag++;
		break;
	    default:
		ERROR("-f option may be specified at most 3 times");
		break;
	    }
	    break;
	case 'g':
	    gflag = YES;
	    gain_arg = optarg;
	    break;
	case 'r':
	    switch (rflag)
	    {
	    case 0:
		rrange1 = optarg;
		rflag++;
		break;
	    case 1:
		rrange2 = optarg;
		rflag++;
		break;
	    default:
		ERROR("-r option may be specified at most 2 times");
		break;
	    }
	    break;
	case 't':
	    tflag = YES;
	    type_name = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'z':
	    zflag = YES;
	    break;
	case 'I':
	    Iflag = YES;
	    break;
	case 'O':
	    Oflag = YES;
	    oper_name = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'R':
	    Rflag = YES;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    /*
     * Process file names and open input files.
     */

    if (argc - optind > 3)
    {
	Fprintf(stderr, "%s: too many file names specified.\n", ProgName);
	SYNTAX;
    }
    if (argc - optind < 3)
    {
	Fprintf(stderr, "%s: too few file names specified.\n", ProgName);
	SYNTAX;
    }

    iname1 = argv[optind++];
    iname2 = argv[optind++];
    oname = argv[optind++];

    REQUIRE(strcmp(iname1, "-") || strcmp(iname2, "-"),
	    "Input files cannot both be <stdin>");

    REQUIRE(!Rflag || strcmp(iname1, "-"),
	    "Can't recycle <stdin>");

    REQUIRE(!strcmp(oname, "-")
	    || (strcmp(oname, iname1) && strcmp(oname, iname2)),
	    "Output file cannot be same as input file");

    iname1 = eopen(ProgName, iname1, "r", FT_FEA, NONE, &ihd1, &ifile1);
    iname2 = eopen(ProgName, iname2, "r", FT_FEA, NONE, &ihd2, &ifile2);

    if (debug_level)
	Fprintf(stderr, "%s: input files: %s, %s.\n", ProgName, iname1, iname2);

    /*
     * Process parameters and options.
     */

    (void) read_params(param_name, SC_NOCOMMON, (char *) NULL);

    /* operation code */

    if (!Oflag)
    {
	oper_name =
	    (symtype("operation") != ST_UNDEF)
		? getsym_s("operation")
		    : "ADD";
	REQUIRE(oper_name, "Parameter \"operation\" not STRING");
    }
    if (debug_level)
	Fprintf(stderr, "%s: operation \"%s\"\n", ProgName, oper_name);
    oper = lin_search(operation_names, oper_name);
    REQUIRE(oper != -1, "Invalid operation name");
    if (debug_level)
	Fprintf(stderr, "%s: operation code %d\n", ProgName, oper);

    /* field names */

    switch (fflag)
    {
    case 0:
	in_fld1 =
	    (symtype("in_field1") != ST_UNDEF)
		? getsym_s("in_field1")
		    : "samples";
	REQUIRE(in_fld1, "Parameter \"in_field1\" not STRING");
	in_fld2 =
	    (symtype("in_field2") != ST_UNDEF)
		? getsym_s("in_field2")
		    : "samples";
	REQUIRE(in_fld2, "Parameter \"in_field2\" not STRING");
	break;
    case 1:
	in_fld2 = in_fld1;
    }
    if (debug_level)
	Fprintf(stderr, "%s: in_fields \"%s\" and \"%s\"\n",
		ProgName, in_fld1, in_fld2);

    if (fflag < 3)
    {
	out_fld =
	    (symtype("out_field") != ST_UNDEF)
		? getsym_s("out_field")
		    : DEF_OUT_FLD;
	REQUIRE(out_fld, "Parameter \"out_field\" not STRING");
    }
    if (debug_level)
	Fprintf(stderr, "%s: out_field \"%s\"\n", ProgName, out_fld);

    if (!strcmp(out_fld, DEF_OUT_FLD))
    {
	out_fld = 
	    malloc((strlen(in_fld1) + strlen(oper_name) + strlen(in_fld2) + 3)
		   * sizeof(char));
	REQUIRE(out_fld, "Can't allocate space for output field name");
	if (Iflag)
	    (void) sprintf(out_fld, "%s_%s_%s", in_fld2, oper_name, in_fld1);
	else
	    (void) sprintf(out_fld, "%s_%s_%s", in_fld1, oper_name, in_fld2);
    }
    else if (!strcmp(out_fld, "-"))
    {
	out_fld = in_fld2;
	warn_if_repl = NO;
    }
    if (debug_level)
	Fprintf(stderr, "%s: out_field \"%s\"\n", ProgName, out_fld);

    /* output data type */

    inf_type1 = get_fea_type(in_fld1, ihd1);
    REQUIRE(inf_type1 != UNDEF, "First input field not in file");
    inf_type2 = get_fea_type(in_fld2, ihd2);
    REQUIRE(inf_type2 != UNDEF, "Second input field not in file");

    if (debug_level)
	Fprintf(stderr, "%s: input type codes {%d, %d}\n",
		ProgName, inf_type1, inf_type2);

    if (!tflag)
    {
	type_name =
	    (symtype("output_type") != ST_UNDEF)
		? getsym_s("output_type")
		    : DEF_OUT_TYP;
	REQUIRE(type_name, "Parameter \"output_type\" not STRING");
    }
    if (debug_level)
	Fprintf(stderr, "%s: output_type \"%s\"\n", ProgName, type_name);

    if (!strcmp(type_name, DEF_OUT_TYP))
    {
	outf_type = cover_type(inf_type1, inf_type2);
	if (oper == OP_CPLX)
	    outf_type = cover_type(outf_type, BYTE_CPLX);
    }
    else
    {
	outf_type = lin_search(type_codes, type_name);
	if (outf_type == CHAR) outf_type = BYTE;
	REQUIRE(is_type_numeric(outf_type), "Invalid output type");
    }
    if (debug_level)
	Fprintf(stderr, "%s: output type code %d\n", ProgName, outf_type);

    /* gain factor */

    if (gflag)
    {
	gain.real = 1.0;
	gain.imag = 0.0;
	(void) sscanf(gain_arg, "%lf,%lf", &gain.real, &gain.imag);
    }
    else
    {
	switch (symtype("gain_real"))
	{
	case ST_UNDEF:
	    gain.real = 1.0;
	    break;
	case ST_FLOAT:
	    gain.real = getsym_d("gain_real");
	    break;
	default:
	    ERROR("Parameter \"gain_real\" not FLOAT");
	    break;
	}

	switch (symtype("gain_imag"))
	{
	case ST_UNDEF:
	    gain.imag = 0.0;
	    break;
	case ST_FLOAT:
	    gain.imag = getsym_d("gain_imag");
	    break;
	default:
	    ERROR("Parameter \"gain_imag\" not FLOAT");
	    break;
	}
    }
    if (debug_level)
	Fprintf(stderr, "%s: gain {%g, %g}\n",
		ProgName, gain.real, gain.imag);

    /* ranges (start, nan, end) */

    if (!rflag)
    {
	switch (symtype("start"))
	{
	case ST_UNDEF:
	    start_rec1 = start_rec2 = 1;
	    break;
	case ST_INT:
	    start_rec1 = start_rec2 = getsym_i("start");
	    break;
	case ST_IARRAY:
	    REQUIRE(getsym_ia("start", start_recs, 2) == 2,
		    "Length of array parameter \"start\" is not 2");
	    start_rec1 = start_recs[0];
	    start_rec2 = start_recs[1];
	    break;
	default:
	    ERROR("Parameter \"start\" not IARRAY or INT");
	    break;
	}

	switch (symtype("nan"))
	{
	case ST_UNDEF:
	    num_recs1 = num_recs2 = 0;
	    break;
	case ST_INT:
	    num_recs1 = num_recs2 = getsym_i("nan");
	    break;
	case ST_IARRAY:
/*!*//* Why is there no getsym_la? */
	    REQUIRE(getsym_ia("nan", num_recs, 2) == 2,
		    "Length of array parameter \"nan\" is not 2");
	    num_recs1 = num_recs[0];
	    num_recs2 = num_recs[1];
	    break;
	default:
	    ERROR("Parameter \"nan\" not IARRAY or INT");
	    break;
	}

	end_rec1 =
	    (num_recs1 == 0) ? LONG_MAX : start_rec1 - 1 + num_recs1;
	end_rec2 =
	    (num_recs2 == 0) ? LONG_MAX : start_rec2 - 1 + num_recs2;
    }
    else
    {
	start_rec1 = 1;
	end_rec1 = LONG_MAX;
	lrange_switch(rrange1, &start_rec1, &end_rec1, 0);
	num_recs1 =
	    (end_rec1 == LONG_MAX) ? 0 : end_rec1 - start_rec1 + 1;

	switch (rflag)
	{
	case 1:
	    start_rec2 = start_rec1;
	    end_rec2 = end_rec1;
	    num_recs2 = num_recs1;
	    break;
	case 2:
	    start_rec2 = 1;
	    end_rec2 = LONG_MAX;
	    lrange_switch(rrange2, &start_rec2, &end_rec2, 0);
	    num_recs2 =
		(end_rec2 == LONG_MAX) ? 0 : end_rec2 - start_rec2 + 1;
	    break;
	default:
	    ERROR("Internal error--miscounted -r options");
	    break;
	}
    }

    if (debug_level)
    {
	Fprintf(stderr, "%s: start_recs {%d, %d}\n",
	       ProgName, start_rec1, start_rec2);
	Fprintf(stderr, "%s: end_recs {%ld, %ld}\n",
	       ProgName, end_rec1, end_rec2);
	Fprintf(stderr, "%s: num_recs {%ld, %ld}\n",
	       ProgName, num_recs1, num_recs2);
    }

    REQUIRE(start_rec1 >= 1,
	    "Can't start before beginning of first input file");
    REQUIRE(start_rec1 <= end_rec1,
	    "Empty range of records specified in first input file");
    REQUIRE(start_rec2 >= 1,
	    "Can't start before beginning of second input file");
    REQUIRE(start_rec2 <= end_rec2,
	    "Empty range of records specified in second input file");

    if (Rflag)
	num_recs3 = num_recs2;
    else
    {
	if (num_recs1 == 0)
	    num_recs3 = num_recs2;
	else if (num_recs2 == 0)
	    num_recs3 = num_recs1;
	else if (num_recs1 == num_recs2)
	    num_recs3 = num_recs2;
	else
	{
	    num_recs3 = MIN(num_recs1, num_recs2);
	    if (!zflag)
	    {
		Fprintf(stderr,
			"%s: Warning: range lengths (%ld, %ld) are ",
			ProgName, num_recs1, num_recs2);
		Fprintf(stderr, "different; using %ld.\n", num_recs3);
	    }
	}
    }
    end_rec3 = (num_recs3 == 0) ? LONG_MAX : num_recs3;

    if (debug_level)
    {
	Fprintf(stderr, "%s: output end_rec %ld\n", ProgName, end_rec3);
	Fprintf(stderr, "%s: output num_recs %ld\n", ProgName, num_recs3);
    }

    symerr_exit();

    /*
     * More consistency checking.
     */

    if (!zflag
	&& genhd_type("record_freq", (int *) NULL, ihd1) != HD_UNDEF
	&& genhd_type("record_freq", (int *) NULL, ihd2) != HD_UNDEF
	&& ((rec_fr1 = get_genhd_val("record_freq", ihd1, 0.0))
	    != (rec_fr2 = get_genhd_val("record_freq", ihd2, 0.0))))
    {
	Fprintf(stderr,
		"%s: Warning: record frequencies (%g, %g) difer.\n",
		ProgName, rec_fr1, rec_fr2);
    }

    if (debug_level)
	Fprintf(stderr, "%s: record_freqs (%g, %g); difference %g.\n",
		ProgName, rec_fr1, rec_fr2, rec_fr1 - rec_fr2);

    /*
     * Create output-file header.
     */

    ohd = copy_header(ihd2);

    if (get_fea_type(out_fld, ihd2) == UNDEF)
	fld_names = NULL;	/* Copy all input fields unchanged. */
    else
    {
	char	**out_names;

	if (!zflag && warn_if_repl)
	    Fprintf(stderr,
		    "%s: output field %s replaces existing field in %s.\n",
		    ProgName, out_fld, iname2);
	if (debug_level)
	    Fprintf(stderr,
		    "%s: removing old field %s\n",
		    ProgName, out_fld);

	(void) del_fea_fld(out_fld, ohd);

	/* Make list of fields to copy unchanged: all but output field. */

	fld_names =
	    (char **) malloc((1 + ohd->hd.fea->field_count) * sizeof(char *));
	REQUIRE(fld_names, "Can't allocate memory for list of field names");
	out_names = ohd->hd.fea->names;
	for (i = 0; fld_names[i] = out_names[i]; i++)
	{ }
    }

    if (debug_level > 1)
    {
	Fprintf(stderr, "%s: fields to copy unchanged from input:", ProgName);
	if (!fld_names)
	    Fprintf(stderr, " ALL.\n");
	else
	{
	    for (i = 0; fld_names[i]; i++)
		Fprintf(stderr, "\t%s", fld_names[i]);
	    Fprintf(stderr, "\n");
	}
    }

    size1 = get_fea_siz(in_fld1, ihd1, &rank1, &dim1);
    size2 = get_fea_siz(in_fld2, ihd2, &rank2, &dim2);
    REQUIRE(size1 > 0, "Bad field definition in input file 1");
    REQUIRE(size2 > 0, "Bad field definition in input file 2");
    REQUIRE(size1 == size2, "Input field sizes differ");

    if (!zflag)
    {
	if (rank1 != rank2)
	    Fprintf(stderr,
		    "%s: Warning: input field ranks differ (%d, %d).\n",
		    ProgName, rank1, rank2);
	else if (rank2 > 1)
	    for (i = 0; i < rank2; i++)
		if (dim1[i] != dim2[i])
		{
		    Fprintf(stderr,
			    "%s: Warning: input field dimens dim[%d] ",
			    ProgName, i);
		    Fprintf(stderr, "differ (%d, %d).\n", dim1[i], dim2[i]);
		}
    }

    if (debug_level)
	Fprintf(stderr, "%s: adding new field %s\n", ProgName, out_fld);

    REQUIRE(add_fea_fld(out_fld, size2, rank2,
			dim2, outf_type, (char **) NULL, ohd) != -1,
	    "Can't create output field in output file header");

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname1, ihd1);
    add_source_file(ohd, iname2, ihd2);
    add_comment(ohd, get_cmd_line(argc, argv));
    (void) sprintf(sbuf, "field %s added by %s\n", out_fld, ProgName);
    add_comment(ohd, sbuf);
    (void) copy_genhd(ohd, ihd2, (char *) NULL);

    *add_genhd_e("operation", (short *) NULL, 1, operation_names, ohd) = oper;
    *add_genhd_d("gain_real", (double *) NULL, 1, ohd) = gain.real;
    *add_genhd_d("gain_imag", (double *) NULL, 1, ohd) = gain.imag;
    (void) add_genhd_c("in_field1", in_fld1, 0, ohd);
    (void) add_genhd_c("in_field2", in_fld2, 0, ohd);
    (void) add_genhd_c("out_field", out_fld, 0, ohd);
    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs2;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec2;
    *add_genhd_l("nan1", (long *) NULL, 1, ohd) = num_recs1;
    *add_genhd_l("start1", (long *) NULL, 1, ohd) = start_rec1;

    update_waves_gen(ihd2, ohd, (float) start_rec2, (float) 1.0);

    /*
     * Open output file and write header.
     */

    oname = eopen(ProgName, oname, "w", NONE, NONE, &ohd, &ofile);

    if (debug_level)
	Fprintf(stderr, "%s: output file \"%s\"\n", ProgName, oname);

    if (debug_level)
	Fprintf(stderr, "%s: writing output header to file\n", ProgName);

    write_header(ohd, ofile);

    /*
     * Allocate storage.
     */

    if (debug_level)
	Fprintf(stderr, "%s: allocating records\n", ProgName);

    irec1 = allo_fea_rec(ihd1);
    REQUIRE(irec1 != NULL, "can't allocate memory for input record (file 1)");

    in1_ptr = get_fea_ptr(irec1, in_fld1, ihd1);
    REQUIRE(in1_ptr, "can't locate field in input record (file 1)");

    irec2 = allo_fea_rec(ihd2);
    REQUIRE(irec2 != NULL, "can't allocate memory for input record (file 2)");

    in2_ptr = get_fea_ptr(irec2, in_fld2, ihd2);
    REQUIRE(in2_ptr, "can't locate field in input record (file 2)");

    orec = allo_fea_rec(ohd);
    REQUIRE(orec != NULL, "can't allocate memory for output record");

    out_ptr = get_fea_ptr(orec, out_fld, ohd);
    REQUIRE(out_ptr, "can't locate data field in output record");

    scale = !(gain.real == 1.0 && gain.imag == 0.0);

    if (scale)
    {
	if (debug_level)
	    Fprintf(stderr, "%s: will rescale data\n", ProgName);

	gain_type = (gain.imag == 0.0) ? DOUBLE : DOUBLE_CPLX;
	scal_type = cover_type(gain_type, inf_type1);

	if (debug_level)
	{
	    Fprintf(stderr, "%s: gain factor type %d ", ProgName, gain_type);
	    Fprintf(stderr, "(%s)\n", type_codes[gain_type]);
	    Fprintf(stderr, "%s: rescaled data type %d ", ProgName, scal_type);
	    Fprintf(stderr, "(%s)\n", type_codes[scal_type]);
	}

	gain_ptr = arr_alloc(1, &size1, gain_type, 0);

/*!*//* replicate gain factor to match in_fld1;
	remove this silliness if we get a more general arr_op
	(supporting <scalar><op><array>) */

	switch (gain_type)
	{
	case DOUBLE:
	    {
		dbl_ptr = (double *) gain_ptr;
		for (i = 0; i < size1; i++)
		    dbl_ptr[i] = gain.real;
	    }
	    break;
	case DOUBLE_CPLX:
	    {
		cplx_ptr = (double_cplx *) gain_ptr;
		for (i = 0; i < size1; i++)
		    cplx_ptr[i] = gain;
	    }
	    break;
	}

	scal_ptr = arr_alloc(1, &size1, scal_type, 0);
    }
    else
    {
	scal_type = inf_type1;
	scal_ptr = in1_ptr;
    }

    /*
     * Main read-write loop.
     */

    if (debug_level)
	Fprintf(stderr, "%s: skipping to start of data\n", ProgName);

    fea_skiprec(ifile1, start_rec1 - 1, ihd1);
    offset = ftell(ifile1);
    fea_skiprec(ifile2, start_rec2 - 1, ihd2);

    /* This can treat single samples as general FEA records.  Consider
       special handling of FEA_SD files with no extraneous fields,
       using get_feasd_recs instead of get_fea_rec. */

    for (num_out = 0;
	 num_out < end_rec3
	 && (get1 = getdata1()) != EOF
	 && (get2 = get_fea_rec(irec2, ihd2, ifile2)) != EOF;
	 num_out++)
    {
	if (debug_level >= 2)
	    Fprintf(stderr, "%s: reading input\n", ProgName);

	copy_fea_rec(irec2, ihd2, orec, ohd, fld_names, (short **) NULL);

	if (scale)
	    (void) arr_op(OP_MUL, size1, gain_ptr, gain_type,
			  in1_ptr, inf_type1, scal_ptr, scal_type);

	if (Iflag)
	    arr_op(oper, size2, in2_ptr, inf_type2,
		   scal_ptr, scal_type, out_ptr, outf_type);
	else
	    arr_op(oper, size2, scal_ptr, scal_type,
		   in2_ptr, inf_type2, out_ptr, outf_type);

	if (debug_level >= 3)
	{
	    Fprintf(stderr, "input1 record\n");
	    print_fea_rec(irec1, ihd1, stderr);
	    Fprintf(stderr, "input2 record\n");
	    print_fea_rec(irec2, ihd2, stderr);
	    Fprintf(stderr, "output record %ld\n", num_out + 1);
	    print_fea_rec(orec, ohd, stderr);
	}

	put_fea_rec(orec, ohd, ofile);
    }

    if (!zflag && num_out < end_rec3 && num_recs3 != 0)
    {
	Fprintf(stderr, "%s: Warning: only %ld records written.\n",
		ProgName, num_out);
	if (get1 == EOF)
	    Fprintf(stderr, "%s: end of first input file.\n", ProgName);
	if (get2 == EOF)
	    Fprintf(stderr, "%s: end of second input file.\n", ProgName);
    }

    if (debug_level)
    {
	Fprintf(stderr, "%s: done writing output;  %ld records written.\n",
		ProgName, num_out);
	if (get1 == EOF)
	    Fprintf(stderr, "%s: end of first input file.\n", ProgName);
	if (get2 == EOF)
	    Fprintf(stderr, "%s: end of second input file.\n", ProgName);
    }

    /*
     * Write common.
     */

    if (ofile != stdout)
    {
	if (debug_level)
	    Fprintf(stderr, "%s: updating ESPS common\n", ProgName);

	REQUIRE(putsym_i("start", 1) != -1,
		"can't write \"start\" to Common");
/*!*//* Why is there no putsym_l? */
	REQUIRE(putsym_i("nan", (int) num_out) != -1,
		"can't write \"nan\" to Common");
	REQUIRE(putsym_s("prog", ProgName) != -1,
		"can't write \"prog\" to Common");
	REQUIRE(putsym_s("filename", oname) != -1,
		"can't write \"filename\" to Common");
    }

    exit(0);
    /*NOTREACHED*/
}


int
getdata1()
{
    int	    n;

    n = get_fea_rec(irec1, ihd1, ifile1);
    if (n != EOF || !Rflag)
	return n;

    if (debug_level >= 2)
	Fprintf(stderr, "%s: reusing input.\n", ProgName);

    if (fseek(ifile1, offset, 0) < 0)
    {
	Fprintf(stderr,
		"%s: seek failed; can't get back to start of input.\n",
		ProgName);
	return EOF;
    }

    n = get_fea_rec(irec1, ihd1, ifile1);
    if (n == EOF)
	Fprintf(stderr, "%s: can't read first input file.\n", ProgName);

    return n;
}
