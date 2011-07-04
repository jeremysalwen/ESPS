/*
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing
 *  by Entropic Speech, Inc. must bear the notice
 *
 *  "Copyright (c) 1987, 1988 Entropic Speech, Inc. All rights reserved."
 *
 * 				
 *  Module Name:  non_stat.h
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *  Checked By:
 *
 *  Purpose:  Include file for non_stat.c
 *
 *  Secrets:  None
 *  
 *  @(#)non_stat.h	1.3	7/20/88	ESI
 */

/*
 *    G L O B A L   V A R I A B L E S 
 *
 *		U S E D   B Y
 *
 *	     N O N _ S T A T . C
 *
 */


extern
struct	d_struct {
	double	min;
	int	min_at;
	double	max;
	int	max_at;
	double	stndev;
	double	mean;
} *d_info;

extern 
struct f_struct {
	float	min;
	int	min_at;
	float	max;
	int	max_at;
	double	stndev;
	double	mean;
} *f_info;

extern
struct l_struct {
	long	min;
	int	min_at;
	long	max;
	int	max_at;
	double	stndev;
	double	mean;
} *l_info;

extern
struct s_struct {
	short	min;
	int	min_at;
	short	max;
	int	max_at;
	double	stndev;
	double	mean;
} *s_info;

extern
struct b_struct {
	char	min;
	int	min_at;
	char	max;
	int	max_at;
	double	stndev;
	double	mean;
} *b_info;
