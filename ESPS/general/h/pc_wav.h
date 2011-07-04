/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)pc_wav.h	1.1 5/1/98 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:  Declarations of functions and data structures
 * used by ESPS I/O routines for dealing with sampled-data files in RIFF
 * WAVE format.
 */

#ifndef pc_wav_H
#define pc_wav_H

#include <esps/esps.h>
#include <esps/fea.h>

#ifdef __cplusplus
extern "C" {
#endif

/* How many bytes do we need to read to recognize a PC WAV
 * header?  Defined and initialized in pc_wav.c.
 */
#define PC_WAV_PREFIX_SIZE	((int) PcWavPrefixSize)
extern int	PcWavPrefixSize;

typedef struct pc_wav_hd	pc_wav_hd;

/*
 * FUNCTION DECLARATIONS
 */

extern int
pc_wav_check_prefix ARGS((char *prefix));

extern struct header *
pc_wav_to_feasd ARGS((char *prefix, int num_read, FILE *file));

extern pc_wav_hd *
get_pc_wav_hdr ARGS((struct header *hdr));

long
pc_wav_rec_size ARGS((pc_wav_hd *wav_hd));

extern void
pc_wav_skip_recs ARGS((FILE *file, long nrec, pc_wav_hd *wav_hd));

extern int
pc_wav_get_rec ARGS((struct fea_data *rec,
		     pc_wav_hd *wav_hd, struct header *fea_hd, FILE *file));
extern long
pc_wav_getsd_recs ARGS((char *buffer,
			long num_records, pc_wav_hd *wav_hd, FILE *file));


#ifdef __cplusplus
}
#endif

#endif /* pc_wav_H */


