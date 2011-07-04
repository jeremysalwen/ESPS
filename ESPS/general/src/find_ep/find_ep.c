/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Burton
 * Checked by:
 * Revised by:
 *
 * This program locates the endpoints of a speech segment
 * in a sampled-data file.
 */

static char *sccs_id = "@(#)find_ep.c	1.14	5/12/97	ESI/ERL";
#define VERSION "1.14"
#define DATE "5/12/97"

/* system Includes*/
#include <stdio.h>
#include <math.h>

/*ESPS Includes*/
#include <esps/esps.h>
#include <esps/ftypes.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>

#define SYNTAX \
USAGE("find_ep [-b adbits] [-c context] [-e] [-f final_thresh]\n [-h high_thresh] [-l low_thresh] [-n] [-p start_point] [-r start_point]\n [-s silence_req] [-t time] [-w] [-x debug-level] [infile.sd] [outfile.sd]")


/* system functions*/
/*done via <esps/unix.h>*/

/* ESPS Functions */
void	set_sd_type();
int	get_sd_type();
void	fea_skiprec();
int	getopt();
int	read_params();
void	symerr_exit();
int	get_sd_recd();
void	add_comment();
void	put_sd_recd();
int	putsym_i();
int	putsym_s();
int	getsym_i();
char	*getsym_s();
char	*eopen();

/* Global Variables */

int	debug_level = 0;	/* debugger flag */

extern  optind;
extern	char *optarg, *get_cmd_line();

