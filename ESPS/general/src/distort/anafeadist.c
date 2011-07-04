/* anafea_distort - compute distortion measures between two ESPS FEA_ANA files
 *
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *
 * Module Name:  ana_distort
 *
 * Written By:  Ajaipal S. Virdy
 *
 *
 * Purpose:  compute distortion measures between two ESPS FEA_ANA files
 *	     for comparing speech processes
 *
 *
 */

#ifdef SCCS
	static char *sccs_id = "@(#)anafeadist.c	3.3	1/27/93	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/anafea.h>
#include <esps/fea.h>
#include <math.h>
#include <esps/unix.h>

/* declare external variables for modules in distort */

#include  "distort.h"

/*
 *   G L O B A L    V A R I A B L E S
 *
 *   used in ana_distort and print_distort.
 *
 */

float	*ref_coeff1;	/* reflection coefficients for first file */
float	*ref_coeff2;	/* reflection coefficients for second file */

char	**frame;	/* string for "Voiced" or "Unvoiced" frames */
int	*ref_c_len;	/* length of array ref_coeff for each record */
int	*pitch_len;	/* length of array p_pulse_len for each " */
int	*raw_len;	/* length of array raw_power for each record */
int	*lpc_len;	/* length of array lpc_power for each record */

float	**diff_ref_c;	/* difference between ref_coeff vector */
float	**diff_pitch;	/* difference between p_pulse_len vector */
float	**diff_raw;	/* difference between raw_power vector */
float	**diff_lpc;	/* difference between lpc_power vector */

double	*IS_dist;	/* Itakura-Saito (IS) distortion measure vector */
double	*GNIS_dist;	/* Gain-Normalized IS distortion measure vector */
double	*GOIS_dist;	/* Gain-Optimized IS distortion measure vector */

float	tot_ref_c;	/* variable to sum all ref_coeff */
float	tot_pitch;	/* variable to sum all p_pulse_len */
float	tot_raw;	/* variable to sum all raw_power */
float	tot_lpc;	/* variable to sum all lpc_power */

int	maxorder;
int	maxpulses;
int	maxraw;
int	maxlpc;

long	order_vcd;
long	order_unvcd;

int	i_rec;		/* current record */
int	rec_num;	/* current record number */
int	ele_num;	/* current element number */

/*
 * U N I X
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

/* done via <esps/unix.h>

/*
 * E S P S
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

char	*get_genhd();
short	get_rec_len();
int	get_anafea_rec();

/*
 * L O C A L
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

void	allo_memory();
int	array_len();
static void	check_anafea_arrays();
static char	*check_frames();
static void	convert_to_rc();
void	compute_is_dist();
void	gen_distort();
int	reps_rc();
static void	type_conflict();

/*
 * B E G I N
 *  P R O G R A M
 */

