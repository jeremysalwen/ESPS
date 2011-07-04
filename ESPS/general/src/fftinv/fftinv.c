/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *   "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: fftinv.c	
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This program computes inverse FFTs from FEA_SPEC files to SD files
 */

#ifndef lint
static char *sccs_id = "@(#)fftinv.c	1.13	8/31/95 ESI";  
#endif
char *Version = "1.13";
char *Date = "8/31/95";
char *ProgName = "fftinv";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/feasd.h>
#include <esps/feaspec.h>
#include <esps/fea.h>

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
 USAGE("fftinv [-r range] [-t output_type] [-x debug_level] specfile sdfile")

/*
 Globals
*/
int	    debug_level = 0;		/* debug level */

/*
 * system functions
 */
double  log(), sqrt();
/*
 * ESPS library functions
 */
void	set_sd_type();
char	*eopen();
char	*get_cmd_line();
void	lrange_switch(), get_fft_inv();
void    update_waves_gen();
struct feasd  *allo_feasd_recs();
int	is_type_complex();
/*
 * local functions
 */
static int is_type_numeric();

/*
 * main program
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    FILE    *specstrm = stdin,	/* input and output file streams */
            *sdstrm = stdout;
    struct header  		/* input and output file headers */
            *ih, *oh;
    char    *specfile, 		/* input and output file names */
	    *sdfile;
	    
    struct feaspec  *spec_rec;   /* record for spectral data */
    struct feasd    *sd_rec;     /* record for feasd data */

    extern int
	    optind;
    extern char
            *optarg;
    int	    c;
    long    srec, erec;		/* start and end records */
    double  log2 = log(2.0);	/* needed for base 2 logs */
    char    *range = NULL;	/* string for range specification (-r) */
    int	    r_flag = 0;		/* flag for range option */
    float   *x, *y;		/* arrays for get_fftinv */
    float   scale;		/* scale factor from continuous to discrete */
    long    nan, hf, j;
    long    length;		/* transform length */
    int	    order;

    float   *fdata;		/* pointer to real data in feasd record */
    float_cplx
	    *cdata;		/* pointer to complex data in feasd record */
    long    *frame_len;		/* pointer to frame length */
    double  record_freq;	/* record_freq header item in input */
    double  sf;			/* sample frequency in original sampled data */
    double  start_time;         /* start time of beginning of first frame */
    int	    t_flag = NO;	/* -t option specified? */
    char    *output_type;	/* name of output data type */
    int	    type = FLOAT_CPLX;	/* type code for output data type */
    int	    real = NO;          /* YES for real inverse Xform; NO for complex */

    /*
     * process command line options
     */
    while ((c = getopt(argc, argv, "r:t:x:")) != EOF)
        switch (c)    
        {
	case 'x':
	    debug_level = atoi(optarg);
	    break;
        case 'r': 
            range = optarg;
	    r_flag++;
            break;
	case 't':
	    output_type = optarg;
	    t_flag++;
	    break;
        default: 
            SYNTAX;
            break;
        }

    srec = 1;
    erec = LONG_MAX;
    if (r_flag) 
	lrange_switch (range, &srec, &erec, 1);	

    if (srec < 1)
	ERROR_EXIT("can't have negative starting point");
    if (srec > erec)
	ERROR_EXIT("range must specify at least one record");
    nan = erec - srec + 1;
    if (debug_level) 
	Fprintf(stderr, "fftinv: range is %ld - %ld\n", srec, erec);

    /*
     * process file arguments - first, input file
     */

    if (optind < argc)
	specfile = 
	   eopen("fftinv", argv[optind++], "r", FT_FEA, FEA_SPEC, &ih, &specstrm);
    else {
	Fprintf(stderr, "fft: no input FEA_SPEC file specified.\n");
	SYNTAX;
    }    
    if (debug_level)
	Fprintf(stderr, "Input file is %s\n", specfile);

    if (t_flag)
    {
	type = lin_search(type_codes, output_type);
	if (!is_type_numeric(type))
	{
	    Fprintf(stderr,
	    	    "fftinv: invalid output type %s\n", output_type);
	    exit(1);
	}
    }
    else if (genhd_type("input_data", (int *) NULL, ih) == CHAR
	     && strcmp(get_genhd_c("input_data", ih), "real") == 0)
	type = FLOAT;
    else
	type = FLOAT_CPLX;

    /*
     * allocate spectral record
     */

    spec_rec = allo_feaspec_rec(ih, FLOAT); 

    /*
     * set up output file
     */

    if (optind < argc)
	sdfile = 
	    eopen("fftinv", argv[optind++], "w", NONE, NONE, &oh, &sdstrm);
    else {
	Fprintf(stderr, "fftinv: no output SD file specified.\n");
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "Output file is %s\n", sdfile);

    REQUIRE(genhd_type("spec_type", (int *) NULL, ih) == CODED,
           "Header item \"spec_type\" undefined or wrong type");
    if (*get_genhd_s("spec_type", ih) != SPTYP_CPLX) 
	ERROR_EXIT("FEA_SPEC file spec_type is not complex (SPTYP_CPLX)");

    REQUIRE(genhd_type("freq_format", (int *) NULL, ih) == CODED,
           "Header item \"spec_type\" undefined or wrong type");
    if (*get_genhd_s("freq_format", ih) != SPFMT_ASYM_EDGE) 
	ERROR_EXIT("FEA_SPEC file freq_format is not SPTYP_ASYM_EDGE");

    length = *get_genhd_l("num_freqs", ih) - 1;
    hf = length / 2;
    order = ROUND(log((double) length) / log2);
    if (debug_level) 
	Fprintf(stderr, 
	    "fftinv: transform length = %ld, order = %ld, num_freqs = %ld\n", 
	    length, order, *get_genhd_l("num_freqs", ih));

    if (genhd_type("fft_length", (int *) NULL, ih) != HD_UNDEF)
    {
	if (get_genhd_val("fft_length", ih, -1.0) != (double) length)
	    Fprintf(stderr,
		"%s: fft_length + 1 not equal to num_freqs.\n",
		ProgName);
    }

    x = (float *) calloc((unsigned) length, sizeof(float));
    y = (float *) calloc((unsigned) length, sizeof(float));
    spsassert(x != NULL && y != NULL, 
	"fftinv: couldn't allocate enough memory");

    /*
     * create and fill in FEA_SD file header
     */

    oh = new_header(FT_FEA);
    add_source_file(oh, specfile, ih);
    add_comment(oh, get_cmd_line(argc, argv));
    oh->common.tag = NO;
    oh->variable.refer = ih->variable.refer;
    (void) strcpy(oh->common.prog, ProgName);
    (void) strcpy(oh->common.vers, Version);
    (void) strcpy(oh->common.progdate, Date);
    *add_genhd_l("inv_fft_length", (long *) NULL, 1, oh) = length;

    /*
     * compute frequency and start_time generics
     */

    sf = get_genhd_val("sf", ih, 8000.0);

    REQUIRE(genhd_type("frame_meth", (int *) NULL, ih) == CODED,
	    "Header item \"frame_meth\" undefined or wrong type");
    switch (*get_genhd_s("frame_meth", ih))
    {
    case SPFRM_FIXED:
    case SPFRM_VARIABLE:
	frame_len = spec_rec->frame_len;
	break;
    case SPFRM_NONE:
	/* Frame length not available.
	   Use transform length for lack of anything better. */
	frame_len = &length;
	break;
    default:
	REQUIRE(NO, "Unrecognized value of \"frame_meth\" in input header");
	break;
    }

    /* read record for length of first frame */

    REQUIRE(get_feaspec_rec(spec_rec, ih, specstrm) != EOF,
	    "Can't get first record of input file");

    start_time = 
	get_genhd_val("start_time", ih, 0.0)
	- (double) (*frame_len)/(2*sf);

    record_freq = get_genhd_val("record_freq", ih, 0.0);
    if (record_freq != 0.0)
	start_time += (srec - 1)/record_freq;
    
    /* move to starting position */

    if (srec > 1)
    {
	if (srec > 2)
	    fea_skiprec(specstrm, srec - 2, ih);
	REQUIRE(get_feaspec_rec(spec_rec, ih, specstrm) != EOF,
		"Can't get first input record of specified range");
    }
    nan--;

    /*
     * initialize FEA_SD header, allocate data structures,
     * and write output file header
     */

    real = !is_type_complex(type);

    REQUIRE(init_feasd_hd(oh, type, 1, &start_time, NO, sf) == 0,
	    "Couldn't initialize output FEA_SD header");
    sd_rec = allo_feasd_recs(oh, type, length, (char *) NULL, NO);
    REQUIRE(sd_rec != NULL, "Couldn't allocate FEA_SD data record");

    if (type == FLOAT_CPLX)
	cdata = (float_cplx *) sd_rec->data;
    else if (!real)
	cdata = (float_cplx *) malloc(length * sizeof(float_cplx));
	
    write_header(oh, sdstrm);

    /*
     * main loop
     */

    do
    {
	long	frmlen = *frame_len;

	if (debug_level) Fprintf(stderr, "fftinv: frame %ld\n", srec++);
	if (debug_level > 1) {
	    Fprintf(stderr, 
		"fftinv: real and imag components in spectral record:\n");
	    for (j = 0; j < length + 1; j++) 
		Fprintf(stderr, "%f\t%f\n", 
		    spec_rec->re_spec_val[j], spec_rec->im_spec_val[j]);
	}	
	/*
	 * compute factor to get from continuous density to discrete
	 * spectral distribution
	 */

	scale = sqrt(frmlen * sf);

	/*
	 * reorder spectral components to get_fftinv order
	 * remember that ASYM_EDGE stores frequencies (-hf, ..., 0, ..., hf),
	 * hf = length / 2
	 */
	for (j = 0; j < length; j++) {
	    if (j < hf) {
		    x[j] = scale * spec_rec->re_spec_val[j + hf];
		    y[j] = scale * spec_rec->im_spec_val[j + hf];
		}
	    else {
		    x[j] = scale * spec_rec->re_spec_val[j - hf];
		    y[j] = scale * spec_rec->im_spec_val[j - hf];
	    }
	}
	if (debug_level > 1) {
	    Fprintf(stderr, 
		"fftinv: real and imag spectral components input to get_fftinv:\n");
	    for (j = 0; j < length; j++) 
		Fprintf(stderr, "%f\t%f\n", x[j], y[j]);
	}	

	/* do inverse fft */
	get_fft_inv(x, y, order);

	if (debug_level > 1) {
	    Fprintf(stderr, "fftinv: sampled data output from get_fft_inv:\n");
	    for (j = 0; j < length; j++) 
		Fprintf(stderr, "%f\n", x[j]);
	}

	/* pack the data into the output record */

	if (real)
	    type_convert(frmlen, (char *) x, FLOAT,
			 (char *) sd_rec->data, type, (void (*)()) NULL);
	else
	{
	    for(j = 0; j < frmlen; j++)
	    {
		cdata[j].real = x[j];	  
		cdata[j].imag = y[j];
	    }
	    if (type != FLOAT_CPLX)
		type_convert(frmlen, (char *) cdata, FLOAT_CPLX,
			     (char *) sd_rec->data, type, (void (*)()) NULL);
	}

	REQUIRE((put_feasd_recs(sd_rec, 0L, frmlen, oh, sdstrm) == 0),
	 "Couldn't write output record");

    } while (nan-- && get_feaspec_rec(spec_rec, ih, specstrm) != EOF);

    exit(0);
    /*NOTREACHED*/
}


static int
is_type_numeric(type)
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
    default:
        return NO;
    }
}
