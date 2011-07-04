/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)ereader.c	1.3	1/15/93	ERL";

/* ereader.c */

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <esps/esps.h>
#include <esps/sd.h>			/* SD file stuff */
#include <esps/fea.h>
#include <esps/feasd.h>

#define SPARC_SAMPLE_FREQ 8000

extern int debug_level, da_location, max_buff_bytes, w_verbose, position;
extern struct feasd *rec;    /* calling prog should have record allocated */

/*  read and resample an ESPS file pointed by ih, istrm, put in 'data'
 *  read a file pointed by istrm, put in 'data'
 *  read a file pointed by fd, put in 'data'
 *  receive memory buffer, does nothing
 */

read_any_file(data, size, inp, onp, istrm, ih, fd, freq)
     struct header *ih;		/* an ESPS header */
     int size,			/* sizr (in bytes) of each sample */
       inp,	     /* Number of samples to read */
       onp,          /* Number of output samples */
       fd;	     /* open file descriptor (alternate to stream I/O) */
     FILE *istrm;    /* an open and positioned stream */
     short *data;    /* data output buffer; assumed to be at least size*np long */
     double freq;
{
  long actsize;

  if(! ih ) 
    {
      if(istrm)
	return(fread(data, size, inp, istrm));
      if(fd >= 0)
	return(read(fd,data,size*inp)/size);
      else 
	{
	  fprintf(stderr,"Bad args passed to read_any_file()\n");
	  return(-1);
	}
    }
  else
    {
      if(istrm) 
	{
	  short *pre_resamp_data;
	  pre_resamp_data = (short *) rec->data;
	  actsize = get_feasd_recs( rec, 0L, inp, ih, istrm );
	  resample( pre_resamp_data, data, (double)freq/SPARC_SAMPLE_FREQ,
		   (long) inp, 1.0);
	  return(actsize);
	}
      else{
	fprintf(stderr,"Bad arguments to read_any_file()\n");
	return(-1);
      }      
    }
}



/* The following resample function was written by Tom Veatch,
 * Department of Linguistics, University of Pennsylvania,
 * Philadelphia, PA 19104.  Thanks, Tom.
 *
 * Copyright (c) 1990 Thomas C. Veatch
 *
 * Tom grants anyone the right to make or distribute verbatim copies
 * of this resample function, provided that the
 * copyright notice and permission notice are preserved,
 * and that the distributor grants the recipient permission
 * for further redistribution as permitted by this notice.
 *
 */

#define SETCOEFSINDS {j=(int)i*step;k=j+1;c2=fmod(i*step,1.0);c1=1.0-c2;}

resample(in,out,step,n,gain)
short *in;                        /* input data array*/
short *out;                       /* output data array */
register float step;              /* step: input sf / SPARC_SAMPLE_FREQ */
register float gain;              /* gain factor */
long n;                           /* size of elements in in/out */
{
  /* extern static float start_loc = 0;
     float loc; */

  register int i,j,k,nout;
  register float c1,c2;

  /* 2-point interpolate samples from input to output. */
  nout = n/step; /* number of output samples */

  if (gain!=1.0)
    for (i=0;i<nout;i++) {
      SETCOEFSINDS
      out[i]=gain*(c1*in[j]+c2*in[k]);
    }
  else
    for (i=0;i<nout;i++) {
      SETCOEFSINDS
      out[i]=(c1*in[j]+c2*in[k]);
    }
}

