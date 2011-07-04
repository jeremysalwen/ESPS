/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
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
 * Written by:  David Burton
 * Checked by:
 * Revised by:
 *
 * Brief description: This program vector quantizes a FEA field 
 *                    in an array of codebook records
 *
 */

static char *sccs_id = "@(#)vqdst.c	1.5	1/27/97	ESI/ERL";

/*
 * include files
 */
#include <stdio.h>

/*
 * ESPS Includes
 */
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
#include <esps/feadst.h>
#include <esps/anafea.h>
#include <esps/unix.h>

/*
 * defines
 */
#define ERROR_EXIT(text) {(void)fprintf(stderr, "vqdst: %s - exiting\n", \
		 text); exit(1);}
#define SYNTAX USAGE ("vqdst [-x debug_level] [-P params_file] [-d distortion]\
[-f field]\n[-n rep_number] [-s] [-t] [-c] [-q] infile1.fea infile2.cbk outfile.dst")
#define ONE 1

/*
 * system functions and variables
 */
/*done via <esps/unix.h>

/*
 * external ESPS functions
 */
char *get_cmd_line();
int  lin_search();
char *eopen();
char *getsym_s();

/*
 * global variable declarations
 */
extern  optind;
extern	char *optarg;
int	debug_level = 0;    /*flag for debug output*/

static long n_rec();

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
    char		*version = "1.5";
    char		*date = "1/27/97";
    char		*params_file = NULL;/*default parameter file name*/
    char		*cbk_in = NULL;    /*input FEA_VQ codebook name*/
    FILE		*cbk_strm = NULL;    /*codebook stream pointer*/
    struct header	*cbk_ih = NULL;	    /*codebook header pointer*/

    char		*data_in = NULL;    /*input FEA file name*/
    FILE		*data_strm = NULL;	    /*input stream pointer*/
    struct header	*data_ih = NULL;    /*input data headerpointer*/
    struct fea_data	*data_rec = NULL;    /*input data record pointer*/

    char		*dst_out = NULL;    /*output FEA file - quantized*/
    struct header	*dst_oh = NULL;    /*output file header*/
    FILE		*dst_strm = stdout; /* output stream pointer */
    struct feadst	*dst_rec = NULL;    /*output distortion record*/

    struct vqcbk	**codebooks = NULL; /*pointer to codebook info*/

    char		*cmd_line = NULL;    /*to hold command line*/

    int			c;		    /*for getopt return*/

    double		(*distort)();	    /*routine to use for distortion*/
    float		*input_vec;	    /*one feature vector*/
    long		nfea;		    /*number of feature vectors*/

    double		*dist_mean;	    /*mean distortion*/
    double		*dist_val;	    /*distortions for current frame*/

    int			ncbkrec;	    /*rec. number current codebook*/
    long		i;		    /*loop index*/
    char		*field = NULL;	    /*field to be quantized*/
    char		*field_rep = "Not Applicable";
                                            /*description of field*/
    char		*enc_dist = "MSE";  /*encoding distortion*/
    long		fea_size;	    /*size of FEA field data*/
    int			dist_type;	    /*integer rep. of dsit. msre*/
    int			sflag = 0;	    /* check source flag*/
    int			tflag = 0;	    /*check signal flag*/
    int			cflag = 0;	    /*check field flag*/
    int			dflag = 0;	    /*distortion measure flag*/
    int			fflag = 0;	    /*quantized field flag*/
    int			qflag = 0;	    /*add fea file headers*/
    short		spec_rep;	    /*holds fana spec_param value*/
    short		rep_number = -1;    /*rep # of input FEA file*/
    int			header_size;	    /*needed by genhd_type()*/


/*
 *Initialization
 */
    cmd_line = get_cmd_line(argc, argv);    /*store copy of command line*/    
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:P:d:f:n:stcq")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'P':
		params_file = optarg;
		break;
	    case 'd':
		dflag++;
		enc_dist = optarg;
		break;
	    case 'f':
		fflag++;
		field = optarg;
		break;
	    case 's':
		sflag++;
		break;
	    case 't':
		tflag++;
		break;
	    case 'c':
		cflag++;
		break;
	    case 'q':
		qflag++;
		break;
	    case 'n':
		rep_number = (short) atoi(optarg);
		break;
	    default:
		SYNTAX;
	}
    }