void
anafea_distort()
{
   struct anafea	*anafea_rec1;	/* data structure for file1 */
   struct anafea 	*anafea_rec2;	/* data structure for file2 */

   int	rows;		/* size of row to allocate */

   int	j;		/* temporary indices */

/*
 *  If the -e option was given then treat the FEA_ANA file as
 *  "generic" file.
 *
 */

   if ( eflag ) {
      (void) gen_distort ();
      return;
   }

   if ( debug_level > 3)
      (void) fprintf (stderr, "\n");

   if ( get_rec_len (f1h) != get_rec_len (f2h) ) {
      (void) fprintf (stderr,
      "anafea_distort: warning, elements per record not equal in both files.\n");
    } else
	if (debug_level > 4)
	   (void) fprintf (stderr,
	   "anafea_distort: same number of elements in these FEA_ANA files.\n");

   /* skip appropiate number of records (as specified by the -f option) */

   anafea_rec1 = allo_anafea_rec (f1h);
   anafea_rec2 = allo_anafea_rec (f2h);

   if (debug_level > 1)
      (void) fprintf (stderr,
      "anafea_distort: skipping %d records.\n", s_rec - 1);

/* We've already skipped appropriate number of records */

/*
 * This is the way I originally skipped records:
 *
 *   for ( rec_num = 1; rec_num < s_rec; rec_num++ ) {
 *
 *       if ( get_anafea_rec (anafea_rec1, f1h, f1p) == EOF ) {
 *	  (void) fprintf (stderr,
 *	  "anafea_distort: not enough records in %s.\n", f1str);
 *	  exit (1);
 *       }
 *       if ( get_anafea_rec (anafea_rec2, f2h, f2p) == EOF ) {
 *	  (void) fprintf (stderr,
 *	  "anafea_distort: not enough records in %s.\n", f2str);
 *	  exit (1);
 *       }
 *
 *   } \* end for *\
 */

   if ((order_vcd = *(long *) get_genhd ("order_vcd", f1h)) !=
		*(long *) get_genhd ("order_vcd", f2h)) {
	(void) fprintf (stderr,
	"anafea_distort: order_vcd not equal in %s and %s.\n",
	f1str, f2str);
	exit (1);
   }

   if ((order_unvcd = *(long *) get_genhd ("order_unvcd", f1h)) !=
		*(long *) get_genhd ("order_unvcd", f2h)) {
	(void) fprintf (stderr,
	"anafea_distort: order_unvcd not equal in %s and %s.\n",
	f1str, f2str);
	exit (1);
   }

   (void) fflush (stderr);

/*
 * The following variables are used to compute the size of
 * our multi-dimensional arrays
 */

     maxorder = MAX (order_vcd, order_unvcd);
    maxpulses = MAX (*(long *) get_genhd ("maxpulses", f1h),
		     *(long *) get_genhd ("maxpulses", f2h));
       maxraw = MAX (*(long *) get_genhd ("maxraw", f1h),
		     *(long *) get_genhd ("maxraw", f2h));
       maxlpc = MAX (*(long *) get_genhd ("maxlpc", f1h),
		     *(long *) get_genhd ("maxlpc", f2h));

    if (debug_level > 5) {
	Fprintf (stderr,
	"anafea_distort: order_vcd = %ld, order_unvcd = %ld\n",
	order_vcd, order_unvcd);
	Fprintf (stderr,
	"anafea_distort: maxpulses = %ld, maxraw = %ld, maxlpc = %ld\n",
	maxpulses, maxraw, maxlpc);
	(void) fflush (stderr);
    }

/*
 * Now allocate:
 *
 *   number of records [rows]
 * 	    by
 *   MAX(order_vcd, order_unvcd) + maxpulses + maxraw + maxlpc
 *
 * memory space to store all
 * differences between elements in each record for computing the Element
 * Average, Record Average, and File Average.
 *
 */

   rows = (e_rec - s_rec + 1);	/* number of rows to allocate */

   /* allocate memory for all arrays */

   (void) allo_memory (rows, maxorder, maxpulses, maxraw, maxlpc);

/*
 * Here is the big loop for ESPS FEA_ANA files.
 * Note: we have to check for consistency
 *       each time through the loop.
 */


   if (debug_level > 4)
      (void) fprintf (stderr,
      "anafea_distort: for (rec_num = %d; rec_num <= %d; rec_num++) {\n",
      s_rec, e_rec);

   (void) fflush (stderr);

    tot_ref_c = 0.0;	/* variable to sum all ref_coeff */
    tot_pitch = 0.0;	/* variable to sum all p_pulse_len */
    tot_raw = 0.0;	/* variable to sum all raw_power */
    tot_lpc = 0.0;	/* variable to sum all lpc_power */


/*
 *
 *  M A I N    L O O P:
 *
 */

   for ( rec_num = s_rec, i_rec = 0; rec_num <= e_rec; rec_num++, i_rec++ ) {

     if (debug_level > 4)
	(void) fprintf (stderr,
	"\n  anafea_distort: getting record no. %d.\n", rec_num);

     if( get_anafea_rec (anafea_rec1, f1h, f1p) == EOF){
	 Fprintf(stderr,
	    "distort: anafeadist: Hit end of file in input file 1\n");
	exit(1);
     }
     if( get_anafea_rec (anafea_rec2, f2h, f2p) == EOF){
	 Fprintf(stderr,
	    "distort: anafeadist: Hit end of file in input file 2\n");
	exit(1);
   }
     /* check for consistency in file1 and file2 */

     if (debug_level > 4)
	(void) fprintf (stderr,
	"  anafea_distort: checking for consistency in this record.\n");

     frame[i_rec] = check_frames (anafea_rec1, anafea_rec2);

/*
 * Compare the lengths of all the arrays in anafea data structure
 * (i.e. the following arrays must be of the same size in each record,
 *       although the lengths can vary among different records).
 *
 */

      check_anafea_arrays (f1h, anafea_rec1, f2h, anafea_rec2);

/*
 * Determine the maximum element number for this record
 *
 */

	if ( strcmp (frame[i_rec], " Voiced ") == 0 ) {

	   e_ele = MAX (order_vcd, pitch_len[i_rec]);
	   e_ele = MAX (e_ele, raw_len[i_rec]);
	   e_ele = MAX (e_ele, lpc_len[i_rec]);

   /* save the length of ref_coeff array for computing the average */

	   ref_c_len[i_rec] = order_vcd;

	}
	else if ( strcmp (frame[i_rec], "Unvoiced") == 0 ) {

	   /* Unvoiced frame */

	   e_ele = MAX (order_unvcd, pitch_len[i_rec]);
	   e_ele = MAX (e_ele, raw_len[i_rec]);
	   e_ele = MAX (e_ele, lpc_len[i_rec]);

   /* save the length of ref_coeff array for computing the average */

	   ref_c_len[i_rec] = order_unvcd;

	}


     if (debug_level > 2) {
	(void) fprintf (stderr,
	"  anafea_distort: ref_c_len[%d] = %d, pitch_len[%d] = %d\n",

	i_rec, ref_c_len[i_rec], i_rec, pitch_len[i_rec]);
	(void) fprintf (stderr,
	"                 raw_len[%d] = %d,   lpc_len[%d] = %d\n",
	i_rec, raw_len[i_rec], i_rec, lpc_len[i_rec]);
     }

   (void) fflush (stderr);

     if ((ref_coeff1 = (float *) calloc ((unsigned) ref_c_len[i_rec],
				         sizeof (float))) == NULL ) {
	(void) fprintf (stderr,
	"anafea_distort: calloc: could not allocate memory for ref_coeff1.\n");
	exit (1);
     }

     if ((ref_coeff2 = (float *) calloc ((unsigned) ref_c_len[i_rec],
					 sizeof (float))) == NULL ) {
	(void) fprintf (stderr,
	"anafea_distort: calloc: could not allocate memory for ref_coeff2.\n");
	exit (1);
     }


/*
 * Convert spectral representation to reflection coefficients,
 * then compute the Itakura-Saito Distortion measures for this frame.
 */

     (void) convert_to_rc (anafea_rec1->spec_param,
			   anafea_rec2->spec_param, ref_c_len[i_rec]);

     (void) compute_is_dist (ref_coeff1, anafea_rec1->lpc_power[0],
			     ref_coeff2, anafea_rec2->lpc_power[0],
			     ref_c_len[i_rec]);

/*
 * Now begin to compute:
 *   Difference, Difference Magnitude, and Difference Magnitude Squared.
 */	

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "  anafea_distort: for (ele_num = %d, j = 0;\n", s_ele - 1);
	   (void) fprintf (stderr,
	   "                      ele_num < %d; ele_num++,j++) {\n",
	   e_ele);
	   (void) fflush (stderr);
	}

