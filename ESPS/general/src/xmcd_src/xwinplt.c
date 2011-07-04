/*
|									
|   This material contains proprietary software of Entropic Speech,
|   Inc.  Any reproduction, distribution, or publication without the
|   prior written permission of Entropic Speech, Inc. is strictly
|   prohibited.  Any public distribution of copies of this work	
|   authorized in writing by Entropic Speech, Inc. must bear the
|   notice						
|						
|   "Copyright (c) 1987,1988 Entropic Speech, Inc. All rights reserved."
|								
|						
|   xwinplt.c - Unix plotting functions for X windows
|						
|   Rodney W. Johnson, Entropic Speech, Inc.
|   The window plot code came from Rob Jacob at the Naval Research Lab.
|   Modified for SunViews by Alan Parker.  Range functions added by Parker.
|   Modified for X Windows by Ajaipal S. Virdy, Neural Systems Lab.
|
*/
#ifdef SCCS
static char    *sccs_id = "%W% %G% ESI";
#endif

/* #include <sys/file.h> */

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
/* #include <X11/Label.h> */

#include "../include/Xw/Xw.h"
#include "../include/Xw/SText.h"
#include "../include/Xw/VPW.h"
#include "../include/Xw/RCManager.h"
#include "../include/Xw/PButton.h"
#include "../include/Xw/Toggle.h"
#include "../include/Xw/Form.h"
#include "../include/Xw/WorkSpace.h"

#define PI	3.14159265358979323846
#define NCIRC	120		/* Normal circles */
#define NARC	50		/* Number of segments in arc, if it were 360
				 * deg. */
#define NONE	0
#define RIGHT	1
#define MIDDLE	2
#define LEFT	4
#define SPACES \
"                                        \
                                        "	/* 80 blanks */
#define HSPACES \
"                                       "	/* about 40 spaces */

/* WARNING these numbers are from drawbox.c in plotsd */

#define BOX_START 500
#define BOX_END 5000
#define BOX_TOP 500+70
#define BOX_BOTTOM 3000-70

/* X Window related define's for determining dimensions of a widget */

#define	DEFAULT_WIDTH	500
#define	DEFAULT_HEIGHT	500

/* G L O B A L
 *  V A R I A B L E S
 *   R E F E R E N C E D
 */

extern char    *ProgName;
extern int	debug;

extern	char	*title;
extern	int	R_flag;
extern	int	P_flag;
extern	char	*icon_name;
extern  char	*play_options;

extern	Widget	toplevel;

/* L O C A L
 *  V A R I A B L E S
 */

char		*filename;

static int      width, height;
static int      xorig, yorig;

static int      xpos = 0, ypos = 0;
static int      xlow = 0, ylow = 0, xhigh = 1000, yhigh = 1000;

int             beginplot;	/* read from ESPS Common file */
int             endplot;	/* read from ESPS Common file */
int             points;

int             new_start;	/* new start point selected */
int             new_nan;	/* new nan point selected */

int             box_start;
int             box_end;
int		box_top;
int		box_bottom;

int		mark_x=0;
int		cursor_x=0;

int             range_state = 1;
int             x_start;
int             x;
int		first_paint = 1;

Pixmap		make_icon();

void 		save_common(), get_common();
void 		s_text(), s_imagetext();
void		play_file(), play_range(), zoom_in(), zoom_out();
void		quit_proc();
void		do_plot();
#ifndef HP
char		*sprintf(), *strcpy(), *strcat();
#endif
extern 	char 	*tmpname;
int		played=0;
char		range_arg[100];
char		buf[512];	/* general purpose buffer */

static	char	*msg1 = "LEFT: select start of range, MIDDLE: play entire plot, RIGHT: zoom out\n";

static	char	*msg1a = "LEFT: select start of range, MIDDLE: <nothing>, RIGHT: zoom out\n";

static	char	*msg2 = "LEFT: zoom in, MIDDLE: play range, RIGHT: reselect range\n";

static	char	*msg3 = "Saving range...\n";

static	char	*msg4 = "LEFT: zoom in, MIDDLE: save range, RIGHT: reselect range\n";
int	text_offset=0;
static	char	*msg1_ptr;

static	void	repaint_canvas();

static int
scalex(x)
	int             x;
{
	return (xorig + width * (x - xlow) / (xhigh - xlow));
}

static int
scaley(y)
	int             y;
{
	return (yorig + height * (yhigh - y) / (yhigh - ylow)) + text_offset;
}



Window		MCWindow;
GC		mc_gc;
XFontStruct	*font_info;
Display		*display;
int		screen;
int		WidgetsCreated = 0;

