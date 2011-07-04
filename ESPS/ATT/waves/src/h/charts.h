/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)charts.h	1.2 9/26/95 ATT/ERL/ESI */
/* charts.h */
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




typedef struct wobject {
  Objects *obj;
  int label_loc, label_size;
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
  Menu menu;
#else
  Char *menu;
#endif
  struct chartfile *charts;
  struct wobject *next;
} Wobject;

typedef struct hit {
  double start, end, like;
  struct hit *next;
} Hit ;

typedef struct word {
  char *str;
  int color, plotted;
  Hit *hits;
  struct word *prev, *next;
} Word;

typedef struct chartfile {
  char *sig_name;
  char *chart_name;
  int type, color, changed;
  double freq;
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
  Pixfont *font;
#else
  char *font;
#endif
  int eventx, eventy;		/* coords. when menu appeared */
  char *comment;
  Wobject *obj;
  Word *first_word;
  struct chartfile *next;
} Chartfile;

Wobject *get_wobject(), *new_wobject();

Chartfile *read_charts(), *new_chartfile(), *get_chart_level(),
  *get_chartfile();
