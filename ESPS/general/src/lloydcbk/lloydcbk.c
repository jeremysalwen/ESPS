/*
| This material contains proprietary software of Entropic Speech, Inc.
| Any reproduction, distribution, or publication without the the prior
| written permission of Entropic Speech, Inc. is strictly prohibited.
| Any public distribution of copies of this work authorized in writing by
| Entropic Speech, Inc. must bear the notice
|
|              "Copyright 1986 Entropic Speech, Inc."
|
| Written by:  David Burton (lloyd.c originally drafted by J.T.Buck)
| Checked by: John Shore  
|
| Module:	lloydcbk.c
|
| This is the main module of the SPS program lloydcbk.  It handles the 
| user-interface, and it calls lloyd.c.
| Lloydcbk takes an ASCII file as input and outputs a SCBK file.
*/

    static char *sccs_id = "@(#)lloydcbk.c	3.4	10/20/93 ESI";

/* 
 * system includes
*/
#include <stdio.h>
/* SDI 27/6/06 changed ifndef to !defined and added LINUX */
#if !defined(DEC_ALPHA) && !defined(LINUX)
char *calloc();
char *realloc();
#endif

/* 
 * ESPS Includes
*/
#include <esps/esps.h>
#include <esps/scbk.h>

/*
 * defines
*/
#define HUGE 5.0e9
#define INCREMENT 1024
#define INIT_SIZE 16384
#define SYNTAX USAGE("lloydcbk [-x debug_level] [-P param_file] [-h history_file] \
[-d distortion] \n [-n codebook_size] [-c convergence] [-o description] [-s source_file] [-t codebook_type] \n [-e element] [-S source_list] infile outfile")

/*
 *System Functions
*/
char *get_cmd_line();
/* SDI 27/6/06 changed ifndef to if !defined and added LINUX */
#if !defined(IBM_RS6000) && !defined(LINUX_OR_MAC)
int fscanf();
#endif
char   *strcpy ();
double  atof ();

/*
 * ESPS functions
*/
char *eopen();
/*
 * Global Variables
*/
short debug_level = 0;
extern char *optarg;
extern  optind;

