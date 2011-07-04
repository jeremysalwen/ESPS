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
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  Derek Lin
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)codec_stubs.c	1.3	1/15/93	ERL";


/* codec_stubs.c */

#include <stdio.h>

int da_done = 1;

play_from_file(istr, length, srate, amp, ehead, nrep)
     FILE *istr;
     double *srate;
     int amp;	/* Max sample value in the file or region. */
     void *ehead;
     int nrep;
{
  double frequency = *srate;
  int nout;

  nout = dacmaster_codec(-1, NULL, length, &frequency, amp, istr, ehead, nrep);
  return(nout);
}


