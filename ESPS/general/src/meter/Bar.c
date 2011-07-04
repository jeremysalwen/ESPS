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
 * Brief description:  A bar widget based on Motif
 *
 */

static char *sccs_id = "@(#)Bar.c	1.2	6/23/93	ERL";

#include <stdio.h>
#include <math.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h> 
#include <Xm/Xm.h>
#include "BarP.h"
#include "Bar.h"

#define MAX_DB 90.

#define Offset(field) XtOffsetOf(XeBarRec, bar.field)
/*bar heigth, width not here yet */
static XtResource resources[] = {
  { XtNnumEntries, XtCNumEntries, XtRInt, sizeof(int),
      Offset(num_entries), XtRString, "1"},
  { XtNvalues, XtCValues, XtRPointer, sizeof(float *),
      Offset(values), (XtRPointer), NULL},
  { XtNmaxValue, XtCMaxValue, XtRFloat, sizeof(float),
      Offset(max_value), XtRString, "100"},
  { XtNminValue, XtCMinValue, XtRFloat, sizeof(float),
      Offset(min_value), XtRString, "0"},
  { XtNscale, XtCScale, XtRFloat, sizeof(float),
      Offset(scale), XtRString,  "1"},
  { XtNconstRC, XtCConstRC, XtRFloat, sizeof(float),
      Offset(constRC), XtRString,  "-0.1"},
  { XtNintv, XtCIntv, XtRFloat, sizeof(float),
      Offset(intv), XtRString,  "1"},
/*  { XtNdefaultBarWidth, XtCDefaultBarWidth, XtRDimension, sizeof(Dimension),
      Offset(default_bar_width), XtRString, "15"},*/
  { XtNspace, XtCSpace, XtRDimension, sizeof(Dimension),
      Offset(space), XtRString, "10"},
  { XtNx, XtCX, XtRPosition, sizeof(Position),
      Offset(x), XtRString, "10"},
  { XtNy, XtCY, XtRPosition, sizeof(Position),
      Offset(y), XtRString, "10"},
  { XtNforegroundColor, XtCForegroundColor, XtRPixel, sizeof(Pixel),
      Offset(foreground_color), XtRString, "LimeGreen"},
  { XtNbackgroundColor, XtCBackgroundColor, XtRPixel, sizeof(Pixel),
      Offset(background_color), XtRString, "Grey"},
  { XtNforegroundColorClip, XtCForegroundColorClip, XtRPixel, sizeof(Pixel),
      Offset(foreground_color_clip), XtRString, "Red"},
  { XtNbackgroundColorClip, XtCBackgroundColorClip, XtRPixel, sizeof(Pixel),
      Offset(background_color_clip), XtRString, "black"},
};

#undef Offset

static void Initialize(), Destroy();
static Boolean SetValues();
static void paint_rect();

XeBarClassRec XebarClassRec = {
  /* CoreClassPart */
  {
    (WidgetClass) &widgetClassRec,      /* superclass */
    "Bar",                              /* class name */
    sizeof(XeBarRec),                   /* widget_size */
    NULL,                               /* class_initialize */
    NULL,                               /* class_part_initialize */
    FALSE,                              /* class_inited */
    Initialize,                         /* initialize */
    NULL,                               /* initialize_hook */
    XtInheritRealize,                   /* realize */
    NULL,                               /* actions */
    0,                                  /* num_actions */
    resources,                          /* resources */
    XtNumber(resources),                /* num_resources */
    NULLQUARK,                          /* xrm_class */
    TRUE,                               /* compress_motion */
    TRUE,                               /* compress_exposure *//*??????????? */
    TRUE,                               /* compress_interleave */
    TRUE,                               /* visible_interest *//*?????????????*/
    Destroy,                            /* destroy */
    NULL,                               /* resize *//*?????*/
    NULL,                               /* expose */
    SetValues,                          /* set_values */
    NULL,                               /* set_value_hook */
    XtInheritSetValuesAlmost,           /* set_values_almost */
    NULL,                               /* get_values_hook */
    NULL,                               /* accept_focus */
    XtVersion,                          /* version */
    NULL,                               /* callback private */
    NULL,                               /* tm_table */
    NULL,                               /* query_geometry */
    NULL,                               /* display_accelerator */
    NULL,                               /* extension */
  },
  /* primitive class fields *//* See Vol. 4., Oreilly's serie */
  {
    _XtInherit,                         /* border_highlight */
    _XtInherit,                         /* border_unhighlight */
    XtInheritTranslations,              /* translation */
    NULL,                               /* arm_and_activeate */
    NULL,                               /* syn resources */
    0,                                  /* num_syn_resources */
    NULL,                               /* extension */
  },
  /* bar class fields */
  {
    0,                                   /* ignore */
  }
};

WidgetClass XebarWidgetClass = (WidgetClass) &XebarClassRec;

int bartop_pix;