Cursor		MousePointer;
int		Texth;
int		Button1Down;

static int	LeftMark_x = 0;
static int	RightMark_x = 0;
static int	RangeShown = 0;
static char	RangeString[100];
static int	lines_drawn = 0;

/* G L O B A L
 *  W I D G E T S
 */

Widget		outer;
Widget		PrevW, QuitW, SRW, PRW, RRW, FormW, WSW, Form2W, ZoomInW;
Widget		SaveW, RefreshW;
Widget		ZoomOutW;

#define NONE 0
#define SELECT_RANGE 1
#define PLAY_RANGE 2

int	OptionSelected = NONE;

void SelectRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    Arg		arg[5];
    Cardinal	i;

    if (OptionSelected == PLAY_RANGE) {
	i = 0;
	XtSetArg (arg[i], XtNset, False);  i++;
	XtSetValues ((Widget) PRW, arg, (Cardinal) i);
    }
    OptionSelected = SELECT_RANGE;

    /* Load in hand as the mouse pointer font */

    MousePointer = XCreateFontCursor (display, XC_hand2);
    XDefineCursor (display, MCWindow, MousePointer);
}

void PlayRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    Arg		arg[5];
    Cardinal	i;

    fprintf (stderr, "Play Range\n");

    if (OptionSelected == SELECT_RANGE) {
	i = 0;
	XtSetArg (arg[i], XtNset, False);  i++;
	XtSetValues ((Widget) SRW, arg, (Cardinal) i);
    }
    OptionSelected = PLAY_RANGE;

    /* Load in watch to indicate D/A progress during play mode */

    MousePointer = XCreateFontCursor (display, XC_watch);
    XDefineCursor (display, MCWindow, MousePointer);
}


void Quit(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    (void) unlink (tmpname);
    exit (0);
}


void Refresh(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    /* Load in watch mouse pointer during screen refresh */

    MousePointer = XCreateFontCursor (display, XC_watch);
    XDefineCursor (display, MCWindow, MousePointer);

    if (RangeShown)
	ClearRange ();
    erase ();
    repaint_canvas ();

    MousePointer = XCreateFontCursor (display, XC_top_left_arrow);
    XDefineCursor (display, MCWindow, MousePointer);
}


void ZoomIn(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    Arg		arg[1];
    Cardinal	i;

    if (!RangeShown)
	return;

    ClearRange ();

    /* Load in watch to indicate D/A progress during play mode */

    MousePointer = XCreateFontCursor (display, XC_watch);
    XDefineCursor (display, MCWindow, MousePointer);

    fprintf (stderr, "Zooming in . . .\n");
    zoom_in ();

    i = 0;
    XtSetArg (arg[i], XtNset, False);  i++;
    XtSetValues ((Widget) ZoomInW, arg, (Cardinal) i);

    /* Load in watch to indicate D/A progress during play mode */

    MousePointer = XCreateFontCursor (display, XC_top_left_arrow);
    XDefineCursor (display, MCWindow, MousePointer);
}

void ZoomOut(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    Arg		arg[1];
    Cardinal	i;

    if (RangeShown)
	ClearRange ();

    /* Load in watch while waiting to zoom out */

    MousePointer = XCreateFontCursor (display, XC_watch);
    XDefineCursor (display, MCWindow, MousePointer);

    fprintf (stderr, "Zooming out . . .\n");
    zoom_out ();

    i = 0;
    XtSetArg (arg[i], XtNset, False);  i++;
    XtSetValues ((Widget) ZoomOutW, arg, (Cardinal) i);

    /* Load in watch to indicate D/A progress during play mode */

    MousePointer = XCreateFontCursor (display, XC_top_left_arrow);
    XDefineCursor (display, MCWindow, MousePointer);
}


void SaveRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    Arg		arg[1];
    Cardinal	i;
    fprintf (stderr, "Saving range . . .\n");
    save_common ();

    i = 0;
    XtSetArg (arg[i], XtNset, False);  i++;
    XtSetValues ((Widget) SaveW, arg, (Cardinal) i);
}

#define undraw_line(display,screen,win,x1,y1,x2,y2) \
	   draw_line(display, screen, win, x1, y1, x2, y2);

ClearRange()
{
    Arg		arg[5];
    Cardinal	i;

	/* clear the range and display "Show  Range" pb */
	s_imagetext (HSPACES, 5, 40);
	i = 0;
	XtSetArg (arg[i], XtNlabel, (caddr_t) "Show Range");  i++;
	XtSetValues ((Widget) RRW, arg, (Cardinal) i);
	RangeShown = 0;

	undraw_line (display, screen, MCWindow,
		     LeftMark_x, box_top,
		     LeftMark_x, box_bottom);
	undraw_line (display, screen, MCWindow,
		     RightMark_x, box_top,
		     RightMark_x, box_bottom);
	/* We don't need to do the following: */
	/* lines_drawn = 0;  */
}

