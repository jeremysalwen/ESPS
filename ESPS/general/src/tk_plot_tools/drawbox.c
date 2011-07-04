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
    static char *sccs_id = "@(#)drawbox.c	1.1 9/19/97    ERL";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>


draw_box (xmin, xmax, xstep, xdp, ymin, ymax, ystep, ydp, nogrid)
    double  xmin, xmax, xstep;
    int     xdp;
    double  ymin, ymax, ystep;
    int     ydp;
    int	    nogrid;  /* no horiz grid if nogrid == 1 */
{
    char    xfmt[10], yfmt[10];
    char    s[80];
    char    label[50];
    double  tmp;
    int     ix, iy;

    Sprintf(xfmt, "%%.%df", xdp);	/* variable width (centered) */
    Sprintf(yfmt, "%%5.%df", ydp);	/* fixed min width (right justified) */
    tk_snd_plot_cmd ("c 1");
    tk_snd_plot_cmd ("s 3");

    tk_snd_plot_cmd ("set_class box");
    tk_snd_plot_cmd ("m 500 500");
    tk_snd_plot_cmd ("d 500 5500");
    tk_snd_plot_cmd ("d 3000 5500");
    tk_snd_plot_cmd ("d 3000 500");
    tk_snd_plot_cmd ("d  500 500");

    tk_snd_plot_cmd ("s 1");
    tk_snd_plot_cmd ("c 6");

 /* Draw vertical grid */

    tk_snd_plot_cmd("set_class v_grid");
    for (tmp = xmin; tmp <= xmax; tmp = tmp + xstep) {
	ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	tk_snd_plot_cmd_1arg ("m 500 %4d", ix);
	tk_snd_plot_cmd_1arg ("d 3000 %4d", ix);
	Sprintf (label, xfmt, tmp);
	tk_snd_plot_cmd ("c 1");
	tk_snd_plot_cmd_1arg ("m 3200 %4d",ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	tk_snd_plot_cmd("t 5 1");
	tk_snd_plot_cmd (label);
    	tk_snd_plot_cmd ("c 6");
    }

 /* Draw ticks */
    tk_snd_plot_cmd("set_class ticks");
    for (tmp = xmin + xstep / 2; tmp <= xmax; tmp = tmp + xstep) {
	ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	tk_snd_plot_cmd_1arg ("m 475 %4d", ix);
	tk_snd_plot_cmd_1arg ("d 525 %4d", ix);
	tk_snd_plot_cmd_1arg ("m 2975 %4d", ix);
	tk_snd_plot_cmd_1arg ("d 3025 %4d", ix);
    }

 /* Draw horizontal grid */
    tk_snd_plot_cmd("set_class h_grid");
    for (tmp = ymin; tmp <= ymax; tmp = tmp + ystep) {
	iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	if (nogrid)
		tk_snd_plot_cmd_1arg ("m %4d 500", iy);
	else {
		tk_snd_plot_cmd_1arg ("m %4d 5500", iy);
		tk_snd_plot_cmd_1arg ("d %4d 500", iy);
	}
	Sprintf (label, yfmt, tmp);
	tk_snd_plot_cmd ("c 1");
	sprintf (s,"m %4d %4d", iy + 50, 400 - 80*strlen(label));
	tk_snd_plot_cmd(s);
	tk_snd_plot_cmd("t 5 1");
	tk_snd_plot_cmd (label);
    	tk_snd_plot_cmd ("c 6");
    }
    tk_snd_plot_cmd("c 4");
}
