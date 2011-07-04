/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney W. Johnson, Ajaipal S. Virdy
 * Checked by:
 * Revised by:
 *
 * Plot ESPS Sampled-Data files in multiline format.
 *
 * Plots ESPS Sampled-Data files at fixed x-axis scale with
 * continuation on additional lines as needed to accommodate the data.
 * Permits integral number of device resolution units per unit.
 */

static char *sccs_id = "@(#)mlplot.c	3.14	1/18/97	ESI/ERL";

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include "mlplot.h"
#include <esps/esignal_fea.h>

#define	PMODE	0640		/* default directory protection */

#define M_CONN 1		/* default: connected lines on plot */
#define M_POINT 2		/* plot individual data points */
#define M_VERT 3		/* plot vertical lines connected to x-axis */

#define NOAXIS 0		/* do not draw x-axis */
#define AXIS 1			/* draw x-axis */

#define CHR_GRID_TOP 7
#define CHR_GRID_BOT -3
#define CHR_GRID_RT 6

#define LBL_MAX 50

/*
 * The following are *default* settings for the Imagen Laser Printer
 * with 300 dots/inch. Note: some of the settings can be changed by
 * command line options.
 *
 * U and V correspond to X and Y axes, respectively.
 *
 * The drawing frame is the rectangular box in which the graph is
 * plotted.
 *
 */

#define DEF_PAGEHEIGHT 2400	/* pageheight set to 8 inches */
#define DEF_PAGEWIDTH 3000	/* pagewidth set to 10.0 inches */

#define DEF_DELTAV 400		/* height of drawing frame (rectangular box) */
#define DEF_DELTAU 2400		/* width of drawing frame */

#define DEF_U0 345		/* origin location for first frame */
#define DEF_V0 1675		/* origin location for first frame */

#define DEF_VSHIFT 500		/* difference between each frame origin */

#define DEF_XTICINT 100		/* mark tics on abscissa every 100th sample */
#define DEF_Y_SUBDIV 4		/* divide ordinate into 4 intervals */

#define DEF_XSCALE 4		/* 4 pixels/point */

#define DEF_MAXLINES 4		/* draw 4 frames on a page */
#define TAG_MAXLINES 3		/* draw 3 frames on a page */

#define TAG_EXTRAV 144		/* leave extra space between frames for TAG */

#define DEF_TTL_CHARSP 30	/* title character spacing */
#define DEF_COM_CHARSP 20	/* command line character spacing */
#define DEF_LBL_CHARSP 24	/* label character spacing */
#define DEF_TAG_CHARSP 24	/* tag character spacing */

#define DEF_X_TTLBASE 75	/* x title base */
#define DEF_Y_TTLBASE 35	/* y title base */

#define DEF_HDRBASE 2135	/* header base */

/*
 * The following defines apply to MASSCOMP Universe coordinate system.
 */

#define	MC_UNIVERSE	65535
#define	MC_REGION	MC_UNIVERSE/5
#define	MC_U_ORIGIN	-31767		/* horizontal coordinate */
#define	MC_V_ORIGIN	-21592		/* vertical coordinate */
#define	MAX_PAGES	25		/* only 25 regions can be displayed */

#define	MC_U0	-31767
#define	MC_V0	-31767

/* miscellanuous define's */

#define	MAX_FILES   10

#define	COM_CHARS   150		/* No. of characters of command line to */
				/* put on one printed line. */
#define MAX_COM_LEN 2*COM_CHARS	/* Fit command on two printed lines. */
#define COM_FMT	    "%% %.298s"	/* Truncate string to MAX_COM_LEN, */
				/* allowing two chars for initial "% ". */

/*
 * MACROS used in this program:
 */

#define ERROR(text) {Fprintf(stderr, "%s: %s\n", ProgName, text); exit(1);}

#define ERROR2(text1,text2) {Fprintf(stderr, "%s: %s %s\n", \
			ProgName, text1, text2); exit(1);}

#define SYNTAX USAGE( \
"mlplot [-l int][-m{cpv}][-n][-o outdir][-{pr} range][-s start]\n\
[-t title][-x debug level][-y range][-z][-L file.esps][-N]\n\
[-T device][-V title][-X scale][fil1.sd ... filen.sd]")

#define TRYALLOC(type,num,var) { \
    if (((var) = (type *) malloc((unsigned)(num)*sizeof(type)))==NULL) \
    { Fprintf(stderr, "%s:  can't allocate memory for %d points.\n", \
		ProgName, (int)(num)); \
	exit(1); }}


/* ESPS functions referenced */