/*
 *
 *	E L E M E N T    L O O P:
 *
 */

	for (ele_num = s_ele - 1, j = 0; ele_num < e_ele; ele_num++, j++) {

	   if (debug_level > 4)
	      (void) fprintf (stderr,
	      "    anafea_distort: ele_num = %d, j = %d\n", ele_num, j);

	   diff_ref_c[i_rec][j] = ref_coeff1[ele_num] -
			   		ref_coeff2[ele_num];

	   tot_ref_c += diff_ref_c[i_rec][j];

	   if ( ele_num > pitch_len[i_rec] - 1 ) {
	       if (debug_level > 4)
		  (void) fprintf (stderr,
	       "    anafea_distort: not enough elements in p_pulse_len array.\n");
	   } else {

		diff_pitch[i_rec][j] = anafea_rec1->p_pulse_len[ele_num] -
					anafea_rec2->p_pulse_len[ele_num];

		tot_pitch += diff_pitch[i_rec][j];
	   }

	   if ( ele_num > raw_len[i_rec] - 1 ) {
	      if (debug_level > 4)
		 (void) fprintf (stderr,
		 "    anafea_distort: not enough elements in raw_power array.\n");
	   } else {

		diff_raw[i_rec][j] = anafea_rec1->raw_power[ele_num] -
					anafea_rec2->raw_power[ele_num];

		tot_raw += diff_raw[i_rec][j];
	   }

	   if ( ele_num > lpc_len[i_rec] - 1 ) {
	      if (debug_level > 4)
		 (void) fprintf (stderr,
		 "    anafea_distort: not enough elements in lpc_power array.\n");
	   } else {

		diff_lpc[i_rec][j] = anafea_rec1->lpc_power[ele_num] -
			 		anafea_rec2->lpc_power[ele_num];

		tot_lpc += diff_lpc[i_rec][j];
	   }


	   if (debug_level > 3) {
	      (void) fprintf (stderr,
	      "    anafea_distort: tot_diff: ref_c = %f, pitch = %f\n",
	      tot_ref_c, tot_pitch);
	      (void) fprintf (stderr,
	      "                             raw = %f,   lpc = %f\n",
	      tot_raw, tot_lpc);
	   }

	   (void) fflush (stderr);


	} /* end for */

	free ((char *) ref_coeff1);
	free ((char *) ref_coeff2);

   }  /* end big for loop */


   if ( Eflag )		/* then print results for each element */
      pr_ana_ele();

   if ( rflag && !Eflag ) {	/* print only Record and File Averages */
      pr_ana_rec_avg();   	/* print Record Average */
      pr_ana_file_avg(); 	/* print File Average */
      return;
   }

   pr_ana_ele_avg();	/* print Element Average */

   if ( rflag )
      pr_ana_rec_avg();  	/* print Record Average */

   pr_ana_file_avg();  	/* print File Average */

   pr_is_dist();	/* print Spectral Distortions */
   pr_tot_is_dist();	/* print TOTAL Spectral Distortions */

} /* end anafea_distort() */


