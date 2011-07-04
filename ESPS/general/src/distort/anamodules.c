/* ana_modules.c - supporting functions for ana_distort and anafea_distort
 *
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Module Name:  compute_is_dist - compute various IS distortion measures
 *		 allo_memory - allocate memory for supporting arrays
 *		 array_len - get length of array.
 *
 * Written By:  Ajaipal S. Virdy
 *
 *
 * Purpose: computes Itakura-Saito (IS), Gain-normalized IS, and Gain-optimized
 *	    IS distortion measures.  Also allocates memory.
 *
 *
 */

#ifdef SCCS
	static char *sccs_id = "@(#)anamodules.c	3.5	7/8/96	ESI";
#endif

/*
 * System Includes
*/
#include <stdio.h>

/*
 * Include ESPS stuff
*/
#include <esps/unix.h>

#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char *calloc();
#endif

/*
 * G L O B A L
 *  V A R I A B L E S
 *   R E F E R E N C E D
 */

#include "ana.h"

extern char	*f1str, *f2str;
extern int	sflag;		/* Itakura-Saito distortion flag */
extern int	debug_level;

/*
 * U N I X
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

/* done via <esps/unix.h>*/


void
compute_is_dist (rc1, lpc_power1, rc2, lpc_power2, order)
float	*rc1;
float	lpc_power1;
float	*rc2;
float	lpc_power2;
int	order;		/* length of reflection coefficient array */
{

	float	*auto_corr;	/* auto-correlation coeficient array */
	float	*filt_coeff;	/* filter coefficient array for both files */
	float	*cdwd;		/* codeword array */

	float	ac_err;		/* Log LPC gain of auto-correlation coef. */
	float	cdwd_err;	/* Log LPC gain of codeword array */

	double	is_dist_td();
	double	gnis_dist_td();
	double	gois_dist_td();

	double	log();

	int	i;	/* temporary indices */
	int	n;

     if (debug_level > 2)
	(void) fprintf (stderr,
	"\ncompute_is_dist: order = %d\n", order);

/*
 * allocate memory for:  auto_corr, filt_coeff, and cdwd
 *
 */

     if ((auto_corr = (float *) calloc ((unsigned) (order + 1),
					sizeof(float))) == NULL) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for auto_corr.\n");
	exit (1);
     }

     if ((filt_coeff = (float *) calloc ((unsigned) (order + 1),
					 sizeof(float))) == NULL) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for filt_coeff.\n");
	exit (1);
     }

     if ((cdwd = (float *) calloc ((unsigned) (order + 1),
				   sizeof (float))) == NULL) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for cdwd.\n");
	exit (1);
     }

     if (debug_level > 4)
	(void) fprintf (stderr,
	"compute_is_dist: memory allocated, now do proper conversions.\n\n");

/*
 * Convert the reflection coefficients to auto-correlation coefficients:
 *
 */

     if (debug_level > 4) {
	for (i = 0; i <= order - 1; i++)
	    (void) fprintf (stderr,
	    "compute_is_dist: rc1[%d] = %f\n", i, rc1[i]);
	(void) fprintf (stderr, "\n");
     }

     (void) refl_to_auto (rc1 - 1, lpc_power1, auto_corr, order);

     if (debug_level > 4) {
	for (i = 1; i <= order + 1; i++)
	    (void) fprintf (stderr,
	    "compute_is_dist: auto_corr[%d] = %f\n", i, auto_corr[i]);
	(void) fprintf (stderr, "\n");
     }

/*
 * Convert reflection coefficients to filter coefficients:
 *
 */

     if (debug_level > 4) {
	for (i = 0; i <= order - 1; i++)
	    (void) fprintf (stderr,
	    "compute_is_dist: rc2[%d] = %f\n", i, rc2[i]);
	(void) fprintf (stderr, "\n");
     }

     (void) refl_to_filter (rc2 - 1, filt_coeff, order + 1);

     if (debug_level > 4) {
	for (i = 0; i <= order; i++)
	    (void) fprintf (stderr,
	    "compute_is_dist: filt_coeff[%d] = %f\n", i, filt_coeff[i]);
	(void) fprintf (stderr, "\n");
     }

/*
 * Compute CODEWORD
 *
 */

     for (n = 0; n <= order; n++) {
       cdwd[n] = 0.0;
       for ( i = 0; i <= order - n; i++ )
	   cdwd[n] += filt_coeff[i] * filt_coeff[i+n];
     }

     if (debug_level > 4) {
	for (i = 0; i <= order; i++)
	    (void) fprintf (stderr,
	    "compute_is_dist: cdwd[%d] = %f\n", i, cdwd[i]);
	(void) fprintf (stderr, "\n");
     }

