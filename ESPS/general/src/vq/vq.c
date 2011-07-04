/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore
 * Checked by:
 * Modification History: 
 *   1/8/91: -i option and codword index field added by Bill Byrne.
 *
 * Brief description:  This program is used to vector-quantize FEA fields.
 */

static char *sccs_id = "@(#)vq.c	3.14	1/20/97	ESI/ERL";

/*
 * include files
 */
#include <stdio.h>
#include <string.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
/*
 * defines
 */
#define MAXLINE 200
#define MAXCMNT 2047  /* Number of characters in user entered comments. */
#define Fprintf (void)fprintf
#define Fflush (void)fflush
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}
#define SYNTAX USAGE ("vq [-x debug_level] [-h histfile] [-c record] [-f fieldname] [-i] cbk.fea fea.in fea.out")
#define FEA_BUFSIZE 500
/*
 * system functions and variables
 */
#ifndef IBM_RS6000
int getopt();
double	atof();
long time();
int atoi(), fclose();
char *strtok();
char *strcat();
char *fgets();
void rewind();
char *mktemp(), *ctime();
void rewind();
#if !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *malloc();
char *calloc();
#endif
double sqrt();
#endif
extern  optind;
extern	char *optarg;
/*
 * external SPS functions
 */
char *uniq_name();
char *get_cmd_line();
long *add_genhd_l();
short *add_genhd_s();
float *add_genhd_f();
char *add_genhd_c();
char *get_genhd();
int genhd_type();
int getsym_i(), lin_search();
struct header *read_header();
int rc_to_lar();
long vqencode();
char *savestring();
int add_fea_fld();
int del_fea_fld();
/*
 * global function declarations
 */
void vec_cy_f();
double mse_lar_dist();
void wrt_fea_vec();
/*
 * global variable declarations
 */
char		    *ProgName = "vq";
char		    *histfile = "vqhist";	/*output ASCII file for history data*/
FILE		    *histrm;
char		    *local_no_yes[] = {"NO", "YES", NULL}; 

int		debug_level = 0; /* level of history detail;
				  global for library access*/

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
    char		*Version = "3.14";
    char		*Date = "1/20/97";
    char		*cbk_fea = NULL;    /*input FEA_VQ file for codebook*/
    struct header	*cbk_ih;
    FILE		*cbk_strm;	    
    char		*data_in;	    /*input FEA file for quantization*/
    FILE		*in_strm = stdin;   
    struct header	*fea_ih;
    struct fea_data	*fea_rec;
    struct fea_data	*fea_rec_out;
    char		*data_out;	    /*output FEA file - quantized*/
    struct header	*fea_oh;
    FILE		*out_strm = stdout;
    char		*cmd_line;	    /*to hold command line*/
    int			c;		    /*for getopt return*/
    long		tloc;		    /*for local time*/
    long		fea_dim;	    /*numb. columns in feadata*/
    struct vqcbk	*cbk;		    /*the VQ codebook*/
    double		(*distort)();	    /*routine to use for distortion*/
    float		*input_vec;	    /*one feature vector*/
    long		nfea;		    /*number of feature vectors*/
    long		curr_ind;	    /*current feature vector*/
    long		max_ind;	    /*vector index of max distortion*/
    double		curr_dist;	    /*distortion of current vector*/
    double		max_dist;	    /*maximum vector distortion*/
    double		tot_sqdist;	    /*cumulative square distortion*/
    double		tot_dist;	    /*cumulative distortion*/
    double		dist_mean;	    /*mean distortion*/
    double		dist_variance;	    /*variance of distortion*/
    double		dist_sd;	    /*standard deviation of distortion*/
    int			dist_err = 0;	    /*error return from distort*/
    int			ncbkrec;	    /*record number of initial codebook*/
    int			c_flag = 0;	    /*flag for -c option*/
    long		i;
    float		*field;		    /*pointer to field to be quantized*/
    float		*field_out;	    /*pointer to quantized field in output*/
    char		*fieldname = "spec_param"; /*name of FEA field*/
    char		*enc_dist_name = "encode_distortion"; /*a generic name*/
    char		*tempname;	    /*holder for generic name*/
    long		fea_size;	    /*size of FEA field data*/
    int			dumnumb;	    /*dummy variable for number of generics */
    char		*cbk_field;	    /*name of design field*/
    int	    		sizedummy;	    /*dummy variable for genhd_type*/
    int                 i_flag = 0;         /*flag for -i option*/
    char                *ndx_fieldname;     /*field name for code word index*/
    long                *cw_ndx=NULL;       /*pointer to code word index*/
