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
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:  
 *
 * Apply clipping and center-clipping operations to FEA fields.
 *
 */

static char *sccs_id = "@(#)clip.c	1.2	8/31/95	ESI/ERL";

/*
  Based on feafunc.c.
*/

#define VERSION "1.2"
#define DATE "8/31/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define REQUIRE(test, text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("clip [-c range] [-f input_field [-f output_field]] [-m range] [-r range] [-x debug_level] [-C const_value] [-P params] input.fea output.fea") ;

#define ABS(x) ((x) < 0 ? -(x) : (x))

#define CONVERT(src, dst, type) \
((void) type_convert(1L, (char *) &src, DOUBLE, (char *) &dst, type, \
(void (*)()) NULL))

double	    floor(), ceil();
extern char *type_codes[];

char	    *get_cmd_line();
void	    lrange_switch(), frange_switch();
char	    *arr_alloc();

static int  numeric();			/* tests whether type is numeric */

char	    *ProgName = "clip";
char	    *Version = VERSION;
char	    *Date = DATE;
char	    sbuf[256];			/* to hold comment */
int	    debug_level = 0;		/* debug level, global for library*/

long	    num_read;			/* number of records skipped or read */

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
    int		    cflag = NO;	/* -c option specified? */
    char	    *crange;	/* arguments of -c option */
    double	    ctr_min,
		    ctr_max;	/* limits of center-clipping interval */
    int		    ctr_clip;	/* do center-clipping? */
    int		    Cflag = NO;	/* -C option specified? */
    double	    const_val;	/* argument of -C option */
    int		    fflag = 0;	/* -f option specified 0, 1, or 2 times? */
    char	    *inf_name;	/* field name for input field */
    int		    inf_type;	/* data type of input field */
    char	    *outf_name;	/* field name for output field */
    int		    warn_if_repl = YES;	/* print warning if output
					   field exists in input file? */
    char	    **fld_names;	/* fields to be copied unchanged */
    int		    mflag = NO;	/* -m option specified? */
    char	    *mrange;	/* arguments of -m option */
    double	    clip_min,
		    clip_max;	/* limits of clipping interval */
    int		    rflag = NO;	/* -r option specified? */
    char	    *rrange;	/* arguments of -r option */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				   (0 means all up to end of file) */

    char	    *param_name = NULL;	/* parameter file name */

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
    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    int		    i;		/* loop index */


    /*
     * Parse command-line options.
     */

    while ((ch = getopt(argc, argv, "c:f:m:r:x:C:P:")) != EOF)
	switch (ch)
	{
	case 'c':
	    cflag = YES;
	    crange = optarg;
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
	case 'm':
	    mflag = YES;
	    mrange = optarg;
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'C':
	    Cflag = YES;
	    const_val = atof(optarg);
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
	Fprintf(stderr, "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr, "%s: too few file names specified.\n", ProgName);
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

    /* -f: field names */

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
	|| (strcmp(outf_name, "[input_name]_CLIP") == 0)) {
	outf_name =
	    malloc((unsigned)(strlen(inf_name) + strlen("_CLIP") + 1));
	REQUIRE(outf_name, "can't allocate space for output field name");
	(void) strcpy(outf_name, inf_name);
	(void) strcat(outf_name, "_CLIP");
    }
    else if (strcmp(outf_name, "-") == 0)
    {
	outf_name = inf_name;
	warn_if_repl = NO;
    }

    if (debug_level) {
	Fprintf(stderr,
		"input_field: \"%s\".  output_field: \"%s\".\n",
		inf_name, outf_name);
    }

    inf_type = get_fea_type(inf_name, ihd);
    if (inf_type == UNDEF) {
	Fprintf(stderr, "%s: input_field %s not in input file\n",
		ProgName, inf_name);
	exit(1);
    }
    if (!numeric(inf_type))
    {
	Fprintf(stderr, "%s: invalid input data type.\n", ProgName);
	exit(1);
    }

    /* -c & -C: center-clipping limits and const_value */

    if (cflag)
    {
	ctr_max = -DBL_MAX;
	ctr_min = DBL_MAX;
	frange_switch(crange, &ctr_min, &ctr_max);
    }
    else if (symtype("ctr_max") != ST_UNDEF && symtype("ctr_min") != ST_UNDEF)
    {
	ctr_max = getsym_d("ctr_max");
	ctr_min = getsym_d("ctr_min");
    }
    else
    {
	ctr_max = -DBL_MAX;
	ctr_min = DBL_MAX;
    }

    ctr_clip = (ctr_min <= ctr_max);

    if (ctr_clip && !Cflag)
    {
	const_val =
	    (symtype("const_value") != ST_UNDEF)
		? getsym_d("const_value") : 0.0;
    }

    if (debug_level)
    {
	Fprintf(stderr, "ctr_min: %g.  ctr_max: %g.\n", ctr_min, ctr_max);
	if (ctr_clip) Fprintf(stderr, "const_value: %g.\n", const_val);
	else Fprintf(stderr, "No center-clipping.");
    }

    /* -m: clipping limits */

    if (mflag)
    {
	clip_max = DBL_MAX;
	clip_min = -DBL_MAX;
	frange_switch(mrange, &clip_min, &clip_max);
    }
    else
    {
	clip_max =
	    (symtype("clip_max") != ST_UNDEF)
		? getsym_d("clip_max") : DBL_MAX;
	clip_min =
	    (symtype("clip_min") != ST_UNDEF)
		? getsym_d("clip_min") : -DBL_MAX;
    }

    if (debug_level)
	Fprintf(stderr, "clip_min: %g.  clip_max: %g.\n", clip_min, clip_max);

    /* -r: range of records */

    if (rflag)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	num_recs = (end_rec != LONG_MAX) ? end_rec - start_rec + 1 : 0;
    }
    else
    {
	start_rec = (symtype("start") != ST_UNDEF) ? getsym_i("start") : 1;
	num_recs = (symtype("nan") != ST_UNDEF) ? getsym_i("nan") : 0;
	end_rec = (num_recs != 0) ? start_rec - 1 + num_recs : LONG_MAX;
    }

    if (debug_level)
	Fprintf(stderr, "start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

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
		    ProgName, outf_name);

	if (debug_level)
	    Fprintf(stderr, "%s: removing old field %s.\n",
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

    size = get_fea_siz(inf_name, ihd, &rank, &dimens);

    REQUIRE(size != 0, "bad field definition in input file");

    if (debug_level)
	Fprintf(stderr, "%s: adding new field %s.\n", ProgName, outf_name);

    REQUIRE(add_fea_fld(outf_name, size, rank,
			dimens, inf_type, (char **) NULL, ohd) != -1,
	    "can't create output field in output file header");

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));
    sprintf(sbuf, "function field %s added by %s\n", outf_name, ProgName);
    add_comment(ohd, sbuf);
    (void) copy_genhd(ohd, ihd, (char *) NULL);

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

    irec = allo_fea_rec(ihd);
    REQUIRE(irec != NULL, "can't allocate memory for input record");

    inf_ptr = get_fea_ptr(irec, inf_name, ihd);
    REQUIRE(inf_ptr, "can't locate field in input record");

    orec = allo_fea_rec(ohd);
    REQUIRE(orec != NULL, "can't allocate memory for output record");

    outf_ptr = get_fea_ptr(orec, outf_name, ohd);
    REQUIRE(outf_ptr, "can't locate data field in output record");

    /*
     * MAIN READ-WRITE LOOP
     */

    num_read = start_rec - 1;

    fea_skiprec(ifile, num_read, ihd);

    /* This can treat single samples as general FEA records.  Consider
       special handling of FEA_SD files with no extraneous fields,
       using get_feasd_recs instead of get_fea_rec. */

