/* NameToPixel - convert name to pixel
 *
 * Author - taken from /usr/local/src/X11R2/clients/x???
 */

#ifdef SCCS
   static char	*SccsId = "@(#)NamePix.c	1.2 3/9/89 ESI NSL";
#endif

#include <stdio.h>
#include <values.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define	Dynamic 1

unsigned long
NameToPixel(dpy, scr, name, pixel)
  Display	*dpy;
  int		scr;
  char		*name;
  unsigned long	pixel;
{
    char	*Module = "NameToPixel";
    XColor	scolor, ecolor;

    if (!name || !*name)
	return pixel;

    if (!XAllocNamedColor(dpy, DefaultColormap(dpy, scr), name,
			  &scolor, &ecolor)) {
	fprintf(stderr, "%s: unknown color, or allocation failure: %s\n",
	Module, name);
	exit (1);
	/* NOTREACHED */
    }
    if ((ecolor.pixel != BlackPixel(dpy, scr)) &&
	(ecolor.pixel != WhitePixel(dpy, scr)) &&
	(DefaultVisual(dpy, scr)->class & Dynamic))
	/* save_colors = 1; */
	;
    return (ecolor.pixel);
}
