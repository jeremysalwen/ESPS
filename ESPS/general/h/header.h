/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."

  @(#)header.h	1.97 7/23/96 ESI

  This file contains the header structures for ESPS files.   It also
  contains some declarations used by all esps programs for the header
  access routines.

*/

#ifndef header_H
#define header_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <esps/esps.h>

/*
 * Structure of a SPS data file header
 */

/* Z domain functions are represented by the following structure; */

struct zfunc {
  short        nsiz;	/* length of numerator polynomial */
  short        dsiz;	/* length of denominator polynomial */
  float        *zeros;	/* pointer to numerator polynomial */
  float        *poles;	/* pointer to denominator polynomial */
};

/* do not change anything of these values without understanding the code
   in headers.c  Changes here might prevent processing of old data
   files.
*/

#define	MAX_SOURCES	10
#define	MAX_STRING	32767
#define	NSPARES		10
#define	HD_VERSION	"1.97"
#define	DATESIZE	26
#define	VERSIONSIZE	8
#define	PROGSIZE	16
#define GENTABSIZ	100
#define USERSIZ		8

struct header {

/*  fields common to all SPS headers  */

/* fixed size part first */
    struct fixpart {
    short	type;			/* file type */
#ifndef NOPAD
    short	pad1;	/**/
#endif
    long	check;			/* check field  */
    char	date[DATESIZE];		/* file creation date */
    char	hdvers[VERSIONSIZE];	/* header version */
    char	prog[PROGSIZE];		/* program name */
    char	vers[VERSIONSIZE];	/* prog version */
    char	progdate[DATESIZE];	/* prog compile date */
    long	ndrec;			/* number of data records */
    short	tag;			/* YES if data has tag */
    short	nd1;			/* used for reading old files only */
    long	ndouble;		/* number of doubles */
    long	nfloat;			/* number of floats */
    long	nlong;			/* number of longs */
    long	nshort;			/* number of shorts */
    long	nchar;			/* number of chars */
    long	fixpartsiz;		/* fixed header size */
    long	hsize;			/* total header size */
    char	user[USERSIZ];		/* user name */
    short	edr;			/* YES if EDR_ESPS, NO for native */
    short	machine_code;		/* machine that produced file */
    short	spares[NSPARES];	/* spares */
    } common;

/* variable part size next */
    struct varsize {
    char	*source[MAX_SOURCES];	/* pointers to src file names */
    struct header *srchead[MAX_SOURCES];/* pointers to src headers */
    char	*typtxt;		/* text field */
    char	*comment;		/* comment field */
    char	*refer;			/* reference file for tags */
    short	nnames;			/* number of source file names */
    short	nheads;			/* number of source file headers */
    struct gen_hd *gentab[GENTABSIZ];	/* generic item symbol table */
    short 	ngen;			/* number of generic items */
    struct header *refhd;		/* reference header */
    char 	*current_path;		/* cwd at time file was created */
    } variable;
    
/* type specific portion of the header */
   union {
    struct	sd_header *sd;		/* sampled data file */
    struct	spec_header *spec;	/* spectral record file */
    struct	filt_header *filt;	/* filter coefficient file */
    struct 	scbk_header *scbk;	/* scaler codebook file */
    struct	fea_header *fea;	/* feature file */

/* any local type specific headers will be defined in the following
   include file.  
*/
#ifdef ESI
#include <esps/localhd1.h>
#endif

   } hd;
};

/* the include file localhd2.h contain the structure definitions for
   local type specific headers. 
*/
#ifdef ESI
#include <esps/localhd2.h>
#endif

/* What follows are the general type specific header structures
*/

/* Sampled Data File specific header */
#define SD_SPARES 16

struct sd_header {
   short	equip;		/* A/D equipment */
#ifndef NOPAD
   short	pad1;	/**/
#endif
   float	max_value;	/* range of A/D or max value */
   float	sf;		/* sample frequency */
   float	src_sf;		/* source file sf */
   short	synt_method;	/* synthesis method */
#ifndef NOPAD
   short	pad2;  /**/
#endif
   float	scale;		/* scale factor */
   float	dcrem;		/* DC term removed */
   short	q_method;	/* quant method */
   short	v_excit_method;	/* voiced excitation */
   short	uv_excit_method;/* unvoiced excitation */
   short	spare1;		/* spare */
   short	nchan;		/* number of channels for multiplexed data */
   short	synt_interp;	/* reflection coefficient interpolation */
   short	synt_pwr;	/* power source for synthesis */
   short	synt_rc;	/* reflection coefficient interpolation */
   short	synt_order;	/* synthesis filter order */
#ifndef NOPAD
   short	pad3;  /**/
#endif
   long		start;		/* starting point processed */
   long		nan;		/* number of points processed */
   short	spares[SD_SPARES];/* spares */
   struct zfunc	*prefilter;	/* prefilter */
   struct zfunc *de_emp;	/* deemphasis filter */
};

/* SPEC Spectral Record File specific header */
#define SPEC_SPARES 20

struct spec_header {
   long		start;		/* starting point analyzed */
   long		nan;		/* number of points analyzed */
   short	frmlen;		/* analysis window width */
   short	order_vcd;	/* model order voiced */
   short	order_unvcd;	/* model order unvoiced */
   short	win_type;	/* data window type */
   float	sf;		/* sampling frequency */
   short	spec_an_meth;	/* analysis method */
#ifndef NOPAD
   short	pad1; /**/
#endif
   float	dcrem;		/* DC term removed */
   short	post_proc;	/* post-processing method */
   short	frame_meth;	/* how speech was divided into frames */
   short	voicing;	/* voicing indicator appears in each record */
   short	freq_format;	/* how to determine the set of frequencies */
   short	spec_type;	/* are data power, log power, complex, etc. */
   short	contin;		/* discrete distribution or continuous density */
   long 	num_freqs;	/* number of frequencies */
   short	spares[SPEC_SPARES];/* spares */
   float	*freqs;		/* frequencies (if listed in header) */
   struct	zfunc *pre_emp;	/* preemphasis filter */
};


/* FILT filter coefficient header */
#define FILT_SPARES 20

struct	filt_header {
   short	max_num;        /* maximum number of numerator coefficients. */
   short	max_den;        /* maximum number of denominator coefficients. */
   short	func_spec;      /* desired response function specification */
   short	nbands;         /* number of frequency bands */
   short	npoints;        /* number of points  */
   short	g_size;         /* grid size parameter */
   short	nbits;          /* number of bits      */
   short	type;           /* type of filter  */
   short	method;         /* filter design method  */
   short	spares[FILT_SPARES]; /* spares */
#ifndef NOPAD
   short	pad1; /**/
#endif
   float	*bandedges;     /* array of band edges */
   float	*points;        /* array of or points */
   float	*gains;         /* array of gain values  */
   float	*wts;           /* array of weighting values */
};

/* SCBK scaler quantization codebook header */
#define SCBK_SPARES 20

struct  scbk_header {
   long		num_items;	/* number of items processed */
   short	distortion;	/* distortion measure used */
   unsigned short num_cdwds;	/* number of codeswords */
   float	convergence;	/* convergence threshold */
   short	codebook_type;	/* type of codebook */
   short	element_num;	/* element number */
   short	spares[SCBK_SPARES]; /* spares */
};

/* FEA feature file header */
#define FEA_SPARES 16

struct fea_header {
   short	fea_type;	/* indicates special feature-file types */
   short	segment_labeled;/* if YES, records contain filename, 
				   start and length of segment */
   unsigned short field_count;	/* number of fields */
#ifndef NOPAD
   short	field_order; 	/* YES if fields stored in field order
				   on file, instead of by data type (default)*/
#endif
   char 	**names;	/* name of each field */
   long		*sizes;		/* total number of items in field */
   short 	*ranks;		/* number of dimensions in field */
   long		**dimens;	/* array dimensions for field */
   short 	*types;		/* type of field */
   char 	***enums;	/* arrays of values for coded types */
   long		*starts;	/* starting point for this field */
   short 	*derived;	/* indicates whether field was derived */
   char		***srcfields;	/* for derived fields, 
				   array of source field names */
   short	spares[FEA_SPARES]; /* spares */
   long		ndouble;	/* number of doubles in feature record */
   long		ndcplx;		/* number of double complex in feature record */
   long		nfloat;		/* number of floats in feature record */
   long		nfcplx;		/* number of float complex in feature record */
   long		nlong;		/* number of longs in feature record */
   long		nlcplx;		/* number of long complex in feature record */
   long		nshort;		/* number of shorts in feature record */
   long		nscplx;		/* number of short complex in feature record */
   long		nbyte;		/* number of bytes in feature record */
   long		nbcplx;		/* number of byte complex in feature record */
};

/* symbol table for generic header items 

   in this structure, codes points to an array of the 
   possible coded values for the symbol.  It is an array
   of character strings, terminated by a NULL. 
*/

struct gen_hd {
   char		*name;		/* symbol name */
   unsigned int	size;		/* size of item */
   short	type;		/* type of symbol */
   char		*d_ptr;		/* pointer to the data */
   char		**codes;	/* codes for CODED data type */
   struct gen_hd *next;		/* next in chain */
};

/* ESPS header preamble.   This little header comes before the header
   described above.    It is intended to make it possible to identify
   and use an ESPS file, if the standard header processing cannot be
   done.
*/

struct preamble {
#ifndef DEC_ALPHA
   long		machine_code;	/* defined below */
   long		check_code;	/* version check code */
   long		data_offset;	/* data offset (in bytes, from 0) in file */
   long		record_size;	/* record size in bytes */
   long		check;		/* ESPS check number, same as main header */
   long		edr;		/* YES if EDR_ESPS, NO if native */
   long		align_pad_size; /* alignment pad need for some SD files */
   long		foreign_hd; 	/* pointer to foreign header, -1 if */
#else
   int		machine_code;	/* defined below */
   int		check_code;	/* version check code */
   int		data_offset;	/* data offset (in bytes, from 0) in file */
   int		record_size;	/* record size in bytes */
   int		check;		/* ESPS check number, same as main header */
   int		edr;		/* YES if EDR_ESPS, NO if native */
   int		align_pad_size; /* alignment pad need for some SD files */
   int		foreign_hd; 	/* pointer to foreign header, -1 if */
#endif
				/* there is none */
};


/* The next part of this file defines symbols to be used in programs for
   certain field values.
*/

/* A/D equipment codes; value for:
 * equip
 */
#define EF12M	1
#define AD12F	2
#define DSC	3
#define LPA11	4
#define AD12FA	5
#define AD12	6
#define AD16M   7
#define LOCALad1  8
#define LOCALad2  9
#define RTI_732	  10

extern char *equip_codes[]; 

/* quantization methods; values for:
 * piq_method rcq_method pwq_method lpq_method q_method, ros_q_method
 */

#define LPC10	1
#define MULAW	2 /* only for q_method in SD */
#define LPC10PA 3
#define LPC10PB 4
#define ROSA	5
#define AVGROSA 6
#define ROSPA	7
#define ROSPB	8
#define ROSPWRA 9
#define ROSPWRB 10
#define ROSB	11
#define JND	12
#define LPC10PC 13
#define ROS1	14
#define ROS1B	15
#define ROS2	16
#define ROS1C	17
#define ROS1D	18
#define ROS1E	19
#define ROS2B	20
#define ROS2C	21
#define VQ	22
#define SPAREqm1	23
#define SPAREqm2	24
#define SPAREqm3	25
extern char *quant_methods[]; 

/* window type codes; values for:
 * win_type 
 */
#define HAMMING	1

extern char *win_type_codes[]; 


/* synthesis methods; values for:
 * synt_method
 */
#define PSYNCH	1

extern char *synt_methods[];

/* synthesis reflection coefficient computation methods; values for:
 * synt_rc
 */
#define ANA	1
#define	SINX	2

extern char *synt_ref_methods[];

/* excitation methods; values for:
 * u_exite_method, uv_exite_method
 */
#define WHITE 1
#define IMPULSE	2

extern char *excit_methods[];

/* synthesis interpolation methods; values for:
 * synt_interp
 */
#define PULSE	1	
#define	SAMPLE	2

extern char *synt_inter_methods[];


/* synthesis power source; values for:
 * synt_pwr
 */
#define RAWPULSE	1
#define	LPCPULSE	2

extern char *synt_pwr_codes[]; 

/* values for:  spec_an_meth */
extern char *spec_an_methods[];

/* values for:  post_proc */
extern char *post_proc_codes[];

/* values for: frame_meth */
#define FM_NONE		0
#define FM_FIXED 	1
#define FM_VARIABLE 	2

extern char *frame_methods[]; 

/* values for: freq_format */
#define SYM_CTR		1
#define SYM_EDGE	2
#define ASYM_CEN	3
#define ASYM_EDGE	4	
#define ARB_VAR		5
#define ARB_FIXED	6

extern char *freq_format_codes[];

/* values for: spec_type */
#define ST_PWR		1
#define ST_DB		2
#define ST_REAL		3
#define ST_CPLX		4

extern char *spec_type_codes[]; 


/* values for FILT and FEAFILT func_spec */
#define BAND		1
#define POINT		2
#define IIR		3

extern char *filt_func_spec[];
extern char *feafilt_func_spec[];

/* values for FILT and FEAFILT type */
#define FILT_LP		1
#define FILT_HP		2
#define FILT_BP		3
#define FILT_BS		4
#define FILT_ARB	5

extern char *filt_type[];
extern char *feafilt_type[];

/* values for FILT and FEAFILT method */
#define PZ_PLACE	1
#define PARKS_MC	2
#define WMSE		3
#define BUTTERWORTH	4
#define BESSEL		5
#define CHEBYSHEV1	6
#define CHEBYSHEV2	7
#define ELLIPTICAL	8
#define CONST_BASED     9
#define WINDOW_METH    10

extern char *filt_method[];
extern char *feafilt_method[];

/* values for FEAFILT complex_filter and define_pz */
/* already defined as YES and NO */

extern char *feafilt_yesno[];

/* values for SCBK distortion */
#define SQUARED_ERROR 1

extern char *scbk_distortion[];

/* values for SCBK codebook_type */
#define RC_CBK		1
#define RC_UNVCD_CBK	2
#define RC_VCD_CBK	3
#define PP_CBK		4
#define PD_CBK		5
#define FL_CBK		6
#define EP_CBK		7
#define PPR_CBK		8
#define PDD_CBK		9
#define LAR_VCD_CBK	10
#define LAR_UNVCD_CBK	11
#define LAR_CBK		12

extern char *scbk_codebook_type[];

/* value for fea_type */
#define FEA_GEN		1

extern char *fea_file_type[];

/* 
 * values used by header access programs only
 */

#define PT_ENDPAR	0		/* codes for variable items */
#define PT_SOURCE	1
#define PT_TYPTXT	2
#define PT_REFER	3
#define PT_HEADER	4
#define PT_PRE		5
#define PT_FILTER	6
#define PT_PRIOR	7
#define PT_WEIGHT	8
#define PT_PREFILTER	9
#define PT_LPF		10
#define PT_COMMENT	11
#define PT_DEEMP	12
#define PT_GENHD	13
#define PT_REFHD	14
#define PT_CWD		15
#define PT_MAX		15		/* change as we add codes */

#define FIXPART_LEN		(sizeof (struct fixpart)) /* in bytes */
#define FIX_HEADER_SIZE		(FIXPART_LEN / sizeof (int))
#define OLD_HD_CHECK_VAL	27182
#define HD_CHECK_VAL		27162
#define SD_SIZE			(sizeof (struct sd_header))
#define SPEC_SIZE		(sizeof (struct spec_header))
#define ROS_SIZE		(sizeof (struct ros_header))
#define FILT_SIZE		(sizeof (struct filt_header))
#define SCBK_SIZE		(sizeof (struct scbk_header))
#define FEA_SIZE		(sizeof (struct fea_header))

#ifdef ESI
#define ANA_SIZE		(sizeof (struct ana_header))
#define PIT_SIZE		(sizeof (struct pit_header))
#endif

/* add new machine types here.  Don't change these numbers, just add
   them
*/

#define MASSCOMP_CODE 	1
#define SUN3_CODE  	2
#define CONVEX_CODE	3
#define SUN4_CODE	4
#define HP300_CODE	5	
#define SUN386i_CODE	6
#define DS3100_CODE	7
#define	MACII_CODE	8
#define SG_CODE		9
#define HP800_CODE	10
#define VAX_CODE	11
#define DG_AVIION_CODE  12
#define APOLLO_68K_CODE 13
#define APOLLO_10000_CODE 14
#define HP400_CODE	15
#define CRAY_CODE	16
#define SONY_RISC_CODE	17
#define SONY_68K_CODE	18
#define STARDENT_3000_CODE   19
#define IBM_RS6000_CODE	20
#define HP700_CODE	21
#define DEC_ALPHA_CODE  22
#define SOLARIS_86_CODE	23
#define LINUX_OR_MAC_CODE	24
#define UNKNOWN_CODE	99

#if defined(SONY_RISC) || defined(SONY_68K)
#define SONY
#endif

#if defined(M5500) || defined(M5600)
#define MACH_CODE MASSCOMP_CODE
#endif

#ifdef SUN3
#define MACH_CODE SUN3_CODE
#endif

#ifdef CONVEX
#define MACH_CODE CONVEX_CODE
#endif

#ifdef SUN4
#define MACH_CODE SUN4_CODE
#endif

#ifdef HP300
#define MACH_CODE HP300_CODE
#endif

#ifdef HP800
#define MACH_CODE HP800_CODE
#endif

#ifdef SUN386i
#define MACH_CODE SUN386i_CODE
#endif

#ifdef DS3100
#define MACH_CODE DS3100_CODE
#endif

#ifdef MACII
#define MACH_CODE MACII_CODE
#endif

#ifdef SG
#define MACH_CODE SG_CODE
#endif

#ifdef VAX
#define MACH_CODE VAX_CODE
#endif

#ifdef SONY_RISC
#define MACH_CODE SONY_RISC_CODE
#endif

#ifdef SONY_68K
#define MACH_CODE SONY_68K_CODE
#endif

#ifdef APOLLO_68K
#define MACH_CODE APOLLO_68K_CODE
#endif

#ifdef STARDENT_3000
#define MACH_CODE STARDENT_3000_CODE
#endif

#ifdef HP700
#define MACH_CODE HP700_CODE
#endif

#ifdef HP400
#define MACH_CODE HP400_CODE
#endif

#ifdef IBM_RS6000
#define MACH_CODE IBM_RS6000_CODE
#endif

#ifdef DEC_ALPHA
#define MACH_CODE DEC_ALPHA_CODE
#endif

#ifdef SOLARIS_86
#define MACH_CODE SOLARIS_86_CODE
#endif

#ifdef LINUX_OR_MAC
#define MACH_CODE LINUX_OR_MAC_CODE
#endif

#ifndef MACH_CODE
#define MACH_CODE UNKNOWN_CODE
#endif

extern char *machine_codes[];


/* declare functions */

struct header *
read_header ARGS((FILE *fd));

struct header *
copy_header ARGS((struct header *src));

int
skip_header ARGS((FILE *file));

struct header *
new_header ARGS((int type));

void
write_header ARGS((struct header *p, FILE *fd));

void
free_header ARGS((struct header *hd, long unsigned int flags, char *ptr));

void
add_source_file ARGS((struct header *hd, char *name, struct header *srchd));

void
add_comment ARGS((struct header *hd, char *text));

struct zfunc *
new_zfunc ARGS((int ns, int ds, float *num, float *den));

char *
eopen ARGS((char *prog_name, char *file_name, char *mode,
	    int type, int subtype, struct header **header, FILE **stream));

char *
eopen2 ARGS((char *prog_name, char *file_name, char *mode,
	     int type, int subtype, struct header **header, FILE **stream));

char *
add_genhd_efile ARGS((char *item_name, char *file_name, struct header *hd));

char *
add_genhd_afile ARGS((char *item_name, char *file_name, struct header *hd));

char *
get_genhd_efile_name ARGS((char *name, struct header *hd));

char *
get_genhd_afile_name ARGS((char *name, struct header *hd));

struct header *
get_genhd_efile ARGS((char *name, struct header *hd));

FILE *
get_genhd_afile ARGS((char *name, struct header *hd));

int
genhd_type ARGS((char *name, int *size, struct header *hd));

char **
genhd_list ARGS((int *size, struct header *hd));

char *
get_genhd ARGS((char *name, struct header *hd));

char *
add_genhd ARGS((char *name, int type, int size,
		char *ptr, char **codes, struct header *hd));

char *
add_genhd_c ARGS((char *name, char *ptr, int size, struct header *hd));

double *
add_genhd_d ARGS((char *name, double *ptr, int size, struct header *hd));

float *
add_genhd_f ARGS((char *name, float *ptr, int size, struct header *hd));

long *
add_genhd_l ARGS((char *name, long int *ptr, int size, struct header *hd));

short *
add_genhd_s ARGS((char *name, short int *ptr, int size, struct header *hd));

short *
add_genhd_e ARGS((char *name, short int *ptr,
		  int size, char **codes, struct header *hd));

int
copy_genhd ARGS((struct header *dest, struct header *src, char *name));

int
copy_genhd_uniq ARGS((struct header *dest, struct header *src, char *name));

void
add_genzfunc ARGS((char *name, struct zfunc *filter, struct header *hd));

struct zfunc *
get_genzfunc ARGS((char *name, struct header *hd));

char **
genhd_codes ARGS((char *name, struct header *hd));

char **
get_genhd_coded ARGS((char *name, struct header *hd));

double
get_genhd_val ARGS((char *name, struct header *hd, double def_val));

long
get_genhd_val_l ARGS((char *name, struct header *hd, long def_val));

void
update_waves_gen ARGS((struct header *ih,
		       struct header *oh, double start, double step));
/* start and step are specified as float in pre-ANSI style in updatewave.c;
 * therefore they are passed as double.
 */

void
inhibit_hdr_date ARGS((void));


#ifdef DEC_ALPHA
#include <malloc.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* header_H */
