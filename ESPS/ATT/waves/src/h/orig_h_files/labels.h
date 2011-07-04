/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)labels.h	1.5 11/8/95 ATT/ERL/ESI */
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
 * Brief description: Definitions for labels
 *
 */




#define LAB_MAXFIELDS  32
#define RESIZE_STATUS_RESET		0
#define RESIZE_STATUS_RESIZE		1
#define RESIZE_STATUS_FORCED_RESIZE	2
#define RESIZE_STATUS_PLOTTED		4
#define RESIZE_STATUS_NEW		8

typedef struct fields {
  char *str;
  int field_id, xloc, yloc, color, font;
  struct fields *next;
} Fields;
  
typedef struct label {
	double time;
	int color;
	Fields *fields;
	struct label *prev;
	struct label *next;
} Label;

typedef struct labelfile {
  char *sig_name;
  char *label_name;
  char *comment;
  int type, color, changed, color_t, resize_ststus;
  double time_t;
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
  Pixfont *font;
#else
  char *font;
#endif
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
  Menu menu;
#else
  caddr_t menu;
#endif
  char separator[2];
  char fontfile[NAMELEN];
  char active[LAB_MAXFIELDS];
  int nfields, height;
  struct objects *obj;
  Label *first_label;
  Label *last_label;
  Label *move_pending;
  struct labelfile *next;
} Labelfile;

/*
#ifndef WAVEFORM_COLOR
#define WAVEFORM_COLOR 123
#define CURSOR_COLOR 122
#define MARKER_COLOR 121
#define RETICLE_COLOR 125
#endif
*/
struct objects {
  char *name;
  char *signal_name;
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
  Canvas canvas;
#else
  caddr_t canvas;
#endif
  Methods *methods;
  Labelfile *labels;
  int x_off, xloc, yloc, width, height, resize_status;
  int oldcursor_x, oldcursor_y;
  double sec_cm, start, cursor, old_cursor;
  struct objects *next;
};
#define Objects struct objects

typedef struct vlist {
  double start, end;
  int type;
  struct vlist *next;
} Vlist;

Fields *new_field();
Label *new_label();
Labelfile *new_labelfile(), *read_labels();


