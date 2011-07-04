/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|         "Copyright (c) 1986, 1987, 1989 Entropic Speech, Inc.		|
|                         All rights reserved."				|
|									|
+-----------------------------------------------------------------------+
|									|
|  draw_box -- draw labeled box in a form suitable for plotas		|
|									|
|  Shankar Narayan, EPI							|
|  Adapted by Joseph T. Buck. 						|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)drawbox.c	3.3	10/9/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define Printf (void) printf

draw_box (xmin, xmax, xstep, xdp, ymin, ymax, ystep, ydp)
    double  xmin, xmax, xstep;
    int     xdp;
    double  ymin, ymax, ystep;
    int     ydp;
{
    char    xfmt[10], yfmt[10];
    double  tmp;
    int     ix, iy;
    char    label[50];

    Sprintf(xfmt, "%%.%df", xdp);	/* variable width (centered) */
    Sprintf(yfmt, "%%5.%df", ydp);	/* fixed min width (right justified) */
    Printf ("\nc 1\n");

    Printf ("m 500 500\n");
    Printf ("d 500 5500\nd 3000 5500\nd 3000 500\nd  500 500\n");

    Printf ("c 2\n");

 /* Draw vertical grid */

    for (tmp = xmin; tmp <= xmax; tmp = tmp + xstep) {
	ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	Printf ("m 500 %4d\n", ix);
	Printf ("d 3000 %4d\n", ix);
	Sprintf(label, xfmt, tmp);
	Printf ("m 3200 %4d\nt 5 1\n", ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	Printf ("%s\n", label);
    }

 /* Draw ticks */
    for (tmp = xmin + xstep / 2; tmp <= xmax; tmp = tmp + xstep) {
	ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	Printf ("m 475 %4d\n", ix);
	Printf ("d 525 %4d\n", ix);
	Printf ("m 2975 %4d\n", ix);
	Printf ("d 3025 %4d\n", ix);
    }

 /* Draw horizontal grid */
    for (tmp = ymin; tmp <= ymax; tmp = tmp + ystep) {
	iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	Printf ("m %4d 5500\n", iy);
	Printf ("d %4d 500\n", iy);
	Sprintf(label, yfmt, tmp);
	Printf ("m %4d %4d\nt 5 1\n", iy + 50, 400 - 80*strlen(label));
			/* 1/2 height of size 5 digit is 50 */
	Printf ("%s\n", label);
    }
}
