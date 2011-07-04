/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1989, Entropic Speech, Inc. All rights reserved."	|
|									|
|									|
|   make_icon.c - make ESPS X-windows icons				|
|									|
|   Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifdef XWIN

#ifdef SCCS
static char    *sccs_id = "@(#)make_icon.c	1.2	3/9/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "sine.icon"
#include "spec.icon"
#include "image.icon"
#include "epi.icon"
#include "hist.icon"

#define ICON_SINE   1
#define ICON_SPEC   2
#define ICON_IMAGE  3
#define ICON_EPI    4
#define ICON_HIST   5

char	*icon_names[] = {"NONE", "SINE", "SPEC", "IMAGE", "EPI", "HIST",
			NULL};

extern int	debug_level;

Pixmap
make_icon(display, screen, name)
    char    *name;
    Display *display;
    int	    screen;

{
    XIconSize	*size_list;
    int	    	count;
    int	    	icon_width, icon_height;
    char    	*icon_bits;

    switch (lin_search2(icon_names, name))
    {
    case NONE:
	return None;
	break;
    case ICON_SINE:
	icon_width = sine_width;
	icon_height = sine_height;
	icon_bits = sine_bits;
	break;
    case ICON_SPEC:
	icon_width = spec_width;
	icon_height = spec_height;
	icon_bits = spec_bits;
	break;
    case ICON_IMAGE:
	icon_width = image_width;
	icon_height = image_height;
	icon_bits = image_bits;
	break;
    case ICON_EPI:
	icon_width = epi_width;
	icon_height = epi_height;
	icon_bits = epi_bits;
	break;
    case ICON_HIST:
	icon_width = hist_width;
	icon_height = hist_height;
	icon_bits = hist_bits;
	break;
    default:
	Fprintf(stderr, "Icon name \"%s\" not recognized.\n", name);
	return None;
	break;
    }

    if (XGetIconSizes(display, RootWindow(display, screen),
		      &size_list, &count)
	&& (icon_width < size_list->min_width
	    || icon_height < size_list->min_height))
    {
	Fprintf(stderr, "Can't use icon\n");
	return None;
    }
    else return
	XCreateBitmapFromData(display, RootWindow(display, screen),
			      icon_bits, icon_width, icon_height);
}

#endif
