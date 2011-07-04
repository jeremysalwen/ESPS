/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 * "Copyright (c) 1986, 1987, 1989 Entropic Speech, Inc. All rights reserved."
 *
 *
 * Module: aplot - A program for generating x-y plots of ascii data
 *
 * Written By: S. Shankar Narayan
 * -z option added by David Burton
 * USAGE: aplot [-d] [-l] [-z] [input_file]
 * -d --- dot plot (i.e., a dot for each point)
 * -l --- line plot (i.e, vertical line for each value)
 * -z suppress grid lines
 * inp_file - input file with data and scaling information.
 *
 */
#ifndef lint
    static char *sccs_id = "@(#)aplot.c	3.8	10/20/93	ESI";
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define ProgName "aplot"
#define Printf (void) printf

#define SYNTAX \
USAGE("aplot [-d] [-l] [-z] [-T] [input_file]");
		    /* -T handled by cover script, not C progam. */

main (argc, argv)
    int     argc;
    char    *argv[];
{
    int     line_plot_flag = 0,
	    dot_plot_flag = 0,
	    normal_flag = 1,
	    zflag = 0;			/* flag for suppress grid option*/
    float   xmin, xmax, xstep, ymin, ymax, ystep;
    int     color, i, j, ix, iy;
    float   x, y;
    int     newline_flag,
	    lnt;
    int	    c;
    FILE    *fd = stdin;
    extern char	*optarg;
    extern  optind;

    while ((c = getopt (argc, argv, "dlz")) != EOF)
    {
	switch (c)
	{
	    case 'd': 
		dot_plot_flag++;
		break;
	    case 'l': 
		line_plot_flag++;
		break;
	    case 'z':
		zflag++;
		break;
	    default: 
		SYNTAX;
	}
    }
    if (line_plot_flag && dot_plot_flag)
    {
	Fprintf(stderr, "%s: Cannot specify both -d and -l flags\n",
		ProgName);
	exit (1);
    }
    if (line_plot_flag || dot_plot_flag)
	normal_flag = 0;

    if (optind < argc - 1)
    {
	Fprintf(stderr, "%s: more than one file name.\n", ProgName);
	SYNTAX;
    }

    if (optind < argc && strcmp(argv[optind], "-") != 0)
	TRYOPEN ("aplot", argv[optind], "r", fd);

    if (fscanf (fd, "%d", &lnt) == EOF)
    {
	Fprintf(stderr, "%s: EOF encountered\n", ProgName);
	exit (1);
    }
    if (fscanf (fd, "%d %f %f %f %f %f %f",
		&color, &xmin, &xmax, &xstep, &ymin, &ymax, &ystep) == EOF)
    {
	Fprintf(stderr, "%s: EOF encountered\n", ProgName);
	exit (2);
    }

    draw_box (xmin, xmax, xstep, ymin, ymax, ystep, !zflag);

    for (j = 0; j < color; j++)
    {
	Printf("f %d\n", 2 * j + 1);
	for (i = 0; i < lnt; i++)
	{
	    if (fscanf (fd, "%f %f", &x, &y) == NULL)
	    {
		Fprintf(stderr, "%s: EOF encountered\n", ProgName);
		exit (1);
	    }
	    iy = 3000 - 2500.0 * (y - ymin) / (ymax - ymin);
	    ix = 500 + 5000.0 * (x - xmin) / (xmax - xmin);
	    if (normal_flag)
	    {
		if (i == 0)
		    Printf("m %d %d\n", iy, ix);
		if (i != 0)
		    Printf("d %d %d\n", iy, ix);
	    }
	    if (dot_plot_flag)
	    {
		Printf("m %d %d\n", iy - 1, ix - 1);
		Printf("d %d %d\n", iy + 1, ix + 1);
		Printf("m %d %d\n", iy + 1, ix - 1);
		Printf("d %d %d\n", iy - 1, ix + 1);
	    }
	    if (line_plot_flag)
	    {
		if (ymin < 0 && ymax > 0)
		{
		    int     zero = 3000 + 2500.0 * ymin / (ymax - ymin);
		    Printf("m %d %d\n", zero, ix);
		    Printf("d %d %d\n", iy, ix);
		}
		else
		{
		    Printf("m 3000 %d\n", ix);
		    Printf("d %d %d\n", iy, ix);
		}
	    }
	}
    }

    Printf("f 1\n");
    /* print header for the plot */
    newline_flag = 0;
    iy = 3400;
    i = getc (fd);
    while ((i = getc (fd)) > 0)
    {
	if (newline_flag == 1)
	{
	    newline_flag = 0;
	    if (i == '#')
	    {
		i = getc (fd);
		Printf("f %c\n", i);
		Printf("m %d 500\nt 5 1\n", iy += 200);
	    }
	    else
	    {
		Printf("m %d 500\nt 5 1\n", iy += 200);
		putchar (i);
	    }
	}
	else
	    putchar (i);
	if (i == 13 || i == 10)
	    newline_flag = 1;

    }

    print_time (200, 4400);

    exit(0);
    /*NOTREACHED*/
}