static char *
check_frames (anafea_rec1, anafea_rec2)
struct anafea	*anafea_rec1;
struct anafea	*anafea_rec2;
{

/*
 * This module checks if two frames are compatible
 * (i.e. both frames must be either "Voiced" for "Unvoiced")
 * if one is "Voiced" and the other "Unvoiced", then
 * check_frames prints a message on stderr and exits with exit (1)
 * otherwise it returns a string of type of frame (" Voiced " or "Unvoiced").
 *
 */

	static char	*Voiced     = " Voiced ";
	static char	*Unvoiced   = "Unvoiced";
	static char	*Silence    = "Silence ";
	static char	*Transition = " Trans. ";

/*
 *	External Variable Used:
 *
 *   int	debug_level;	print messages for debugging purpose
 *   int	rec_num;	current record number
 *
 */

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");


    switch (*anafea_rec1->frame_type) {

	case VOICED:
		if (*anafea_rec2->frame_type != VOICED)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: voiced frame\n");
		return (Voiced);

	case UNVOICED:
		if (*anafea_rec2->frame_type != UNVOICED &&
			 *anafea_rec2->frame_type != NONE)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: unvoiced frame\n");
		return (Unvoiced);

	case NONE:
		if (*anafea_rec2->frame_type != UNVOICED &&
			 *anafea_rec2->frame_type != NONE)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: NONE type frame\n");
		return (Unvoiced);

	case SILENCE:
		if (*anafea_rec2->frame_type != SILENCE)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: silent frame\n");
		Fprintf (stderr,
		"anafea_distort: cannot handle SILENCE frames, yet.\n");
		exit (1);
		break;

	case UNKNOWN:
		if (*anafea_rec2->frame_type != UNKNOWN)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: silent frame\n");
		Fprintf (stderr,
		"anafea_distort: cannot handle UNKNOWN frames, yet.\n");
		exit (1);
		break;

	case TRANSITION:
		if (*anafea_rec2->frame_type != TRANSITION)
		    type_conflict (*anafea_rec1->frame_type,
		    *anafea_rec2->frame_type);
		if (debug_level > 3)
		    (void) fprintf (stderr,
		    "check_frames: transition frame\n");
		Fprintf (stderr,
		"anafea_distort: cannot handle TRANSITION frames, yet.\n");
		exit (1);
		break;

	default:
		(void) fprintf (stderr,
		"anafea_distort: unknown frame type, code: %d\n",
		*anafea_rec1->frame_type);
		exit (1);
		break;

    }

}	