ShowRange()
{
    Arg		arg[5];
    Cardinal	i;

	/* show the last selected range and "Clear Range" pb */
	s_imagetext (RangeString, 5, 40);
	i = 0;
	XtSetArg (arg[i], XtNlabel, (caddr_t) "Clear Range");  i++;
	XtSetValues ((Widget) RRW, arg, (Cardinal) i);
	RangeShown = 1;

	draw_line (display, screen, MCWindow,
		   LeftMark_x, box_top,
		   LeftMark_x, box_bottom);
	draw_line (display, screen, MCWindow,
		   RightMark_x, box_top,
		   RightMark_x, box_bottom);
	/* We don't need to do the following: */
	/* lines_drawn = 1;  */
}

void ReselectRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    if (RangeShown)
	ClearRange ();
    else
	ShowRange ();
}


void StartRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    char	*Module = "StartRange";

    Window	root, child;
    int		root_x, root_y;
    int		win_x, win_y;
    unsigned
	int	mask;

    XEvent	report;
    int		save_start;

    static int	first_time = 1;

    if (OptionSelected != SELECT_RANGE)
	return;

    if (RangeShown)
	return;

    fprintf( stderr, "Start range selected\n" );

    /* Load in left arrow as default mouse pointer */

    MousePointer = XCreateFontCursor (display, XC_sb_h_double_arrow);
    XDefineCursor (display, MCWindow, MousePointer);

    if (first_time) {
	XSelectInput (display, MCWindow,
		      ButtonPressMask | ButtonReleaseMask |
		      ExposureMask | PointerMotionMask);
	first_time = 0;
    }

    XQueryPointer (display, MCWindow, &root, &child, &root_x, &root_y,
		   &x_start, &win_y, &mask);

    save_start = new_start = beginplot + (points * (x_start - box_start)
					  / (box_end - box_start));
    LeftMark_x = mark_x = x_start;

    if (debug & 0x4)
	fprintf (stderr, "x_start = %d, win_y = %d;  new_start = %d\n",
		 x_start, win_y, new_start);

    draw_line (display, screen, MCWindow,
	       LeftMark_x, box_top,
	       LeftMark_x, box_bottom);
/*
    draw_line (display, screen, MCWindow,
	       x_start, box_top,
	       x_start, box_bottom);
*/
    lines_drawn = 0;

    sprintf (RangeString, "Range selected: %d ", new_start);
    s_imagetext (RangeString, 5, 40);

    if (debug & 0x1)
	fprintf (stderr, "%s: tracking the mouse . . .\n", Module);

    Button1Down = 1;

    while (Button1Down) {

	/* get rid of any other motion events still on queue */
	while (XCheckMaskEvent (display, PointerMotionMask, &report))
	    ;

	if (lines_drawn) {
	    undraw_line (display, screen, MCWindow,
			 x_start, box_top,
			 x_start, box_bottom);
	    /* lines_drawn = 0; */
	}

	XQueryPointer (display, MCWindow, &root, &child, &root_x, &root_y,
		       &x_start, &win_y, &mask);

	new_start = beginplot + (points * (x_start - box_start)
				 / (box_end - box_start));

	if (x_start >= box_start && x_start <= box_end) {
	    draw_line (display, screen, MCWindow,
		       x_start, box_top,
		       x_start, box_bottom);
	    lines_drawn = 1;
	}

	RightMark_x = mark_x = x_start;

	sprintf (RangeString, "Range selected: %d to %d", save_start, new_start);
	s_imagetext (RangeString, 5, 40);

	if (debug & 0x1)
	    fprintf (stderr, "%s: Range selected: %d to %d\n",
		     Module, save_start, new_start);
	
	XMaskEvent (display, ButtonReleaseMask | PointerMotionMask, &report);

	/* XtNextEvent (&report); */

	if (report.type == ButtonRelease)
	    break;

    }	/* end while */

    lines_drawn = 0;
    new_nan = new_start;
    new_start = save_start;

    {
	Arg		arg[1];
	Cardinal	i;

	/* Clear the "Select/Range Mode" toggle switch */
	i = 0;
	XtSetArg (arg[i], XtNset, False);  i++;
	XtSetValues ((Widget) SRW, arg, (Cardinal) i);

	OptionSelected = NONE;
	RangeShown = 1;

	/* Display "Clear Range" PushButton */
	i = 0;
	XtSetArg (arg[i], XtNlabel, (caddr_t) "Clear Range");  i++;
	XtSetValues ((Widget) RRW, arg, (Cardinal) i);
    }

    /* Load in default mouse pointer */

    MousePointer = XCreateFontCursor (display, XC_top_left_arrow);
    XDefineCursor (display, MCWindow, MousePointer);

}