#define SETUP(min, max, marg) \
    {	\
	if (ctr_min > max) ctr_clip = NO;	\
	else ctr_min = ceil(ctr_min);		\
	\
	if (ctr_max < min) ctr_clip = NO;	\
	else ctr_max = floor(ctr_max);		\
	\
	if (ctr_clip)	\
	{   \
	    if (const_val < min - marg || const_val > max + marg)   \
	    {	\
		Fprintf(stderr,						    \
			"%s: replacement value %g out of range for %s.\n",  \
			ProgName, const_val, type_codes[inf_type]);	    \
		exit(1);						    \
	    }	\
	}   \
    }

#define	REAL_LOOP(type, TYPE) \
    {	\
	type    *in_dat = (type *) inf_ptr,			\
		*out_dat = (type *) outf_ptr;			\
	type    clip_lo, clip_hi, ctr_lo, ctr_hi, c_val;	\
	type    val;						\
	\
	if (ctr_clip)	\
	{   \
	    CONVERT(const_val, c_val, TYPE);	\
	    CONVERT(ctr_min, ctr_lo, TYPE);	\
	    CONVERT(ctr_max, ctr_hi, TYPE);	\
	}   \
	\
	CONVERT(clip_min, clip_lo, TYPE);	\
	CONVERT(clip_max, clip_hi, TYPE);	\
	\
	if (debug_level)	\
	{   \
	    if (ctr_clip)	\
		Fprintf(stderr, "ctr_lo %g.  ctr_hi %g.  c_val %g.\n",	    \
			(double) ctr_lo, (double) ctr_hi, (double) c_val);  \
	    else		\
		Fprintf(stderr, "No center-clipping.\n");		    \
	    \
	    Fprintf(stderr, "clip_lo %g.  clip_hi %g.\n",		    \
		    (double) clip_lo, (double) clip_hi);		    \
	}   \
	\
	while (num_read++ < end_rec && get_fea_rec(irec, ihd, ifile) != EOF) \
	{   \
	    copy_fea_rec(irec, ihd, orec, ohd, fld_names, (short **) NULL); \
	    \
	    for (i = 0; i < size; i++)	\
	    {   \
		val = in_dat[i];					    \
		out_dat[i] =						    \
		    (ctr_clip && ctr_lo <= val && val <= ctr_hi) ? c_val    \
		    : (val < clip_lo) ? clip_lo				    \
		    : (val > clip_hi) ? clip_hi				    \
		    : val;						    \
	    }   \
	    \
	    put_fea_rec(orec, ohd, ofile);  \
	}   \
    }

