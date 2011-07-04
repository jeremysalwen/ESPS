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
|  Program:  next_slide							|
|									|
|  This program causes the program xloadimage, if invoked with the	|
|  -prog_slideshow option, to advance to the next image.  The two	|
|  programs communicate by using a one-byte property on the root	|
|  window as a semaphore.						|
|									|
|  Module:  next_slide.c						|
|									|
|  Main program.							|
|									|
|  Rod Johnson, Entropic Research Laboratory, Inc.  90/09/25		|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)next_slide.c	1.2  11/8/90";
#endif
static char *Copyright =
	"Copyright (c) 1990 Entropic Research Laboratory, Inc.";

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "ready.h"

#define LIM	60

main(argc, argv)
    int		argc;
    char	**argv;
{
    Display	*disp;
    int		scrn;
    int		delay;
    int		lim;

    switch (argc)
    {
    case 1:
	lim = LIM;
	break;
    case 2:
	lim = atoi(argv[1]);
	break;
    default:
	fprintf(stderr, "Usage: next_slide [max_wait]\n");
	exit(1);
	break;
    }

    disp = XOpenDisplay((char *) NULL);
    if (!disp)
    {
	fprintf(stderr, "Can't open display.\n");
	exit(1);
    }
    scrn = DefaultScreen(disp);

    intern_ready(disp);

    if (lim <= 0)
    {
	set_ready(disp, scrn, RDY_BUSY);
	exit(0);
    }

    for (delay = 0; delay < lim; )
    {
	switch (get_ready(disp, scrn))
	{
	case RDY_BUSY:
	    delay++;
	    break;
	case RDY_READY:
	    set_ready(disp, scrn, RDY_BUSY);
	    exit(0);
	    break;
	case RDY_PAUSE:
	    break;
	case RDY_QUIT:
	    set_ready(disp, scrn, RDY_BUSY);
	    exit(1);
	}
	sleep(1);
    }

    fprintf(stderr, "xloadimage timed out.\n");
    exit(1);
}
