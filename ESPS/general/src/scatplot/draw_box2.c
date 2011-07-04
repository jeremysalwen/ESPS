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
|                          All rights reserved."			|
|									|
+-----------------------------------------------------------------------+
|									|
|  draw_box -- draw labeled box in a form suitable for plotas		|
|									|
|  Shankar Narayan, EPI							|
|  Adapted by Joseph T. Buck.						|
|  This version is a modification of the original draw_box.		|
|    by Ajaipal S. Virdy						|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)draw_box2.c	3.4	10/9/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define Printf (void) printf
#define EPS 0.00001

double fabs();


draw_box (xmin, xmax, xstep, xdp, ymin, ymax, ystep, ydp, ux, uy, lx, ly)
    double  xmin, xmax, xstep, ymin, ymax, ystep;
    int	    ux, uy;	/* upper left corner coordinates */
    int	    lx, ly;	/* lower right corner coordinates */
{
    char    xfmt[10], yfmt[10];
    double  tmp;
    int	    ix, iy;
    char    label[50];
    int	    width, height;

    Sprintf(xfmt, "%%g");
    Sprintf(yfmt, "%%g");

    Printf("\nc 1\n");

    width = lx - ux;
    height = ly - uy;

    Printf("m %d %d\n", uy, ux);
    Printf("d %d %d\n", uy, lx);
    Printf("d %d %d\n", ly, lx);
    Printf("d %d %d\n", ly, ux);
    Printf("d %d %d\n", uy, ux);

    Printf("c 2\n");

 /* Draw vertical grid */

    for (tmp = xmin; tmp <= xmax + EPS*xstep; tmp = tmp + xstep)
    {
	ix = ux + width * (tmp - xmin) / (xmax - xmin);
	Printf("m %d %4d\n", uy, ix);
	Printf("d %d %4d\n", ly, ix);
	Sprintf(label, xfmt, (fabs(tmp) < EPS*xstep) ? 0.0 : tmp);
	Printf("m %d %4d\nt 3 1\n", ly + 150, ix - 24*strlen(label));
			/* 1/2 width of size 3 character is 24 */
	Printf("%s\n", label);
    }

 /* Draw ticks */

    for (tmp = xmin + xstep / 2; tmp <= xmax + EPS*xstep; tmp = tmp + xstep)
    {
	ix = ux + width * (tmp - xmin) / (xmax - xmin);
	Printf("m %d %4d\n", uy - 25, ix);
	Printf("d %d %4d\n", uy + 25, ix);
	Printf("m %d %4d\n", ly - 25, ix);
	Printf("d %d %4d\n", ly + 25, ix);
    }

 /* Draw horizontal grid */

    for (tmp = ymin; tmp <= ymax + EPS*ystep; tmp = tmp + ystep)
    {
	iy = ly - height * (tmp - ymin) / (ymax - ymin);
	Printf("m %4d %d\n", iy, lx);
	Printf("d %4d %d\n", iy, ux);
	Sprintf(label, yfmt, (fabs(tmp) < EPS*ystep) ? 0.0 : tmp);
	Printf("m %4d %d\nt 3 1\n", iy + 30, 420 - 48*strlen(label));
	Printf("%s\n", label);
    }
}
