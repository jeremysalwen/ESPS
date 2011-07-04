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
 * Revised by:
 *
 * Brief description:  This program is used to design a VQ codebook from 
 *                     FEA data
 *
 */

static char *sccs_id = "@(#)vqdes.c	3.10	1/20/97	ESI/ERL";
int  	    debug_level = 2;	    /*level of history detail*/

/*
 * include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
/*
 * defines
 */
#define Fprintf (void)fprintf
#define Fflush (void)fflush
#define DEBUG(n) if (debug_level >= n) Fprintf
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}
#define SYNTAX USAGE ("vqdes [-x debug_level] [-P param_file] [-h histfile] [-k checkfile] [-i] input.fea output.fea")
#define FEA_BUFSIZE 1000000
#define MAX_ITER 500
/*
 * system functions and variables
 */
int getopt ();
double	atof();
double log();
long time();
int atoi(), strcmp(), fclose();
void rewind();
extern  optind;
extern	char *optarg;
char *mktemp(), *ctime(), *strcpy();
void rewind();
#ifndef DEC_ALPHA
char *calloc();
#endif
/*
 * external SPS functions
 */
int symtype();
long *add_genhd_l();
short *add_genhd_s();
float *add_genhd_f();
char *get_genhd();
char *add_genhd_c();
long get_fea_siz();
short get_fea_type();
char *get_cmd_line(), *getsym_s();
void symerr_exit(), write_header(), set_sd_type();
int getsym_i(), lin_search();
struct header *read_header();
struct fea_data *allo_fea_rec();
float **f_mat_alloc();
int rc_to_lar();
void vec_copy_f();
/*
 * global function declarations
 */
long rc_get_chunk();
double mse_lar_dist();
void s_param_err();
int checkpnt();
/*
 * global variable declarations
 */
char		    *ProgName = "vqdes";
char		    *cmd_line;		/*string for command line*/
char		    *histfile = "vqdeshist";	/*output ASCII file for history data*/
FILE		    *histrm;
char		    *input_ana = NULL;  /*input FEA file*/
FILE		    *fea_strm = NULL;
struct header	    *fea_ih;
struct fea_data	    *fea_rec;
char		    *fea_field = NULL;  /*name of FEA data field*/
long		    ndrec_ana;		/* num records in FEA file
					   or -1 if unknown */
int		    whole_fits;		/* flag for whether all FEA data fits
					   in feadata */
char		    *out_vq = NULL;	/*output FEA file for codebook*/
struct header	    *vq_oh;
FILE		    *vq_strm = NULL;
char		    *checkfile = "vqdes.chk";    /*checkpoint file*/
FILE		    *chkstrm;
struct header	    *chk_oh;
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
    char		*Version = "3.10";
    char		*Date = "1/20/97";
    char		*paramfile = NULL;  /*parameter file name*/
    char		*init_file = NULL;  /*file with initial codebook*/
    FILE		*init_strm;
    struct header	*init_ih;
    int			init_rec;	    /*record number of initial codebook*/
    int			init_behav;	    /*whether to cluster initial codebook*/
    int			c;		    /*for getopt return*/
    long		tloc;		    /*for local time*/
    long		i, j;		    /*loop variables*/
    long		fea_size;	    /*size of FEA field data*/
    short		vqfea_type;	    /*generic header item type in
					      initial codebook*/
    float		*fea_ptr;	    /* location of FEA field data
					       in fea_rec */
    float		**feadata;	    /*matrix of training FEA data*/
    long		*enc;		    /*final encode indices for data*/ 
    long		fea_len;	    /*numb. rows in feadata*/
    long		fea_dim;	    /*numb. columns in feadata*/
    long		c_rows;		    /*size of VQ codebook*/
    struct vqcbk	*cbk;		    /*the VQ codebook*/
    struct vqcbk	*icbk;		    /*an initial VQ codebook*/
    int			cbk_type = -1;    /*type of codebook (param file)*/
    double		(*distort)();	    /*routine to use for distortion*/
    int			vq_ret;		    /*return value from vqdesign*/    
    int			init_flag = 0;	    /*flag for -i option*/
    int			max_iter = MAX_ITER;/*max no. of iterations*/
