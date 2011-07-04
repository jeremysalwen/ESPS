/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)Objects.h	1.15 12/6/95 ATT/ERL/ESI */
/* Objects.h */
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
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/file.h>
#ifdef OS5
#include <sys/fcntl.h>
#endif

#ifdef FOR_XVIEW
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/text.h>
#include <xview/cursor.h>
#include <xview/svrimage.h>
#include <xview/icon.h>
#include <xview/cms.h>
#include <string.h>
#include <wave_colors.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#define W_WAVEFORM_COLOR (cmap_size - 5)
#define TRUE 1
#define FALSE 0
#endif

#define MAX_CMAP_SIZE 128

#include <reticle.h>
#include <Methods.h>
#include <Utils.h>
#include <Signals.h>
#include <assert.h>
#include <w_lengths.h>

/* for job control of external ESPS calls */
#define FOREGROUND 0
#define BACKGROUND 1

#define BIG_TIME 1.0e19		/* effectively infinity for signals */
#define OUTDATED 300000


/* #define MAX_BUFF_SIZE (32000000) *//* max buffer size for signal data */
/* replaced by global variable max_buff_bytes (see globals.c) */

#define Max(a,b) (((a) > (b)) ? (a) : (b))
#define Min(a,b) (((a) < (b)) ? (a) : (b))

/* Definitions for bits in the "options" control word */
#define BW_PLOTS             0x01
#define REPAINT_ON_RELEASE   0x04
#define DONT_SAVE_SPGM       0x08
#define SAVE_AFTER_EDIT      0x10
#define ENABLE_CURSOR_HACK   0x100

static char the_obvious_one[]=
"Copyright(C) 1986-1993 AT&T,\nCopyright(C) 1993 Entropic Research Laboratory, Inc., All rights reserved.";

/*
 * a structure for segmentation marks, etc.
 */
typedef struct mark {
  double time;
  int color;
  char *label;
  struct mark *next;
} Marks;

/* A handy list element for generating homebrew menus. */
typedef struct m_list {
  char *str;			/* points to the "menu" text string */
  int active;			/* a flag indicating top of displayed part */
  int tapped;			/* gen. purpose flag */
  struct m_list *prev;
  struct m_list *next;
  caddr_t data;		/* any data structure pointer you choose! */
  } Menu_list;

/*
 * Object == linked list of related signals.
 */

typedef struct object {
  char *name;			/* The name by which the object is addressed */
  Signal *signals;		/* head of a linked list of signals */
  Methods *methods;		/* head of a linked list of common methods  */
  Marks *marks;
  int zoom_ganged;		/* all objects zoom together */
  int scroll_ganged;		/* all objects scroll together */
  int edit_ganged;		/* all objects edited simultaneously */
  int dont_save_sgrams;		/* disables saving of spectrograms */
  struct object *next;		/* pointer to next object */
} Object;

/*
 * View == linked list of display data on a given signal;
 * sig->views contains a pointer to the head of the list.
 *
 * Each view may be a different graphical representation of the
 * same signal data.  Views come in different types.  Fields common
 * to all views are in this structure.  Additional data may be
 * found in additional structures pointed to by the pointer ``extra''.
 * Time, frequency, and amplitude are common enough to warrant inclusion
 * of their associated variables in the common structure.
 */

/* device-dependent plotting constants (could be made global variables...) */
#define PIX_PER_CM 32.582
#define SCREEN_WIDTH 1152
#define SCREEN_HEIGHT 900

/*
 * Under SunView, the frame rectangle is the outer rectangle of the frame,
 * and the drawing area is smaller by 5 pixels on 2 sides and the bottom
 * and 20 pixels on top.
 * Under XView, the frame rectangle is the inner rectangle of the frame,
 * and the frame is window-manager territory.  For xwaves windows, the
 * canvas rectangle coincides with the frame rectangle, but the drawing
 * area (the canvas paint window) is smaller by a 1-pixel border on all
 * 4 sides.
 */
#ifdef FOR_SUNVIEW
#define FRAME_MARGIN 5
#define FRAME_HEADER 20
#define WMGR_MARGIN 0
#define WMGR_HEADER 0
#else
#define FRAME_MARGIN 1
#define FRAME_HEADER 1
/* Window-manager dependent numbers---should be made variables.
 * These are for olwm.
 */
#define WMGR_MARGIN 5
#define WMGR_HEADER 26
#endif

