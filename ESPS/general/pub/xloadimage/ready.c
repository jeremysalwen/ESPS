/*----------------------------------------------------------------------+
|									|
|  Copyright (c) 1990 Entropic Research Laboratory, Inc.		|
|									|
|  Permission to use, copy, modify, distribute, and sell this software	|
|  and its documentation for any purpose is hereby granted without	|
|  fee, provided that the above copyright notice appear in all copies	|
|  and that both that copyright notice and this permission notice	|
|  appear in supporting documentation.  Neither Entropic Research	|
|  Laboratory nor the author makes any representations about the	|
|  suitability of this software for any purpose.  It is provided "as	|
|  is" without express or implied warranty.				|
|									|
|  ENTROPIC RESEARCH LABORATORY AND THE AUTHOR DISCLAIM ALL WARRANTIES	|
|  WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF	|
|  MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ENTROPIC RESEARCH	|
|  LABORATORY OR THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT, OR	|
|  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS	|
|  OF USE, DATA, OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,		|
|  NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN		|
|  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.		|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module:  ready.c							|
|									|
|  This module is include in programs xloadimage and next_slide.  The	|
|  latter program causes xloadimage, if invoked with the option		|
|  -prog_slideshow, to advance to the next image.  The functions in	|
|  this module create and access a semaphore that the two programs use	|
|  for communication.  This is implemented as a one-byte property	|
|  "XLOADIM_READY" on the root window.					|
|									|
|  Rod Johnson, Entropic Research Laboratory, Inc.  90/09/25		|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)ready.c	1.2  11/8/90";
#endif
static char *Copyright =
	"Copyright (c) 1990 Entropic Research Laboratory, Inc.";

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "ready.h"

static Atom		xloadim_ready = None;

void
intern_ready(disp)
    Display	*disp;
{
    xloadim_ready = XInternAtom(disp, "XLOADIM_READY", False);
}


void
set_ready(disp, scrn, val)
    Display	    *disp;
    int		    scrn;
    int		    val;
{
    unsigned char   data = val;

    if (xloadim_ready == None)
	intern_ready(disp);
    XChangeProperty(disp, RootWindow(disp, scrn), xloadim_ready, xloadim_ready,
		    8, PropModeReplace, &data, 1);
    XFlush(disp);
}


void
init_ready(disp, scrn)
    Display	*disp;
    int		scrn;
{
    set_ready(disp, scrn, RDY_BUSY);
    XSelectInput(disp, RootWindow(disp, scrn), PropertyChangeMask);
}


int
is_ready(disp, event)
    Display	*disp;
    XEvent	*event;
{
    return event->xproperty.atom == xloadim_ready;
}


int
get_ready(disp, scrn)
    Display	    *disp;
    int		    scrn;
{
    Atom	    ac_type;
    int		    ac_fmt;
    unsigned long   nitems, bytes_aft;
    unsigned char   *prop;

    if (XGetWindowProperty(disp, RootWindow(disp, scrn), xloadim_ready,
			    0L, 1L, False, xloadim_ready,
			    &ac_type, &ac_fmt, &nitems, &bytes_aft,
			    &prop) == Success
	&& prop && nitems > 0)
    {
	return *prop;
    }
    else
    {
	return 0;
    }
}
