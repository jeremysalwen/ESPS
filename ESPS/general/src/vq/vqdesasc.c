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
 * Design a VQ codebook from ASCII input data
 */

static char *sccs_id = "@(#)vqdesasc.c	3.9	1/20/97	ESI/ERL";

int   debug_level = 2;	    /*level of history detail*/


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
#define MAXLINE 500
#define MAXCMNT 2047  /* Number of characters in user entered comments. */
#define Fprintf (void)fprintf
#define Fflush (void)fflush
#define DEBUG(n) if (debug_level >= n) Fprintf
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}
#define SYNTAX USAGE ("vqdesasc [-l max_line_len] [-x debug_level] [-P param_file] [-h histfile]\n [-k checkfile] [-i] [-c comment] [-C comment_file] input output.fea")
#define FEA_BUFSIZE 1000000 /*maximum size of training sequence array*/
#define MAX_ITER 500
/*
 * system functions and variables
 */
int getopt ();
double	atof();
long time();
int atoi(), strcmp(), fclose();
char *strtok();
char *fgets();
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
int add_fea_fld();
long *add_genhd_l();
short *add_genhd_s();
float *add_genhd_f();
char *add_genhd_c();
char *get_cmd_line(), *getsym_s();
void symerr_exit(), write_header(), set_sd_type();
int getsym_i(), lin_search();
struct header *read_header();
struct fea_data *allo_fea_rec();
float **f_mat_alloc();
int rc_to_lar();
void put_fea_rec();
char *get_fea_ptr();
short fea_encode();
struct vqcbk *allo_vqfea_rec();
/*
 * global function declarations
 */
long gen_get_chunk();
int checkpnt();
double mse_lar_dist();
void s_param_err();
long find_fea_dim();
long read_fea_vec();
/*
 * global variable declarations
 */
char		    *ProgName = "vqdesasc";
char		    *cmd_line;		/*string for command line*/
char		    comment[MAXCMNT + 2];	    /*string for command line comment*/
char		    *histfile = "vqdesaschist";	/*output ASCII file for history data*/
FILE		    *histrm;
char		    *input_file = NULL;	    /*input ASCII file*/
FILE		    *input_strm;
char		    *out_fea = NULL;	    /*output FEA file for codebook*/
struct header	    *fea_oh;
FILE		    *fea_strm = NULL;
char		    *checkfile = "vqdesasc.chk";    /*checkpoint file*/
FILE		    *chkstrm;
struct header	    *chk_oh;
long		    ndrec;		/*num records in input file*/
int		    whole_fits;		/*flag for whether all input data fits
					   in feadata*/
int                 max_line_len = MAXLINE;   /*max input line length  */

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
    char		*Version = "3.9";
    char		*Date = "1/20/97";
    char		*paramfile = NULL;  /*parameter file name*/
    char		*cmntfile = NULL;	/*comment file name*/
    FILE		*cmnt_strm;
    char		*init_file = NULL;	/*file with initial codebook*/
    FILE		*init_strm;
    struct header	*init_ih;
    int			init_rec;	    /*record number of initial codebook*/
    int			init_behav;	    /*whether to cluster initial codebook*/
    int			c;		    /*for getopt return*/
    long		tloc;		    /*for local time*/
    long		i, j;		    /*loop variable*/
    float		**feadata;	    /*matrix of training RC data*/
    long		*enc;		    /*final encode indices for data*/ 
    long		fea_len;	    /*numb. rows in feadata*/
    long		fea_dim;	    /*numb. columns in feadata*/
    long		new_dim;	    /*dim of input vector  */
    long		c_rows;		    /*numb. rows in codebook*/
    struct vqcbk	*cbk;		    /*the VQ codebook*/
    struct vqcbk	*icbk;		    /*initial VQ codebook*/
    double		(*distort)();	    /*routine to use for distortion*/
    int			vq_ret;		    /*return value from vqdesign*/    
    char		*line;		    /*one input ASCII line*/
    int			cflag = 0;		/*flag for -c option*/
    int			cfile_flag = 0;	    /*flag for -C option*/
    int			init_flag = 0;	    /*flag for -i option*/
    int			max_iter = MAX_ITER;/*max no. of iterations*/
