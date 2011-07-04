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
 * Written by:  Ajaipal S. Virdy
 * Checked by:
 * Revised by:
 *
 * Plot "generic" ESPS files in multiline format.
 *
 */

static char *sccs_id = "@(#)genplot.c	3.9	1/18/97	ESI/ERL";

#include <stdio.h>
#include <ctype.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/sd.h>
#include <esps/fea.h>
#include <esps/esignal_fea.h>
#include "genplot.h"

#define	PMODE	0640		/* default directory protection */

#define M_CONN 1		/* default: connected lines on plot */
#define M_POINT 2		/* plot individual data points */
#define M_VERT 3		/* plot vertical lines connected to x-axis */

#define NOAXIS 0		/* do not draw x-axis */
#define AXIS 1			/* draw x-axis */

#define CHR_GRID_TOP 7
#define CHR_GRID_BOT -3
#define CHR_GRID_RT 6

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

#define DEF_PAGEHEIGHT 2400	/* pageheight set to 8.0 inches */
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

#define DEF_TTL_CHARSP 30	/* title character spacing */
#define DEF_COM_CHARSP 20	/* command line character spacing */
#define DEF_LBL_CHARSP 24	/* label character spacing */

#define DEF_X_TTLBASE 75	/* x title base */
#define DEF_Y_TTLBASE 35	/* y title base */

#define DEF_HDRBASE 2135	/* header base */

/*
 * Type of functions to use (see -f option)
 */

#define	NONE 0
#define	SQRT 1
#define LOG  2

/*
 * The following defines apply to MASSCOMP Universe coordinate system.
 */

#define	MC_UNIVERSE	65535
#define	MC_REGION	MC_UNIVERSE/5
#define	MC_U_ORIGIN	-31767		/* horizontal coordinate */
#define	MC_V_ORIGIN	-21592		/* vertical coordinate */
#define	MAX_PAGES	25		/* only 25 regions can be displayed */

#define	MC_U0	-31767;
#define	MC_V0	-31767;

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
"genplot [-e range]... [-f function][-i range]... [-l int][-m{cpv}]\n\
[-n][-o outdir][-r range][-s start][-t title][-x debug level][-y range]\n\
[-z][-E][-T device][-V title][-X scale] file")

#define TRYALLOC(type,num,var) { \
    if (((var) = (type *) malloc((unsigned)(num)*sizeof(type)))==NULL) \
    { Fprintf(stderr, "%s:  can't allocate memory for %d points.\n", \
		ProgName, (int)(num)); \
	exit(1); }}


/* math routines accessed */

double	sqrt();
double	log();

/* ESPS functions referenced */

int	getopt();
void	lrange_switch();
void	frange_switch();
long	*grange_switch();
long	*fea_range_switch();
short	get_rec_len();
int	get_gen_recd();
char	*get_cmd_line();
char    *e_temp_name();

/* G L O B A L   V A R I A B L E S */

char	*ProgName = "genplot";

int	page_num;	/* current page (region) being printed */
char	*outdir = NULL;	/* write pages in this directory */
FILE	*outfp;		/* file pointer for storing pages in directory */
int	nflag = 0;	/* suppress output to stdout if this flag's set */

char	*device = "gps"; /* default output format */

int	gps = YES;	/* gps format (default) */
int	tek = NO;	/* tektronix format (incl. imagen) */
int	imagen = NO;	/* imagen format */

int	debug_level = 0;

long	**u;		/* x-coordinate value to plot in a drawing frame */
long	**v;		/* y-coordinate value to plot in a drawing frame */

double 	*data;  	/* array to store records read from an ESPS file */

double	*ylow;		/* minimum ordinate */
double	*yhigh;		/* maximum ordinate */
double	*yticint;	/* interval for marking tics on y-axis */
double	*yscale;	/* y-axis scaling/division */

long	P0 = MC_U0;
long	Q0 = MC_V0;

double  **Elem_Matrix;	/* matrix to store records for each element */
long	*Tag_Array;	/* array to store Tags for each element */

char	*func_codes[] = { "NONE", "SQRT", "LOG", NULL };

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

