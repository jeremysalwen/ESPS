/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
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

static char *sccs_id = "@(#)markerutil.c	1.2	9/26/95	ATT/ERL";

#include <marker.h>

Marker *
first_free_marker(s)		/* free marker == time not set */
Sentence *s;
{
  Word *w;
  Marker *m;

  for(w=s->first;w && w->left != s->last; w = w->right){
    for(m=w->first; m && m->left != w->last; m = m->right){
      if(m->time <= 0.)return(m);
    }
  }
  fprintf(stderr,"no free marker found\n");
  return(s->last->last);
}