static void
type_conflict (type1, type2)
short	type1;
short	type2;
{
    /* inconsistent frames */

    (void) fprintf (stderr,
    "anafea_distort: fatal error, one %s frame other %s frame.\n",
    frame_types[type1], frame_types[type2]);
    (void) fprintf (stderr,
    "anafea_distort: record number %d\n", rec_num);
    (void) fflush (stderr);
    exit (1);
}


static void
check_anafea_arrays (f1h, anafea_rec1, f2h, anafea_rec2)
struct header	*f1h;
struct anafea	*anafea_rec1;
struct header	*f2h;
struct anafea	*anafea_rec2;
{
   int	pitch_len2;  /* temporary variable to compare length of p_pulse_len */
   int	raw_len2;    /* temporary variable to compare length of raw_power */
   int	lpc_len2;    /* temporary variable to compare length of lpc_power */

   int    maxpulses1, maxpulses2;
   int    maxraw1, maxraw2;
   int    maxlpc1, maxlpc2;

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

     /* check if pitch pulse lengths are equal */

     maxpulses1 = *(int *) get_genhd ("maxpulses", f1h);
     maxpulses2 = *(int *) get_genhd ("maxpulses", f2h);

     pitch_len[i_rec] = array_len (anafea_rec1->p_pulse_len, maxpulses1, 0);
     pitch_len2 = array_len (anafea_rec2->p_pulse_len, maxpulses2, 0);

     if ( pitch_len[i_rec] != pitch_len2 ) {
	(void) fprintf (stderr,
      "check_anafea_arrays: pitch pulse lengths are not equal in record no. %d\n",
	 rec_num);
	 exit (1);
     }

     /* check if raw power lengths are equal */

     maxraw1 = *(int *) get_genhd ("maxraw", f1h);
     maxraw2 = *(int *) get_genhd ("maxraw", f2h);

     raw_len[i_rec] = array_len (anafea_rec1->raw_power, maxraw1, -1);
     raw_len2 = array_len (anafea_rec2->raw_power, maxraw2, -1);

     if ( raw_len[i_rec] != raw_len2 ) {
	(void) fprintf (stderr,
	"check_anafea_arrays: raw power lengths not equal in record no. %d\n",
	rec_num);
	exit (1);
     }

     /* check if lpc power lengths are equal */

     maxlpc1 = *(int *) get_genhd ("maxlpc", f1h);
     maxlpc2 = *(int *) get_genhd ("maxlpc", f2h);

     lpc_len[i_rec] = array_len (anafea_rec1->lpc_power, maxlpc1, -1);
     lpc_len2 = array_len (anafea_rec2->lpc_power, maxlpc2, -1);

     if ( lpc_len[i_rec] != lpc_len2 ) {
	(void) fprintf (stderr,
	"check_anafea_arrays: lpc power lengths not equal in record no. %d\n",
	rec_num);
	exit (1);
     }

}


