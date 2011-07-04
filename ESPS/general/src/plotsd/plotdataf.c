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
|  plot_dataf -- plot data from an array of floats			|
|									|
|  Joseph T. Buck, EPI							|
|  Adapted from a program by Shankar Narayan                     	|
|  John Shore - Modified to take parameter for interval between points	|
|		to be plotted (to permit speedup).         		|
|	        Use interval=1 to plot every point.			|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)plotdataf.c	3.4	1/26/93	ESI";
#endif

#define Printf (void) printf
#include <esps/esps.h>

int penbounds[] = { 500, 5500, 3000, 500 };

plot_dataf(data, lnt, xmin, xmax, ymin, ymax, nskip, interval)
    float   *data;	/*array of points - a subset of these will be plotted*/
    long    lnt;    	/*number of points to plot in data[]*/
    int	    interval;	/*interval between points in data[] to be plotted*/
    long    nskip;	/*x coordinate of first point to be plotted*/
    double  ymin, ymax, xmin, xmax;
{
    int     i, ix, iy, min, max, j;
    char    com = 'm';
    float   xscl = penbounds[1] - penbounds[0],
	    yscl = penbounds[3] - penbounds[2];

    if (interval == 1)
    for (i = 0; i < lnt; i++) {
	ix = penbounds[0] + (nskip + i*interval - xmin) * xscl / (xmax - xmin);
	iy = penbounds[2] + (data[i*interval] - ymin) * yscl / (ymax - ymin);
	Printf ("%c %d %d\n", com, iy, ix);
	if (i == 0) com = 'd';
    }
    else {
#ifdef DEBUG
      FILE *errlog = fopen("foo","w");
#endif
      min=data[0]; max=data[0];
      for (j=0; j < interval; j++) {
	min = MIN(data[j],min);
	max = MAX(data[j],max);
      }
      ix = penbounds[0] + (nskip - xmin) * xscl / (xmax - xmin);
      iy = penbounds[2] + (min - ymin) * yscl / (ymax - ymin);
      Printf ("m %d %d\n", iy, ix);
#ifdef DEBUG
      fprintf (errlog,"m %d %d\n", iy, ix);
#endif
      iy = penbounds[2] + (max - ymin) * yscl / (ymax - ymin);
      Printf ("d %d %d\n", iy, ix);
#ifdef DEBUG
      fprintf (errlog,"d %d %d\n", iy, ix);
#endif
      for (i = 1; i < lnt; i++) {
        min=max=data[i*interval-1];
        for (j=0; j < interval+1; j++) {
	  min = MIN(data[j+i*interval-1],min);
	  max = MAX(data[j+i*interval-1],max);
        }
	ix = penbounds[0] + (nskip + i*interval - xmin) * xscl / (xmax - xmin);
	iy = penbounds[2] + (min - ymin) * yscl / (ymax - ymin);
	Printf ("m %d %d\n", iy, ix);
#ifdef DEBUG
	fprintf (errlog,"m %d %d\n", iy, ix);
#endif
	iy = penbounds[2] + (max - ymin) * yscl / (ymax - ymin);
	Printf ("d %d %d\n",  iy, ix);
#ifdef DEBUG
	fprintf (errlog,"d %d %d\n", iy, ix);
#endif
      }
    }
}
