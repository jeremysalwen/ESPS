/* root.c:
 *
 * this loads an image onto the root window.  changes to allow proper
 * freeing of previously allocated resources made by Deron Dann Johnson
 * (dj@eng.sun.com).
 *
 * jim frost 10.03.89
 *
 * Copyright 1989, 1990 Jim Frost.  See included file "copyright.h" for
 * complete copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)root.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "xloadimage.h"
#include <X11/Xatom.h>

#define RETAIN_PROP_NAME	"XA_XLOADIMAGE_COLORS_XID"

static void
updateProperty(dpy, w, name, type, format, data, nelem)
     Display	*dpy;
     Window	w;
     char	*name;
     Atom	type;
     int	format;
     int	data;
     int	nelem;
{
  /* intern the property name */
  Atom	atom = XInternAtom(dpy, name, 0);

  /* create or replace the property */
  XChangeProperty(dpy, w, atom, type, format, PropModeReplace, 
		  (unsigned char *)&data, nelem);
}


/* Sets the close-down mode of the client to 'RetainPermanent'
 * so all client resources will be preserved after the client
 * exits.  Puts a property on the default root window containing
 * an XID of the client so that the resources can later be killed.
 */

void
preserveResource(dpy, w)
     Display	*dpy;
     Window	w;
{
  /* create dummy resource */
  Pixmap pm= XCreatePixmap(dpy, w, 1, 1, 1);
	
  /* create/replace the property */
  updateProperty(dpy, w, RETAIN_PROP_NAME, XA_PIXMAP, 32, (int)pm, 1);
	
  /* retain all client resources until explicitly killed */
  XSetCloseDownMode(dpy, RetainPermanent);
}


/* Flushes any resources previously retained by the client,
 * if any exist.
 */

static void
freePrevious(dpy, w)
     Display	*dpy;
     Window	w;
{
  Pixmap *pm;			
  Atom	actual_type;		/* NOTUSED */
  int	format;
  int	nitems;
  int	bytes_after;

  /* intern the property name */
  Atom atom = XInternAtom(dpy, RETAIN_PROP_NAME, 0);

  /* look for existing resource allocation */
  if ((XGetWindowProperty(dpy, w, atom, 0, 1, 1/*delete*/,
			  AnyPropertyType, &actual_type, &format, &nitems,
			  &bytes_after, &pm) == Success) &&
      nitems == 1) 
    if ((actual_type == XA_PIXMAP) && (format == 32) &&
	(nitems == 1) && (bytes_after == 0)) {
      /* blast it away */
      XKillClient(dpy, (Pixmap *) *pm);
      XFree(pm);
    }
    else if (actual_type != None) {
      fprintf(stderr,
	      "%s: warning: invalid format encountered for property %s\n",
	      RETAIN_PROP_NAME, "xloadimage");
    }
}

void imageOnRoot(disp, scrn, image, verbose)
     Display      *disp;
     int           scrn;
     Image        *image;
     unsigned int  verbose;
{ Pixmap   pixmap;
  Colormap xcmap;


  freePrevious(disp, RootWindow(disp, scrn));

  if (! sendImageToX(disp, scrn, DefaultVisual(disp, scrn), image,
		     &pixmap, &xcmap, verbose))
    exit(1);

  /* changing the root colormap is A Bad Thing, so deny it.
   */

  if (xcmap != DefaultColormap(disp, scrn)) {
    printf("Loading image onto root would change default colormap (sorry)\n");
    XFreePixmap(disp, pixmap);
    exit(1);
  }

  XSetWindowBackgroundPixmap(disp, RootWindow(disp, scrn), pixmap);
  XClearWindow(disp, RootWindow(disp, scrn));
  XFreePixmap(disp, pixmap);
  preserveResource(disp, RootWindow(disp, scrn));
}
