/*----------------------------------------------------------------------+
|									|
|  This material contains proprietary software of Entropic Speech,	|
|  Inc.  Any reproduction, distribution, or publication without the	|
|  prior written permission of Entropic Speech, Inc. is strictly	|
|  prohibited.  Any public distribution of copies of this work		|
|  authorized in writing by Entropic Speech, Inc. must bear the notice	|
|									|
|    "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  featospec.c								|
|									|
|  Convert ESPS FEA_SPEC files to SPEC files.				|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)featospec.c	1.4	1/7/93	 ESI";  
#endif
char *Version = "1.4";
char *Date = "1/7/93";
char *ProgName = "featospec";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/spec.h>


#include <esps/feaspec.h>


#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("featospec [-x debug_level] in.fspec out.spec")

/*
 * ESPS library functions.
 */

char	*get_cmd_line();
char	*eopen();
int	atoi();
int	debug_level = 0;

/*
 * Main program.
 */

main(argc, argv)
    int	    argc;
    char    **argv;
{
    extern int		optind;
    extern char		*optarg;
    int			ch;

    char		*inname;
    FILE		*infile;
    struct header	*inhd;
    struct feaspec	*inrec;

    char		*outname;
    FILE		*outfile;
    struct header	*outhd;
    struct spec_data	*outrec;


    short		freq_format;
    short		spec_type;
    long		num_freqs;
    short		frame_meth;
    long		i;

				/* Null-terminated list of generic header
				   items  not  to be copied to output file */
    static char	    	*spec_names[] =
    	    	    	  { "freq_format",  "spec_type",    "contin",
			    "num_freqs",    "frame_meth",   NULL,
			    NULL,   	    NULL,   	    NULL };
    int	    	    	num_spec_names = 5;
    char    	    	**gname; /* pointer into list of generic header
				    items in input file */
    int	    	    	type;	/* type of header item */
    int	    	    	size;	/* size of header item */
    int	    	    	nitems;	/* dummy arg for genhd_list() */

    /*
     * Process command line options.
     */

    while ((ch = getopt(argc, argv, "x:")) != EOF)
        switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	default:
	    SYNTAX
	    ;
	    break;
	}

    /*
     * process file arguments
     */

    if (argc - optind < 2)
    {
	Fprintf(stderr, "%s: Too few file names.\n", ProgName);
	SYNTAX
	;
    }
    if (argc - optind > 2)
    {
	Fprintf(stderr, "%s: Too many file names.\n", ProgName);
	SYNTAX
	;
    }

    inname =
	eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_SPEC, &inhd, &infile);
    inrec = allo_feaspec_rec(inhd, FLOAT);

    outname =
	eopen(ProgName, argv[optind++], "w", NONE, NONE, &outhd, &outfile);

    if (debug_level)
	Fprintf(stderr,
	    "Input file: %s\nOutput file: %s \n", inname, outname);

    outhd = new_header(FT_SPEC);
    add_source_file(outhd, inname, inhd);
    outhd->common.tag = inhd->common.tag;
    (void) strcpy(outhd->common.prog, ProgName);
    (void) strcpy(outhd->common.vers, Version);
    (void) strcpy(outhd->common.progdate, Date);
    add_comment(outhd, get_cmd_line(argc, argv));
    outhd->variable.refer = inhd->variable.refer;

    REQUIRE(genhd_type("freq_format", (int *) NULL, inhd) == CODED,
	    "Header item \"freq_format\" undefined or wrong type.")
    freq_format =
	outhd->hd.spec->freq_format = *get_genhd_s("freq_format", inhd);

    REQUIRE(genhd_type("spec_type", (int *) NULL, inhd) == CODED,
	    "Header item \"spec_type\" undefined or wrong type.")
    spec_type =
	outhd->hd.spec->spec_type = *get_genhd_s("spec_type", inhd);

    REQUIRE(genhd_type("contin", (int *) NULL, inhd) == CODED,
	    "Header item \"contin\" undefined or wrong type.")
    outhd->hd.spec->contin = *get_genhd_s("contin", inhd);

    REQUIRE(genhd_type("num_freqs", (int *) NULL, inhd) == LONG,
	    "Header item \"num_freqs\" undefined or wrong type.")
    num_freqs =
	outhd->hd.spec->num_freqs = *get_genhd_l("num_freqs", inhd);

    REQUIRE(genhd_type("frame_meth", (int *) NULL, inhd) == CODED,
	    "Header item \"frame_meth\" undefined or wrong type.")
    frame_meth =
	outhd->hd.spec->frame_meth = *get_genhd_s("frame_meth", inhd);

    REQUIRE(freq_format != SPFMT_ARB_FIXED,
	    "SPEC file frequency format ARB_FIXED not implemented.")

    switch (freq_format)
    {
    case SPFMT_SYM_CTR:
    case SPFMT_SYM_EDGE:
    case SPFMT_ASYM_CTR:
    case SPFMT_ASYM_EDGE:
	REQUIRE(genhd_type("sf", (int *) NULL, inhd) == FLOAT,
		"Header item \"sf\" undefined or wrong type.")
	outhd->hd.spec->sf = *get_genhd_f("sf", inhd);
	spec_names[num_spec_names++] = "sf";
	break;
    case SPFMT_ARB_FIXED:
    case SPFMT_ARB_VAR:
    default:
	break;
    }

    if (frame_meth == SPFRM_FIXED)
    {
	REQUIRE(genhd_type("frmlen", (int *) NULL, inhd) == LONG,
		"Header item \"frmlen\" undefined or wrong type.")
	outhd->hd.spec->frmlen = *get_genhd_l("frmlen", inhd);
	spec_names[num_spec_names++] = "frmlen";
    }

    /* Copy generic header items that were not used to fill items in
       outhd->hd.spec */

    for (gname = genhd_list(&nitems, inhd); *gname; gname++)
	if (lin_search2(spec_names, *gname) == -1)
	{
	    type = genhd_type(*gname, &size, inhd);
	    add_genhd(*gname, type, size,
		      get_genhd(*gname, inhd),
		      genhd_codes(*gname, inhd),
		      outhd);
	}

    write_header(outhd, outfile);
    outrec = allo_spec_rec(outhd);

    /*
     * Main loop.
     */

    while (get_feaspec_rec(inrec, inhd, infile) != EOF)
    {
	if (inhd->common.tag) outrec->tag = *inrec->tag;

	if (inrec->tot_power)
	    outrec->tot_power = *inrec->tot_power;

	for (i = 0; i < num_freqs; i++)
	    outrec->re_spec_val[i] = inrec->re_spec_val[i];

	if (spec_type == SPTYP_CPLX)
	    for (i = 0; i < num_freqs; i++)
		outrec->im_spec_val[i] = inrec->im_spec_val[i];

	if (freq_format == SPFMT_ARB_VAR)
	{
	    outrec->n_frqs = *inrec->n_frqs;
	    for (i = 0; i < num_freqs; i++)
		outrec->frqs[i] = inrec->frqs[i];
	}

	if (frame_meth == SPFRM_VARIABLE)
	    outrec->frame_len = *inrec->frame_len;

	put_spec_rec(outrec, outhd, outfile);
    }

    exit(0);
    /*NOTREACHED*/
}
