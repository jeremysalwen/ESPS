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
    static char *sccs_id = "@(#)draw_box2.c	1.1 9/19/97   ERL";
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
    char    s[80];

    Sprintf(xfmt, "%%g");
    Sprintf(yfmt, "%%g");

    tk_snd_plot_cmd("c 1");
    tk_snd_plot_cmd("s 3");
    tk_snd_plot_cmd("set_class box");

    width = lx - ux;
    height = ly - uy;

    sprintf(s,"m %d %d\n", uy, ux);
    tk_snd_plot_cmd(s);
    sprintf(s,"d %d %d\n", uy, lx);
    tk_snd_plot_cmd(s);
    sprintf(s,"d %d %d\n", ly, lx);
    tk_snd_plot_cmd(s);
    sprintf(s,"d %d %d\n", ly, ux);
    tk_snd_plot_cmd(s);
    sprintf(s,"d %d %d\n", uy, ux);
    tk_snd_plot_cmd(s);

    tk_snd_plot_cmd("c 6");
    tk_snd_plot_cmd("s 1");

 /* Draw vertical grid */

    tk_snd_plot_cmd("set_class v_grid");
    for (tmp = xmin; tmp <= xmax + EPS*xstep; tmp = tmp + xstep)
    {
	ix = ux + width * (tmp - xmin) / (xmax - xmin);
	sprintf(s,"m %d %4d", uy, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"d %d %4d", ly, ix);
	tk_snd_plot_cmd(s);
	Sprintf(label, xfmt, (fabs(tmp) < EPS*xstep) ? 0.0 : tmp);
	sprintf(s,"m %d %4d", ly + 150, ix - 24*strlen(label));
			/* 1/2 width of size 3 character is 24 */
	tk_snd_plot_cmd(s);
	tk_snd_plot_cmd("t 5 1");
	tk_snd_plot_cmd(label);
    }

 /* Draw ticks */

    tk_snd_plot_cmd("set_class ticks");
    for (tmp = xmin + xstep / 2; tmp <= xmax + EPS*xstep; tmp = tmp + xstep)
    {
	ix = ux + width * (tmp - xmin) / (xmax - xmin);
	sprintf(s,"m %d %4d", uy - 25, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"d %d %4d", uy + 25, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"m %d %4d", ly - 25, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"d %d %4d", ly + 25, ix);
	tk_snd_plot_cmd(s);
    }

 /* Draw horizontal grid */

    tk_snd_plot_cmd("set_class h_grid");
    for (tmp = ymin; tmp <= ymax + EPS*ystep; tmp = tmp + ystep)
    {
	iy = ly - height * (tmp - ymin) / (ymax - ymin);
	sprintf(s,"m %4d %d", iy, lx);
	tk_snd_plot_cmd(s);
	sprintf(s,"d %4d %d", iy, ux);
	tk_snd_plot_cmd(s);
	Sprintf(label, yfmt, (fabs(tmp) < EPS*ystep) ? 0.0 : tmp);
	sprintf(s,"m %4d %d", iy + 30, 420 - 48*strlen(label));
	tk_snd_plot_cmd(s);
	tk_snd_plot_cmd("t 5 1");
	tk_snd_plot_cmd(label);
    }
}
