/* include file for feature file subtype ANA (coherence records)

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."

  		@(#)anafea.h	1.7 2/20/96 EPI
*/

#ifndef anafea_H
#define anafea_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * defines for frame_type
 */
/*NONE is also a valid value (= 0)*/
#define UNKNOWN	    1
#define VOICED	    2
#define UNVOICED    3
#define SILENCE	    4
#define TRANSITION  5
extern char *frame_types[];

/*
 *defines for type of spectral representation
 */
/*NONE (= 0) is also a possible value*/
#define RC	1   /*reflection coefficients*/
#define LAR	2   /*log area fractions*/
#define LSF	3   /*line spectrum frequencies*/
#define AUTO	4   /*autocorrelations*/
#define AFC	5   /*autoregressive filter coefficients */
#define CEP     6   /*cepstrum coefficients */
#define AF	7   /*area functions */
extern char *spec_reps[];

/*
 * declarations for support functions
 */

struct anafea *
allo_anafea_rec ARGS((struct header *hd));

int
init_anafea_hd ARGS((struct header *hd, long int order_vcd,
		     long int order_unvcd, long int maxpulses,
		     long int maxraw, long int maxlpc,
		     int filt_nsiz, int filt_dsiz));
/* filt_nsiz and filt_dsiz are specified as short in pre-ANSI style
 * in anafeasupp.c; therefore they are passed as int.
 */

int
get_anafea_rec ARGS((struct anafea *ana_rec, struct header *hd, FILE *file));

void
put_anafea_rec ARGS((struct anafea *ana_rec, struct header *hd, FILE *file));


/*
 * structure definition for FEA_ANA records 
 */
struct anafea
{
    long	*tag;		/*position tag in reference file*/
    long	*frame_len;	/*number of samples in frame*/
    long	*num_pulses;	/*number of pulses in frame*/
    short	*frame_type;	/*classification type of speech frame*/
    float	*voiced_fraction; /*fraction of voiced to unvoiced energy*/
    float	*raw_power;	/*pulse powers (or unvoiced power)*/
    float	*lpc_power;	/*lpc error powers*/
    float	*p_pulse_len;	/*pitch pulse lengths*/
    float	*spec_param;	/*spectral parameters -- meaning is 
				 determined by header item spec_rep*/
    float	*filt_zeros;	/*filter numerator polynomial*/
    float	*filt_poles;	/*filter denominator polynomial*/
    struct fea_data *fea_rec;	/*pointer to corresponding FEA record*/

};

/* The items in the cohdata structures have the following shapes (dimension
 * information refers to generic header items):
 *
 *	Item		Size
 *
 *	tag		scalar
 *	frame_len	scalar
 *	num_pulses	scalar
 *	frame_type	scalar
 *	voiced_fraction	scalar
 *	raw_power	vector	(maxraw)
 *	lpc_power	vector	(maxlpc)
 *	p_pulse_len	vector	(maxpulses)
 *	spec_param	vector	(MAX(order_vcd, order_unvcd)
 *	filt_zeros	vector  (filt_nsiz)
 *	filt_poles	vector  (filt_dsiz)
 *
 * The pointer fea_rec does not correspond to actual data that is
 * part of the anafea record.  
 */

#ifdef __cplusplus
}
#endif

#endif /* anafea_H */
