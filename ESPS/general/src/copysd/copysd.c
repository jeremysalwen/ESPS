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
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:  David Burton, John Shore
 *
 * Brief description: copies FEA_SD records to new file (with type change)
 *
 */

static char *sccs_id = "@(#)copysd.c	3.15	1/18/97	ESI/ERL";

#define VERSION "3.15"
#define DATE "1/18/97"
/*
 * system and ESPS include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>
#include <sys/param.h>

/*
 * private definitions
 */
#define SYNTAX \
USAGE("copysd [-a add_constant] [-d data_type] [-g] [-o] [-{pr} range] [-s scale]\n\t[-x debug_level] [-S time_range] [file1 [file2 ...]] outfile")
#define ERROR_EXIT(text) \
{(void) fprintf(stderr, "copysd: %s - exiting\n", text); exit(1);}
#define REQUIRE(test, text) {if (!(test)) ERROR_EXIT(text)}
#define SIZE 8192		/* size of copy buffer */
#define MAX_IN_FILES 100	/* max number of input files */

/*
 * system functions and variables
 */

extern int  optind;
char *optarg;
double fabs();

/*
 * external ESPS functions
 */
char *eopen();
char *get_cmd_line();
void lrange_switch(), trange_switch();
double_cplx realmult();
double_cplx realadd();

/*
 * global declarations
 */
void get_range();
int debug_level = 0;

/*
 * main program
 */
