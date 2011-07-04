/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)ereader.c	1.1 7/24/95 ERL";

/* ereader.c */

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <esps/esps.h>
#include <esps/sd.h>			/* SD file stuff */
#include <esps/fea.h>
#include <esps/feasd.h>

#define ERR 4
#define REALLY_HUGE_INTEGER 1000000000
#define MAXDA  32767

extern int debug_level, da_location, max_buff_bytes, w_verbose, position;

int qflag = 0, clip = 0, gflag = 0, right = 0, left = 0, bflag = 0;
float gain = 1.0;



/* Take advantage of some of the type independence offered by entropic
   data-reading routines. */

read_any_file(data, size, np, istrm, ih, fd)
     struct header *ih;		/* an ESPS header */
     int size,			/* size (in bytes) of each sample */
       np,	     /* Number of samples to read */
       fd;	     /* open file descriptor (alternate to stream I/O) */
     FILE *istrm;    /* an open and positioned stream */
     short *data;    /* data output buffer; assumed to be at least size*np long */
{
  if(! ih ) {
    if(istrm)
      return(fread(data, size, np, istrm));
    if(fd >= 0)
      return(read(fd,data,size*np)/size);
    else {
      fprintf(stderr,"Bad args passed to read_any_file()\n");
      return(0);
    }
  } else
    if(istrm) {
      int elements;
      register int i;
      register short *sp;
      register float ft, g=gain, round = 0.5;
      static struct feasd *fept = NULL;

      if(! fept) {
	if(!(fept = allo_feasd_recs(ih, SHORT, np, data, NO))) {
	  fprintf(stderr,"Problems creating a FEA record in ereader()\n");
	  return(-1);
	}
      } else {
	fept->data = (char*)data;
	if(ih->common.type == FT_FEA)
	  fept->num_channels = ih->hd.fea->sizes[0];
	else
	  if(ih->common.type == FT_SD)
	    fept->num_channels = ih->hd.sd->nchan;
	  else
	    fept->num_channels = size/sizeof(short);
	fept->num_records = np;
      }

      if ((i = get_feasd_recs(fept,(long)0,np,ih,istrm)) != np) {
	  if (i == 0) {
	    return(i);
	  } else {
	    np = i;
	  }
	}

      elements = (np * size)/sizeof(short);
	
      if (clip) {
	  register smax = MAXDA, st;
	  for (i=elements, sp = data; i--; ) {
	    if((st = *sp) > smax)
	      st = smax;
	    else
	      if(st < -smax)
		st = -smax;
	    *sp++ = st;
	}
      }
	
      /* either, 1) multiply sample by gain factor (slow), 2) shift sample
	 bits to right (faster), or 3) just copy sample (fastest)
	 */
	if (gflag)  {
	  for (i=elements, sp = data ; i--;)
	    if((ft = *sp * g) >= 0.0)
	      *sp++ = ft + round;
	    else
	      *sp++ = ft - round;
	} else
	  if (bflag && left) {
	    register int shift = left;
	    for (i=elements, sp = data; i--; )
	      *sp++ <<= shift;
	  } else
	    if (bflag && right) {
	      register int shift = right;
	      for (i=elements, sp = data; i--; )
		*sp++ >>= shift;
	    }
      return(np);
    } else
      fprintf(stderr,"Bad arguments to read_any_file()\n");
  return(0);
}
