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
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:  Private part of bar widget
 * @(#)BarP.h	1.3 9/20/93 ERL
 */

#ifndef BARP_H
#define BARP_H

/*#include "Bar.h"*/
#ifdef HP700
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/XmP.h>

/* Instance part */

typedef struct _XeBarPart{
  int num_entries;
  float *values;                 /* values in dB */
  float *old_values;
  float *last_maxstate_linear;   /* previous max in linear scale */
  int *inc;                      /* increment */
  float *last_state;             /* immediate previous max in dB */
  float max_value;
  float min_value;
  float scale;
  float *constRC;
  float *intv;
  Dimension default_bar_width;
  Dimension space;
  GC bar_GC;
  GC inverse_bar_GC;
  GC clip_GC;
  GC inverse_clip_GC;
  Position x;
  Position y;
  Dimension height;
  Dimension width;
  Pixel background_color;
  Pixel foreground_color;
  Pixel foreground_color_clip;
  Pixel background_color_clip;
} XeBarPart;

typedef struct _XeBarRec{
  CorePart core;
  XmPrimitivePart primitive;
  XeBarPart bar;
} XeBarRec;

/* Class part */

typedef struct _XeBarClassPart {
  int ignore;
} XeBarClassPart;

typedef struct _XeBarClassRec {
  CoreClassPart core_class;
  XmPrimitiveClassPart primitive_class;
  XeBarClassPart bar_class;
} XeBarClassRec;

extern XeBarClassRec XebarClassRec;

#endif /* BARP_H */
  
  

