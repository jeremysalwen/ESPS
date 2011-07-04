/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Joe Buck
 * Checked by:
 * Revised by:  John Shore (to work correctly on pipes)
 *
 * This routine attempts to skip n records, where each record is siz bytes,
 * in a file. Negative arguments are allowed for non-pipes.
 * For files, fseek is used; for pipes, characters are read and discarded.
 */

static char *sccs_id = "@(#)skiprec.c	1.18	5/1/98	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>
#include <sphere/fob.h>
#include <sphere/spchksum.h>
#include <sphere/spfile.h>

long            lseek();
char	       *get_sphere_hdr();

extern		debug_level;

void
skiprec(stream, nrecs, siz)
   long            nrecs;
   int             siz;
   FILE           *stream;
{
   long            n;		/* total number of bytes to skip */

   spsassert(stream != NULL, "skiprec: called on null stream");
   spsassert(siz >= 0, "skiprec: called with invalid record size");

   n = nrecs * siz;
#ifdef DOESNT_WORK_ON_OSX
   if (lseek(fileno(stream), 0L, 1) < 0) {
      /* we're on a pipe */
      if (nrecs >= 0) {
	 while (n--)
	    getc(stream);
      } else {
	 perror("skiprec: can't back up in pipe");
	 exit(2);
      }
   } else {

#ifdef HP
      /* This is to fix an HP fseek bug that we occasionally bump into */
      int             c;
      if ((c = getc(stream)) == EOF)
	 return;
      else
	 ungetc(c, stream);
#endif

      /* not on a pipe, so fseek will work */
      if (fseek(stream, n, 1) < 0) {
	 perror("skiprec: seek error");
	 exit(2);
      }
   }
#else
   /* On MAC-OSX, the lseek() test above ruins the stream integrity.
      So, for now, we simply don't check (gracefully) for piped
      operation! */
      if (fseek(stream, n, 1) < 0) {
	 perror("skiprec: seek error");
	 exit(2);
      }
#endif
}

void
fea_skiprec(stream, nrecs, hdr)
   FILE           *stream;
   long            nrecs;
   struct header  *hdr;
{
   char           *sphere_hdr_ptr;
   esignal_hd	  *esignal_hdr_ptr;
   pc_wav_hd	  *pc_wav_hdr_ptr;


   spsassert(stream != NULL, "fea_skiprec: called on null stream");
   spsassert(hdr != NULL, "fea_skiprec: called on null hdr");

   if (nrecs == 0)
      return;

   esignal_hdr_ptr = get_esignal_hdr(hdr);

   if (esignal_hdr_ptr) {
      esignal_skip_recs(stream, nrecs, esignal_hdr_ptr);
      return;
   }

   sphere_hdr_ptr = get_sphere_hdr(hdr);

   if (sphere_hdr_ptr) {
      sp_seek(sphere_hdr_ptr, nrecs, 1);
      return;
   }

   pc_wav_hdr_ptr = get_pc_wav_hdr(hdr);

   if (pc_wav_hdr_ptr) {
       pc_wav_skip_recs(stream, nrecs, pc_wav_hdr_ptr);
       return;
   }

   skiprec(stream, nrecs, size_rec(hdr));
   return;
}