#define CPLX_LOOP(r_type, c_type, TYPE) \
    {	\
	c_type  *in_dat = (c_type *) inf_ptr,			\
		*out_dat = (c_type *) outf_ptr;			\
	r_type  clip_lo, clip_hi, ctr_lo, ctr_hi, c_val;	\
	r_type  rval, ival;					\
	\
	if (ctr_clip)	\
	{   \
	    CONVERT(const_val, c_val, TYPE);	\
	    CONVERT(ctr_min, ctr_lo, TYPE);	\
	    CONVERT(ctr_max, ctr_hi, TYPE);	\
	}   \
	\
	CONVERT(clip_min, clip_lo, TYPE);	\
	CONVERT(clip_max, clip_hi, TYPE);	\
	\
	if (debug_level)	\
	{   \
	    if (ctr_clip)	\
		Fprintf(stderr, "ctr_lo %g.  ctr_hi %g.  c_val %g.\n",	    \
			(double) ctr_lo, (double) ctr_hi, (double) c_val);  \
	    else		\
		Fprintf(stderr, "No center-clipping.\n");		    \
	    \
	    Fprintf(stderr, "clip_lo %g.  clip_hi %g.\n",		    \
		    (double) clip_lo, (double) clip_hi);		    \
	}   \
	\
	while (num_read++ < end_rec && get_fea_rec(irec, ihd, ifile) != EOF) \
	{   \
	    copy_fea_rec(irec, ihd, orec, ohd, fld_names, (short **) NULL); \
	    \
	    for (i = 0; i < size; i++)	\
	    {   \
		rval = in_dat[i].real;					    \
		ival = in_dat[i].imag;					    \
		out_dat[i].real =					    \
		    (ctr_clip && ctr_lo <= rval && rval <= ctr_hi) ? c_val  \
		    : (rval < clip_lo) ? clip_lo			    \
		    : (rval > clip_hi) ? clip_hi			    \
		    : rval;						    \
		out_dat[i].imag =					    \
		    (ctr_clip && ctr_lo <= ival && ival <= ctr_hi) ? c_val  \
		    : (ival < clip_lo) ? clip_lo			    \
		    : (ival > clip_hi) ? clip_hi			    \
		    : ival;						    \
	    }   \
	    \
	    put_fea_rec(orec, ohd, ofile);  \
	}   \
    }


    switch (inf_type)
    {
    case BYTE:
	SETUP(CHAR_MIN, CHAR_MAX, 0.5);
 	REAL_LOOP(char, BYTE);
	break;
    case SHORT:
	SETUP(SHRT_MIN, SHRT_MAX, 0.5);
	REAL_LOOP(short, SHORT);
	break;
    case LONG:
	SETUP(LONG_MIN, LONG_MAX, 0.5);
	REAL_LOOP(long, LONG);
	break;
    case FLOAT:
	SETUP(-FLT_MAX, FLT_MAX, 0.0);
	REAL_LOOP(float, FLOAT);
	break;
    case DOUBLE:
	SETUP(-DBL_MAX, DBL_MAX, 0.0);
	REAL_LOOP(double, DOUBLE);
	break;
    case BYTE_CPLX:
	SETUP(CHAR_MIN, CHAR_MAX, 0.5);
	CPLX_LOOP(char, byte_cplx, BYTE);
	break;
    case SHORT_CPLX:
	SETUP(SHRT_MIN, SHRT_MAX, 0.5);
	CPLX_LOOP(short, short_cplx, SHORT);
	break;
    case LONG_CPLX:
	SETUP(LONG_MIN, LONG_MAX, 0.5);
	CPLX_LOOP(long, long_cplx, LONG);
	break;
    case FLOAT_CPLX:
	SETUP(-FLT_MAX, FLT_MAX, 0.0);
	CPLX_LOOP(float, float_cplx, FLOAT);
	break;
    case DOUBLE_CPLX:
	SETUP(-DBL_MAX, DBL_MAX, 0.0);
	CPLX_LOOP(double, double_cplx, DOUBLE);
	break;
    }

    if (num_read <= end_rec && num_recs != 0)
	Fprintf(stderr, "%s: only %ld records read.\n",
		ProgName, num_read - start_rec);

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


/* There are separate static copies of "numeric" in atoarray.c, feasdsupp.c,
   feafunc.c, and typeconver.c.  Make public. */

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