#define MAX_CANVAS_WIDTH (SCREEN_WIDTH - 2*WMGR_MARGIN - 2*FRAME_MARGIN)
#define MAX_CANVAS_HEIGHT \
    (SCREEN_HEIGHT - WMGR_HEADER - WMGR_MARGIN - FRAME_HEADER - FRAME_MARGIN)

/* A homebrew scrollbar to be used for scrolling waveforms, etc.
   (because Sun didn't provide a generally useful one... ) */
typedef struct s_bar {
  int x,			/* left end (in canvas coords) */
      y,			/* top */
      width,			/* horiz. dimension */
      height,			/* vertical dimension */
      is_on,			/* enable/disable boolean */
      color,			/* color re the waves colormap */
      fresh;			/* TRUE if it has just been replotted */
  struct view *view;			/* host view */
} S_bar;

#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
typedef Canvas	canvas_t;
#else
typedef caddr_t	canvas_t;
#endif

/* The Menuop structure relates text items to procedure calls and data. */
typedef struct menuop {
  char *name;
  void (*proc)();
  void *data;
  struct menuop *next;
} Menuop;

/* A named list of Menuop lists. */
typedef struct moplist {
  char *name;
  Menuop *first_op;
  Menuop *reserve_ops;
  struct moplist* next;
} Moplist;

typedef struct view {
  canvas_t canvas;		/* place where view is displayed */
  Signal *sig;			/* contains data to display */
  struct view *next;

  double start_time,		/* coordinate bounds */
         start_yval, end_yval;
  double cursor_time, cursor_yval;	/* cursor coordinates */
  double lmarker_time, rmarker_time;	/* left & right time region markers */
  double tmarker_yval, bmarker_yval;	/* top & bottom y region markers */

  int *x_offset,		/* display translation (pixels) */
      *y_offset,
      *z_offset;
  double *x_scale,		/* scale factors (e.g. sec/cm) */
         *y_scale,
         *z_scale;
  double *val_scale,		/* scale and translate numeric readouts */
         *val_offset;
  double *plot_max,		/* display range for each channel */
         *plot_min;
  double cross_level;		/* level for level-crossing cursor moves */
  double zoom_ratio,	        /* for "zoom in" and "zoom out" */
  page_step;			/* for"page" operations */
  int dims,			/* number of dimensions to plot */
  background,			/* for waveforms */
  invert_dither,		/* for spectrograms */
  overlay_as_number,		/* for formant overlays on spectrograms */
  redraw_on_release,		/* when editing signals with mouse */
  rewrite_after_edit,		/* write to file after mouse release */
  spect_interp,			/* interpolate between sgram. data points */
  h_rescale,			/* stretch to fill for spectrograms */
  rescale_scope,		/* rescale just displayed part, or all */
  shorten_header;		/* display whole file name */
  int *v_rescale,		/* per element switches for: auto rescale */
      *show_vals,		/* numeric value cursor readout */
      *show_labels,		/* symbolic channel name printout */
      *reticle_on,		/* enable a reticle */
      *elements,		/* indices of channels to plot */
      *colors,			/* color of displayed data elements */
      *line_types;		/* line types for connecting consec. elems. */
  Reticle **ret;		/* reticles for each element */
  S_bar *scrollbar;		/* homebrew scrollbar */
  int plotted,			/* view has been plotted at least once */
      readout_height,		/* height of area devoted to numerics */
      overlay_n,		/* overlay number (if an overlay) */
      cursor_channel,           /* channel closest to cursor. */
      tmarker_chan,		/* displayed data chan closest to top */
      bmarker_chan,		/* displayed data chan closest to bottom */
      find_crossing,		/* enable/disable level-crossing cursor moves */
      show_current_chan;	/* highlight current channel samp value */
  int width, height;		/* canvas dims. for repaint/resize proc. */
  Menuop *left_but_proc,	/* procedures called on button events */
         *mid_but_proc,		/* middle */
         *move_proc;		/* MS_MOVE */
  char *mark_reference;		/* symbol name of time for finding marks */
  void (*right_but_proc)(),	/* right */
       (*handle_scrollbar)();	/* perform scrollbar operations on view */
  int (*data_plot)(),		/* waveform (or whatever) */
      (*cursor_plot)(),		/* cursor */
      (*vmarker_plot)(),		/* vertical marker(s) */
      (*hmarker_plot)(),		/* horizontal marker(s) */
      (*reticle_plot)(),	/* reticle(s) */
      (*x_print)(),		/* display time of cursor location */
      (*y_print)(),		/* channel value(s) at cursor */
      (*set_scale)(),		/* set scales and offsets */
      (*time_to_x)(),		/* change time to x position */
      (*yval_to_y)(),		/* change frequency to y position */
      (*xy_to_chan)();	/* x,y pixel location to closest channel */
  double (*x_to_time)(),	/* change x position to time */
         (*y_to_yval)();	/* change y position to frequency */
  caddr_t extra;		/* ptr to "extra" info e.g. a structure */
  int extra_type;		/* type of extra info (if any) */
  int (*free_extra)();		/* releases extra storage space */
} View;

