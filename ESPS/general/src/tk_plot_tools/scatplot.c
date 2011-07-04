/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|	 "Copyright (c) 1986, 1987, 1988, 1990 Entropic Speech, Inc.		|
|			  All rights reserved."				|
|									|
+-----------------------------------------------------------------------+
|									|
|  scatplot - generate a scatter plot in a form suitable for plotas	|
|									|
|  by Ajaipal S. Virdy, ESI						|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)scatplot.c	1.2 7/23/98    ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define SYNTAX \
USAGE("scatplot [-e elements] [-r range] [-s symbols] [-t text] [-x debug]\n [-H text] [-V text] [-X range] [-Y range] file1 [file2] ...")

#define TRYALLOC(type, num, var, name) { \
    if (((var) = (type *) malloc((unsigned)(num)*sizeof(type)))==NULL) \
    { Fprintf(stderr, "%s:  can't allocate memory for %s.\n", \
		ProgName, (name)); \
	exit(1); }}

#define Printf (void) printf

#define MAXTEXT 1

/*
 * The following defines are the coordinates of the upper left hand corner
 * and the lower right corner which define the drawing frame.
 *
 * The coordinate system is defined as follows:
 *
 *		
 *		---------------------------------
 *		|(0,0)		      (0,lowerx)|
 *		|				|
 *		|				|
 *		|				|
 *		|	(y1,x1)			|
 *		|				|
 *		|				|
 *		|(lowery,0)	 (lowery,lowerx)|
 *		--------------------------------------> increasing x
 *		|
 *		|
 *		v increasing y
 *
 */

#define	DEF_UPPER_X 600		/* this number shouldn't be modified */
#define	DEF_UPPER_Y 600		/* this number shouldn't be modified */
#define	DEF_LOWER_X 5500	/* this can be adjusted */
#define	DEF_LOWER_Y 3500	/* this can be adjusted, also */

#define	CHAR_SIZE 35
#define	COM_CHARS 60	/* maximum number of characters that fit on a line */

#define	ODD(n)	(((n/2) * 2) != n)


char	    *eopen();
void	    addstr();
double	    **d_mat_alloc();
char	    *get_cmd_line();
void	    frange_switch();
void	    lrange_switch();
long	    *grange_switch();
void	    plotscale();
void	    plotexscale();
char	    *savestring();
short	    get_rec_len();
char        *e_name_temp();
char        *e_temp_name();

static void	pr_darray(), pr_larray();

extern int	optind;		/* used by getopt() */
extern char	*optarg;	/* used by getopt() */

char	    *ProgName = "scatplot";

int		debug_level = 0;

/*
 * Main Program
 */