main (argc, argv)	
int     argc;
char  **argv;
{
    int		c;			/*parameter parsing*/

    FILE 	   *f1p, *f2p;		/* file steam pointers */
    struct header  *f1h, *f2h;		/* pointers to header structure */
    char	   *f1 = NULL, *f2 = NULL; /* filename pointers */

    int		lflag = 0;		/* low threshold flag */
    int		hflag = 0;		/* high threshold flag */
    int		fflag = 0;		/* final threshold flag*/
    int		nflag = 0;		/* next segment flag */
    int		eflag = 0;		/* supress error flag */
    int		wflag = 0;		/* write output file flag */

    float	scalad = 1.;		/* 2**(adbits - 12) */
    int		error = 0;		/* error status */
    int		num_of_files = 0;	/* number of command line files*/    
    int		adbits = 12;		/*number of bits in A/D converter*/
    long	start = 1;		/* starting point in file */
    float	sil_req = 200.;		/* silence required between words*/
    float	low_thresh = 0.;	/* variable for first threshold */
    float	high_thresh = 0.;	/* variable for second threshold */
    float	final_thresh = 0.;	/* variable for end threshold */    
    float	sf = 8000.;		/* input file sampling frequancy */

    float	context = 10.;		/* internal frame size in
							    milliseconds */
    float	time = 150.;		/* time period for computing
							    statistics */

    double	*buff;		/* buffer for writing output data */
    long	s_pt; 		/* starting record position */
    long	e_pt;		/* ending record position */

    int	    	endpoints();	/* function that finds endpoints */
    void	error_exit();	/* function that prints error messages*/

/* get options from command line and set various flags */

    while ((c = getopt (argc, argv, "x:r:p:s:l:h:f:c:t:b:nwe")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		if(debug_level > 0)
		    Fprintf(stderr, "find_ep: debug_level = %d\n",
		    debug_level);
		break;
            case 'r':
	    case 'p': 
    		start = atoi (optarg);
		if(start < 1){
		    Fprintf(stderr, 
		    "find_ep: starting point must be >= 1\n");
		    exit(1);
		}
		break;
	    case 's': 
		sil_req = atof (optarg);
		break;
	    case 'l': 
		lflag++;
		low_thresh = atof( optarg);
		if(low_thresh <= 0){
		    Fprintf(stderr, "find_ep: Low_thresh must be > 0\n");
		    exit(1);
		}
		break;
	    case 'h':
		hflag++;
		high_thresh = atof( optarg);
		break;
	    case 'f':
		fflag++;
		final_thresh = atof( optarg);
		break;
	    case 'c':
		context = atof( optarg);
		if(context < 1){
		    Fprintf(stderr, 
		    "find_ep: Context must be >= 1 msec.\n");
		    exit(1);
		}
		break;
	    case 't': 
		time = atof( optarg);
		break;
	    case 'b':
		adbits = atoi(optarg);
		scalad = (float)( pow( (double)(2.), (double)(adbits - 12) ) );
		break;
	    case 'n':
		nflag++;
		break;
	    case 'e':
		eflag++;
		break;
	    case 'w':
		wflag++;
		break;
	    default:
		SYNTAX;
	}
    }

    /*
     * Check for some inconsistencies
    */

    if( lflag && fflag) /* both low and final threshold specified*/
	if(final_thresh >= low_thresh){
	    Fprintf(stderr, "find_ep: final_thresh must be < low_thresh\n");
	    exit(1);
	}

    if( time < context){
	Fprintf(stderr, 
	"Time interval for statistics (-t) must be >= context (-c)\n");
	exit(1);
    }

/* find out how many files are specified and open appropriate files */

    num_of_files = (argc - optind);

    if(num_of_files > 1 && !wflag ) /*too many files specified*/
    {
	SYNTAX;
	exit(1);
    }

    if(debug_level > 0)
	Fprintf(stderr, "find_ep: Number of files = %d\n", num_of_files);

/*
 * Read Common to define symbols
*/
    (void)read_params((char *)NULL, SC_CHECK_FILE, (char *)NULL);

 /*
  * Get input and output file names
 */

    /* 
     * No file specified, get input file name from Common
    */
    
    if(num_of_files == 0 ){

	/*
	* Get input file name 
	*/
        if(symtype("filename") == ST_UNDEF){
	    Fprintf(stderr, "find_ep: No input file specified\n");
	    SYNTAX;
	    exit(1);
	}
	else
	    f1 = getsym_s("filename");
    }

    /* 
     * Only one file specified, decide whether it is input or output 
     */

    if(num_of_files == 1){
	if(wflag){/*specified file is output filename - get input
		    filename from ESPS Common*/
	    f2 = argv[optind++];

	    /* 
	     * Get input file name from Common
	     */
	    if(symtype("filename") == ST_UNDEF){
		Fprintf(stderr, "find_ep: No input file in ESPS Common\n");
	        exit(1);
	    }
	    else
		f1 = getsym_s("filename");
	}	    

	else {/* specified file is input filename - no output file */

	    f1 = argv[optind++];
	    f2 = "\"none specified\"";
	}
    }
    /* 
     * both input and output files are specified
     */

    if(num_of_files == 2){
	f1 = argv[optind++];
	f2 = argv[optind++];
    }

    symerr_exit();

    if(debug_level > 0){
	Fprintf(stderr, "find_ep: Input file is %s\n", f1);
	Fprintf(stderr, "find_ep: Output file is %s\n", f2);
    }

/* 
 *open input file 
*/

    f1 = eopen("find_ep", f1, "r", FT_FEA, FEA_SD, &f1h, &f1p);

/*
 * if needed, open output file
*/

    if(wflag && (strcmp(f1, "<stdin>")== 0)){
	Fprintf(stderr,"find_ep: -w option cannot be used with stdin\n");
	exit(1);
    }

    if(wflag){
	f2 = eopen("find_ep", f2, "w", NONE, NONE, &f2h, &f2p);
	if(strcmp(f2, "<stdout>") == 0){
	    Fprintf(stderr, "find_ep: Standard output cannot be used\n");
	    exit(1);
	}
    }
/* 
 * check if input file is multichannel
*/
    if(get_fea_siz("samples", f1h,(short *) NULL, (long **) NULL) != 1)
    {
	Fprintf(stderr,
		"find_ep: Multichannel data not supported yet - exiting.\n");
	exit(1);
    }

/*
 * Check if input data is complex
*/
    if(is_field_complex(f1h, "samples") == YES)
    {
	Fprintf(stderr, "find_ep: Complex data not supported - exiting.\n");
	exit(1);
    }

/*
 * Get sampling frequency from input file
*/
    sf = (float)get_genhd_val("record_freq",f1h, (double)8000.);

    if(debug_level > 0)
	Fprintf(stderr, "find_ep: Sampling frequency = %f\n", sf);

/*
 * if -n specifed, Get starting point from common if possible
*/
    if(nflag && ( strcmp(f1, "<stdin>") == 0 ) ){
	/* 
	 * standard input and -n cannot be used together
	*/
	Fprintf(stderr, 
	"find_ep: Standard input and -n option cannot both be used\n");
	exit(1);
    }

    if (nflag)
    {
	if (symtype("filename") == ST_UNDEF)
	{
	    Fprintf(stderr, "find_ep: no filename in ESPS Common\n");
	    exit(1);
	}
	if (strcmp(f1, getsym_s("filename")) != 0)
	{
	    Fprintf(stderr, 
	    "find_ep: ESPS Common filename does not match input filename\n");
	    exit(1);
	}

	if (symtype("start") == ST_UNDEF)
	{
	    Fprintf(stderr, "find_ep: No start value in Common\n");
	    exit(1);
	}
        start = getsym_i("start");

	if (symtype("nan") == ST_UNDEF)
	{
	    Fprintf(stderr, "find_ep: No nan value in Common\n");
	    exit(1);
	}
	start += getsym_i("nan");
	start += ROUND(sf * .05) + 1;/*move 50 msec 
					beyond end of last segment*/
	/* check for errors */
	symerr_exit();

	if (debug_level > 0)
	    Fprintf(stderr, 
		    "find_ep: -n option used; starting point from common\n");
    }

    if(debug_level > 0)
	Fprintf(stderr, "find_ep: Start value = %ld\n", start);

    /*
     * Skip forward to the starting point
    */

    if(debug_level > 3)
	Fprintf(stderr, "find_ep: Size of data record = %d\n", 
		(size_rec(f1h)));

    fea_skiprec(f1p, start-1, f1h);


    /*
     * Setups all done; go find endpoints
    */
    error = 0;
    error = endpoints(f1h, f1p, start, &s_pt, &e_pt, sf, scalad,  
	    sil_req, low_thresh, hflag, high_thresh, fflag, 
	    final_thresh, context, time);


    /*
     * Check error status; if > 0, problem finding endpoints
    */
    if(debug_level > 0)
	Fprintf(stderr, "find_ep: Error status = %d\n", error);

    if(error > 0)
	error_exit(error, eflag);

    /*
     * write output file, if requested
     */


    if(wflag){

	if(debug_level > 0){
	    Fprintf(stderr, "find_ep: Writing output file\n");
	    Fprintf(stderr,  "find_ep: start = %ld,  end = %ld\n", 
		s_pt, e_pt);
	}
	/*
	 *  make header
	*/

	f2h = copy_header(f1h);
	(void)strcpy(f2h -> common.prog, "find_ep");
	(void)strcpy(f2h -> common.vers, VERSION);
	(void)strcpy(f2h -> common.progdate, DATE);
	add_source_file(f2h, f1, f1h);
	(void)add_comment(f2h, get_cmd_line(argc, argv));
	update_waves_gen(f1h, f2h, (float)s_pt, 1.0);

	write_header(f2h, f2p);
	/*
	 * get points from input file
	*/

	rewind(f1p);
	f1h = read_header(f1p);
	fea_skiprec(f1p, s_pt-1, f1h);
	/* 
	 * check if number of points is greater than an int
	*/
	if( (int)(e_pt-s_pt+1) != (e_pt-s_pt+1) ){
	    Fprintf(stderr, "Data segment is too big to copy\n");
	    Fprintf(stderr, "Try without the -w option\n");
	    exit(1);
	}

	buff = (double *)calloc((unsigned)(e_pt-s_pt+1), sizeof(double));
	spsassert(buff != NULL, "Couldn't allocate space for buff");

	if(get_sd_recd(buff, (int)(e_pt-s_pt+1), f1h, f1p) != (e_pt-s_pt+1)){
	    Fprintf(stderr, 
	    "find_ep: Bogus starting (%ld) or end (%ld) points found\n", 
			s_pt, e_pt);
	    Fprintf(stderr, "find_ep: Problem with find_ep program\n");
	    exit(1);
	}
	/* write output data */
	put_sd_recd(buff, (int)(e_pt-s_pt+1), f2h, f2p);
    }

    /*
     * Set start and nan in ESPS Common
    */
    /* I would have used putsym_l (put sym long), if it existed*/

    if((putsym_i("start", (int)s_pt)) == -1){
	Fprintf(stderr, 
	"find_ep: START could not be written to ESPS Common\n");
	exit(1);
    }

    if(debug_level > 0)
	Fprintf(stderr, "find_ep: start (%ld) written to ESPS Common\n", 
	s_pt);

    if((putsym_i("nan", (int)(e_pt-s_pt+1))) == -1){
	Fprintf(stderr, 
	"find_ep: NAN could not be written to ESPS Common\n");
	exit(1);
    }

    if(debug_level > 0)
	Fprintf(stderr, "find_ep: Nan (%ld)written to ESPS Common\n", 
	    e_pt - s_pt +1);

    if((putsym_s("prog", "find_ep")) == -1){
	Fprintf(stderr, 
	"find_ep: PROGRAM Name could not be written to ESPS Common\n");
	exit(1);
    }

    if(!nflag)
        if((putsym_s("filename", f1)) == -1){
	    Fprintf(stderr, 
	    "find_ep: FILE Name could not be written to ESPS Common\n");
	    exit(1);
	}


    exit(0);
    return(0);
}


