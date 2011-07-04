/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988 Entropic Speech, Inc.  All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: imagedot.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imagedot.c	1.3	6/28/93	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

char		*arr_alloc();
void		arr_free();

extern void	(*dev_row)();
void		pr_darray();

extern int	debug_level;
extern long	nrows, ncols;
extern int	oflag;
extern int	alg;

static int	rownum;
static char	*dotbuf;
static double	**rowbuf;


void
dot_init()
{
    long    rowbufdim[2];
    int	    u, v;

    if (debug_level >= 2)
	Fprintf(stderr, "nrows: %ld.\tncols: %ld.\n", nrows, ncols);

    dotbuf = (char *) arr_alloc(1, &ncols, CHAR, 0);
    rownum = 0;

    switch (alg)
    {
    case HT_OD1:
	break;
    case HT_OD2:
	break;
    case HT_OD3:
	break;
    case HT_OD4:
	break;
    case HT_FS:
    case HT_FS2:
    	rowbufdim[0] = 2;   rowbufdim[1] = ncols + 2;
	rowbuf = (double **) arr_alloc(2, rowbufdim, DOUBLE, 0);
	for (u = 0; u < 2; u++)
	    for (v = 0; v < ncols + 2; v++)
	    rowbuf[u][v] = 0.0;
	break;
    case HT_DK1:
	Fprintf(stderr, "Knuth algorithm not yet supported.\n");
	exit(1);
	break;
    case HT_16LVL:
	break;
    case HT_16OD1:
    case HT_16OD1_2:
    case HT_16OD1_3:
	break;
    default:
	Fprintf(stderr, "Algorithm not recognized.\n");
	exit(1);
    }
}

