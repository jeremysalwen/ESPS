/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)reticle.h	1.2 9/26/95 ATT/ERL/ESI */
/* reticle.h */
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
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */


#ifdef FOR_XVIEW
#include <xview/font.h>
#endif

/* marking styles for tic marks */
#define CONTINUOUS 7		/* reticle lines span the plot */
#define EDGES 64		/* marks drawn at edges of plot */
#define MAJOR 1	      /* marks drawn at orthogonal axis's major intervals */
#define MINOR1 2		/* "                      minor1 "  */
#define MINOR2 4		/* "                      minor2 "  */
#define CENTERED 8		/* re orthogonal axis's marks */
#define LEFT_ALIGNED 16
#define RIGHT_ALIGNED 32
/* location of axis numbering */
#define NUM_LB 1		/* left or bottom */
#define NUM_RT 2		/* right or top */
#define NUM_BOTH 3		/* top and bottom or right and left */


typedef struct {
  double val;			/* data value at a tick mark */
  int	 loc;			/* pixel coord */
} LocVal;

typedef struct {		/* axis marker description */
  double inter;			/* in dimensions of corresponding axis */
  double length;		/* in dimensions of orthogonal axis */
  int    style;			/* EDGES, MAJOR, etc. */
			/* If list is non-NULL, ignore inter. */
  LocVal *list;			/* list of variably spaced values and coords */
  int    num;			/* length of list */
} Marker;

typedef struct {		/* axis reticle and numbering description */
  Marker maj, min1, min2;
  char   *precision;
  int    num_loc;
  double num_inter;
} Interval;

typedef struct bound {    /* boundaries or margins (pixel coordinates) */
  int top, bottom, left, right;
} Bound;

typedef struct range {		/* data limits (in data dimensions) */
  double start, end;
} Range;

typedef struct reticle {	/* "compleat" reticle description */
  Interval ordinate;
  Interval abscissa;
  Bound    bounds;
  int      color, linetype;
  char     *abs_label, *ord_label;
#ifdef FOR_SUNVIEW
  struct pixfont *font; /* SUNVIEW */
#else
#ifdef FOR_XVIEW
  Xv_Font  font; /* XVIEW */
#else
  caddr_t  font;
#endif
#endif
  Range    ordi, absc;
} Reticle;

Bound *reticle_get_margins();
Reticle *new_spectrogram_reticle(), *new_wave_reticle();

