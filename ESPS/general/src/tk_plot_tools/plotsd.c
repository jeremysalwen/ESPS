/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Joe Buck (originally)
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */


static char *sccs_id = "@(#)plotsd.c	1.2 7/23/98    ERL";
int	debug_level = 0;

/*----------------------------------------------------------------------+
|									|
+-----------------------------------------------------------------------+
|									|
|  plotsd -- plot ESPS sample data in a form suitable for plotas	|
|									|
|  Joseph T. Buck, EPI							|
|  adapted from a program by Shankar Narayan.				|
|  Converted 1986 Apr 22 by Rod Johnson to read new SPS format.		|
|  Modified for SPS Common by Ajaipal S. Virdy on 9/2/86		|
|  Modified for -m, -y, and -Y options by John Shore on 9/19/86		|
|  Modified for -E option by Rod Johnson Sept 1987.			|
|  Modified for FEA_SD by John Shore, Oct. 1989                         |
|									|
+----------------------------------------------------------------------*/

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>

#define PROGNAME "plotsd"
#define SYNTAX \
USAGE("plotsd [-{prs} range] [-t text]... [-x level] [-y range]\n\
 [-E expansion] [-M maxpoints] [-Y range] file")
#define TMPNAME "plsdXXXXXX"
#define BUFSIZE 1000
#define MAXTEXT 10
#define Printf (void) printf

void	plotscale(), plotexscale();
void	lrange_switch(), frange_switch();
char	*eopen();
char    *e_temp_name();

