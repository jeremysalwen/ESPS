/*  distort.h - include file for distort.c
 *
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *  Module Name:  distort.h
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *
 *  Purpose:  Include file for distort.c
 *
 *  Secrets:  None
 *  
 * @(#)distort.h	3.1	11/3/87	ESI
 */

/*
 *    E X T E R N A L   V A R I A B L E S 
 *
 *		U S E D   I N
 *
 *	  D  I  S  T  O  R  T .  C
 *
 */

extern	char		*f1str, *f2str;		/* string pointers */
extern	FILE		*f1p, *f2p;		/* file pointers */
extern	struct header	*f1h, *f2h;		/* pointers to structures */

extern	int		debug_level;	/* print variables for debugging */
extern	int		eflag;		/* element range flag */
extern	int		Eflag;		/* E option flag */
extern	int		rflag;		/* r flag */
extern	int		sflag;		/* Itakura-Saito distortion flag */
extern	int		nflag;		/* suppress output for SPEC files */

extern	long		s_rec;		/* start record */
extern	long		e_rec;		/* end record */
extern	long		s_ele;		/* starting element position */
extern	long		e_ele;		/* ending element position */


/*
 * Define's  used in  D I S T O R T . C
 *
*/

#define	  ABS(x)	((x) > 0 ? (x) : (-x))
#define   SQUARE(x)	(x) * (x)
