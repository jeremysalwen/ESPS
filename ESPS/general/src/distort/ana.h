/*  ana.h - include file for ana_distort.c
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *  Module Name:  ana.h
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *
 *  Purpose:  Include file for ana_distort.c and anafea_dist.c
 *
 *
 * @(#)ana.h	3.1	11/3/87	ESI
 *
 */


/*
 *   G L O B A L    V A R I A B L E S 
 *
 *   used in ana_distort, anafea_distort, and print_distort.
 *
 */

extern char	**frame;	/* string for "Voiced" or "Unvoiced" frames */
extern int	*ref_c_len;	/* length of array ref_coeff for each record */
extern int	*pitch_len;	/* length of array p_pulse_len for each " */
extern int	*raw_len;	/* length of array raw_power for each record */
extern int	*lpc_len;	/* length of array lpc_power for each record */

extern float	**diff_ref_c;	/* difference between ref_coeff vector */
extern float	**diff_pitch;	/* difference between p_pulse_len vector */
extern float	**diff_raw;	/* difference between raw_power vector */
extern float	**diff_lpc;	/* difference between lpc_power vector */

extern double	*IS_dist;	/* Itakura-Saito (IS) distortion measure vector */
extern double	*GNIS_dist;	/* Gain-Normalized IS distortion measure vector */
extern double	*GOIS_dist;	/* Gain-Optimized IS distortion measure vector */

extern float	tot_ref_c;	/* variable to sum all ref_coeff */
extern float	tot_pitch;	/* variable to sum all p_pulse_len */
extern float	tot_raw;	/* variable to sum all raw_power */
extern float	tot_lpc;	/* variable to sum all lpc_power */

extern int	maxorder;
extern int	maxpulses;
extern int	maxraw;
extern int	maxlpc;

extern int	i_rec;		/* current record */
extern int	rec_num;	/* current record number */
extern int	ele_num;	/* current element number */