int	getopt();
void	lrange_switch();
void	frange_switch();
short	get_rec_len();
int	get_gen_recd();
char	*get_cmd_line();
double	*allo_gen_recd();
char	*arr_alloc();
char    *e_temp_name();

/* G L O B A L   V A R I A B L E S */

char	*ProgName = "mlplot";

int	page_num;	/* current page (region) being printed */
char	*outdir = NULL;	/* write pages in this directory */
FILE	*outfp;		/* file pointer for storing pages in directory */
int	nflag = 0;	/* suppress output to stdout if this flag's set */
int	num_of_files;	/* number of sampled data files to plot */

char	*device = "gps"; /* default output format */

int	gps = YES;	/* gps format (default) */
int	tek = NO;	/* tektronix format (incl. imagen) */
int	imagen = NO;	/* imagen format */

int	debug_level = 0;

long	**u;		/* x-coordinate value to plot in a drawing frame */
long	**v;		/* y-coordinate value to plot in a drawing frame */

double 	**data;  	/* array to store records read from
			   ESPS sampled data files */

double	*ylow;		/* minimum amplitude */
double	*yhigh;		/* maximum amplitude */
double	*yticint;	/* interval for marking tics on y-axis */
double	*yscale;	/* y-axis scaling/division */

long	P0 = MC_U0;
long	Q0 = MC_V0;

char	*env_bundle;
int	gps_bundle = 20;

/* plotting functions */

int	gps_plotline();
int	tek_plotline();
void	tek_plotpoints();

/* pointer to determine which function to use for plotting */

int	(*plotting_func)();

/*
 * B E G I N
 *  M A I N
 *   P R O G R A M
 */