/*
 * Read Params and Common
 */
	(void)read_params(params_file, SC_CHECK_FILE,  (char *)NULL);

/*
 * process file arguments 
 */
    if((argc - optind) != 3 && (argc - optind) != 2)
	SYNTAX;

    if ((argc - optind) == 3) /* all three filenames specified*/
	data_in = eopen("vqdst", argv[optind++], "r", FT_FEA, NONE,
		&data_ih, &data_strm);


    else if((argc - optind) == 2){/* only codebook and dst file specified; 
				get input from Common*/

	/*
	 * Get input file name and open file
	 */
        if(symtype("filename") == ST_UNDEF){
	    ERROR_EXIT("No input file in ESPS Common\n");
	}
	else
	    data_in = eopen("vqdst", getsym_s("filename"), "r", 
		FT_FEA, NONE, &data_ih, &data_strm);
    }

    /*
     * Now open other two files
     */
    /* First open input codebook file*/
	cbk_in  = eopen("vqdst", argv[optind++], "r", FT_FEA, FEA_VQ,
		&cbk_ih, &cbk_strm);	

	if(strcmp(cbk_in, "<stdin>") == 0)
	    ERROR_EXIT("Stdin cannot be used for the codebook file");

    /* Now work on output FEA_DST file*/
    dst_out = argv[optind];
    if( ( dst_strm = fopen(dst_out, "r") ) == NULL || 
			strcmp(dst_out, "-") == 0 ){
        /* output file does not exist*/
	/*Open output file*/
	if(strcmp(dst_out, "-") == 0){
	    dst_out = "<stdout>";
	    dst_strm = stdout;
	  }
	else
	    if((dst_strm = fopen(dst_out, "w")) == NULL){
		Fprintf(stderr, "vqdst: Cannot open %s\n", dst_out);
		exit(1);
	    }
	/*
         * Set up output header and write it
	 */
        dst_oh = new_header(FT_FEA);
	if(init_feadst_hd(dst_oh, (long)ONE) > 0)
	    ERROR_EXIT("Error filling FEA_DST header");

        (void) strcpy (dst_oh->common.prog, "vqdst");
	(void) strcpy (dst_oh->common.vers, version);
        (void) strcpy (dst_oh->common.progdate, date);
	add_source_file(dst_oh, cbk_in, cbk_ih);
        add_source_file(dst_oh, data_in, data_ih);
        add_comment(dst_oh, cmd_line);
    
        write_header(dst_oh, dst_strm);
    }
    else if (qflag){/*append to existing output file*/
	/*open file, read header, make appropriate checks*/

	dst_out = eopen("vqdst", dst_out, "r", FT_FEA, FEA_DST,
		&dst_oh, &dst_strm);
	if(fclose(dst_strm) != 0)
	    ERROR_EXIT("Troube closing fea_dst file");
	if(*(long *)get_genhd("max_num_sects", dst_oh) != ONE)
    	    ERROR_EXIT(
	    "FEA_DST file represents multisection data; not supported yet")
	if((dst_strm = fopen(dst_out, "a")) == NULL){
	    Fprintf(stderr, "vqdst: Could not open %s for appending\n", 
			    dst_out);
	    exit(1);
	}
    }
    else{
      Fprintf(stderr, "vqdst: Source file information cannot be included\n");
      Fprintf(stderr, "vqdst: Try -q option\n");
	exit(1);
    }
	
    if(debug_level > 0){
	Fprintf(stderr,
	    "vqdst: in FEA = %s, in VQCBK = %s, FEADST = %s\n",
	    data_in, cbk_in, dst_out);
	Fprintf(stderr, 
	    "vqdst: input field =  %s, encoding distortion = %s\n", 
	    field, enc_dist);
	Fprintf(stderr, 
	    "vqdst: dflag = %d, fflag = %d, sflag = %d, tflag = %d\n", 
	    dflag, fflag, sflag, tflag);
	Fprintf(stderr, "vqdst: cflag = %d\n", cflag);
    }