void EndRange(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    if (OptionSelected != SELECT_RANGE)
	return;

    fprintf (stderr, "End range selected\n");
}

void DrawPlot(widget, closure, callData)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    caddr_t callData;		/* unused */
{
    char		*Module = "DrawPlot";
    XGCValues		values;
    XtGCMask		valuemask = 0;

    static int		count = 0;

    if (debug & 0x1)
	fprintf (stderr, "%s: Number of times WorkSpace Widget exposed:  %d\n",
		 Module, ++count);

    display = XtDisplay (widget);
    MCWindow = XtWindow (widget);

    /* Load in left arrow as default mouse pointer */

    MousePointer = XCreateFontCursor (display, XC_top_left_arrow);
    XDefineCursor (display, MCWindow, MousePointer);

    if (debug & 512)
	fprintf (stderr, "%s: create Graphics Context\n",
		 Module);

    mc_gc = XtGetGC (widget, valuemask, &values);

    repaint_canvas ();
}

init_device()
{
    char		*Module = "xmcd";

/*
    XGCValues		values;
    unsigned long	valuemask = 0;
*/

    int			MCWinWidth, MCWinHeight;

    Cardinal	i;
    Arg arg[10];

    static XtCallbackRec Quitcallbacks[] = { { Quit, NULL}, {NULL, NULL} };
    static XtCallbackRec SRcallbacks[] = { { SelectRange, NULL}, {NULL, NULL} };
    static XtCallbackRec PRcallbacks[] = { { PlayRange, NULL}, {NULL, NULL} };
    static XtCallbackRec Refreshcallbacks[] = { { Refresh, NULL}, {NULL, NULL} };

    static XtCallbackRec ZoomIncallbacks[] = { { ZoomIn, NULL}, {NULL, NULL} };
    static XtCallbackRec ZoomOutcallbacks[] = { { ZoomOut, NULL}, {NULL, NULL} };
    static XtCallbackRec Savecallbacks[] = { { SaveRange, NULL}, {NULL, NULL} };
    static XtCallbackRec ERcallbacks[] = { { ReselectRange, NULL}, {NULL, NULL} };

    static XtCallbackRec WSexposecallbacks[] = { { DrawPlot, NULL}, {NULL, NULL} };
    static XtCallbackRec WSselectcallbacks[] = { { StartRange, NULL}, {NULL, NULL} };
    static XtCallbackRec WSreleasecallbacks[] = { { EndRange, NULL}, {NULL, NULL} };

    if (debug & 0x1)
	fprintf (stderr, "%s: initialize device...\n",
		 Module);

/*    toplevel = XtInitialize (NULL, "XMcd", options, XtNumber(options),
 *			     &Argc, Argv);
 */

    display = XtDisplay (toplevel);
    screen = DefaultScreen (display);
    width = DisplayWidth (display, screen);
    height = DisplayHeight (display, screen);

    i = 0;
    XtSetArg( arg[i], XtNwidth, 200);  i++;
    outer = XtCreateManagedWidget ("Paned", XwvPanedWidgetClass, toplevel,
				   arg, i);
    /* XtAddCallback (outer, XtNdestroyCallback, Destroyed, NULL); */

    if (debug & 0x1)
	fprintf (stderr, "%s: screen = %d, width = %d, height = %d\n",
		 Module, screen, width, height);

    if ((font_info = XLoadQueryFont (display, "6x10")) == NULL) {
       if ((font_info = XLoadQueryFont (display, "6x13")) == NULL) {
	fprintf(stderr,"%s: can't load font.\n", Module);
	exit(1);
       }
    }

    Texth = font_info->max_bounds.ascent + font_info->max_bounds.descent;

    /*******************************/
    /* C R E A T E   W I D G E T S */
    /*******************************/


    /* Create Label Widget for displaying title */

    {
	int	LabelHeight = 12;

	i = 0;
	XtSetArg (arg[i], XtNlabel, title);  i++;

	/* The following is used to prevent the vPaned widget from displaying
	 * sashes (control widgets) in the bottom right corner of each pane.
	 */

	XtSetArg( arg[i], XtNmin, LabelHeight);  i++;
	XtSetArg( arg[i], XtNmax, LabelHeight);  i++;

	XtSetArg (arg[i], XtNbackground,
		  NameToPixel (display, screen, "blue",
			       WhitePixel (display, screen)));  i++;
	XtSetArg (arg[i], XtNforeground,
		  NameToPixel (display, screen, "white",
			       WhitePixel (display, screen)));  i++;

	/* XtCreateManagedWidget ("Title", labelWidgetClass, outer, arg, i); */
	XtCreateManagedWidget ("Title", XwstatictextWidgetClass, outer, arg, i);
    }

    /* Create a Form Widget for placing Toggle Widgets at proper place */

    {
	int	FormHeight = 20;

	i = 0;
	XtSetArg (arg[i], XtNwidgetType, XwWORK_SPACE);  i++;
	XtSetArg( arg[i], XtNmin, FormHeight);  i++;
	XtSetArg( arg[i], XtNmax, FormHeight);  i++;
	FormW = XtCreateManagedWidget ("Form", XwformWidgetClass, outer, arg, i);
    }

    {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Quit");  i++;
	XtSetArg (arg[i], XtNxRefWidget, FormW);  i++;
	XtSetArg (arg[i], XtNxResizable, True);	 i++;
	XtSetArg (arg[i], XtNyRefWidget, FormW);  i++;
	XtSetArg (arg[i], XtNyResizable, True);  i++;

	XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++;
	XtSetArg (arg[i], XtNrelease, Quitcallbacks);  i++;
	PrevW = QuitW = XtCreateManagedWidget ("Quit", XwpushButtonWidgetClass,
					       FormW, arg, i);
    }

    if (R_flag || P_flag) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Clear Range");  i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW);  i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++;
	/* XtSetArg (arg[i], XtNsquare, False);  i++; */
	XtSetArg (arg[i], XtNselect, ERcallbacks);  i++;
	PrevW = RRW = XtCreateManagedWidget ("RR", XwpushButtonWidgetClass,
					     FormW, arg, i);
    }

    if (P_flag) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Play selected range");  i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW);  i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxResizable, True);  i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	/* XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++; */
	XtSetArg (arg[i], XtNsquare, False);  i++;
	XtSetArg (arg[i], XtNselect, PRcallbacks);  i++;
	PrevW = PRW = XtCreateManagedWidget ("PR", XwtoggleWidgetClass, FormW, arg, i);
    }

    /* Create Refresh pushButton Widget */

    {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Refresh");  i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW);  i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxAttachRight, True);  i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++;
	XtSetArg (arg[i], XtNselect, Refreshcallbacks);  i++;
	RefreshW = XtCreateManagedWidget ("Refresh", XwpushButtonWidgetClass,
					  FormW, arg, i);
    }

    /* Create a Form Widget for placing Toggle Widgets at proper place */

    if (R_flag || P_flag) {
	int	FormHeight = 20;

	i = 0;
	XtSetArg (arg[i], XtNwidgetType, XwWORK_SPACE);  i++;
	XtSetArg( arg[i], XtNmin, FormHeight);  i++;
	XtSetArg( arg[i], XtNmax, FormHeight);  i++;
	Form2W = XtCreateManagedWidget ("Form2", XwformWidgetClass, outer, arg, i);
    }


    if (R_flag || P_flag) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Save Range"); i++;

	XtSetArg (arg[i], XtNxRefWidget, Form2W);  i++;
	XtSetArg (arg[i], XtNxResizable, True);	 i++;
	XtSetArg (arg[i], XtNyRefWidget, Form2W);  i++;
	XtSetArg (arg[i], XtNyResizable, True);  i++;

	XtSetArg (arg[i], XtNsquare, False);  i++;
	XtSetArg (arg[i], XtNselect, Savecallbacks);  i++;
	PrevW = SaveW = XtCreateManagedWidget ("Save", XwtoggleWidgetClass,
					     Form2W, arg, i);
    }


    if (R_flag || P_flag) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Select/Range Mode"); i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxResizable, True);	i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	/* XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++; */
	XtSetArg (arg[i], XtNsquare, False);  i++;
	XtSetArg (arg[i], XtNselect, SRcallbacks);  i++;
	PrevW = SRW = XtCreateManagedWidget ("SR", XwtoggleWidgetClass, Form2W, arg, i);
    }

    if (R_flag || P_flag ) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Zoom In");  i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	/* XtSetArg (arg[i], XtNxAttachRight, True);  i++; */
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	XtSetArg (arg[i], XtNsquare, False);  i++;
	XtSetArg (arg[i], XtNselect, ZoomIncallbacks);  i++;
	PrevW = ZoomInW = XtCreateManagedWidget ("ZoomIn", XwtoggleWidgetClass,
					       Form2W, arg, i);
    }

    if (R_flag || P_flag ) {
	i = 0;
	XtSetArg (arg[i], XtNlabel, "Zoom Out");  i++;

	XtSetArg (arg[i], XtNxRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNyRefWidget, (caddr_t) PrevW); i++;
	XtSetArg (arg[i], XtNxAttachRight, True);  i++;
	XtSetArg (arg[i], XtNxAddWidth, True);  i++;
	XtSetArg (arg[i], XtNxVaryOffset, True);  i++;

	XtSetArg (arg[i], XtNsquare, False);  i++;
	XtSetArg (arg[i], XtNselect, ZoomOutcallbacks);  i++;
	PrevW = ZoomOutW = XtCreateManagedWidget ("ZoomOut", XwtoggleWidgetClass,
					       Form2W, arg, i);
    }

    /* Create a WorkSpace Widget for drawing */

    i = 0;
    XtSetArg (arg[i], XtNexpose, WSexposecallbacks);  i++;
    XtSetArg (arg[i], XtNtraversalType, XwHIGHLIGHT_ENTER);  i++;
    XtSetArg (arg[i], XtNselect, WSselectcallbacks);  i++;
    XtSetArg (arg[i], XtNrelease, WSreleasecallbacks);  i++;
    XtSetArg (arg[i], XtNwidth, DEFAULT_WIDTH);  i++;
    XtSetArg (arg[i], XtNheight, DEFAULT_HEIGHT);  i++;
    WSW = XtCreateManagedWidget ("WorkSpace", XwworkSpaceWidgetClass, outer, arg, i);

    xorig = yorig = 1;

    if (debug & 0x1)
	fprintf (stderr, "%s: device initialized\n",
		 Module);

    /* OLD Method:
     * MCWindow = XCreateSimpleWindow (display, RootWindow (display, screen),
     *				       0, 0,
     *				       MCWinWidth, MCWinHeight,
     *				       1, BlackPixel (display, screen),
     *				       WhitePixel (display, screen));
     *
     * XSetStandardProperties (display, MCWindow, Module, Module,
     *			       None, NULL, 0, &szhint);
     * XMapWindow (display, MCWindow);
     */

