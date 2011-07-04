/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 *
 * Written By: S. Shankar Narayan
 * Modified for ESPS 3.0 by David Burton
 * Module:	lpcsynt.c
 *
 * lpcsynt [-x] [-p range] [-r range] [-P parfile] fea_ana_file sd_file
 *
 * This is the main module of the ESPS program lpcsynt, which performs 
 * pitch synchronous synthesis from an FEA_ANA file.  
*/

#ifndef lint
static char *sccs_id = "@(#)lpcsynt.c	3.13 6/19/91 ESI";
#endif

/*
 * System includes
*/

#include <stdio.h>

/*  
 * ESPS Includes
*/
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/anafea.h>
#include <esps/feasd.h>
#include <esps/unix.h>

/*
 * DEFINES
*/
#define SYNTAX USAGE("lpcsynt [-p range] [-r range] [-x] [-P params_file] fea_ana_file sd_file")
#define ORDER 100
#define BUFSIZE 512
#define TWENTY 20
#define MAX_PFILT_SIZE TWENTY
/*
 * ESPS Functions
*/
long   *add_genhd_l ();
short  *add_genhd_s ();
float  *add_genhd_f ();
char   *add_genhd_c ();
void write_header ();
struct header  *read_header ();
struct anafea  *allo_anafea_rec ();
int     get_anafea_rec ();
void put_anafea_rec ();
char *eopen();
void lrange_switch();
void put_sd_recs();
void add_genzfunc();

/* 
 * System Functions
char   *strcpy (char *to, char *from);
*/

/*
 * Global Variables
*/

int     debug_level = 0;
float   int_rc[ORDER], rc0[ORDER], rc1[ORDER], rc2[ORDER];
struct anafea  *anafea_rec;

