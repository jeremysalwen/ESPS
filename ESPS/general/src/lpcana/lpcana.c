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
 * Written by:  Shankar Narayan
 * Checked by:
 * Revised by:  John Shore (added per-pulse output, + general method)
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)lpcana.c	3.23	1/23/97	ESI/ERL";

/*
 * System Includes
 */

# include <stdio.h>
#include <math.h>

/*
 * ESPS includes
 */

#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/anafea.h>
#include <esps/unix.h>
#include <esps/window.h>
#include <esps/ana_methods.h>

/*
 * DEFINES
 */

#define SYNTAX USAGE("lpcana [-P param_file] [-r range] [-p range] [-m method] [-F] \n [-x debug_level] input_file output_file");

#define ERROR_EXIT(text) {(void) fprintf(stderr, "lpcana: %s - exiting\n", text); exit(1);}


#define TWENTY 20
#define UNVOICED_FRM 0
#define VOICED_FRM 1
#define BUFSIZE 4096
/* Thresholds for voicing decision */

#define P_OFFSET 3
#define PEAK_THRESHOLD 7.5
#define ZCROSS_THRESHOLD 0.20
#define LPC_GAIN_THRESHOLD 0.5
#define K1_THRESHOLD -0.25

/* system functions and variables */
/* done via <esps/unix.h>*/
FILE *tmpfile();

/* external ESPS functions */

long   *add_genhd_l ();
short  *add_genhd_s ();
float  *add_genhd_f ();
char   *add_genhd_c ();
char   *get_cmd_line ();
void write_header ();
void put_sd_recf();
struct header  *read_header ();
struct anafea  *allo_anafea_rec ();
int     get_anafea_rec ();
void put_anafea_rec ();
void lrange_switch();

long    debug_level = 0;
short   pulse_count = 0;

float   *res_data, *raw_data, *tmpbuf;	    /* hold data to be analyzed*/
float   fpse[TWENTY], fpeak = 0, bpeak, peakval;
float   tcons;

int     anal_method = AM_MBURG;
int     Fflag = 0;  /* flag for -F (don't redo spectral analysis per pulse */

short   frame_size = 160, frame_beg, buf_size, min_pp = 20;
short   plocn, zcross_count = 0,  dynamic_frame_size;
short   fplocn[TWENTY], fpsize[TWENTY], fptype[TWENTY];

long    start, Nan = 0, start_p, end_p = 0;/* beginning and end point*/

short   matsiz = 11, lpc_order = 10;

long    frame_num = 1, frame_count;

int	sincn = 0;


/* Definitions related to output fea_ana file */

FILE * fptr_fea_ana_file = NULL;	/*stream ptr for output file*/
char   *fea_ana_file = NULL;		/*holds output file name*/
struct header  *fea_oh;			/*points to output header*/
struct anafea  *anafea_rec;		/*output data structure*/

extern  optind;
extern char *optarg;