/*   XSelectInput (display, MCWindow,
 *		  KeyPressMask | ButtonPressMask | ExposureMask | ResizeRedirectMask);
 */

    /* mc_gc = XCreateGC (display, MCWindow, valuemask, &values); */

}


erase()
{
    if (WidgetsCreated)
	XClearWindow (display, MCWindow);
    else
	return;
}

label(s)
	char            s[];
{

/* struct pr_size  size, pf_textwidth();
 *
 * pw_text(pw, xpos, ypos, PIX_SRC, SunFont, s);
 */

	XDrawString (display, MCWindow, mc_gc,
		     xpos, ypos,
		     s, strlen (s));

	/* size = pf_textwidth(strlen(s), SunFont, s); */
	xpos += XTextWidth (font_info, s, strlen (s));
}

line(x1, y1, x2, y2)
	int             x1, y1, x2, y2;
{
	move(x1, y1);
	cont(x2, y2);
}

circle(x, y, r)
	int             x, y, r;
{
	static          tblinited = 0;
	static double   sine[NCIRC], cosine[NCIRC];

	int             i, scaledx, scaledy, scaledr, oldx, oldy, newx, newy;

	if (!tblinited) {	/* cache of NCIRC sines and cosines */
		double          theta, incr;

		tblinited = 1;
		incr = (2 * PI) / NCIRC;
		for (theta = incr, i = 1; i < NCIRC; theta += incr, i++) {
			sine[i] = sin(theta);
			cosine[i] = cos(theta);
		}
	}
	scaledx = scalex(x);
	scaledy = scaley(y);
	scaledr =.5 + scalex(r);
	oldx = scaledx + scaledr;
	oldy = scaledy;
	XSetForeground(display, mc_gc, BlackPixel (display, screen));

	for (i = 1; i < NCIRC; i++) {
		newx = scaledx + scaledr * cosine[i];
		newy = scaledy + scaledr * sine[i];
		/* pw_vector(pw, oldx, oldy, newx, newy, PIX_SRC, 1); */
		XDrawLine (display, MCWindow, mc_gc,
			   oldx, oldy, newx, newy);
		oldx = newx;
		oldy = newy;
	}
	/* close up */
	/* pw_vector(pw, oldx, oldy, scaledx + scaledr, scaledy, PIX_SRC, 1); */
	XDrawLine (display, MCWindow, mc_gc,
		   oldx, oldy, scaledx + scaledr, scaledy);
}