/*
 * Get info from parameter file, if appropriate
 */
    if( (dflag == 0) && (symtype("distortion") != ST_UNDEF) )
      	    enc_dist = getsym_s("distortion");

    if( (fflag == 0) && (symtype("field") == ST_UNDEF) )
	    ERROR_EXIT("No field to quantize specified");
    if(fflag == 0)
	    field = getsym_s("field");

    symerr_exit();    


/*
 * Check if input data is complex - exit if it is
 */
    if(is_field_complex(data_ih, field) == YES)
      ERROR_EXIT("Complex data fields not supported yet");
    
/*
 *  allocate space for data records
 */
    data_rec = allo_fea_rec(data_ih);

    /* get number of codebook records from codebook header*/
    ncbkrec = n_rec(&cbk_strm, &cbk_ih);

    /* allocate space for all the codebook info I'll need*/
     codebooks = (struct vqcbk **)calloc((unsigned)ncbkrec, 
				sizeof(struct vqcbk *));

/*
 * Get codebook info from input file and pack codebooks array
 */
    for(i = 0; i < ncbkrec; i++){
	codebooks[i] = allo_vqfea_rec(cbk_ih);
        if (get_vqfea_rec(codebooks[i], cbk_ih, cbk_strm) == EOF)
	     ERROR_EXIT("EOF when trying to read codebook record\n");
    }


/*
 * Get field_rep info, if appropriate
 */
    if(data_ih->hd.fea->fea_type == FEA_ANA){
	/*get character representation corresponding to spec_rep field*/
	spec_rep  = *(short *)get_genhd("spec_rep", data_ih);
	field_rep = spec_reps[spec_rep];
	if(debug_level > 0){
	    Fprintf(stderr, "spec_rep = %d; field_rep = %s\n",
	    spec_rep, field_rep);
	}
    }
	
/*
 * Allocate space for mean distortion array
 */

    dist_mean = (double *)calloc((unsigned)ncbkrec, sizeof(double));
    dist_val = (double *)calloc((unsigned)ncbkrec, sizeof(double));
/*
 * Check for inconsistencies
 */
    	
    if(((fea_size = get_fea_siz(field, data_ih,(short *)NULL,(long **)NULL)))
		 == 0)
	    ERROR_EXIT("Field does not exist in input record");


    if(cflag){
	if(data_ih->hd.fea->fea_type == FEA_ANA){
        for(i = 0; i < ncbkrec; i++)
	    if(strcmp(field_rep, codebooks[i]->field_rep) != 0){
	       Fprintf(stderr,
		"Input FEA and codebook file #%d have different field_reps\n"
		    , i+1);
		exit(1);
		}
	}
	
        for(i = 0; i < ncbkrec; i++)
	    if(strcmp(field, codebooks[i]->field) != 0){
	       Fprintf(stderr,
		"Input FEA and codebook file #%d have different field names\n"
		    , i+1);
		exit(1);
		}
    }

    if(sflag){
        /*compare input header input_source with the source_name value in all
	 the codebook records*/
	    
	    for(i = 0; i < ncbkrec; i++)
		if(strcmp(get_genhd_c("input_source", data_ih),
			    codebooks[i]->source_name) != 0){
		   Fprintf(stderr,
	          "Input FEA and codebook #%d SOURCE names differ\n", i+1);
		    exit(1);
		}

    }

    if(tflag){ /*check FEA's input_signal with signal_name in all codebooks*/
	    
	    for(i = 0; i < ncbkrec; i++)
		if(strcmp(get_genhd_c("input_signal", data_ih),
			    codebooks[i]->signal_name) != 0){
		   Fprintf(stderr,
		  "Input FEA and codebook #%d SIGNAL names differ\n", i+1);
		    exit(1);
		}
    }

    for(i = 0; i< ncbkrec; i++)
	if(fea_size != *codebooks[i]->dimen){
	    Fprintf(stderr,
  "vqdst: Codeword in codebook %d and input field dimensions differ\n",i);
	exit(1);
    }

