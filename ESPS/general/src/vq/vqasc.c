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
 * Vector-quantize an ASCII input data stream.
 */

static char *sccs_id = "@(#)vqasc.c	3.8	1/20/97	ESI/ERL";

#define MAX_FEA_DIM	100    /*maximum dimension of feature vectors*/
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
#define SYNTAX USAGE ("vqasc [-l max_line_len] [-x debug_level] [-h histfile]\n [-c record] cbk.fea data.in data.out")
#define FEA_BUFSIZE 500
/*
 * system functions and variables
 */
int getopt();
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
char *malloc();
#endif
double sqrt();
/*
 * external SPS functions
 */
char *get_cmd_line();
long *add_genhd_l();
short *add_genhd_s();
float *add_genhd_f();
char *add_genhd_c();
int getsym_i(), lin_search();
struct header *read_header();
int rc_to_lar();
long vqencode();
/*
 * global function declarations
 */
double mse_lar_dist();
long read_fea_vec();
void wrt_fea_vec();

/*
 * global variable declarations
 */
char		    *ProgName = "vqasc";
char		    *histfile = "vqaschist";	/*output ASCII file for history data*/
FILE		    *histrm;
int                 max_line_len = MAXLINE;   /*max input line length  */

int debug_level = 0;

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
    char		*Version = "3.8";
    char		*Date = "1/20/97";
    char		*cbk_fea = NULL;    /*input FEA file for codebook*/
    struct header	*cbk_ih;
    FILE		*cbk_strm;	    
    char		*data_in;	    /*ASCII input file of vectors*/
    FILE		*in_strm = stdin;   
    char		*data_out;	    /*ASCII output file - encoded
					       vectors*/
    FILE		*out_strm = stdout;
    char		*cmd_line;	    /*to hold command line*/
    int			c;		    /*for getopt return*/
    long		tloc;		    /*for local time*/
    long		fea_dim;	    /*numb. columns in feadata*/
    struct vqcbk	*cbk;		    /*the VQ codebook*/
    int			hdetail = 0;	    /*level of history detail*/
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
    double		dist_sd;	    /*standard deviation of distortion*/
    int			dist_err = 0;	    /*error return from distort*/
    int			ncbkrec;	    /*record number of initial codebook*/
    int			c_flag = 0;	    /*flag for -c option*/

/*
 *Initialization
 */
    cmd_line = get_cmd_line(argc, argv);    /*store copy of command line*/    
    input_vec = (float *) calloc(MAX_FEA_DIM, sizeof (float));
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:h:c:l:")) != EOF) {
	switch (c) {
	    case 'x': 
		hdetail = atoi (optarg);
		debug_level = hdetail;
		break;
	    case 'h':
		histfile = optarg;
		break;
	    case 'c':
		ncbkrec = atoi(optarg);
		c_flag++;
		break;
	    case 'l':
		max_line_len = atoi(optarg);
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
        TRYOPEN (argv[0], cbk_fea, "r", cbk_strm);
    }
    else {
	Fprintf(stderr, "vqasc: no FEA codebook file specified.\n");
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
	Fprintf(stderr, "vqasc: no input data file specified.\n");
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
	Fprintf(stderr, "vqasc: no output data file specified.\n");
	SYNTAX;
    }
    TRYOPEN(argv[0], histfile, "w", histrm);
/*
 * read FEA file and get codebook
 */
    if ((cbk_ih = read_header(cbk_strm)) == NULL)
	NOTSPS(argv[0], cbk_fea);
    cbk = allo_vqfea_rec(cbk_ih);
    spsassert (cbk != NULL, "vqasc: NULL returned from allo_vqfea_rec");

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

/*
 * select distortion function
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
    if (hdetail > 0) {
        tloc = time(0);
        Fprintf(histrm, "vqasc history ouput on %s", ctime(&tloc));
        Fprintf(histrm, "vqasc version %s, date %s\n", Version, Date);
        Fprintf(histrm, "command line:\n%s", cmd_line);
    }	
    if (hdetail > 1) {
	Fprintf(histrm, "\nCodebook:\n\n");
	print_fea_rec(cbk->fea_rec, cbk_ih, histrm);
    }
/*
 * main processing goes here
 */
    nfea = max_dist = tot_dist = tot_sqdist = 0.0;
    /*
     *Read in the first vector and make sure something is there
     */
    fea_dim = read_fea_vec(in_strm, *cbk->dimen, input_vec);
    if (fea_dim == 0) ERROR_EXIT("no data in input file\n");
    /*
     *Now keep doing it until there are no more left.
     */
    while (fea_dim != 0) {
	nfea++;
	/*
	 *check dimensions
	 */
	if (fea_dim != *cbk->dimen) {
	    Fprintf(stderr, 
		"vqasc: Dimension of input vector %d doesn't match codebook.\n", nfea);
	    Fprintf(histrm, 
		"vqasc: Dimension of input vector %d doesn't match codebook.\n", nfea);
	    exit(1);
	}
	/*Encode the vector.
	 */
        switch (*cbk->cbk_struct) {
	    case FULL_SEARCH:  
		curr_ind = vqencode(input_vec, *cbk->dimen, cbk->codebook, 
		  *cbk->current_size, *cbk->dimen, &curr_dist, distort, &dist_err);
		break;
	    default:
		ERROR_EXIT("invalid cbk_struct type");
	}
	/*Write out the encoded vector, and do history stuff.
         */
	wrt_fea_vec(out_strm, cbk->codebook[curr_ind], *cbk->dimen);
	tot_dist += curr_dist;
	tot_sqdist += curr_dist * curr_dist;
	if (hdetail > 2) {
	    Fprintf(histrm, "vqasc: encoding input vector %d\n", nfea);
	    if (hdetail > 3) {
		Fprintf(histrm, "     Input vector:\n");
		wrt_fea_vec(histrm, input_vec, *cbk->dimen);
	    }
	    Fprintf(histrm, 
		"     Encoding index = %d, distortion = %g\n",  curr_ind, curr_dist);
	    Fprintf(histrm, 
		"     Current average distortion = %g\n", tot_dist / nfea);
	    if (hdetail > 3) {
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
	/*
	 *Read in the next one and loop back.
         */
	fea_dim = read_fea_vec(in_strm, *cbk->dimen, input_vec);
    }
    if (hdetail > 0) {
	dist_mean = tot_dist / nfea;
	dist_sd = sqrt((tot_sqdist / nfea) - dist_mean * dist_mean);
	Fprintf(histrm, "vqasc: encoded %d input vectors\n", nfea);
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
    *error = 0;
    return msedist / cdwd_dim;
}    

long
read_fea_vec(strm, f_dim, fea_vec)
FILE *strm;
long f_dim;
float *fea_vec;
{
/*This routine reads a line from standard input, converts 
 *whitespace-separated strings to floating point, and stores up
 *to f_dim of them in fea_vec.  The routine's return value is the
 *number of tokens found.
 */
    long nval = 0;
    char *tok;
    static char *input_line = NULL;

    if (input_line == NULL) 
      input_line = (char *) malloc(max_line_len);

    if (fgets(input_line, max_line_len, strm) == NULL) return 0;
    if ((tok = strtok(input_line, " \t\n")) != NULL) {
	nval++;
	*fea_vec++ = atof(tok);
    }
    while ((tok = strtok(0, " \t\n")) != NULL) {
	nval++;
	if (nval <= f_dim) *fea_vec++ = atof(tok);
    }
    return nval;
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