main (argc, argv)
int     argc;
char  **argv;
{
    int     getopt(), c;		/* for option processing */
    FILE    *out_fd;			/* output stream */
    FILE    *in_fd[MAX_IN_FILES];	/* input streams */
    struct header *oh;			/* output file header */
    struct header *ih[MAX_IN_FILES];	/* input file headers */
    char    *ofile;			/* output file name */
    char    *in_files[MAX_IN_FILES];	/* input file names */
    struct feasd *sd_rec;		/* fea_sd data record */
    int     i, j;			/* loop variables */
    int	    oflag = 0, dflag = 0,	/* option flags */ 
	    pflag = 0, Sflag = 0,
	    sflag = 0, aflag = 0,
	    gflag = 0;
    int	    nofile = 0;			/* set if no input file on cmd line */
    long    s_rec, e_rec;		/* start, and end point */
    long    lrec[MAX_IN_FILES];		/* actual end point for each file */
    char    *range = NULL;		/* string from -r option */
    short   type;			/* output data type */
    char    combuf[100 + MAXPATHLEN];	/* string for comment */
    double  scale = 1.0;		/* scale factor from -s option */
    double  add_constant = 0;	        /* additive constant from -a option */
    int	    havestdin = 0;		/* flag set when one input is stdin */
    int	    nfiles;			/* number of input files */
    int     n;				/* number points wanted from 
					   call to get_sd_rec */
    int	    n_read;			/* number points from get_sd_rec */
    long    n_to_go;			/* number left in range */
    int	    tot_rec = 0;		/* total records read (all files)*/
    int	    tot_rec_file;		/* total records read (current file)*/
    int	    nan_first;			/* number points copied from 1st file*/
    double  first_sf;			/* samp. freq. of first input file */

    double_cplx *cdata;			/* pointer to input data records */
    short   in_type;			/* input type for file 1 */
    int	    num_channels;		/* number of channels in input files */
    double  start_time = 0;		/* waves generic */
    char    data_type;			/* pointer for output data type */

/*
 * process command line options
 */
    while ((c = getopt(argc, argv, "a:d:op:r:s:x:S:g")) != EOF) {
	switch (c) {
	case 'a': 
	    add_constant = atof (optarg);
	    aflag++;
	    break;
	case 'd': 
	    dflag++;
	    data_type = optarg[0];
	    if((type = (short)lin_search(type_codes, optarg)) == -1){
		switch (data_type) {
		case 'b':
		    type = BYTE;
		    break;
		case 's':
		    type = SHORT;
		    break;
                case 'l':
		    type = LONG;
		    break;
		case 'f':
		    type = FLOAT;
		    break;
                case 'd':
		    type = DOUBLE;
		    break;
		default:
		    Fprintf(stderr,
			    "copysd: invalid output data type (%s) specified\n",
			    optarg);
                    exit(1);
		}
	    }
	    break;
	case 'o': 
	    oflag++;
	    break;
	case 'p': 
	case 'r': 
	    range = optarg;
	    pflag++;
	    break;
	case 's': 
	    scale = atof (optarg);
	    if (strchr(optarg, ':')) Fprintf(stderr, 
		"copysd: WARNING - `:' in argument for scale option -s; maybe you want -S\n");
	    sflag++;
	    break;
	case 'x': 
	    debug_level = atoi(optarg);
	    break;
	case 'S': 
	    range = optarg;
	    Sflag++;
	    break;
	case 'g':
	    gflag = 1;		/* copy all generics from first input file */
	    break;
	default:
	    SYNTAX;
	}
    }

/*
 * check for multiple range specifications
 */
    if (pflag + Sflag > 1) {
	ERROR_EXIT("range should only be specified once");
    }


/* Determine and open output file
 */
    if (argc - optind < 1) {
	Fprintf(stderr, "copysd: no output file specified.\n");
	SYNTAX;
    }
    ofile = eopen("copysd", argv[argc - 1], "w", NONE, NONE, &oh, &out_fd);
    if (debug_level)
	Fprintf(stderr, "copysd: output file is %s\n", ofile);
/*
 * Determine input files and check them -- start by getting the first 
 * one; at the same time, we set up to create the output header
 */
    if (argc - optind < 2) {
	nofile = 1;  
	in_files[0] = NULL;
	if (debug_level)
	    Fprintf(stderr, "copysd: no input file on command line\n");
    }
    else {
	in_files[0] = eopen("copysd", argv[optind], "r", FT_FEA, FEA_SD, 
	    &ih[0], &in_fd[0]);
	optind++;	
    }
    /*
     *  If command line has no input file name, then it might be in
     *  ESPS common; we need to read_params anyway (provided stdin not
     *  used), so we do it here (checking common only); However,
     *  we don't check common if more than one input file is given
     *  since this is error prone.  
     */
    if (nofile || ((in_fd[0] != stdin) && (argc - optind < 2))) {
	if (debug_level)
	    Fprintf(stderr, "copysd: checking ESPS common\n");
    	(void) read_params((char *)NULL,SC_CHECK_FILE,in_files[0]);	
    }
    if (nofile) {
	if (symtype("filename") == ST_UNDEF) {
	    ERROR_EXIT("no input file name on command line or in common");
	}
	else {
	    if (debug_level)
		Fprintf(stderr, 
		    "copysd: input file name from common is %s\n",
		    getsym_s("filename"));
	    if (strcmp(ofile, getsym_s("filename")) == 0)
		ERROR_EXIT("input name from common same as output file");
	    in_files[0] = eopen("copysd", getsym_s("filename"), "r", 
		FT_FEA, FEA_SD, &ih[0], &in_fd[0]);
	}
    }
    if (in_fd[0] == stdin) havestdin = 1;
    if (debug_level) 
	Fprintf(stderr, "copysd: first input file is %s\n", in_files[0]);

/*
 * create and write output header
 */
    num_channels = 
         get_fea_siz("samples", ih[0], (short *) NULL, (long **) NULL);
    first_sf = get_genhd_val("record_freq", ih[0], -1.0);
    oh = new_header(FT_FEA);
    (void) strcpy(oh->common.prog, "copysd");
    (void) strcpy(oh->common.vers, VERSION);
    (void) strcpy(oh->common.progdate, DATE);
    oh->common.tag = NO;
    in_type = get_fea_type("samples", ih[0]);
    if (!oflag) add_source_file(oh, in_files[0], ih[0]);
    if (aflag) 
        *add_genhd_d("add_constant", (double *) NULL, 1, oh) = add_constant;
    if (sflag) 
        *add_genhd_d("scale_factor", (double *) NULL, 1, oh) = scale;

    /*
     * Now open the rest of the input files and process headers
     */
    nfiles = 1;
    while(optind < argc - 1) {
	in_files[nfiles] = eopen("copysd", argv[optind], "r", FT_FEA, FEA_SD, 
	    &ih[nfiles], &in_fd[nfiles]);
	if (debug_level) 
	    Fprintf(stderr, "copysd: input file is %s\n", in_files[nfiles]);
	if (in_fd[nfiles] == stdin) {
	    if (havestdin) {
		 ERROR_EXIT("only one stdin allowed");
	    }
	    else
		havestdin = 1;
	}
	/*
	 * Check input files for consistency
	 */
	if (get_fea_siz("samples", ih[nfiles], (short *) NULL, (long **) NULL)
	    != num_channels)
	{
	    Fprintf(stderr, 
		    "copysd: all files must have same number of channels\n");
    	    Fprintf(stderr,
		    "   first file has %d channels, %s has %d\n",
		    num_channels, in_files[nfiles],
		    get_fea_siz("samples",
				ih[nfiles], (short *) NULL, (long **) NULL));
	    exit(1);
        }

 	if (get_genhd_val("record_freq", ih[nfiles], 0.0) != first_sf) {
    	    Fprintf(stderr,
		    "copysd: all files must have same sampling frequency\n");
    	    Fprintf(stderr,
		    "   freq. in first file is %g, freq. in %s is %g\n",
		    first_sf, in_files[nfiles],
		    get_genhd_val("record_freq", ih[nfiles], 0.0));
	    exit(1);
 	}

	if (!oflag) add_source_file(oh, in_files[nfiles], ih[nfiles]);
	optind++;
	nfiles++;
	if (nfiles > MAX_IN_FILES) 
	    ERROR_EXIT("can't handle more than %d files");
    }
    add_comment (oh, get_cmd_line(argc,argv));
    /*
     * determine range of records to copy
     */
    get_range(&s_rec, &e_rec, range, pflag, Sflag, ih[0]); 
    if (debug_level) Fprintf(stderr,  
	 "copysd: full range is %ld to %ld\n", s_rec, e_rec);
    /*
     * determine copy range for each file and add comments accordingly;
     * can do this exactly for each file that's not a pipe;
     * at the same time, fill out start and nan based on first file
     */
    for (i=0; i < nfiles; i++) {
	lrec[i] = e_rec;
	if (ih[i]->common.ndrec > -1) {
	    /*
	     * not a pipe and record length fixed---can count on common.drec
	     */
	    if (lrec[i] > ih[i]->common.ndrec) 
		lrec[i] = ih[i]->common.ndrec;
	    if (s_rec > lrec[i]) Fprintf(stderr, 
		"copysd: WARNING - no records in range for file %s\n",
		in_files[i]);
	}   /* else it's a pipe, or record length is variable---
	     * go with e_rec, whatever it is
	     */
	(void) sprintf(combuf, "  Copied from %s, records %ld to %ld\n",
		       in_files[i], s_rec, lrec[i]);
	add_comment (oh, combuf);

	if (debug_level)
	    Fprintf(stderr, "copysd: range for %s is %ld to %ld\n", 
			    in_files[i], s_rec, lrec[i]);
    }

    /*
     * Allocate data record and set up pointer---same for all input files.
     * This depends on the fact that nothing in the feasd record structure
     * except the number of channels depends on the header, and we have
     * checked that all the headers have the same number.
     */
    sd_rec = allo_feasd_recs(ih[0], DOUBLE_CPLX,
			     (long) SIZE, (char *) NULL, NO);
    cdata = (double_cplx *) sd_rec->data;

    /*
     * write the header
     */
    if (!dflag)
	type = in_type;

    REQUIRE(init_feasd_hd(oh, type,
			  num_channels, &start_time, NO, first_sf) == 0,
	    "Couldn't initialize output FEA_SD header")

    if (gflag)
	copy_genhd(oh, ih[0], NULL);
    (void) update_waves_gen(ih[0], oh, (float)s_rec, (float)1);

    write_header(oh, out_fd);
/*
 * copy records from each input file
 */
    for (i=0; i < nfiles; i++) {
	if (debug_level)
	    Fprintf(stderr, "copysd: copying file %s\n", in_files[i]);
	tot_rec_file = 0;
	(void) fea_skiprec (in_fd[i], s_rec - 1, ih[i]);
	n = SIZE;
	n_to_go = lrec[i] - s_rec + 1;
	if (debug_level > 2)
	    Fprintf(stderr, 
		"copysd: need total of %ld in chunks of %d\n", n_to_go, SIZE);
	while (n_to_go > 0)
	{
	    long    num_elements;

	    if (n_to_go < n)
		n = n_to_go;
	    n_read = get_feasd_recs(sd_rec, 0L, (long)n, ih[i], in_fd[i]);
	    if (debug_level > 2) 
		Fprintf(stderr, 
		    "copysd: asked for %ld samples, got %ld\n", n, n_read);
	    if (n_read == 0) /*read all there is to read*/
		break;

	    num_elements = n_read * num_channels;
	    if (sflag && aflag)
		for (j = 0; j < num_elements; j++)
		    cdata[j] = realadd(realmult(cdata[j], scale), add_constant);
	    else if (sflag) 
		for (j = 0; j < num_elements; j++)
		    cdata[j] = realmult(cdata[j], scale);
	    else if (aflag)
		for (j = 0; j < num_elements; j++)
		    cdata[j] = realadd(cdata[j], add_constant);


	    REQUIRE(put_feasd_recs(sd_rec,
				   0L, (long) n_read, oh, out_fd) == 0,
		    "Couldn't write output data record")

	    n_to_go -= n_read;
	    tot_rec_file += n_read;
    	    if (debug_level > 2)
		Fprintf(stderr, 
		    "copysd: %ld records so far, %ld to go\n",
		    tot_rec_file, n_to_go);
	    if (n_read < n) /*read all there is to read*/
		break;
	}
	if (i == 0) nan_first = tot_rec_file;
	tot_rec += tot_rec_file;
	if (debug_level > 1) {
	    Fprintf(stderr, 
		    "copysd: copied %ld total records from file %s\n", 
		    tot_rec_file, in_files[i]);
	    Fprintf(stderr, 
		    "copysd: grand total records so far = %ld\n", tot_rec);
	}    
	if (tot_rec_file == 0) Fprintf(stderr, 
	    "copysd: WARNING - no records copied for file %s\n", in_files[i]);
    }
    (void) fclose (out_fd);
/*
 * put output file info in ESPS common
 */
    if (strcmp(ofile, "<stdout>") != 0) {
	(void)putsym_s("filename",in_files[0]);
        (void)putsym_s("prog",argv[0]);
        (void)putsym_i("start",(int)s_rec);
        (void)putsym_i("nan",(int) nan_first);
    }
    if (debug_level) Fprintf(stderr, "copysd: normal exit\n");
    exit(0);
}