/*
 *Initialization
 */
    cmd_line = get_cmd_line(argc, argv); /*store copy of command line*/
/*
 * process command line options
 */
    while ((c = getopt (argc, argv, "x:P:h:c:C:k:l:i")) != EOF) {
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
	    case 'C':
		cmntfile = optarg;
		cfile_flag++;
		break;
    	    case 'c':
		if ((i = strlen (optarg)) <= MAXCMNT)
		    strcpy (comment, optarg);
		else {
		    Fprintf(stderr,
			"vqdesasc: Comment field has %d characters, max is %d\n", i, MAXCMNT);
		    exit (1);
		}
		comment[i] = '\n';
		comment[i+1] = '\0';
		cflag++;
		break;
	    case 'k':
		checkfile = optarg;
		break;
	    case 'i':
		init_flag++;
		break;
	    case 'l':
		max_line_len = atoi(optarg);
		break;
	    default:
		SYNTAX;
	}
    }

    line = (char *) malloc(max_line_len);

    /*Get comment if one wasn't supplied.
     */
    if ((!cflag) && (!cfile_flag)){ 
    	printf("vqdesasc: Please enter one-line comment describing training sequence.\n");
	fflush (stdout);
	i = 0;
	while ((int)(comment[i] = getchar()) != '\n' && i < MAXCMNT) i++;
	printf("Thank you, processing continuing...\n");
    }
/*
 * process file arguments - NOTE: we don't allow stdin for the input (yet)
 */
    if (optind < argc) {
	input_file = argv[optind++];
        TRYOPEN (argv[0], input_file, "r", input_strm);
	}
    else {
	Fprintf(stderr, "vqdesasc: no input file specified.\n");
	SYNTAX;
        }
    if (optind < argc) {
	out_fea = argv[optind++];
	if (strcmp (out_fea, "-") == 0)
	    out_fea = "<stdout>";
	else
	    TRYOPEN (argv[0], out_fea, "w", fea_strm);
	}
    else {
	Fprintf(stderr, "vqdesasc: no output file specified.\n");
	SYNTAX;
        }
    TRYOPEN(argv[0], histfile, "w", histrm);
/*
 * read first training vector to determine dimension
 */
    ndrec = 1;
    if (fgets(line, max_line_len, input_strm) != NULL) 
	fea_dim = find_fea_dim(line);
    if ((line == NULL) || (fea_dim == 0)) ERROR_EXIT("no data in input file");
    if (fea_dim < 2) ERROR_EXIT("feature vectors must have 2 or more components");
/*
 * read parameters from parameter file and SPS common - require that
 * the filename in common match that of the input SD file
 */
    (void) read_params(paramfile, SC_CHECK_FILE, input_file);
    if(symtype("max_iter") != ST_UNDEF) 
	max_iter = getsym_i("max_iter");
/*
 *create FEA header 
 */
    fea_oh = new_header(FT_FEA);
    c_rows = getsym_i("vq_size");
    if (init_vqfea_hd(fea_oh, c_rows, fea_dim) != 0) 
	ERROR_EXIT("error filling FEA header");
    (void) strcpy (fea_oh->common.prog, "vqdesasc");
    (void) strcpy (fea_oh->common.vers, Version);
    (void) strcpy (fea_oh->common.progdate, Date);
    if (init_flag) {
	/*
         *get initial codebook from file and record given in 
	 *parameter file
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

	add_source_file(fea_oh, init_file, init_ih);
    }
    add_comment(fea_oh, cmd_line);
    if (cfile_flag)  { /*comment file given*/
        TRYOPEN(argv[0], cmntfile, "r", cmnt_strm);
	while (fgets(comment, MAXCMNT, cmnt_strm) != NULL) 
	    add_comment(fea_oh, comment);
    }
    else  /*comment line from command line or prompt*/
	add_comment(fea_oh, comment);
/*
 *Now that the header is complete, we can write it out 
 */
    write_header(fea_oh, fea_strm);
    Fflush(fea_strm); /*just in case vqdesasc bombs :-)*/
/*
 *Allocate FEA_VQ record.
 */
    cbk = allo_vqfea_rec(fea_oh);