/*
 *Initialization
 */
    cmd_line = get_cmd_line(argc, argv);    /*store copy of command line*/    
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:h:c:f:i")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'h':
		histfile = optarg;
		break;
	    case 'c':
		ncbkrec = atoi(optarg);
		c_flag++;
		break;
	    case 'f':
		fieldname = optarg;
 		break;
            case 'i':
                i_flag++;
		break;
	    default:
		SYNTAX;
	}
    }
    
/*
 * process file arguments 
 */
    if (optind < argc) {
	cbk_fea = argv[optind++];
	if (strcmp (cbk_fea, "-") == 0)
	    ERROR_EXIT("vq: can't use - for codebook -- exiting");
        TRYOPEN (argv[0], cbk_fea, "r", cbk_strm);
    }
    else {
	Fprintf(stderr, "vq: no FEA codebook file specified.\n");
	SYNTAX;
        
    }
    if (optind < argc) {
	data_in = argv[optind++];
	if (strcmp (data_in, "-") == 0)
	    data_in = "<stdin>";
	else
	    TRYOPEN (argv[0], data_in, "r", in_strm);
	
    }
    else {
	Fprintf(stderr, "vq: no input data file specified.\n");
	SYNTAX;
    }

    if (optind < argc) {
	data_out = argv[optind++];
	if (strcmp (data_out, "-") == 0)
	    data_out = "<stdout>";
	else
	    TRYOPEN (argv[0], data_out, "w", out_strm);
    }
    else {
	Fprintf(stderr, "vq: no output data file specified.\n");
	SYNTAX;
    }

    TRYOPEN(argv[0], histfile, "w", histrm);
/*
 * read FEA file and get codebook
 */

    if ((cbk_ih = read_header(cbk_strm)) == NULL)
	NOTSPS(argv[0], cbk_fea);
    if (cbk_ih->common.type != FT_FEA) 
	ERROR_EXIT("codebook file not FEA file");
    if (cbk_ih->hd.fea->fea_type != FEA_VQ) 
	ERROR_EXIT("codebook file not FEA_VQ file");
    cbk = allo_vqfea_rec(cbk_ih);
    assert (cbk != NULL);

    if (!c_flag)
	ncbkrec = cbk_ih->common.ndrec;

    if (ncbkrec >= 1)
    {
	fea_skiprec(cbk_strm, (long) ncbkrec - 1, cbk_ih);

	if (get_vqfea_rec(cbk, cbk_ih, cbk_strm) == EOF)
	    ERROR_EXIT("EOF when trying to read initial codebook");
    }
    else if (ncbkrec == -1)
    {
	/* Variable-length records (e.g. Esignal Ascii format).
	 * Read records to find the last.
	 */
	struct vqcbk     *next_cbk, *tmp_cbk;

	next_cbk = allo_vqfea_rec(cbk_ih);

	if (get_vqfea_rec(cbk, cbk_ih, cbk_strm) == EOF)
	    ERROR_EXIT("EOF when trying to read initial codebook");

	while (get_vqfea_rec(next_cbk, cbk_ih, cbk_strm) != EOF)
	{
	    tmp_cbk = cbk;
	    cbk = next_cbk;
	    next_cbk = tmp_cbk;
	}
    }
    else
	ERROR_EXIT("codebook record number less than 1");

    fea_dim = *cbk->dimen;

/*
 *select distortion function
 */
    switch (*cbk->dist_type) {
	case MSE:  
	    distort = NULL;
	    break;
	case MSE_LAR:
	    distort = mse_lar_dist;
	    break;
	default:
	    ERROR_EXIT("invalid distortion type");
    }