void
dot_row(row)
    double  *row;
{
    int	    v;

    if (debug_level >= 3)
	pr_darray("row:\n", (int) ncols, row);

    switch (alg)
    {
    case HT_OD1:
	{
	    static double   thresh[4][4] =
		{
		    {15.0/17.0,  1.0/17.0,  8.0/17.0, 10.0/17.0},
		    { 4.0/17.0, 14.0/17.0, 11.0/17.0,  5.0/17.0},
		    { 9.0/17.0,  7.0/17.0,  2.0/17.0, 16.0/17.0},
		    { 6.0/17.0, 12.0/17.0, 13.0/17.0,  3.0/17.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = row[v] > thresh[rownum%4][v%4];
	}
	break;
    case HT_OD2:
	{
	    static double   thresh[8][8] =
		{
		    { 1.0/65.0, 63.0/65.0,  6.0/65.0, 60.0/65.0,
			21.0/65.0, 43.0/65.0, 18.0/65.0, 48.0/65.0},
		    {62.0/65.0,  4.0/65.0, 57.0/65.0,  7.0/65.0,
			42.0/65.0, 24.0/65.0, 45.0/65.0, 19.0/65.0},
		    {11.0/65.0, 53.0/65.0, 16.0/65.0, 50.0/65.0,
			31.0/65.0, 33.0/65.0, 28.0/65.0, 38.0/65.0},
		    {56.0/65.0, 10.0/65.0, 51.0/65.0, 13.0/65.0,
			36.0/65.0, 30.0/65.0, 39.0/65.0, 25.0/65.0},
		    {41.0/65.0, 23.0/65.0, 46.0/65.0, 20.0/65.0,
			61.0/65.0,  3.0/65.0, 58.0/65.0,  8.0/65.0},
		    {22.0/65.0, 44.0/65.0, 17.0/65.0, 47.0/65.0,
			2.0/65.0, 64.0/65.0,  5.0/65.0, 59.0/65.0},
		    {35.0/65.0, 29.0/65.0, 40.0/65.0, 26.0/65.0,
			55.0/65.0,  9.0/65.0, 52.0/65.0, 14.0/65.0},
		    {32.0/65.0, 34.0/65.0, 27.0/65.0, 37.0/65.0,
			12.0/65.0, 54.0/65.0, 15.0/65.0, 49.0/65.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = row[v] > thresh[rownum%8][v%8];
	}
	break;
    case HT_OD3:
	{
	    static double   thresh[8][8] =
		{
		    {  1.0/128.0,  65.0/128.0,  17.0/128.0,  81.0/128.0,
			  5.0/128.0,  69.0/128.0,  21.0/128.0,  85.0/128.0},
		    { 97.0/128.0,  33.0/128.0, 113.0/128.0,  49.0/128.0,
			101.0/128.0,  37.0/128.0, 117.0/128.0,  53.0/128.0},
		    { 25.0/128.0,  89.0/128.0,   9.0/128.0,  73.0/128.0,
			 29.0/128.0,  93.0/128.0,  13.0/128.0,  77.0/128.0},
		    {121.0/128.0,  57.0/128.0, 105.0/128.0,  41.0/128.0,
			125.0/128.0,  61.0/128.0, 109.0/128.0,  45.0/128.0},
		    {  7.0/128.0,  71.0/128.0,  23.0/128.0,  87.0/128.0,
			  3.0/128.0,  67.0/128.0,  19.0/128.0,  83.0/128.0},
		    {103.0/128.0,  39.0/128.0, 119.0/128.0,  55.0/128.0,
			 99.0/128.0,  35.0/128.0, 115.0/128.0,  51.0/128.0},
		    { 31.0/128.0,  95.0/128.0,  15.0/128.0,  79.0/128.0,
			 27.0/128.0,  91.0/128.0,  11.0/128.0,  75.0/128.0},
		    {127.0/128.0,  63.0/128.0, 111.0/128.0,  47.0/128.0,
			123.0/128.0,  59.0/128.0, 107.0/128.0,  43.0/128.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = row[v] >= thresh[rownum%8][v%8];
	}
	break;
    case HT_OD4:
	{
	    static double   thresh[8][8] =
		{
		    { 1.0/65.0, 48.0/65.0, 18.0/65.0, 63.0/65.0,
			 6.0/65.0, 43.0/65.0, 21.0/65.0, 60.0/65.0},
		    {32.0/65.0, 49.0/65.0, 15.0/65.0, 34.0/65.0,
			27.0/65.0, 54.0/65.0, 12.0/65.0, 37.0/65.0},
		    {35.0/65.0, 14.0/65.0, 52.0/65.0, 29.0/65.0,
			40.0/65.0,  9.0/65.0, 55.0/65.0, 26.0/65.0},
		    {62.0/65.0, 19.0/65.0, 45.0/65.0,  4.0/65.0,
			57.0/65.0, 24.0/65.0, 42.0/65.0,  7.0/65.0},
		    {11.0/65.0, 38.0/65.0, 28.0/65.0, 53.0/65.0,
			16.0/65.0, 33.0/65.0, 31.0/65.0, 50.0/65.0},
		    {22.0/65.0, 59.0/65.0,  5.0/65.0, 44.0/65.0,
			17.0/65.0, 64.0/65.0,  2.0/65.0, 47.0/65.0},
		    {41.0/65.0,  8.0/65.0, 58.0/65.0, 23.0/65.0,
			46.0/65.0,  3.0/65.0, 61.0/65.0, 20.0/65.0},
		    {56.0/65.0, 25.0/65.0, 39.0/65.0, 10.0/65.0,
			51.0/65.0, 30.0/65.0, 36.0/65.0, 13.0/65.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = row[v] >= thresh[rownum%8][v%8];
	}
	break;
    case HT_FS:
    case HT_FS2:
	{
	    for (v = 0; v < ncols; v++)
		rowbuf[0][v+1] =
		    rowbuf[1][v+1] + ( (row[v] < 0.0) ? 0.0
				     : (row[v] > 1.0) ? 1.0
				     : (alg == HT_FS2) ? row[v]*row[v]
				     : row[v] );
    
	    rowbuf[1][1] = 0.0;
	    for (v = 0; v < ncols; v++)
	    {
		double	alpha =	7.0/16.0,
			beta =	3.0/16.0,
			gamma =	5.0/16.0,
			delta = 1.0/16.0;
		double	err;

		dotbuf[v] = (rowbuf[0][v+1] < 0.5) ? 0 : 1;
		err = rowbuf[0][v+1] - dotbuf[v];
		rowbuf[0][v+2]	+= err*alpha;
		rowbuf[1][v+2]	 = err*delta; /* Yes, '=' here, not '+='. */
		rowbuf[1][v+1]	+= err*gamma;
		rowbuf[1][v]	+= err*beta;
	    }
	}
	break;
    case HT_DK1:
	{
	    
	}
	break;
    case HT_16LVL:
	{
	    for (v = 0; v < ncols; v++)
		dotbuf[v] = ( (row[v] < 0.0) ? 0
			    : (row[v] > 1.0) ? 15
			    : (int) (15.0 * row[v] + 0.5) );
	}
	break;
    case HT_16OD1:
	{
	    static double   thresh[4][4] =
		{
    		    {14.5/16.0,  0.5/16.0,  7.5/16.0,  9.5/16.0},
		    { 3.5/16.0, 13.5/16.0, 10.5/16.0,  4.5/16.0},
		    { 8.5/16.0,  6.5/16.0,  1.5/16.0, 15.5/16.0},
		    { 5.5/16.0, 11.5/16.0, 12.5/16.0,  2.5/16.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = ( (row[v] < 0.0) ? 0
			    : (row[v] > 1.0) ? 15
			    : (int) (15.0 * row[v] + thresh[rownum%4][v%4]) );
	}
	break;
    case HT_16OD1_2:
	{
	    static double   thresh[4][4] =
		{
    		    {14.5/16.0,  0.5/16.0,  7.5/16.0,  9.5/16.0},
		    { 3.5/16.0, 13.5/16.0, 10.5/16.0,  4.5/16.0},
		    { 8.5/16.0,  6.5/16.0,  1.5/16.0, 15.5/16.0},
		    { 5.5/16.0, 11.5/16.0, 12.5/16.0,  2.5/16.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = ( (row[v] < 0.0) ? 0
			    : (row[v] > 1.0) ? 15
			    : (int) (15.0 * row[v] * row[v]
						+ thresh[rownum%4][v%4]) );
	}
	break;
    case HT_16OD1_3:
	{
	    static double   thresh[4][4] =
		{
    		    {14.5/16.0,  0.5/16.0,  7.5/16.0,  9.5/16.0},
		    { 3.5/16.0, 13.5/16.0, 10.5/16.0,  4.5/16.0},
		    { 8.5/16.0,  6.5/16.0,  1.5/16.0, 15.5/16.0},
		    { 5.5/16.0, 11.5/16.0, 12.5/16.0,  2.5/16.0}
		};

	    for (v = 0; v < ncols; v++)
		dotbuf[v] = ( (row[v] < 0.0) ? 0
			    : (row[v] > 1.0) ? 15
			    : (int) (15.0 * row[v] * row[v] * row[v]
						+ thresh[rownum%4][v%4]) );
	}
	break;
    }

    (*dev_row)(dotbuf);
    rownum++;
}

void
dot_fin()
{
    switch (alg)
    {
    case HT_OD1:
    case HT_OD2:
    case HT_OD3:
    case HT_OD4:
	break;
    case HT_FS:
    case HT_FS2:
	arr_free((char *) rowbuf, 2, DOUBLE, 0);
	break;
    case HT_DK1:
	break;
    case HT_16LVL:
	break;
    case HT_16OD1:
    case HT_16OD1_2:
    case HT_16OD1_3:
	break;
    }

    arr_free((char *) dotbuf, 1, CHAR, 0);
    if (debug_level >= 2)
	Fprintf(stderr, "Finish converting gray levels to dots.\n");
}

/*
 * for debug printout of floating arrays
 */

void
pr_darray(text, n, arr)
    char    *text;
    int	    n;
    double  *arr;
{
    int	    i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr, "\n");
    }
}