/*
 * convert distortion string to integer representation
 */
    if((dist_type = lin_search(dist_types, enc_dist)) == -1)
	ERROR_EXIT("Invalid encodeing distortion specified");

    if(debug_level > 0){
	Fprintf(stderr,"vqdst: enc_dist = %s; dist_type = %d\n",
		enc_dist, dist_type);
    }
/*
 *select distortion function
 */
    switch (dist_type) {
	case MSE:  
	    distort = NULL;
	    break;
	case GOIS:
	    ERROR_EXIT("GOIS distortion not supported yet");
	    break;
	case GNIS:
	    ERROR_EXIT("GNIS distortion not supported yet");
	    break;
	case IS:
	    ERROR_EXIT("IS distortion is not supported yet");
	    break;
	default:
	    ERROR_EXIT("invalid distortion type");
    }


/*
 * main processing goes here
 */
    nfea = 0;

    /*
     * Allocate space for input feature vector
    */
    input_vec = (float *)calloc((unsigned)fea_size, sizeof(float));

    /*
     * determine data type of input field - exit if not floats
     */
    if ( get_fea_type(field, data_ih) != FLOAT )
      ERROR_EXIT("Numeric type of input field must be float");

    /*
     *Start Processing
     */
    
    while (get_fea_rec(data_rec, data_ih, data_strm) != EOF) {
	nfea++;
	/*
         *get a vector to encode
	 */
       	input_vec = (float *) get_fea_ptr(data_rec, field, data_ih);

	/*
	 *encode the vector
	 */
        switch (*codebooks[0]->cbk_struct) {
	    case FULL_SEARCH:  
		if(vqdist(input_vec, ncbkrec, codebooks, dist_val, distort)
		    > 0)
		    ERROR_EXIT("Invalid distortion mesure passed to vqdist");
		break;
	    default:
		ERROR_EXIT("invalid cbk_struct type");
	}
	/* sum distortion for each codebook*/
	for(i=0;i<ncbkrec;i++)
	    dist_mean[i] += dist_val[i];
    }
    /*
     * Now compute average distortion
     */
    for(i=0; i<ncbkrec; i++)
	dist_mean[i] = dist_mean[i] / nfea;

    if(debug_level > 0){
	Fprintf(stderr, "vqdst: Number of input vectors = %d\n", nfea);
	Fprintf(stderr, "vqdst: Mean square error:\n");
	for(i=0; i<ncbkrec; i++)
		Fprintf(stderr, "\tfor codebook %d = %f\n",
		i, dist_mean[i]);
    }
    /*
     * Write output FEA_DST records
     */
    dst_rec = allo_feadst_rec(dst_oh);
    for(i=0; i< ncbkrec; i++){
	*dst_rec->data_sect_mthd = SM_ONE;
	*dst_rec->data_sect_num = 1;
	*dst_rec->cbk_sect_mthd = SM_ONE;
	*dst_rec->cbk_sect_num = 1;
	(void)strcpy(dst_rec->quant_field, field);
	(void)strcpy(dst_rec->quant_field_rep, field_rep);
	*dst_rec->cbk_struct = *codebooks[i]->cbk_struct;
	*dst_rec->cbk_type = *codebooks[i]->cbk_type;
	*dst_rec->dsgn_dst = *codebooks[i]->dist_type;
	*dst_rec->encde_dst = dist_type;
	*dst_rec->cbk_sect_size = *codebooks[i]->current_size;
	(void)strncpy(dst_rec->cbk_name, cbk_in,15);/* temporary fix*/
	*dst_rec->cbk_rec = i+1;
	(void)strncpy(dst_rec->data_name, data_in,15);/*temporary fix*/
	(void)strcpy(dst_rec->cbk_source, codebooks[i]->source_name);
	(void)strcpy(dst_rec->cbk_signal, codebooks[i]->signal_name);
	(void)strcpy(dst_rec->source_type, codebooks[i]->source_type);
	if(genhd_type("input_source", &header_size, data_ih) != HD_UNDEF)
	    (void)strcpy(dst_rec->input_source, 
				get_genhd_c("input_source", data_ih));
	if(genhd_type("input_signal", &header_size, data_ih) != HD_UNDEF)
	    (void)strcpy(dst_rec->input_signal, 
				get_genhd_c("input_signal", data_ih));
	*dst_rec->in_rep_number = (short)rep_number;
	*dst_rec->data_sect_dst = dist_mean[i];
	*dst_rec->data_sect_size = (short)nfea;
	*dst_rec->average_dst = dist_mean[i];
	*dst_rec->cbk_train_dst = (float)*codebooks[i]->final_dist;

	put_feadst_rec(dst_rec, dst_oh, dst_strm);
    }    
    exit(0);
}