/*
 *initialization
 */
    cmd_line = get_cmd_line(argc, argv); /*store copy of command line*/
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:P:h:k:i")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'P':
		paramfile = optarg;
		break;
	    case 'h':
		histfile = optarg;
		break;
	    case 'k':
		checkfile = optarg;
		break;
	    case 'i':
		init_flag++;
		break;
	    default:
		SYNTAX;
	}
    }
/*
 * process file arguments
 */
    if (optind < argc) {
	input_ana = argv[optind++];
	if (strcmp (input_ana, "-") == 0) {
	    input_ana = "<stdin>"; 
	    /*For now, can't use stdin*/
	    ERROR_EXIT("can't use standard input for training sequence");
	}
	else
	    TRYOPEN (argv[0], input_ana, "r", fea_strm);
	}
    else {
	Fprintf(stderr, "vqdes: no input FEA file specified.\n");
	SYNTAX;
        }
    if (optind < argc) {
	out_vq = argv[optind++];
	if (strcmp (out_vq, "-") == 0)
	    out_vq = "<stdout>";
	else
	    TRYOPEN (argv[0], out_vq, "w", vq_strm);
	}
    else {
	Fprintf(stderr, "vqdes: no output file specified.\n");
	SYNTAX;
        }
    TRYOPEN(argv[0], histfile, "w", histrm);
/*
 * initial history and debug output
 */
    if (debug_level > 0) {
        tloc = time(0);
        Fprintf(histrm, "vqdes history output on %s", ctime(&tloc));
        Fprintf(histrm, "vqdes version %s, date %s\n", Version, Date);
        Fprintf(histrm, "command line:\n%s", cmd_line);
	Fprintf(histrm, "checkpoint file is %s\n", checkfile);
	Fflush(histrm);
    }
/*
 * read FEA header and allocate record
 */
    if ((fea_ih = read_header(fea_strm)) == NULL)      
	ERROR_EXIT("couldn't read input FEA file header");
    if (fea_ih->common.type != FT_FEA)
	ERROR_EXIT("input not FEA file");
    ndrec_ana = fea_ih->common.ndrec;
    fea_rec = allo_fea_rec(fea_ih);
/*
 * read parameters from parameter file and SPS common - require that
 * the filename in common match that of the input FEA file
 */
    (void) read_params(paramfile, SC_CHECK_FILE, input_ana);
    if(symtype("max_iter") != ST_UNDEF) 
	max_iter = getsym_i("max_iter");
    fea_dim = getsym_i("fea_dim");
    c_rows = getsym_i("vq_size");
    fea_field = getsym_s("fea_field");
    symerr_exit();  /*exit if any of the parameters were missing*/
    if (init_flag) {
	/*
         *get name and record for initial codebook -- make sure file is ok
	 */
	init_file = getsym_s("init_file");
	if (strcmp(checkfile, init_file) == 0)
	    ERROR_EXIT("Checkpoint file has same name as init_file.")
	init_rec = getsym_i("init_rec");
	TRYOPEN(argv[0], init_file, "r", init_strm);
	if ((init_ih = read_header(init_strm)) == NULL) 
	    ERROR_EXIT("couldn't read input FEA_VQ file header");
	if ((init_ih->common.type != FT_FEA) || (init_ih->hd.fea->fea_type != FEA_VQ))
	    ERROR_EXIT("initial codebook file not FEA or not FEA_VQ");
    }
    /*
     * cbk_type is optional
     */
    if (symtype("cbk_type") != ST_UNDEF) 
	if ((cbk_type = lin_search(vq_cbk_types, getsym_s("cbk_type"))) == -1)
	    ERROR_EXIT("invalid cbk_type in parameter file");
    /*
     *check to make sure fea_field exists, has right type, and is big
     * enough
     */
    fea_size = get_fea_siz(fea_field, fea_ih, (short *) NULL, (long **) NULL);
    if (fea_size == 0) {
	Fprintf(stderr, fea_field, 
	    "vqdes: field %s doesn't exist in input file\n", fea_field);
	exit(1);
    }
    if (fea_dim > fea_size) ERROR_EXIT("VQ_DIM larger than FEA field size");
    if (fea_dim != fea_size)  
	Fprintf(histrm, 
	  "vqdes: WARNING - size of FEA field greater than VQ_DIM\n");
    if (get_fea_type(fea_field, fea_ih) != FLOAT) 
	ERROR_EXIT("specified fea_field is not type FLOAT");