static double
bear(x0, y0, x1, y1)
	int             x0, y0, x1, y1;
{
	return atan2((double) y1 - (double) y0, (double) x1 - (double) x0);
}

/*
 * like UNIX hypot, but takes integers as arguments; returns a double
 */
static double
myhypot(x, y)
	int             x, y;
{
	return sqrt((double) x * (double) x + (double) y * (double) y);
}

arc(x, y, x0, y0, x1, y1)
	int             x, y, x0, y0, x1, y1;
{
	double          theta0, theta1, theta, incr;
	int             r;

	r = (int) myhypot(x0 - x, y0 - y);
	theta0 = bear(x, y, x0, y0);
	theta1 = bear(x, y, x1, y1);
	if (theta0 >= theta1)
		theta1 += 2 * PI;

	incr = (2 * PI) / NARC;
	move((int) (x + r * cos(theta0)), (int) (y + r * sin(theta0)));
	for (theta = theta0; theta <= theta1; theta += incr)
		cont((int) (x + r * cos(theta)), (int) (y + r * sin(theta)));

	/* one extra to close up */
	cont((int) (x + r * cos(theta1)), (int) (y + r * sin(theta1)));	
}

move(x, y)
	int             x, y;
{
	xpos = scalex(x);
	ypos = scaley(y);
}

