/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988 Entropic Speech, Inc.  All rights reserved."	|
|    Copyright (c) 1996 Entropic Research Lab. All rights reserved.	|
+-----------------------------------------------------------------------+
|									|
|  Program: transpec.c							|
|									|
|  This program reads a FEA file with a numeric vector field contain-	|
|  ing spectral information, transforms the vectors to a different	|
|  spectral representation, and outputs a FEA file that contains the	|
|  new spectral representatin and all the other original fields.  By	|
|  default, the new spectral representation is written over the old	|
|  one, but if an  output_spec_field  is supplied, the original param-  |
|  eters are left in place, and a new field containing the transformed	|
|  parameters is added to each record.					|
|									|
|  FEA_ANA-only version by David Burton					|
|  General FEA-file version by Rodney Johnson				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)transpec.c	1.6       9/18/96     ERL";
#endif
#define VERSION "1.6"
#define DATE "9/18/96"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/anafea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("transpec [-i input_field] [-m target_rep] [-o output_field]\n [-r source_rep] [-s samp_freq] [-w grid_width] [-x debug_level]\n [-P params] infile.fea outfile.fea") \
 ;

/*
 * ESPS Functions
 */

char	*get_cmd_line();
char	*uniq_name();
void	lrange_switch();
void	copy_fea_rec();
int     lin_search();
int     rc_reps();
int     reps_rc();
void	pr_farray();
int     getopt();

/*
 * ESPS Variables
 */

char	*ProgName = "transpec";
char	*Version = VERSION;
char	*Date = DATE;

int	debug_level = 0;	/* remove this when references in library
				   are removed */

/*
 * MAIN PROGRAM
 */
