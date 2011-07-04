/* draw_line - draw a rubberbanding line
 *
 * Author -   Ajaipal S. Virdy, Neural Systems Lab
 */

#ifdef SCCS
   static char	*SccsId = "@(#)draw_line.c	1.3 8/8/89 ESI NSL";
#endif

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define undraw_line(display,screen,x,y,w,h) \
	   draw_line(display, screen, x, y, w, h);

extern int	debug_level;

draw_line(display, screen, win, x, y, x2, y2)
Display	*display;
int	screen;
Window	win;
int	x, y;
unsigned int x2, y2;
{
	char		*Module = "draw_line";

	static int	first_time = 1;

	static GC	gcontext;
	unsigned long	valuemask = 0;	/* Ignore XGCvalues and use defaults */

	if (first_time) {

	   /* Initialize Graphics Context */

	   gcontext = XCreateGC (display, win,
				 valuemask, NULL);

	   /* X Manual suggests using BlackPixel for GXxor: use WhitePixel */

	   XSetForeground (display, gcontext, 1);
	   XSetBackground (display, gcontext, 0);
	   XSetSubwindowMode (display, gcontext, IncludeInferiors);
	   XSetFunction (display, gcontext, GXxor);

	   first_time = 0;
	}

	if (debug_level & 32)
	   fprintf (stderr, "%s: x = %d, y = %d, x2 = %d, y2 = %d\n",
	   Module, x, y, x2, y2);

	XDrawLine (display, win, gcontext, x, y, x2, y2);

	XFlush (display);
}