/*
 * If lpc_power1 is zero, then we cannot compute Itakura-Saito distortion
 * measure for this record. Therefore, I'm assigning it a negative value, so
 * that pr_is_distort will display a STAR for this record.
 *
 */

     if (debug_level > 4) {
	(void) fprintf (stderr,
	"compute_is_dist: lpc_power1 = %f, lpc_power2 = %f\n",
	lpc_power1, lpc_power2);
	(void) fprintf (stderr, "\n");
     }

     if (lpc_power1 == 0) {
	(void) fprintf (stderr,
        "compute_is_dist: lpc_power[0] in record %d in %s is zero-\n",
	rec_num, f1str);
	(void) fprintf (stderr,
	"-cannot compute Itakura-Saito distortion measure for this record.\n");
	(void) fflush (stderr);

	IS_dist[i_rec] = -1.0;

	free ((char *) auto_corr);
	free ((char *) filt_coeff);
	free ((char *) cdwd);
	return;

     } else
	  ac_err = (float) log ((double) lpc_power1);

     if (lpc_power2 == 0) {
	(void) fprintf (stderr,
        "compute_is_dist: lpc_power[0] in record %d in %s is zero-\n",
	rec_num, f2str);
	(void) fprintf (stderr,
	"-cannot compute Itakura-Saito distortion measure for this record.\n");
	(void) fflush (stderr);

	IS_dist[i_rec] = -1.0;

	free ((char *) auto_corr);
	free ((char *) filt_coeff);
	free ((char *) cdwd);
	return;

     } else
	  cdwd_err = (float) log ((double) lpc_power2);

     if (debug_level > 3)
	(void) fprintf (stderr,
	"compute_is_dist: ac_err = %g, cdwd_err = %g\n",
	ac_err, cdwd_err);

     IS_dist[i_rec] = is_dist_td (auto_corr, ac_err,
				  cdwd - 1, cdwd_err, order + 1);

     GNIS_dist[i_rec] = gnis_dist_td (auto_corr, ac_err,
				      cdwd - 1, order + 1);

     GOIS_dist[i_rec] = gois_dist_td (auto_corr, ac_err,
				      cdwd - 1, order + 1);

     if (debug_level > 3) {
	(void) fprintf (stderr,
	"compute_is_dist: i_rec = %d, IS_dist = %g,\n",
	i_rec, IS_dist[i_rec]);
	(void) fprintf (stderr,
	"                 GNIS_dist = %g, GOIS_dist = %g\n",
	GNIS_dist[i_rec], GOIS_dist[i_rec]);
     }

/*
 * If the -s option was given, then compute the symmetric version of the
 * Itakura-Saito distortion measure.
 *
 */

     if (sflag) {

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "compute_is_dist: now computing symmetric IS distortions.\n");

	if (debug_level > 4)
	   for (i = 0; i <= order - 1; i++)
	       (void) fprintf (stderr,
	       "compute_is_dist: rc2[%d] = %f\n", i, rc2[i]);

/*
 * Convert the reflection coefficients to auto-correlation coefficients:
 *
 */

	(void) refl_to_auto (rc2 - 1, lpc_power2, auto_corr, order);

	if (debug_level > 4)
	   for (i = 1; i <= order + 1; i++)
	       (void) fprintf (stderr,
	       "compute_is_dist: auto_corr[%d] = %f\n", i, auto_corr[i]);

	if (debug_level > 4)
	   for (i = 0; i <= order - 1; i++)
	       (void) fprintf (stderr,
	       "compute_is_dist: rc1[%d] = %f\n", i, rc1[i]);

/*
 * Convert reflection coefficients to filter coefficients:
 *
 */

	(void)refl_to_filter (rc1 - 1, filt_coeff, order + 1);

	if (debug_level > 4)
	   for (i = 0; i <= order; i++)
	       (void) fprintf (stderr,
	       "compute_is_dist: filt_coeff[%d] = %f\n", i, filt_coeff[i]);