int
main(argc, argv)
    int  argc;
    char **argv;
{
    extern int	optind;		/* for use of getopt() */
    extern char	*optarg;	/* for use of getopt() */
    int		ch;		/* command-line option letter */

    int		iflag = NO;	/* -i option specified? */
    char	*input_field;	/* name of input field */
    int		input_type;	/* type of input field */

    int		mflag = NO;	/* -m option specified? */
    int		target_rep;	/* numeric code for target spec rep */

    int		oflag = NO;	/* -o option specified? */
    char	*output_field;	/* name of output field */
    int		new_field;	/* output field different from input? */

    int		rflag = NO;	/* -r option specified? */
    int		source_rep;	/* numeric code for source spec rep */

    int		sflag = NO;	/* sampling frequency from command line
				   or param file? */
    float	samp_freq;	/* sampling frequency */

    int		wflag = NO;	/* -w option specified? */
    double	grid_width;	/* grid spacing for LSF computation */

    int		debug = 0;	/* debug level */

    char	*params = NULL;
				/* parameter file name */

    char	*iname;		/* input file name */
    FILE	*ifile;		/* input stream */
    struct header
		*ihd;		/* input file header */
    struct fea_data
		*irec;		/* input record */
    float	*in_vec;	/* pointer to field in input record */
    
    long	order;		/* filter order */

    char	*oname;		/* output file name */
    FILE	*ofile;		/* output stream */
    struct header
		*ohd;		/* output file header */
    struct fea_data
		*orec;		/* output record */
    float	*out_vec;	/* pointer to field in output record */

    float	*in_params;	/* copy of in_vec */
    float	*rc;		/* reflection coefficients */
    int	        size;		/* dummy */
    char	*name;		/* temporary */

/*
 * Parse command-line options.
 */

    while ((ch = getopt(argc, argv, "i:m:o:r:s:w:x:P:")) != EOF)
	switch (ch)
	{
	case 'i':
	    iflag = YES;
	    input_field = optarg;
	    break;
	case 'm':
	    mflag = YES;
	    target_rep = lin_search(spec_reps, optarg);
	    break;
	case 'o':
	    oflag = YES;
	    output_field = optarg;
	    break;
	case 'r':
	    rflag = YES;
	    source_rep = lin_search(spec_reps, optarg);
	    break;
	case 's':
	    sflag = YES;
	    samp_freq = atof(optarg);
	    break;
	case 'w':
	    wflag = YES;
	    grid_width = atof(optarg);
	    break;
	case 'x':
	    debug = atoi(optarg);
	    break;
	case 'P':
	    params = optarg;
	    break;
	}
/*
 * Process file names and open files.
 */

    if (argc - optind > 2)
    {
	Fprintf(stderr, "Too many file names specified.\n");
	SYNTAX
    }

    if (argc - optind < 2)
    {
	Fprintf(stderr, "Too few file names specified.\n");
	SYNTAX
    }

    iname = eopen(ProgName, argv[optind++],
			    "r", FT_FEA, NONE, &ihd, &ifile);

    REQUIRE(strcmp(iname, argv[optind]) != 0,
	    "Input and output files must not be the same.")

    oname = eopen(ProgName, argv[optind++],
			    "w", FT_FEA, NONE, &ohd, &ofile);

    if (debug)
	Fprintf(stderr, "Input file: %s.  Output file: %s.\n",
		iname, oname);

/*
 * Get parameter values.
 */

    (void) read_params(params, SC_NOCOMMON, (char *) NULL);

    if (!iflag)
	if (symtype("input_field") == ST_STRING)
	    input_field = getsym_s("input_field");
	else
	{
	    REQUIRE(symtype("input_field") == ST_UNDEF,
		    "Symbol \"input_field\" not string.")
	    input_field = "spec_param";
	}
    input_type = get_fea_type(input_field, ihd);
    REQUIRE(input_type == FLOAT,
	    "Input field undefined or not float.")

    if (debug)
	Fprintf(stderr, "Input_field: %s.  (Type: %d.)\n",
		input_field, input_type);

    if (!mflag)
    {
	if (symtype("target_rep") == ST_UNDEF)
	{
	    Fprintf(stderr,
		"No -m option; symbol \"target_rep\" undefined.\n");
	    SYNTAX
	}
	REQUIRE(symtype("target_rep") == ST_STRING,
		"Symbol \"target_rep\" not string.")
	target_rep = lin_search(spec_reps, getsym_s("target_rep"));
    }
    REQUIRE(target_rep != -1,
	    "Can't determine target representation.")

    if (debug)
	Fprintf(stderr, "Target_rep: %d.\n", target_rep);

    if (!oflag)
	if (symtype("output_field") == ST_STRING)
	{
	    output_field = getsym_s("output_field");
	    if (strcmp(output_field, "") == 0)
		output_field = NULL;
	}
	else
	{
	    REQUIRE(symtype("output_field") == ST_UNDEF,
		    "Symbol \"output_field\" not string.")
	    output_field = NULL;
	}
    if (output_field)
    {
	REQUIRE(strcmp(input_field, output_field) != 0,
		"New field name conflicts with old.")
	new_field = YES;
    }
    else
    {
	output_field = input_field;
	new_field = NO;
    }

    if (debug)
	Fprintf(stderr, "Output_field: %s.  (New_field: %d.)\n",
		output_field, new_field);

    if (!rflag)
	if (symtype("source_rep") == ST_STRING)
	    source_rep = lin_search(spec_reps, getsym_s("source_rep"));
	else
	{
	    REQUIRE(symtype("source_rep") == ST_UNDEF,
		    "Symbol \"source_rep\" not string.")
	    REQUIRE(genhd_type("spec_rep", (int *) NULL, ihd) == CODED,
		    "Header item \"spec_rep\" undefined or not coded.")
	    source_rep =
		lin_search(spec_reps,
			    genhd_codes("spec_rep", ihd)
				[*get_genhd_s("spec_rep", ihd)]);
	    /*
	     * In a FEA_ANA file, we could probably get away with assuming
	     *	that  genhd_codes(...)  was the same as  spec_reps  and
	     *	simply letting  source_rep = *get_genhd_s(...); .  This
	     *	wouldn't be valid for general FEA files.  When
	     *	get_genhd_cstring() is available, consider using it
	     *	instead of  genhd_codes(...)[*get_genhd_s(...)] .
	     */
	}
	REQUIRE(source_rep != -1,
		"Can't determine source representation.")

    if (debug)
	Fprintf(stderr, "Source_rep: %d.\n", source_rep);

    if (!sflag && (source_rep == LSF || target_rep == LSF))
    {
	if (symtype("samp_freq_name") == ST_STRING)
	{
	    char	*samp_freq_name = getsym_s("samp_freq_name");

	    REQUIRE(genhd_type(samp_freq_name, (int *) NULL, ihd) == FLOAT,
		    "Header item named by symbol \"samp_freq_name\" undefined or not float.")

	    samp_freq = *get_genhd_f(samp_freq_name, ihd);
	}
	else
	if (symtype("samp_freq") == ST_FLOAT)
	{
	    samp_freq = getsym_d("samp_freq");
	    sflag = YES;
	}
	else
	{
	    REQUIRE(genhd_type("src_sf", (int *) NULL, ihd) == FLOAT,
		    "Can't find sampling frequency.")
	    samp_freq = *get_genhd_f("src_sf", ihd);
	}

	if (debug)
	    Fprintf(stderr, "Sampling frequency: %g.  (sflag: %d.)\n",
		    samp_freq, sflag);
    }

    if (!wflag && target_rep == LSF)
    {
	if (symtype("grid_width") == ST_FLOAT)
	    grid_width = getsym_d("grid_width");
	else
	{
	    REQUIRE(symtype("grid_width") == ST_UNDEF,
		    "Symbol \"grid_width\" not float.")
	    grid_width = 4.0;
	}

	if (debug)
	    Fprintf(stderr, "Grid_width: %g.\n", grid_width);
    }

/*
 * Get order of input field.
 */

    order = get_fea_siz(input_field, ihd, (short *) NULL, (long **) NULL);

    if (debug)
	Fprintf(stderr, "Order: %ld.\n", order);

/*
 * Create and write output-file header.
 */

    ohd = copy_header(ihd);

    if (debug)
	Fprintf(stderr, "Copied header.\n");

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));

    if (new_field)
    {
	REQUIRE(add_fea_fld( output_field, order, 1,
			(long *) NULL, FLOAT, (char **) NULL, ohd ) != -1,
		"Can't create output field in output file header.")

	if (debug)
	    Fprintf(stderr, "Created field %s in output header.\n",
		    output_field);
    }

    if (genhd_type("spec_rep", (int *) NULL, ohd) != CODED)
	(void) add_genhd_e("spec_rep", (short *) NULL, 1, spec_reps, ohd);
    *get_genhd_s("spec_rep", ohd) = target_rep;

    if (debug)
	Fprintf(stderr, "Put spec_rep %d (%s) in output header.\n",
		*get_genhd_s("spec_rep", ohd),
		spec_reps[*get_genhd_s("spec_rep", ohd)]);

    if (target_rep == LSF)
    {

	name = uniq_name("LSF_grid_width", genhd_list(&size, ohd));
	(void) add_genhd_d(name, &grid_width, 1, ohd );

	if (debug)
	    Fprintf(stderr, "Added header item %s = %g to output header.\n",
		    name, grid_width);

	if (sflag)
	{
	    name = uniq_name("src_sf", genhd_list(&size, ohd));
	    (void) add_genhd_f(name, &samp_freq, 1, ohd );

	    if (debug)
		Fprintf(stderr,
			"Added header item %s = %g to output header.\n",
			name, samp_freq);
	}
    }

    write_header(ohd, ofile);

    if (debug)
	Fprintf(stderr, "Wrote header.\n");

