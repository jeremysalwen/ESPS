/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|     "Copyright (c) 1987-1990 Entropic Speech, Inc.			|
|      Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.	|
|                    All rights reserved."				|
|									|
+-----------------------------------------------------------------------+
|									|
|   This file should be included in all ESPS programs.
|									|
+----------------------------------------------------------------------*/
/* @(#)esps.h	1.46 24 Mar 1997 ERL */

#ifndef esps_H
#define esps_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Allow esps header files to be used either with or without ANSI
 * function prototypes.
 */

#undef ARGS
#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) \
    || defined(__cplusplus)
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

#ifndef FILE
# include <stdio.h>
#endif

/* Lint-pleasing macros */
#define Fprintf (void)fprintf
#define Sprintf (void)sprintf
#define Strcpy  (void)strcpy
#define Fclose  (void)fclose

#ifdef M5600
#define MAXHOSTNAMELEN 64
#endif

/* Commonly performed operations */
#ifndef lint
#define CANTOPEN(prog,file) { Fprintf(stderr,"%s: can't open ",prog); \
			      perror(file); exit(1);}

#define USAGE(text) { Fprintf(stderr,"Usage: %s\n",text); exit(1);}

#define NOTSPS(prog,bfile) { Fprintf (stderr, "%s: %s is not an ESPS file\n", \
				prog,bfile); exit(1);}

#define BOOL(arg) (arg == 1 || arg == 'Y' || arg == 'y')

#define get_genhd_c get_genhd
#define get_genhd_s (short *)get_genhd
#define get_genhd_l (long *)get_genhd
#define get_genhd_f (float *)get_genhd
#define get_genhd_d (double *)get_genhd

#define malloc_s(n) (short *)malloc((n)*sizeof (short))
#define malloc_i(n) (int *)malloc((n)*sizeof (int))
#define malloc_l(n) (long *)malloc((n)*sizeof (long))
#define malloc_f(n) (float *)malloc((n)*sizeof (float))
#define malloc_d(n) (double *)malloc((n)*sizeof (double))

#define calloc_s(n) (short *)calloc((n),sizeof (short))
#define calloc_i(n) (int *)calloc((n),sizeof (int))
#define calloc_l(n) (long *)calloc((n),sizeof (long))
#define calloc_f(n) (float *)calloc((n),sizeof (float))
#define calloc_d(n) (double *)calloc((n),sizeof (double))
#endif

#ifdef lint
void USAGE(), CANTOPEN(), NOTSPS();
char *get_genhd_c();
short *get_genhd_s(), *malloc_s(), *calloc_s();
long *get_genhd_l(), *malloc_l(), *calloc_l();
float *get_genhd_f(), *malloc_f(), *calloc_f();
double *get_genhd_d(), *malloc_d(), *calloc_d();
int *malloc_i(), *calloc_i();
#endif

#ifdef MACII
#define ROUND(x) (((x) > 0)? (int)(0.5+(x)) : 0-(int)(0.5-(x)))
#define LROUND(x) (((x) > 0)? (long)(0.5+(x)) : 0-(long)(0.5-(x)))
#else
#define ROUND(x) (((x) > 0)? (int)(0.5+(x)) : -(int)(0.5-(x)))
#define LROUND(x) (((x) > 0)? (long)(0.5+(x)) : -(long)(0.5-(x)))
#endif

#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) 
#ifdef MIN
#undef MIN
#endif
#define MIN(a, b) (((a) < (b)) ? (a) : (b)) 

#define TRYOPEN(prog,file,mode,fd) \
	if ((fd = fopen (file, mode)) == NULL) CANTOPEN (prog, file)

#define YES 1
#define NO 0
#define NONE 0
#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif
#ifndef true
# define true 1
# define false 0
#endif

#define DEFAULT_PORT 4050

/* type codes */
#define DOUBLE 1
#define FLOAT 2
#define LONG 3
#define SHORT 4
#define CHAR 5
#define UNDEF 6
#define CODED 7
#define BYTE 8
#define EFILE 9
#define AFILE 10
#define DOUBLE_CPLX 11
#define FLOAT_CPLX 12
#define LONG_CPLX 13
#define SHORT_CPLX 14
#define BYTE_CPLX 15

typedef struct {char    real, imag;}    byte_cplx;
typedef struct {short   real, imag;}    short_cplx;
typedef struct {long    real, imag;}    long_cplx;
typedef struct {float   real, imag;}    float_cplx;
typedef struct {double  real, imag;}    double_cplx;

extern char *type_codes[];

#define HD_UNDEF UNDEF

#define UE

#ifndef NOSPSINCLUDE

#include <esps/header.h>
#include <esps/param.h>
#include <esps/ftypes.h>
#include <esps/spsassert.h>
#include <esps/limits.h>
#include <esps/epaths.h>

#endif


void
skiprec ARGS((FILE *stream, long int nrecs, int siz));

void
fea_skiprec ARGS((FILE *stream, long int nrecs, struct header *hdr));


#if defined(DEC_ALPHA)
#include <stdlib.h>
extern void     *memmove();
extern char     *strcpy();
extern char     *strncpy();
extern char     *strcat();
extern char     *strncat();
extern int      memcmp();
extern int      strcmp();
extern int      strcoll();
extern int      strncmp();
extern size_t   strxfrm();
extern char     *strchr();
extern char     *strpbrk();
extern char     *strrchr();
extern char     *strstr();
extern char     *strtok();
extern char     *strerror();
#endif

#ifdef LINUX_OR_MAC
extern double   atof();
extern long     atol();
extern int      atoi();
extern char *getenv();
#endif


#ifdef __cplusplus
}
#endif

#endif /* esps_H */