main(argc, argv)
    int	    argc;
    char    **argv;
{
    int		c;		/* command-line option letter */

    /*
     *  C O M M A N D
     *  L I N E
     *  O P T I O N S
     */

    char	*erange;	/* -e option argument */
    int		eflag = NO;	/* -e option specified? */
    long	*elem_array;	/* element numbers specified with -e */
    long	nelem;		/* number of elements specified with -e */

    char	**rrange;	/* -r option arguments */
    int		rnum = 0;	/* number of -r options specified */
    long	*nans;		/* number of records in range for each file */
    long	*ndrec;		/* number of records in each file */

    int		Xflag = NO;	/* -X option specified? */
    int		Yflag = NO;	/* -Y option specified? */

    char	*symbol = "xo*abcdefghijklmnpqrstuvwyz";
				/* plotting symbols */

    char	*text[MAXTEXT];	/* text strings specified with -t option */
    int		ntext = 0;	/* number of -t options specified */
    int		tflag = NO;	/* -t option specified? */

    char	*Htext = "";	/* text specified with -H option */
    char	Hflag = NO;	/* -H option specified? */

    char	*Vtext = "";	/* text specified with -V option */
    char	Vflag = NO;	/* -V option specified? */
    char        s[80];
    char 	*popt = NULL;	/* -p option for Eddie */
    char        *Wflag = NULL;  /*  X geometry */
    char        *title = NULL;  /* used for holding user-specifed title -
				 not used yet here; see aplot.c */

    /*
     * I N P U T
     * F I L E S
     *
     */

    char	**files;	/* file names */
    FILE	**filep;	/* file pointers */
    int		i_file;		/* file number (loop index) */
    int		nfiles;		/* number of input files */

    struct header **h;		/* file headers */

    int		si_read = NO;	/* had stdin been copied to temp file? */
    char	*si_temp;	/* temp file for stdin records */
    struct header
		*si_hdr;	/* header read from stdin */
    long	si_nrec;	/* number of records read from stdin */

    /*
     * T E M P O R A R Y
     * V A R I A B L E S
     *
     */

    int		i;		/* loop index */
    int		xdp, ydp;	/* number of decimal places for axis labels */

    double	*data;		/* buffer for input data */
    double	**xdata;	/* vectors of x data from each file */
    double	**ydata;	/* vectors of y data from each file */

    double	x_min, xlow;	/* min x data value, x-axis lower limit */
    double	x_max, xhigh;	/* max x data value, x-axis upper limit */

    double	y_min, ylow;	/* min y data value, y-axis lower limit */
    double	y_max, yhigh;	/* max y data value, y-axis upper limit */

    double	xstep, ystep;	/* spacing between axis labels */

    /*
     *  P L O T T I N G
     *   V A R I A B L E S
     */

    int		upperx = DEF_UPPER_X;	/* plotter coord of x-axis right end */
    int		uppery = DEF_UPPER_Y;	/* plotter coord of y-axis top */
    int		lowerx = DEF_LOWER_X;	/* plotter coord of x-axis left end */
    int		lowery = DEF_LOWER_Y;	/* plotter coord of y-axis bottom */

    int		nchars;		/* length of H or V text string */
    long	tag;		/* tag value read from input record */
    char	command_line[300];  /* command line to print above plot*/
    char	txtbuf[300];	/* used when command_line must be broken */
    char	*tmpptr;	/* pointer into txtbuf */


/* parse command line for parameters */

    Sprintf(command_line, "%% %s", get_cmd_line(argc, argv));

    while ((c = getopt(argc, argv, "e:r:s:t:x:H:V:X:Y:W:")) != EOF)
	switch (c) 
        {
	case 'e':
	    erange = optarg;
	    if (eflag) {
		Fprintf(stderr,
		    "%s: please give only one -e option.\n", ProgName);
		exit(1);
	    }
	    elem_array = grange_switch(erange, &nelem);
	    if (ODD(nelem)) {
		Fprintf(stderr,
		    "%s: -e option: even number of elements required.\n",
		ProgName);
		exit(1);
	    }
	    eflag = YES;
	    break;
	case 'r':
	    if (rnum == 0)
	    {
		TRYALLOC(char *, 1, rrange, "-r option arguments")
		rrange[0] = NULL;
	    }
	    addstr(optarg, &rrange);
	    rnum++;
	    break;
	case 's':
	    symbol = optarg;
	    break;
	case 't':
	    tflag = YES;
	    if (ntext < MAXTEXT)
		text[ntext++] = optarg;
	    else
	    {
		Fprintf(stderr, "plotsd: Too many -t options\n");
		exit(1);
	    }
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'H':
	    Htext = optarg;
	    Hflag = YES;
	    break;
	case 'V':
	    Vtext = optarg;
	    Vflag = YES;
	    break;
	case 'X':
	    frange_switch(optarg, &x_min, &x_max);
	    Xflag = YES;
	    break;
	case 'Y':
	    frange_switch(optarg, &y_min, &y_max);
	    Yflag = YES;
	    break;
	case 'W':
	    Wflag = optarg;
	    break;
	default:
	    SYNTAX
	    ;
	}

    if (!eflag) {
	Fprintf(stderr,
	    "%s: please specify an element range with the -e option.\n",
	    ProgName);
	SYNTAX
	;
    }

/* miscellaneous initializations and consistency checks */

    nfiles = argc - optind;

    if (nfiles == 0)
    {
	Fprintf(stderr, "%s: no input file specified.\n", ProgName);
	SYNTAX
	;
    }

    tk_init("scatplot",NULL,Wflag,title);
    if (debug_level >= 1)
    {
	Fprintf(stderr, "%d input files.\n", nfiles);
	Fprintf(stderr, "Allocating storage for 7 arrays.\n");
    }

    TRYALLOC(long, nfiles, ndrec, "number of data records in each file")
    TRYALLOC(long, nfiles, nans, "number of records in range for each file")
    TRYALLOC(char *, nfiles, files, "file names")
    TRYALLOC(FILE *, nfiles, filep, "file pointers")
    TRYALLOC(struct header *, nfiles, h, "file headers")
    TRYALLOC(double *, nfiles, xdata, "pointers to x data vectors")
    TRYALLOC(double *, nfiles, ydata, "pointers to y data vectors")

    for (i = 0; i < nelem; i++)
	elem_array[i] -= 1;	/* element numbers to data array indices */

    if (nelem < 2*nfiles)
    {
	if (debug_level >= 1)
	    Fprintf(stderr, "Extending elem_array.\n");

	if (( elem_array =
		(long *) realloc((char *) elem_array, 2*nfiles*sizeof(long)) )
	    == NULL)
	{
	    Fprintf(stderr, "%s: can't reallocate storage.\n", ProgName);
	    exit(1);
	}
	for (i = nelem; i < 2*nfiles; i += 2)
	{
	    elem_array[i] = elem_array[nelem - 2];
	    elem_array[i + 1] = elem_array[nelem - 1];
	}
    }

    if (debug_level >= 1)
	pr_larray("elemen numbers (decremented)", 2*nfiles, elem_array);

    if (strlen(symbol) < nfiles)
    {
	Fprintf(stderr,
	    "%s: please specify %d symbols with the -s option.\n",
	    ProgName, nfiles);
	exit(1);
    }

/* open files, read headers, count records, */
/* copy to temp file if necessary. */

    if (debug_level >= 1)
	Fprintf(stderr, "Opening files.\n");

    for (i_file = 0; i_file < nfiles; i_file++, optind++)
    {
	if (debug_level >= 2)
	    Fprintf(stderr,
		"File number %d (%s).\n", i_file + 1, argv[optind]);

	if (strcmp(argv[optind], "-") == 0 && si_read)
	{
	    if (debug_level >= 2)
		Fprintf(stderr,
		    "File is <stdin>.  Opening temp file %s.\n",
		    si_temp);

	    files[i_file] = "<stdin>";
	    TRYOPEN(ProgName, si_temp, "r", filep[i_file])
	    ;
	    h[i_file] = si_hdr;
	    ndrec[i_file] = si_nrec;
	}
	else
	{
	    files[i_file] =
		eopen(ProgName, argv[optind], "r", NONE, NONE,
					&h[i_file], &filep[i_file]);

	    /*
	     * Check for complex data
	    */
	    if(is_file_complex(h[i_file]) != 0){
	      Fprintf(stderr, "scatplot: %s contains complex data.\n\tComplex data files are not yet supported.\n", files[i_file]);
	      exit(1);
	    }

	    if (debug_level >= 2)
		Fprintf(stderr, "File is %s.\n", files[i_file]);

	    ndrec[i_file] = h[i_file]->common.ndrec;

	    if (ndrec[i_file] == -1 || filep[i_file] == stdin)
	    {
		char	*tmpname;   /* name of temporary file */
		FILE	*tmpstrm;   /* temp file pointer */
		int	recsize;    /* number of bytes in input record */
		long	nrec;	    /* number of records read */
		char	*tmpbuf;    /* buffer for input data */

		tmpname = e_temp_name("scatplXXXXXX");
		TRYOPEN(ProgName, tmpname, "w+", tmpstrm)
		;

		if (debug_level >= 2)
		    Fprintf(stderr,
			"Copying records to temp file %s.\n", tmpname);

		recsize = size_rec(h[i_file]);
		TRYALLOC(char, recsize, tmpbuf, "temporary record buffer")

		if (debug_level >= 2)
		    Fprintf(stderr,
			"Allocated space for %d-byte buffer.\n", recsize);
		if (debug_level >= 3)
		    Fprintf(stderr, "Reading records:");

		nrec = 0;
		while (fread(tmpbuf, recsize, 1, filep[i_file]) == 1)
		{
		    if (fwrite(tmpbuf, recsize, 1, tmpstrm) != 1)
		    {
			Fprintf(stderr, "%s: write error on %s\n",
			    ProgName, tmpname);
			exit(1);
		    }
		    nrec++;

		    if (debug_level >= 3)
			Fprintf(stderr, " %ld", nrec);
		}

		if (debug_level >= 3)
		    Fprintf(stderr, "\n");

		if (filep[i_file] == stdin)
		{
		    si_read = YES;
		    si_temp = tmpname;
		    si_hdr = h[i_file];
		    si_nrec = nrec;

		    if (debug_level >= 2)
			Fprintf(stderr, "si_read = %d.\n", si_read);
		}
		else
		{
		    /* Here only if ndrec[i_file] == -1 but
		       filep[i_file] != stdin.  Not sure this
		       can happen. */

		    unlink(tmpname);
		    free(tmpname);
		}

		free(tmpbuf);
		(void) rewind(tmpstrm);
		filep[i_file] = tmpstrm;
		ndrec[i_file] = nrec;
	    }
	}

	if ( (elem_array[2*i_file] == -1 || elem_array[2*i_file + 1] == -1)
	    && !h[i_file]->common.tag )
	{
		Fprintf(stderr,
		    "%s: data is not tagged in %s, cannot plot element 0.\n",
		    ProgName, files[i_file]);
		exit(1);
	}

	if (debug_level >= 2)
	    Fprintf(stderr, "%ld records in file.\n", ndrec[i_file]);
		

    }  /* end for (i_file ... ) */

    if (si_read) unlink(si_temp);

/* read data into arrays, fine extreme x and y values */

    if (debug_level >= 1)
	Fprintf(stderr, "Reading data into memory.\n");

    for (i_file = 0; i_file < nfiles; i_file++)
    {
	int	rec_len;	/* number of elements in input record */
	long	start_p, last_p;    /* first and last records in range */
	long	nan;		/* number of records in range */
	long	x_ele, y_ele;	/* element numbers for x and y data */
	static	first = YES;	/* first trip through loop? */

	if (debug_level >= 2)
	    Fprintf(stderr,
		"File number %d (%s).\n", i_file + 1, files[i_file]);

	start_p = 1;
	last_p = ndrec[i_file];
	if (rnum > 0)
	    lrange_switch(rrange[MIN(i_file, rnum-1)], &start_p, &last_p, 0);

	nans[i_file] =
	    nan = last_p - start_p + 1;

	if (debug_level >=2)
	{
	    Fprintf(stderr,
		"start_p = %ld, nan = %ld, last_p = %ld\n",
		start_p, nan, last_p);
	}

	if (start_p < 1)
	{
	    Fprintf(stderr,
		"%s: start point less than one for file %d.\n",
		ProgName, i_file);
	    exit(1);
	}

	if (start_p > last_p)
	{
	    Fprintf(stderr,
		"%s: start point after end point for file %d.\n",
		ProgName);
	    exit(1);
	}

	if (last_p > ndrec[i_file])
	{
	    Fprintf(stderr,
		"%s: only %ld points in file %d.\n",
		    ProgName, ndrec[i_file], i_file);
	    exit(1);
	}

	skiprec(filep[i_file], start_p - 1, size_rec(h[i_file]));

	rec_len = get_rec_len(h[i_file]);

	if (debug_level >= 2)
	    Fprintf(stderr,
		"allocating space for arrays of lengths %d, %d, %d.\n",
		rec_len, nan, nan);

	TRYALLOC(double, rec_len, data, "input data array")
	TRYALLOC(double, nan, xdata[i_file], "x data")
	TRYALLOC(double, nan, ydata[i_file], "y data")

	x_ele = elem_array[2*i_file];
	y_ele = elem_array[2*i_file + 1];

	if (debug_level >= 2)
	{
	    Fprintf(stderr,
		"Getting data from elements %d, %d.\n",
		x_ele + 1, y_ele + 1);
	}

	for (i = 0; i < nan; i++)
	{
	    if (get_gen_recd(data, &tag, h[i_file], filep[i_file]) == EOF)
	    {
		Fprintf(stderr,
		    "%s: only %d records read in %s\n",
		    ProgName, i, files[i_file]);
		exit(1);
	    }

	    xdata[i_file][i] = (x_ele == -1) ? tag : data[x_ele];
	    ydata[i_file][i] = (y_ele == -1) ? tag : data[y_ele];

	    if (debug_level >= 3)
		Fprintf(stderr,
		    "Read %g, %g.\n", xdata[i_file][i], ydata[i_file][i]);

	    if (first)
	    {
		first = NO;
		if (!Xflag)
		    x_min = x_max = xdata[i_file][i];
		if (!Yflag)
		    y_min = y_max = ydata[i_file][i];
	    }
	    else
	    {
		if (!Xflag)
		    if (xdata[i_file][i] < x_min) x_min = xdata[i_file][i];
		    else
		    if (xdata[i_file][i] > x_max) x_max = xdata[i_file][i];

		if (!Yflag)
		    if (ydata[i_file][i] < y_min) y_min = ydata[i_file][i];
		    else
		    if (ydata[i_file][i] > y_max) y_max = ydata[i_file][i];
	    }
	}

	free((char *) data);
    }

    if (debug_level >= 1)
    {
	Fprintf(stderr, "Done reading data.\n");
	Fprintf(stderr,
	    "x_min = %g, x_max = %g, y_min = %g, y_max = %g\n",
	    x_min, x_max, y_min, y_max);
    }


/* scale and draw axes */

    if (x_min == x_max)
    {
	if (x_min < 0) x_max = 0.0;
	else
	if (x_max > 0) x_min = 0.0;
	else
	{
	    x_min = 0-1.0;
	    x_max = 1.0;
	}
    }
    if (y_min == y_max)
    {
	if (y_min < 0) y_max = 0.0;
	else
	if (y_max > 0) y_min = 0.0;
	else
	{
	    y_min = 0-1.0;
	    y_max = 1.0;
	}
    }

    if (Xflag)
	plotexscale(x_min, x_max, 1.0, &xlow, &xhigh, &xstep, &xdp);
    else
	plotscale(x_min, x_max, 1.0, &xlow, &xhigh, &xstep, &xdp);

    if (Yflag)
	plotexscale(y_min, y_max, 1.0, &ylow, &yhigh, &ystep, &ydp);
    else
	plotscale(y_min, y_max, 1.0, &ylow, &yhigh, &ystep, &ydp);

    if (debug_level >= 1)
	Fprintf(stderr,
	    "xlow = %g, xhigh = %g, ylow = %g, yhigh = %g\n",
	    xlow, xhigh, ylow, yhigh);


    draw_box(xlow, xhigh, xstep, xdp, ylow, yhigh, ystep, ydp,
		    upperx, uppery, lowerx, lowery);
    tk_snd_plot_cmd("c 2");

/* plot data */

    tk_snd_plot_cmd("set_class data");
    for (i_file = 0; i_file < nfiles; i_file++)
    {
	if (debug_level >= 2)
	    Fprintf(stderr, "Plotting data from file %d.\n", i_file + 1);

	plot_letter( xdata[i_file], ydata[i_file], (int) nans[i_file], 3,
		     xlow, xhigh, ylow, yhigh, symbol[i_file],
		     upperx, uppery, lowerx, lowery );
    }

/* print titles, etc. */

    /*
     * Print command line on top of page.
     * Break the line into two lines if it is more than COM_CHARS
     * characters long.
     */

    tk_snd_plot_cmd("c 5");
    sprintf(s,"l 325 %d 325 %d",upperx, lowerx);
    tk_snd_plot_cmd(s);
    sprintf(s,"m 300 %d", upperx);
    tk_snd_plot_cmd(s);
    tk_snd_plot_cmd("t 2 1");
    tk_snd_plot_cmd(command_line);

    /*
     * If the -t option was given, place the text above the graph,
     * otherwise print the default string there.  Likewise for the
     * -H and -V options.
     */

    if (tflag) {
	sprintf(s,"m 500 %d",upperx);
	tk_snd_plot_cmd(s);
	tk_snd_plot_cmd("t 5 1");
	tk_snd_plot_cmd(text[0]);
    }
    else
	if (nfiles > 1) {
	   sprintf(s,"m 500 %d",upperx);
	   tk_snd_plot_cmd(s);
	   tk_snd_plot_cmd("t 5 1");
	   sprintf(s,"Scatter Plot of %d ESPS Files",nfiles);
     	   tk_snd_plot_cmd(s);
        }
 	else {
	   sprintf(s,"m 500 %d",upperx);
	   tk_snd_plot_cmd(s);
	   tk_snd_plot_cmd("t 5 1");
	   sprintf(s,"File: %s",files[0]);
	   tk_snd_plot_cmd(s);
	}

    if (!Hflag && nelem == 2)
	if (elem_array[0] == -1)
	    Htext = savestring("tag");
	else {
	    Sprintf(txtbuf, "element%ld", elem_array[0] + 1);
	    Htext = savestring(txtbuf);
	}

    nchars = strlen(Htext);
    sprintf(s,"m %d %d",lowery + 350,(lowerx - upperx) / 2 + upperx - 
		nchars * CHAR_SIZE);
    tk_snd_plot_cmd(s);
    tk_snd_plot_cmd("t 5 1");
    tk_snd_plot_cmd(Htext);

    if (!Vflag && nelem == 2)
	if (elem_array[1] == -1)
	    Vtext = savestring("tag");
	else {
	    Sprintf(txtbuf, "element%ld\n", elem_array[1] + 1);
	    Vtext = savestring(txtbuf);
	}

    nchars = strlen(Vtext);
    sprintf(s,"m %d 90",(lowery - uppery) / 2 + uppery + nchars * CHAR_SIZE);
    tk_snd_plot_cmd(s);
    tk_snd_plot_cmd("t 5 1");
    tk_snd_plot_cmd(Vtext);

    print_time(500, 4600);

    tk_start();

    exit(0);
    /*NOTREACHED*/
}


/*
 * For debug printout of double arrays
 */

static void
pr_darray(text, n, arr)
    char    *text;
    long    n;
    double  *arr;
{
    int	    i;

    Fprintf(stderr, "%s -- %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%g ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr, "\n");
    }
}

/*
 * for debug printout of arrays of longs
 */

static void
pr_larray(text, n, arr)
    char    *text;
    int	    n;
    long    *arr;
{
    int	    i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%ld ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr, "\n");
    }
}
