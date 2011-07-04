/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Burton, Johnson, Shore
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)auto.c	1.11	8/31/95	ERL";

char *Version = "1.11";
char *Date = "8/31/95";

/*
 * System Includes
*/
#include <stdio.h>
#include <math.h>

/*
 * ESPS INCLUDES
*/
#include <esps/esps.h>
#include <esps/sd.h>
#include <esps/anafea.h>
#include <esps/fea.h>
#include <esps/unix.h>
#include <esps/window.h>


/*
 * Defines
 */
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}

#define SYNTAX \
USAGE("auto [-l frame_len] [-o order] [-{pr} range] [-w window_type]\n [-x debug_level] [-P param] [-S step_size] [-B] sd.file fea_ana.file")

#define BUFSIZE 8192

#define STRCOV_CONV 1e-5
#define STRCOV_ITER 20

/*
 * global variables
 */
char	*ProgName = "auto";
int	debug_level = 0;		/* debug level */
void	get_range();

/*
 * ESPS Functions
 */
    int get_auto();
    char *get_cmd_line();
    void lrange_switch();
    void put_sd_recf();

    extern int
            optind;
    extern char
            *optarg;

/*
 * main program
 */
main(argc, argv)
    int     argc;
    char    **argv;
{
    char    *param_file = NULL;  /*parameter file name*/
    FILE    *ifile = stdin,	/* input and output file streams */
            *ofile = stdout,
	    *tmp = NULL;	/*tmp file stream*/
    struct header  		/* input and output file headers */
            *ih, *oh;
    char    *iname, 		/* input and output file names */
	    *oname;
	    
    struct anafea
	    *fea_ana_rec;	/* record for fea_ana data */

    short   window_type = WT_RECT;  /* type of data window*/
    char    *w_type = "RECT";	/*window type description*/
    char    *window_func;	/*variable to use in lin_search*/
    int     order = 10;		/* order of auto */
    char    *prange = NULL;	/* string for range specification (-p) */
    int     o_flag = 0;		/* flag for order option */
    int	    p_flag = 0;		/* flag for range option */
    int     w_flag = 0;		/* flag for data window option*/
    int     B_flag = 0;		/* flag for strcov option */
    long    first, i, j;
    long    frame_len = -1;	/* number of sampled data points per frame*/
    long    step_size = 0;	/* number of points between frames*/
    int	    actsize;		/* actual number points read */
    long    nframes = 1;	/*number fixed length frames to process*/
    float   tot_power;		/* total power */
    float   *x, *xwindow;	/* array for data and windowed data*/
    double  *y;			/* array for autocorrelation of data*/
    long    position;		/* current position in FEA_SD file */
    int     c;			/*option argument*/
    long    num_of_points = 0;	/*total points in input file*/
    int     npts = 0;		/* # pts returned by get_sd_recf*/
    long    nan;		/*total points analyzed*/
    static float
	    tmpdata[BUFSIZE];	/*temporary data storage*/
    int     first_time = YES;	/* flag for get_sd_orec*/
    char    *power = "unwindowed";  /*power computation option*/

    /*
     * process command line options
     */
    while ((c = getopt(argc, argv, "w:P:o:p:r:x:l:S:B")) != EOF)
        switch (c) {
	case 'P':
	    param_file = optarg;
	    break;
        case 'o':
            order = atoi(optarg);
	    o_flag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
        case 'p':
	case 'r':
            prange = optarg;
	    p_flag++;
            break;
	case 'l':
	    frame_len = atoi(optarg);
	    break;
	case 'S':
	    step_size = atoi(optarg);
	    break;
	case 'w':
	    w_flag++;
	    w_type = optarg;
	    break;
	case 'B':
	    B_flag++;
	    break;
        default: 
            SYNTAX;
            break;
        }
    /*
     * process file arguments
     */
    if (optind < argc)
	iname = argv[optind++];
    else {
	Fprintf(stderr, "auto: no input sampled_data file specified.\n");
	SYNTAX;
    }
    if (optind < argc)
	oname = argv[optind++];
    else {
	Fprintf(stderr, "%s: no output FEA_ANA file specified.\n", ProgName);
	SYNTAX;
    }
    if (optind < argc)
    {
	Fprintf(stderr, "%s: too many file names.\n", ProgName);
	SYNTAX;
    }
    iname = eopen(ProgName, iname, "r", FT_FEA, FEA_SD, &ih, &ifile);
    oname = eopen(ProgName, oname, "w", NONE, NONE, &oh, &ofile);
    if (debug_level)
    {
	Fprintf(stderr, "Input file is %s\n", iname);
	Fprintf(stderr, "Output file is %s\n", oname);
    }

    /*
     * is file single-channel and real?
     */
    if (get_fea_siz("samples", ih, (short *) NULL, (long **) NULL) != 1)
	ERROR_EXIT("Multichannel data not supported yet");

    if (is_field_complex(ih, "samples"))
	ERROR_EXIT("Complex data not supported yet");
    
    /*
     * determine frame_length, power option, and range
     */
    (void) read_params(param_file, SC_CHECK_FILE, iname);

    if (!o_flag)
	if (symtype("order") != ST_UNDEF)
	    order = getsym_i("order");

    if(symtype("power") != ST_UNDEF)
	power = getsym_s("power");

    if (!w_flag)
	if (symtype("window") != ST_UNDEF)
	    w_type = getsym_s("window");

    if (!B_flag) 
      if (symtype("strcov_auto") != ST_UNDEF)
	if (!strcmp(getsym_s("strcov_auto"), "yes"))
	  B_flag = 1;

    /*
     * Add prefix to window type and find numeric value
     */
    window_func = malloc((unsigned)(strlen(w_type) + 4));
    (void)strcpy(window_func, "WT_");
    (void)strcat(window_func, w_type);
    if((window_type = lin_search(window_types, window_func)) == -1){
	ERROR_EXIT("ERROR: Invalid window function specified");
    }

   /*
    * Get number of points in the file
    */
    num_of_points = 0;
    if(ih->common.ndrec == -1){/*input is pipe - count the points*/
	tmp = tmpfile();
	while((npts = get_sd_recf(tmpdata, (int)BUFSIZE, ih, ifile)) != 0) {
	    put_sd_recf(tmpdata, npts, ih, tmp);
	    num_of_points += npts;
	}
	rewind(tmp);
	ifile = tmp;	/*reassign input file stream to tmp*/
    }
    else
	num_of_points = ih->common.ndrec;


    get_range
	(&first, &frame_len, &nframes, prange, 
	p_flag, num_of_points, &nan, &step_size);

    symerr_exit();  /*exit if any of the parameters were missing*/

    if(debug_level > 1){
	Fprintf(stderr, 
          "\nauto: first point = %ld, frame length = %ld, step size = %ld\n",
		    first, frame_len, step_size);
	Fprintf(stderr, 
   "\nauto: # of frames = %ld, total points analyzed = %ld\n", nframes, nan);
	Fprintf(stderr, "\nauto: number of points in input file = %ld\n",
		num_of_points);
    }

/*
 * Check for inconsistencies
 */
    if (first < 1)
	ERROR_EXIT("can't have negative starting point");
    if (frame_len == 0)
	ERROR_EXIT("range must specify more than one point");
    if (debug_level) Fprintf(stderr, 
   "\nauto: frame length %ld, order %ld, NO. frames %ld, step size = %ld\n", 
	    frame_len, order, nframes, step_size);

    if (order > frame_len) 
	ERROR_EXIT("order greater than frame length");

/*
 * create, fill in, and write FEA_ANA file header
 */
    oh = new_header(FT_FEA);
    if(init_anafea_hd(oh, (long)0, (long)order, (long)1, (long)1, (long)1, (short)0, (short)0) != 0)
	ERROR_EXIT("Error filling FEA_ANA header");
    add_source_file(oh, iname, ih);
    oh->common.tag = YES;
    (void) strcpy(oh->common.prog, ProgName);
    (void) strcpy(oh->common.vers, Version);
    (void) strcpy(oh->common.progdate, Date);
    oh->variable.refer = iname;
    add_comment(oh, get_cmd_line(argc, argv));
    *(long *) get_genhd("start", oh) = first;
    *(long *) get_genhd("nan", oh) = nan;
    *(long *) get_genhd("frmlen", oh) = frame_len;
    *(float *)get_genhd("src_sf", oh) = get_genhd_val("record_freq", ih, 0.0);
    *(short *)get_genhd("spec_rep", oh) = AUTO;

    /* add generic header item for data window, power option and step_size*/
    /* this illustrates both "styles" of using add_genhd*/

    *add_genhd_l("step_size", (long *) NULL, 1, oh) = step_size;

    (void) add_genhd_e("data_window", &window_type, 1, window_types, oh);
    (void) add_genhd_c("power", power, 0, oh);

    if (B_flag) 
      (void) add_genhd_c("strcov_auto", "yes", 0, oh);
    else 
      (void) add_genhd_c("strcov_auto", "no", 0, oh);
      
    (void)update_waves_gen(ih, oh, (float)(first + (frame_len+1)/2), 
		(float)step_size);

    write_header(oh, ofile);

/*
 * Allocate memory for data and autocorrelations
 */

    x = (float *) calloc((unsigned) frame_len, sizeof(float));
    spsassert(x != NULL, "Couldn't allocate enough memory for input data");
    xwindow = (float *) calloc((unsigned) frame_len, sizeof(float));
    spsassert(xwindow != NULL, 
		    "Couldn't allocate enough memory for windowed data");
    y = (double *) calloc((unsigned) order+1, sizeof(double));
    spsassert(y != NULL, "Couldn't allocate space for autocorrelations");

 /*
  * allocate anafea record and move to starting position
  */
    fea_ana_rec = allo_anafea_rec(oh); 
    if (first > 1) fea_skiprec(ifile, first - 1, ih);

    position = first;
    /*
     * main loop
     */

    for (i = 0; i < nframes; i++) {

	/*
	 * get sampled data and window it
	 */

    	actsize = get_sd_orecf(x, (int)frame_len, (int)step_size, 
				first_time, ih, ifile);
	first_time = NO;

	if (actsize != frame_len) break;

	/* window data */
	if((window(frame_len, x, xwindow, window_type, (double *)NULL)) != 0)
	    ERROR_EXIT("Invalid window function specified");

	if (debug_level) 
	    Fprintf(stderr, "auto: got %d points to analyze\n", actsize);

	if (debug_level > 2) {
	    Fprintf(stderr, 
	    "auto: sampled data input to get_auto for frame %d:\n", i+1);
	    for (j = 0; j < frame_len; j++) 
		Fprintf(stderr, "%f\n", xwindow[j]);
	}

	/*
	 * compute autocorrelations and power
	 */
	    /* first compute autocorrelations of windowed data */

	if (B_flag) 
	  strcov_auto(xwindow, actsize, y, order, order +1, (int) 0, 
		      'f', STRCOV_CONV, STRCOV_ITER); 
	else
	  (void)get_auto(xwindow, actsize, y, order);

	for(j=0; j<order; j++)
	  fea_ana_rec->spec_param[j] = (float)y[j+1];

	/* compute power of input or windowed data */

	if(strcmp(power, "unwindowed") == 0)
	  (void)get_auto(x, actsize, y, 0);
	else if(strcmp(power, "windowed") != 0)
	  ERROR_EXIT("Invalid power computation window option");

	tot_power = (float)y[0]/actsize;

	/*
	 * fill in and write out fea_ana record
	 */
	*fea_ana_rec->frame_type = NONE;
	*fea_ana_rec->raw_power = tot_power;
	*fea_ana_rec->frame_len = (long)actsize;
        *fea_ana_rec->tag = (long)position;
        position += step_size;

	put_anafea_rec(fea_ana_rec, oh, ofile);

    }
/*
 * put info in ESPS common
 */
    if (strcmp(oname, "<stdout>") != 0) {
	(void) putsym_s("filename", iname);
	(void) putsym_s("prog", ProgName);
	(void) putsym_i("start", (int) first);
	(void) putsym_i("nan", (int) nan);
	(void) putsym_i("step_size", (int)step_size);
	(void) putsym_i("frmlen", (int)frame_len);
    }    
    exit(0);
    return (0);
}

