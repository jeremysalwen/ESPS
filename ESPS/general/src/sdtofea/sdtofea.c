/*----------------------------------------------------------------------+
|									|
|  This material contains proprietary software of Entropic Speech,	|
|  Inc.  Any reproduction, distribution, or publication without the	|
|  prior written permission of Entropic Speech, Inc. is strictly	|
|  prohibited.  Any public distribution of copies of this work		|
|  authorized in writing by Entropic Speech, Inc. must bear the notice	|
|									|
|    "Copyright (c) 1989 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  sdtofea.c								|
|									|
|  Convert ESPS SD files to FEA_SD files.				|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)sdtofea.c	1.2	1/6/93	ESI";  
#endif
char *Version = "1.2";
char *Date = "1/6/93";
char *ProgName = "sdtofea";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/fea.h>

#include <esps/feasd.h>


#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("sdofea [-x debug_level] in.sd out.fea")

#define BUF_SIZE    8192

/*
 * ESPS library functions.
 */ 

char	*get_cmd_line();
char	*eopen();
int	atoi();
char	*arr_alloc();
char	*uniq_name();
void	add_genzfunc();
int 	typesiz();

long	miio_get();
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
    struct sd_header	*sd_hd;

    char		*data;

    char		*outname;
    FILE		*outfile;
    struct header	*outhd;
    struct feasd	*outrec;


    int			data_type;	/* code for type of sampled data */
    int			num_channels;	/* number of channels */
    double		start_time;
    double		record_freq;	/* sampling frequency */

    char    	    	**new_names;	/* names of header items
					   from init_feasd_hd() */
    char		**old_names;	/* names of input header items */
    char    	    	**gname;	/* pointer into new_names list */
    char    	    	*uname;		/* header item name, possibly
					   altered by uniq_name() */
    int	    	    	type;		/* type of header item */
    int	    	    	size;		/* size of header item */
    int	    	    	nitems;		/* dummy arg for genhd_list() */

    long		num_records;	/* number of records to allocate */
    long		num_read;	/* number of samples read */

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
	eopen(ProgName, argv[optind++], "r", FT_SD, NONE, &inhd, &infile);

    outname =
	eopen(ProgName, argv[optind++], "w", NONE, NONE, &outhd, &outfile);

    if (debug_level)
	Fprintf(stderr,
	    "Input file: %s\nOutput file: %s \n", inname, outname);

    outhd = new_header(FT_FEA);
    add_source_file(outhd, inname, inhd);
    (void) strcpy(outhd->common.prog, ProgName);
    (void) strcpy(outhd->common.vers, Version);
    (void) strcpy(outhd->common.progdate, Date);
    add_comment(outhd, get_cmd_line(argc, argv));
    outhd->variable.refer = inhd->variable.refer;

    data_type = get_sd_type(inhd);
    if (data_type == CHAR) data_type = BYTE;

    sd_hd = inhd->hd.sd;
    num_channels = sd_hd->nchan;
    if (num_channels == 0) num_channels = 1;

    start_time = get_genhd_val("start_time", inhd, 0.0);
    record_freq = sd_hd->sf;

    spsassert(
	!init_feasd_hd(outhd, data_type,
		       num_channels, &start_time, NO, record_freq),
	"Error filling FEA_SD header." )

    if (sd_hd->equip != NONE)
	(void) add_genhd_e("equip", &sd_hd->equip, 1, equip_codes, outhd);
    if (sd_hd->max_value != 0.0)
	(void) add_genhd_f("max_value", &sd_hd->max_value, 1, outhd);
    if (sd_hd->src_sf != 0.0)
	(void) add_genhd_f("src_sf", &sd_hd->src_sf, 1, outhd);
    if (sd_hd->synt_method != NONE)
	(void) add_genhd_e("synt_method",
			   &sd_hd->synt_method, 1, synt_methods, outhd);
    if (sd_hd->scale != 0.0)
	(void) add_genhd_f("scale", &sd_hd->scale, 1, outhd);
    if (sd_hd->dcrem != 0.0)
	(void) add_genhd_f("dcrem", &sd_hd->dcrem, 1, outhd);
    if (sd_hd->q_method != NONE)
	(void) add_genhd_e("q_method",
			   &sd_hd->q_method, 1, quant_methods, outhd);
    if (sd_hd->v_excit_method != NONE)
	(void) add_genhd_e("v_excit_method",
			   &sd_hd->v_excit_method, 1, excit_methods, outhd);
    if (sd_hd->uv_excit_method != NONE)
	(void) add_genhd_e("uv_excit_method",
			   &sd_hd->uv_excit_method, 1, excit_methods, outhd);
    if (sd_hd->synt_interp != NONE)
	(void) add_genhd_e("synt_interp",
			   &sd_hd->synt_interp, 1, synt_inter_methods, outhd);
    if (sd_hd->synt_pwr != NONE)
	(void) add_genhd_e("synt_pwr",
			   &sd_hd->synt_pwr, 1, synt_pwr_codes, outhd);
    if (sd_hd->synt_rc != NONE)
	(void) add_genhd_e("synt_rc",
			   &sd_hd->synt_rc, 1, synt_ref_methods, outhd);
    if (sd_hd->synt_order != 0)
	(void) add_genhd_s("synt_order", &sd_hd->synt_order, 1, outhd);
    if (sd_hd->start != 0)
	(void) add_genhd_l("start", &sd_hd->start, 1, outhd);
    if (sd_hd->nan != 0)
	(void) add_genhd_l("nan", &sd_hd->nan, 1, outhd);

    if (sd_hd->prefilter)
	(void) add_genzfunc("prefilter", sd_hd->prefilter, outhd);

    if (sd_hd->de_emp)
	(void) add_genzfunc("de_emp", sd_hd->de_emp, outhd);

    /* Copy generic header items, renaming to avoid conflicts with those
       put in by init_feasd_hd(). */

    new_names = genhd_list(&nitems, outhd);
    old_names = genhd_list(&nitems, inhd);
    
    if (old_names)
	for (gname = old_names; *gname; gname++)
	    if (strcmp(*gname, "start_time"))
	    {
		if (genhd_type(*gname, &size, outhd) == HD_UNDEF)
		    uname = *gname;
		else
		{
		    uname =
			uniq_name(uniq_name(*gname, old_names), new_names);

		    Fprintf(stderr,
			    "%s: Input file had header item \"%s\".  ",
			    ProgName, *gname);
		    Fprintf(stderr,
			    "Renaming to \"%s\" to avoid conflict.\n",
			    uname);
		}
		type = genhd_type(*gname, &size, inhd);
		(void) add_genhd(uname, type, size,
				 get_genhd(*gname, inhd),
				 genhd_codes(*gname, inhd),
				 outhd);
		if (debug_level)
		    Fprintf(stderr, "Adding header item \"%s\".\n", uname);
	    }

    write_header(outhd, outfile);

    num_records = BUF_SIZE / (num_channels * typesiz(data_type));
    if (num_records == 0) num_records = 1;

    
    outrec = allo_feasd_recs(outhd, data_type, num_records, (char *) NULL, NO);
    data = outrec->data;

    /*
     * Main loop.
     */

    while ((num_read =
	    miio_get(data_type, data, (int) (num_records * num_channels),
		     inhd->common.edr, inhd->common.machine_code, infile))


	   > 0)
    {
	if (num_read % num_channels != 0)
	    Fprintf(stderr,
		    "%s: Warning--partial record read.\n", ProgName);
		
	REQUIRE(put_feasd_recs(outrec, 0,
			       num_read/num_channels, outhd, outfile) == 0,
		"error writing output");
    }

    exit(0);
    /*NOTREACHED*/
}