/*
 * Allocate records and buffers.
 */

    irec = allo_fea_rec(ihd);
    in_vec = (float *) get_fea_ptr(irec, input_field, ihd);

    orec = allo_fea_rec(ohd);
    out_vec = (float *) get_fea_ptr(orec, output_field, ohd);

    if (target_rep == RC)
	rc = out_vec;
    else
    {
	rc = malloc_f((unsigned) order);
	REQUIRE(rc, "Can't allocate storage for reflection coefficients.")
    }

    if (source_rep== RC)
	in_params = rc;
    else
    {
	in_params = malloc_f((unsigned) order);
	REQUIRE(in_params,
		"Can't allocate storage for copy of input parameters.")
    }

    if (debug)
    {
	Fprintf(stderr,
		"Allocated storage.  in_vec at 0x%8x, in_params at 0x%8x,\n",
		in_vec, in_params);
	Fprintf(stderr, "    rc at 0x%8x, out_vec at 0x%8x.\n",
		rc, out_vec);
    }
/*
 * Main procssing loop.
 */

    while (get_fea_rec(irec, ihd, ifile) != EOF)
    {
	int	i;

	copy_fea_rec(irec, ihd, orec, ohd, (char **) NULL, (short **) NULL);

	if (debug >= 2)
	    Fprintf(stderr, "Read and copied a record.\n");

	/*
	 * rc is aliased with in_params when source_rep == RC,
	 * with out_vec when target_rep == RC,
	 * and with both for the trivial transformation RC -> RC.
	 */

	for (i = 0; i < order; i++)
	    in_params[i] = in_vec[i];

	if (debug >= 3)
	    pr_farray("in_params", (int) order, in_params);

	if (source_rep != RC)
	{
	    REQUIRE(reps_rc(in_params, source_rep, rc,
			    (int) order, samp_freq/2.0) == 0,
		    "Error transforming to reflection coefficients.")

	    if (debug >= 3)
	    {
		Fprintf(stderr, "reps_rc(in_params, %s, rc, %ld, %g)\n",
			spec_reps[source_rep], order, samp_freq/2.0);
		pr_farray("rc", (int) order, rc);
	    }
	}

	if (target_rep!= RC)
	{
	    REQUIRE(rc_reps(rc, out_vec, target_rep,
			    (int) order, samp_freq/2.0, grid_width) == 0,
		    "Error transforming from reflection coefficients.")
	    if (debug >= 3)
	    {
		Fprintf(stderr, "rc_reps(rc, out_vec, %s, %ld, %g, %g)\n",
			spec_reps[target_rep], order, samp_freq/2.0,
			grid_width);
		pr_farray("out_vec", (int) order, out_vec);
	    }
	}

	put_fea_rec(orec, ohd, ofile);

	if (debug >= 2)
	    Fprintf(stderr, "Wrote a record.\n");
    }

    if (debug)
	Fprintf(stderr, "Reached EOF.\n\n");

    exit(0);
    /*NOTREACHED*/
}


/*
 * for debug printout of floating arrays
 */

void pr_farray(text, n, arr)
    char    *text;
    int	    n;
    float   *arr;
{
    int	    i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}