main (argc, argv)
int     argc;
char  **argv;
{
    char   *Version = "3.13";
    char    *Date = "6/19/91";

    FILE * fea_strm = stdin;	    /*input file strm ptr*/
    FILE * sd_strm = stdout;	    /*output file strm ptr*/
    char   *fea_ana_file = "<stdin>"; /*input file name*/
    char  *out_sd_file = "<stdout>"; /*output file name*/
    struct header  *fea_ih, *sd_oh; /*ptr to input file header*/
    int     c;			    /* option switch variable*/

    float   outbfr[BUFSIZE];	    /*holds synthesized data*/
    float  synstate[TWENTY];	    /*holds state variables*/
    short   bfr[BUFSIZE];	    /*holds output data as shorts*/
    float   lattice_filt ();	    /*synthesize data*/
    float   gauss ();		    /* make unvoiced excitation*/
    float   res_power;		    /*holds residual from fea_ana frame*/
    double  sqrt ();
    double  rmsval;		    /*holds rms residual energy*/
    double  input;		    /*input energy to lattice filt*/

    extern char *optarg;
    extern  optind;
    char   *get_cmd_line (), *cmd_line;
    char   *param_file = NULL;
    char   *p_switch = NULL;	    /*holds -p option text*/
    char   *r_switch = NULL;
    int    rflag = 0;
    int    pflag = 0;

    long     start, nan;
    int synt_filt_order = 0;	    /*holds filter order*/
    long synt_order;
    int frame_size;		    /*current analysis frame size*/
    long locn;			    /*current location in output data file*/
    int     blksze, oblksze, prev_seg_type, seg_type;
    long  start_p, end_p, i;
    long  start_r, end_r, nan_r;
    short   range_err = 0;
    double record_freq;             /*holds sample frequency of 
				      original sampled data*/
    short var1, var2, var3, var4, 
                var5, var6;         /* temporarily hold symbolic coded values*/
    double start_time = 0;
    double new_start_time = 0;

    int     pfilt_order, ns, ds;
    double  tmpd[MAX_PFILT_SIZE];
    float   pfilt_num[MAX_PFILT_SIZE], pfilt_den[MAX_PFILT_SIZE], 
                  pfilt_state[MAX_PFILT_SIZE];

    cmd_line = get_cmd_line (argc, argv);
    while ((c = getopt (argc, argv, "P:xr:p:")) != EOF)
    {
	switch (c)
	{
	    case 'P': 
		param_file = optarg;
		break;
	    case 'x': 
		debug_level++;
		break;
            case 'r':
		rflag++;
		r_switch = optarg;
		break;
	    case 'p': 
		pflag++;
		p_switch = optarg;
		break;
	    default: 
		SYNTAX;
		break;
	}
    }
    if (optind < argc) 
	fea_ana_file = eopen("lpcsynt", argv[optind++], "r", 
			    FT_FEA, FEA_ANA, &fea_ih, &fea_strm);
    else {
        Fprintf(stderr, "lpcsynt: no input FEA_ANA file specified.\n");
        SYNTAX;
    }

    if (optind < argc) 
	out_sd_file = eopen("lpcsynt", argv[optind++], "w", 
			    NONE, NONE, (struct header **)NULL, &sd_strm);
    else {
        Fprintf(stderr, "lpcsynt: no output file specified.\n");
        SYNTAX;
    }

    if(debug_level > 0)
	Fprintf(stderr, "lpcsynt: input file = %s, output file = %s\n",
			fea_ana_file, out_sd_file);


/* get stuff from input header*/

    if (*(short *) get_genhd ("spec_rep", fea_ih) != RC)
    {
	Fprintf (stderr, "lpcsynt: input FEA_ANA file doesn't contain RCs");
	exit (1);
    }

    frame_size = (int)(*get_genhd_s ("nominal_frame_size", fea_ih));

/* create output header*/
    sd_oh = new_header (FT_FEA);
    add_source_file (sd_oh, fea_ana_file, fea_ih);
    add_comment (sd_oh, cmd_line);
    sd_oh -> variable.refer = fea_ih -> variable.refer;
    sd_oh -> common.tag = NO;

    (void)strcpy (sd_oh -> common.prog, "lpcsynt");
    (void)strcpy (sd_oh -> common.vers, Version);
    (void)strcpy (sd_oh -> common.progdate, Date);

/*
 * Initialize fea_sd header
*/
    if((record_freq =  get_genhd_val ("src_sf", fea_ih, (double) -1.)) == -1){
      Fprintf(stderr, 
	      "lpcsynt: Source sample frequency in %s is bogus - exiting.\n", 
	      fea_ana_file);
      exit(1);
    }

    if((init_feasd_hd(sd_oh, SHORT, 1, &start_time, 0, record_freq)) !=0){
      Fprintf(stderr, "lpcsynt: Couldn't allocate FEA_SD header - exiting.\n");
      exit(1);
    }
/*
 * fill in generics
*/

    (void)add_genhd_d("src_sf", &record_freq, 1, sd_oh);
    var1 = PULSE;
    (void)add_genhd_e("synt_interp", &var1, 1, synt_inter_methods, sd_oh);
    var2 = LPCPULSE;
    (void)add_genhd_e("synt_pwr", &var2, 1, synt_pwr_codes, sd_oh);
    var3 = ANA;
    (void)add_genhd_e("synt_rc", &var3, 1, synt_ref_methods, sd_oh);
    var4 = IMPULSE;
    (void)add_genhd_e("v_excit_method", &var4, 1, excit_methods, sd_oh);
    var5 = WHITE;
    (void)add_genhd_e("uv_excit_method", &var5, 1, excit_methods, sd_oh);
    var6 = PSYNCH;
    (void)add_genhd_e("synt_method", &var6, 1, synt_methods, sd_oh);

    synt_filt_order = *(long *) get_genhd ("order_vcd", fea_ih);
    synt_order = synt_filt_order;
    (void)add_genhd_l("synt_order", &synt_order, 1, sd_oh);


    /* read ESPS parameter file */
    (void)read_params(param_file, SC_NOCOMMON, (char *)NULL);

    symerr_exit();

    /*
     * Initialize filter arrays to zero
    */

    for(i = 0; i < MAX_PFILT_SIZE; i++){
      pfilt_num[i] = 0;
      pfilt_den[i] = 0;
      pfilt_state[i] = 0;
    }

/*
 * Get post filter numerator parameters
*/
    if(symtype("post_filt_num") == ST_UNDEF){
      pfilt_num[0] = 1.;
      pfilt_num[1] = -1.;
      pfilt_num[2] = 0.;
      ns = 3;
    }
    else if ((ns = getsym_da ("post_filt_num", tmpd, MAX_PFILT_SIZE)) ==  0){
      exit(1);
    }
    else
      for (i = 0; i < ns; i++)
	pfilt_num[i] = (float)tmpd[i];

/*
 * Get postfilter denominator parameters
*/
    if(symtype("post_filt_den") == ST_UNDEF){
      pfilt_den[0] = 1.;
      pfilt_den[1] = -.875;
      pfilt_den[2] = 0.;
      ds = 3;
    }
    else if ((ds = getsym_da ("post_filt_den", tmpd, MAX_PFILT_SIZE)) == 0){
      exit(1);
    }
    else
      for (i = 0; i < ds; i++)
	pfilt_den[i] = (float)tmpd[i];

    if (ns > ds)
	pfilt_order = ns;
    else
	pfilt_order = ds;

    (void)add_genzfunc("prefilter", 
		       new_zfunc (ns, ds, pfilt_num, pfilt_den), sd_oh);

/*parse range */
    start = start_p = *(long *) get_genhd ("start", fea_ih);
    nan = *(long *) get_genhd ("nan", fea_ih);
    end_p = start + nan - 1;
    if (pflag)
    {
	lrange_switch (p_switch, &start_p, &end_p, 1);
	if ((start_p < start) || (end_p > (start + nan - 1)))
	    range_err++;
	start = start_p;
	nan = end_p - start_p + 1;
    }
    else if (rflag){
      lrange_switch(r_switch, &start_r, &end_r, 1);
      if(start_r < 0)
	range_err++;
      nan_r = end_r - start_r + 1;
    }
    else if (rflag && pflag){
      Fprintf(stderr, 
     "lpcsynt: \"r\" and \"p\" options cannot both be specified - exiting.\n");
      exit(1);
    }
    else/* set pflag*/
      pflag++;

    if (range_err > 0)
    {
	Fprintf (stderr,
		"lpcsynt: specified range %d:%d not in input file\n",
		start_p, end_p);
	exit (1);
    }


/* allocate fea record */
    anafea_rec = allo_anafea_rec (fea_ih);

/* Minor initializations */

    for (i = 0; i < TWENTY; i++)
      synstate[i] = 0.0;

    if(pflag){/* Skip initial points */
    i = 0;
    do
    {
	if (!get_ana (&locn, &seg_type, &blksze, &res_power,
		    synt_filt_order, fea_ih, fea_strm))
	{
	    Fprintf (stderr, "lpcsynt: EOF encountered before start reached\n");
	    exit (1);
	}
	if (debug_level)
	    Fprintf (stderr, "Locn = %ld\tlag = %d\n",
		    locn, blksze);
	i++;
    } while (start > locn || blksze == 0);
  }
 else{/*skip initial records*/
    i = 0;
    do{
	if (!get_ana (&locn, &seg_type, &blksze, &res_power,
		    synt_filt_order, fea_ih, fea_strm))
	{
	    Fprintf (stderr, "lpcsynt: EOF encountered before start reached\n");
	    exit (1);
	}
	if (debug_level)
	    Fprintf (stderr, "Locn = %ld\tlag = %d\n",
		    locn, blksze);
	i++;
	start = locn;

    } while (start_r >= i || blksze == 0);
  }


    if(start == 0){
	  /* fudge start time from record freq*/
	  start = record_freq*(start_r-1)/get_genhd_val(
			"record_freq", fea_ih, (double) 1.);
	}
      
    if (debug_level)
	Fprintf (stderr, "start: locn = %ld\n", locn);
/*
 * update start_time generic
*/
    start_time = get_genhd_val("start_time", fea_ih, (double)0);

    new_start_time = start_time + (start-1)/record_freq;

    (void)add_genhd_d("start_time", &new_start_time, (int)1, sd_oh);      


/*
 * ALL set, now write output header
*/
    write_header (sd_oh, sd_strm);

/* Insert leading zeros if start < locn. If the difference is greater than
   one frame, complain and exit */

    if (i == 1 && locn - start > frame_size)
    {
	Fprintf (stderr,
		"lpcsynt: no analysis data near point %d; first locn = %ld\n",
		start, locn);
	exit (1);
    }
    zeros ((locn - start), sd_oh, sd_strm);
    prev_seg_type = VOICED;

/* Do the desired speech */
  if(pflag){
    while (locn < (start + nan))
    {
	if (debug_level)
	    Fprintf (stderr, "locn = %ld\tsize = %d\n", locn, blksze);
	if (prev_seg_type == UNVOICED && seg_type == VOICED)
	    for (i = 0; i <= synt_filt_order; synstate[i++] = 0.0);

	if (seg_type == VOICED)
	{
	    rmsval = sqrt ((double) res_power * blksze);
	    input = rmsval;
	    outbfr[0] = lattice_filt (int_rc, synt_filt_order, synstate, input);

	    input = 0.0;
	    for (i = 1; i < blksze; i++)
		outbfr[i] = lattice_filt (int_rc, synt_filt_order, synstate, input);

	}
	else
	{
	    /*  Unvoiced segment */
	    rmsval = sqrt ((double) res_power);
	    for (i = 0; i < blksze; i++)
	    {
		input = rmsval * gauss ();
		outbfr[i] = lattice_filt (int_rc, synt_filt_order, synstate, input);
	    }
	}

	/* Post-filtering */
	iir_filter (outbfr, blksze, pfilt_num, pfilt_den, pfilt_state, pfilt_order);

	/* convert output data to shorts*/
	for (i = 0; i < blksze; i++){
	    bfr[i] = (short)outbfr[i];
        }

	/*put out data*/
	put_sd_recs (bfr, blksze, sd_oh, sd_strm);

	prev_seg_type = seg_type;
	oblksze = blksze;
	if (!get_ana (&locn, &seg_type, &blksze, &res_power,
		    synt_filt_order, fea_ih, fea_strm))
	{

/* Don't beef if we're within one frame of the end */
	    if (start + nan - locn - oblksze > frame_size)
	    {
		Fprintf (stderr, "lpcsynt: EOF encountered in input file\n");
		Fprintf(stderr, 
		"lpcsynt: Input FEA file represents only %ld points\n",
		(long)(locn+oblksze));
		exit (2);
	    }
	    else
	    {
		zeros ((start + nan - locn), sd_oh, sd_strm);
		exit (0);
	    }
	}
	if (debug_level)
	    Fprintf (stderr, "Locn = %ld\tlag = %d\n",
		    locn, blksze);
    }
  }
  else{/*the -r option was used*/
    int used = 1;
    while (used <= nan_r){

	if (debug_level)
	    Fprintf (stderr, "locn = %ld\tsize = %d\n", locn, blksze);
	if (prev_seg_type == UNVOICED && seg_type == VOICED)
	    for (i = 0; i <= synt_filt_order; synstate[i++] = 0.0);

	if (seg_type == VOICED)
	{
	    rmsval = sqrt ((double) res_power * blksze);
	    input = rmsval;
	    outbfr[0] = lattice_filt (int_rc, synt_filt_order, synstate, input);

	    input = 0.0;
	    for (i = 1; i < blksze; i++)
		outbfr[i] = lattice_filt (int_rc, synt_filt_order, synstate, input);

	}
	else
	{
	    /*  Unvoiced segment */
	    rmsval = sqrt ((double) res_power);
	    for (i = 0; i < blksze; i++)
	    {
		input = rmsval * gauss ();
		outbfr[i] = lattice_filt (int_rc, synt_filt_order, synstate, input);
	    }
	}

	/* Post-filtering */
	iir_filter (outbfr, blksze, pfilt_num, pfilt_den, pfilt_state, pfilt_order);

	/* convert output data to shorts*/
	for (i = 0; i < blksze; i++){
	    bfr[i] = (short)outbfr[i];
        }

	/*put out data*/
	put_sd_recs (bfr, blksze, sd_oh, sd_strm);

	prev_seg_type = seg_type;
	oblksze = blksze;
	if (!get_ana (&locn, &seg_type, &blksze, &res_power,
		    synt_filt_order, fea_ih, fea_strm))
		exit (0);
      
	/*update "used" counter*/
	used++;

	if (debug_level)
	    Fprintf (stderr, "Locn = %ld\tlag = %d\n",
		    locn, blksze);
      }
  }

    exit (0);
    return(0); /*lint pleasing*/
}