main (argc, argv)
int     argc;
char  **argv;
{

/* command line variables*/
    char *param_file = NULL;	/* params file name*/
    FILE *histrm = NULL;		/*file pointer for history file*/
    char *hist_file = NULL;	/*history file name*/
    short distortion = SQUARED_ERROR;	/*distortion measure*/
    short distortion2 = -1;		/* variable for params option*/
    int  n;		/*codebook size*/
    int n2 = 0;		/* variable for parms option*/
    double convergence;		/*convergence threshold*/
    double convergence2 = -1.;	/* variable for params option*/
    char *description;		/*contains sequence of commands used to 
				 obtain input data*/
    char **source_file;		/* file(s) that contain the input data*/
    char *source_list;		/* file that contains a list of source files*/
    short cbk_type;		/* parameter that codebook represents*/
    short element;		/* which element of cbk_type*/
    
    double *dp;

    FILE *istrm = stdin;	/* input stream pointer*/
    char *in_file = NULL;	/* holds name of input file*/

    FILE *ostrm = stdout;	/*output stream pointer*/
    char *out_file = NULL;	/*holds output file name*/
    struct header  *oh;		/* points to output codebook header*/
    struct scbk_data *data_record; /*points to codebook data structure*/

    int c;			/* parameter variable*/
    /* these variables are returned by lloyd()*/
    double final_dist;		/* final codebook distortion*/
    double *cdwd_dist;		/* dist. for each codeword*/
    long *final_pop;		/* # of input vectors/codeword*/
    double *cbk;		/* holds codebook*/

    short i;		/* misc. counters*/
    /* option flags*/
    short n_opt = 0;		/* codebook size*/
    short d_opt = 0;		/* distortion type*/
    short c_opt = 0;		/* convergence ratio*/
    short h_opt = 0;		/* history file*/

    /* these variables are used for memory allocation*/
    int init_size = INIT_SIZE; 
    long data_cnt = 0;
    int size_inc = INCREMENT;
    int size_limit = INIT_SIZE;
    long data_limit = INIT_SIZE;

    double *data;		/* holds input data*/
    char *progname = "lloydcbk"; /* holds program name*/
    char *Version = "3.4";	/* Version number*/
    char *Date = "10/20/93";		/* Date*/

    void do_qtable_entry();	/* turns codewords into bin edges*/
    void d_param_err();		/* writes error message*/


/*process options*/
    while ((c = getopt (argc, argv, "x:P:n:c:h:o:s:d:t:e:S:")) != EOF) {
	switch ((char) c) {
	    case 'x': 
		debug_level = atoi(optarg);
		break;
	    case 'P': 
		param_file = optarg;
		break;
	    case 'n':
		n_opt++;
		n = atoi(optarg);
		if(n <= 0)
                {
                  Fprintf(stderr, 
                          "Codebook size (-n%d) must be greater than 0\n", n);
		  exit(1);
                }
		break;
	    case 'c':
		c_opt++;
		convergence = atof(optarg);
		if(convergence < 0.0)
		{
                 Fprintf(stderr, 
		  "Convergence threshold (-c) must be equal to or greater than 0\n");
		 exit(1);
		}
		break;
	    case 'h':
		h_opt++;
		hist_file = optarg;
		break;
	    case 'o':
/*		description = optarg;*/
		(void)fprintf(stderr, 
			"descrption option (-o) is not supported yet\n");
		break;
	    case 's':
/*		**source_file = optarg;
		(**source_file)++;
*/
		Fprintf(stderr, "Source file (-s) option not supported yet\n");
		break;
	    case 'd':
		d_opt++;
		if ((distortion = lin_search(scbk_distortion, optarg)) == -1)
		{
			Fprintf(stderr,
			        "Only SQUARED_ERROR distortion supported\n");
			exit(1);
		}
		break;
	    case 't':
		if((cbk_type = lin_search(scbk_codebook_type, optarg)) == -1)
		{
			Fprintf(stderr, "illegal codebook type\n");
			exit(1);
		}
		break;
	    case 'e':
		element = atoi(optarg);
		break;
	    case 'S':
/*		*source_list = optarg; */
		Fprintf(stderr, "Source list option (-S) is not supported yet\n");
		break;
	    default: 
		SYNTAX;
	}
    }

   
/*determine and open input source files*/
/* this is where -s and -S stuff would go*/
/*don't forget to chexk for quantized data if the source files are ANA files*/
/* read header of input files
use something like this

    if ((ih = read_header (istrm)) == NULL) {
    NOTSPS (argv[0], in_file);
    }
    if (ih->common.type == FT_ANA) { 
check if parameters are quantized
add header to SCBK file
do next one
    
*/


/* open input/output files */
     if (optind < argc) {
	in_file = argv[optind++];
	if (strcmp (in_file, "-") == 0)
	    in_file = "<stdin>";
	else
	    TRYOPEN (argv[0], in_file, "r", istrm);
	}
    else {
	Fprintf(stderr, "lloydcbk: no input file specified.\n");
	SYNTAX;
        }
    if (optind < argc) {
	out_file = eopen("lloydcbk", argv[optind++], "w", NONE, NONE,
		    (struct header**)NULL, &ostrm);
	}
    else {
	Fprintf(stderr, "lloydcbk: no output file specified.\n");
	SYNTAX;
        }

    if (debug_level) {
	Fprintf(stderr, 
	"lloydcbk: input file = %s, output file = %s\n", in_file, out_file);
	}

/*read parameter file and check for valid options*/

      (void)read_params(param_file, SC_NOCOMMON, (char *)NULL);
	if(symtype("cbk_size") != ST_UNDEF)
             n2 = getsym_i("cbk_size"); 
	if(symtype("convergence") != ST_UNDEF)
	     convergence2 = getsym_d("convergence"); 
	if(symtype("distortion") != ST_UNDEF){
             if ((distortion2 = lin_search(scbk_distortion, 
	     getsym_s("distortion"))) != SQUARED_ERROR) 
	        d_param_err(getsym_s("distortion"), param_file);
	}  

/*Command line options override parameter file*/
    if (n_opt == 0) n = n2;
    if (c_opt == 0) convergence = convergence2;
    if (d_opt == 0) distortion = distortion2;

    if (debug_level > 0) {
	Fprintf(stderr, 
	    "lloydcbk: Codebook size is %d\n", n);
	Fprintf(stderr, 
	    "lloydcbk: Convergence threshold is %f\n", convergence);
	Fprintf(stderr, 
	    "lloydcbk: History file is %s\n", hist_file);
	Fprintf(stderr, "lloydcbk: Distortion measure is %d\n", distortion);
	Fprintf(stderr, "lloydcbk: Codebook type is %d\n", cbk_type);
	Fprintf(stderr, "lloydcbk: Element number is %d\n", element);
	Fprintf(stderr, "lloydcbk: debug_level = %d\n", debug_level);
    }

/* check if parameters were read from parameter file and are valid */
    if(n2 > 0) n_opt++;
    if(convergence2 >= 0) c_opt++;
    if(distortion2 > 0) d_opt++;

/* Check if enough options were specified*/
    if(!(n_opt && c_opt && d_opt))
    {
	(void)fprintf(stderr, 
              "Codebook size, convergence threshold, distortion measure,\n");
	(void)fprintf(stderr,"must be specified\n");
         exit(1);
    }


/* allocate memory for variable parts of data arrays */

   cdwd_dist = (double *) calloc ((unsigned)n, sizeof(double));
    spsassert(cdwd_dist != NULL, "Could not allocate cdwd_dist memory")
   final_pop = (long *)   calloc ((unsigned)n, sizeof(long));
    spsassert(final_pop != NULL, "Could not allocate final_pop memory");
   cbk = (double *) calloc ((unsigned)n, sizeof(double));
    spsassert(cbk != NULL, "could not allocate cbk memory");

/*open history file and output headings*/
    if (h_opt) {
          TRYOPEN(argv[0], hist_file, "w", histrm);

	  Fprintf(histrm,
		"\t\t\t\tHISTORY FILE FOR lloydcbk\n");
	  Fprintf(histrm, " version %s, date %s\n", Version, Date);
	  Fprintf(histrm, 
	        " input file = %s, output file = %s\n", in_file, out_file);
	  Fprintf(histrm, " Distortion measure is %d\n", distortion);
	  Fprintf(histrm, " Codebook type is %d\n", cbk_type);
	  Fprintf(histrm, " Element number is %d\n", element);
	}


/*create and write header of output file*/
    oh = new_header(FT_SCBK);

/*add comment containing command line and other output header stuff*/
    (void)add_comment(oh, get_cmd_line(argc, argv));

    (void) strcpy (oh->common.prog, progname);
    (void) strcpy (oh->common.vers, Version);
    (void) strcpy (oh->common.progdate, Date);

    oh->hd.scbk->distortion = distortion;
    oh->hd.scbk->num_cdwds = n;
    oh->hd.scbk->convergence = convergence;
    oh->hd.scbk->codebook_type = cbk_type;
    oh->hd.scbk->element_num = element;

/* still need to compute num_items and store in hd.scbk->num_items*/
/* and then write_header */

/*read input data and store in data array*/

/*allocate initial block of memory for data*/
    data = (double *) calloc((unsigned)init_size, sizeof (double) );
    spsassert(data != NULL, "Could not allocate memory for data");
    dp = data;
    while( (fscanf(istrm, "%lf", dp++)) != EOF) 
    {
       data_cnt++;
       if(data_cnt == data_limit) /*allocate additional memory for data array*/
       {
	  data_limit += size_inc;
	  size_limit += size_inc;
          data = (double *)  
	    realloc ((char *) data, (unsigned)(size_limit * sizeof(double)));
	spsassert(data != NULL, "Could not allocate memory for data");
	  dp = data + data_cnt;
       }
    }

/*print data array, if we're in big trouble*/
   if(debug_level > 4)
   {
      (void)fprintf(stderr, "data is \n");
      for(i=0; i<data_cnt; i++)
          (void)fprintf(stderr, "data[%d] is %f\n", i, data[i]);
      (void)fprintf(stderr, "\n");
   } 

/* add num_items to output header and allocate memory for data record*/

   oh->hd.scbk->num_items = data_cnt;
   write_header(oh, ostrm);
   data_record = allo_scbk_rec(oh);

      /* choose which distortion measure to use and design codebook*/

    switch (distortion) {
	    case SQUARED_ERROR: /* only one supported at this time*/
                if(debug_level > 1)
		{
   		   (void)fprintf(stderr,
                   "lloydcbk:data_cnt is %d, n is %d, convergence is %f\n",
                    data_cnt, n, convergence);
		}

                (void)lloyd(data, data_cnt, cbk, (unsigned)n, convergence, 
			histrm, &final_dist, cdwd_dist, final_pop);
		break;
	    default: {
		Fprintf(stderr, "lloydcbk: invalid distortion measure type %s\n",
		scbk_distortion[distortion]);
		exit(1);
		}
	}

/* fill in scbk data record*/
  data_record->final_dist = (float)final_dist;
  for(i=0; i<n; i++){
     data_record->cdwd_dist[i] = (float)cdwd_dist[i];
     data_record->final_pop[i] = final_pop[i];
   }
/* do qtable stuff*/
    do_qtable_entry(cbk, (unsigned)n, data_record);

    /*write out record*/
	if (debug_level > 1) {
	    Fprintf(stderr, "output scbk record:\n");
	    print_scbk_rec(data_record, oh, stderr);
	}

	put_scbk_rec(data_record,oh,ostrm);


/*clean up and exit*/
    exit (0);
    return(0);
}

void
do_qtable_entry(codebook, num_of_cdwds, data_struct)
double codebook[];
unsigned num_of_cdwds;
struct scbk_data *data_struct;
{
unsigned i;

for(i=0; i<num_of_cdwds; i++) 
{
   if(i != num_of_cdwds-1)
       data_struct->qtable[i].enc = (float)(codebook[i] + codebook[i+1])/2;
   data_struct->qtable[i].dec = (float)codebook[i];
   data_struct->qtable[i].code = i;
}

data_struct->qtable[num_of_cdwds-1].enc = HUGE;
}

void
d_param_err(dtype, p_file)
char *dtype;
char *p_file;
{
    Fprintf(stderr, "lloydcbk: unknown distortion type %s\
 in parameter file %s\n", dtype, p_file);
    exit (1);
}
