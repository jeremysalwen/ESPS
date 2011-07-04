
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
|   Modified to use Motif widgets by Derek Lin
|
*/
#ifdef SCCS
static char    *sccs_id = "@(#)xwinplt.c	1.4 8/8/89 ESI";
#endif

/* #include <sys/file.h> */

#include <stdio.h>
#include <math.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>

#include <esps/esps.h>

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
extern int	debug_level;

extern	char	*title;
extern	char	*icon_name;

extern  int     Argc;
extern  char    **Argv;

extern char     *geom;
extern Widget   toplevel;

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

Pixmap		make_icon();

void 		s_text(), s_imagetext();
void		quit_proc();
void		do_plot();
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

static void	repaint_canvas();

XmString astring;

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

Widget		QuitW, RefreshW, FormW, WSW;

/* 
 * CALLBACK PROCEDURES
 */

static void Quit(widget, client_data, call_data)
    Widget widget;		
    caddr_t client_data;
    XmAnyCallbackStruct *call_data;	
{
    XtCloseDisplay(XtDisplay(widget));
    (void) unlink (tmpname);
    exit (0);
}


static void Refresh(widget, client_data, call_data)
    Widget widget;
    caddr_t client_data;
    XmAnyCallbackStruct *call_data;
{
    erase();
    repaint_canvas ();
}


void DrawPlot(widget, closure, call_data)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    XmAnyCallbackStruct call_data;		/* unused */
{
    char		*Module = "DrawPlot";
    XGCValues		values;
    XtGCMask		valuemask = 0;

    static int		count = 0;

    if (debug_level )
	fprintf (stderr, "%s: Number of times WorkSpace Widget exposed:  %d\n",
		 Module, ++count);

    if(WidgetsCreated){
      display = XtDisplay (widget);
      MCWindow = XtWindow (widget);
      
      if (debug_level)
	fprintf (stderr, "%s: create Graphics Context\n",
		 Module);
      
      mc_gc = XtGetGC (widget, valuemask, &values);

      erase();
      repaint_canvas ();
    }
}

void DrawPlot_resize(widget, closure, call_data)
    Widget widget;		/* unused */
    caddr_t closure;		/* unused */
    XmAnyCallbackStruct call_data;		/* unused */
{
    char		*Module = "DrawPlot_resize";
    XGCValues		values;
    XtGCMask		valuemask = 0;

    static int		count = 0;

    if (debug_level )
	fprintf (stderr, "%s: Number of times WorkSpace Widget resizedd: %d\n",
		 Module, ++count);

    if(WidgetsCreated){
      display = XtDisplay (widget);
      MCWindow = XtWindow (widget);
      
      if (debug_level)
	fprintf (stderr, "%s: create Graphics Context\n",
		 Module);
      
      mc_gc = XtGetGC (widget, valuemask, &values);
      
      erase();
      repaint_canvas ();
    }
}