main(argc, argv)
    int     argc;
    char    **argv;
{
    char	*ProgName = PROGNAME;

    extern int	optind;		/* Used in command-line parsing by getopt */
    extern char	*optarg;	/* Used in command-line parsing by getopt */
    int		c;		/* Command-line option letter */

    long	nan;		/* Number of points in range to be plotted */
    long	ndrec;		/* Number of points in file */
    long	start_p,	/* First point in range to be plotted */
		last_p;		/* Last point in range to be plotted */
    long	tag0, tag1;	/* Endpoints of unexpanded range (E option) */
    int		xdp,		/* No. of decimal places for x scale marks */
		ydp;		/* No. of decimal places for y scale marks */
    long	i;		/* Loop index */
    int		ix, iy;		/* "Stanford" plot universe coordinates */
    long	numpoints;	/* Actual number of points plotted */
    long	maxpoints = 1000;   /* Approx. maximum no. of points to plot */
				  /* (if nan>maxpoints, points are skipped) */

    int		ntext = 0;	/* Number of lines of text below plot */
    int		pflag = NO,	/* -p option specified? */
		yflag = NO;	/* -y or -Y option specified? */
    int		yexact = 0;	/* -Y option specified? */
    double	expand = 0.0;	/* Expansion fraction specified with -E */
    int		Eflag = NO;	/* -E option specified? */
    char        *Wflag = NULL;  /* used to specify x geometry */
    char        *title = NULL;  /* used for holding user-specifed title -
				 not used yet here; see aplot.c */

    char	*srange = NULL;	/* Range (sec) specified with -s option */
    int		sflag = NO;	/* -s option specified? */

    char	*text[MAXTEXT];	/* Lines of text below plot */
    char	*prange = NULL;	/* Range (samples) specified with -p */
    char	*filename = NULL;   /* Input file name */

    float	*data;		/* Data to plot */
    int		interval;	/* Skip (interval - 1) pts. for each plotted */
    
    double	xmin, xmax;	/* Ends of x axis (data coordinates) */
    double	ymin , ymax;	/* Ends of y axis (data coordinates) */
    float	yminf, ymaxf;	/* Ends of y axis (data coordinates) */
    double	xstep, ystep;	/* Spacing between ticks on axes */
    int		nogrid=0;	/* suppress horizontal grid if 1 */

    FILE	*istrm;		/* Input file pointer */
    struct header   *h;		/* Input file header */
    int		Dflag=0;
    char	s[80];

/* parse command line for parameters */

    while ((c = getopt(argc, argv, "r:p:s:t:x:y:E:M:Y:D:W:")) != EOF)
	switch (c) 
        {
	    case 'r':
	    case 'p':
		prange = optarg;
		pflag++;
		break;
	    case 's':
		srange = optarg;
		sflag++;
		break;
	    case 't':
	        if (ntext < MAXTEXT)
		    text[ntext++] = optarg;
		else 
                {
		    Fprintf(stderr, "%s: Too many -t options\n", ProgName);
		    exit(1);
		}
		break;
	    case 'x':
		debug_level = atoi(optarg);
		break;
	    case 'y': 
		frange_switch(optarg, &ymin, &ymax);
		yflag++;
		break;
	    case 'E':
		expand = atof(optarg);
		Eflag = YES;
		break;
	    case 'M':
		maxpoints = atoi(optarg);
		break;
	    case 'Y': 
		frange_switch(optarg, &ymin, &ymax);
		yflag++;
		yexact = 1;
		break;
	    case 'D':
		Dflag = 1;
		break;
	    case 'W':
		Wflag = optarg;
	        break;
	    default:
		SYNTAX;
	}

/* Process input file name and read parameter file */
	
    if (argc != optind) filename = argv[optind];

    (void) read_params((char *) NULL, SC_CHECK_FILE, filename);

    if (filename == NULL)
    {
	if(symtype("filename") == ST_UNDEF)
	{
	    Fprintf(stderr, "%s: no input file\n", ProgName);
	    SYNTAX
	}
	filename = getsym_s("filename");
    }
     
    filename = eopen(ProgName, filename, "r", FT_FEA, FEA_SD, &h, &istrm);

    if (get_fea_siz("samples", h, (short) NULL, (long) NULL) != 1) {
      Fprintf(stderr, "%s: sorry, can't work on multi-channel files\n",
	      ProgName);
      exit(1);
    }

    tk_init(PROGNAME, filename, Wflag, title);

/* Find x-axis limits */

    if (pflag && sflag)
    {
	Fprintf(stderr,
	"%s: conflicting options: -p and -s\n", ProgName);
	exit(1);
    }

    start_p = 1;
    last_p = LONG_MAX;

    if (!pflag && !sflag)
    {
    	if (symtype("start") != ST_UNDEF) start_p = getsym_i("start");
    	if (symtype("nan") != ST_UNDEF) last_p = start_p + getsym_i("nan") - 1;
    }

    if (pflag) lrange_switch(prange, &start_p, &last_p, 0);

    if (sflag)
    {
	double	start_s,	    /* Beginning of range (sec) */
		last_s;		    /* End of range (sec) */

	/* Sampling frequency */
	double	sf = get_genhd_val("record_freq", h, 1.0);

	if (sf == 0.0) sf = 1.0;
	start_s = 0.0;
	last_s = FLT_MAX;
	frange_switch(srange, &start_s, &last_s);
	start_p = start_s * sf + 1;
	if (last_s == FLT_MAX)
	    last_p = LONG_MAX;
	else
	    last_p = last_s * sf + 1;
    }

    if (h->common.ndrec != -1)	/* Input is file. */
	ndrec = h->common.ndrec;    /*  Get ndrec from header. */
    else			/* Input is pipe. */
    {				    /* Use temp file to count. */
	char		*template = TMPNAME;
	char		*tempname;
	FILE		*tmpstrm;
	static float	buf[BUFSIZE];
	int		num_read;

	tempname = e_temp_name(template);
	TRYOPEN(ProgName, tempname, "w+", tmpstrm);
	(void) unlink(tempname); 
	ndrec = 0;
	do
	{
	    num_read = get_sd_recf(buf, BUFSIZE, h, istrm);
	    if (num_read != 0) put_sd_recf(buf, num_read, h, tmpstrm);
	    ndrec += num_read;
	} while (num_read == BUFSIZE);
	(void) rewind(tmpstrm);
	Fclose(istrm);
	istrm = tmpstrm;
    }

    if (last_p > ndrec) last_p = ndrec;

    if (start_p >= last_p)
    {
	 Fprintf(stderr, "%s: start point after end point!\n", ProgName);
	 exit(1);
    }

    nan = last_p - start_p + 1;

/* Write to ESPS common file before -E alters limits. */

    (void) putsym_s("filename", filename);
    (void) putsym_s("prog", ProgName);
    (void) putsym_i("start", (int) start_p);
    (void) putsym_i("nan", (int) nan);
    
    if (Eflag)			/* Expand range (-E option) */
    {
	tag0 = start_p; tag1 = last_p;
	if (expand < 0.0) expand = 0.0;
	    
	expand *= 0.5 * nan;
	plotscale((double) start_p - expand, (double) last_p + expand,
			1.0, &xmin, &xmax, &xstep, &xdp);
	if (xmin > 1)
	    start_p = LROUND(xmin);
	else
	    start_p = 1;
	if (xmax < ndrec)
	    last_p = LROUND(xmax);
	else
	    last_p = ndrec;
	nan = last_p - start_p + 1;
    }
    else
	plotscale((double) start_p, (double) last_p, 1.0, &xmin, &xmax, &xstep, &xdp);

    if (debug_level)
	Fprintf(stderr,
	"%s: start_p = %ld, nan = %ld, last_p = %ld, filename = %s\n",
	ProgName, start_p, nan, last_p, filename);

/*
 * Whew!
 * Begin program:
 *
 */

    if (debug_level)
	Fprintf(stderr, "%s: wrote all symbols\n", ProgName);

    if (last_p > ndrec) 
    {
	Fprintf(stderr, "%s: only %d points in file\n", ProgName, ndrec);
	exit(1);
    }

/* Read data. */

    skiprec(istrm, (long) start_p - 1, size_rec(h));

    if ( (data = (float *) malloc((unsigned) nan * sizeof(float))) == NULL )
    {
	Fprintf(stderr, "%s: can't allocate memory for %d data points\n",
		 ProgName, nan);
	exit(1);
    }

    if (debug_level)
	Fprintf(stderr, "%s: getting sampled data record\n", ProgName);

    (void) get_sd_recf(data, (int) nan, h, istrm);

    if (nan < maxpoints || maxpoints == 0)  /* Plot all pts. in range */
    {
	interval = 1;
	numpoints = nan;
    }
    else			/* Skip points */
    {
	interval = nan / maxpoints;
        numpoints = nan / interval;
    }

/* Get y-axis limits. */

    if (!yflag)
    {   
	yminf = ymaxf = (nan == 0) ? 0.0 : data[0];
        for (i = 1; i < nan; i++) 
	{
	    if (data[i] < yminf)
		yminf = data[i];
	    else if (data[i] > ymaxf)
		ymaxf = data[i];
	}
	ymin = (double) yminf;
	ymax = (double) ymaxf;
	if ((ymax - ymin) < .001) {
		ymax+=.001;
		nogrid = 1;
	}
    }



    if (debug_level)
	Fprintf(stderr, "%s: plotting points\n", ProgName);

    if (yexact) 
	plotexscale(ymin, ymax, 1.0, &ymin, &ymax, &ystep, &ydp);
    else
	plotscale(ymin, ymax, 1.0, &ymin, &ymax, &ystep, &ydp);

    draw_box(xmin, xmax, xstep, xdp, ymin, ymax, ystep, ydp, nogrid);
    tk_snd_plot_cmd("c 2");
    plot_dataf(data, numpoints, xmin, xmax, ymin, ymax, start_p, interval);
    tk_snd_plot_cmd("c 5");
    tk_snd_plot_cmd("m 3400 500");
    tk_snd_plot_cmd("t 5 1");
    sprintf(s,"File: %s",filename);
    tk_snd_plot_cmd(s);

    if (Eflag)
    {
	iy = 3000 + ymin * 2500.0 / (ymax - ymin);

	ix = 500.0 + (tag0 - xmin) * 5000.0 / (xmax - xmin);
	sprintf(s,"m %d %d", iy, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"d 500 %d", ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"m 480 %d",ix);
        tk_snd_plot_cmd("t 4 2");
	sprintf(s,"%d", tag0);
        tk_snd_plot_cmd(s);

	ix = 500.0 + (tag1 - xmin) * 5000.0 / (xmax - xmin);
	sprintf(s,"m %d %d", iy, ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"d 500 %d", ix);
	tk_snd_plot_cmd(s);
	sprintf(s,"m 480 %d",ix);
        tk_snd_plot_cmd(s);
        tk_snd_plot_cmd("t 4 2");
        sprintf(s,"%d", tag1);
	tk_snd_plot_cmd(s);
    }

    iy = 3600;
    for (i = 0; i < ntext; i++) 
    {
	tk_snd_plot_cmd("c 5");
        tk_snd_plot_cmd_1arg("m %d 500", iy);
	tk_snd_plot_cmd("t 5 1");
        tk_snd_plot_cmd(text[i]);
	iy += 200;
    }

    if(!Dflag) /* supress date */
    	print_time(200, 4400);

    tk_start();
    
    (void) putsym_i("beginplot", (int) xmin);
    (void) putsym_i("endplot", (int) xmax);

    exit(0);
    /*NOTREACHED*/
}

