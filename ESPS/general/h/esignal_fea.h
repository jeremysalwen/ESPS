/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)esignal_fea.h	1.2 10/6/97 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:  Declarations of functions and data structures
 * used by ESPS I/O routines for dealing with files in Esignal format.
 *
 */

#ifndef esignal_fea_H
#define esignal_fea_H

#include <esps/esps.h>
#include <esps/fea.h>

#ifdef __cplusplus
extern "C" {
#endif


/*!*//* Separate definitions here, in esig2fea.c, and in fea2esig.c.
 * Consolidate.
 */
#define ESPS_TAG	    "Tag"
#define ESPS_FEA_SUBTYPE    "FeaSubtype"


/* How many bytes do we need to read to recognize an Esignal
 * header?  Defined and initialized in esignal_fea.c.
 */
#define ESIGNAL_PREFIX_SIZE	((int) EsignalPrefixSize)
extern int	EsignalPrefixSize;

typedef struct esignal_hd	esignal_hd;

/*
 * FUNCTION DECLARATIONS
 */

extern int
esignal_check_prefix ARGS((char *));

extern struct header *
esignal_to_fea ARGS((char *, int, FILE *));

extern esignal_hd *
get_esignal_hdr ARGS((struct header *));

extern long
esignal_rec_size ARGS((esignal_hd *));

extern void
esignal_skip_recs ARGS((FILE *, long, esignal_hd *));

extern int
esignal_get_rec ARGS((struct fea_data *, esignal_hd *, struct header *, FILE *));

extern long
esignal_getsd_recs ARGS((char *, long, esignal_hd *, FILE *));


#ifdef __cplusplus
}
#endif

#endif /* esignal_fea_H */