/*
 * initial history and debug output 
 */

    if (debug_level > 0) {
        tloc = time(0); 
        Fprintf(histrm, "vq history ouput on %s", ctime(&tloc));
        Fprintf(histrm, "vq version %s, date %s\n", Version, Date); 
        Fprintf(histrm, "command line:\n%s", cmd_line);
	Fprintf(histrm, "codebook type: %s\n", vq_cbk_types[*cbk->cbk_type]);
	Fprintf(histrm, "codebook structure: %s\n", cbk_structs[*cbk->cbk_struct]);
	Fprintf(histrm, "codebook distortion: %s\n", dist_types[*cbk->dist_type]);
	Fprintf(histrm, "codebook dimension: %d\n", *cbk->dimen);
	Fprintf(histrm, "codebook size: %d\n", *cbk->current_size);	
    }	
    if (debug_level > 1) {
	Fprintf(histrm, "\nCodebook:\n\n");
	print_fea_rec(cbk->fea_rec, cbk_ih, histrm);
    }

/*
 * read FEA input header, allocate FEA record and temp space
 */
    if ((fea_ih = read_header(in_strm)) == NULL)      
	ERROR_EXIT("couldn't read input FEA file header");
    if (fea_ih->common.type != FT_FEA)
	ERROR_EXIT("file to encode not FEA file");
    /* if input not a pipe, can check to make sure data is there*/
    if (fea_ih->common.ndrec == 0) 
	ERROR_EXIT("no data in input file\n");
    fea_rec = allo_fea_rec(fea_ih);
    input_vec = (float *) calloc(fea_dim, sizeof (float));