/*
 *create FEA header 
 */
    vq_oh = new_header(FT_FEA);
    if (init_vqfea_hd(vq_oh, c_rows, fea_dim) != 0) 
	ERROR_EXIT("error filling FEA header");
    (void) strcpy (vq_oh->common.prog, "vqdes");
    (void) strcpy (vq_oh->common.vers, Version);
    (void) strcpy (vq_oh->common.progdate, Date);
    add_source_file(vq_oh, input_ana, fea_ih);
    if (init_flag) add_source_file(vq_oh, init_file, init_ih);
    add_comment(vq_oh, cmd_line);
    /*
     *     *add generic header item for field name
     */
    (void) add_genhd_c("fea_field", fea_field, 0, vq_oh);
/*
 *Now that the header is complete, we can write it out
 */
    write_header(vq_oh, vq_strm);
    Fflush(vq_strm); /*just in case vqdes bombs :-)*/
/*
 *Allocate FEA_VQ record.
 */
    cbk = allo_vqfea_rec(vq_oh);
/*
 *Now finish processing the parameters and initializing the codebook
 */
    if (init_flag) {
	if ((init_behav = lin_search(vq_init, getsym_s("init_behav"))) == -1){
	    Fprintf(stderr, "vqdes: invalid init_behav in parameter file\n");
	    exit(1);
	}
	/* error and consistency checking
	 */
	if (init_ih->common.ndrec < init_rec && init_ih->common.ndrec != -1)
	    ERROR_EXIT("not enough records in initial codebook file");
	/*
	 * get the initial codebook and do more consistency checking
	 */
	fea_skiprec(init_strm, (long) init_rec - 1, init_ih);
	icbk = allo_vqfea_rec(init_ih);
	if (get_vqfea_rec(icbk, init_ih, init_strm) == EOF)    
	    ERROR_EXIT("EOF when trying to read initial codebook");
	if (*icbk->current_size > c_rows) 
	    ERROR_EXIT("initial codebook is too big");
	if (*icbk->current_size == c_rows && init_behav == INIT_NOCLUSTER) 
	    ERROR_EXIT(
	    "initial codebook size == design size with INIT_NOCLUSTER specified");
	if ((cbk_type != -1) && (*icbk->cbk_type != cbk_type)) {
	    Fprintf(histrm, 
		"vqdes: initial codebook type %s differs from param file value %s\n", 
		vq_cbk_types[*icbk->cbk_type], vq_cbk_types[cbk_type]);
	    Fprintf(stderr, 
		"vqdes: initial codebook type %s differs from param file value %s\n", 
		vq_cbk_types[*icbk->cbk_type], vq_cbk_types[cbk_type]);
	    exit(1);
	}
	if (*icbk->dimen != fea_dim) 
	    ERROR_EXIT("dimension in initial codebook doesn't match training input");
	/*
	 *check for generic header item giving FEA field
	 */
	vqfea_type = genhd_type("fea_field", (int *)NULL, init_ih);
	if (vqfea_type == HD_UNDEF || vqfea_type != CHAR) { 
	    Fprintf(histrm, 
	    "vqdes: WARNING - initial codebook has no generic header item fea_field\n");
	    Fprintf(stderr, 
	    "vqdes: WARNING - initial codebook has no generic header item fea_field\n");
	}
	if (strcmp(get_genhd("fea_field", init_ih), fea_field) != 0) {
	    Fprintf(histrm, 
		"vqdes: WARNING - inconsistent fea_field in initial codebook\n");
	    Fprintf(histrm, "\tfield from parameter file: %s\n", fea_field);
	    Fprintf(histrm, "\tfield from initial codebook: %s\n", get_genhd("fea_field",init_ih));
	    Fprintf(stderr, 
		"vqdes: WARNING - inconsistent fea_field in initial codebook\n");
	    Fprintf(stderr, "\tfield from parameter file: %s\n", fea_field);
	    Fprintf(stderr, "\tfield from initial codebook: %s\n", get_genhd("fea_field",init_ih));
	}
	/* Now copy the initial codebook
	 */
	*cbk->current_size = *icbk->current_size;
	for (i = 0; i < *cbk->current_size; i++) 
	    for (j = 0; j < fea_dim; j++) 
		cbk->codebook[i][j] = icbk->codebook[i][j];
    }
    else {
	/* no initial codebook -- start from scratch
 	 */	
	*cbk->current_size = 0;
	init_behav = INIT_NOCLUSTER;
    }
    /*These parameters apply whether or not there's an initial codebook.
     */
    *cbk->num_iter = 0;
    *cbk->dimen = fea_dim;
    *cbk->cbk_struct = fea_encode(getsym_s("cbk_struct"), "cbk_struct", vq_oh);
    if (*cbk->cbk_struct == -1) s_param_err(getsym_s("cbk_struct"));
    *cbk->design_size = c_rows;
    *cbk->conv_ratio = getsym_d("conv_ratio");
    *cbk->cbk_type = cbk_type;
    *cbk->dist_type = fea_encode(getsym_s("dist_type"), "dist_type", vq_oh);
    if (*cbk->dist_type == -1) s_param_err(getsym_s("dist_type"));    
    symerr_exit();  /*exit if any of the parameters were missing*/