int
endpoints(f1h, f1p, start, s_pt, e_pt, sf, scalad,  
    sil_req, low_thresh, hflag, high_thresh, fflag, 
    final_thresh, context, time)
/*This function finds the beginning and end point of an utterance
  in a sampled data file. It does this by comparing the average adjusted
  magnitude in each context size frame with three thresholds.
*/

#define AAM_MIN      1.2
#define HUGE_AAM    10.
#define LOW_FACTOR   2.5
#define HIGH_FACTOR  4.0
#define FINAL_FACTOR 0.8

struct header *f1h;
FILE *f1p;
long  start, *s_pt, *e_pt;
int  hflag, fflag;
float sil_req, low_thresh, high_thresh, final_thresh, 
	context, time, sf, scalad;
{
/* - quick definition of input parameters

    struct header *f1h ;	 input file header pointer
    FILE	*f1p ;		 input file sream pointer
    int		hflag ;		 high threshold flag 
    int		fflag ;		 final threshold flag

    int		scalad ;	2**(adbits - 12)
    long	start ;		 starting point in file 
    float	sil_req ;	 silence required between words
    float	low_thresh ;	 variable for first threshold 
    float	high_thresh ;	 variable for second threshold 
    float	final_thresh ;	 variable for end threshold     
    float	context ;	 internal frame size in milliseconds 
    float	time ;		 time period for computing statistics 
    long	*s_pt; 		 starting record position 
    long	*e_pt;		 ending record position 
    float	sf ;		 sampling frequency of input data
*/

/* - Returned Error Codes
    0	Successful Completion
    1	Possible Word at End of File
    2	Incorrect Ordering of Thresholds
    3	Not Enough Room to Compute Statistics
    4	Nonsilence Interval Analyzed
    5	No Word Found - Low_Thresh Not Exceeded
    6	No Word Found - High_Thresh Not Exceeded
the messages are printed by find_ep.c
*/

    int lowcnt = 1;	    /*counts number of frames after end of word*/
    int exlow = NO;	    /* flag for low threshold*/
    int ipf;		    /* internal points/frame*/
    int nsil;		    /* number of silence frames*/
    int nstatf;		    /* number of frames used to compute statistics*/
    long start_pt;	    /* points to current starting point*/
    int calaam_err = 0;	    /*contains calaam error status*/
    
    float aam = 0;	    /* average adjusted magnitude */
    float final_tmp;	    /* holds temporary final threshold */
    float high_tmp;	    /* holds temporary high threshold */

    int calaam();	    /* compute average adjusted magnitude */

    /*
     * set up parameters
    */
    
    ipf = ROUND(sf*context/1000.);
    nsil = ROUND(sil_req/context);
    if (nsil < 1) nsil = 1;
    nstatf  = ROUND(time/context);
    start_pt = start;

    if(debug_level > 0){
	Fprintf(stderr, "endpoints: Just entered ENDPOINTS\n");
	Fprintf(stderr, "endpoints: Scalad = %f\n", scalad);
	Fprintf(stderr, "endpoints: Start = %ld\n", start);
	Fprintf(stderr, "endpoints: Sil_Req = %f\n", sil_req);
	Fprintf(stderr, "endpoints: low_thresh = %f\n", low_thresh);
	Fprintf(stderr, "endpoints: high_thresh = %f\n", high_thresh);
	Fprintf(stderr, "endpoints: final_thresh = %f\n", final_thresh);
	Fprintf(stderr, "endpoints: context = %f\n", context);
	Fprintf(stderr, "endpoints: time = %f\n", time);
    }

    /*
     * if needed, compute statistics 
    */
    
    if(low_thresh <= 0.){

	if(debug_level >0)
	    Fprintf(stderr, "endpoints: Computing Statistics\n");

	if( (calaam_err = calaam( nstatf, ipf, &aam, f1h, f1p)) != 0)
	    return(3);
	if(aam <= AAM_MIN*scalad)
	    aam = AAM_MIN*scalad;
	if(aam > HUGE_AAM*scalad)
	    return(4);
	start_pt += nstatf*ipf;
	low_thresh = LOW_FACTOR*aam;

	if(debug_level > 0)
	    Fprintf(stderr, "endpoints: Low_Thresh = %f\n", low_thresh);
    }

    /*
     * set high and final threshold
    */
    
    high_tmp = HIGH_FACTOR*low_thresh;
    final_tmp = FINAL_FACTOR*low_thresh;
    if(!hflag) 
	high_thresh = high_tmp;
    if(!fflag)
	final_thresh = final_tmp;
    /* check threshold ordering*/
    if(final_thresh >= low_thresh || low_thresh >= high_thresh)
	return(2);

    if(debug_level > 0){
	Fprintf(stderr, "endpoints: High_Thresh = %f\n", high_thresh);
	Fprintf(stderr, "endpoints: Final_Thresh = %f\n", final_thresh);
    }

LOW_SEARCH: /* label for GOTO */

    /*
     * Search for frame > low_thresh
    */

    aam = 0;
    
    if(debug_level > 0)
	Fprintf(stderr, "endpoints: Looking for frame > Low_Thresh\n");

    while(aam < low_thresh){
	if( (calaam_err = calaam( 1, ipf, &aam, f1h, f1p)) != 0){
	    if(exlow)
		return(6);
	    else
		return(5);
	}
	start_pt += ipf;	
#ifdef DEBUG
	if(debug_level > 2){
	    Fprintf(stderr, "endpoints: AAM = %f; Start_Pt = %ld\n", aam,
	    start_pt - ipf);
	}
#endif
    }

    /*
     * store potential starting point of word
    */

    *s_pt = start_pt - ipf;

    if(debug_level >0)
	Fprintf(stderr, "endpoints: Potential starting point = %ld\n",
	*s_pt);

    /*
     * Now we must cross high_thresh before crossing final_thresh
    */
    
    exlow = YES;

    if(debug_level > 0)
	Fprintf(stderr, "endpoints: Looking for frame > High_Thresh\n");

    while(aam < high_thresh && aam > final_thresh){
	if( (calaam_err = calaam( 1, ipf, &aam, f1h, f1p)) != 0)
	    return(6);
	start_pt += ipf;
#ifdef DEBUG
	if(debug_level > 2){
	    Fprintf(stderr, "endpoints: AAM = %f; Start_Pt = %ld\n", 
	    aam, start_pt - ipf);
	}
#endif
    }


    /* If aam < final_thresh, throw away potential starting point  and
	look for new starting point (frame > low_thresh)*/

    if(aam < final_thresh){
	
	if(debug_level > 0)
	    Fprintf(stderr, 
"endpoints: AAM < Final_Thresh; throw away potential starting point and look for frame > Low_Thresh\n");
	goto LOW_SEARCH;	
    }


FINAL_SEARCH: /*label for GOTO */

    /*
     * Look for crossing of final_thresh
    */

    if(debug_level > 0)
	Fprintf(stderr, "endpoints: Looking for frame < Final_Thresh\n");

    while(aam > final_thresh){
	if( (calaam_err = calaam( 1, ipf, &aam, f1h, f1p)) != 0)
	    break;
	start_pt += ipf;
#ifdef DEBUG
	if(debug_level > 2){
	    Fprintf(stderr, "endpoints: AAM = %f, Start_Pt = %ld\n", 
	    aam, start_pt - ipf);
	}
#endif
    }

    if(aam > final_thresh){
	/*EOF reached; We have partial word*/
	*e_pt = start_pt - ipf + calaam_err;
	return(1);
    }

    /*
     * Mark end of utterance
    */
    
    *e_pt = start_pt -1 -ipf;

    if(debug_level > 0)
	Fprintf(stderr, "endpoints: Potential endpoint of word = %ld\n", 
	    *e_pt);

    /*
     * We must stay below final_thresh for NSIl frames
    */

    lowcnt = 1;

    if(debug_level > 0)
	Fprintf(stderr, "endpoints: AAM must stay < Low_Thresh\n");

    while(lowcnt < nsil && aam < low_thresh){
	if( (calaam_err = calaam( 1, ipf, &aam, f1h, f1p)) != 0)
	    break;
	start_pt += ipf;
	lowcnt++;
#ifdef DEBUG
        if(debug_level > 2){
	    Fprintf(stderr, 
	    "endpoints: Frame # %d,  AAM = %f, start_pt  = %ld\n", 
	    lowcnt -1, aam, start_pt - ipf);
	}
#endif
    }

    if(aam > low_thresh){ /*go back and search for crossing of final_thresh*/

	if(debug_level >0)
	    Fprintf(stderr, 
"endpoints: AAM exceeded Low_Thresh; go back and look for crossing of Final_Thresh\n");

	goto FINAL_SEARCH;
    }

    /*
     * Utterance found; Is it long enough
    */
    
    if( ( (*e_pt - *s_pt + 1)/sf ) < .150 ){

	if(debug_level >0)
	    Fprintf(stderr, "endpoints: Word not long enough; start again\n");

	exlow = NO;
	goto LOW_SEARCH;
    }

    /*
     * Got here; Passed all tests
    */

    return(0);
}