/*
 * consistency checks
 */
    fea_size = get_fea_siz(fieldname, fea_ih, (short *) NULL, (long **) NULL);
    if (fea_size == 0) {
	Fprintf(histrm, 
	    "vq: field %s doesn't exist in input FEA file\n", fieldname);
	Fprintf(stderr, 
	    "vq: field %s doesn't exist in input FEA file\n", fieldname);
	exit(1);
    }
    if (fea_dim > fea_size) ERROR_EXIT("codeword size larger than FEA field size");
    if (fea_dim != fea_size)  {
	Fprintf(histrm, 
	  "vq: WARNING - size of FEA field greater than codewords\n");
	Fprintf(stderr, 
	  "vq: WARNING - size of FEA field greater than codewords\n");
    }
    if (get_fea_type(fieldname, fea_ih) != FLOAT) 
	ERROR_EXIT("specified field is not type FLOAT");
    if (get_genhd("quantized", fea_ih) != NULL &&
	    (*(short *) get_genhd("quantized", fea_ih)) == YES) {
	Fprintf(histrm, 
	    "vq: WARNING - input file already has some quantized fields\n");
	Fprintf(stderr, 
	    "vq: WARNING - input file already has some quantized fields\n");
    }
    /*
     * check for consistency between quantization field and codebook design
     */
    if (genhd_type("fea_field", &sizedummy, cbk_ih) != HD_UNDEF) {
	if (strcmp(get_genhd("fea_field", cbk_ih), fieldname) != 0) {
	    cbk_field = get_genhd("fea_field", cbk_ih);
	    Fprintf(histrm, 
		"vq: WARNING - inconsistent fea_field in codebook\n");
	    Fprintf(histrm, "\tinput field being quantized: %s\n", fieldname);
	    Fprintf(histrm, 
		"\tcodebook designed from field named: %s\n", cbk_field);
	    Fprintf(stderr, 
		"vq: WARNING - inconsistent fea_field in codebook\n");
	    Fprintf(stderr, "\tinput field being quantized: %s\n", fieldname);
	    Fprintf(stderr, 
		"\tcodebook designed from field named:  %s\n", cbk_field);
	}
    }
    else {
	Fprintf(histrm, "vq: WARNING - codebook has no record of design fieldname\n");
	Fprintf(stderr, "vq: WARNING - codebook has no record of design fieldname\n");	
    }
    /*
     *create and define header of output FEA file
     */
    fea_oh = copy_header(fea_ih);
    (void) strcpy (fea_oh->common.prog, "vq");
    (void) strcpy (fea_oh->common.vers, Version);
    (void) strcpy (fea_oh->common.progdate, Date);
    add_source_file(fea_oh, cbk_fea, cbk_ih);
    add_source_file(fea_oh, data_in, fea_ih);
    add_comment(fea_oh, cmd_line);
    /*
     * fill "quantized" field if there (FEA_ANA), else create it
     */
    if (get_genhd("quantized", fea_ih) != NULL)
	*(short *) get_genhd("quantized", fea_oh) = YES;
    else
    *add_genhd_e("quantized", (short *) NULL, 1, local_no_yes, fea_oh) = YES;
    /*
     * add generic header items that describe codebook used and quantized
     * field
     */
    tempname = uniq_name("quantized_field", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_c(tempname, fieldname, strlen(fieldname), fea_oh);
    tempname = uniq_name("cbk_current_size", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_l(tempname, cbk->current_size, 1, fea_oh);
    tempname = uniq_name("cbk_dimen", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_l(tempname, cbk->dimen, 1, fea_oh);
    tempname = uniq_name("cbk_dist_type", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_e(tempname, cbk->dist_type, 1, dist_types, fea_oh);
    tempname = uniq_name("cbk_cbk_struct", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_e(tempname, cbk->cbk_struct, 1, cbk_structs, fea_oh);
    tempname = uniq_name("cbk_cbk_type", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_e(tempname, cbk->cbk_type, 1, vq_cbk_types, fea_oh);
    tempname = uniq_name("cbk_final_dist", genhd_list(&dumnumb, fea_oh));
    (void) add_genhd_d(tempname, cbk->final_dist, 1, fea_oh);
    enc_dist_name = uniq_name("encode_distortion", genhd_list(&dumnumb, fea_oh));
    *add_genhd_d(enc_dist_name, (double *) NULL, 1, fea_oh) = -1;

    if ( (ndx_fieldname = (char *) malloc( (unsigned)(strlen(fieldname) + 10) )) == NULL) {
	Fprintf(stderr, "%s: can't allocate field name memory.\n", ProgName);
	exit(1);
    }
    strcpy(ndx_fieldname, fieldname);
    strcat(ndx_fieldname, "_cwndx");

    if (-1 == add_fea_fld(ndx_fieldname, 1L, 0,
			  (long *) NULL, LONG, (char **) NULL, fea_oh)) {
	Fprintf(stderr, "trouble adding %s to output record.\n", ndx_fieldname);
	exit(1);
    }
    if (i_flag) {
	if (del_fea_fld(fieldname, fea_oh)==-1) {
	    Fprintf(stderr, "%s: can't remove %s.\n", ProgName, fieldname);
	    exit(1);
	}
    }

    update_waves_gen(fea_ih, fea_oh, 1.0, 1.0);
    write_header(fea_oh, out_strm);
    fea_rec_out = allo_fea_rec(fea_oh);

    if ( (field = (float *) get_fea_ptr(fea_rec, fieldname, fea_ih)) == NULL) {
	Fprintf(stderr, "can't get field %s in input file.\n", fieldname);
	exit(1);
    }
    if ( (cw_ndx = (long *) get_fea_ptr(fea_rec_out, ndx_fieldname, fea_oh)) == NULL) {
	Fprintf(stderr, "can't get field %s.\n", ndx_fieldname);
	exit(1);
    }
    if (!i_flag) 
	if ( (field_out = (float *) get_fea_ptr(fea_rec_out, fieldname, fea_oh)) == NULL) {
	    Fprintf(stderr, "can't get field %s in output file.\n", fieldname);
	    exit(1);
	}

/*
 * main processing goes here
 */
    nfea = max_dist = tot_dist = tot_sqdist = 0.0;
    while (get_fea_rec(fea_rec, fea_ih, in_strm) != EOF) {
	nfea++;
	vec_cy_f(input_vec, field, fea_dim);
	/*
	 *encode the vector
	 */
        switch (*cbk->cbk_struct) {
	    case FULL_SEARCH:  
		curr_ind = vqencode(input_vec, fea_dim, cbk->codebook, 
		  *cbk->current_size, *cbk->dimen, &curr_dist, distort, &dist_err);
		break;
	    default:
		ERROR_EXIT("invalid cbk_struct type");
	}
	/* copy codeword index to output field */
	*cw_ndx = curr_ind;
	/*Write out the encoded vector, making sure to zero 
	 *the refl. coefficients beyond the codebook dimension
         */
	copy_fea_rec( fea_rec, fea_ih, fea_rec_out, fea_oh, (char **)NULL, (short **)NULL);
	if (!i_flag) {
	    vec_cy_f(field_out, cbk->codebook[curr_ind], fea_dim);
	    for (i = fea_dim; i < fea_size; i++) 
		field_out[i] = 0.0;
	}
	put_fea_rec(fea_rec_out, fea_oh, out_strm);
	/*
	 *history stuff
 	 */
	tot_dist += curr_dist;
	tot_sqdist += curr_dist * curr_dist;
	if (debug_level > 2) {
	    Fprintf(histrm, "vq: encoding input vector %d\n", nfea);
	    if (debug_level > 3) {
		Fprintf(histrm, "     Input vector:\n");
		wrt_fea_vec(histrm, input_vec, *cbk->dimen);
	    }
	    Fprintf(histrm, 
		"     Encoding index = %d, distortion = %g\n",  curr_ind, curr_dist);
	    Fprintf(histrm, 
		"     Current average distortion = %g\n", tot_dist / nfea);
	    if (debug_level > 3) {
		Fprintf(histrm, "     Encoded vector:\n");
		wrt_fea_vec(histrm, cbk->codebook[curr_ind], *cbk->dimen);
	    }
	    Fflush(histrm);
	}
	if (curr_dist > max_dist) {
	    max_dist = curr_dist;
	    max_ind = curr_ind;
	}
	if (dist_err) ERROR_EXIT("error return from distort function");
    }
    /*  compute mean encoding distortion and write it back in the header
     */
    if (out_strm != stdout) {
	dist_mean = tot_dist / nfea;	
	*(double *)get_genhd(enc_dist_name, fea_oh) = dist_mean;
	rewind(out_strm);
	write_header(fea_oh, out_strm);
    }
    /*
     * write some summary statistics to the history file
     */
    if (debug_level > 0) {
	dist_variance = (tot_sqdist / nfea) - dist_mean * dist_mean;
	dist_sd = sqrt(dist_variance);
	Fprintf(histrm, "vq: encoded %d input vectors\n", nfea);
	Fprintf(histrm, "\tmean distortion = %g, stand. dev. = %g\n", dist_mean, dist_sd);
	Fprintf(histrm, "\tmaximum distortion was %g at vector %d\n", max_dist, max_ind);
	Fflush(histrm);
    }
    exit(0);
}

double
mse_lar_dist(fea_vector, vec_dim, codeword, cdwd_dim, error)
float *fea_vector;
long vec_dim;
float *codeword;
long cdwd_dim;
int *error;
/*
 *This distortion function assumes both the feature vector and 
 *the codeword are reflection coefficients; it converts both to 
 *log area ratios and then does mean-square error.  
 */
{
    double diff;
    float fea_lar, cd_lar;
    double msedist = 0.0;
    while (vec_dim-- != 0) {
    	(void) rc_to_lar(*fea_vector++, &fea_lar);
	(void) rc_to_lar(*codeword++, &cd_lar);
	diff = fea_lar - cd_lar;
	msedist += diff * diff;
    }
    return msedist / cdwd_dim;
}    


void
wrt_fea_vec(strm, fea_vec, size)
FILE *strm;		/*file stream to use*/
float *fea_vec;		/*feature vector to print*/
long size;	/*dimension of feature vector*/
/*
 *utility function for printing a feature vector*/
{
    long i;
    for (i = 0; i < size; i++) Fprintf(strm, "\t%f ", fea_vec[i]);
    Fprintf(strm, "\n");

}



void
vec_cy_f(dst, src, n)
float  *dst, *src;	/*pointers to destination and source vectors*/
long n;		/*size of vectors*/
/*
 *This routine copies src to dst.
 */
{
    while (n-- != 0)
	*dst++ = *src++;
}