int
vqdist(in_vec, ncdbks, codebooks, dist_val, distort)
float	*in_vec;	    /*feature vector to be encoded*/
int	ncdbks;		    /*number of codebooks*/
struct vqcbk **codebooks;	/*the full search vq codebook*/
double	*dist_val;	    /*dists between in_vec and selected codewords*/
double	(*distort)();	    /*routine to compute distortions*/

/*
 *This routine encodes a single feature vector in each of ncdbks full-search
 *VQ codebooks.  The codebooks are passed through a pointer to a pointer
 * to the vqcbk struct. The distortion values are returned in dist_val.
 *If distort == NULL, a mean-square-error distortion is used.  Otherwise
 *the pointer is used to call a distortion routine.  
 */
{
    int i, j, k;
    int cbk_size, cdwd_dimen;

    if(debug_level > 1){/* check input parameters*/
	Fprintf(stderr,"vqdst: vqdist: number of codebooks = %d\n", ncdbks);
    }

    /*
     * All codebooks must have the same size and shape for this version
     */
    cbk_size = (*codebooks[0]->current_size);
    cdwd_dimen = (*codebooks[0]->dimen);

    if(debug_level > 0){
	Fprintf(stderr,"vqdst:vqdist: cbk_size = %d; cdwd_dimen = %d\n",
			cbk_size, cdwd_dimen);
    }

    if (distort == NULL) {	/*use mean-square-error distortion*/
	double se, diff;
	for(i = 0; i < ncdbks; i++){
	    dist_val[i] = DBL_MAX;

	    for(j = 0; j < cbk_size; j++) {
		se = 0.0;

		for(k = 0; k < cdwd_dimen; k++){
		    diff = (in_vec[k] - codebooks[i]->codebook[j][k]);
		    se += diff * diff;
		}
		if(debug_level >2)
		    Fprintf(stderr,
"vqdst: vqdist: codebook %d, codeword %d distortion = %lf\n", i, j, se);
		if (se < dist_val[i]) {
		    dist_val[i] = se;
		}
	    }
        /*
	 * find average distortion
         */
	dist_val[i] = dist_val[i]/cdwd_dimen;

	}
    }
    else { /*encode using externally supplied distort function
	    */
	/* no other distortion measures supported. If we
	    reach here, we have an error*/
        return(1);
    }
    return (0);
}


/*
 * Get number of records in a file.
 * Replace input stream with temporary file if input is a pipe
 * or record length is variable.
 */

static long
n_rec(file, hd)
    FILE	    **file;
    struct header   **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe, or record length
				     * is variable. */
    {
	FILE		*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	struct fea_data	*tmprec; /* record for writing and reading temp file */
	long		ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);
	tmprec = allo_fea_rec(tmphdr);

	for (ndrec = 0; get_fea_rec(tmprec, *hd, *file) != EOF; ndrec++)
	    put_fea_rec(tmprec, tmphdr, tmpstrm);

	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}


