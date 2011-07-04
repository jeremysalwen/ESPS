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
|  spectofea.c								|
|									|
|  Convert ESPS SPEC files to FEA_SPEC files.				|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)spectofea.c	1.7	1/6/93	 ESI";  
#endif
char *Version = "1.7";
char *Date = "1/6/93";
char *ProgName = "spectofea";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/spec.h>
#include <esps/fea.h>

#include <esps/feaspec.h>


#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("specofea [-f {BYTE|FLOAT}] [-x debug_level] in.spec out.fspec")

/*
 * ESPS library functions.
 */

char	*get_cmd_line();
char	*eopen();
int	atoi();
char	*uniq_name();

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
    struct spec_data    *inrec;

    char		*outname;
    FILE		*outfile;
    struct header	*outhd;
    struct feaspec	*outrec;


    int			spec_val_format = BYTE;

    long		num_freqs;
    long		i;

    char    	    	**new_names; /* names of header items
				       from init_feaspec_hd() */
    char		**old_names; /* names of input header items */
    char    	    	**gname; /* pointer into new_names list */
    char    	    	*uname;	/* header item name, possibly
				   altered by uniq_name() */
    int	    	    	type;	/* type of header item */
    int	    	    	size;	/* size of header item */
    int	    	    	nitems;	/* dummy arg for genhd_list() */

    /*
     * Process command line options.
     */

    while ((ch = getopt(argc, argv, "x:f:")) != EOF)
        switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'f':
	    spec_val_format = lin_search(type_codes, optarg);
	    REQUIRE(spec_val_format == FLOAT || spec_val_format == BYTE,
		"-f type must be FLOAT or BYTE");
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
	eopen(ProgName, argv[optind++], "r", FT_SPEC, NONE, &inhd, &infile);
    inrec = allo_spec_rec(inhd);

    outname =
	eopen(ProgName, argv[optind++], "w", NONE, NONE, &outhd, &outfile);

    if (debug_level)
	Fprintf(stderr,
	    "Input file: %s\nOutput file: %s \n", inname, outname);

    outhd = new_header(FT_FEA);
    add_source_file(outhd, inname, inhd);
    outhd->common.tag = inhd->common.tag;
    (void) strcpy(outhd->common.prog, ProgName);
    (void) strcpy(outhd->common.vers, Version);
    (void) strcpy(outhd->common.progdate, Date);
    add_comment(outhd, get_cmd_line(argc, argv));
    outhd->variable.refer = inhd->variable.refer;

    spsassert(
	!init_feaspec_hd(outhd, YES,
		inhd->hd.spec->freq_format,
		inhd->hd.spec->spec_type,
		inhd->hd.spec->contin,
		inhd->hd.spec->num_freqs,
		inhd->hd.spec->frame_meth,
		inhd->hd.spec->freqs,
		inhd->hd.spec->sf,
		(long) inhd->hd.spec->frmlen,
		spec_val_format),
	"Error filling FEA_SPEC header." )

    /* Copy generic header items, renaming to avoid conflicts with those
       put in by init_feaspec_hd(). */

    new_names = genhd_list(&nitems, outhd);
    old_names = genhd_list(&nitems, inhd);
    
    if (old_names)
	for (gname = old_names; *gname; gname++)
	{
	    if (genhd_type(*gname, &size, outhd) == HD_UNDEF)
		uname = *gname;
	    else
	    {
		uname = uniq_name(uniq_name(*gname, old_names), new_names);
		(void) fprintf(stderr,
			       "%s: Input file had header item \"%s\".  ",
			       ProgName, *gname);
		(void) fprintf(stderr,
			       "Renaming to \"%s\" to avoid conflict.\n",
			       uname);
	    }
	    type = genhd_type(*gname, &size, inhd);
	    (void) add_genhd(uname, type, size,
			     get_genhd(*gname, inhd),
			     genhd_codes(*gname, inhd),
			     outhd);
	}

    write_header(outhd, outfile);
    outrec = allo_feaspec_rec(outhd, FLOAT);
    num_freqs = inhd->hd.spec->num_freqs;

    /*
     * Main loop.
     */

    while (get_spec_rec(inrec, inhd, infile) != EOF)
    {
	if (inhd->common.tag) *outrec->tag = inrec->tag;

	*outrec->tot_power = inrec->tot_power;

	for (i = 0; i < num_freqs; i++)
	    outrec->re_spec_val[i] = inrec->re_spec_val[i];

	if (inhd->hd.spec->spec_type == SPTYP_CPLX)
	    for (i = 0; i < num_freqs; i++)
		outrec->im_spec_val[i] = inrec->im_spec_val[i];

	if (inhd->hd.spec->freq_format == SPFMT_ARB_VAR)
	{
	    *outrec->n_frqs = inrec->n_frqs;
	    for (i = 0; i < num_freqs; i++)
		outrec->frqs[i] = inrec->frqs[i];
	}

	if (inhd->hd.spec->frame_meth == SPFRM_VARIABLE)
	    *outrec->frame_len = inrec->frame_len;

	put_feaspec_rec(outrec, outhd, outfile);
    }

    exit(0);
    /*NOTREACHED*/
}