int
calaam( nf, npf, aam, fh, fp)

/* Calculate the average adjusted magnitude (AAM)*/
/* AAM is the magnitude of sample value minus average value*/

FILE *fp;		/*file stream pointer*/
struct header *fh;	/* file header pointer*/
int nf;			/*number of frames to average over*/
int npf;		/*number of points per frame*/
float *aam;		/* returned average adjusted magnitude*/

{
    static double *buff = NULL;   /*data buffer*/
    int i;	    /*counter*/
    int total_pts;  /*number of point to get*/
    float sum=0;    /*sum of points*/
    float avg=0;    /*average of points*/
    static int buffsize = 0;

    total_pts = nf * npf;

    if(total_pts > buffsize){
        if(buff != NULL) free((char *)buff);
        buff = (double *)calloc( (unsigned)total_pts, sizeof(double));
        spsassert(buff != NULL, "Couldn't allocate space for buff");
    }
    buffsize = total_pts;

#ifdef DEBUG
    if(debug_level >1){
	Fprintf(stderr, "calaam: Just entired CALAAM\n");
	Fprintf(stderr, "calaam: Number of frames = %d\n", nf);
	Fprintf(stderr, "calaam: Number of points/frame = %d\n", npf);
	Fprintf(stderr, "calaam: Total number of points = %d\n", total_pts);
    }
#endif
    if( (get_sd_recd(buff, total_pts, fh, fp)) != total_pts)
	return(1);
#ifdef DEBUG
    if(debug_level > 9)
	for(i=0;i<total_pts;i++)
	    Fprintf(stderr, "calaam: Input point %d = %lf\n", 
	    i, buff[i]);
#endif
    /*
     * Compute Average
    */

    for(i=0;i< total_pts; i++)
	sum += buff[i];
#ifdef DEBUG
    if(total_pts <= 0){
	Fprintf(stderr, 
	"find_ep: endpoints: calaam: total_pts <= 0\n");
	exit(1);
    }
#endif    
    avg = sum/(float)total_pts;
#ifdef DEBUG
    if(debug_level > 2)
	Fprintf(stderr, "calaam: Average = %f\n", avg);
#endif
    /*
     * Compute AAM
    */
    *aam = 0;
    for(i=0; i < total_pts; i++){
	*aam +=  (float)fabs( (double)(buff[i] - avg) );
#ifdef DEBUG
	if(debug_level > 7)
	Fprintf(stderr, "calaam: magnitude[%d] = %f\n", i, 
	(float)fabs( (double)(buff[i] - avg)));
#endif
    }
#ifdef DEBUG
    if(debug_level > 2)
	Fprintf(stderr, "calaam: sum of AAMs = %f\n", *aam);
#endif
    *aam = *aam/(float)total_pts;

    /*
     * ALL DONE; clean up and return
    */
    
    return(0);

}


void
error_exit(error_status, eflag)
int error_status, eflag;
{
	switch (error_status) {
	    case 1: 
		Fprintf(stderr,"find_ep: Possible word at end of file\n");
		if(eflag){
		    break;
		}
		else{
		    exit(1);
		}
		break;
	    case 2: 
	        Fprintf(stderr, 
		"find_ep: Incorrect ordering of thresholds\n");
		exit(1);
	    case 3: 
		Fprintf(stderr, 
		"find_ep: Not enough room to compute statistics\n");
		exit(1);
	    case 4: 
		Fprintf(stderr, "find_ep: Nonsilence interval analyzed\n");
		exit(1);
	    case 5:
		Fprintf(stderr, 
		"find_ep: No word found - low_thresh not exceeded\n");
		exit(1);
	    case 6:
		Fprintf(stderr, 
		"find_ep: No word found - high_thresh not exceeded\n");
		exit(1);
	    default:
		Fprintf(stderr, 
		"find_ep: Invalid error code returned from endpoints()\n");
		exit(1);
	}
    
}
