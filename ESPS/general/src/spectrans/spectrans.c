 /*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright (c) 1990 Entropic Speech, Inc."
 *
 * Program: spectrans	
 *
 * Written by:  David Burton
 * Checked by: John Shore
 *
 * spectrans -- transform spectreal coefficients in a FEA_ANA file
 */

#ifndef lint
static char *sccs_id = "@(#)spectrans.c	3.7 3/5/90 ESI";
#endif

/*
 * system include files
 */

#include <stdio.h>

/*
 * sps include files
*/
 
#include <esps/esps.h> 
#include <esps/fea.h>
#include <esps/anafea.h>
#include <esps/ftypes.h>
#include <esps/unix.h>
/*
 * defines
 */

#define ERROR_EXIT(text) {(void) fprintf(stderr, "spectrans: %s - exiting\n", text); exit(1);}
#define SYNTAX USAGE ("spectrans -x debug_level -m spec_rep input.fea output.fea")

/*
 * external SPS functions
 */
char *eopen();
char *get_cmd_line();
void write_header();
struct header *read_header();
struct anafea *allo_anafea_rec();
int get_anafea_rec();
void put_anafea_rec();

/*
 * global variable declarations
 */

extern optind;
extern char *optarg;

int		    debug_level = 0;
long		    order_v = 0,  order_uv = 0; /*voiced and 
							unvoiced order*/
int		    maxorder = 0;   	/* max of order_v and order_uv*/
float		    *input, *output, *rc;/*arrays containing parameters*/

/*
 * main program
*/

main (argc, argv)
int argc;
char **argv;
{
/*
 * setup and initialization
 */
    char *version = "3.7";
    char *date = "3/5/90";
    char	    *out_fea = NULL;	    /*output FEA_ANA file*/
    FILE	    *outfea_strm = stdout;
    struct header   *fea_oh = NULL;
    char	    *in_fea = NULL;	    /*input FEA_ANA file*/
    struct header   *fea_ih = NULL;
    FILE	    *infea_strm = stdin;
    int		    c;			    /*for getopt return*/
    struct anafea	*anafea_rec = NULL;
    char		    *cmd_line;		/*string for command line*/
    float bwidth;
    double frq_res = 4.;
    
    void trnsfrm_spec();
    int rc_reps(), reps_rc();

    int target_rep,  input_rep; /*output and input 
				spectral representations*/
    char *spec_name; /* holds name of desired spectral parameteres */
    cmd_line = get_cmd_line(argc, argv); /*store copy of command line*/
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:m:w:")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'w':
		frq_res = atof(optarg);
		break;
	    case 'm':
		spec_name = optarg;
		break;
	    default:
		SYNTAX;
	}
    }
/*
 * process file arguments
 */
    if (optind < argc) {
	in_fea = eopen("spectrans", argv[optind++], "r", FT_FEA, FEA_ANA, 
			&fea_ih, &infea_strm);
    }
    else {
	Fprintf(stderr, "spectrans: no input file specified.\n");
	SYNTAX;
        }
    if (optind < argc) {
	out_fea = eopen("spectrans", argv[optind++], "w", NONE, NONE, 
		    (struct header **)NULL, &outfea_strm);
	}
    else {
	Fprintf(stderr, "spectrans: no output file specified.\n");
	SYNTAX;
        }

/*
 * Check data type of spec_param field
*/
    if(is_field_complex(fea_ih, "spec_param") != 0){
     Fprintf(stderr, "spectrans: complex data not supported yet - exiting.\n");
     exit(1);
   }

/*
 * get order info
 */
    order_v = *(long *)get_genhd("order_vcd", fea_ih);
    order_uv = *(long *)get_genhd("order_unvcd", fea_ih);
    maxorder = MAX(order_v,  order_uv);
/*
 * Check input and output spectral representation - abort, if the same
 */

    if((target_rep = lin_search(spec_reps, spec_name)) == -1)
        ERROR_EXIT("Invalid spectral type");
    if ((input_rep = *(short *)get_genhd("spec_rep", fea_ih)) == target_rep)
        ERROR_EXIT("Input and output spectral types are the same");


/*
 * Write debug output
 */
    if (debug_level > 0) {
	Fprintf(stderr, "spectrans: %s\n",  cmd_line);
	Fprintf(stderr, "spectrans: INput FEA_ANA file = %s\n", in_fea);
	Fprintf(stderr, "spectrans: version %s,  %s\n",  version,  date);
	Fprintf(stderr, "spectrans: Debug level = %d, output spectral type = %s\n", 
	    debug_level, spec_name);
	Fprintf(stderr, "spectrans: order_v = %d, order_uv = %d\n", 
	    order_v,  order_uv);
	Fprintf(stderr, "spectrans: input spec. rep. = %s, output spec. rep. is %s\n", 
	    spec_reps[input_rep], spec_reps[target_rep]);
    }
