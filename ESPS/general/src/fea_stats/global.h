/*
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 *  Module Name:  global.h
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *  Checked By:
 *
 *  Purpose:  Include file for fea_stats.c
 *
 *  Secrets:  None
 *  
 *  @(#)global.h	1.4	6/18/98	ESI
 */

/*
 *    G L O B A L   V A R I A B L E S 
 *
 *		U S E D   B Y
 *
 *	     F E A _ S T A T S . C
 *
 */

extern	char	*Version;
extern	char	*Date;

extern	char	*ProgName;
extern	int	debug_level;

extern	long	n_rec;
extern	long	s_rec;
extern	long	e_rec;

extern	struct
	header	*esps_hdr;

extern	struct
	header	*fea_oh;

extern	struct
	fea_header	*fea_hdr;

extern	struct
	fea_data	*fea_rec;

extern	FILE	*instrm;
extern	FILE	*outstrm;

extern	char	*infile;
extern	char	*outfile;

extern	int	*stat_field;
extern	long	statsize;
extern	int	nstat;

extern	int	Rflag;
extern	int	bflag;  /* added by DB june 1998 to support ensig enhancement*/
extern	int	iflag;
extern	long	*irange;
extern	long	nitem;
extern	int	Mflag;
extern	int	Iflag;
extern	int	Cflag;
extern	int	Eflag;
extern	int	Aflag;
extern	long	**item_arrays;
extern	long	*n_items;
extern	long	*max_elems;

extern	float	**Data;

extern	int	ndouble;
extern	int	nfloat;
extern	int	nlong;
extern	int	nshort;
extern  int	nchar;

extern	char	*statfield;	/* string denoting the field on which to
				   perform statistics on */

extern	int	update_file;
extern	char	*class_name;

extern	char	*command_line;