zeros (n, h, fd)
long n;
struct header  *h;
FILE * fd;
/*add a zero to output file*/
{
    static short    zero[1] =
    {
	0
    };
    while (n-- > 0)
	put_sd_recs (zero, 1, h, fd);
}

fcopy (dst, src, n)
float  *dst, *src;
/*copy float pointer*/
{
    while (n-- > 0)
	*dst++ = *src++;
}


get_ana (plocn, ptype, psize, pres_power,
    synt_filt_order, hd, stream)
long   *plocn;
int    *ptype, *psize, synt_filt_order;
float  *pres_power;
struct header  *hd;
FILE * stream;

/* This routine reads parameters from the fea_ana file and 
returns interpolated spectral parameters */

{
    int     i, rcs_not_ok;
    static long  locn[3];
    static int  type[3], size[3], EOF_flag = 0;
    static float    res_power[3];

    if (debug_level)
	Fprintf (stderr, "get_ana: ");

    if (EOF_flag)
    {
	return 0;
    }

    if (!read_ana (&locn[2], &size[2], &type[2], &res_power[2], rc2,
		synt_filt_order, hd, stream))
    {
	/*  EOF encountered. Flush buffer */
	*plocn = locn[1];
	*ptype = type[1];
	*psize = size[1];
	*pres_power = res_power[1];
	EOF_flag = 1;
	return 1;
    }
    /* Check if rc's are valid.  Otherwise, complain and quit */
    rcs_not_ok = 0;
    for (i = 0; i < synt_filt_order; i++)
	if (rc2[i] * rc2[i] > 1) {
	    rc2[i] = .99 * ((rc2[i] > 0) ? 1 : -1);
	    rcs_not_ok = 1;
	}
    if (rcs_not_ok)
    {
	Fprintf(stderr, 
	    "Invalid RC's in record starting at locn %ld\n", locn[2]);
	Fprintf(stderr, "  replaced with + or - .99\n");
    }

    if (type[0] == VOICED && type[1] == VOICED && type[2] == VOICED)
	for (i = 0; i < synt_filt_order; i++)
	    int_rc[i] = (rc0[i] + rc1[i] + rc2[i]) / 3.0;
    else
	for (i = 0; i < synt_filt_order; i++)
	    int_rc[i] = rc1[i];

    *ptype = type[1];
    *plocn = locn[1];
    *pres_power = res_power[1];
    *psize = size[1];
    *ptype = type[1];

    for (i = 0; i < 2; i++)
    {
	type[i] = type[i + 1];
	res_power[i] = res_power[i + 1];
	size[i] = size[i + 1];
	locn[i] = locn[i + 1];
    }

    for (i = 0; i < synt_filt_order; i++)
	rc0[i] = rc1[i];

    for (i = 0; i < synt_filt_order; i++)
	rc1[i] = rc2[i];
    return 1;
}


