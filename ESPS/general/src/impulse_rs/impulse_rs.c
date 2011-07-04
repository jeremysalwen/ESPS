/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:  Bill Byrne, 5/24/91, for FEAFILT
 * Revised by:  Derek Lin, 11/22/92, for numerical stability
 * Revised by:  David Burton - 7/30/97, fix uninitialized memory
 *
 * Brief description:
 *  This, the main module of impulse_resp, interprets the command line,	
 *  sets up and checks input and output files, reads the input data,	
 *  computes the output values with the help of block_filter2, and	
 *  writes the output.  Input is an ESPS filter file.  Output is an	
 *  ESPS filter file or sampled-data file.				
 *
 */

static char *sccs_id = "@(#)impulse_rs.c	3.11 7/30/97	12/6/92	ERL";


#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feafilt.h>
#include <esps/feasd.h>

#define Version "3.11"
#define Date "7/30/97"
#define ProgName "impulse_resp"

#define ERROR_EXIT(text) {(void) Fprintf(stderr, "%s: %s - exiting\n", \
                                         argv[0], text); exit(1);}

#define SYNTAX \
USAGE("impulse_resp [-i] -n size [-r range] [-p range] [-s][-x debug_level]\n\
 input.filt output")

#define TRYALLOC(type,num,var) { \
    if (((var) = (type *) malloc((unsigned)(num)*sizeof(type)))==NULL) \
    { Fprintf(stderr, "%s:  can't allocate memory for %d points.\n", \
		ProgName, (int)(num)); \
	exit(1); }}

#define FILETYPE {Fprintf(stderr, \
        "%s: internal error--confused about output file type.", \
        ProgName); exit(1);}

char    *get_cmd_line();
void    lrange_switch();
int     block_filter2();
char	*eopen();
int     debug_level = 0;

