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
|  featosd.c								|
|									|
|  Convert ESPS FEA_SD files to SD files.				|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)featosd.c	1.3	1/7/93	ESI";  
#endif
char *Version = "1.3";
char *Date = "1/7/93";
char *ProgName = "featosd";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>

#include <esps/feasd.h>

#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("featosd [-x debug_level] in.feasd out.sd")

#define BUF_SIZE    8192

/*
 * ESPS library functions.
 */

char	    	*get_cmd_line();
char	    	*eopen();
int	    	atoi();
struct zfunc	*get_genzfunc();

long	    	miio_put();
int		debug_level = 0;

/*
 * Main program.
 */

main(argc, argv)
    int	    argc;
    char    **argv;
{
    extern int	    	optind;
    extern char		*optarg;
    int			ch;

    char		*inname;
    FILE		*infile;
    struct header	*inhd;
    struct feasd	*inrec;
    int			in_type; /* code for type of input data */
    int			complex; /* input data type complex? */

    double  	    	*start_time;
				/* pointer to input header item start_time */
    double  	    	*record_freq;
				/* pointer to input header item record_freq */

    char		*outname;
    FILE		*outfile;
    struct header	*outhd;
    char    	    	*outrec;
    struct sd_header	*sd_hd;
    int			out_type; /* code for type of output data */


    int			num_channels;	/* number of channels */
    long    	    	num_records;
    long    	    	num_read;

				/* Null-terminated list of generic header
				   items that get special treatment */
    static char	    	*spec_names[] =
		  { "record_freq",	"equip",	    "max_value",
		    "src_sf",		"synt_method",	    "scale",
		    "dcrem",		"q_method",	    "v_excit_method",
		    "uv_excit_method",	"synt_interp",	    "synt_pwr",
		    "synt_rc",		"synt_order",	    "start",
		    "nan",		"prefilter_siz",    "prefilter_zeros",
		    "prefilter_poles",	"de_emp_siz",	    "de_emp_zeros",
		    "de_emp_poles",	NULL };

    char    	    	**gname; /* pointer into list of generic header
				    items in input file */
    int	    	    	type;	/* type of header item */
    int	    	    	size;	/* size of header item */
    int	    	    	nitems;	/* dummy arg for genhd_list() */
    long    	    	i;	/* loop index */

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
	eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_SD, &inhd, &infile);

    start_time = get_genhd_d("start_time", inhd);
    REQUIRE(start_time, "no header item \"start_time\" in input file");
    record_freq = get_genhd_d("record_freq", inhd);
    REQUIRE(record_freq, "no header item \"record_freq\" in input file");

    in_type = get_fea_type("samples", inhd);
    switch (in_type)
    {
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
	out_type = in_type;
	complex = NO;
	break;
    case DOUBLE_CPLX:
	out_type = DOUBLE;
	complex = YES;
	break;
    case FLOAT_CPLX:
	out_type = FLOAT;
	complex = YES;
	break;
    case LONG_CPLX:
	out_type = LONG;
	complex = YES;
	break;
    case SHORT_CPLX:
	out_type = SHORT;
	complex = YES;
	break;
    case BYTE_CPLX:
	out_type = BYTE;
	complex = YES;
	break;
    default:
	Fprintf(stderr, "%s: bad type code (%d) in input file header.\n",
		ProgName, in_type);
	exit(1);
	break;
    }
	
    outname =
	eopen(ProgName, argv[optind++], "w", NONE, NONE, &outhd, &outfile);

    if (debug_level)
	Fprintf(stderr,
	    "Input file: %s\nOutput file: %s \n", inname, outname);

    outhd = new_header(FT_SD);
    set_sd_type(outhd, out_type);
    add_source_file(outhd, inname, inhd);
    (void) strcpy(outhd->common.prog, ProgName);
    (void) strcpy(outhd->common.vers, Version);
    (void) strcpy(outhd->common.progdate, Date);
    add_comment(outhd, get_cmd_line(argc, argv));
    outhd->variable.refer = inhd->variable.refer;
    

    sd_hd = outhd->hd.sd;
    num_channels = get_fea_siz("samples", inhd,
				    (short *) NULL, (long **) NULL);
    sd_hd->nchan = complex ? 2*num_channels : num_channels;

    if (debug_level)
	Fprintf(stderr, "Processing generic header items.\n");

    sd_hd->sf = *record_freq;
    if (genhd_type("equip", &size, inhd) != HD_UNDEF)
	sd_hd->equip = *get_genhd_s("equip", inhd);
    if (genhd_type("max_value", &size, inhd) != HD_UNDEF)
    {
	if (size == 1)
	    sd_hd->max_value = get_genhd_val("max_value", inhd, 0.0);
	else
	{
	    double  *max_val = get_genhd_d("max_value", inhd);

	    sd_hd->max_value = 0;
	    for (i = 0; i < size; i++)
		if (sd_hd->max_value < max_val[i])
		    sd_hd->max_value = max_val[i];
	    Fprintf(stderr,
		    "%s: max_value is vector; using largest element.\n",
		    ProgName);
	    (void) copy_genhd(outhd, inhd, "max_value");
	}
    }
    if (genhd_type("src_sf", &size, inhd) != HD_UNDEF)
	sd_hd->src_sf = get_genhd_val("src_sf", inhd, 0.0);
    if (genhd_type("synt_method", &size, inhd) != HD_UNDEF)
	sd_hd->synt_method = *get_genhd_s("synt_method", inhd);
    if (genhd_type("scale", &size, inhd) != HD_UNDEF)
	sd_hd->scale = get_genhd_val("scale", inhd, 0.0);
    if (genhd_type("dcrem", &size, inhd) != HD_UNDEF)
	sd_hd->dcrem = get_genhd_val("dcrem", inhd, 0.0);
    if (genhd_type("q_method", &size, inhd) != HD_UNDEF)
	sd_hd->q_method = *get_genhd_s("q_method", inhd);
    if (genhd_type("v_excit_method", &size, inhd) != HD_UNDEF)
	sd_hd->v_excit_method = *get_genhd_s("v_excit_method", inhd);
    if (genhd_type("uv_excit_method", &size, inhd) != HD_UNDEF)
	sd_hd->uv_excit_method = *get_genhd_s("uv_excit_method", inhd);
    if (genhd_type("synt_interp", &size, inhd) != HD_UNDEF)
	sd_hd->synt_interp = *get_genhd_s("synt_interp", inhd);
    if (genhd_type("synt_pwr", &size, inhd) != HD_UNDEF)
	sd_hd->synt_pwr = *get_genhd_s("synt_pwr", inhd);
    if (genhd_type("synt_rc", &size, inhd) != HD_UNDEF)
	sd_hd->synt_rc = *get_genhd_s("synt_rc", inhd);
    if (genhd_type("synt_order", &size, inhd) != HD_UNDEF)
	sd_hd->synt_order = get_genhd_val("synt_order", inhd, 0.0);
    if (genhd_type("start", &size, inhd) != HD_UNDEF)
	sd_hd->start = get_genhd_val("start", inhd, 0.0);
    if (genhd_type("nan", &size, inhd) != HD_UNDEF)
	sd_hd->nan = get_genhd_val("nan", inhd, 0.0);

    if (genhd_type("prefilter_siz", &size, inhd) != HD_UNDEF
	&& genhd_type("prefilter_zeros", &size, inhd) != HD_UNDEF
	&& genhd_type("prefilter_poles", &size, inhd) != HD_UNDEF)
    {
	sd_hd->prefilter = get_genzfunc("prefilter", inhd);
    }
    else
    {
	if (genhd_type("prefilter_siz", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "prefilter_siz");
	if (genhd_type("prefilter_poles", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "prefilter_poles");
	if (genhd_type("prefilter_zeros", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "prefilter_zeros");
    }

    if (genhd_type("de_emp_siz", &size, inhd) != HD_UNDEF
	&& genhd_type("de_emp_zeros", &size, inhd) != HD_UNDEF
	&& genhd_type("de_emp_poles", &size, inhd) != HD_UNDEF)
    {
	sd_hd->de_emp = get_genzfunc("de_emp", inhd);
    }
    else
    {
	if (genhd_type("de_emp_siz", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "de_emp_siz");
	if (genhd_type("de_emp_poles", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "de_emp_poles");
	if (genhd_type("de_emp_zeros", &size, inhd) != HD_UNDEF)
	    (void) copy_genhd(outhd, inhd, "de_emp_zeros");
    }

    for (gname = genhd_list(&nitems, inhd); *gname; gname++)
	if (lin_search2(spec_names, *gname) == -1)
	{
	    type = genhd_type(*gname, &size, inhd);
	    (void) add_genhd(*gname, type, size,
		      get_genhd(*gname, inhd),
		      genhd_codes(*gname, inhd),
		      outhd);
	}

    if (debug_level)
	Fprintf(stderr, "Writing header.\n");

    write_header(outhd, outfile);

    num_records = BUF_SIZE / (num_channels*typesiz(in_type));
    if (num_records < 1) num_records = 1;

    if (debug_level)
	Fprintf(stderr,
		"Allocating space for %ld output records.\n", num_records);

    inrec = allo_feasd_recs(inhd, in_type, num_records, (char *) NULL, NO);
    if (complex)
    {
	if (debug_level)
	    Fprintf(stderr, "Allocating output buffer.\n");

	outrec = calloc((unsigned) num_records,
			(unsigned) (num_channels*2*typesiz(out_type)));
	spsassert(outrec, "can't allocate space for output buffer");
    }
    else
	outrec = inrec->data;

    /*
     * Main loop.
     */

    while ((num_read = get_feasd_recs(inrec, 0, num_records, inhd, infile))
	   > 0)
    {

	if (debug_level >= 2)
	    Fprintf(stderr, "Read %ld records.\n", num_read);

#define	CASE(type, c_type) \
		{ \
		    c_type	*inbuf = (c_type *) inrec->data; \
		    type	*outbuf = (type *) outrec; \
		    \
		    for (i = 0; i < num_read * num_channels; i++) \
		    { \
			*outbuf++ = inbuf[i].real; \
			*outbuf++ = inbuf[i].imag; \
		    } \
		} \
		break;
		    
	if (complex)
	    switch (in_type)
	    {
	    case DOUBLE_CPLX:	CASE(double, double_cplx)
	    case FLOAT_CPLX:	CASE(float, float_cplx)
	    case LONG_CPLX: 	CASE(long, long_cplx)
	    case SHORT_CPLX:	CASE(short, short_cplx)
	    case BYTE_CPLX: 	CASE(char, byte_cplx)
	    }

#undef	CASE

	REQUIRE(miio_put(out_type, outrec,
			 (int) (num_read * (complex ? 2 : 1) * num_channels),
			 outhd->common.edr, outfile),
		"error writing output");

	if (debug_level >= 2)
	    Fprintf(stderr, "Wrote output records.\n");
    }

    exit(0);
    /*NOTREACHED*/
}