void
get_range(srec, fsize, nfrm, rng, rflag, total_pts, nan, step)
long *srec;			/* starting record */
long *fsize;			/* frame size */
long *nfrm;			/* number of frames */
char *rng;			/* range string from range option */
int rflag;			/* flag for whether range option used */
long total_pts;			/* length of input file*/
long *nan;			/* total number of points to process*/
long *step;			/* number of points between frames*/
{
    long last, lnan;
    *srec = 1;
    last = total_pts;
    if (rflag) {
	lrange_switch (rng, srec, &last, 1);	
	if(last == 0)
	    last = total_pts;
    }
    else {
	if(symtype("start") != ST_UNDEF) 
	    *srec = getsym_i("start");
	if(symtype("nan") != ST_UNDEF) {
	    lnan = getsym_i("nan");
	    if (lnan == 0)
		last = total_pts;
	    else
		last = *srec + lnan - 1;
	}	
    }

    *nan = last - *srec + 1;
    if (debug_level) 
	Fprintf(stderr, "auto: range is %ld to %ld\n", *srec, last);

/*
 * Get Frame size info
*/
    if (*fsize == -1) /*-l option not used -- get from param file*/
	if(symtype("frame_len") != ST_UNDEF) 
	    *fsize = getsym_i("frame_len");

/*
 * if no frame_len set or if set to 0, use whole range as frame_len
*/
    if (*fsize == 0 || *fsize == -1) { 
	*nfrm = 1;
	*fsize = *nan;	
	*step = *fsize;
    }
    else { /*frame_len was set explicitly*/ 

	/*
	 * Get step size info
	*/
	    if(*step == 0){
	        if(symtype("step_size") != ST_UNDEF)
		    *step = getsym_i("step_size");
	    }
    
	    if(*step == 0)
		*step = *fsize;

	    if (*fsize > *nan) {
		Fprintf(stderr, 
    "auto: Frame size is greater than total number of points - exiting\n");
	    exit(1);
	    }
	    else {
		*nfrm =  ((*nan - *fsize) / *step) + 1;
		*nan = (*nfrm - 1)* *step + *fsize;
	    }
    }

}