static void
convert_to_rc (spec_param1, spec_param2, order)
float	*spec_param1;
float	*spec_param2;
int	order;
{

/*
 * G L O B A L
 *  V A R I A B L E S
 *   R E F E R E N C E D
 *
 * struct header *f1h;
 * struct header *f2h;
 *
 * float    *ref_coeff1;
 * float    *ref_coeff2;
 *
 */

     float  bw = 4000.;
     int    i;

/*
 * Convert the spectral parameter representations to reflection coefficients
 * for file1.
 */

    switch (*(short *) get_genhd ("spec_rep", f1h)) {

	case RC:
	     for (i = 0; i <= order - 1; i++)
		 ref_coeff1[i] = spec_param1[i];
	     break;

	case LAR:
	     if (reps_rc (spec_param1, LAR, ref_coeff1, order, bw) == -1)
	     {
		Fprintf (stderr,
		"anafea_distort: convert_to_rc: trouble converting from LAR to RC.\n");
		exit (1);
	     }
	     break;

	case LSF:

	     bw = *(float *) get_genhd ("src_sf", f1h) / 2.0;
 	     if (reps_rc (spec_param1, LSF, ref_coeff1, order, bw) == -1)
 	     {
 		Fprintf (stderr,
 		"anafea_distort: convert_to_rc: trouble converting from LSF to RC.\n");
 		exit (1);
 	     }

	     break;

	case AUTO:
	     if (reps_rc (spec_param1, AUTO, ref_coeff1, order, bw) == -1)
	     {
		Fprintf (stderr,
		"anafea_distort: convert_to_rc: trouble converting from AUTO to RC.\n");
		exit (1);
	     }
	     break;

	default:
	     Fprintf (stderr,
	     "anafea_distort: convert_to_rc: Unknown spectral representation, code: %d.\n",
	     *(short *) get_genhd ("spec_rep", f1h));
	     exit (1);
	     break;

    }  /* end switch (*(short *) get_genhd ("spec_rep", f1h)) */

     if (debug_level > 4) {
	for (i = 0; i <= order - 1; i++)
	    (void) fprintf (stderr,
	    "convert_to_rc: ref_coeff1[%d] = %f\n", i, ref_coeff1[i]);
	(void) fprintf (stderr, "\n");
     }

/*
 * Now convert the spectral parameter representations to reflection
 * coefficients for file2.
 */

    switch (*(short *) get_genhd ("spec_rep", f2h)) {

	case RC:
	     for (i = 0; i <= order - 1; i++)
		 ref_coeff2[i] = spec_param2[i];
	     break;

	case LAR:
	     if (reps_rc (spec_param2, LAR, ref_coeff2, order, bw) == -1)
	     {
		Fprintf (stderr,
		"anafea_distort: convert_to_rc: trouble converting from LAR to RC.\n");
		exit (1);
	     }
	     break;

	case LSF:

	     bw = *(float *) get_genhd ("src_sf", f2h) / 2.0;
 	     if (reps_rc (spec_param2, LSF, ref_coeff2, order, bw) == -1)
 	     {
 		Fprintf (stderr,
 		"anafea_distort: convert_to_rc: trouble converting from LSF to RC.\n");
 		exit (1);
 	     }

	     break;

	case AUTO:
	     if (reps_rc (spec_param2, AUTO, ref_coeff2, order, bw) == -1)
	     {
		Fprintf (stderr,
		"anafea_distort: convert_to_rc: trouble converting from AUTO to RC.\n");
		exit (1);
	     }
	     break;

	default:
	     Fprintf (stderr,
	     "anafea_distort: convert_to_rc: Unknown spectral representation, code: %d.\n",
	     *(short *) get_genhd ("spec_rep", f2h));
	     exit (1);
	     break;

    }  /* end switch (*(short *) get_genhd ("spec_rep", f2h)) */

     if (debug_level > 4) {
	for (i = 0; i <= order - 1; i++)
	    (void) fprintf (stderr,
	    "convert_to_rc: ref_coeff2[%d] = %f\n", i, ref_coeff2[i]);
	(void) fprintf (stderr, "\n");
     }

}