/*
 * create and write header for output file
 */
    fea_oh = copy_header(fea_ih);
    (void) strcpy (fea_oh->common.prog, "spectrans");
    (void) strcpy (fea_oh->common.vers, version);
    (void) strcpy (fea_oh->common.progdate, date);
    add_source_file(fea_oh, in_fea, fea_ih);
    add_comment(fea_oh, cmd_line);
    /*
     * put new spectral type in header
     */
    *(short *)get_genhd("spec_rep", fea_oh) = (short)target_rep;
    /*
     * for LSFs, LSFs, add maximum frequency spacing of search grid 
     */
    if(target_rep == LSF){
	int size;
	if(genhd_type("LSF_grid_width", &size, fea_oh) == HD_UNDEF)
	    (void)add_genhd_d("LSF_grid_width", &frq_res, 1, fea_oh);
	 else
	    *get_genhd_d("LSF_grid_width", fea_oh) = frq_res;
    }

    /* Write output header*/

    write_header(fea_oh, outfea_strm);
/*
 * allocate records for SPS files 
 */
    anafea_rec = allo_anafea_rec(fea_ih);
/*
 * allocate memory
 */
    input = (float *)calloc((unsigned) maxorder, sizeof(float));
    rc = (float *)calloc((unsigned) maxorder, sizeof(float));
    output = (float *)calloc((unsigned) maxorder, sizeof(float));
/*
 * main processing starts here; first get bandwidth info if LSFs
 */
   if(input_rep == LSF || target_rep == LSF) 
	bwidth = *(float *)get_genhd("src_sf", fea_ih)/2.;
    /*
     * main loop
     */
    while((get_anafea_rec(anafea_rec, fea_ih, infea_strm)) != EOF){
	if(debug_level > 4) {
	    Fprintf(stderr, "spectrans: Print each input feature record\n");
	    print_fea_rec(anafea_rec->fea_rec, fea_ih, stderr);
	}

	/* 
	 *  do spectral transformation and write record
	 */
	(void)trnsfrm_spec(anafea_rec, input_rep, target_rep, 
	    bwidth, (float)frq_res);
	put_anafea_rec(anafea_rec, fea_oh, outfea_strm);
	if(debug_level > 4) {
	    Fprintf(stderr, "spectrans: Output feature record\n");
	    print_fea_rec(anafea_rec->fea_rec, fea_oh, stderr);
	}
    }
    exit(0);
}


void
trnsfrm_spec(data_rec, input_type,  output_type, bwidth, frq_res)
struct anafea *data_rec;
int input_type,  output_type;
float bwidth, frq_res;
/* 
 *transform spec_param to out_param 
 */
{
   int voiced = 0,  order, i;
   int ret_val = 0;		/* return valuefor conversion routines */
   voiced = (*data_rec->frame_type == VOICED || 
	    *data_rec->frame_type == TRANSITION) ? 1 : 0;
   order = (voiced) ? order_v : order_uv;

    if(debug_level > 0){
	Fprintf(stderr, "spectrans: Voiced = %d, order = %d, bwidth = %f\n",
	     voiced, order, bwidth);
	Fprintf(stderr, "spectrans: input_type = %d, output_type = %d\n", 
		input_type, output_type);
    }
    /*
     * Copy spec_param array to input[]
     */
    for(i=0;i<order;i++) input[i] = data_rec->spec_param[i];
    /*
     * Clear spec_param for output
     */
    for(i=0;i<maxorder;i++) data_rec->spec_param[i] = 0.;
    if(debug_level > 0){
	for(i=0;i<order;i++)
	    Fprintf(stderr, "spectrans: input[%d] = %f\n", i, input[i]);
    }
    /*
     * First transform to reflection coefficients 
     */
    if(input_type != RC)
        ret_val = reps_rc(input, input_type, rc, order, bwidth);
    if (ret_val) ERROR_EXIT("error transforming to RCs");
    /*
     * Transform from Reflection Coefficients to desired spectral type
     */
    if(debug_level > 0){
	for(i=0;i<order;i++)
	    Fprintf(stderr, "spectrans: rc[%d] = %f\n", i, rc[i]);
	Fprintf(stderr, "spectrans: order = %d, bwidth = %f, frq_res =%f\n", 
	    order, bwidth, frq_res);
    }
    if(output_type != RC){
	/*
	 * convert from RCs to desired type
         */
	if(input_type == RC)
	    ret_val = rc_reps(input, output, output_type, 
			order, bwidth, frq_res);
	else
	    ret_val = rc_reps(rc, output, output_type, 
			order, bwidth, frq_res);
	if (ret_val) ERROR_EXIT("error transforming from RCs");
	if(debug_level > 0){
	    for(i=0;i<order;i++)
		Fprintf(stderr, "spectrans: output[%d] = %f\n", i, output[i]);
	}
	/*
	 * Copy back into spec_param
         */
	for(i=0;i<order;i++) data_rec->spec_param[i] = output[i];
    }
    else{ 
	/*RC desired; copy directly to spec_param 
	 */
	for(i=0;i<order;i++) data_rec->spec_param[i] = rc[i];
    }
}
