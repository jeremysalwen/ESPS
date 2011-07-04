 /* include file for generalized analysis FEA files (private to me_spec)

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing
  by Entropic Speech, Inc. must bear the notice			
 								
     "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."
 				

    @(#)genana.h	1.2	12/13/88	EPI
*/

/*
 * declarations for support functions
 */
struct genana	*allo_genana_rec();
int		get_genana_rec();

/*
 * structure definition for genana records
 */

struct genana
{
    long	*tag;		/* position tag in reference file */
    long	*frame_len;	/* number of samples in frame */
    long	*num_pulses;	/* number of pulses in frame */
    short	*frame_type;	/* classification type of speech frame */
    float	*raw_power;	/* pulse powers (or unvoiced power) */
    float	*p_pulse_len;	/* pitch pulse lengths */
    float	*spec_param;	/* spectral parameters  */
    struct fea_data *fea_rec;	/* pointer to corresponding FEA record */
};

/* The items in the genana structures have the following shapes
 *
 *	Item		Shape	Size
 *
 *	tag		scalar
 *	frame_len	scalar
 *	num_pulses	scalar
 *	frame_type	scalar
 *	raw_power	scalar or
 *			vector	(maxraw)
 *	p_pulse_len	vector	(maxpulses)
 *	spec_param	vector	(MAX(order_vcd, order_unvcd))
 *
 * The pointer fea_rec does not correspond to actual data that is
 * part of the genana record.  
 */