init_device()
{
    char		*Module = "xmcd";

    Cardinal	i;
    Arg arg[100];
    int n, x, y;
    int Dwidth;
    int Dheight;
    XWindowAttributes	windowattr;
    Window		MCWindow;
    Display		*display;

    if (debug_level)
	fprintf (stderr, "%s: initialize device...\n",
		 Module);
    
    n = 0;
    astring = XmStringCreateLtoR(title, "chset1");
    XtSetArg (arg[n], XmNtitle, title); n++;
    if(geom)
      XParseGeometry(geom, &x, &y,  (unsigned int *)&Dwidth,
		     (unsigned int *)&Dheight);
    else{
      x = 0;
      y = 0;
      Dwidth = DEFAULT_WIDTH;
      Dheight = DEFAULT_HEIGHT;
    }
    XtSetArg (arg[n], XtNx, x); n++;
    XtSetArg (arg[n], XtNy, y); n++;
    XtSetArg (arg[n], XtNwidth, Dwidth); n++;
    XtSetArg (arg[n], XtNheight, Dheight); n++;
    XtSetValues( toplevel, arg, n);
        
    display = XtDisplay (toplevel);
    MCWindow = XtWindow (toplevel);
    screen = DefaultScreen (display);
    width = DisplayWidth (display, screen);
    height = DisplayHeight (display, screen);

    if (debug_level)
	fprintf (stderr, "%s: screen = %d, width = %d, height = %d\n",
		 Module, screen, width, height);

    if ((font_info = XLoadQueryFont (display, "6x10")) == NULL) {
	fprintf(stderr,"%s: can't load font.\n", Module);
	exit(1);
    }

    Texth = font_info->max_bounds.ascent + font_info->max_bounds.descent;

    FormW = XtCreateManagedWidget ("Form", xmFormWidgetClass, toplevel,NULL,0);

    n = 0;
    astring = XmStringCreateLtoR("Quit", "chset1");
    XtSetArg (arg[n], XmNlabelString, astring); n++;
    XtSetArg (arg[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNtopOffset, 3); n++;
    XtSetArg (arg[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNleftOffset, 3); n++;
    QuitW = XtCreateManagedWidget ("Quit", xmPushButtonWidgetClass,
				   FormW, arg, n);
    XmStringFree(astring);
    XtAddCallback( QuitW, XmNactivateCallback, Quit, NULL);

    n = 0;
    astring = XmStringCreateLtoR("Refresh", "chset1");
    XtSetArg (arg[n], XmNlabelString, astring); n++;
    XtSetArg (arg[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNtopOffset, 3); n++;
    XtSetArg (arg[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNrightOffset, 3); n++;
    RefreshW = XtCreateManagedWidget ("Refresh", xmPushButtonWidgetClass,
				      FormW, arg, n);
    XmStringFree(astring);
    XtAddCallback( RefreshW, XmNactivateCallback, Refresh, NULL);

    n = 0;
    XtSetArg (arg[n], XmNx, x); n++;
    XtSetArg (arg[n], XmNy, y); n++;
    XtSetArg (arg[n], XmNborderWidth, 1 ); n++;
    XtSetArg (arg[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (arg[n], XmNtopWidget, QuitW); n++;
    XtSetArg (arg[n], XmNtopOffset, 3); n++;
    XtSetArg (arg[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNbottomOffset, 3); n++;
    XtSetArg (arg[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNleftOffset, 3); n++;
    XtSetArg (arg[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg (arg[n], XmNrightOffset, 3); n++;
    WSW = XtCreateManagedWidget ("WorkSpace", xmDrawingAreaWidgetClass, FormW, arg, n);
    xorig = yorig = 1;
    XtAddCallback( WSW, XmNexposeCallback, DrawPlot, NULL);
    XtAddCallback( WSW, XmNresizeCallback, DrawPlot_resize, NULL);

    if (debug_level)
	fprintf (stderr, "%s: device initialized\n",
		 Module);
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
	XDrawString (display, MCWindow, mc_gc,
		     xpos, ypos,
		     s, strlen(s));
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
    
    if (debug_level)
	fprintf (stderr, "%s: get window attributes\n", Module);

    if (WidgetsCreated) {
	XGetWindowAttributes (display, MCWindow, &windowattr);
	width = windowattr.width;
	height = windowattr.height;
    } else {
	width = DEFAULT_WIDTH;
	height = DEFAULT_HEIGHT;
    }

    if (debug_level)
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
}


void
start_plot()
{
    char	*Module = "start_plot";
    XEvent	report;
    int		process_events = 1;
    XWMHints    hints;

    if (debug_level)
	fprintf(stderr, "%s: in start plot. . .\n", Module);

    /* window_main_loop(frame); */

    /* wait until window is created before drawing */

    XtRealizeWidget(toplevel);

    WidgetsCreated = 1;

    if (debug_level)
	fprintf(stderr, "%s: making icon.\n", Module);

    hints.flags = IconPixmapHint;
    hints.icon_pixmap = make_icon(XtDisplay(toplevel), screen, icon_name);
    XSetWMHints(XtDisplay(toplevel), XtWindow(toplevel), &hints);


    if (debug_level)
	fprintf(stderr, "%s: starting main loop.\n", Module);

    XtMainLoop();
}


void
s_text(s, x, y)
	char           *s;
	int             x, y;
{
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

	XSetForeground(display, mc_gc, BlackPixel (display, screen));
	XSetBackground(display, mc_gc, WhitePixel (display, screen));
	XDrawImageString (display, MCWindow, mc_gc,
			  x, y-10,
			  s, strlen (s));
}