read_ana (plocn, psize, ptype, pres_power, rc,
    synt_filt_order, hd, stream)
long   *plocn;
int    *psize, *ptype;
float  *pres_power;
float   rc[];
int     synt_filt_order;
struct header  *hd;
FILE * stream;
/* get data from a FEA_ANA frame*/
{
    if (get_anafea_rec (anafea_rec, hd, stream) == EOF)/* eof */
    {
	if (debug_level)
	    Fprintf (stderr, "no more anafea_records.\n");
	return 0;
    }

    *plocn = *anafea_rec -> tag;
    *psize = *anafea_rec -> frame_len;
    *ptype = *anafea_rec -> frame_type;
    *pres_power = anafea_rec -> lpc_power[0];

    fcopy (rc, anafea_rec -> spec_param, synt_filt_order);

    if (debug_level)
	Fprintf (stderr, "locn: %ld, size: %d, power: %f, order: %d.\n",
		*plocn, *psize, *pres_power, synt_filt_order);
    return 1;
}

iir_filter (outbfr, blksze, num, den, state, order)
float   outbfr[], num[], den[], state[];
int     blksze, order;
/*post-empahsis filtering*/
{
    int     i, j;
    float   inp, out;
    for (i = 0; i < blksze; i++)
    {
	inp = outbfr[i];
	for (j = 1; j <= order; j++)
	    inp -= state[j] * den[j];
	out = inp * num[0];
	for (j = 1; j <= order; j++)
	    out += state[j] * num[j];
	for (j = order; j > 1; j--)
	    state[j] = state[j - 1];
	state[1] = inp;
	outbfr[i] = out;
    }
}

/* Synthesis filter in ladder form */

float   lattice_filt (rc, order, lsstate, input)
float   rc[], lsstate[], input;
int     order;
{
    int     i;
    float   output, ki;

    output = input;

    for (i = order; i > 0; i--)
    {
	ki = rc[i - 1];
	output += ki * lsstate[i];
	lsstate[i + 1] = lsstate[i] - ki * output;
    }
    lsstate[1] = output;
    return (output);
}