main (argc, argv)
    int	    argc;
    char    *argv[];
{
    extern int	optind;		/* used by getopt() */
    extern char	*optarg;	/* used by getopt() */

    int		ch;		/* used for parsing command line */

    FILE	*infile = stdin;
    char	*infilename = "<stdin>";

    struct header *hd;		/* pointer to ESPS header for input file */

    /*
     *  C O M M A N D
     *    L I N E
     *      O P T I O N
     *
     *  V A R I A B L E S
     *
     */

    char	*rrange = NULL;	/* range of records to plot */
    long	firstrec;	/* first record to plot */
    long	lastrec;	/* last record to plot */
    long	nrec;		/* number of records to plot */
    long	ndrec;		/* number of records in file */

    char 	*yrange = NULL;	/* ordinate range */
    int		yflag = 0;
    int		ahi_flag = 0,
		alo_flag = 0;
    double	alow = -DBL_MAX;    /* lower ordinate limit */
    double	ahigh = DBL_MAX;    /* upper ordinate limit */

    char	*erange = "1";	/* element range */
    int		eflag = 0;	/* default */
    long	first_ele;	/* first element to plot */
    long	last_ele;	/* last element to plot */

    long	*elem_array;
    long	nelem = 1;

    char	**fea_fields;	/* field specification given with -e or -i */
    char	**fea_names;
    long	*fea_indices;
    int		nfld = 0;

    char	*irange = NULL;	/* item range */
    int		iflag = 0;
    long	first_itm;
    long	last_itm;

    long	*item_array;
    long	nitem = 0;

    int		function_type = 0;	/* type of function to use */

    long	start = 1;	/* offset starting record */
    int		mode = M_CONN;	/* default mode for plotting points */
    int		axflag = AXIS;	/* default: plot x-axis (y=0) */
    int		Eflag = 0;	/* plot element vs. tags */

    /*
     *  I N D I C E S
     *      F O R
     *  A R R A Y S
     *    A N D
     *    L O O P S
     */

    int		i, j;		/* temporary indices */
    int		index;		/* index to Elem_Matrix and Tag_Array */
    long	i_ele;		/* temporary element index */

    long	tag;		/* variable to store tags */

    long	x;		/* temporary index */
    long	xmin;		/* starting sample number for a frame */
    long	xmax;		/* number of samples to plot in a frame */
    long	xlow;		/* sample number at beginning of each frame */
    long	xinc;		/* no. of samples to increment each frame */

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
     * V A R I A B L E S
     * N E E D E D   T O
     * I M P L E M E N T
     *
     * T H E  -E   O P T I O N
     *
     */

    long	old_xmin;
    double	delta_y;
    int		recd_num = 0;

    /*
     * M I S C E L L A N E O U S
     *   V A R I A B L E S
     *
     */

    char	element_text[15];
    int		continue_plotting = YES;

    int		rec_num;	/* current record number */

    int		n_graphs = 1;	/* no. of graphs to plot */
    int		n_points;	/* no. of points to plot in a graph */

    char	command_line[MAX_COM_LEN+1];	/* "+1" for terminal null */
    long	x_com_charsp = DEF_COM_CHARSP;
    char	comtxt[COM_CHARS+1];	/* "+1" for terminal null */

		/* function to perform device initialization */
    void	initialize();

    if ((fea_fields = (char **) calloc((unsigned) 1, sizeof(char *))) == NULL)
    {
	Fprintf(stderr,
	    "%s: can't allocate memory for %d points.\n",
	    ProgName, (int)(nfld + 1));
	exit(1);
    }


    Sprintf(command_line, COM_FMT, get_cmd_line(argc, argv));

    /* Parse command line.  Get options and determine input file. */

    while ((ch = getopt(argc, argv, "e:f:i:l:m:no:r:s:t:x:y:zET:V:X:")) != EOF)
	switch (ch)
	{
	case 'e':
	    eflag = 1;
	    if (isalpha(optarg[0]))
	    {
		fea_fields[nfld++] = optarg;
		if ((fea_fields =
			(char **) realloc( (char *) fea_fields,
			    (unsigned) (nfld + 1) * sizeof(char *) )
		    ) == NULL)
		{
		    Fprintf(stderr,
			"%s: can't reallocate memory for %d points.\n",
			ProgName, (int)(nfld + 1));
		    exit(1);
		}
		fea_fields[nfld] = NULL;
	    }
	    else erange = optarg;
	    break;
	case 'f':
	    if ((function_type = lin_search(func_codes, optarg)) == -1)
	    {
		Fprintf(stderr, "%s: unknown function type %s.\n",
		    ProgName, optarg);
		exit(1);
	    }
	    if (debug_level > 0)
		Fprintf(stderr, "%s: function_type = %d, func_code = %s\n",
		    ProgName, function_type, func_codes[function_type]);
	    break;
	case 'i':
	    iflag = 1;
	    if (isalpha(optarg[0]))
	    {
		fea_fields[nfld++] = optarg;
		if ((fea_fields =
			(char **) realloc( (char *) fea_fields,
			    (unsigned) (nfld + 1) * sizeof(char *) )
		    ) == NULL)
		{
		    Fprintf(stderr,
			"%s: can't reallocate memory for %d points.\n",
			ProgName, (int)(nfld + 1));
		    exit(1);
		}
		fea_fields[nfld] = NULL;
	    }
	    else irange = optarg;
	    break;
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
	case 'r':
	    rrange = optarg;
	    break;
	case 's':
	    start = atol(optarg);
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
	case 'E':
	    Eflag++;
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

    if (optind != argc - 1)
	SYNTAX;

    if (strcmp(argv[optind], "-") != 0)
    {
	infilename = argv[optind];
	TRYOPEN(ProgName, infilename, "r", infile)
    }
    else
    {
	infilename = "<stdin>";
	infile = stdin;
    }

    /* Read the header. */

    if ((hd = read_header(infile)) == NULL)
	NOTSPS(ProgName, infilename)

    if (get_esignal_hdr(hd))
    {
	ERROR("sorry, Esignal format not supported.");
    }

    if (!iflag && !eflag)
	eflag = 1;

    if (iflag && eflag)
    {
	Fprintf(stderr,
	    "%s: conflicting options, -i and -e cannot be used together.\n",
	    ProgName);
	exit(1);
    }

    if (nfld > 0 && hd->common.type != FT_FEA)
	ERROR("named fields allowed only with FEA files.")

    if ((hd->common.tag == NO) && Eflag)
    {
	Fprintf(stderr,
	    "%s: data is not tagged; cannot use -E option.\n",
	    ProgName);
	exit(1);
    }

    /* perform intialization */

    (void) initialize();

    if (gps)
    {
	env_bundle = getenv("BUNDLE");
	if (env_bundle != NULL) gps_bundle = atoi(env_bundle);
    }

    /* get the range to be plotted */

    ndrec = hd->common.ndrec;
    if (ndrec == -1)		/* Input is a pipe. */
    {
	char	*mktemp();
	char	*tmp_file;		/* temporary file name */
	FILE	*tmp_strm;		/* temporary stream */
	char	*template = "genplotXXXXXX";
	int	recsize;
	char	*tmp_buf;

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

	recsize = size_rec(hd);

	if (recsize == -1)
	{
	    ERROR("variable record size not supported.");
	}

	if (debug_level == 6)
	    Fprintf(stderr,
		"%s: opened temp file %s, recsize = %d; allocate memory..\n",
		ProgName, tmp_file, recsize);

	TRYALLOC(char, recsize, tmp_buf)

	ndrec = 0;
	while (fread (tmp_buf, recsize, 1, stdin) == 1)
	{
	    if (fwrite (tmp_buf, recsize, 1, tmp_strm) != 1) {
		Fprintf (stderr, "%s: write error on %s\n",
		ProgName, tmp_file);
		exit (1);
	    }
	    ndrec++;
	}

	if (debug_level == 6) {
	    Fprintf (stderr,
	    "%s: %ld records from stdin written to temporary file (%s).\n",
	    ProgName, ndrec, tmp_file);
	}

	(void) rewind(tmp_strm);
	Fclose(stdin);
	infile = tmp_strm;
    }

    if (iflag)
    {
	firstrec = start;
	lastrec = start;
    }
    else
    {
	firstrec = start;
	lastrec = ndrec + start - 1;
    }

    lrange_switch(rrange, &firstrec, &lastrec, 0);

    if (lastrec < firstrec) {
	Fprintf(stderr, "%s: bad range given, last record < first record.\n",
		ProgName);
	exit(1);
	}


    if (firstrec < start) 
	firstrec = start;

    if (lastrec > ndrec + start - 1)
	lastrec = ndrec + start - 1;

    nrec = lastrec - firstrec + 1;

    if (debug_level > 1)
    {
	Fprintf(stderr,
	    "%s: firstrec = %ld, lastrec = %ld, nrec = %ld.\n",
	    ProgName, firstrec, lastrec, nrec);
	(void) fflush(stderr);
    }


    /* get number of elements to plot */

    if (eflag)
    {
	if (nfld == 0)
	{
	    elem_array = grange_switch(erange, &nelem);
     	    first_ele = elem_array[0];
	    last_ele = elem_array[nelem - 1];
	}
	else
	{
	    if (is_file_complex(hd)) {
		Fprintf(stderr,
		 "genplot: Sorry, cannot deal with a feature file with complex fields this way, yet.\n");
	 	Fprintf(stderr,
		 "genplot: Use fea_element and then plot by element numbers.\n");
		exit(1);
	    }
	    elem_array =
		fea_range_switch(fea_fields, hd,
			&nelem, &fea_names, &fea_indices);
	    first_ele = LONG_MAX;
	    last_ele = LONG_MIN;
	    for (i = 0; i < nelem; i++)
	    {
		if (elem_array[i] < first_ele) first_ele = elem_array[i];
		if (elem_array[i] > last_ele) last_ele = elem_array[i];
	    }
	}

	if (first_ele < 0)
	{
	    Fprintf(stderr,
		"%s: element number should not be less than zero.\n",
		ProgName);
	    exit(1);
	}
	if (last_ele > get_rec_len(hd) )
	{
	    last_ele = get_rec_len(hd);
	    Fprintf(stderr,
		"%s: only %d elements in a record, truncating specified range.\n",
		ProgName, last_ele);
	}

	if ((first_ele == 0) && (hd->common.tag == NO))
	{
	    Fprintf(stderr,
		"%s: can't plot element zero: data is not tagged.\n",
		ProgName);
	    exit(1);
	}
	  /*
	   * Subtract 1 from first and last element because dbuf in
	   * get_gen_recd starts at element 0.
	   *
	   */

	   first_ele -= 1;
	   last_ele -= 1;

	   for (i = 0; i < nelem; i++)
		elem_array[i] -= 1;

	if (debug_level > 1)
	{
	    Fprintf(stderr, "%s: first_ele = %ld, last_ele = %ld.\n",
		ProgName, first_ele, last_ele);
	    (void) fflush(stderr);
	}

	if (debug_level > 10)
	{
	    Fprintf(stderr,
		"\n%s: num. of elements selected: nelem = %ld\n",
		ProgName, nelem);
	    for (i = 0; i < nelem; i++)
		Fprintf(stderr, "elem_array[%d] = %d\n", i, elem_array[i]);
	}

    } /* end if (eflag) */


    /* get items from feature file for plotting */

    if (iflag)
    {
	if (debug_level > 5)
	    Fprintf(stderr, "%s: irange = %s\n", ProgName, irange);

	if (nfld == 0)
	{
	    item_array = grange_switch(irange, &nitem);	
     	    first_itm = item_array[0];
	    last_itm = item_array[nitem - 1];
	}
	else
	{
	    if (is_file_complex(hd)) {
		Fprintf(stderr,
		 "genplot: Sorry, cannot deal with a feature file with complex fields this way, yet.\n");
	 	Fprintf(stderr,
		 "genplot: Use fea_element and then plot by element numbers.\n");
		exit(1);
	    }
	    item_array =
		fea_range_switch(fea_fields, hd,
			&nitem, &fea_names, &fea_indices);
	    first_itm = LONG_MAX;
	    last_itm = LONG_MIN;
	    for (i = 0; i < nitem; i++)
	    {
		if (item_array[i] < first_itm) first_itm = item_array[i];
		if (item_array[i] > last_itm) last_itm = item_array[i];
	    }
	}

	if (first_itm < 0)
	{
	    Fprintf(stderr,
		 "item range should not be less than zero.\n", ProgName);
	    exit(1);
	}
	if (item_array[nitem - 1] > get_rec_len(hd) )
	{
	    Fprintf(stderr,
		"%s: only %d items in a record.\n",
		ProgName, get_rec_len(hd));
	    exit(1);
	}

	if ((first_itm == 0) && (hd->common.tag == NO))
	{
	    Fprintf(stderr,
		"%s: can't plot item zero: data is not tagged.\n",
		ProgName);
	    exit(1);
	}

	for (i = 0; i < nitem; i++)
	    item_array[i] -= 1;

	if (debug_level > 10)
	{
	    Fprintf(stderr,
		"\n%s: number of items selected: nitem = %ld\n",
		ProgName, nitem);
	    for (i = 0; i < nitem; i++)
		Fprintf(stderr,
		    "item_array[%d] = %d\n", i, item_array[i]);
	}

    } /* end if (iflag) */

    /* get ordinate range */

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

    if (debug_level > 1)
    {
	Fprintf(stderr, "%s: skipping %ld records...\n",
	    ProgName, firstrec - start);
	(void) fflush(stderr);
    }

    if (firstrec > start)
	fea_skiprec(infile, firstrec - start, hd);

    xinc = deltau / xscale;

/*
 * We must compute the maximum ordinate of each element that
 * we are going to plot.  I'll compute in the main loop.
 *
 *	yscale = deltav / (yhigh - ylow);
 *
 *	yticint = (yhigh - ylow) / DEF_Y_SUBDIV;
 */

/*
 * N O T E:
 *
 * If the -e option is given, then there will be nelem different
 *	graphs to plot (i.e. n_graphs will be nelem and
 *			     n_points will be nrec).
 *
 * If the -i option is given, then there will be nrec different
 *	graphs to plot (i.e. n_graphs will be nrec and
 *			     n_points will be nitem).
 *
 */

    if (eflag)
    {
	n_graphs = nelem;
	n_points = nrec;
    }
    if (iflag)
    {
	n_graphs = nrec;
	n_points = nitem;
    }

    if (debug_level > 1)
    {
	Fprintf(stderr, "%s: xinc = %ld\n", ProgName, xinc);
	Fprintf(stderr, "%s: n_graphs = %d, n_points = %d\n",
	    ProgName, n_graphs, n_points);
	Fprintf(stderr, "%s: allocating memory for data, u, and v....\n",
	    ProgName);
	(void) fflush(stderr);
    }

/*
 * Allocate memory for data, u, and v.
 * We need to allocate:
 *
 *	(number of graphs to plot) [n_graphs]
 *			by
 *	(number of data points in one "window") [xinc]
 *
 * memory space.
 *
 * Note: data will be xinc by n_graphs, whereas
 *	 u and v are n_graphs by xinc
 *
 */

    (void) allo_d_matrix(n_graphs, n_points);
    (void) allo_d_array(n_graphs);

    if (debug_level > 2)
	Fprintf(stderr,
	    "%s: record length is %d\n", ProgName, get_rec_len(hd));

    if ((data = (double *) calloc((unsigned) get_rec_len(hd), sizeof(double)))
	== NULL )
    {
	Fprintf(stderr,
	    "%s: calloc:  could not allocate memory for data.\n",
	ProgName);
	exit(1);
    }

    if ((Tag_Array = (long *) calloc((unsigned) nrec, sizeof(long)))
	== NULL )
    {
	Fprintf(stderr,
	    "%s: calloc:  could not allocate memory for Tag_Array.\n",
	    ProgName);
	exit(1);
    }

    /* read data from the ESPS generic file and store in data array */

    if (debug_level > 3)
    {
	Fprintf(stderr,
	    "%s: reading %ld data records from ESPS file...\n",
	    ProgName, nrec);
	if (func_codes[function_type] != NULL)
	    Fprintf(stderr,
		"%s: The %s function will be applied to all data records.\n",
		ProgName, func_codes[function_type]);
	(void) fflush(stderr);
    }

    /*
     * R E A D
     *   D A T A
     *
     */

    if ( eflag )
    {

	if (debug_level > 2)
	    Fprintf(stderr,
		"%s: reading data from generic file (eflag set).\n",
		ProgName);

	for ( i = 0; i < nrec; i++ )
	{
	    if (get_gen_recd(data, &tag, hd, infile) == EOF)
	    {
		Fprintf(stderr, "%s: only %d records read.\n",
		    ProgName, i);
		exit(1);
	    }

	    Tag_Array[i] = tag;

	    for (i_ele = 0; i_ele < nelem; ++i_ele)
	    {
		if (elem_array[i_ele] == -1)
		    Elem_Matrix[i_ele][i] = Tag_Array[i];
		else
		    switch (function_type)
		    {
		    case NONE:
			Elem_Matrix[i_ele][i] = data[elem_array[i_ele]];
			break;
		    case SQRT:
			if (data[elem_array[i_ele]] < 0.0)
			{
			    Fprintf(stderr,
				"%s: fatal error: negative argument for sqrt.\n",
				ProgName);
			    Fprintf(stderr,
				"%s: at element %d, record number %d\n",
				ProgName, elem_array[i_ele] + 1, i);
			    exit(1);
			}
			Elem_Matrix[i_ele][i] = sqrt(data[elem_array[i_ele]]);
			break;
		    case LOG:
			if (data[elem_array[i_ele]] <= 0.0)
			{
			    Fprintf(stderr,
				"%s: fatal error: argument out of range for log.\n",
				ProgName);
			    Fprintf(stderr,
				"%s: at element %d, record number %d\n",
				ProgName, elem_array[i_ele] + 1, i);
			    exit(1);
			}
			Elem_Matrix[i_ele][i] = log(data[elem_array[i_ele]]);
			break;
		    default:
			Fprintf(stderr, "%s: illegal function specified.\n",
			    ProgName);
			exit(1);
			break;
		    } /* end switch (function_type) */

		/*
		 * Compute the maximum and minimum values of the each element
		 * across all records as the data is read in.
		 */

		if (alo_flag)
		    ylow[i_ele] = alow;
		else
		{
		    if (i == 0)
			ylow[i_ele] = Elem_Matrix[i_ele][i];
		    else
		    {
			if (Elem_Matrix[i_ele][i] < ylow[i_ele])
			    ylow[i_ele] = Elem_Matrix[i_ele][i];
		    }
		}

		if (ahi_flag)
		    yhigh[i_ele] = ahigh;
		else
		{
		    if (i == 0)
			yhigh[i_ele] = Elem_Matrix[i_ele][i];
		    else
		    {
			if (Elem_Matrix[i_ele][i] > yhigh[i_ele])
			    yhigh[i_ele] = Elem_Matrix[i_ele][i];
		    }
		}

		if (debug_level > 5)
		{
		    Fprintf(stderr,
			"\n%s: loop index i = %d, elem_array[%d] = %d\n",
			ProgName, i, i_ele, elem_array[i_ele]);
		    Fprintf(stderr,
			"%s: Elem_Matrix[%d][%d] = %g, data[%d] = %g, tag = %d\n",
			ProgName, i_ele, i, Elem_Matrix[i_ele][i],
			elem_array[i_ele], data[elem_array[i_ele]], tag);
		    Fprintf(stderr,
			"%s: yhigh[%d] = %g, ylow[%d] = %g\n",
			ProgName, i_ele, yhigh[i_ele], i_ele, ylow[i_ele]);
		}
	    }

	} /* end for (i =0; i < nrec; i++) */

	for (i_ele = 0; i_ele < nelem; ++i_ele)
	    if (ylow[i_ele] >= yhigh[i_ele])
	    {
		Fprintf(stderr, "%s: element %ld has zero range.",
		    ProgName, elem_array[i_ele] + 1);
		exit(1);
	    }
    
    }  /* end if ( eflag ) */


    /*
     * R E A D
     *   D A T A
     *
     */

    if ( iflag )
    {

	if (debug_level > 2)
	    Fprintf(stderr,
		"%s: reading data from feature file (iflag set).\n",
		ProgName);

	for ( i = 0; i < nrec; i++ )
	{

	    if (get_gen_recd(data, &tag, hd, infile) == EOF)
	    {
		Fprintf(stderr, "%s: only %d records read.\n",
		    ProgName, i);
		exit(1);
	    }

	    Tag_Array[i] = tag;

	    for (i_ele = 0; i_ele < nitem; ++i_ele)
	    {
		if (item_array[i_ele] == -1)
		    Elem_Matrix[i][i_ele] = Tag_Array[i];
		else
		    switch (function_type)
		    {
		    case NONE:
			Elem_Matrix[i][i_ele] = data[item_array[i_ele]];
			break;
		    case SQRT:
			if (data[item_array[i_ele]] < 0.0)
			{
			    Fprintf(stderr,
				"%s: fatal error: argument out of range for sqrt.\n",
				ProgName);
			    Fprintf(stderr,
				"%s: at item %d record number %d\n",
				ProgName, item_array[i_ele], i);
			    exit(1);
			}
			Elem_Matrix[i][i_ele] = sqrt(data[item_array[i_ele]]);
			break;
		    case LOG:
			if (data[item_array[i_ele]] <= 0.0)
			{
			    Fprintf(stderr,
				"%s: fatal error: argument out of range for log.\n",
				ProgName);
			    Fprintf(stderr,
				"%s: at item %d, record number %d\n",
				ProgName, item_array[i_ele], i);
			    exit(1);
			}
			Elem_Matrix[i][i_ele] = log(data[item_array[i_ele]]);
			break;
		    default:
			Fprintf(stderr, "%s: illegal function specified.\n",
			    ProgName);
			exit(1);
			break;
		    } /* end switch (function_type) */

		/*
		 * Compute the maximum and minimum values of the each element
		 * across all records as the data is read in.
		 */

		if (alo_flag)
		    ylow[i] = alow;
		else
		{
		    if (i_ele == 0)
			ylow[i] = Elem_Matrix[i][i_ele];
		    else
		    {
			if (Elem_Matrix[i][i_ele] < ylow[i])
			    ylow[i] = Elem_Matrix[i][i_ele];
		    }
		}

		if (ahi_flag)
		    yhigh[i] = ahigh;
		else
		{
		    if (i_ele == 0)
			yhigh[i] = Elem_Matrix[i][i_ele];
		    else
		    {
			if (Elem_Matrix[i][i_ele] > yhigh[i])
			    yhigh[i] = Elem_Matrix[i][i_ele];
		    }
		}

		if (debug_level > 5)
		{
		    Fprintf(stderr,
			"\n%s: loop index i = %d, item_array[%d] = %d\n",
			ProgName, i, i_ele, item_array[i_ele]);
		    Fprintf(stderr,
			"%s: Elem_Matrix[%d][%d] = %g, data[%d] = %g\n",
			ProgName, i, i_ele, Elem_Matrix[i][i_ele],
			item_array[i_ele], data[item_array[i_ele]]);
		    Fprintf(stderr,
			"%s: yhigh[%d] = %g, ylow[%d] = %g\n",
			ProgName, i, yhigh[i], i, ylow[i]);
		}
	    }

	    if (ylow[i] >= yhigh[i]) 
	    {
		Fprintf(stderr, "record %d has zero range.", i);
		exit(1);
	    }

	} /* end for (i = 0; i < nrec; i++) */

    }  /* end if ( iflag ) */


    if (debug_level > 0)
	Fprintf(stderr,
	    "\n%s: data has been read, now compute scaling factors\n",
	    ProgName);

    /*
     * Now compute yscale from yhigh and ylow arrays.
     */

    for ( i_ele = 0; i_ele < n_graphs; ++i_ele )
    {
	delta_y = yhigh[i_ele] - ylow[i_ele];
	yscale[i_ele] = deltav / delta_y;
	yticint[i_ele] = (yhigh[i_ele] - ylow[i_ele]) / DEF_Y_SUBDIV;

	if (debug_level > 3 && !yflag)
	    Fprintf(stderr,
		"%s: yhigh[%d] = %g, ylow[%d] = %g\n",
		ProgName, i_ele, yhigh[i_ele], i_ele, ylow[i_ele]);

	if (debug_level > 3)
	    Fprintf(stderr,
		"%s: yscale[%d] = %g, yticint[%d] = %g\n",
		ProgName, i_ele, yscale[i_ele], i_ele, yticint[i_ele]);
    }

    page_num = 0;

    ulow = u0;
    vlow = v0;

    index = 0;

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

    if (debug_level > 0)
	Fprintf(stderr,
	    "%s: beginning main WHILE loop:\n", ProgName);

    if (Eflag)
    {
	xlow = Tag_Array[0] - 1;
	xmin = Tag_Array[0];
    }
    else if (iflag)
    {
	xlow = item_array[0];
	xmin = item_array[0] + 1;
    }
    else
    {
	xlow = firstrec - 1;
	xmin = firstrec;
    }

    if (debug_level > 4)
	Fprintf(stderr,
	    "%s: xlow = %d, xmin = %d\n\n BEGIN MAIN LOOP:\n\n",
	ProgName, xlow, xmin);

    if (gps && !nflag) gps_null_comment(stdout);

    while (continue_plotting)
    {
	if (Eflag)
	    xmax = MIN(xlow + xinc, Tag_Array[nrec - 1]);
	else if (iflag)
	    xmax = MIN(xlow + xinc, nitem + item_array[0]); 
	else
	    xmax = MIN(xlow + xinc, lastrec);

	if (debug_level > 4)
	    Fprintf(stderr,
		"\n\n MAIN LOOP\n%s: xmax = %d, xmin = %d, xlow = %d, xinc = %d\n",
		ProgName, xmax, xmin, xlow, xinc);

	/*
	 * R E A D
	 *   S T O R E D
	 *         D A T A:
	 *
	 * Read in data from Elem_Matrix and scale each plot accordingly.
	 *
	 */


	/*
	 * save the value of vlow (the vertical border)
	 * you need it to go back to the starting place to begin plotting.
	 *
 	 */

	old_vlow = vlow;


	/*
	 * Note:  xmin and xmax have different meanings depending upon
	 *	  the -E option (Eflag)
	 */

	for ( x = xmin, i = 0; x <= xmax; ++x, ++index, ++i )
	{

	    /*                                    ^
	     *					  |
	     *
	     * Notice:  index is initialized outside MAIN LOOP,
	     *		it is used to index Tag_Array and Elem_Matrix
	     *		matrices across records.
	     */

	    int	BreakLoop = 0;	/* used to get out of the loop
				 * I think a goto would have been better
				 */

	    vlow = old_vlow;	/* get back to the original vertical starting position */

	    for (i_ele = 0; i_ele < n_graphs; ++i_ele)
	    {

		if (Eflag)
		{
		    /*
		     * S T O P !
		     *
		     * when Tag value is greater than or equal to xmax
		     *
		     */

		    if (debug_level > 3)
			Fprintf(stderr,
			    "%s: Tag_Array[%d] = %ld, x = %d, xmin = %d, xmax = %d, xlow = %d\n",
			    ProgName, index, Tag_Array[index], x, xmin, xmax, xlow);

		    if ((Tag_Array[index] <= xmax) && (index < nrec))
			u[i_ele][i] = ulow + 
				(Tag_Array[index] - xlow) * xscale;
		    else
		    {
			old_xmin = x;
			xmax = x - 1;

			BreakLoop = YES;

			if (debug_level > 3)
			    Fprintf(stderr,
				"%s: setting old_xmin and xmax to %d AND breaking loop cycle.\n",
				ProgName, x);

			break;
		    }
		}
		else
		    u[i_ele][i] = ulow + (x - xlow) * xscale;

		v[i_ele][i] = vlow +
		    LROUND((Elem_Matrix[i_ele][index] - ylow[i_ele]) *
			yscale[i_ele]);


		if (debug_level > 7)
		{
		    Fprintf(stderr, "\n%s: u[%d][%d] = %ld, v = %ld\n",
			ProgName, i_ele, i, u[i_ele][i], v[i_ele][i]);
		    Fprintf(stderr,
			"%s: Elem_Matrix[%d][%d] = %g, ylow = %g, yscale = %g\n",
			ProgName, i_ele, index, Elem_Matrix[i_ele][index],
			ylow[i_ele], yscale[i_ele]);
		}

		/*
		 * Keep track of which line we are plotting
		 * and shift vlow accordingly.
		 */

		if (i_ele == 0)
		    line_num = nlines;

		if (++line_num < maxlines)
		    vlow -= vshift;
		else
		{
		    vlow = v0;
		    line_num = 0;
		}

	    }

	    if (BreakLoop)
		break;

	    /*
	     * Somehow, all the stuff up there works!
	     *
	     */
	}

	vlow = old_vlow;	/* get the old vertical postion */

	/*
	 * B E G I N
 	 *
 	 *    P L O T T I N G:
 	 *
 	 */

	for (i = 0, i_ele = 0, rec_num = firstrec;
	    i < n_graphs;
	    ++i, ++i_ele, rec_num++)
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
			    infilename, (char *) NULL, xscale, plotting_func);

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

	    delta_y = yhigh[i_ele] - ylow[i_ele];

	    drawaxes(ulow, vlow, xlow, ylow[i_ele], xinc + 1, delta_y,
 		    xscale, yscale[i_ele], xticint, yticint[i_ele], 
 		    lbl_charsp, axflag, plotting_func);


	    /*
	     * label the current element we're plotting
	     * remember to add one to the element number, right?
	     * (see -e range specifications up there)/
	     *
	     */

	    if (iflag)
		Sprintf(element_text, "record%d", rec_num);
	    else if (elem_array[i] == -1)
		Sprintf(element_text, "tag");
	    else if (nfld == 0)
		Sprintf(element_text, "element%d", elem_array[i] + 1);
	    else
		Sprintf(element_text, "%s[%ld]", fea_names[i], fea_indices[i]);

	    text(   y_ttl_base + deltau + 400,
		    vlow + deltav/2
		    + (y_ttl_charsp * strlen(element_text))/2,
		    y_ttl_charsp, -90, element_text, plotting_func);


	    if (debug_level > 3)
	    {
		Fprintf(stderr, "%s: plotting data...\n", ProgName);
		(void) fflush(stderr);
	    }


	    switch (mode)
	    {

	    case M_POINT:
		if (tek)
		    tek_plotpoints(xmax - xmin + 1, u[i_ele], v[i_ele]);
		else /* gps */
		    for (x = xmin, j = 0; x <= xmax; ++x, ++j)
			DRAW(u[i_ele][j], v[i_ele][j],
			     u[i_ele][j], v[i_ele][j], plotting_func)
		break;

	    case M_CONN:
		if (tek)
		    tek_plotline(xmax - xmin + 1, u[i_ele], v[i_ele]);
		else /* gps */
		    gps_plotline(xmax - xmin + 1, u[i_ele], v[i_ele]);
		break;

	    case M_VERT:
		{
		    long    base = 
			    (ylow[i_ele] >= 0) ? vlow :
			    (yhigh[i_ele] <= 0) ? vlow + deltav :
				vlow - LROUND(ylow[i_ele] * yscale[i_ele]);

		    for (x = xmin, j = 0; x <= xmax; ++x, ++j)
			DRAW(u[i_ele][j], v[i_ele][j],
			    u[i_ele][j], base,
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

	if (Eflag)
	{
	    recd_num += xmax - xmin + 1;
	    xlow += xinc; xmin = old_xmin;

	    if (debug_level > 3)
		Fprintf(stderr, "%s: xlow = %d, recd_num = %d\n",
		    ProgName, xlow, recd_num);

	    if (recd_num >= nrec)
		continue_plotting = NO;
	}
	else
	{
	    xlow += xinc;
	    xmin = xlow + 1;

	    if (iflag)
	    {
		if (xmin >= nitem + item_array[0])
		    continue_plotting = NO;
	    }
	    else
		if (xmin > lastrec)
		    continue_plotting = NO;
	}

    }	/* Main while loop */

    if (nlines > 0)
	if (imagen)
	    tek_termpage();

    exit(0);
    /*NOTREACHED*/
} /* end main */


allo_d_matrix(rows, columns)
    int	    rows;
    int	    columns;
{
    char    *arr_alloc();
    long    dim[2];

    dim[0] = rows;
    dim[1] = columns;

    Elem_Matrix = (double **) arr_alloc(2, dim, DOUBLE, 0);
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
