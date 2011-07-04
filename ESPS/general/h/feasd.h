/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|     "Copyright (c) 1989-1990 Entropic Speech, Inc.			|
|      Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.	|
|                    All rights reserved."				|
| 									|
+-----------------------------------------------------------------------+
|									|
|   include file for feature file subtype FEA_SD (sampled data)		|
|									|
+----------------------------------------------------------------------*/
/* @(#)feasd.h	1.3	2/20/96	ESI */

#ifndef feasd_H
#define feasd_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * structure definition for FEA_SD records
 */

struct feasd
{
    short	data_type;
    long	num_records, num_channels;
    char	*data, *ptrs;
};

/*
 * Declarations for support functions.
 */

struct feasd *
allo_feasd_recs ARGS((struct header *hd, int data_type,
		      long int num_records, char *data, int make_ptrs));

long
get_feasd_recs ARGS((struct feasd *data, long int start,
		     long int num_records, struct header *hd, FILE *file));

long
get_feasd_orecs ARGS((struct feasd *data, long int start, long int framelen,
		      long int step, struct header *hd, FILE *file));

int
init_feasd_hd ARGS((struct header *hd, int data_type, int num_channels,
		    double *start_time, int mult_st_t, double record_freq));

int
put_feasd_recs ARGS((struct feasd *data, long int start,
		     long int num_records, struct header *hd, FILE *file));


#ifdef __cplusplus
}
#endif

#endif /* feasd_H */