cont(x, y)
	int             x, y;
{
	int             oldx = xpos, oldy = ypos;
	move(x, y);
	/* pw_vector(pw, oldx, oldy, xpos, ypos, PIX_SRC, 1); */
	XSetForeground(display, mc_gc, BlackPixel (display, screen));
	XDrawLine (display, MCWindow, mc_gc,
		   oldx, oldy, xpos, ypos);
}

point(x, y)
	int             x, y;
{
	move(x, y);
	label(".");
}

linemod(s)
	char            s[];
{
}

space(x0, y0, x1, y1)
	int             x0, y0, x1, y1;
{
    char		*Module = "space";

    XWindowAttributes	windowattr;
    
    /* width = (int) window_get(canvas, CANVAS_WIDTH);
     * height = (int) window_get(canvas, CANVAS_HEIGHT)-text_offset;
     */

    if (debug & 0x4)
	fprintf (stderr, "%s: get window attributes\n", Module);

    if (WidgetsCreated) {
	XGetWindowAttributes (display, MCWindow, &windowattr);
	width = windowattr.width;
	height = windowattr.height;
    } else {
	width = DEFAULT_WIDTH;
	height = DEFAULT_HEIGHT;
    }

    if (debug & 0x4)
	fprintf (stderr, "%s: width = %d, height = %d\n",
		 Module, width, height);

    xlow = x0;
    ylow = y0;
    xhigh = x1;
    yhigh = y1;

    xorig = 1;
    yorig = 1;

}

static void
repaint_canvas()
{

	do_plot();
	box_start = scalex(BOX_START);
	box_end = box_start + scalex(BOX_END);
	box_top = scaley(-BOX_TOP);
	box_bottom = scaley(-BOX_BOTTOM);

	if (R_flag && first_paint) {
		s_text (msg1_ptr, 20, 20);	/* display help bar */
		get_common();
		first_paint = 0;
	}
 	mark_x = cursor_x = 0;
	range_state = 1;

	/* window_set(canvas, CANVAS_REPAINT_PROC, (int (*)()) NULL, 0); */
}


void
start_plot()
{
    char	*Module = "start_plot";
    XEvent	report;
    int		process_events = 1;
    XWMHints    hints;

    if (debug & 0x2)
	fprintf(stderr, "%s: in start plot. . .\n", Module);

    /* window_main_loop(frame); */

    /* wait until window is created before drawing */

    WidgetsCreated = 1;

    XtRealizeWidget(toplevel);

    if (debug & 0x2)
	fprintf(stderr, "%s: making icon.\n", Module);

    hints.flags = IconPixmapHint;
    hints.icon_pixmap = make_icon(display, screen, icon_name);
    XSetWMHints(display, XtWindow(toplevel), &hints);


    if (debug & 0x2)
	fprintf(stderr, "%s: starting main loop.\n", Module);

    XtMainLoop();

#ifdef COMMENT
    while (XNextEvent (display, &report))
	if (report.type == Expose)
	    break;

    do_plot ();

    if (debug & 0x2)
	fprintf (stderr, "%s: processing events . . .\n", Module);

    /*********************************/
    /*  P R O C E S S    E V E N T S */
    /*********************************/

    while (process_events) {

	XNextEvent (display, &report);

	switch (report.type) {
	    case ResizeRequest: {
		erase ();
		do_plot ();
		repaint_canvas ();
	    }
	    break;

	    case Expose: {
		do_plot ();
		repaint_canvas ();
	    }
	    break;
		
	    case KeyPress: {
		char	buffer[20];
		int		bufsize = 20;
		KeySym	key;
		XComposeStatus	compose;

		XLookupString (&report, buffer, bufsize, &key, &compose);
		if (key == XK_q || key == XK_Q)
		    process_events = 0;
	    }
	    break;

	    default:		/* NOT REACHED */
	    break;

	}			/* end switch (report.type) */
		    
    }				/* end while (process_events) */
	
    XCloseDisplay (display);

#endif COMMENT

}