static void Initialize(request, new, args, num_args)
     Widget request, new;
     ArgList args;
     Cardinal *num_args;
{
  XeBarWidget gw = (XeBarWidget) new;
  int i;
  XGCValues gcvalues;
  XtGCMask gcvalueMask;
  Window root = RootWindowOfScreen(XtScreen(new));
  Display *dpy = XtDisplay(new);

  /* make sure window size is not zero */

  if( request->core.width == 0)
    new->core.width = gw->bar.default_bar_width;

  if( request->core.height == 0)
   new->core.height = gw->bar.height;

  /* actual bar height */
  gw->bar.height = (int) (0.8 *  new->core.height);
  gw->bar.default_bar_width = new->core.width;
  bartop_pix = new->core.height - gw->bar.height;
  gw->bar.scale = ((float) gw->bar.height) / MAX_DB;

  /* make sure the min & max settings are valid */
  if(gw->bar.min_value >= gw->bar.max_value){
    XtWarning("Maximum must be greater than Minimum");
    gw->bar.min_value = gw->bar.max_value - 1;
  }

  /* allocate data array */
  gw->bar.values = (float *) XtCalloc(gw->bar.num_entries, sizeof(float));
  gw->bar.old_values = (float *) XtCalloc(gw->bar.num_entries, sizeof(float));
  gw->bar.old_values[0] = -1;
  gw->bar.last_maxstate_linear = (float *) XtCalloc(gw->bar.num_entries, sizeof(float));
  gw->bar.last_maxstate_linear[0] = -1;
  gw->bar.inc = (int *) XtCalloc(gw->bar.num_entries, sizeof(int));
  gw->bar.inc[0] = 0;
/*  gw->bar.intv = (float *) XtCalloc(1, sizeof(float));*/
  gw->bar.last_state = (float *) XtCalloc(gw->bar.num_entries, sizeof(float));
  gw->bar.last_state[0] = -1;
  
/* Vol.1, p194 Oreiley's Serie for Monochrmome color defaults */

  /* create GC */
  gcvalueMask = GCForeground | GCLineWidth;

  gcvalues.foreground = gw->bar.foreground_color;
  gcvalues.line_width = 3;
  gw->bar.bar_GC = XCreateGC(dpy, root, gcvalueMask, &gcvalues);

  gcvalueMask = GCForeground;
  gcvalues.foreground = gw->bar.background_color;
  gcvalues.background = gw->bar.foreground_color;
  gw->bar.inverse_bar_GC = XCreateGC(dpy, root, gcvalueMask, &gcvalues);

  gcvalues.foreground = gw->bar.foreground_color_clip;
  gw->bar.clip_GC = XCreateGC(dpy, root, gcvalueMask, &gcvalues);

  gcvalues.foreground = gw->bar.background_color_clip;
  gw->bar.inverse_clip_GC = XCreateGC(dpy, root, gcvalueMask, &gcvalues);

  /* set up scaling factor for pixel space */
  
}

static void Destroy(w)
     Widget w;
{
  XeBarWidget gw = (XeBarWidget) w;
  XtFree((char *) gw->bar.values);
}


#define ln10 2.302585093

static Boolean SetValues(old, req, new, args, num_args)
     Widget old, req, new;
     ArgList args;
     Cardinal *num_args;
{
  XeBarWidget oldgw = (XeBarWidget) old;
  XeBarWidget newgw = (XeBarWidget) new;
  int newpix, oldpix;
  int last_state_pix;

  newpix = (newgw->bar.values[0] <= MAX_DB) ? 
    (int) (newgw->bar.values[0] * newgw->bar.scale) : newgw->bar.height ;
  oldpix = (int) (newgw->bar.old_values[0] * newgw->bar.scale);
  last_state_pix = (int) (newgw->bar.last_state[0] * newgw->bar.scale);

  /* draw the bar */
  if( oldpix > newpix) {
    paint_rect(new, bartop_pix, (newgw->bar.height - newpix), 1);
  }
  if( oldpix < newpix){
    paint_rect(new, (bartop_pix + newgw->bar.height - newpix), newpix, 0);
  }
  if( oldpix == newpix ){       
    paint_rect(new, bartop_pix, (newgw->bar.height - newpix), 1);
    paint_rect(new, (bartop_pix + newgw->bar.height - newpix), newpix, 0);
  }    
  
  /* clipping and draw red region, or erase the red*/
  if( newpix >= newgw->bar.height )
    XFillRectangle (XtDisplay(new), XtWindow(new),
		    newgw->bar.clip_GC,
		    0, 0,
		    newgw->bar.default_bar_width,
		    bartop_pix);
  else
    if( oldpix < newgw->bar.height)
      XFillRectangle (XtDisplay(new), XtWindow(new),
		      newgw->bar.inverse_clip_GC,
		      0, 0,
		      newgw->bar.default_bar_width,
		      bartop_pix);

  /* draw the line - last state */
  if( newpix >= last_state_pix ){ 
    newgw->bar.last_maxstate_linear[0] = exp (newgw->bar.values[0] * ln10 / 20.);
    newgw->bar.inc[0] = 0;
  }
  else{
    if( newpix > oldpix )
      paint_rect(new, bartop_pix, (newgw->bar.height - newpix), 1);
    XDrawLine( XtDisplay(new), XtWindow(new), newgw->bar.bar_GC,
	      0, (bartop_pix+ newgw->bar.height - last_state_pix), 
	      newgw->bar.default_bar_width,  
	      (bartop_pix+ newgw->bar.height - last_state_pix));
  }

  newgw->bar.last_state[0] = 20 * log10( exp( newgw->bar.constRC[0] * newgw->bar.inc[0] * newgw->bar.intv[0]) * newgw->bar.last_maxstate_linear[0]);
  newgw->bar.inc[0]++;
  
  newgw->bar.old_values[0] = newgw->bar.values[0];
  return(FALSE); /* if background color has changed, generate the GC */ 
}       



static void paint_rect(wd, y, h, inv)
     Widget wd;
     int y, h, inv;
{
  XeBarWidget w = (XeBarWidget) wd;
  if(inv)
    XFillRectangle(XtDisplay(w), XtWindow(w),
		   w->bar.inverse_bar_GC, 
		   0, y,
		   w->bar.default_bar_width,
		   h);
  else
    XFillRectangle(XtDisplay(w), XtWindow(w),
		   w->bar.bar_GC, 
		   0, y,
		   w->bar.default_bar_width,
		   h);
}
       