/*
 *Create and write header for checkpoint file.
 */
    chk_oh = copy_header(vq_oh);
    TRYOPEN(argv[0], checkfile, "w", chkstrm);
    (void) strcpy (chk_oh->common.prog, "vqdes");
    (void) strcpy (chk_oh->common.vers, Version);
    (void) strcpy (chk_oh->common.progdate, Date);
    add_source_file(chk_oh, input_ana, fea_ih);
    if (init_flag) add_source_file(chk_oh, init_file, init_ih);
    add_comment(chk_oh, cmd_line);
    (void) add_comment(chk_oh, "This is a checkpoint file.");
    write_header(chk_oh, chkstrm);    	
/*
 *Allocate and do pre-processing of FEA data array
 */
    if (ndrec_ana != -1 && ndrec_ana * fea_dim <= FEA_BUFSIZE)
	/* number of FEAs is known, and all will fit */
    {
	whole_fits = 1;
	fea_len = ndrec_ana;
    }
    else /* unknown number, or too many to process at once */
    {
	whole_fits = 0;
	fea_len = FEA_BUFSIZE / fea_dim;
    }
    feadata = f_mat_alloc((unsigned) fea_len, (unsigned) fea_dim);
    if (feadata == NULL) ERROR_EXIT("couldn't allocate enough memory");
    if (whole_fits) {
	/*go ahead and fill the array now*/ 
	fea_ptr = (float *) get_fea_ptr(fea_rec, fea_field, fea_ih);
	for (i = 0; i < ndrec_ana; i++) {
	    (void) get_fea_rec(fea_rec, fea_ih, fea_strm);
	    vec_copy_f(feadata[i], fea_ptr, fea_dim);
	}
    }
/*
 *initialize the space for the code points returned by vqdesign
 */
    enc = (long *) calloc((unsigned) fea_len, sizeof(long));    
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
 * history output of design parameters
 */
    if (debug_level > 0) {
        Fprintf(histrm, "\nParameter values:\n");
	Fprintf(histrm, "\tdimension of training vectors = %d\n", fea_dim);
	Fprintf(histrm, "\tname of FEA field with training vectors = %s\n", fea_field);
	Fprintf(histrm, "\tfractional conv. thresh = %g\n", *cbk->conv_ratio);
	Fprintf(histrm, "\tmax number of iterations on one level = %d\n",
	    max_iter);
	Fprintf(histrm, "\tsize of VQ codebook= %d\n", *cbk->design_size);
	Fprintf(histrm, "\tdistortion type = %s\n", dist_types[*cbk->dist_type]);
	Fprintf(histrm, "\tcodebook structure = %s\n", cbk_structs[*cbk->cbk_struct]);
	if (init_flag) Fprintf(histrm, 
	    "Initial codebook supplied from file %s, record %d\n", init_file, init_rec);
    }	
