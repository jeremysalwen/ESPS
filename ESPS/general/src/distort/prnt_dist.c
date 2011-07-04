/* prnt_dist.c - print output of distort in a nice table format
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 * 				
 *  Module Name:  prnt_dist
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *
 *  Purpose:  Print output in tabular form
 *
 *  
 */

#ifdef SCCS
	static char *sccs_id = "@(#)prnt_dist.c	3.4	1/27/97	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include "distort.h"
#include "ana.h"
#include "spec.h"

pr_table_hdr1()
{
	(void) fprintf ( stdout,
	"      \t\t       \t\t    \t\tDiff\t\tDiff\n" );

	(void) fprintf ( stdout,
	"Record\t\tElement\t\tDiff\t\tMag\t\tSquared\n" );

	(void) fprintf ( stdout,
	"------\t\t-------\t\t----\t\t---\t\t-------\n" );

}


pr_ana_ele()		/* Print each element difference (-E option) */
{

     int	n_rec;		/* total number of records */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */


/*
 *   	 External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *	 GLOBAL Variables Referenced:
 *
 *   char	**frame;	string for " Voiced " for "Unvoiced"
 *
 *   int	*ref_c_len;     length of array ref_coeff for each record
 *   int	*pitch_len;     length of array p_pulse_len for each record
 *   int	*raw_len;       length of array raw_power for each record
 *   int	*lpc_len;       length of array lpc_power for each record
 *
 *   float	**diff_ref_c;	difference between ref_coeff vector
 *   float	**diff_pitch;	difference between p_pulse_len vector
 *   float	**diff_raw;	difference between raw_power vector
 *   float	**diff_lpc;	difference between lpc_power vector
 *
 *
*/


     (void) pr_table_hdr1();	/* print header for first table */

     n_rec = (e_rec - s_rec + 1);

     for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

/*
 * Print out the first line of the output seperately
 * from the rest -- so that the record number and frame type (i.e.
 * voiced or unvoiced) only appears once on the top.
 *
*/
	(void) fprintf ( stdout,
	"%d (%s)\tref_coeff[%d]\t", rec_num, frame[i_rec], s_ele - 1);
	(void) fprintf ( stdout, (diff_ref_c[i_rec][0] > 0) ? " " : "" );
	(void) fprintf ( stdout,
	"%4.3e\t%4.3e\t%4.3e\n",
	diff_ref_c[i_rec][0], ABS (diff_ref_c[i_rec][0]),
	SQUARE (diff_ref_c[i_rec][0]) );

	for ( i_ele = s_ele, j = 1; j < ref_c_len[i_rec]; i_ele++, j++ ) {
	  (void) fprintf ( stdout,
	  "             \tref_coeff[%d]\t", i_ele);
	  (void) fprintf ( stdout, (diff_ref_c[i_rec][j] > 0) ? " " : "" );
	  (void) fprintf ( stdout,
	  "%4.3e\t%4.3e\t%4.3e\n",
	  diff_ref_c[i_rec][j], ABS (diff_ref_c[i_rec][j]),
	  SQUARE (diff_ref_c[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

	if ( strcmp(frame[i_rec], " Voiced ") == 0 ) {

	   for (i_ele = s_ele - 1, j = 0; j < pitch_len[i_rec]; i_ele++, j++) {
	     (void) fprintf ( stdout,
	     "             \tp_pulse_len[%d]\t", i_ele);
	     (void) fprintf ( stdout, (diff_pitch[i_rec][j] > 0) ? " " : "" );
	     (void) fprintf ( stdout,
	     "%4.3e\t%4.3e\t%4.3e\n",
	     diff_pitch[i_rec][j], ABS (diff_pitch[i_rec][j]),
	     SQUARE (diff_pitch[i_rec][j]) );
	   }

	   (void) fprintf (stdout, "\n");

	}

	for (i_ele = s_ele - 1, j = 0; j < raw_len[i_rec]; i_ele++, j++) {
	  (void) fprintf ( stdout,
	  "             \traw_power[%d]\t", i_ele);
	  (void) fprintf ( stdout, (diff_raw[i_rec][j] > 0) ? " " : "" );
	  (void) fprintf ( stdout,
	  "%4.3e\t%4.3e\t%4.3e\n",
	  diff_raw[i_rec][j], ABS (diff_raw[i_rec][j]),
	  SQUARE (diff_raw[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

	for (i_ele = s_ele - 1, j = 0; j < lpc_len[i_rec]; i_ele++, j++) {
	  (void) fprintf ( stdout,
	  "             \tlpc_power[%d]\t", i_ele);
	  (void) fprintf ( stdout, (diff_lpc[i_rec][j] > 0) ? " " : "" );
	  (void) fprintf ( stdout,
	  "%4.3e\t%4.3e\t%4.3e\n",
	  diff_lpc[i_rec][j], ABS (diff_lpc[i_rec][j]),
	  SQUARE (diff_lpc[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

      }  /* main for loop */

} /* end pr_ana_ele() */


pr_table_hdr2()
{

	(void) fprintf ( stdout,
	"      \t\t	        ELEMENT AVERAGE\n" );
	(void) fprintf ( stdout, "\n" );
	(void) fprintf ( stdout,
"       \t\tAVG            AVG           AVG          MAX         MAX\n" );

	(void) fprintf ( stdout,
"Element\t\tDiff           Mag           Squared      Mag         Squared\n" );

	(void) fprintf ( stdout,
"-------\t\t--------       -------       -------      -------     -------\n" );

}

#define PRNT_FIELDS(vec_name, vec, len_vec) { \
    for (i_ele = s_ele - 1, j = 0; i_ele < e_ele; i_ele++, j++) { \
 \
	diff_sum = 0.0; \
	mag_sum = 0.0; \
	sqrd_sum = 0.0; \
	count = 0; \
	max_mag = 0.0; \
	max_sqrd = 0.0; \
 \
	for ( i_rec = 0; i_rec < n_rec; i_rec++ ) \
	    if ( j < len_vec[i_rec] ) { \
	       diff_sum += vec[i_rec][j]; \
	       mag_sum += ABS (vec[i_rec][j]); \
	       sqrd_sum += SQUARE (vec[i_rec][j]); \
	       ++count; \
	       if (i_rec == 0) { \
		  max_mag = ABS (vec[i_rec][j]); \
		  max_sqrd = SQUARE (vec[i_rec][j]); \
		  max_rec = i_rec; \
	       } else { \
		  max_mag = \
			MAX (max_mag, ABS (vec[i_rec][j])); \
		  max_sqrd = \
			MAX (max_sqrd, SQUARE (vec[i_rec][j])); \
		  max_rec = i_rec; \
	       } \
	    } \
 \
	if (debug_level > 4) { \
	   (void) fprintf (stderr, \
	   "pr_ana_ele_avg: vec_name: diff_sum = %5.4e, mag_sum = %4.3e\n", \
	   diff_sum, mag_sum); \
	   (void) fprintf (stderr, \
	   "                           sqrd_sum = %g, count = %d\n", \
	   sqrd_sum, count); \
	} \
 \
	if (count != 0) { \
	   avg_diff = diff_sum / (float) count; \
	   avg_mag = mag_sum / (float) count; \
	   avg_sqrd = sqrd_sum / (float) count; \
	   (void) fprintf ( stdout, \
	   "vec_name[%d]\t", i_ele); \
	   (void) fprintf ( stdout, (avg_diff > 0) ? " " : "" ); \
	   (void) fprintf ( stdout, \
	   "%4.3e     %4.3e     %4.3e    %4.3e   %4.3e\n", \
	   avg_diff, avg_mag, avg_sqrd, \
	   max_mag, max_sqrd ); \
	} \
 \
    } \
 \
    (void) fprintf (stdout, "\n"); \
 \
} /* end PRNT_FIELDS */


pr_ana_ele_avg()	/*  print Element Average  */
{

     int	n_rec;		/* total number of records */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */

     float	diff_sum;	/* sum of differences across all records */
     float	mag_sum;	/* sum of magnitudes across all records */
     float	sqrd_sum;	/* sum of magnitude squared " " " */

     int	count;		/* number of records to average */

     float	avg_diff;	/* average difference across records */
     float	avg_mag;	/* average magnitudes   "       "     */
     float	avg_sqrd;	/* average magnitude squared "  "     */

     float	max_mag;	/* maximum magnitude across records */
     float	max_sqrd;	/* maximum magnitude squared across records */

     int	max_rec;	/* record number where maximum occurs */

/*
 *   	 External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *	 GLOBAL Variables Referenced:
 *
 *   int	*ref_c_len;     length of array ref_coeff for each record
 *   int	*pitch_len;     length of array p_pulse_len for each record
 *   int	*raw_len;       length of array raw_power for each record
 *   int	*lpc_len;       length of array lpc_power for each record
 *
 *   float	**diff_ref_c;	difference between ref_coeff vector
 *   float	**diff_pitch;	difference between p_pulse_len vector
 *   float	**diff_raw;	difference between raw_power vector
 *   float	**diff_lpc;	difference between lpc_power vector
 *
 *
*/


    (void) pr_table_hdr2();	/* print Element Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    PRNT_FIELDS(ref_coeff, diff_ref_c, ref_c_len);
    PRNT_FIELDS(p_pulse_len, diff_pitch, pitch_len);
    PRNT_FIELDS(raw_power, diff_raw, raw_len);
    PRNT_FIELDS(lpc_power, diff_lpc, lpc_len);

} /* end pr_ana_ele_avg() */

pr_table_hdr3()
{
	(void) fprintf ( stdout,
	"      \t\t	         RECORD AVERAGE\n" );
	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"Record\t\tElement\t\tAVG Diff\tAVG Mag\t\tAVG Squared\n" );

	(void) fprintf ( stdout,
	"------\t\t-------\t\t--------\t-------\t\t-----------\n" );

}

pr_ana_rec_avg()	/*  print Record Average  */
{
    char	*STAR = "    *      ";
    int		n_rec;
    int		i_rec;
    int		rec_num;
    int		i_ele;
    int		j;

    float	diff_sum;
    float	mag_sum;
    float	squared_sum;

    float	avg_diff;
    float	avg_mag;
    float	avg_squared;

    int		count;

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/

    (void) pr_table_hdr3();	/* print Record Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	/* Compute the Record Average for ref_coeff */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < ref_c_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_ref_c[i_rec][j];
	    mag_sum += ABS (diff_ref_c[i_rec][j]);
	    squared_sum += SQUARE (diff_ref_c[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_ana_rec_avg: ref_coeff: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "%d (%s)\tref_coeff\t%4.3e\t%4.3e\t%4.3e\n",
	   rec_num, frame[i_rec], avg_diff, avg_mag, avg_squared );
	}

	/* Now compute the Record Average for p_pulse_len */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < pitch_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_pitch[i_rec][j];
	    mag_sum += ABS (diff_pitch[i_rec][j]);
	    squared_sum += SQUARE (diff_pitch[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_ana_rec_avg: p_pulse_len: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "             \tp_pulse_len\t%4.3e\t%4.3e\t%4.3e\n",
	   avg_diff, avg_mag, avg_squared );
	}

	/* Now compute the Record Average for raw_power */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < raw_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_raw[i_rec][j];
	    mag_sum += ABS (diff_raw[i_rec][j]);
	    squared_sum += SQUARE (diff_raw[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_ana_rec_avg: raw_power: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "             \traw_power\t%4.3e\t%4.3e\t%4.3e\n",
	   avg_diff, avg_mag, avg_squared );
	} else
	   (void) fprintf ( stdout,
	   "             \traw_power\t%s\t%s\t%s\n",
	   STAR, STAR, STAR);

	/* Now compute the Record Average for lpc_power */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < lpc_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_lpc[i_rec][j];
	    mag_sum += ABS (diff_lpc[i_rec][j]);
	    squared_sum += SQUARE (diff_lpc[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_ana_rec_avg: lpc_power: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "             \tlpc_power\t%4.3e\t%4.3e\t%4.3e\n",
	   avg_diff, avg_mag, avg_squared );
	} else
	   (void) fprintf ( stdout,
	   "             \tlpc_power\t%s\t%s\t%s\n",
	   STAR, STAR, STAR);

	(void) fprintf (stdout, "\n");

    }	/* record loop */

} /* end pr_ana_rec_avg() */


pr_table_hdr4()		/* File Average Table Header */
{
	(void) fprintf ( stdout,
	"      \t\t	       TOTAL FILE AVERAGE\n" );
	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"      \tMEAN Diff\t\tMEAN Diff Mag\t\tMEAN Diff Squared\n" );

	(void) fprintf ( stdout,
	"      \t---------\t\t-------------\t\t-----------------\n" );

}


pr_ana_file_avg()		/* Print File Average */
{

    float	tot_diff = 0.0;	
    float	tot_mag = 0.0;
    float	tot_sqrd = 0.0;

    float	mean_diff;
    float	mean_mag;
    float	mean_sqrd;

    int		count = 0;

    int		n_rec;		/* total number of records */
    int		i_rec;
    int		i_ele;
    int		j;

    (void) pr_table_hdr4();	/* print File Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < ref_c_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_ref_c[i_rec][j];
	    tot_mag += ABS (diff_ref_c[i_rec][j]);
	    tot_sqrd += SQUARE (diff_ref_c[i_rec][j]);
	    ++count;
	}


    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < pitch_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_pitch[i_rec][j];
	    tot_mag += ABS (diff_pitch[i_rec][j]);
	    tot_sqrd += SQUARE (diff_pitch[i_rec][j]);
	    ++count;
	}


    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < raw_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_raw[i_rec][j];
	    tot_mag += ABS (diff_raw[i_rec][j]);
	    tot_sqrd += SQUARE (diff_raw[i_rec][j]);
	    ++count;
	}


    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < lpc_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_lpc[i_rec][j];
	    tot_mag += ABS (diff_lpc[i_rec][j]);
	    tot_sqrd += SQUARE (diff_lpc[i_rec][j]);
	    ++count;
	}

    if (debug_level > 4) {
	(void) fprintf (stderr,
	"pr_ana_file_avg: tot_diff = %4.3e, tot_mag = %4.3e, tot_sqrd = %4.3e\n",
	tot_diff, tot_mag, tot_sqrd);

	(void) fprintf (stderr,
	"pr_ana_file_avg: count = %d\n",
	count );
    }
	
    if (count != 0) {
	mean_diff = tot_diff / (float) count;
	mean_mag = tot_mag / (float) count;
	mean_sqrd = tot_sqrd / (float) count;
	(void) fprintf ( stdout,
	"      \t %4.3e \t\t %4.3e \t\t %4.3e\n",
	mean_diff, mean_mag, mean_sqrd );
    }

    (void) fprintf (stdout, "\n");

} /* end pr_ana_file_avg() */


pr_gen_ele ( diff )	/* Print Generic Elements */
double	**diff;
{

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */


/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/


     (void) pr_table_hdr1();  /* Print Table for -E option */

     n_rec = (e_rec - s_rec + 1);

     for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	(void) fprintf ( stdout,
	"  %d  ", rec_num);

	for (i_ele = s_ele, j = 0; i_ele <= e_ele; i_ele++, j++) {

	    if ( i_ele == s_ele)
	       (void) fprintf ( stdout, "    \t" );
	    else
	       (void) fprintf ( stdout, "             \t" );

	    (void) fprintf ( stdout,
	    "element%d\t%4.3e\t%4.3e\t%4.3e\n",
	    i_ele, diff[i_rec][j], ABS (diff[i_rec][j]),
	    SQUARE (diff[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

      }

}


pr_gen_ele_avg (diff)		/*  Print Generic Element Average  */
double	**diff;
{

     int	n_rec;		/* total number of records */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */

     float	diff_sum;	/* sum of differences across all records */
     float	mag_sum;	/* sum of magnitudes across all records */
     float	sqrd_sum;	/* sum of magnitude squared " " " */

     int	count;		/* number of records to average */

     float	avg_diff;	/* average difference across records */
     float	avg_mag;	/* average magnitudes   "       "     */
     float	avg_sqrd;	/* average magnitude squared "  "     */

     float	max_mag;	/* maximum magnitude across records */
     float	max_sqrd;	/* maximum magnitude squared across records */

     int	max_rec;	/* record position where maximum occurs */

/*
 *   	 External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/


    (void) pr_table_hdr2();	/* print Element Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for (i_ele = s_ele, j = 0; i_ele <= e_ele; i_ele++, j++) {

	diff_sum = 0.0;
	mag_sum = 0.0;
	sqrd_sum = 0.0;
	count = 0;
	max_mag = 0.0;
	max_sqrd = 0.0;

	for ( i_rec = 0; i_rec < n_rec; i_rec++ ) {
	    diff_sum += diff[i_rec][j];
	    mag_sum += ABS (diff[i_rec][j]);
	    sqrd_sum += SQUARE (diff[i_rec][j]);
	    ++count;
	    if (i_rec == 0) {
		max_mag = ABS (diff[i_rec][j]);
		max_sqrd = SQUARE (diff[i_rec][j]);
		max_rec = i_rec;
	    } else {
		max_mag = MAX (max_mag, ABS (diff[i_rec][j]));
		max_sqrd = MAX (max_sqrd, SQUARE (diff[i_rec][j]));
		max_rec = i_rec;
	    }
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_ana_ele_avg: elements: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                          sqrd_sum = %e,    count = %d\n",
	   sqrd_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_sqrd = sqrd_sum / (float) count;
	   (void) fprintf ( stdout,
	   "element%d\t", i_ele);
	   (void) fprintf ( stdout, (avg_diff > 0) ? " " : "" );
	   (void) fprintf ( stdout,
	   "%4.3e     %4.3e     %4.3e    %4.3e   %4.3e\n",
	   avg_diff, avg_mag, avg_sqrd,
	   max_mag, max_sqrd );
	}

    }

    (void) fprintf (stdout, "\n");

}


pr_gen_rec_avg (diff) 	/*  print Record Average  */
double	**diff;
{

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */


     float	diff_sum;	/* sum of differences across all elements */
     float	mag_sum;	/* sum of magnitudes across all elements */
     float	sqrd_sum;	/* sum of magnitude squared " " " */

     int	count;		/* number of elements to average */

     float	avg_diff;	/* average difference across elements */
     float	avg_mag;	/* average magnitudes   "       "     */
     float	avg_sqrd;	/* average magnitude squared "  "     */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/

    (void) pr_table_hdr3();	/* Print Record Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	diff_sum = 0.0;
	mag_sum = 0.0;
	sqrd_sum = 0.0;
	count = 0;

	for (i_ele = s_ele, j = 0; i_ele <= e_ele; i_ele++, j++ ) {
	    diff_sum += diff[i_rec][j];
	    mag_sum += ABS (diff[i_rec][j]);
	    sqrd_sum += SQUARE (diff[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_gen_rec_avg: elements: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    sqrd_sum = %4.3e,   count = %d\n",
	   sqrd_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_sqrd = sqrd_sum / (float) count;

	   (void) fprintf ( stdout,
	   "  %d           \t    *     \t%4.3e\t%4.3e\t%4.3e\n",
	   rec_num, avg_diff, avg_mag, avg_sqrd );
	}

    }	/* record loop */

    (void) fprintf (stdout, "\n");

}


pr_gen_file_avg (diff)		/* Print File Average */
double	**diff;
{

     int	n_rec;		/* total number of records */
     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */

     int	count;		/* number of records to average */

     float	mean_diff;	/* mean difference across records */
     float	mean_mag;	/* mean magnitude across records */
     float	mean_sqrd;	/* mean magnitude squared across records */


     float	tot_diff;	/* difference across ALL records & elements */
     float	tot_mag;	/* magnitude of difference across " " " "   */
     float	tot_sqrd;	/* magnitude squared difference across " "  */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/

    (void) pr_table_hdr4();	/* print File Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    count = 0;
    tot_diff = 0.0;
    tot_mag = 0.0;
    tot_sqrd = 0.0;

    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele, j = 0; i_ele <= e_ele; i_ele++, j++) {
	    tot_diff += diff[i_rec][j];
	    tot_mag += ABS (diff[i_rec][j]);
	    tot_sqrd += SQUARE (diff[i_rec][j]);
	    ++count;
	}


    if (debug_level > 4) {
	(void) fprintf (stderr,
	"pr_gen_file_avg: tot_diff = %4.3e, tot_mag = %4.3e, tot_sqrd = %4.3e\n",
	tot_diff, tot_mag, tot_sqrd);

	(void) fprintf (stderr,
	"pr_gen_file_avg: count = %d\n",
	count );
    }
	
    if (count != 0) {
	mean_diff = tot_diff / (float) count;
	mean_mag = tot_mag / (float) count;
	mean_sqrd = tot_sqrd / (float) count;
	(void) fprintf ( stdout,
	"      \t %4.3e \t\t %4.3e \t\t %4.3e\n",
	mean_diff, mean_mag, mean_sqrd );
    }

    (void) fprintf (stdout, "\n");

}


pr_is_dist()		/* Print Spectral Distortions */
{

     char	*STAR = "  *    ";

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *   char	**frame;	frame type (Voiced or Unvoiced)
 *
*/


	if (sflag)
	   (void) fprintf ( stdout,
	   "      \t\t	SPECTRAL DISTORTIONS - SYMMETRIC\n" );
	else
	   (void) fprintf ( stdout,
	   "      \t\t	    SPECTRAL DISTORTIONS\n" );

	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"      Record\t\t IS \t\t\tGNIS\t\t\tGOIS\n" );

	(void) fprintf ( stdout,
	"      ------\t\t----\t\t\t----\t\t\t----\n" );


    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	if (IS_dist[i_rec] == -1.0)
	   (void) fprintf ( stdout,
	   "      %d (%s)\t%s\t\t\t%s\t\t\t%s\n",
	   rec_num, frame[i_rec], STAR, STAR, STAR);
	else
	   (void) fprintf ( stdout,
	   "      %d (%s)\t%6.4f\t\t\t%6.4f\t\t\t%6.4f\n",
	   rec_num, frame[i_rec], IS_dist[i_rec], GNIS_dist[i_rec],
	   GOIS_dist[i_rec]);

    }
} /* end pr_is_dist() */


pr_tot_is_dist()		/* Print TOTAL Spectral Distortions */
{

     char	*STAR = "  *    ";

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *   char	**frame;	frame type (Voiced or Unvoiced)
 *
*/

/*  Local Variables  */

     double	U_is = 0;
     double	U_gnis = 0;
     double	U_gois = 0;
     int	unvcd = 0;

     double	V_is = 0;
     double	V_gnis = 0;
     double	V_gois = 0;
     int	vcd = 0;

     double	ALL_is = 0;
     double	ALL_gnis = 0;
     double	ALL_gois = 0;
     int	tot_rec = 0;


	(void) fprintf ( stdout, "\n" );
	(void) fprintf ( stdout,
	"      \t\t	   TOTAL SPECTRAL DISTORTIONS\n" );

	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"      Frame \t\t IS \t\t\tGNIS\t\t\tGOIS\n" );

	(void) fprintf ( stdout,
	"      ------\t\t----\t\t\t----\t\t\t----\n" );


    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	if (IS_dist[i_rec] != -1.0) {

	   /*
	    * If IS_dist[i_rec] = -1.0 that means lpc_power[0] was zero,
	    * therefore we cannot include that record in our computations.
	    */

	   if (strcmp(frame[i_rec], "Unvoiced") == 0) {

	      U_is   += IS_dist[i_rec];
	      U_gnis += GNIS_dist[i_rec];
	      U_gois += GOIS_dist[i_rec];
	      ++unvcd;

	   } else {	/* "Voiced" Frame */

	      V_is   += IS_dist[i_rec];
	      V_gnis += GNIS_dist[i_rec];
	      V_gois += GOIS_dist[i_rec];
	      ++vcd;
	   }

	   ALL_is   += IS_dist[i_rec];
	   ALL_gnis += GNIS_dist[i_rec];
	   ALL_gois += GOIS_dist[i_rec];

	   ++tot_rec;
	}
    }

    if (unvcd != 0) {
	U_is /= unvcd;
	U_gnis /= unvcd;
	U_gois /= unvcd;
    }
    if (vcd != 0) {
	V_is /= vcd;
	V_gnis /= vcd;
	V_gois /= vcd;
    }

    if (tot_rec != 0) {
        ALL_is /= tot_rec;
	ALL_gnis /= tot_rec;
	ALL_gois /= tot_rec;
    }

    if (unvcd == 0)
       (void) fprintf ( stdout,
       "      %s\t\t%s\t\t\t%s\t\t\t%s\n",
       "Unvoiced", STAR, STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      %s\t\t%6.4f\t\t\t%6.4f\t\t\t%6.4f\n",
       "Unvoiced", U_is, U_gnis, U_gois);

    if (vcd == 0)
       (void) fprintf ( stdout,
       "      %s\t\t%s\t\t\t%s\t\t\t%s\n",
       " Voiced ", STAR, STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      %s\t\t%6.4f\t\t\t%6.4f\t\t\t%6.4f\n",
       " Voiced ", V_is, V_gnis, V_gois);

    if (tot_rec == 0)
       (void) fprintf ( stdout,
       "      %s\t\t%s\t\t\t%s\t\t\t%s\n",
       "  ALL   ", STAR, STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      %s\t\t%6.4f\t\t\t%6.4f\t\t\t%6.4f\n",
       "  ALL   ", ALL_is, ALL_gnis, ALL_gois);

} /* end pr_tot_is_dist() */


pr_spec_ele()		/* Print each element difference (-E option) */
{

     int	n_rec;		/* total number of records */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */


/*
 *   	 External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *	 GLOBAL Variables Referenced:
 *
 *   char	**frame;	string for " Voiced " for "Unvoiced"
 *
 *   int	*freq_len;	length of array re_spec_val, im_spec_val,
 *				and frqs_len for each record
 *
 *   float	**diff_real;	difference between re_spec_val vector
 *   float	**diff_imag;	difference between im_spec_val vector
 *   float	**diff_frqs;	difference between frqs vector
 *
 *
*/


     (void) pr_table_hdr1();	/* print header for first table */

     n_rec = (e_rec - s_rec + 1);

     for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

/*
 * Print out the first line of the output seperately
 * from the rest -- so that the record number and frame type (i.e.
 * voiced or unvoiced) only appears once on the top.
 *
*/
	(void) fprintf ( stdout,
	"%d (%s)\tre_spec_val[%d]\t", rec_num, frame[i_rec], s_ele - 1);
	(void) fprintf ( stdout, (diff_real[i_rec][0] > 0) ? " " : "" );
	(void) fprintf ( stdout,
	"%4.3e\t%4.3e\t%4.3e\n",
	diff_real[i_rec][0], ABS (diff_real[i_rec][0]),
	SQUARE (diff_real[i_rec][0]) );

	for (i_ele = s_ele, j = 1; j < freq_len[i_rec]; i_ele++, j++) {
	  (void) fprintf ( stdout,
	  "             \tre_spec_val[%d]\t", i_ele);
	  (void) fprintf ( stdout, (diff_real[i_rec][j] > 0) ? " " : "" );
	  (void) fprintf ( stdout,
	  "%4.3e\t%4.3e\t%4.3e\n",
	  diff_real[i_rec][j], ABS (diff_real[i_rec][j]),
	  SQUARE (diff_real[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

	if ( strcmp(frame[i_rec], " Voiced ") == 0 ) {

	   for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	     (void) fprintf ( stdout,
	     "             \tim_spec_val[%d]\t", i_ele);
	     (void) fprintf ( stdout, (diff_imag[i_rec][j] > 0) ? " " : "" );
	     (void) fprintf ( stdout,
	     "%4.3e\t%4.3e\t%4.3e\n",
	     diff_imag[i_rec][j], ABS (diff_imag[i_rec][j]),
	     SQUARE (diff_imag[i_rec][j]) );
	   }

	   (void) fprintf (stdout, "\n");

	}

	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	  (void) fprintf ( stdout,
	  "             \tfrqs[%d]\t", i_ele);
	  (void) fprintf ( stdout, (diff_frqs[i_rec][j] > 0) ? " " : "" );
	  (void) fprintf ( stdout,
	  "%4.3e\t%4.3e\t%4.3e\n",
	  diff_frqs[i_rec][j], ABS (diff_frqs[i_rec][j]),
	  SQUARE (diff_frqs[i_rec][j]) );
	}

	(void) fprintf (stdout, "\n");

      }  /* main for loop */

} /* end pr_spec_ele() */


pr_spec_ele_avg()	/*  print Element Average  */
{

     int	n_rec;		/* total number of records */

     int	i_rec;		/* record index */
     int	i_ele;		/* element index */
     int	j;		/* temporary element index */

     float	diff_sum;	/* sum of differences across all records */
     float	mag_sum;	/* sum of magnitudes across all records */
     float	sqrd_sum;	/* sum of magnitude squared " " " */

     int	count;		/* number of records to average */

     float	avg_diff;	/* average difference across records */
     float	avg_mag;	/* average magnitudes   "       "     */
     float	avg_sqrd;	/* average magnitude squared "  "     */

     float	max_mag;	/* maximum magnitude across records */
     float	max_sqrd;	/* maximum magnitude squared across records */

     int	max_rec;	/* record number where maximum occurs */

/*
 *   	 External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
 *	 GLOBAL Variables Referenced:
 *
 *   int	*freq_len;     length of array  for each record
 *
 *   float	**diff_real;	difference between re_spec_val vector
 *   float	**diff_imag;	difference between im_spec_val vector
 *   float	**diff_frqs;	difference between frqs vector
 *
 *
*/


    (void) pr_table_hdr2();	/* print Element Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for (i_ele = s_ele - 1, j = 0; i_ele < e_ele; i_ele++, j++) {

	diff_sum = 0.0;
	mag_sum = 0.0;
	sqrd_sum = 0.0;
	count = 0;
	max_mag = 0.0;
	max_sqrd = 0.0;

	for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	    if ( j < freq_len[i_rec] ) {
	       diff_sum += diff_real[i_rec][j];
	       mag_sum += ABS (diff_real[i_rec][j]);
	       sqrd_sum += SQUARE (diff_real[i_rec][j]);
	       ++count;
	       if (i_rec == 0) {
		  max_mag = ABS (diff_real[i_rec][j]);
		  max_sqrd = SQUARE (diff_real[i_rec][j]);
		  max_rec = i_rec;
	       } else {
		  max_mag =
			MAX (max_mag, ABS (diff_real[i_rec][j]));
		  max_sqrd =
			MAX (max_sqrd, SQUARE (diff_real[i_rec][j]));
		  max_rec = i_rec;
	       }
	    }

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_ele_avg: re_spec_val: diff_sum = %5.4e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                           sqrd_sum = %g, count = %d\n",
	   sqrd_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_sqrd = sqrd_sum / (float) count;
	   (void) fprintf ( stdout,
	   "re_spec_val[%d]\t", i_ele);
	   (void) fprintf ( stdout, (avg_diff > 0) ? " " : "" );
	   (void) fprintf ( stdout,
	   "%4.3e     %4.3e     %4.3e    %4.3e   %4.3e\n",
	   avg_diff, avg_mag, avg_sqrd,
	   max_mag, max_sqrd );
	}

    }

    (void) fprintf (stdout, "\n");

    for (i_ele = s_ele - 1, j = 0; i_ele < e_ele; i_ele++, j++) {

	diff_sum = 0.0;
	mag_sum = 0.0;
	sqrd_sum = 0.0;
	count = 0;
	max_mag = 0.0;
	max_sqrd = 0.0;

	for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	    if ( j < freq_len[i_rec] ) {
	       diff_sum += diff_imag[i_rec][j];
	       mag_sum += ABS (diff_imag[i_rec][j]);
	       sqrd_sum += SQUARE (diff_imag[i_rec][j]);
	       ++count;
	       if (i_rec == 0) {
		  max_mag = ABS (diff_imag[i_rec][j]);
		  max_sqrd = SQUARE (diff_imag[i_rec][j]);
		  max_rec = i_rec;
	       } else {
		  max_mag =
			MAX (max_mag, ABS (diff_imag[i_rec][j]));
		  max_sqrd =
			MAX (max_sqrd, SQUARE (diff_imag[i_rec][j]));
		  max_rec = i_rec;
	       }
	    }

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_ele_avg: im_spec_val: diff_sum = %5.4e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                           sqrd_sum = %g, count = %d\n",
	   sqrd_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_sqrd = sqrd_sum / (float) count;
	   (void) fprintf ( stdout,
	   "im_spec_val[%d]\t", i_ele);
	   (void) fprintf ( stdout, (avg_diff > 0) ? " " : "" );
	   (void) fprintf ( stdout,
	   "%4.3e     %4.3e     %4.3e    %4.3e   %4.3e\n",
	   avg_diff, avg_mag, avg_sqrd,
	   max_mag, max_sqrd );
	}

    }

    (void) fprintf (stdout, "\n");

    for (i_ele = s_ele - 1, j = 0; i_ele < e_ele; i_ele++, j++) {

	diff_sum = 0.0;
	mag_sum = 0.0;
	sqrd_sum = 0.0;
	count = 0;
	max_mag = 0.0;
	max_sqrd = 0.0;

	for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	    if ( j < freq_len[i_rec] ) {
	       diff_sum += diff_frqs[i_rec][j];
	       mag_sum += ABS (diff_frqs[i_rec][j]);
	       sqrd_sum += SQUARE (diff_frqs[i_rec][j]);
	       ++count;
	       if (i_rec == 0) {
		  max_mag = ABS (diff_frqs[i_rec][j]);
		  max_sqrd = SQUARE (diff_frqs[i_rec][j]);
		  max_rec = i_rec;
	       } else {
		  max_mag =
			MAX (max_mag, ABS (diff_frqs[i_rec][j]));
		  max_sqrd =
			MAX (max_sqrd, SQUARE (diff_frqs[i_rec][j]));
		  max_rec = i_rec;
	       }
	    }

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_ele_avg: frqs: diff_sum = %5.4e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                           sqrd_sum = %g, count = %d\n",
	   sqrd_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_sqrd = sqrd_sum / (float) count;
	   (void) fprintf ( stdout,
	   "frqs[%d]\t", i_ele);
	   (void) fprintf ( stdout, (avg_diff > 0) ? " " : "" );
	   (void) fprintf ( stdout,
	   "%4.3e     %4.3e     %4.3e    %4.3e   %4.3e\n",
	   avg_diff, avg_mag, avg_sqrd,
	   max_mag, max_sqrd );
	}

    }

    (void) fprintf (stdout, "\n");

} /* end pr_spec_ele_avg() */


pr_spec_file_avg()		/* Print File Average */
{

    float	tot_diff = 0.0;	
    float	tot_mag = 0.0;
    float	tot_sqrd = 0.0;

    float	mean_diff;
    float	mean_mag;
    float	mean_sqrd;

    int		count = 0;

    int		n_rec;		/* total number of records */
    int		i_rec;
    int		i_ele;
    int		j;

    (void) pr_table_hdr4();	/* print File Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_real[i_rec][j];
	    tot_mag += ABS (diff_real[i_rec][j]);
	    tot_sqrd += SQUARE (diff_real[i_rec][j]);
	    ++count;
	}


    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_imag[i_rec][j];
	    tot_mag += ABS (diff_imag[i_rec][j]);
	    tot_sqrd += SQUARE (diff_imag[i_rec][j]);
	    ++count;
	}


    for ( i_rec = 0; i_rec < n_rec; i_rec++ )
	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    tot_diff += diff_frqs[i_rec][j];
	    tot_mag += ABS (diff_frqs[i_rec][j]);
	    tot_sqrd += SQUARE (diff_frqs[i_rec][j]);
	    ++count;
	}


    if (debug_level > 4) {
	(void) fprintf (stderr,
	"pr_spec_file_avg: tot_diff = %4.3e, tot_mag = %4.3e, tot_sqrd = %4.3e\n",
	tot_diff, tot_mag, tot_sqrd);

	(void) fprintf (stderr,
	"pr_spec_file_avg: count = %d\n",
	count );
    }
	
    if (count != 0) {
	mean_diff = tot_diff / (float) count;
	mean_mag = tot_mag / (float) count;
	mean_sqrd = tot_sqrd / (float) count;
	(void) fprintf ( stdout,
	"      \t %4.3e \t\t %4.3e \t\t %4.3e\n",
	mean_diff, mean_mag, mean_sqrd );
    }

    (void) fprintf (stdout, "\n");

} /* end pr_spec_file_avg() */


pr_spec_rec_avg()	/*  print Record Average  */
{
    char	*STAR = "    *      ";
    int		n_rec;
    int		i_rec;
    int		rec_num;
    int		i_ele;
    int		j;

    float	diff_sum;
    float	mag_sum;
    float	squared_sum;

    float	avg_diff;
    float	avg_mag;
    float	avg_squared;

    int		count;

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *   int	s_ele, e_ele;   start element, end element
 *
*/

    (void) pr_table_hdr3();	/* print Record Average Table Header */

    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	/* Compute the Record Average for ref_coeff */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_real[i_rec][j];
	    mag_sum += ABS (diff_real[i_rec][j]);
	    squared_sum += SQUARE (diff_real[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_rec_avg: re_spec_val: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "%d (%s)\tre_spec_val\t%4.3e\t%4.3e\t%4.3e\n",
	   rec_num, frame[i_rec], avg_diff, avg_mag, avg_squared );
	}

	/* Now compute the Record Average for im_spec_val */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_imag[i_rec][j];
	    mag_sum += ABS (diff_imag[i_rec][j]);
	    squared_sum += SQUARE (diff_imag[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_rec_avg: im_spec_val: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "             \tim_spec_val\t%4.3e\t%4.3e\t%4.3e\n",
	   avg_diff, avg_mag, avg_squared );
	}

	/* Now compute the Record Average for frqs */

	diff_sum = 0.0;
	mag_sum = 0.0;
	squared_sum = 0.0;
	count = 0;

	for (i_ele = s_ele - 1, j = 0; j < freq_len[i_rec]; i_ele++, j++) {
	    diff_sum += diff_frqs[i_rec][j];
	    mag_sum += ABS (diff_frqs[i_rec][j]);
	    squared_sum += SQUARE (diff_frqs[i_rec][j]);
	    ++count;
	}

	if (debug_level > 4) {
	   (void) fprintf (stderr,
	   "pr_spec_rec_avg: frqs: diff_sum = %4.3e, mag_sum = %4.3e\n",
	   diff_sum, mag_sum);
	   (void) fprintf (stderr,
	   "                    squared_sum = %4.3e,   count = %d\n",
	   squared_sum, count);
	}
	
	if (count != 0) {
	   avg_diff = diff_sum / (float) count;
	   avg_mag = mag_sum / (float) count;
	   avg_squared = squared_sum / (float) count;

	   (void) fprintf ( stdout,
	   "             \tfrqs\t%4.3e\t%4.3e\t%4.3e\n",
	   avg_diff, avg_mag, avg_squared );
	} else
	   (void) fprintf ( stdout,
	   "             \tfrqs\t%s\t%s\t%s\n",
	   STAR, STAR, STAR);

	(void) fprintf (stdout, "\n");

    }	/* record loop */

} /* end pr_spec_rec_avg() */


pr_spec_is_dist()		/* Print Spectral Distortions */
{

     char	*STAR = "  *    ";

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *
 *   char	**frame;	frame type (Voiced or Unvoiced)
 *
*/

	if (sflag)
	   (void) fprintf ( stdout,
	   "      \t\t	SPECTRAL DISTORTIONS - SYMMETRIC\n" );
	else
	   (void) fprintf ( stdout,
	   "      \t\t	    SPECTRAL DISTORTIONS\n" );

	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"      \tRecord\t\t\t IS \t\t\tPNIS\n" );

	(void) fprintf ( stdout,
	"      \t------\t\t\t----\t\t\t----\n" );


    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	if (IS_dist[i_rec] == -1.0) 
	   (void) fprintf ( stdout,
	   "      \t%d (%s)\t\t%s\t\t%s\n",
	   rec_num, frame[i_rec], STAR, STAR);
	else
	   (void) fprintf ( stdout,
	   "      \t%d (%s)\t\t%6.4f\t\t%6.4f\n",
	   rec_num, frame[i_rec], IS_dist[i_rec], PNIS_dist[i_rec]);

    }

} /* end pr_spec_is_dist() */


pr_spec_tot_is_dist()		/* Print TOTAL Spectral Distortions */
{

     char	*STAR = "  *    ";

     int	n_rec;		/* total number of records */
     int	rec_num;	/* current record number */

     int	i_rec;		/* record index */

/*
 *   	  External Variables Referenced: 
 *
 *   int	s_rec, e_rec;	start record, end record
 *
 *   char	**frame;	frame type (Voiced or Unvoiced)
 *
*/

/*  Local Variables  */

     double	U_is = 0;
     double	U_pnis = 0;

     int	unvcd = 0;

     double	V_is = 0;
     double	V_pnis = 0;

     int	vcd = 0;

     double	ALL_is = 0;
     double	ALL_pnis = 0;

     int	tot_rec = 0;


	(void) fprintf ( stdout, "\n" );
	(void) fprintf ( stdout,
	"      \t\t	   TOTAL SPECTRAL DISTORTIONS\n" );

	(void) fprintf ( stdout, "\n" );

	(void) fprintf ( stdout,
	"      \tFrame \t\t\t IS \t\t\tPNIS\n" );

	(void) fprintf ( stdout,
	"      \t------\t\t\t----\t\t\t----\n" );


    n_rec = (e_rec - s_rec + 1);

    for ( i_rec = 0, rec_num = s_rec; i_rec < n_rec; i_rec++, rec_num++ ) {

	if (IS_dist[i_rec] != -1.0) {

	   if (strcmp(frame[i_rec], "Unvoiced") == 0) {

	      U_is   += IS_dist[i_rec];
	      U_pnis += PNIS_dist[i_rec];
	      ++unvcd;

	   } else {	/* "Voiced" Frame */

	      V_is   += IS_dist[i_rec];
	      V_pnis += PNIS_dist[i_rec];
	      ++vcd;
	   }

	   ALL_is   += IS_dist[i_rec];
	   ALL_pnis += PNIS_dist[i_rec];

	   ++tot_rec;
	}
    }

    if (unvcd != 0) {
	U_is /= unvcd;
	U_pnis /= unvcd;
    }
    if (vcd != 0) {
	V_is /= vcd;
	V_pnis /= vcd;
    }

    ALL_is /= tot_rec;
    ALL_pnis /= tot_rec;


    if (unvcd == 0)
       (void) fprintf ( stdout,
       "      \t%s\t\t%s\t\t\t%s\n",
       "Unvoiced", STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      \t%s\t\t%6.4f\t\t%6.4f\n",
       "Unvoiced", U_is, U_pnis);

    if (vcd == 0)
       (void) fprintf ( stdout,
       "      \t%s\t\t%s\t\t\t%s\n",
       " Voiced ", STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      \t%s\t\t%6.4f\t\t%6.4f\n",
       " Voiced ", V_is, V_pnis);

    if (tot_rec == 0)
       (void) fprintf ( stdout,
       "      \t%s\t\t%s\t\t\t%s\n",
       "  ALL   ", STAR, STAR);
    else
       (void) fprintf ( stdout,
       "      \t%s\t\t%6.4f\t\t%6.4f\n",
       "  ALL   ", ALL_is, ALL_pnis);

} /* end pr_spec_tot_is_dist() */