/* This macro may be used to determine the time corresponding to the right
   edge of the canvas (end time, or ET).  It currently assumes that the
   right reticle margin (if any) has zero width.  It should probably be
   redefined to call a private method for each distinct data type. */
#define ET(v) \
    ((v)->start_time + (*(v)->x_scale * \
			(v->width - *(v)->x_offset)/PIX_PER_CM))

/* View Types */

#define VIEW_STANDARD		0x0
#define VIEW_BITMAP		0x1
#define VIEW_OVERLAY            0x10
/*
 * Scale Types
 */

#define FIXED_SCALE	0x0
#define VARIABLE_SCALE  0x1

/*
 * View ``extra'' Structures
 */

typedef struct struct_ViewBitmap {
  int	    scale_type;
  int	    depth, height, width;
  int	    maxval;
  int	    start_samp;
  double    start_time, start_yval, x_scale, y_scale;
  char	    *bitmap;
} ViewBitmap;

/* Pending_input provides a structure for deferring filename completion
   for panel text item input until a selection from a list of alternatives
   is made. */
typedef struct pending_input {
   char *results_to;		/* target array to be filled */
   char *path_prefix;		/* the pathname to be completed */
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
   Panel_item item;	 /* the text item, if any, to be updated *//*SUNVIEW*/
#else
   caddr_t item;	 /* the text item, if any, to be updated */
#endif
   caddr_t list;		/* pointer to alternatives list */
   canvas_t canvas;		/* alternatives display window */
   void (*proc)();	     /*  to be called with item and full name as arg.*/
   int destroy_on_select;	/* boolean */
   char *banner;		/* optional character string for label */
   struct pending_input *next;
   struct pending_input *prev;
   } Pending_input;

/*
 * Extern definitions (at end, so types are all defined)
 */

extern int do_color;


#ifdef FOR_SUNVIEW
extern u_char red[MAX_CMAP_SIZE], blue[MAX_CMAP_SIZE], green[MAX_CMAP_SIZE];
extern struct singlecolor ffg, fbg;
#endif
#ifdef FOR_XVIEW
extern Xv_singlecolor  rgb[MAX_CMAP_SIZE];         /* colormap data */
extern Xv_singlecolor ffg, fbg;	
#endif
extern char *spect_params, cms_name[];

extern int cmap_size;

extern int  attachment;		/* Is this program an attachment? */

#if !defined(IBM_RS6000) && !defined(hpux) && !defined(DS3100) && !defined(DEC_ALPHA) && !defined(OS5) && !defined(LINUX_OR_MAC)
extern char *malloc(), *realloc();
#endif
extern char *meth_make_object(), *meth_quit();
extern char *new_ext(), *view_get_value();
extern int read_data(), write_data();
extern int print_x(), print_y(), print_spect_y();
extern int plot_spectrogram(), plot_spect_cursors();
extern int plot_cursors(), plot_markers(), plot_reticles();
extern Header *get_header();
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
extern Notify_value kill_signal_view();/*SUNVIEW*/
#endif
extern Object *new_object();
extern Signal *new_signal(), *get_signal();
extern View *new_view(), *new_waves_view(), *setup_view(),
       *new_spect_view(), *find_view();
extern View *new_spect_display();
extern void display_view();
extern Reticle *new_spect_reticle(), *new_wave_reticle();
extern Bound *reticle_get_margins();
extern int draw_reticle();
extern void pr_draw_reticle(), free_reticle();
#if defined(FOR_SUNVIEW) || defined(FOR_XVIEW)
extern Canvas new_spect_window();/*SUNVIEW*/
#endif
extern double generic_x_to_time(), nonlin_y_to_yval(), generic_y_to_yval();
extern int generic_time_to_x(), generic_yval_to_y(), generic_xy_to_chan(),
           generic_x_to_index(), scale_for_canvas();
extern void redoit();
extern void free_spectrogram();

#define DSP32_FAB2 1
#define DSP32C_VME 2
#define DSP_SGI    3

extern char notice_msg[];
