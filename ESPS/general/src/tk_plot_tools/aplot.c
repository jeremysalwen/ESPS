/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 * "Copyright (c) 1997 Entropic Research Lab, Inc. All rights reserved."
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
    static char *sccs_id = "@(#)aplot.c	1.3 7/21/98	ERL";
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define ProgName "aplot"
#define Printf (void) printf

#define SYNTAX \
USAGE("aplot [-d] [-l] [-z] [-T title_string] [input_file]");

int debug_level = 0;

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
    char    s[80];
    char    *Wflag = NULL;
    char    *filename = NULL;
    char    *title = NULL;

    while ((c = getopt (argc, argv, "dlzT:W:")) != EOF)
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
	    case 'W':
		Wflag = optarg;
	        break;
	    case 'T':
		title = optarg;
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

    if (optind < argc && strcmp(argv[optind], "-") != 0) {
	TRYOPEN ("aplot", argv[optind], "r", fd);
	filename = argv[optind];
    }

    tk_init("aplot", filename, Wflag, title);
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

    tk_snd_plot_cmd("set_class data");
    for (j = 0; j < color; j++)
    {
	sprintf(s,"f %d", 2 * j + 1);
	tk_snd_plot_cmd(s);
	tk_snd_plot_cmd("c 3");
	for (i = 0; i < lnt; i++)
	{
	    if (fscanf (fd, "%f %f", &x, &y) != 2)
	    {
		Fprintf(stderr, "%s: EOF encountered\n", ProgName);
		exit (1);
	    }
	    iy = 3000 - 2500.0 * (y - ymin) / (ymax - ymin);
	    ix = 500 + 5000.0 * (x - xmin) / (xmax - xmin);
	    if (normal_flag)
	    {
		if (i == 0) {
		    sprintf(s,"m %d %d", iy, ix);
		    tk_snd_plot_cmd(s);
		}
		if (i != 0) {
		    sprintf(s,"d %d %d", iy, ix);
		    tk_snd_plot_cmd(s);
		}
	    }
	    if (dot_plot_flag)
	    {
		sprintf(s,"l %d %d %d %d", iy - 1, ix - 1, iy +1, ix + 1);
		tk_snd_plot_cmd(s);
		sprintf(s,"l %d %d %d %d", iy + 1, ix - 1, iy -1, ix + 1);
		tk_snd_plot_cmd(s);
	    }
	    if (line_plot_flag)
	    {
		if (ymin < 0 && ymax > 0)
		{
		    int     zero = 3000 + 2500.0 * ymin / (ymax - ymin);
		    sprintf(s,"l %d %d %d %d", zero, ix, iy, ix);
		    tk_snd_plot_cmd(s);
		}
		else
		{
		    sprintf(s,"l 3000 %d %d %d", ix, iy, ix);
		    tk_snd_plot_cmd(s);
		}
	    }
	}
    }

    tk_snd_plot_cmd("f 1");
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
/*
		sprintf(s,"f %c", i);
		tk_snd_plot_cmd(s);
		sprintf(s,"m %d 500", iy += 200);
		tk_snd_plot_cmd(s);
		tk_snd_plot_cmd("t 5 1");
*/
	    }
	    else
	    {
/*
		sprintf(s,"m %d 500", iy += 200);
		tk_snd_plot_cmd(s);
		tk_snd_plot_cmd("t 5 1");
		putchar (i);
*/
	    }
	}
/*
	else
	    putchar (i);
*/
	if (i == 13 || i == 10)
	    newline_flag = 1;

    }

    print_time (200, 4400);

    tk_start();
    exit(0);
    /*NOTREACHED*/
}

 /* Draw box */

