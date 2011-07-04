/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  plotphdata -- plot phase data from 2 floating-point arrays		|
|									|
|  Rodney Johnson, Entropic Speech, Inc.  Adapted via plot2data()	|
|	from plot_data() by Shankar Narayan.				|
|									|
|  Module:	plotphdata.c						|
|									|
|  Given arrays of x- and y-coordinates, plotphdata writes plot  	|
|  commands in ``Stanford'' plotas input format to plot the line	|
|  connecting the points with the given coordinates.  The argument	|
|  len is the number of points to plot; xmin ... ymax are the data	|
|  coordinates that are to correspond to the edges of the plot.  The	|
|  y values are assumed to be defined modulo (ymax - ymin) so that the	|
|  connecting lines may "wrap around" from the top edge to the bottom	|
|  or vice versa.							|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)plotphdata.c	1.1	2/16/88	ESI";
#endif

#define Printf (void) printf

plotphdata(xdata, ydata, len, color, xmin, xmax, ymin, ymax)
    int     len, color;
    float   *xdata, *ydata;
    double  xmin, xmax, ymin, ymax;
{
    int	    ix, iy, ix0, i;
    double  dx;
    double  xdif = xmax - xmin;
    double  ydif = ymax - ymin;

    Printf("c %d\n", color);

    for (i = 0; i < len; i++)
    {
	ix = 0.5 + 500 + 5000*(xdata[i] - xmin)/xdif;
	iy = 0.5 + 3000 - 2500*(ydata[i] - ymin)/ydif;
	if (i == 0)
	    Printf("m %d %d\n", iy, ix);
	else if (ydata[i] - ydata[i-1] > 0.5*ydif)
	{
	    dx = (xdata[i] - xdata[i-1]) * (ydata[i] - ymax)
		    / (ydata[i] - ydif - ydata[i-1]);
	    ix0 = 0.5 + 500 + 5000*(xdata[i] - dx - xmin)/xdif;
	    Printf("d %d %d\n", 3000, ix0);
	    Printf("m %d %d\n", 500, ix0);
	    Printf("d %d %d\n", iy, ix);
	}
	else if (ydata[i] - ydata[i-1] < -0.5*ydif)
	{
	    dx = (xdata[i] - xdata[i-1]) * (ydata[i] - ymin)
		    / (ydata[i] + ydif - ydata[i-1]);
	    ix0 = 0.5 + 500 + 5000*(xdata[i] - dx - xmin)/xdif;
	    Printf("d %d %d\n", 500, ix0);
	    Printf("m %d %d\n", 3000, ix0);
	    Printf("d %d %d\n", iy, ix);
	}
	else
	    Printf("d %d %d\n", iy, ix);
    }
}
