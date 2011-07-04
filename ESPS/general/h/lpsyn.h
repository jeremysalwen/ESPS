/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)lpsyn.h	1.3 2/20/96 ERL
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:  used by lp_syn(1-ESPS), ps_ana(1-ESPS)
 *
 */

#ifndef lpsyn_H
#define lpsyn_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#include <esps/anafea.h>

/* Excitation types */
#define EX_SHORT_RESID 0x104
#define EX_DOUBLE_RESID 0x108
#define EX_GAUSSIAN 0x200
#define EX_SINGLE_P_D 0x308
#define EX_IMPULSE  0x400
#define EX_NONE 0

/* window types -- these will not be supported once the lpc() routine is
replaced by compute_rc() which uses ESPS standard window definitions */
#define W_RECT 0
#define W_HAMM 1
#define W_COS4 2
#define W_HANN 3

/* Maximum LPC order assumed, e.g. for fixed arrays. */
#define MAX_ORD 40

typedef struct lp_frame {
  double *coef;			/* filter coefficients (if LP, a0 assumed 1.0) */
  double rms;			/* in the analysis window */
  double time;			/* time location of analysis window center */
  double wsize,			/* wind. dur. used in analysis */
         period;		/* pitch period for pitch synchronous analyses */
  int order;			/* LP filter order */
  int n;			/* number of samples in this frame */
  int fr_type;			/* analysis type (V/UV) */
  int ex_type;			/* excitation type */
  int sp_type;			/* filter parameter type */
  short *ex;			/* excitation signal */
  struct lp_frame *next;	/* linkage to next frame */
  struct lp_frame *prev;	/* linkage to previous frame */
} LP_frame;

typedef struct aparms {
  int type,			/* type of speech (V, UV, etc.) */
      order;			/* for LPC analysis */
  short returned,			/* code for parameter type to return */
        method,                   /* analysis method */
        wtype;			/* analysis window type to use */
  double preemp,		/* 1st-order preemphasis filter constant */
      per_step,			/* analysis step size for periodic analyses */
      per_wsize,		/* wind. dur. for periodic (UNVOICED) analyses */
      ps_wsize,			/* wind. dur. for VOICED analysis */
      period,			/* pitch period for pitch synchronous analyses */
      phase,			/* phase of pitch-synchronous window center */
      stab;			/* stability factor (-dB) */
} Aparams ;

typedef struct pblock {
  double freq,			/* sample frequency for filter output */
         dur,			/* Dur. of speech represented by this block. */
         start_time;		/* E.g. for alignment with original speech */
  int frames;			/* number of frames in this block. */
  char *text;			/* E.g. for phonetic sequence, orthography... */
  LP_frame *head,		/* Of parameter frames list */
          *tail;
  struct pblock *next,		/* For linking up block lists */
                *prev;
} Pblock;


LP_frame *
new_lp_frame ARGS((void));

int
free_lp_frame ARGS((LP_frame *lp));

void
free_lp_frames ARGS((LP_frame *lp));

LP_frame *
link_to_new_lp ARGS((double *coef, int order, double period, double wsize,
		     double rms, short int *ex, int n, double time,
		     int fr_type, int ex_type, int sp_type,
		     LP_frame *lp_prev));

LP_frame *
copy_to_new_lp ARGS((double *coef, int order, double period, double wsize,
		     double rms, short int *ex, int n, double time,
		     int fr_type, int ex_type, int sp_type,
		     LP_frame *lp_prev));

Pblock *
new_pblock ARGS((LP_frame *head, LP_frame *tail, double freq, double dur,
		 double start, int frames, char *text, Pblock *prev));

void
free_pblock ARGS((Pblock *pb));

void
free_pblocks ARGS((Pblock *pb));


#ifdef __cplusplus
}
#endif

#endif /* lpsyn_H */
