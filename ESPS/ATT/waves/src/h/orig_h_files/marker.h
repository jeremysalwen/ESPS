/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)marker.h	1.3 11/6/95 ATT/ERL/ESI */
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
 *  marker.h
 *  externals and defines for speech annotation program
 */

#include <stdio.h>
#include <math.h>
#ifdef OS5
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/file.h>

#ifdef FOR_SUNVIEW
#include <suntool/sunview.h>
#include <suntool/panel.h>
#include <suntool/canvas.h>
#include <suntool/text.h>
#include <suntool/seln.h>
#endif
#ifdef FOR_XVIEW
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/text.h>
#ifndef APOLLO_68K
#include <xview/seln.h>
#endif
#include <xview/font.h>
#endif

#include <Methods.h>

#ifdef FOR_SUNVIEW
#define marks_BOUND	struct pr_subregion
#endif
#ifdef FOR_XVIEW
#define marks_BOUND	Font_string_dims
#endif

#define PIX_PER_CM 32.582
#define SCREEN_WIDTH 1152
#define SCREEN_HEIGHT 900

#define NFILE 128
#define NLABEL 1000
#define NSPELLING 1000
#define NTEXT 2000
#define NCOMMENT 2000

typedef struct markeR {
  char label[NLABEL];
  float time;
  struct markeR *right;
  struct markeR *left;
  struct worD *word;
  marks_BOUND bound;
} Marker;

typedef struct worD {
  char spelling[NSPELLING];
  char transcription[NSPELLING];
  struct worD *right;
  struct worD *left;
  struct markeR *first;
  struct markeR *last;
  marks_BOUND bound;
} Word;

typedef struct sentencE {
  char text[NTEXT];
  struct sentencE *left;
  struct sentencE *right;
  struct worD *first;
  struct worD *last;
  marks_BOUND bound;
} Sentence;

/*
 * Since the marker program assumes only one file at a time,
 * file and object data are collapsed into this all-inclusive
 * Object structure.
 */
typedef struct objects {
  char name[NFILE];
  Canvas canvas;	/* queue of labels */
  Methods *methods;
  struct objects *next;
  /*
   *	items added solely for use by plot window
   */
  Canvas pcanvas;	/* plot of labels */
  int x_off, xloc, yloc, width, height, color;
  double sec_cm, start, cursor, time;
} Objects;


Word *get_word();
Marker *get_marker();
Sentence *get_sentence();
Sentence *read_markers();
char *get_receiver_name();
char *get_methods();
char *return_value();
char *meth_read(), *meth_write(), *meth_mark(), *meth_replot(),
     *meth_quit(), *meth_make(), *meth_kill();