main (argc, argv)
    int     argc;
    char    *argv[];
{

/* Definitions related to input SD File */

    FILE    *fptr_sd_file = stdin;	/* input file stream */
    char    *input_sd_file;		/* input file name */
    struct header   *sd_ih;		/* input file header ptr */

    FILE    *tmp;			/* ptr for temporary file stream */
    struct header   *tmphdr;		/* header for temporary file */
    static float    tmpdata[BUFSIZE];	/* for copying data to tmp file */
    int     npts = 0;			/* # pts returned by get_sd_recf */
    int     common_read = NO;		/* flag for reading ESPS Common */
    float   t;
    int     c, dataptr, i, j;		/* misc. variables */
    char    *param_file = NULL;
    int     num_of_files = 0;		/* number of command line filenames */
    char   *cmd_line;			/* string for command line */
    char   *p_switch = NULL;
    char   *Version = "3.23";
    char   *Date = "1/23/97";
    short   range_err = 0;
    long start_orig =0;			/* starting point */
    long real_Nan = 0;			/* store real nan info */
    int   mflag = 0;			/* flag for -m */
    int   sflag = 0;			/* flag for -s option*/
    long num_channels;
    char *analysis;			/* analysis method type */

/* check the command line options. */

    cmd_line = get_cmd_line (argc, argv);/* store copy of command line */

/* this includes an undocumented sync option -- see refcof (1-ESPS) */

    while ((c = getopt (argc, argv, "P:x:m:r:p:s:F")) != EOF)
    {
	switch (c)
	{
	    case 'r':
	    case 'p': 
		p_switch = optarg;
		break;
	    case 'P': 
		param_file = optarg;
		break;
	    case 'm':
	        analysis = optarg;
		mflag++;
		break;
	    case 'x': 
		debug_level = atoi(optarg);
		break;
	    case 's':
	        sincn = atoi(optarg);
	        sflag++;
	        break;
	    case 'F':
		Fflag++;
		break;
            default: 
		SYNTAX;
		break;
	}
    }

/* Get the filenames. */

/* 
 * Get the number of filenames on the command line
 */ 
    if((num_of_files = argc - optind) > 2){
	Fprintf(stderr, "Only two file names allowed\n");
	SYNTAX;
    }
    if(debug_level > 0)
	Fprintf(stderr, "lpcana: num_of_files = %d\n", num_of_files);

    if(num_of_files == 0){
	Fprintf(stderr, "No output filename specified\n");
	SYNTAX;
    }

    if(num_of_files == 1) {
    /*
     * Only output file specified on command line
     */
    	fea_ana_file = eopen("lpcana", argv[optind], "w", NONE, NONE,
			     (struct header **)NULL, &fptr_fea_ana_file);
	if (debug_level) 
	{
	    Fprintf (stderr,"Output file is %s\n", fea_ana_file);
	}
    /* check common for input filename*/
	if((read_params(param_file, SC_CHECK_FILE, (char *)NULL) ) != 0){
	    Fprintf(stderr, 
           "No input file specified on command line and no Common exists\n");
	    exit(1);
	}
	if(symtype("filename") == ST_UNDEF){
	    Fprintf(stderr, 
	    "No input file specified on command line or in Common\n");
	    exit(1);
	}
	else{
	    if(debug_level > 0)
		Fprintf(stderr, "Input file name from Common is %s\n",
		getsym_s("filename"));
	    input_sd_file = eopen("lpcana", getsym_s("filename"), "r",
				  FT_FEA, FEA_SD, &sd_ih, &fptr_sd_file);
	}

	common_read = YES;
    }

    if (num_of_files == 2)
    /*
     * Both input and output file names specified on command line
     */
    {
	input_sd_file = eopen("lpcana", argv[optind++], "r", FT_FEA, FEA_SD,
			      &sd_ih, &fptr_sd_file);
	if (debug_level) 
	    Fprintf (stderr,"Input file is %s\n", input_sd_file);

    	fea_ana_file = eopen("lpcana", argv[optind], "w", NONE, NONE,
			     (struct header **)NULL, &fptr_fea_ana_file);
	if (debug_level) 
	{
	    Fprintf (stderr,"Output file is %s\n", fea_ana_file);
	}
    }

/*
 * Check for unsupported input types
 */
    if((num_channels = 
        get_fea_siz("samples", sd_ih,(short *) NULL, (long **) NULL)) != 1)
    {
	Fprintf(stderr, 
		"lpcana: Multichannel data not supported yet - exiting.\n");
	exit(1);
    }

    if(is_field_complex(sd_ih, "samples") == YES)
    {
	Fprintf(stderr, "lpcana: Complex data not supported - exiting.\n");
	exit(1);
    }    



/*parse range -- whole file unless -p option overrides or common file
		contains start and nan */
    start = 1;
    Nan = 0;

    if(sd_ih->common.ndrec == -1)   /* input is a pipe or record
				     * length is variable */
    {
    /*
     * Copy input into a temporary file 
     */
    	tmp = tmpfile();

	/* Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(sd_ih);
	write_header(tmphdr, tmp);

    	while ((npts = get_sd_recf(tmpdata, BUFSIZE, sd_ih, fptr_sd_file)) 
	       != 0)
	{
	    put_sd_recf(tmpdata, npts, tmphdr, tmp);
	    Nan += npts;
	}

	fclose(fptr_sd_file);
        rewind(tmp);
	sd_ih = read_header(tmp);
	fptr_sd_file = tmp;
    }
    else
	Nan = sd_ih->common.ndrec;
    
    real_Nan = Nan;

/*
 * read params and Common, if approppriate
 */
    if (common_read == NO && strcmp("<stdin>",input_sd_file) != 0){ 
	/* not pipe */
	/* then read params and read common if file matches */
	(void) read_params(param_file, SC_CHECK_FILE, input_sd_file); 

    }
    else /* common_read == no && strcmp("<stdin>",input_sd_file) == 0 */ {
	/*it is a pipe - read params, but don't read common*/
	(void)read_params(param_file, SC_NOCOMMON, (char *)NULL);
    }

/*
 * Check analysis method
 */

    if (!mflag) {
	if (symtype("method") != ST_UNDEF)
	    analysis = getsym_s("method");
	else
	    analysis = "MBURG";
    }

    if ((anal_method = lin_search(ana_methods, analysis)) == -1) {
  
	(void)fprintf(stderr, "lpcana: analysis method is %s\n", analysis);
	ERROR_EXIT("Invalid spectrum analysis method");
    }
  
    if (debug_level) Fprintf(stderr,
			     "lpcana: analysis method is %s\n",
			     ana_methods[anal_method]);

    if ((anal_method != AM_AUTOC) 
	&& (anal_method != AM_STRCOV) 
	&& (anal_method != AM_STRCOV1)
	&& sflag)
    {
	(void) fprintf(stderr, 
		       "lpcana: method not AUTOC, STRCOV, or STRCOV1.\n");
	(void) fprintf(stderr, " -s option ignored\n");
    }

    if (!p_switch) {
	if(symtype("start") != ST_UNDEF)
	    start = getsym_i("start");
          
	if(symtype("nan") != ST_UNDEF)
	    Nan = getsym_i("nan");

	if(Nan == 0)/* do whole file */
	    Nan = real_Nan;	
      
	if ((start < 1) || (Nan < 0) || (Nan > real_Nan))
	  range_err++;

    }
    else {	/* get start and nan from the command line*/
	lrange_switch (p_switch, &start_p, &end_p, 1);

	if ((start_p < 1) || (end_p > (start_p + real_Nan - 1)))
	    range_err++;

	start = start_p;

	if (end_p == 0)
	    end_p = real_Nan;

	Nan = end_p - start_p + 1;
    }

    if (range_err > 0)
    {
	Fprintf (stderr,
		 "lpcana: specified range %ld:+%ld not in input file\n",
		 start, Nan);
	exit (1);
    }

/*
 * Get rest of parameters
*/    
    if(symtype("lpc_frame_size") != ST_UNDEF)
        frame_size = getsym_i ("lpc_frame_size");
    if(symtype("lpc_filter_order") != ST_UNDEF)
        lpc_order = getsym_i ("lpc_filter_order");

    if(lpc_order > TWENTY){
	Fprintf(stderr, "lpcana: Lpc_filter_order must be <= 20\n");
	exit(1);
    }

    if(symtype("minimum_pulse_length") != ST_UNDEF)
        min_pp = getsym_i ("minimum_pulse_length");

    symerr_exit (); /* exit if any parameters were missing*/

/*
 * Compute constants
*/
    dynamic_frame_size = frame_size;
    matsiz = lpc_order + 1;
    frame_beg = 3 * frame_size;
    buf_size = 4 * frame_size;

    frame_count = Nan / frame_size;
    Nan = frame_count * frame_size;

    if (debug_level > 0){
	Fprintf(stderr, "lpcana: Input file = %s\n", input_sd_file);
	Fprintf(stderr, "lpcana: Output file = %s\n", fea_ana_file);
	Fprintf(stderr, 
	"lpcana: Frame_size = %d, lpc_order = %d, min_pp= %d\n", 
	   frame_size, lpc_order, min_pp);
	Fprintf(stderr, "lpcana: frame_beg = %d, buf_size = %d\n",
	    frame_beg, buf_size);
	Fprintf(stderr, "lpcana: frame_count = %ld, nan = %ld\n",
	    frame_count, Nan);
    }

/* Allocate memory for arrays */
    res_data = (float *) malloc ((unsigned)(buf_size * sizeof *res_data));
    if (res_data == NULL)
    {
	Fprintf (stderr, "lpcana: couldn't allocate dynamic memory for array - res_data\n");
	exit (1);
    }

    raw_data = (float *) malloc ((unsigned)(buf_size * sizeof *raw_data));
    if (raw_data == NULL)
    {
	Fprintf (stderr, "lpcana: couldn't allocate dynamic memory for array - raw_data\n");
	exit (1);
    }

    for (i = 0; i < buf_size; i++) 
	raw_data[i] = res_data[i] = 0;

    tmpbuf = (float *) malloc ((unsigned)(frame_size * sizeof *tmpbuf));
    if (tmpbuf == NULL)
    {
	Fprintf (stderr, "lpcana: couldn't allocate dynamic memory for array - tmpbuf\n");
	exit (1);
    }

/* Create output header */
    fea_oh = new_header (FT_FEA);
    fea_oh -> common.tag = YES;
    fea_oh -> variable.refer = input_sd_file;
    if (init_anafea_hd (fea_oh, (long)lpc_order, (long)
			lpc_order, (long)1, (long)1, (long)1, 
			    (short)0, (short)0) != 0)
    {
	Fprintf (stderr, "error filling FEA_ANA header");
	exit (1);
    }
    add_source_file (fea_oh, input_sd_file, sd_ih);
    (void)strcpy (fea_oh -> common.prog, "lpcana");
    (void)strcpy (fea_oh -> common.vers, Version);
    (void)strcpy (fea_oh -> common.progdate, Date);
    (void)add_comment (fea_oh, cmd_line);

    *(float *) get_genhd ("src_sf", fea_oh) = get_genhd_val("record_freq", sd_ih, (double) -1.);

    *(long*) get_genhd ("start", fea_oh) = start;
    *(long*) get_genhd ("nan", fea_oh) = Nan;
    *add_genhd_s ("p_offset", (short *) NULL, 1, fea_oh) = P_OFFSET;
    (void)add_genhd_c ("dcrem", "yes", 0, fea_oh);
    (void)add_genhd_c ("psynch", "yes", 0, fea_oh);
    *add_genhd_s ("matsiz", (short *) NULL, 1, fea_oh) = matsiz; 
    *(short *) get_genhd ("spec_rep", fea_oh) = RC;
    *(long *) get_genhd("frmlen", fea_oh) = (long)0; 
    (void) add_genhd_s ("nominal_frame_size", &frame_size, 1, fea_oh);
    *add_genhd_e("method", NULL, 1, ana_methods, fea_oh) = anal_method;
    if (Fflag) 
      (void)add_genhd_c ("old_framing", "yes", 0, fea_oh);
    (void) update_waves_gen(sd_ih, fea_oh, (float)start, (float)0);
    if (sflag) 
	*add_genhd_s("sinc_n", NULL, 1, fea_oh) = sincn;

/* Write Output Header */
    write_header (fea_oh, fptr_fea_ana_file);

/* Allocate records for ESPS files */
    anafea_rec = allo_anafea_rec (fea_oh);

/* tcons = Exponential decay factor with time constant of 2.5 msecs */
    tcons = 400.0 / get_genhd_val("record_freq", sd_ih, (double) -1.);
    if(tcons < 0){
	Fprintf(stderr, "lpcana: record_freq < 0 in input file- exiting.\n");
	exit(1);
    }
    tcons = pow (0.25, tcons);

    if (start > 0)
	fea_skiprec (fptr_sd_file, (long) start - 1, sd_ih);

/*
 * MAIN LOOP
*/
    
    /* save for putsym in common */

    start_orig = start; 

    for (frame_num = 1; frame_num <= frame_count; frame_num++)
    {
	if (debug_level)
	    Fprintf (stderr, "Locn = %d\n", start);

/* Read Speech Data */
	if(get_sd_recf(&raw_data[frame_beg], 
	    frame_size, sd_ih, fptr_sd_file) == 0)
	{
	    Fprintf(stderr, "EOF hit while reading data\n");
	    exit(1);
	}

/* Remove DC in the speech */
	remove_dc (&raw_data[frame_beg], frame_size);

/* Compute Residual Signal */

	whiten_data (&raw_data[frame_beg - lpc_order], frame_size, lpc_order, matsiz, &res_data[frame_beg]);

/* Backward Scanning of residual data */
	j = frame_beg + frame_size - 1;
	bpeak = tmpbuf[frame_size - 1] = res_data[j--];
	for (dataptr = frame_size - 2; dataptr > -1; dataptr--)
	{
	    bpeak = tcons * bpeak;
	    t = res_data[j--];
	    if (t > bpeak)
		bpeak = tmpbuf[dataptr] = t;
	    else
		tmpbuf[dataptr] = 0;
	}

/* Forward Scanning of residual data */
	j = frame_beg - P_OFFSET;
	for (dataptr = 0; dataptr < frame_size; dataptr++)
	{
	    fpeak = tcons * fpeak;
	    if (tmpbuf[dataptr] > fpeak)
	    {
		fpeak = tmpbuf[dataptr];
		if (dataptr - plocn < min_pp || zcross_count < 2)
		{
		    if (peakval < fpeak)
		    {
			plocn = dataptr;
			peakval = fpeak;
			zcross_count = 0;
		    }
		}
		else
		{
		    if (plocn < 0)
		    {
			double  ratio = tcons;
			int     lnt = dataptr - plocn;

			ratio = pow (ratio, (double) lnt);
			if (fpeak * ratio < peakval)
			    build_frame (plocn);
		    }
		    else
			build_frame (plocn);

		    peakval = fpeak;
		    plocn = dataptr;
		    zcross_count = 0;
		}
	    }
	    if (raw_data[j] * raw_data[j - 1] < 0)
		zcross_count++;
	    j++;
	}

/* Shift Data in */
	j = frame_size;
	for (i = 0; i < 3 * frame_size; i++)
	{
	    raw_data[i] = raw_data[j];
	    res_data[i] = res_data[j++];
	}

	for (i = 0; i <= pulse_count; i++)
	    fplocn[i] -= frame_size;

	plocn -= frame_size;
	start += frame_size;
    }

/*
 * Write Common info, if appropriate
 */
    if(strcmp(fea_ana_file, "<stdout>") !=0){
	if(putsym_s("filename", input_sd_file) != 0)
	    Fprintf(stderr, "Could not write into ESPS Common");
	(void)putsym_s("prog", argv[0]);
	(void)putsym_i("start", (int)start_orig);
	(void)putsym_i("nan", (int)Nan);
    }
    (void)fclose (fptr_sd_file);
    exit(0);
    return(0);
}

build_frame (next_pulse_locn)
short   next_pulse_locn;
{
    static short    flnt = 0;
    long    i, j, k, m, pulse_length;
    float   lpc_filter[TWENTY], rc[TWENTY], rcf[TWENTY], gain, fgain;
    static float *frame_data = NULL;
    short   vcount = 0;
    short   noz;
    float   se, t, peak, threshold;
    static float    prev_t = 0;
    double  r[TWENTY], rf[TWENTY];

    if (frame_data == NULL) 
	frame_data = (float *) malloc(buf_size * sizeof *frame_data);

    pulse_length = next_pulse_locn - fplocn[pulse_count];

    if (flnt + pulse_length / 2 > dynamic_frame_size)
    {
	dynamic_frame_size -= flnt - frame_size;
	if (debug_level)
	{
	    Fprintf (stderr, "Locn = %d\tSize = %d\n", fplocn[0] + start, flnt);
	    for (i = 0; i < pulse_count; i++)
		Fprintf (stderr, "%d\tLocn = %d\tSize = %d\n", i, fplocn[i], fpsize[i]);
	}
/* Spectrum Analysis of Data */

	j = fplocn[0] + frame_beg - lpc_order - P_OFFSET;

	(void)compute_rc(&raw_data[j], flnt + lpc_order, anal_method, 0, 
			 WT_RECT, lpc_order, sincn, 20 , 1e-5, rc, r, &gain); 

/* Make voicing decision */

	for (k = 0; k < pulse_count; k++)
	{
	    se = 0;
	    j = frame_beg + fplocn[k];
	    peak = res_data[j];
	    j -= P_OFFSET;
	    for (i = 0; i < fpsize[k]; i++)
		se += res_data[j++];
	    fpse[k] = se;

	    noz = 0;
	    j = frame_beg + fplocn[k] - P_OFFSET;
	    for (i = 0; i < fpsize[k]; i++)
	    {
		t = raw_data[j++];
		if (prev_t * t < 0)
		    noz++;
		prev_t = t;
	    }

	    fptype[k] = UNVOICED_FRM;
	    t = fpsize[k];
	    threshold = PEAK_THRESHOLD * (1.0 - pow (tcons, t));
	    if (noz <= ZCROSS_THRESHOLD * fpsize[k] || se < peak * threshold)
		fptype[k] = VOICED_FRM;

	    if (noz < 2 || noz < 0.03 * fpsize[k])
		fptype[k] = UNVOICED_FRM;

	    if (rc[1] < K1_THRESHOLD || gain > LPC_GAIN_THRESHOLD)
		fptype[k] = UNVOICED_FRM;

	    if (se < 0.4 * peak * threshold)
		fptype[k] = VOICED_FRM;

	    if (debug_level && peak && gain)
		Fprintf (stderr, "%6d\t%3d\t%6.3f\t%3d\t%8.2f\t%6.3f\t%6.3f\t%d\n",
			start + fplocn[k], fpsize[k], se / peak, noz, 1.0 / gain, rc[1], rc[2], fptype[k]);
	}

	vcount = 0;
	for (i = 0; i < pulse_count; i++)
	    if (fptype[i] == VOICED_FRM)
		vcount++;

/* Voiced frames should have all blocks voiced */
	if (2 * vcount > pulse_count)
	    for (i = 0; i < pulse_count; i++)
		fptype[i] = VOICED_FRM;

/* Output Analysis Data */

	for (i = 0; i < pulse_count; i++)
	{

	    if (fpsize[i])
	    {
		*anafea_rec -> tag = start + fplocn[i];
		*anafea_rec -> frame_len = fpsize[i];
		if (fptype[i] == VOICED_FRM)
		{
		    *anafea_rec -> frame_type = VOICED;
		    *anafea_rec -> voiced_fraction = 1.0;
		}
		else
		{
		    *anafea_rec -> frame_type = UNVOICED;
		    *anafea_rec -> voiced_fraction = 0.0;
		}

		anafea_rec -> p_pulse_len[0] = fpsize[i];
		*anafea_rec -> num_pulses = 1;

		fpse[i] = fpse[i] / fpsize[i];

		if (fpse[i] == 0)
			fpse[i] = 1.0;

		if (!Fflag && (fpsize[i] > 12)) {
		    for (m = 0; m < fpsize[i]; m++) 
			frame_data[m] = raw_data[m + fplocn[i] + frame_beg];

		    if (debug_level > 1) 
			pr_farray(frame_data, fpsize[i], "DC-removed frame");

		    (void)compute_rc(frame_data, fpsize[i], anal_method, 0, 
				     WT_RECT, lpc_order, sincn, 20 , 1e-5, 
				     rcf, rf, &fgain); 

		    for (j = 0; j < lpc_order; j++)
			anafea_rec -> spec_param[j] = rcf[j + 1];
		}
		else {

		    if (debug_level && (!Fflag) && (fpsize[i] <= 12))
			Fprintf(stderr, 
		    "lpcana: very short pulse, didn't recalculate spectrum\n");


		    for (j = 0; j < lpc_order; j++)
			anafea_rec -> spec_param[j] = rc[j + 1];
		}
		anafea_rec -> raw_power[0] = fpse[i] / gain;
		anafea_rec -> lpc_power[0] = fpse[i];

		put_anafea_rec (anafea_rec, fea_oh, fptr_fea_ana_file);
	    }
	}
	fplocn[0] = fplocn[pulse_count];
	pulse_count = 0;
	flnt = 0;
    }
    fpsize[pulse_count] = pulse_length;
    flnt += pulse_length;
    pulse_count++;
    fplocn[pulse_count] = next_pulse_locn;
}

/* Remove dc value in a data sequence */
remove_dc (x, size)
    float   x[];
    short   size;
{
#define c0	63.0 / 64.0
#define c1	1.0 / 64.0
    short   i;
    static float    dc = 0.0;
    for (i = 0; i < size; i++)
    {
	dc = c0 * dc + c1 * x[i];
	x[i] -= dc;
    }
}

/* Whiten a given segment of data using linear predicition */
whiten_data (x, lnt, order, matsiz, output)
    float   x[], output[];
    int     order, lnt, matsiz;
{
/*
Whiten a given segment of data

input:
	x - input data array of size "lnt"

parameters:
	order - No. of reflection coefficients
	matsiz - sample covariance matrix size

output:
	output - square of the residual signal
	
*/
    register double num, den, dtemp;
    register float  stemp;
    double  f0Tf0, b0Tb0, f0Tb0;
    float  *ptr1, *f, *b;
    int     i, n, stage, vctsiz;
#define rcstage	num

    lnt += order;
/* Allocate memory for f and b arrays */
    f = (float *) malloc ((unsigned)((lnt+1) * sizeof *f));
    if (f == NULL)
    {
	Fprintf (stderr, "whitendata: couldn't allocate dynamic memory for array - f\n");
	exit (1);
    }

    b = (float *) malloc ((unsigned)((lnt+1) * sizeof *b));
    if (b == NULL)
    {
	Fprintf (stderr, "whitendata: couldn't allocate dynamic memory for array - b\n");
	exit (1);
    }

    for (n = 0; n < lnt; n++)
	f[n] = b[n] = x[n];

    f[lnt] = b[lnt] = 0.0;

    vctsiz = lnt - matsiz + 1;

    /* Computesignal energy */

    for (dtemp = 0.0, n = 0; n < vctsiz; n++)
	dtemp += f[n] * f[n];

    b0Tb0 = den = dtemp;
    f0Tf0 = den + f[vctsiz] * f[vctsiz] - f[0] * f[0];

    for (ptr1 = &f[vctsiz], n = 0; n < matsiz - 1; n++, ptr1++)
    {
	dtemp += *ptr1 * *ptr1 - f[n] * f[n];
	den += dtemp;
    }
    den = 2 * den - b0Tb0 - dtemp;

    for (stage = 1; stage <= order; stage++)
    {

	/* Compute first inner product */

	for (ptr1 = &f[stage], dtemp = 0.0, n = 0; n < vctsiz; n++)
	    dtemp += *ptr1++ * b[n];

	/*  Compute recursively the inner product of other vectors */

	f0Tb0 = num = dtemp;
	for (n = stage; n < matsiz - 1; n++)
	{
	    i = vctsiz + n;
	    dtemp += f[i] * b[i - stage] - f[n] * b[n - stage];
	    num += dtemp;
	}

	/*  Compute reflection coefficient and the residuals */

	rcstage = 2.0 * num / den;
	for (ptr1 = b, n = stage; n < lnt; n++)
	{
	    stemp = *ptr1;
	    *ptr1++ = stemp - rcstage * f[n];
	    f[n] -= rcstage * stemp;
	}

	/* Update b0Tb0 and compute of bnTbn */

	dtemp = b0Tb0 + rcstage * rcstage * f0Tf0 - 2.0 * rcstage * f0Tb0;
	f0Tf0 = f0Tf0 + rcstage * rcstage * b0Tb0 - 2.0 * rcstage * f0Tb0;
	i = matsiz - stage - 1;
	for (b0Tb0 = dtemp, n = 0; n < i; n++)
	{
	    stemp = b[n + vctsiz];
	    dtemp += stemp * stemp - b[n] * b[n];
	}

	/* Update den */

	den = den * (1.0 - rcstage * rcstage) - f0Tf0 - dtemp;

	/* Update f0Tf0 for next stage */

	stemp = f[vctsiz + stage];
	f0Tf0 += stemp * stemp - f[stage] * f[stage];
    }

/* Compute residual square and store in output array */
    n = 0;
    for (i = order; i < lnt; i++)
	output[n++] = f[i] * f[i];

    free ((char *) f);
    free ((char *) b);
}

