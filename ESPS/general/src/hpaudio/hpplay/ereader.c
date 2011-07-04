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

static char *sccs_id = "@(#)ereader.c	1.2	3/13/92	ERL";

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
      int     data_type = SHORT; /* holds type of data in sampled data files*/
      register int i;
      static   float *dataf = NULL;
      static int nfloat = 0;

      data_type = get_sd_type(ih);

      /* Allocate float buffer, if not already allocated */
      if(data_type != CHAR && data_type != SHORT){
	if( np > nfloat ) {
	  if(dataf && nfloat)
	    dataf = (float*)realloc(dataf, np*sizeof(float));
	  else
	    dataf = (float*)malloc(np * sizeof(float));
	  if(!dataf) {
	    Fprintf(stderr,"Cannot alloc %ld floats\n",np);
	    exit(1);
	  }
	  nfloat = np;
	}
      }

      if(data_type == CHAR || data_type == SHORT){
	if ((i = get_sd_recs(data,np,ih,istrm)) != np) {
	  if (i == 0) {
	    return(i);
	  } else {
	    np = i;
	  }
	}
      } else{
	if ((i = get_sd_recf(dataf,np,ih,istrm)) != np) {
	  if (i == 0) {
	    return(i);
	  }
	  else {
	    np = i;
	  }
	}
      }
	
      if (clip) {
	if (data_type != CHAR && data_type != SHORT){
	  register float *fp, plim=MAXDA, ft;
	  register short *sp;
	  for (i=np, fp = dataf, sp = data; i--; ) {
	    if((ft = *fp++) > plim)
	      ft = plim;
	    else
	      if(ft < -plim)
		ft = -plim;
	    *sp++ = ft;
	  }
	} else {
	  register short *sp, smax = MAXDA, st;
	  for (i=np, sp = data; i--; ) {
	    if((st = *sp) > smax)
	      st = smax;
	    else
	      if(st < -smax)
		st = -smax;
	    *sp++ = st;
	  }
	}
      }
	
      /* either, 1) multiply sample by gain factor (slow), 2) shift sample
	 bits to right (faster), or 3) just copy sample (fastest)
	 */
      if(data_type != CHAR && data_type != SHORT) {
	register short *sp;
	register float *fp, ft, g=gain, round = 0.5;
	if (gflag) 
	  for (i=np, sp = data, fp = dataf; i--; ) {
	    if((ft = *fp++ * g) >= 0.0)
	      *sp++ = ft + round;
	    else
	      *sp++ = ft - round;
	  } else
	    if (bflag && left) {
	      register int shift = left;
	      for (i=np, sp = data, fp = dataf ; i--;)
		*sp++ = (int)(dataf[i]) << shift;
	    } else
	      if (bflag && right) {
		register int shift = right;
		for (i=np, sp = data, fp = dataf ; i--;)
		  *sp++ = (int)(dataf[i]) >> shift;
	      } else 
		for (i=np, sp=data, fp=dataf; i--;) {
		  if((ft = *fp++) >= 0.0)
		    *sp++ = ft + round;
		  else
		    *sp++ = ft - round;
		}
      } else {
	register short *sp;
	register float ft, g=gain, round = 0.5;
	if (gflag)  {
	  for (i=np, sp = data ; i--;)
	    if((ft = *sp * g) >= 0.0)
	      *sp++ = ft + round;
	    else
	      *sp++ = ft - round;
	} else
	  if (bflag && left) {
	    register int shift = left;
	    for (i=np, sp = data; i--; )
	      *sp++ <<= shift;
	  } else
	    if (bflag && right) {
	      register int shift = right;
	      for (i=np, sp = data; i--; )
		*sp++ >>= shift;
	    }
      }
      return(np);
    } else
      fprintf(stderr,"Bad arguments to read_any_file()\n");
  return(0);
}
