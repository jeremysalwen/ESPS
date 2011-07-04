/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin, Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)lp_procs.c	1.6	11/11/96	ERL";

#include <esps/esps.h>
#include <esps/lpsyn.h>

/*************************************************************************/
LP_frame *new_lp_frame()
{
  LP_frame *lp;

  if((lp = (LP_frame*)malloc(sizeof(LP_frame)))) {
    lp->coef = NULL;
    lp->rms = 0.0;
    lp->time = 0.0;
    lp->order = 0;
    lp->n = 0;
    lp->ex_type = 0;
    lp->sp_type = 0;
    lp->ex = NULL;
    lp->next = NULL;
    lp->prev = NULL;
    return(lp);
  }
  printf("Can't allocate an LP_frame in new_lp_frame()\n");
  return(NULL);
}

/*************************************************************************/
free_lp_frame(lp)
     LP_frame *lp;
{
  if(lp) {
    if(lp->coef) free(lp->coef);
    if(lp->ex) free(lp->ex);
    free(lp);
    return(TRUE);
  }
  return(FALSE);
}

/*************************************************************************/
void
free_lp_frames(lp)
     LP_frame *lp;
{
  LP_frame *l=lp;

  while(l) {
    lp = lp->next;
    free_lp_frame(l);
    l = lp;
  }
}

/*************************************************************************/
LP_frame *link_to_new_lp(coef,order,period,wsize,rms,ex,n,time,fr_type,ex_type,
			 sp_type,lp_prev)
     double *coef, time, rms, period,wsize;
     short *ex;
     int ex_type, sp_type, n, order, fr_type;
     LP_frame *lp_prev;
{
  LP_frame *lp;

  if((lp = new_lp_frame())) {
    lp->coef = coef;
    lp->rms = rms;
    lp->period = period;
    lp->wsize = wsize;
    lp->ex = ex;
    lp->n = n;
    lp->order = order;
    lp->time = time;
    lp->fr_type = fr_type;
    lp->ex_type = ex_type;
    lp->sp_type = sp_type;
    lp->prev = lp_prev;
    if(lp_prev) lp_prev->next = lp;
    return(lp);
  }
  return(NULL);
}

/*************************************************************************/
LP_frame *copy_to_new_lp(coef,order,period,wsize,rms,ex,n,time,fr_type,ex_type,
			 sp_type,lp_prev)
     double *coef, time, rms, period, wsize;
     short *ex;
     int ex_type, sp_type, n, order, fr_type;
     LP_frame *lp_prev;
{
  LP_frame *lp;
  unsigned long alloc_size;

  if((lp = new_lp_frame())) {
    register double *dp1;
    register short *sp1;

#ifdef DEC_ALPHA
    if(n == 0) alloc_size = 1;
	else
#endif
	       alloc_size = n;

    if(!(lp->coef = dp1 = (double*)malloc(sizeof(double)*(unsigned long)order))) {
      printf("Can't allocate coefficient array in copy_to_new_lp()\n");
      return(NULL);
    }
    if(!(lp->ex = sp1 = (short*)malloc(sizeof(short)*alloc_size))) {
      printf("Can't allocate excitation array in copy_to_new_lp()\n");
      return(NULL);
    }
    lp->n = n;
    lp->order = order;
    lp->period = period;
    lp->wsize = wsize;
    lp->rms = rms;
    lp->time = time;
    for(; n--;) *sp1++ = *ex++;
    for(; order--;) *dp1++ = *coef++;
    lp->fr_type = fr_type;
    lp->ex_type = ex_type;
    lp->sp_type = sp_type;
    lp->prev = lp_prev;
    if(lp_prev) lp_prev->next = lp;
    return(lp);
  }
  return(NULL);
}

/*************************************************************************/
Pblock *new_pblock(head,tail,freq,dur,start,frames,text,prev)
     LP_frame *head, *tail;
     double freq, dur, start;
     int frames;
     char *text;
     Pblock *prev;
{
  Pblock *pb;

  if((pb = (Pblock*)malloc(sizeof(Pblock)))) {
    if(text && *text) {
      if(!(pb->text = (char *) malloc(strlen(text)+1))) {
	printf("Allocation problems in new_pblock()\n");
	return(NULL);
      }
      strcpy(pb->text,text);
    }
    pb->head = head;
    pb->tail = tail;
    pb->freq = freq;
    pb->dur = dur;
    pb->start_time = start;
    pb->frames = frames;
    pb->prev = prev;
    if(prev) prev->next = pb;
    return(pb);
  }
  printf("Allocation problems in new_pblock()\n");
  return(NULL);
}

/*************************************************************************/
void
free_pblock(pb)
     Pblock *pb;
{
  if(pb) {
    if(pb->head) free_lp_frames(pb->head);
    if(pb->text) free(pb->text);
    free(pb);
  }
  return;
}

/*************************************************************************/
void
free_pblocks(pb)
     Pblock *pb;
{
  Pblock *p;

  while(pb) {
    p = pb->next;
    free_pblock(pb);
    pb = p;
  }
}