print_time (x, y)
int     x, y;
{
#ifndef DEC_ALPHA
    long    time (); 	
#endif
    long    tloc;
    char   *ctime (), *tptr, tout[26];
    int     i;

    Printf("\nc 2");
    Printf("\nm %d %d\nt 5 1\n", x, y);

    (void) time (&tloc);
    tptr = ctime (&tloc);
    for (i = 0; i < 3; i++)
	tout[i] = tptr[i + 8];	/* day */
    for (i = 3; i < 7; i++)
	tout[i] = tptr[i + 1];	/* month */
    for (i = 7; i < 11; i++)
	tout[i] = tptr[i + 13];	/* year */
    for (i = 11; i < 17; i++)
	tout[i] = tptr[i - 1];	/* time of day */
    tout[17] = '\n';
    tout[18] = '\0';
    Printf("%s\n", tout);
    Printf("\nm 0 0\n");
}

 /* Draw box */

draw_box (xmin, xmax, xstep, ymin, ymax, ystep, grid)
    float   xmin, xmax, xstep, ymin, ymax, ystep;
    int	    grid;
{
    char    label[50];
    float   tmp;
    int     ix, iy;

    Printf("\nc 1\n");

    Printf("m 500 500\n");
    Printf("d 500 5500\nd 3000 5500\nd 3000 500\nd  500 500\n");

    Printf("c 2\n");

    if(grid){

	/* Draw vertical grid */

	for (tmp = xmin; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    Printf("m 500 %4d\n", ix);
	    Printf("d 3000 %4d\n", ix);

	    Sprintf(label, "%g", (fabs(tmp) < 0.001*xstep) ? 0.0 : tmp);
	    Printf("m 3200 %4d\nt 5 1\n", ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	    Printf("%s\n", label);
	}


	/* Draw ticks */
	for (tmp = xmin + xstep / 2; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    Printf("m 475 %4d\n", ix);
	    Printf("d 525 %4d\n", ix);
	    Printf("m 2975 %4d\n", ix);
	    Printf("d 3025 %4d\n", ix);
	}

	/* Draw horizontal grid */
	for (tmp = ymin; tmp <= ymax + 0.001*ystep; tmp = tmp + ystep)
	{
	    iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	    Printf("m %4d 5500\n", iy);
	    Printf("d %4d 500\n", iy);
	    Sprintf(label, "%g", (fabs(tmp) < 0.001*ystep) ? 0.0 : tmp);
	    Printf("m  %4d %d\nt 5 1\n", iy + 50, 400 - 80*strlen(label));
			/* 1/2 height of size 5 digit is 50 */
	    Printf("%s\n", label);
	}
    }
    else
    {/* don't draw the grid - just ticks */
	/* Draw x axis ticks */

	for (tmp = xmin; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    Printf("m 475 %4d\n", ix);
	    Printf("d 525 %4d\n", ix);
	    Printf("m 2975 %4d\n", ix);
	    Printf("d 3025 %4d\n", ix);

	    Sprintf(label, "%g", (fabs(tmp) < 0.001*xstep) ? 0.0 : tmp);
	    Printf("m 3200 %4d\nt 5 1\n", ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	    Printf("%s\n", label);
	}

	/* Draw the y axis ticks*/
	for (tmp = ymin; tmp <= ymax + 0.001*ystep; tmp = tmp + ystep)
	{
	    iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	    Printf("m %4d 525\n", iy);
	    Printf("d %4d 475\n", iy);
	    Sprintf(label, "%g", (fabs(tmp) < 0.001*ystep) ? 0.0 : tmp);
	    Printf("m  %4d %d\nt 5 1\n", iy + 50, 400 - 80*strlen(label));
			/* 1/2 height of size 5 digit is 50 */
	    Printf("%s\n", label);
	}
    }
}
