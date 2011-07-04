/*  spec.h - include file for spec_distort.c
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 * 				
 *  Module Name:  spec.h
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *
 *  Purpose:  Include file for spec_distort.c
 *
 *
 * @(#)spec.h	3.1	11/3/87	ESI
 *
 */

/*
 *   G L O B A L    V A R I A B L E S 
 *
 *   used in spec_distort and print_distort.
 *
 */

extern char	**frame;	/* string for "Voiced" or "Unvoiced" frames */
extern int	*freq_len;	/* length of array  for each record */

extern float	**diff_real;	/* difference between re_spec_val vector */
extern float	**diff_imag;	/* difference between im_spec_val vector */
extern float	**diff_frqs;	/* difference between frqs vector */

extern double	*IS_dist;	/* Itakura-Saito (IS) distortion measure vector */
extern double	*PNIS_dist;	/* Power-Normalized IS distortion measure vector */

extern float	tot_real;	/* variable to sum all re_spec_val */
extern float	tot_imag;	/* variable to sum all im_spec_val */
extern float	tot_frqs;	/* variable to sum all frqs_power */

extern int	i_rec;		/* current record */
extern int	rec_num;	/* current record number */
extern int	ele_num;	/* current element number */