void
main(argc, argv)
    int     argc;
    char    **argv;
{
    struct header
            *ih, *oh;
    FILE    *ifile, *ofile;
    char    *iname, *oname;
    struct  feafilt *i_filt_rec = NULL, *o_filt_rec = NULL;
    struct  feasd *o_sd_rec = NULL;

    char    *cmd_line = get_cmd_line(argc, argv);
    extern  optind;
    extern char *optarg;

    int     c;
    int     size = -1;      /* invalid value; no default */
    char    *range = NULL;
    long    start_rec, end_rec;
    short   invert = NO;
    int     out_type = FEA_FILT;

    double   *impulse, *state, *imp_resp;
    int     st_size;
    int     n, i, type;
    double sf = 1.;
    double start_time = 0.;
    struct sdata *in, *out;
    struct fdata *frec;

/* read command-line options */

    while ((c = getopt(argc, argv, "n:x:p:r:is")) != EOF)
	switch (c)
	{
	case 'n':
	    size = atoi(optarg);
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
        case 'p':
	case 'r':
	    range = optarg;
	    break;
	case 'i':
	    invert = YES;
	    break;
	case 's':
	    out_type = FEA_SD;
	    break;
	default:
	    SYNTAX;
	    break;
	}

/* open files and read headers */

    if (optind != argc - 2) SYNTAX;

    iname = eopen(ProgName, argv[optind++], "r", FT_FEA, FEA_FILT, &ih, &ifile);
    oname = eopen(ProgName, argv[optind++], "w",
			NONE, NONE, (struct header **) NULL, &ofile);
    
    if (size < 0)
    {
        Fprintf(stderr,
            "%s: output length unspecified or negative\n",
            ProgName);
        exit(1);
    }

    start_rec = 1;
    end_rec = LONG_MAX;
    lrange_switch(range, &start_rec, &end_rec, 0);
    if (start_rec < 1)
    {
	Fprintf(stderr, "%s: beginning of range precedes 1.\n", ProgName);
	exit(1);
    }
    if (end_rec < start_rec)
    {
	Fprintf(stderr, "%s: empty range specified.\n", ProgName);
	exit(1);
    }

/* Make output header */

    oh = new_header(FT_FEA);
    Strcpy(oh->common.prog, ProgName);
    Strcpy(oh->common.vers, Version);
    Strcpy(oh->common.progdate, Date);
    add_source_file(oh, iname, ih);
    add_comment(oh, cmd_line);
    (void) add_genhd_l("start_rec", &start_rec, 1, oh);
        /* record number of first input record in output header */
    (void) add_genhd_s("invert", &invert, 1, oh);
        /* record -i option in output header */

   /* if samp_freq exists in input file header, copy it to output filter
	  file header */
  {
    int num_of_elements;
    double samp_freq;
    if (out_type == FEA_FILT && 
	genhd_type("samp_freq", &num_of_elements, ih) != HD_UNDEF)
    {
      samp_freq = get_genhd_val("samp_freq", ih, (double) 1);
      (void) add_genhd_d("samp_freq", &samp_freq, 1, oh);
    }
  }


    switch (out_type)
    {
    case FEA_FILT:
	init_feafilt_hd( oh, (long) size, (long) 0, (filtdesparams *) NULL );
	break;
    case FEA_SD:
	  if((init_feasd_hd(
	     oh, DOUBLE, (int) 1, &start_time, (int) 0, sf)) != 0 ){
	        Fprintf(stderr, 
                "impulse_resp: Couldn't allocate FEA_SD header - exiting.\n");
                exit(1);
	      }
        break;
    default:
        FILETYPE
        break;
    }

    write_header(oh, ofile);

/* set up input and output filter records */

    i_filt_rec = allo_feafilt_rec(ih);
    TRYALLOC(double, size, imp_resp)

    switch (out_type)
    {
    case FEA_FILT:
        o_filt_rec = allo_feafilt_rec(oh);
	spsassert( o_filt_rec != NULL, "can't allocate output FEA_FILT record.");
        *o_filt_rec->num_size = size;
        *o_filt_rec->denom_size = 0;
        break;
    case FEA_SD:
	o_sd_rec = allo_feasd_recs(oh, DOUBLE, size, NULL, NO);
        break;
    default:
        FILETYPE
        break;
    }

/* other initializations */

    TRYALLOC(double, size, impulse)

    if (size > 0) impulse[0] = 1.0;
    for (i = 1; i < size; i++) impulse[i] = 0.0;

    st_size = *get_genhd_l("max_num", ih);
    if ( st_size < *get_genhd_l("max_denom", ih) )
        st_size = *get_genhd_l("max_denom", ih);

    TRYALLOC(double, st_size, state)

    if (debug_level >= 1)
    {
        Fprintf(stderr,
            "first record: %ld, last: %ld.\n", start_rec, end_rec);
        Fprintf(stderr,
            "size: %d, max_num: %ld, max_den: %ld, st_size: %d\n",
		size, *get_genhd_l("max_num", ih), 
		*get_genhd_l("max_denom", ih), st_size);
    }

/* skip to first input record to process */

    for (n = 1; n < start_rec; n++)
        if (get_feafilt_rec(i_filt_rec, ih, ifile) == EOF)
        {
            Fprintf(stderr,
                "%s: premature EOF encountered\n", ProgName);
            exit(1);
        }

    in = (struct sdata *) malloc(sizeof(struct sdata));
    spsassert(in,"impulse_rs: in malloc failed");
    out = (struct sdata *) malloc(sizeof(struct sdata));
    spsassert(out, "impulse_rs: out malloc failed");

/* main loop */

    for ( ; n <= end_rec && get_feafilt_rec(i_filt_rec, ih, ifile) != EOF; n++)
    {
        if (invert)
        {
	  long *nsiz_tmp;
	  long *zsiz_tmp;
	  double *rnum_tmp;
	  double *inum_tmp;
	  double_cplx *zero_tmp;
	  double gain;
	  
	  nsiz_tmp = i_filt_rec->denom_size;
	  rnum_tmp = i_filt_rec->re_denom_coeff;
	  inum_tmp = i_filt_rec->im_denom_coeff;
	  zsiz_tmp = i_filt_rec->pole_dim;
	  zero_tmp = i_filt_rec->poles;
	  
	  i_filt_rec->denom_size = i_filt_rec->num_size;
	  i_filt_rec->num_size = nsiz_tmp;
	  i_filt_rec->re_denom_coeff = i_filt_rec->re_num_coeff;
	  i_filt_rec->re_num_coeff = rnum_tmp;
	  i_filt_rec->im_denom_coeff = i_filt_rec->im_num_coeff;
	  i_filt_rec->im_num_coeff = inum_tmp;

	  i_filt_rec->pole_dim = i_filt_rec->zero_dim;
	  i_filt_rec->zero_dim = zsiz_tmp;
	  i_filt_rec->poles = i_filt_rec->zeros;
	  i_filt_rec->zeros = zero_tmp;

	  i = 1;
	  if( genhd_type("gain_iir", &i, ih) != HD_UNDEF){
	    gain = *get_genhd_d("gain_iir", ih);
	    gain = 1/gain;
	    add_genhd_d("gain_iir", &gain, i, ih);
	  }	
        }
	if( i_filt_rec->poles || i_filt_rec->re_denom_coeff ) 
	  type = IIR_FILTERING;
	else type = FIR_FILTERING;

	if(debug_level) fprintf(stderr,"impulse_rs: filter type %d\n", type);
	frec = init_fdata( type, i_filt_rec, ih, 0, 0);

        if (debug_level >= 1)
        {
            Fprintf(stderr, "record number %d.  tag = %ld.\n",
                n, i_filt_rec->fea_rec->tag);
        }

	in->data = (void *) impulse;
	in->data_type = DOUBLE;
	in->no_channel = 1;
	in->rec = NULL;
	out->data = (void *) imp_resp;
	out->rec = NULL;
	out->data_type = DOUBLE;

        if( 0 !=  block_filter2(size, in, out, &frec))
	  ERROR_EXIT("Can't determine signal or filter type");

        switch (out_type)
	  {
        case FEA_FILT:
	    /* copy imp_resp to float array*/
	    for(i=0; i<size;i++)
		o_filt_rec->re_num_coeff[i] = imp_resp[i];

            o_filt_rec->fea_rec->tag = i_filt_rec->fea_rec->tag;
            put_feafilt_rec(o_filt_rec, oh, ofile);
            break;
        case FEA_SD:
	    o_sd_rec->data = (char *) imp_resp;
	    put_feasd_recs( o_sd_rec, 0L, size, oh, ofile);
            break;
        default:
            FILETYPE
            break;
        }
    }

    /* clean up*/
    free(in);
    free(out);
    free(impulse);
    free(imp_resp);
    free(state);
    exit(0);
    /* NOTREACHED */
}