void
s_text(s, x, y)
	char           *s;
	int             x, y;
{
/*
  pw_text(pw, x, y-10, PIX_SRC, SunFont, SPACES);
  pw_text(pw, x, y-10, PIX_SRC, SunFont, s);
 */
  if (!s) return;
  XDrawString (display, MCWindow, mc_gc,
	       x, y-10,
	       SPACES, strlen (SPACES));
  XDrawString (display, MCWindow, mc_gc,
	       x, y-10,
	       s, strlen (s));
}


void
s_imagetext(s, x, y)
  char *s;
  int  x, y;
{

    /* XDrawImageString (display, MCWindow, mc_gc,
     *	      x, y-10,
     *		      SPACES, strlen (SPACES));
     */

/*
	XSetForeground(display, mc_gc, BlackPixel (display, screen));
	XSetBackground(display, mc_gc, WhitePixel (display, screen));
*/
    XDrawImageString (display, MCWindow, mc_gc,
		      x, y-10,
		      s, strlen (s));
}


void
get_common()
{
	(void) read_params((char *) NULL, SC_CHECK_FILE, (char *) NULL);

	if (symtype("beginplot") == ST_UNDEF || symtype("endplot") == ST_UNDEF){
	  Fprintf(stderr, "mcd: no beginplot or endplot in Common file\n");
	  exit(1);
	}

	filename = getsym_s("filename");
	beginplot = getsym_i("beginplot");
	endplot = getsym_i("endplot");
	points = endplot - beginplot + 1;
}

void
save_common()
{
	if (putsym_s("filename", filename) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_i("start", new_start) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_i("nan", new_nan - new_start + 1) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_s("prog", "range") == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
}


void
play_file()
{

	(void)sprintf(range_arg,"-p%ld:+%ld ",beginplot,points);
	(void)strcpy(buf,"play ");
	(void)strcat(buf,range_arg);
	if (play_options)
		(void)strcat(buf,play_options);
	(void)strcat(buf," ");
	(void)strcat(buf,filename);
	s_text(buf,20,40);
	(void)system(buf);
	s_text(" ",20,40);
}

void
play_range()
{
	(void)sprintf(range_arg,"-p%ld:+%ld ",new_start, new_nan-new_start+1);
  	(void)strcpy(buf,"play ");
  	(void)strcat(buf,range_arg);
	if (play_options)
		(void)strcat(buf,play_options);
	(void)strcat(buf," ");
	(void)strcat(buf,filename);
	s_text(buf,20,40);
  	(void)system(buf);
	(void) sprintf(buf, "Range selected is from %d to %d", new_start, new_nan);
	s_text(buf, 20, 40);
}

void
zoom_in()
{
	char	*Module = "zoom_in";

	(void)sprintf(buf,"Replotting points %ld through %ld ...", 
		new_start,new_nan);
	s_text(buf,20,40);
	(void)sprintf(range_arg,"-p%ld:+%ld",new_start, new_nan-new_start+1);
	(void)strcpy(buf,"plotsd  -E.1 -D ");
	(void)strcat(buf,range_arg);
	(void)strcat(buf," -Tgps ");
	(void)strcat(buf,filename);
	(void)strcat(buf," > ");
	(void)strcat(buf,tmpname);
	(void)system(buf);

	erase();
	do_plot();
	box_start = scalex(BOX_START);
	box_end = box_start + scalex(BOX_END);
	s_text("",20,40);
	s_text(msg1_ptr,20,20);
	get_common();
	range_state=1;
	cursor_x=mark_x=0;
}

void
zoom_out()
{
	
	s_text("Zooming out...\n",20,40);
	(void)strcpy(buf,"plotsd -p1: -D ");
	(void)strcat(buf," -Tgps ");
	(void)strcat(buf,filename);
	(void)strcat(buf," > ");
	(void)strcat(buf,tmpname);
	(void)system(buf);
	erase();
	do_plot();
	box_start = scalex(BOX_START);
	box_end = box_start + scalex(BOX_END);
	s_text("",20,40);
	s_text(msg1_ptr,20,20);
	get_common();
	range_state=1;
	cursor_x=mark_x=0;
}
