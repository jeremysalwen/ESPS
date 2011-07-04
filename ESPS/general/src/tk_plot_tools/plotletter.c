/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  plot_letter -- plot a letter from 2 floating-point arrays		|
|									|
|  Ajaipal S. Virdy, Entropic Speech, Inc. 				|
|									|
|  Module:	plot_letter.c						|
|									|
|  Given arrays of x- and y-coordinates, plot_letter writes plot	|
|  commands in ``Stanford'' plotas input format to plot the line	|
|  connecting the points with the given coordinates.  The argument	|
|  len is the number of points to plot; xmin ... ymax are the data	|
|  coordinates that are to correspond to the edges of the plot.		|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)plotletter.c	1.1 9/19/97    ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>

plot_letter(xdata, ydata, len, color, xmin, xmax, ymin, ymax, letter, ux, uy, lx, ly)
double	xdata[], ydata[];
int	len, color;
double  xmin, xmax, ymin, ymax;
char	letter;
int	ux, uy;
int	lx, ly;
{
    int ix, iy, i;
    int	width, height;
    char s[80];

    (void) sprintf(s,"c %d\n", color);
    tk_snd_plot_cmd(s);

    width = lx - ux;
    height = ly - uy;

    for (i = 0; i < len; i++)
	if (xmin <= xdata[i] && xdata[i] <= xmax
		&& ymin <= ydata[i] && ydata[i] <= ymax)
	{
            ix = ROUND(ux + width * (xdata[i] - xmin)/(xmax - xmin));
            iy = ROUND(ly - height * (ydata[i] - ymin)/(ymax - ymin));
            sprintf(s,"m %d %d",iy + 15, ix - 20);
	    tk_snd_plot_cmd(s);
	    tk_snd_plot_cmd("t 3 1");
	    sprintf(s,"%c",letter);
	    tk_snd_plot_cmd(s);
	}
}