/*
 * main processing goes here
 */
    switch (*cbk->cbk_struct) {
	case FULL_SEARCH:  
	    vq_ret = vqdesign(histrm, debug_level, feadata, fea_len, fea_dim, 
		cbk, enc, init_behav, SPLIT_DIST, rc_get_chunk, 
		(int (*)())NULL, distort, (int (*)())NULL, checkpnt, max_iter);
	    if (vq_ret == VQ_NOCONVG) Fprintf(stderr, 
		"vqdes: FAILED - codebook didn't converge, see history file\n");
    if (debug_level > 0) Fprintf(histrm, 
		"vqdes: vqdesign return %s\n", vq_returns[vq_ret]);
	    break;
	default:
	    ERROR_EXIT("invalid distortion type");
    }
/*
 *write FEA record for final codebook
 */
    put_vqfea_rec(cbk, vq_oh, vq_strm);

    exit(0);
}

void
s_param_err(string)
char *string;
{
    Fprintf(stderr, "vqdes: unknown parameter string %s\n", string);
    exit (1);
}

long
rc_get_chunk(data, len, dim, num_prev, error)
float **data;
long len;
long dim;
long num_prev;
int *error;
{
/*get_chunk routine (for vqdesign) to get FEA training data
 */
    long    i, lim;
    float   *fea_ptr;

    *error = 0;
    if (whole_fits) {
	if (num_prev == 0) 
	    return len;
	else
	    return 0;
    }
    /*we get here only if the whole FEA file didn't fit in data
     */
    if (num_prev == ndrec_ana)
	return 0;

    if (num_prev == 0) {
	rewind(fea_strm);
	fea_ih = read_header(fea_strm);
    }

    fea_ptr = (float *) get_fea_ptr(fea_rec, fea_field, fea_ih);

    /* If the number of remaining input records is unknown or
     * greater than "len", try to fill "data" completely;
     * otherwise try for the known number of remaining records.
     */
    lim = (ndrec_ana == -1 || ndrec_ana - num_prev > len)
	? len : ndrec_ana - num_prev;

    for (i = 0; i < lim; i++) {
	if (get_fea_rec(fea_rec, fea_ih, fea_strm) == EOF) {
	    if (ndrec_ana != -1)
		*error = 1;
	    break;
	}
	vec_copy_f(data[i], fea_ptr, dim);
    }

    return i;
}

int
checkpnt(cdbk, chk_type)
struct vqcbk *cdbk;		/*codebook pointer*/
int chk_type;			/*type of checkpoint*/
{
/*this checkpoint routine writes the codebook to the output file 
 *right before each split operation and also to the checkpoint file
 *right after every encode operation (pass through the training sequence).
 *Only one record is kept in the checkpoint file.  
 */
    if (chk_type == CHK_SPLIT) {
	put_vqfea_rec(cdbk, vq_oh, vq_strm);
	Fflush(vq_strm);
    }
    rewind(chkstrm);
    write_header(chk_oh, chkstrm);
    put_vqfea_rec(cdbk, chk_oh, chkstrm);
    Fflush(chkstrm);
}

#define LOG10 2.302585093

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
 *log area ratios and then does mean-square error.  This program is 
 *quite inefficient - if we really want to design large codebooks 
 *using mse_lar, training sequence should be converted to LARs 
 *before encoding.  
 */
{
    double diff;
    float fea_lar, cd_lar;
    double msedist = 0.0;
    double temp;
    while (vec_dim-- != 0) {
	temp = *fea_vector++;
	fea_lar = log((1.0 + temp) / (1.0 - temp));
	temp = *codeword++;
	cd_lar = log((1.0 + temp) / (1.0 - temp));
	diff = fea_lar - cd_lar;
	msedist += diff * diff;
    }
    *error = 0;
    return msedist / cdwd_dim;
}    

void
vec_copy_f(dst, src, n)
float  *dst, *src;	/*pointers to destination and source vectors*/
long n;		/*size of vectors*/
/*
 *This routine copies src to dst.
 */
{
    while (n-- != 0)
	*dst++ = *src++;
}
