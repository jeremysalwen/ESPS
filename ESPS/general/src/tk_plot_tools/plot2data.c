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
|  plot2data -- plot data from 2 floating-point arrays			|
|									|
|  Rodney Johnson, Entropic Speech, Inc.; adapted from program		|
|	plot_data by Shankar Narayan.					|
|									|
|  Module:	plot2data.c						|
|									|
|  Given arrays of x- and y-coordinates, plot2data writes plot  	|
|  commands in ``Stanford'' plotas input format to plot the line	|
|  connecting the points with the given coordinates.  The argument	|
|  len is the number of points to plot; xmin ... ymax are the data	|
|  coordinates that are to correspond to the edges of the plot.		|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)plot2data.c	1.1 9/19/97   ERL";
#endif

#include <stdio.h>

#define Printf (void) printf

plot2data(xdata, ydata, len, color, xmin, xmax, ymin, ymax)
    int     len, color;
    float   xdata[], ydata[];
    double  xmin, xmax, ymin, ymax;
{
    int ix, iy, i;
    char s[80];

    tk_snd_plot_cmd("set_class data");
    tk_snd_plot_cmd("c 3");

    for (i = 0; i < len; i++)
    {
        ix = 0.5 + 500 + 5000*(xdata[i] - xmin)/(xmax - xmin);
        iy = 0.5 + 3000 - 2500*(ydata[i] - ymin)/(ymax - ymin);
        if (i == 0) {
            sprintf(s,"m %d %d", iy, ix);
	    tk_snd_plot_cmd(s);
	}
        else {
            sprintf(s,"d %d %d", iy, ix);
	    tk_snd_plot_cmd(s);
	}
    }
}