main(argc, argv)
    int	    argc;
    char    *argv[];
{
    extern int	optind;		/* used by getopt() */
    extern char	*optarg;	/* used by getopt() */

    int		ch;		/* used for parsing command line */

    FILE	*sdfile[MAX_FILES];
    char	*sdfilename[MAX_FILES];

    int		isfile = NO;    /* is input a pipe? */

    char	*sdname = NULL;

    FILE	*tagfile = NULL;
    char	*tagfilename = NULL;
    int		Lflag = 0;

    struct header *hd[MAX_FILES];

    struct header *th;
    double	*tag_rec;

    /*
     *  C O M M A N D
     *    L I N E
     *      O P T I O N
     *
     *  V A R I A B L E S
     *
     */

    char	*prange = NULL;	/* range of samples to plot */
    long	firstrec;	/* first record to plot */
    long	lastrec;	/* last record to plot */
    long	nrec;		/* number of records to plot */
    long	nread;		/* number of records read from file */


    char 	*yrange = NULL;	/* ampliude range */
    int		yflag = 0;
    int		ahi_flag = 0,
		alo_flag = 0;
    double	alow = -DBL_MAX;    /* lower amplitude limit */
    double	ahigh = DBL_MAX;    /* upper amplitude limit */

    long	start_index = 0;    /* offset starting index */
    long	start[MAX_FILES];   /* offset starting record */

    int		mode = M_CONN;	/* default mode for plotting points */
    int		axflag = AXIS;	/* default: plot x-axis (y=0) */

    int		Nflag = 0;

    /*
     *  I N D I C E S
     *      F O R
     *  A R R A Y S
     *    A N D
     *    L O O P S
     */

    long	i = 0;		/* temporary index */
    long	indx;		/* index to data */

    long	tag;

    int		i_file;		/* temporary file index */

    long	x;		/* temporary index */
    long	xmin;		/* starting sample number for a frame */
    long	xmax;		/* number of samples to plot in a frame */
    long	xlow;		/* sample number at beginning of each frame */
    long	xinc;		/* no. of samples to increment for each frame */

    int		tag_eof;	/* flag for end-of-tagged-file */ 

    int		nlines = 0;	/* frame number being plotted on a page */
    int		line_num = 0;	/* frame number being plotted on a page */

		/* number of frames to plot on a page */
    int		maxlines = DEF_MAXLINES;

    /*
     *  V A R I A B L E S
     *     U S E D 
     *      F O R
     *   P L O T T I N G
     *
     */

    long	u0 = DEF_U0;	/* x-coordinate of drawing frame */
    long	v0 = DEF_V0;	/* y-coordinate of drawing frame */

    long	xticint = DEF_XTICINT;
    long	xscale = DEF_XSCALE;

    long	ulow;		/* left border of drawing frame */
    long	vlow;		/* bottom border of drawing frame */
    long	old_vlow;	/* temporary variable */

    long	deltav = DEF_DELTAV;
    long	deltau = DEF_DELTAU;

		/* distance between frames on a page */
    long	vshift = DEF_VSHIFT;

    		/* character size for labeling text */
    long	lbl_charsp = DEF_LBL_CHARSP;
    long	tag_charsp = DEF_TAG_CHARSP;

		/* variables for header information */
    long	hdrleft = 0;
    long	hdrright = DEF_PAGEWIDTH;
    long	hdr_charsp = DEF_TTL_CHARSP;
    long	hdrbase = DEF_HDRBASE;

    long	x_ttl_center = DEF_PAGEWIDTH/2;
    long	x_ttl_base = DEF_X_TTLBASE;
    long	x_ttl_charsp = DEF_TTL_CHARSP;

    long	y_ttl_center = DEF_PAGEHEIGHT/2;
    long	y_ttl_base = DEF_Y_TTLBASE;
    long	y_ttl_charsp = DEF_TTL_CHARSP;

    char	*x_ttl_text =  "";  /* text string for labeling abscissa */
    char	*y_ttl_text =  "";  /* text string for labeling ordinate */

    /*
     * M I S C E L L A N E O U S
     *   V A R I A B L E S
     *
     */

    double	delta_y;
    int		continue_plotting = YES;
    char	command_line[MAX_COM_LEN+1];	/* "+1" for termnal null */
    char	*file_text;
    long	x_com_charsp = DEF_COM_CHARSP;
    double	sample_range;
    char	comtxt[COM_CHARS+1];	/* "+1" for termnal null */

		/* function to perform device initialization */
    void	initialize();

    /* Initialize `start' array to ones before -s options encountered. */

    for (i_file = 0; i_file < MAX_FILES; i_file++)
	start[i_file] = 1;

    Sprintf(command_line, COM_FMT, get_cmd_line(argc, argv));

    /* Parse command line.  Get options and determine input file. */

    while ((ch = getopt(argc, argv, "l:m:no:p:r:s:t:x:y:zL:NT:V:X:")) != EOF)
	switch (ch)
	{
	case 'l':
	    xticint = atol(optarg);
	    break;
	case 'm':
	    switch (*optarg)
	    {
	    case 'p':
		mode = M_POINT;
		break;
	    case 'c':
		mode = M_CONN;
		break;
	    case 'v':
		mode = M_VERT;
		break;
	    default:
		SYNTAX;
		break;
	    }
	    break;
	case 'n':
	    nflag++;
	    break;
	case 'o':
	    outdir = optarg;
	    break;
	case 'p':
	case 'r':
	    prange = optarg;
	    break;
	case 's':
	    start[start_index++] = atol(optarg);
	    break;
	case 't':
	    x_ttl_text = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'y':
	    yrange = optarg;
	    yflag++;
	    break;
	case 'z':
	    axflag = NOAXIS;
	    break;
	case 'L':
	    tagfilename = optarg;
	    Lflag++;
	    break;
	case 'N':
	    Nflag++;
	    break;
	case 'T':
	    device = optarg;
	    break;
	case 'V':
	    y_ttl_text = optarg;
	    break;
	case 'X':
	    xscale = atol(optarg);
	    break;
	default:
	    SYNTAX;
	    break;
	}

    num_of_files = argc - optind;
    if(debug_level) fprintf(stderr,"num_of_files: %d\n",num_of_files);

    /* Read the headers. */

    for (i_file = 0; i_file < num_of_files; i_file++, optind++)
    {
	if (strcmp(argv[optind], "-") == 0)
	{
	    if (i_file == 0)
	    {
		sdfilename[0] = "<stdin>";
		eopen(ProgName, "-", "r", FT_FEA, FEA_SD,
			&(hd[0]), &(sdfile[0]));
	    }
	    else
		ERROR("Only first sample data file may be standard input.")
	}
	else
	{
	    sdfilename[i_file] = argv[optind];

	    eopen(ProgName, sdfilename[i_file], "r", FT_FEA, FEA_SD, 
		  &(hd[i_file]), &(sdfile[i_file]));
	}

	if (get_fea_siz("samples", hd[i_file], (short) NULL, 
			(long) NULL) != 1) {
	    Fprintf(stderr, 
		    "%s: sorry, can't work on multi-channel files (%s)\n",
		    ProgName, sdfilename[i_file]);
	    exit(1);
	}

	if (debug_level > 1)
	{
	    Fprintf(stderr, "%s: sdfilename[%d] is %s\n",
	    ProgName, i_file, sdfilename[i_file]);
	    (void) fflush(stderr);
	}

    }

    if (tagfilename != NULL)
    {
	if (strcmp(tagfilename, "-") != 0)
	    {TRYOPEN(ProgName, tagfilename, "r", tagfile)}
	else if (sdfile[0] != stdin)
	{
	    tagfilename = "<stdin>";
	    tagfile = stdin;
	}
	else
	{
	    ERROR("Tagged file and sampled-data file cannot both be standard input.")
	}

	if (debug_level > 0)
	    Fprintf(stderr,
	    "%s: Tagged file specified: %s\n", ProgName, tagfilename);

	maxlines = TAG_MAXLINES;
	v0 -= TAG_EXTRAV + LROUND(10 * hdr_charsp / 6.0);
	vshift += TAG_EXTRAV;

	if ((th = read_header(tagfile)) == NULL)
	{
	    NOTSPS(ProgName, tagfilename)
	}

	if (get_esignal_hdr(th))
	{
	    ERROR("Esignal format not supported for tagged file.")
	}
		
	if (!th->common.tag)
	{
	    ERROR2(tagfilename, "is not tagged.")
	}

	if (num_of_files == 0)
	{
	    if (strcmp(th->variable.source[0], "<stdin>") == 0)
	    {
		ERROR2(tagfilename, "source was <stdin>.")
	    }

	    sdfilename[0] = th->variable.source[0];

	    if (debug_level > 0)
		Fprintf(stderr,
		"%s: Sampled Data file specified: %s\n",
		ProgName, sdfilename[0]);

	    num_of_files = 1;

	    eopen(ProgName, sdfilename[0], "r", FT_FEA, FEA_SD, 
		  &(hd[0]), &(sdfile[0]));

	    if (get_fea_siz("samples", hd[0], (short)NULL, (long)NULL) != 1) {
	      Fprintf(stderr, 
		      "%s: sorry, can't work on multi-channel files (%s)\n",
		      ProgName, sdfilename[0]);
	      exit(1);
	    }
	}
    }

    if (num_of_files == 0)
	SYNTAX;

    if (debug_level > 0)
	Fprintf(stderr,
	"%s: number of ESPS Sampled Data files to process: %d\n",
	ProgName, num_of_files);

    if (num_of_files == 1)
	sdname = sdfilename[0];

    initialize();

    if (gps)
    {
	env_bundle = getenv("BUNDLE");
	if (env_bundle != NULL) gps_bundle = atoi(env_bundle);
    }

    /* get the range to be plotted */

    assert(hd[0]);

    if (hd[0]->common.ndrec != -1) isfile++;

    if (debug_level > 0)
	Fprintf (stderr,
	"%s: first file will be read from a %s.\n",
	ProgName, (isfile ? "file" : "pipe"));

    firstrec = start[0];
    if (isfile)
	lastrec = hd[0]->common.ndrec + start[0] - 1;
    else
	lastrec = LONG_MAX;

    lrange_switch(prange, &firstrec, &lastrec, 0);

    if (lastrec < firstrec) {
	Fprintf(stderr, "%s: bad range given, last record < first record.\n",
		ProgName);
	exit(1);
    }

    for (i_file = 0; i_file < num_of_files; i_file++)
	if (firstrec < start[i_file]) 
	    firstrec = start[i_file];

    if (isfile)
	if (lastrec > hd[0]->common.ndrec + start[0] - 1) 
	    lastrec = hd[0]->common.ndrec + start[0] - 1;

    if (debug_level > 1)
	Fprintf(stderr,
	    "%s: firstrec = %ld, lastrec = %ld.\n",
	    ProgName, firstrec, lastrec);

    (void) fflush(stderr);

    if (yflag)
    {
	frange_switch(yrange, &alow, &ahigh);

	if (alow == ahigh) alow = -ahigh;

	if (alow != -DBL_MAX) alo_flag++;
	if (ahigh != DBL_MAX) ahi_flag++;

	if (debug_level > 1)
	{
	    Fprintf(stderr, "%s: alow = %e, ahigh = %e.\n",
		ProgName, alow, ahigh);
	    (void) fflush(stderr);
	}
    }

    for (i_file = 0; i_file < num_of_files; i_file++)
    {
	if (debug_level > 1)
	{
	    Fprintf(stderr, "%s: skipping %ld records in %s\n",
	    ProgName, firstrec - start[i_file], sdfilename[i_file]);
	    (void) fflush(stderr);
	}
	fea_skiprec(sdfile[i_file], firstrec - start[i_file], hd[i_file]);
    }

    if (debug_level > 1)
    {
	Fprintf(stderr,
	"%s: allocating memory for data, u, v, ylow, yhigh, yscale, yticint.\n",
	ProgName);
	(void) fflush(stderr);
    }

    /*
     * Allocate memory for data, u, and v.
     *
     * We need to allocate:
     *
     *	(number of files to plot) [num_of_files]
     *			by
     *	(number of data records) [nrec]
     *
     * memory space if the first file on the command
     * line is not a pipe.  If we have to read from a
     * pipe, we read the data into a temporary file to
     * determine nrec for that file.  Then we can proceed
     * as normal.
     *
     * Allocate memory for ylow, yhigh, yscale, yticint also.
     * These arrays should have num_of_files elements.
     *
     */

    if (!isfile)
    {
	char	*tmp_file;		/* temporary file name */
	FILE	*tmp_strm;		/* temporary stream */
	char	*template = "mlplotXXXXXX";
	int	recsize;
	char	*tmp_buf;
	long	tmp_rec = 0;

	/*
	 * We have to determine how many records exist
	 * in stdin.  We'll do this by writing stdin
	 * into a temporary file.
	 */

	if (debug_level == 6)
	    Fprintf (stderr,
		"%s: reading stdin into a temporary file...\n", ProgName);


	tmp_file = e_temp_name(template);
	TRYOPEN(ProgName, tmp_file, "w+", tmp_strm)
	(void) unlink(tmp_file);

	recsize = size_rec(hd[0]);

	if (recsize == -1)
	{
	    ERROR("variable record size not supported.")
	}

	if (debug_level == 6)
	    Fprintf(stderr,
		"%s: opened temp file %s, recsize = %d; allocate memory..\n",
		ProgName, tmp_file, recsize);

	TRYALLOC(char, recsize, tmp_buf)

	/*
	 * The pointer in the input stream (stdin) is already
	 * set to the proper postition (because of read_header(3-ESPS)
	 * and skiprec(3-ESPS).  Now use the standard UNIX routines
	 * fread(3) and fwrite(3) to get data from stdin to the
	 * temporary file.
	 */

	while (fread (tmp_buf, recsize, 1, stdin) == 1) {
	    if (fwrite (tmp_buf, recsize, 1, tmp_strm) != 1) {
		Fprintf (stderr, "%s: write error on %s\n",
		ProgName, tmp_file);
		exit (1);
	    }
	    tmp_rec++;
	}

	if (debug_level == 6) {
	    Fprintf (stderr,
	    "%s: %ld records from stdin written to temporary file (%s).\n",
	    ProgName, tmp_rec, tmp_file);
	}

	if (lastrec > tmp_rec + firstrec - 1)
	    lastrec = tmp_rec + firstrec - 1;

	(void) rewind(tmp_strm);
	Fclose(stdin);
	sdfile[0] = tmp_strm;

    }	/* end if (!isfile) */

    nrec = lastrec - firstrec + 1;

    (void) allo_d_matrix(num_of_files, (int) nrec);
    (void) allo_d_array(num_of_files);

    /* read data from the ESPS sampled-data files and store in data array */

    if (debug_level > 3)
    {
	Fprintf(stderr,
	"%s: reading %ld data records from ESPS Sampled Data file...\n",
	ProgName, nrec);
	(void) fflush(stderr);
    }

    for ( i_file = 0; i_file < num_of_files; i_file++ )
    {
	if ( (nread = get_sd_recd(data[i_file],
				(int) nrec, hd[i_file], sdfile[i_file]) )
	    < nrec)
	{
	    Fprintf(stderr,
		"\n\n%s: appending zeros; not enough data in %s.\n\n",
		ProgName, sdfilename[i_file]);

	    for (i = nread; i < nrec; i++) data[i_file][i] = 0.0;
	}

	if (alo_flag && ahi_flag)
	{
	    yhigh[i_file] = ahigh;
	    ylow[i_file] = alow;
	}
	else
	{
	    /*
	     * Compute the maximum and minumum elements in the
	     * Sampled Data file and then determine how many bits
	     * we need to get the required resolution using an
	     * Analog-to-Digital converter.
	     *
	     */

	    for (i = 0; i < nrec; i++)
	    {
		if (i == 0)
		{
		    yhigh[i_file] = data[i_file][i];
		    ylow[i_file] = data[i_file][i];
		}
		else
		{
		    if (data[i_file][i] > yhigh[i_file])
			yhigh[i_file] = data[i_file][i];

		    if (data[i_file][i] < ylow[i_file])
			ylow[i_file] = data[i_file][i];
		}
	    }

	    if (debug_level > 3)
	    {
		Fprintf(stderr,
		"%s: actual: ylow[%d] = %g, yhigh[%d] = %g\n",
		ProgName, i_file, ylow[i_file], i_file, yhigh[i_file]);
		(void) fflush(stderr);
	    }

	    if (yhigh[i_file] <= 0.0)
		yhigh[i_file] = 0.0;
	    else
	    {
		for (sample_range = 1.0;
		    sample_range < yhigh[i_file]; sample_range *= 2.0) {}
		yhigh[i_file] = sample_range;
	    }

	    if (ylow[i_file] >= 0.0)
		ylow[i_file] = 0.0;
	    else
	    {
		for (sample_range = -1.0;
		    sample_range > ylow[i_file]; sample_range *= 2.0) {}
		ylow[i_file] = sample_range;
	    }

	    if (alo_flag)
		ylow[i_file] = alow;
	    else if (ahi_flag)
		yhigh[i_file] = ahigh;
	    else if (yhigh[i_file] > 0 && ylow[i_file] < 0)
	    {
		if (yhigh[i_file] > -ylow[i_file])
		    ylow[i_file] = -yhigh[i_file];
		else
		    yhigh[i_file] = -ylow[i_file];
	    }

	    if (debug_level > 3)
	    {
		Fprintf(stderr, 
		"%s: using: ylow[%d] = %g, yhigh[%d] = %g\n",
		ProgName, i_file, ylow[i_file], i_file, yhigh[i_file]);
		(void) fflush(stderr);
	    }

	}  /* end if (alo_flag && ahi_flag) ... else */

	if (ylow[i_file] >= yhigh[i_file])
	{
	    ERROR("zero amplitude range.")
	}
    }	/* end outer for (i_file = 0; i_file < num_of_files; i_file++) loop */

    if (debug_level > 0)
	Fprintf(stderr, "%s: data has been read, max & min computed.\n",
	ProgName);

    xinc = deltau / xscale;

   /*
    * Now compute yscale from yhigh and ylow arrays.
    */

    for (i_file = 0; i_file < num_of_files; i_file++)
    {
	delta_y = yhigh[i_file] - ylow[i_file];
	yscale[i_file] = deltav / delta_y;
	yticint[i_file] = delta_y / DEF_Y_SUBDIV;

	if (debug_level > 3)
	{
	    Fprintf(stderr,
		"%s: yscale[%d] = %g, yticint[%d] = %g\n",
		ProgName, i_file, yscale[i_file], i_file, yticint[i_file]);
	    (void) fflush(stderr);
	}
    }

    if (tagfilename != NULL)
    {
	tag_rec = allo_gen_recd(th);
	do
	    tag_eof = get_gen_recd(tag_rec, &tag, th, tagfile) == EOF;
	while (!tag_eof && tag < firstrec);

	if ((debug_level > 0) && Lflag)
	    Fprintf(stderr, "%s: %sreached end of tagged file.\n",
	    ProgName, (tag_eof == 0) ? "have not " : "you have ");
    }

    page_num = 0;

    ulow = u0;
    vlow = v0;

    indx = 0;

    if (debug_level > 0)
	Fprintf(stderr, "%s: beginning main loop:\n", ProgName);

/*
 * M  A  I  N
 *
 *    L  O  O  P:
 *
 * Now begin the main loop which intializes the graph (i.e printheader,
 * drawaxes and labels), reads data and plots points.
 *
 */


    if (Nflag)
    {
	xlow = firstrec - 1;
	xmin = firstrec;
    }
    else
    {
	xlow = firstrec - 1 - (firstrec - 1) % xinc;
	xmin = firstrec;
    }

    if (gps && !nflag) gps_null_comment(stdout);

    while (continue_plotting)
    {
	xmax = MIN(xlow + xinc, lastrec);

	if (debug_level > 4)
	    Fprintf(stderr,
		"\n\n MAIN LOOP\n%s: xmax = %d\n", ProgName, xmax);

	/*
	 * R E A D
	 *   S T O R E D
	 *         D A T A:
	 *
	 * Read in data from data and scale each plot accordingly.
	 *
	 */

	/*
	 * save the value of vlow (the vertical border)
	 * you need it to go back to the starting place to begin plotting.
	 *
 	 */

	old_vlow = vlow;

	for ( x = xmin, i = 0; x <= xmax; ++x, ++indx, ++i )
	{
	    /*                                    ^
	     *					  |
	     *
	     * Notice:  index is initialized outside MAIN LOOP,
	     *		it is used to index data 
	     *		matrix across records.
	     */

	    vlow = old_vlow;	/* get back to the
				 * original vertical starting position
				 */

	    for ( i_file = 0; i_file < num_of_files; ++i_file )
	    {
		u[i_file][i] = ulow + (x - xlow) * xscale;

		v[i_file][i] = vlow
		    + LROUND((data[i_file][indx] - ylow[i_file])
				* yscale[i_file]);

		/*
		 * Keep track of which line we are plotting
		 * and shift vlow accordingly.
		 */

		if (i_file == 0)
		    line_num = nlines;

		if (++line_num < maxlines)
		    vlow -= vshift;
		else
		{
		    vlow = v0;
		    line_num = 0;
		}
	    }
	}

	vlow = old_vlow;	/* get the old vertical postion */

	/*
	 * B E G I N
 	 *
 	 *    P L O T T I N G:
 	 *
 	 */

	for ( i_file = 0; i_file < num_of_files; ++i_file )
	{
	    if (nlines == 0)	/* it's the beginning of a new page */
	    {
		++page_num;

		if ((page_num > MAX_PAGES) && gps)
		{
		    if ( !nflag )
		    {
			Fprintf(stderr,
			    "%s: Warning, only 25 regions can be displayed on a Masscomp Graphics Terminal!\n",
			    ProgName);
			Fprintf(stderr,
			    "        stopping output to stdout.\n");
			nflag = 1;
		    }
		}

		if (debug_level > 0)
		    Fprintf(stderr, "\n");

		if ( gps && (outdir != NULL) )
		{
		    if (debug_level > 3)
		    {
			Fprintf(stderr,
			    "%s: initializing page (region) number %d\n",
			    ProgName, page_num);
			(void) fflush(stderr);
		    }
		    init_gps_page();
		}

		if (tek && !imagen)
		{
		    tek_termpage();
		}

		if (debug_level > 3)
		{
		    Fprintf(stderr, "%s: printing command line...\n",
			ProgName);
		    (void) fflush(stderr);
		}

		/*
		 * Print out the command line on top of output page.
		 *
		 */

		if (strlen(command_line) > COM_CHARS)
		{
		    (void) strncpy(comtxt, command_line, COM_CHARS);
		    comtxt[COM_CHARS] = '\0';
		    text( hdrleft, hdrbase + 90,
			    x_com_charsp, 0,
			    comtxt, plotting_func);
		    text( hdrleft, hdrbase + 60,
			    x_com_charsp, 0,
			    &command_line[COM_CHARS], plotting_func);
		}
		else
		    text( hdrleft, hdrbase + 90,
			    x_com_charsp, 0,
			    command_line, plotting_func);

		DRAW(hdrleft, hdrbase + 47, hdrright, hdrbase + 47, plotting_func)

		if (debug_level > 3)
		{
		    Fprintf(stderr, "%s: printing header...\n", ProgName);
		    (void) fflush(stderr);
		}

		/*
		 * Now, print out the header.
		 *
		 */

		printheader(hdrleft, hdrright, hdrbase, hdr_charsp,
			    sdname, tagfilename, xscale, plotting_func);

		text(   x_ttl_center
			- (x_ttl_charsp * strlen(x_ttl_text))/2,
			x_ttl_base,
			x_ttl_charsp, 0, x_ttl_text, plotting_func);

		text(   y_ttl_base,
			y_ttl_center
			- (y_ttl_charsp * strlen(y_ttl_text))/2,
			y_ttl_charsp, 90, y_ttl_text, plotting_func);


	    }   /* end if (nlines == 0) */

	    /*
	     * We should draw axes here:
	     */

	    if (debug_level > 3)
	    {
		Fprintf(stderr, "%s: drawing axes...\n", ProgName);
		(void) fflush(stderr);
	    }

	    delta_y = yhigh[i_file] - ylow[i_file];

	    drawaxes(ulow, vlow, xlow, ylow[i_file], xinc + 1, delta_y,
 		    xscale, yscale[i_file], xticint, yticint[i_file], 
 		    lbl_charsp, axflag, plotting_func);

	    if (tagfilename != NULL)
		while (!tag_eof && (x = tag) <= xmax)
		{
		    long    u = ulow + (x - xlow) * xscale;
		    char    label[LBL_MAX];

		    if (debug_level > 3)
		    {
			Fprintf(stderr,
			"%s: tagged file, labeling tags...\n",
			ProgName);
			(void) fflush(stderr);
		    }

		    DRAW(u, vlow + deltav, u, vlow + deltav + lbl_charsp,
			plotting_func)

		    Sprintf(label, "%ld", x);
		    text(   u, vlow + deltav + lbl_charsp,
			tag_charsp, 90, label, plotting_func);

		    tag_eof = get_gen_recd(tag_rec, &tag, th, tagfile) == EOF;
		}

	    /*
	     * label the current file we're plotting
	     */

	    /*
	     * use strrchr(sdfilename[], '/') to get the name of the
	     * ESPS Sampled Data file without the full path name.
	     */

	    if ( (file_text = strrchr(sdfilename[i_file], '/')) == NULL )
		file_text = sdfilename[i_file];
	    else
		file_text = ++file_text;

	    text(   y_ttl_base + deltau + 400,
		    vlow + deltav/2
		    + (y_ttl_charsp * strlen(file_text))/2,
		    y_ttl_charsp, -90, file_text, plotting_func);


	    if (debug_level > 3)
	    {
		Fprintf(stderr, "%s: plotting data...\n", ProgName);
		(void) fflush(stderr);
	    }

	    switch (mode)
	    {

	    case M_POINT:
		if (tek)
		    tek_plotpoints(xmax - xmin + 1, u[i_file], v[i_file]);
		else /* gps */
		    for (x = xmin, i = 0; x <= xmax; ++x, ++i)
			DRAW(u[i_file][i], v[i_file][i],
			     u[i_file][i], v[i_file][i], plotting_func)
		break;

	    case M_CONN:
		if (tek)
		    tek_plotline(xmax - xmin + 1, u[i_file], v[i_file]);
		else /* gps */
		    gps_plotline(xmax - xmin + 1, u[i_file], v[i_file]);
		break;

	    case M_VERT:
		{
		    long    base = 
			    (ylow[i_file] >= 0) ? vlow :
			    (yhigh[i_file] <= 0) ? vlow + deltav :
				vlow - LROUND(ylow[i_file] * yscale[i_file]);

		    for (x = xmin, i = 0; x <= xmax; ++x, ++i)
			DRAW(u[i_file][i], v[i_file][i],
			    u[i_file][i], base,
			    plotting_func)
		}
		break;

	    } /* end switch (mode) */


	    if (++nlines < maxlines)
	    {
		if (debug_level > 3)
		    Fprintf(stderr,
			"%s: shifting frame down by vshift (%ld) to ",
			ProgName, vshift);

		vlow -= vshift;	/* move next drawing frame down */

		if (debug_level > 3)
		{
		    Fprintf(stderr, "vlow (%ld)\n", vlow);
		    (void) fflush(stderr);
		}

	    }
	    else
	    {		/* you're on the next page */

		/* (nlines >= maxlines) */

		if ( gps )
		{
		    if ( (page_num % 5) == 0 )
		    {
			/* move to left region after every five pages */
			Q0 += MC_REGION;
			P0 = MC_U0;

			if (debug_level > 3)
			{
			    Fprintf(stderr,
				"%s: moving drawing frame origin to (P0 = %ld, Q0 = %ld)\n",
				ProgName, P0, Q0);
			    (void) fflush(stderr);
			}
		    }
		    else
		    {
			/* otherwise increment along x-axis */
			P0 += MC_REGION;

			if (debug_level > 3)
			{
			    Fprintf(stderr,
				"%s: moving drawing frame origin to (u0+P0 = %ld, v0+Q0 = %ld)\n",
				ProgName, u0 + P0, v0 + Q0);
			    (void) fflush(stderr);
			}
		    }

		} /* end if (gps) */

		vlow = v0;		/* reset y-coordinates */
		nlines = 0;		/* reset frame number */

		if (debug_level > 3)
		{
		    Fprintf(stderr,
			"%s: moving origin to (ulow = %ld, vlow = %ld), starting new page\n\n",
			ProgName, ulow, vlow);
		    (void) fflush(stderr);
		}

		if (imagen)
		    tek_termpage();

		if ( gps && (outdir != NULL) )
		    (void) fclose(outfp);

	    }   /* end if (nlines < maxlines) */

	}   /* end plotting loop */

	xlow += xinc;
	xmin = xlow + 1;
	if (xmin > lastrec)
	    continue_plotting = NO;

    }	/* Main while loop */

    if (nlines > 0)
	if (imagen)
	    tek_termpage();

    exit(0);
    /*NOTREACHED*/
} /* end main */


double *
allo_gen_recd(hd)
    struct header *hd;
{
    double  *rec;
    TRYALLOC(double,get_rec_len(hd),rec)
    return rec;
}

allo_d_matrix(rows, columns)
    int	    rows;
    int	    columns;
{
    char    *arr_alloc();
    long    dim[2];

    dim[0] = rows;
    dim[1] = columns;

    data = (double **) arr_alloc(2, dim, DOUBLE, 0);
    u = (long **) arr_alloc(2, dim, LONG, 0);
    v = (long **) arr_alloc(2, dim, LONG, 0);
}

allo_d_array(rows)
    int	    rows;
{
    TRYALLOC(double, rows, yhigh)
    TRYALLOC(double, rows, ylow)
    TRYALLOC(double, rows, yscale)
    TRYALLOC(double, rows, yticint)
}