/*
 *Now finish processing the parameters and initializing the codebook
 */
    if (init_flag) {
	if ((init_behav = lin_search(vq_init, getsym_s("init_behav"))) == -1){
	    Fprintf(stderr, "vqdesasc: invalid init_behav in parameter file\n");
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
	if (*icbk->cbk_type != MISC) {
	    Fprintf(histrm, "vqdesasc: WARNING - initial codebook type not MISC\n");
	    Fprintf(stderr, "vqdesasc: WARNING - initial codebook type not MISC\n");	
	if (*icbk->dimen != fea_dim) 
	    ERROR_EXIT("dimension in initial codebook doesn't match training input");
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
    *cbk->dimen = fea_dim;
    *cbk->cbk_struct = fea_encode(getsym_s("cbk_struct"), "cbk_struct", fea_oh);
    if (*cbk->cbk_struct == -1) s_param_err(getsym_s("cbk_struct"));
    *cbk->design_size = c_rows;
    *cbk->conv_ratio = getsym_d("conv_ratio");
    *cbk->num_iter = 0;
    *cbk->cbk_type = MISC;
    *cbk->dist_type = fea_encode(getsym_s("dist_type"), "dist_type", fea_oh);
    if (*cbk->dist_type == -1) s_param_err(getsym_s("dist_type"));    
    symerr_exit();  /*exit if any of the parameters were missing*/
/*
 *Create and write header for checkpoint file.
 */
    chk_oh = copy_header(fea_oh);
    chk_oh->common.ndrec = 1;
    TRYOPEN(argv[0], checkfile, "w", chkstrm);
    (void) strcpy (chk_oh->common.prog, "vqdes");
    (void) strcpy (chk_oh->common.vers, Version);
    (void) strcpy (chk_oh->common.progdate, Date);
    if (init_flag) add_source_file(chk_oh, init_file, init_ih);
    add_comment(chk_oh, cmd_line);
    add_comment(chk_oh, comment);
    (void) add_comment(chk_oh, "This is a checkpoint file.");
    write_header(chk_oh, chkstrm); 
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
        Fprintf(histrm, "vqdesasc history ouput on %s", ctime(&tloc));
        Fprintf(histrm, "vqdesasc version %s, date %s\n", Version, Date);
        Fprintf(histrm, "command line:\n%s", cmd_line);
	Fprintf(histrm, "checkpoint file is %s\n", checkfile);
        Fprintf(histrm, "\nParameter values:\n");
	Fprintf(histrm, "\tdimension of feature vectors = %d\n", fea_dim);
	Fprintf(histrm, "\tfractional conv. thresh = %g\n", *cbk->conv_ratio);
	Fprintf(histrm, "\tmax number of iterations on one level = %d\n",
	    max_iter);
	Fprintf(histrm, "\tsize of VQ codebook= %d\n", *cbk->design_size);
	Fprintf(histrm, "\tdistortion type = %s\n", dist_types[*cbk->dist_type]);
	Fprintf(histrm, "\tcodebook type = %s\n", cbk_structs[*cbk->cbk_struct]);
	if (init_flag) Fprintf(histrm, 
	    "Initial codebook supplied from file %s, record %d\n", init_file, init_rec);
	Fflush(histrm);
    }
/*
 * read input file through to determine number of training vectors
 * note that we have already read the first one to determine the size. 
 * This approach is bad in that we read the file twice; however, it 
 * does permit dynamic allocation of the array
 */
    while (fgets(line, max_line_len, input_strm) != NULL) {
	ndrec++;
	new_dim = find_fea_dim(line);
	if (new_dim != fea_dim)
	    Fprintf(stderr, 
		"vqdesasc: input vector %ld wrong length; length is %ld, should be %ld\n",
		 ndrec, new_dim, fea_dim);
    }
    rewind(input_strm);
/*
 *Allocate and do pre-processing of RC data array; determine length 
 *of training sequence 
 */
    if ((ndrec * fea_dim) <= FEA_BUFSIZE) /*all RCs will fit*/ {
	whole_fits = 1;
	fea_len = ndrec;
    }
    else {
	whole_fits = 0;
	fea_len = FEA_BUFSIZE / fea_dim;
    }   	
    feadata = f_mat_alloc((unsigned) fea_len, (unsigned) fea_dim);
    if (feadata == NULL) ERROR_EXIT("couldn't allocate enough memory");
    if (whole_fits) /*go ahead and fill the array now*/ 
	for (i = 0; i < ndrec; i++) 
	    (void) read_fea_vec(input_strm, fea_dim, feadata[i]);
/*
 *initialize the space for the code points returned by vqdesign
 */
    enc = (long *) calloc((unsigned) fea_len, sizeof(long));    
    spsassert(enc != NULL, "vqdesasc: couldn't allocate enought memory");
/*
 * main processing starts here
 */
    switch (*cbk->cbk_struct) {
	case FULL_SEARCH:  
	    vq_ret = vqdesign(histrm, debug_level, feadata, fea_len, fea_dim, 
		cbk, enc, init_behav, SPLIT_DIST, gen_get_chunk, 
		(int (*)())NULL, distort, (int (*)())NULL, checkpnt, max_iter);
	    if (vq_ret == VQ_NOCONVG) Fprintf(stderr, 
		"vqdes: FAILED - codebook didn't converge, see history file\n");
	    if (debug_level > 0) Fprintf(histrm, 
		"vqdesasc: vqdesign return %s\n", vq_returns[vq_ret]);
	    break;
	default:
	    ERROR_EXIT("invalid distortion type");
    }
/*
 *write FEA record for final codebook;
 */
    put_vqfea_rec(cbk, fea_oh, fea_strm);
    exit(0);
}

void
s_param_err(string)
char *string;
{
    Fprintf(stderr, "vqdesasc: unknown parameter string %s\n", string);
    exit (1);
}

long
gen_get_chunk(data, len, dim, num_prev, error)
float **data;
long len;
long dim;
long num_prev;
int *error;
{
/*get_chunk routine (for vqdesign) to get ASCII training data
 */
    long i;
    *error = 0;
    if (whole_fits) {
	if (num_prev == 0) 
	    return len;
	else
	    return 0;
    }
    /*we get here only if the whole ANA file didn't fit in feadata
     */
    if (num_prev == ndrec)
	return 0;

    if (num_prev == 0)
	rewind(input_strm);

    for (i = 0; (i < len) && ((i + num_prev) < ndrec); i++) {
	if (read_fea_vec(input_strm, dim, data[i]) == 0)
	    *error = 1;
    }
    return i;
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
 *log area ratios and then does mean-square error.  This program is 
 *quite inefficient - if we really want to design large codebooks 
 *using mse_lar, training sequence should be converted to LARs 
 *before encoding.  
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
    *error = 0;
    return msedist / cdwd_dim;
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
	put_vqfea_rec(cdbk, fea_oh, fea_strm);
	Fflush(fea_strm);
    }
    rewind(chkstrm);
    write_header(chk_oh, chkstrm);
    put_vqfea_rec(cdbk, chk_oh, chkstrm);
    Fflush(chkstrm);
}

long
find_fea_dim(fealine)
char *fealine;	    /*an input line containing a single feature vector*/
{
/*This routine counts the number of white-space-separated tokens in a line.
 */
    long ntoken = 0;
    char *tok;
    if ((tok = strtok(fealine, " \t\n")) != NULL) ntoken++;
    while ((tok = strtok(0, " \t\n")) != NULL) 	ntoken++;
    return ntoken;
}

long
read_fea_vec(strm, f_dim, fea_vec)
FILE *strm;
long f_dim;
float *fea_vec;
{
    long nval = 0;
    char *tok;
    static char *in_line = NULL;

    if (in_line == NULL) 
      in_line = (char *) malloc(max_line_len);

    (void) fgets(in_line, max_line_len, strm);
    if ((tok = strtok(in_line, " \t\n")) != NULL) {
	nval++;
	*fea_vec++ = atof(tok);
    }
    while ((tok = strtok(0, " \t\n")) != NULL) {
	nval++;
	if (nval <= f_dim) *fea_vec++ = atof(tok);
    }
    return nval;
}    

