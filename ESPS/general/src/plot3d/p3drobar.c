/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1990 Entropic Speech, Inc.  All rights reserved."   |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: plot3d							|
|  Module: p3drobar.c							|
|									|
|  Digital readout bar at top of window.				|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)p3drobar.c	1.5	8/6/91	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include "plot3d.h"

extern int	    debug_level;
extern int          force_monochrome_plot;

static void	    init_robar();
static void	    clear_window();
static void	    draw_text();
static void	    repaint();

static void	    do_cursor();
static void	    draw_cursor();
static void	    set_cursor_loc();

void		    send_waves_cursor();
extern int          send_to_waves;

static Canvas	    readout_bar;
static Xv_Window    paintwin;
static Display	    *display;
static Window	    xwin;
static GC	    gc;
static char	    readout_text[250];

static int	    want_cursor;
static double	    cursx, cursy, cursz;
static char	    *x_name = "record";
static char	    *y_name = "item";


void
init_readout_bar(cms)
    Cms		    cms;
{
    extern Frame    canvas_frame;
    extern int	    do_color;

    readout_bar = xv_create(canvas_frame, CANVAS,
			XV_X,			    0,
			XV_Y,			    0,
			XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		    ROBAR_HEIGHT,
			CANVAS_AUTO_EXPAND,	    TRUE,
			CANVAS_AUTO_SHRINK,	    TRUE,
			CANVAS_X_PAINT_WINDOW,	    TRUE,
			0);
			    
    if (do_color)
	xv_set(canvas_paint_window(readout_bar),
			WIN_CMS,		    cms,
			WIN_FOREGROUND_COLOR,	    COL_ROBAR_FG,
			WIN_BACKGROUND_COLOR,	    COL_ROBAR_BG,
			0);

    xv_set(readout_bar,
			CANVAS_REPAINT_PROC,	    init_robar,
			0);
}


static void
init_robar(canv, pw, disp, xw, xrects)
    Canvas	    canv;
    Xv_Window	    pw;
    Display	    *disp;
    Window	    xw;
    Xv_xrectlist    *xrects;
{
    XGCValues	gc_val;
    int		w, h;

    if (debug_level)
	Fprintf(stderr, "init_robar\n");

    paintwin = pw;
    display = disp;
    xwin = xw;
    gc_val.function = GXcopy;
    gc = XCreateGC(display, xwin, GCFunction, &gc_val);
    XCopyGC(display, DefaultGC(display, DefaultScreen(display)),
	    GCForeground|GCBackground, gc);

    xv_set(readout_bar, 
			CANVAS_REPAINT_PROC,	    repaint,
			0);

/*!*//* Should be uncommented; but XView bug causes extra
		  repaints on startup. */
/*     repaint(canv, pw, disp, xw, xrects); */
}


static void
repaint(canv, pw, disp, xw, xrects)
    Canvas	    canv;
    Xv_Window	    pw;
    Display	    *disp;
    Window	    xw;
    Xv_xrectlist    *xrects;
{
    if (debug_level)
	Fprintf(stderr, "repaint readout bar\n");

    clear_window();
    draw_text();
}


void
clear_readouts()
{
    if (debug_level)
	fprintf(stderr, "clear_readouts\n");

    clear_window();
    readout_text[0] = '\0';
    do_cursor();
    want_cursor = NO;
}


void
post_readouts(x, y, z)
    double  x, y, z;
{
    if (debug_level >= 3)
	fprintf(stderr, "x %15g  y %15g  z %15g\n", x, y, z);

    sprintf(readout_text, "%s %-15g  %s %-15g  %s %-15g",
	    x_name, x, y_name, y, "value", z);
    if (send_to_waves)
	send_waves_cursor(x,y);
    draw_text();
    do_cursor();
    set_cursor_loc(x, y, z);
    want_cursor = YES;
    do_cursor();
}


static void
clear_window()
{
    XClearWindow(display, xwin);
}


static void
draw_text()
{
    XDrawImageString(display, xwin, gc,
		     10, 10, readout_text, strlen(readout_text));
}


static void
set_cursor_loc(x, y, z)
    double  x, y, z;
{
    cursx = x;
    cursy = y;
    cursz = z;
}


void
set_xname(n)
    char    *n;
{
    x_name = n;
}


void
set_yname(n)
    char    *n;
{
    y_name = n;
}


static void
do_cursor()
{
    if (want_cursor) draw_cursor();
}


static void
draw_cursor()
{
    extern void	get_axis_lims();
    extern void	drawing_style();
    extern void splotline();
    double	xlo, xhi, ylo, yhi, zlo, zhi;

    get_axis_lims(&xlo, &xhi, &ylo, &yhi, &zlo, &zhi);
    drawing_style(BFN_XOR, GET_COL_BOX_FG, LNS_SOLID);
    splotline(xlo, cursy, zlo, xhi, cursy, zlo);
    splotline(cursx, ylo, zlo, cursx, yhi, zlo);
    splotline(cursx, cursy, zlo, cursx, cursy, cursz);
}