void
get_range(srec, erec, rng, pflag, Sflag, hd)
/*
 * This function facilitates ESPS range processing.  It sets srec and erec
 * to their parameter/common values unless a range option has been used, in
 * which case it uses the range specification to set srec and erec.  If
 * there is no range option and if start and nan do not appear in the
 * parameter/common file, then srec and erec are set to 1 and LONG_MAX.
 * Get_range assumes that read_params has been called; If a command-line
 * range option (e.g., -r range) has been used, get_range should be called
 * with positive pflag and with rng equal to the range specification.
 */
long *srec;			/* starting record */
long *erec;			/* end record */
char *rng;			/* range string from range option */
int pflag;			/* flag for whether -r or -p used */
int Sflag;			/* flag for whether -S used */
struct header *hd;		/* input file header */
{
    long common_nan;

    *srec = 1;
    *erec = LONG_MAX;
    if (pflag) 
	lrange_switch (rng, srec, erec, 1);	
    else if (Sflag) 
	trange_switch (rng, hd, srec, erec);	
    else {
	if(symtype("start") != ST_UNDEF) {
	    *srec = getsym_i("start");
	}
	if(symtype("nan") != ST_UNDEF) {
	    common_nan = getsym_i("nan");
	    if (common_nan != 0) 
		*erec = *srec + common_nan - 1; 
	}
    }
}