draw_box (xmin, xmax, xstep, ymin, ymax, ystep, grid)
    float   xmin, xmax, xstep, ymin, ymax, ystep;
    int	    grid;
{
    char    label[50];
    char    s[80];
    float   tmp;
    int     ix, iy;

    tk_snd_plot_cmd("c 1");
    tk_snd_plot_cmd("s 3");
    tk_snd_plot_cmd("set_class box");

    tk_snd_plot_cmd("m 500 500");
    tk_snd_plot_cmd("d 500 5500");
    tk_snd_plot_cmd("d 3000 5500");
    tk_snd_plot_cmd("d 3000 500");
    tk_snd_plot_cmd("d  500 500");

    tk_snd_plot_cmd("c 6");
    tk_snd_plot_cmd("s 1");

    if(grid){

	/* Draw vertical grid */

        tk_snd_plot_cmd("set_class v_grid");
	for (tmp = xmin; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    tk_snd_plot_cmd_1arg("m 500 %4d", ix);
	    tk_snd_plot_cmd_1arg("d 3000 %4d", ix);

	    Sprintf(label, "%g", (fabs(tmp) < 0.001*xstep) ? 0.0 : tmp);
	    tk_snd_plot_cmd_1arg("m 3200 %4d", ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd(label);
	}


	/* Draw ticks */
        tk_snd_plot_cmd("set_class ticks");
	for (tmp = xmin + xstep / 2; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    tk_snd_plot_cmd_1arg("m 475 %4d", ix);
	    tk_snd_plot_cmd_1arg("d 525 %4d", ix);
	    tk_snd_plot_cmd_1arg("m 2975 %4d", ix);
	    tk_snd_plot_cmd_1arg("d 3025 %4d", ix);
	}

	/* Draw horizontal grid */
        tk_snd_plot_cmd("set_class h_grid");
	for (tmp = ymin; tmp <= ymax + 0.001*ystep; tmp = tmp + ystep)
	{
	    iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	    tk_snd_plot_cmd_1arg("m %4d 5500", iy);
	    tk_snd_plot_cmd_1arg("d %4d 500", iy);
	    Sprintf(label, "%g", (fabs(tmp) < 0.001*ystep) ? 0.0 : tmp);
	    sprintf(s,"m  %4d %d", iy + 50, 400 - 80*strlen(label));
			/* 1/2 height of size 5 digit is 50 */
	    tk_snd_plot_cmd(s);
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd(label);
	}
    }
    else
    {/* don't draw the grid - just ticks */
	/* Draw x axis ticks */

        tk_snd_plot_cmd("set_class ticks");
	for (tmp = xmin; tmp <= xmax + 0.001*xstep; tmp = tmp + xstep)
	{
	    ix = 500 + 5000 * (tmp - xmin) / (xmax - xmin);
	    tk_snd_plot_cmd_1arg("m 475 %4d", ix);
	    tk_snd_plot_cmd_1arg("d 525 %4d", ix);
	    tk_snd_plot_cmd_1arg("m 2975 %4d", ix);
	    tk_snd_plot_cmd_1arg("d 3025 %4d", ix);

	    Sprintf(label, "%g", (fabs(tmp) < 0.001*xstep) ? 0.0 : tmp);
	    tk_snd_plot_cmd_1arg("m 3200 %4d", ix - 40*strlen(label));
			/* 1/2 width of size 5 character is 40 */
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd(label);
	}

	/* Draw the y axis ticks*/
	for (tmp = ymin; tmp <= ymax + 0.001*ystep; tmp = tmp + ystep)
	{
	    iy = 3000 - 2500 * (tmp - ymin) / (ymax - ymin);
	    tk_snd_plot_cmd_1arg("m %4d 525", iy);
	    tk_snd_plot_cmd_1arg("d %4d 475", iy);
	    Sprintf(label, "%g", (fabs(tmp) < 0.001*ystep) ? 0.0 : tmp);
	    sprintf(s,"m  %4d %d", iy + 50, 400 - 80*strlen(label));
			/* 1/2 height of size 5 digit is 50 */
	    tk_snd_plot_cmd(s);
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd(label);
	}
    }
    tk_snd_plot_cmd("c 4");
}