/*
 * Compute CODEWORD
 *
 */
	for ( n = 0; n <= order; n++ ) {
	    cdwd[n] = 0.0;
	    for ( i = 0; i <= order - n; i++ )
		cdwd[n] += filt_coeff[i] * filt_coeff[i+n];
	}

	if (debug_level > 4)
	   for (i = 0; i <= order; i++)
	       (void) fprintf (stderr,
	       "compute_is_dist: cdwd[%d] = %f\n", i, cdwd[i]);

	ac_err   = (float) log ((double) lpc_power2);

	cdwd_err = (float) log ((double) lpc_power1);

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "compute_is_dist: ac_err = %g, cdwd_err = %g\n",
	   ac_err, cdwd_err);

	IS_dist[i_rec] += is_dist_td (auto_corr, ac_err,
				      cdwd - 1, cdwd_err, order + 1);

	GNIS_dist[i_rec] += gnis_dist_td (auto_corr, ac_err,
					  cdwd - 1, order + 1);

	GOIS_dist[i_rec] += gois_dist_td (auto_corr, ac_err,
					  cdwd - 1, order + 1);

	IS_dist[i_rec] /= 2;
	GNIS_dist[i_rec] /= 2;
	GOIS_dist[i_rec] /= 2;

	if (debug_level > 3) {
	   (void) fprintf (stderr,
	   "compute_is_dist: i_rec = %d, IS_dist = %g,\n",
	   i_rec, IS_dist[i_rec]);
	   (void) fprintf (stderr,
	   "                 GNIS_dist = %g, GOIS_dist = %g\n",
	   GNIS_dist[i_rec], GOIS_dist[i_rec]);
	}

     }


     /*
      * The following definitely needs some explanation:
      *
      *  To prevent dividing by zero in the prnt_dist.c routines,
      *  I set IS_dist[i_rec] to -1.0 if lpc_power[0] = 0.
      *  Now, because of roundoff error, IS_dist[i_rec] might turn into
      *  a *very* small negative number.  It is highly unlikely that the
      *  number will be -1.0, so if IS_dist[i_rec] is negative, but NOT
      *  -1.0, then it must be zero.
      *
      */


     if ((IS_dist[i_rec] != -1.0) && (IS_dist[i_rec] <= 0.0)) {
	IS_dist[i_rec] = 0.0;
	GNIS_dist[i_rec] = 0.0;
	GOIS_dist[i_rec] = 0.0;
     }

     free ((char *) auto_corr);
     free ((char *) filt_coeff);
     free ((char *) cdwd);

}


void
allo_memory (rows, maxorder, maxpulses, maxraw, maxlpc)
int	rows;
int	maxorder;
int	maxpulses;
int	maxraw;
int	maxlpc;
{

/*
 * E S P S
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

   double	**d_mat_alloc();	/* memory allocator */

   if ( (diff_ref_c = (float **) d_mat_alloc (rows, maxorder)) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: d_mat_alloc: diff_ref_c: could not allocate memory.\n");
	exit (1);
   }

   if ( (diff_pitch = (float **) d_mat_alloc (rows, maxpulses)) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: d_mat_alloc: diff_pitch: could not allocate memory.\n");
	exit (1);
   }

   if ( (diff_raw = (float **) d_mat_alloc (rows, maxraw)) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: d_mat_alloc: diff_raw: could not allocate memory.\n");
	exit (1);
   }

   if ( (diff_lpc = (float **) d_mat_alloc (rows, maxlpc)) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: d_mat_alloc: diff_lpc: could not allocate memory.\n");
	exit (1);
   }

   if ( (frame = (char **) d_mat_alloc (rows, 9)) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: d_mat_alloc: frame: could not allocate memory.\n");
	exit (1);
   }

   if ( (IS_dist = (double *) calloc ((unsigned) rows, sizeof (double))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: IS_dist: could not allocate memory.\n");
	exit (1);
   }

   if ( (GNIS_dist = (double *) calloc ((unsigned) rows, sizeof (double))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: GNIS_dist: could not allocate memory.\n");
	exit (1);
   }

   if ( (GOIS_dist = (double *) calloc ((unsigned) rows, sizeof (double))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: GOIS_dist: could not allocate memory.\n");
	exit (1);
   }

   if ( (ref_c_len = (int *) calloc ((unsigned) rows, sizeof (int))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: ref_c_len: could not allocate memory.\n");
	exit (1);
   }

   if ( (pitch_len = (int *) calloc ((unsigned) rows, sizeof (int))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: pitch_len: could not allocate memory.\n");
	exit (1);
   }

   if ( (raw_len = (int *) calloc ((unsigned) rows, sizeof (int))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: raw_len: could not allocate memory.\n");
	exit (1);
   }

   if ( (lpc_len = (int *) calloc ((unsigned) rows, sizeof (int))) == NULL ) {
	(void) fprintf (stderr,
	"allo_memory: calloc: lpc_len: could not allocate memory.\n");
	exit (1);
   }

}


int
array_len ( array, maxlen, flag )	/* compute length of array */
float	array[];
int	maxlen;
short	flag;
{
   int length;

   if ( flag < 0 ) {
      for (length = 0; (array[length] >= 0) && (length < maxlen); length++)
	  ;
      return (length);
   }
   if ( flag > 0 ) {
      for (length = 0; (array[length] <= 0) && (length < maxlen); length++)
	  ;
      return (length);
   }

   /* flag must be equal to zero */

      for (length = 0; (array[length] != 0) && (length < maxlen); length++)
	  ;
      return (length);

}
