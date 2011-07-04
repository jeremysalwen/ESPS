/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|	  "Copyright (c) 1986, 1987, 1988 Entropic Speech, Inc.		|
|                         All rights reserved."				|
|									|
+-----------------------------------------------------------------------+
|									|
|  plotspec -- plot spectral data in plotas input format		|
|									|
|  Originally by Shankar Narayan, EPI.  SDS version by Joe Buck.	|
|  SPS version by Rod Johnson.  Modified by John Shore to remove	|
|  common.ndrec dependence.  -d option by Ajaipal S. Virdy.		|
|  -D, -F and -l options by Rod Johnson.  Converted from SPEC to	|
|  FEA_SPEC input by Rod Johnson.					|
|									|
|  Module:  plotspec.c							|
|									|
|  This, the main module of plotspec, interprets the command line,	|
|  sets up and checks input, output, and temporary files, and reads	|
|  the data.  Using a temporary file, it makes two passes over the	|
|  data--one to determine the maximum and minimum values and one to	|
|  draw the plot.  It calls on plotscale to determine axis labels,	|
|  draw_box to draw and label the axes, plotdata to plot the data,	|
|  and printtime to write the date and time on the plot.		|
|									|
|  Input to plotspec is an ESPS FEA_SPEC file.  More than one record	|
|  may be plotted, superposed on one set of axes.  Output, in the	|
|  ``Stanford'' Ascii format accepted by plotas and stou, is written	|
|  to the standard output.						|
|


modified so that atan2: domain error when 0 complex number is encountered
Derek Lin
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
       static char *sccs_id = "@(#)plotspec.c	1.2 7/23/98     ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/constants.h>
#include <esps/fea.h>
#include <esps/feaspec.h>


#define THISPROG "plotspec"

#define SYNTAX \
USAGE("plotspec [-d][-l][-r range][-t text]... [-x debug_level][-y range]\n [-D][-E][-F][-T device][-X range][-Y range] file.spec")
	/* -T option handled by command script, not C program. */

#define TRYALLOC(type,num,var) { \
    if (((var) = (type *) malloc((unsigned)(num)*sizeof(type)))==NULL) \
    { Fprintf(stderr, "%s:  can't allocate memory for %d points.\n", \
		THISPROG, (int)(num)); \
	exit(1); }}

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define Printf (void) printf

#define MAXTEXT	    10
#define kHz	    1000.0

/* external system functions */

double	pow(), log10(), atan2();

/* external ESPS functions */

void	lrange_switch();
void	frange_switch();
void	plotscale();
void	plotexscale();
char	**genhd_codes();

/* local ESPS functions */

void	erase();
char    *e_temp_name();

/* external variables */

extern int  optind;		/* Used in command-line parsing by getopt */
extern char *optarg;		/* Used in command-line parsing by getopt */

/* global variable */

char	*ProgName = THISPROG;
int	debug_level = 0;	


/* MAIN PROGRAM */

main(argc, argv)
    int     argc;
    char    **argv;
{
    int     c;			/* Command-line option letter */

    struct feaspec	*spec_rec;  /* Input data record */
    struct header	*ih;	/* Input ESPS file header */
    struct header	*temp_hd;	/* Temporary ESPS file header */

    FILE    *specfile;		/* Input file pointer */
    FILE    *tempfile;		/* Temporary file pointer */

    char    *specname;		/* Name of input file */

    char    *tempname = "plspXXXXXX";
				/* Template for temp file name */
    char    *newname;		/* Temp file name */
    char    *rrange = NULL;	/* Range of records to plot (-r option) */
    char    *xrange = NULL;	/* Range of frequencies (-X option) */
    char    *yrange = NULL;	/* Spectral value range (-y & -Y options) */

    int	    yflag = NO;		/* -Y or -y option specified? */
    int	    yexact = NO;	/* -Y option specified? */

    char    *text[MAXTEXT];	/* Text lines below plot (-t option) */
    int	    ntext = 0;		/* Number of text lines below plot */

    int	    sflag = NO;		/* -s option specified? */
    int	    dflag = NO;		/* -d option specified? */
    int	    lflag = NO;		/* -l option specified? */
    int	    Dflag = NO;		/* -D option specified? */
    int	    Fflag = NO;		/* -F option specified? */
    int     Eflag = NO;		/* -E option specified? */

    double  xmin, xmax;		/* x-axis limits */
    double  xmin_def;		/* default xmin */
    double  xmink, xmaxk;	/* x-axis limits (kilohertz) */
    double  xstepk;		/* x-axis tic spacing */
    double  ymin, ymax;		/* y-axis limits */
    double  ystep;		/* y-axis tic spacing */

    short   freq_format;	/* How set of freqencies is defined. */
    short   spec_type;		/* Is spectrum log power, complex, etc.? */
    short   contin;		/* Continuous or discrete spectrum? */
    long    num_freqs;		/* Number of points in spectrum record */
    long    start_plot;		/* Index of first point in record to plot */
    long    end_plot;		/* Index of last point in record to plot */
    int	    num_plot;		/* Number of points in record to plot */
    float   sf;			/* Sampling frequency */
    float   *freqs;		/* Frequencies of points in spectrum */
    double  freqmin;		/* Frequency of first point in record */
    double  freqmax;		/* Frequency of last point in record */
    double  datamin;		/* Lowest data value in range to be plotted */
    double  datamax;		/* Highest data value in range to be plotted */

    float   *dist_func;		/* Cumulative distribution function */
    float   *dfreqs;		/* Frequencies of points in distr. function */

    double  delta_h;		/* Difference between successive frequencies */
    double  R0;			/* Running total of power in spectrum */
    double  delta_R;		/* Increment in R0 */
    double  rsv, isv;		/* Real & imag. spectral values from record */
    double  tot_power;		/* Total power in spectrum */
    float   zero_power = 0.0;	/* Used when tot_power field not defined
				    in input file. */

    int	    xdp, ydp;		/* Number of decimal places for scale marks */

    long    firstrec, lastrec;	/* First and last records in specified range */
    long    nplots;		/* Number of records in specified range */
    long    nrec;		/* Number of records to actually plot */

    int	    i, j, iy, k;	/* Loop indices */
    int	    clr = 0;		/* Color */
    char    s[80];
    char    *Wflag = NULL;	/* used to specify x geometry */
    char    *title = NULL;      /* used for holding user-specifed title -
				 not used yet here; see aplot.c */

/* Process command-line options */

    while ((c = getopt(argc, argv, "dlr:st:x:y:DEFX:Y:W:")) != EOF)
	switch (c)
	{
	case 'd':		/* plot cumulative distribution function */
	    dflag++;
	    break;
	case 'l':		/* plot frequencies with log scale */
	    lflag++;
	    break;
	case 'r':
	    rrange = optarg;
	    break;
	case 's':		/* plot records on separate pages */
				/* (not working) */
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
	    yrange = optarg;
	    yflag = YES;
	    break;
	case 'D':		/* convert to dB */
	    Dflag++;
	    break;
	case 'E':		/* don't erase */
	    Eflag++;
	    break;
	case 'F':		/* plot phase */
	    Fflag++;
	    break;
	case 'X':
	    xrange = optarg;
	    break;
	case 'Y':
	    yrange = optarg;
	    yflag = YES;
	    yexact = YES;
	    break;
	case 'W':
	    Wflag = optarg;
	    break;
	default:
	    SYNTAX;
	    break;
	}

/* Process file name argument and open file */


    if (optind >= argc)
    {
	Fprintf(stderr, "%s: no input file\n", ProgName);
	SYNTAX;
	exit(1);
    }
    else if (optind < argc - 1)
    {
	Fprintf(stderr, "%s: more than one input file\n", ProgName);
	SYNTAX;
	exit(1);
    }
    else
	specname = eopen(ProgName,
		argv[optind], "r", FT_FEA, FEA_SPEC, &ih, &specfile);

    tk_init(THISPROG, specname, Wflag, title);

    temp_hd = copy_header(ih);

/* Get generic header items.
   Check for violations of certain limitations. */

    REQUIRE(genhd_type("freq_format", (int *) NULL, ih) == CODED,
	    "Header item \"freq_format\" wrong type or undefined.")
    freq_format = *get_genhd_s("freq_format", ih);

    switch (freq_format)
    {
    case SPFMT_SYM_EDGE:
    case SPFMT_SYM_CTR:
    case SPFMT_ASYM_EDGE:
    case SPFMT_ASYM_CTR:
	break;
    default:
	Fprintf(stderr, "%s: only freq formats %s supported so far.\n",
	    "SYM_EDGE, SYM_CTR, ASYM_EDGE, ASYM_CTR", ProgName);
	exit(1);
	break;
    }

    REQUIRE(genhd_type("spec_type", (int *) NULL, ih) == CODED,
	    "Header item \"spec_type\" wrong type or undefined.")
    spec_type = *get_genhd_s("spec_type", ih);

    REQUIRE(genhd_type("contin", (int *) NULL, ih) == CODED,
	    "Header item \"contin\" wrong type or undefined.")
    contin = *get_genhd_s("contin", ih);

    if (!contin)
    {
	Fprintf(stderr, "%s: discrete distributions not yet supported.\n",
	    ProgName);
	exit(1);
    }

    REQUIRE(genhd_type("num_freqs", (int *) NULL, ih) == LONG,
	    "Header item \"num_freqs\" wrong type or undefined.")
    num_freqs = *get_genhd_l("num_freqs", ih);

    REQUIRE(genhd_type("sf", (int *) NULL, ih) == FLOAT,
	    "Header item \"sf\" wrong type or undefined.")
    (sf = *get_genhd_f("sf", ih)) || (sf = 1.0);

    if (sflag) 
    {
	Fprintf(stderr, "%s: -s option not implemented yet.\n", ProgName);
	exit(1);
    }

    if (dflag && Fflag)
    {
	Fprintf(stderr, "%s: -d and -F options incompatible.\n", ProgName);
	exit(1);
    }

    if (dflag && Dflag)
    {
	Fprintf(stderr, "%s: -d and -D options incompatible.\n", ProgName);
	exit(1);
    }

    if (Dflag && Fflag)
    {
	Fprintf(stderr, "%s: -D and -F options incompatible.\n", ProgName);
	exit(1);
    }

    if (Fflag && spec_type != SPTYP_CPLX)
    {
	Fprintf(stderr, "%s: -F option requires complex data.\n",
		ProgName);
	exit(1);
    }

/* get range of records to try to plot */

    firstrec = 1;
    lastrec = LONG_MAX;
    lrange_switch(rrange, &firstrec, &lastrec, 0);

    if (firstrec < 1) firstrec = 1;

    nplots = lastrec - firstrec + 1;

/* Open temporary file */

    if ((tempfile = fopen(newname = e_temp_name(tempname), "w+")) == NULL) 
    {
	perror(ProgName);
	Fprintf(stderr, "%s: can't open temporary file.\n", ProgName);
	exit(1);
    }
    (void) unlink(newname);
    write_header(temp_hd, tempfile);

    spec_rec = allo_feaspec_rec(ih, FLOAT);
    if (spec_rec->tot_power == NULL)
    {
	spec_rec->tot_power = &zero_power;
    }

/* allocate memory */

    TRYALLOC(float, num_freqs, freqs)

    if (dflag)
    {
	TRYALLOC(float, num_freqs + 1, dist_func)
	TRYALLOC(float, num_freqs + 1, dfreqs)
    }

    switch (freq_format) 
    {
    case SPFMT_SYM_CTR:
	delta_h = sf / (2 * num_freqs);
	if (dflag)
	{
	    freqmin = dfreqs[0] = 0.0;
	    freqmax = dfreqs[num_freqs] = 0.5 * sf;
	    for (j = 1; j < num_freqs; j++)
		dfreqs[j] = j * delta_h;
	}
	else
	{
	    freqmin = 0.5 * delta_h;
	    freqmax = 0.5 * sf - freqmin;
	}
	for (j = 0; j < num_freqs; j++)
	    freqs[j] = (j + 0.5) * delta_h;
	break;
    case SPFMT_SYM_EDGE:
	delta_h = sf / (2 * (num_freqs - 1));
	if (dflag)
	{
	    freqmin = dfreqs[0] = 0.0;
	    freqmax = dfreqs[num_freqs] = 0.5 * sf;
	    for (j = 1; j < num_freqs; j++)
		dfreqs[j] = (j - 0.5) * delta_h;
	}
	else
	{
	    freqmin = 0.0;
	    freqmax = 0.5 * sf;
	}
	for (j = 0; j < num_freqs; j++)
	    freqs[j] = j * delta_h;
	break;
    case SPFMT_ASYM_CTR:
	delta_h = sf / num_freqs;
	if (dflag)
	{
	    freqmin = dfreqs[0] = -0.5 * sf;
	    freqmax = dfreqs[num_freqs] = -freqmin;
	    for (j = 1; j < num_freqs; j++)
		dfreqs[j] = freqmin + j * delta_h;
	}
	else
	{
	    freqmin = -0.5 * sf + 0.5 * delta_h;
	    freqmax = -freqmin;
	}
	for (j = 0; j < num_freqs; j++)
	    freqs[j] = -0.5 * sf + (j + 0.5) * delta_h;
	break;
    case SPFMT_ASYM_EDGE:
	delta_h = sf / (num_freqs - 1);
	if (dflag)
	{
	    freqmin = dfreqs[0] = -0.5 * sf;
	    freqmax = dfreqs[num_freqs] = -freqmin;
	    for (j = 1; j < num_freqs; j++)
		dfreqs[j] = freqmin + (j - 0.5) * delta_h;
	}
	else
	{
	    freqmin = -0.5 * sf;
	    freqmax = -freqmin;
	}
	for (j = 0; j < num_freqs; j++)
	    freqs[j] = -0.5 * sf + j * delta_h;
	break;
    case SPFMT_ARB_VAR:
	Fprintf(stderr, "%s: freq format ARB_VAR not yet supported.\n",
	    ProgName);
	exit(1);
	break;
    case SPFMT_ARB_FIXED:
	Fprintf(stderr, "%s: freq format ARB_FIXED not yet supported.\n",
	    ProgName);
	exit(1);
	break;
    default:
	Fprintf(stderr, "%s: unrecognized freq format %s.\n",
	    ProgName, genhd_codes("freq_format", ih)[freq_format]);
	exit(1);
	break;
    }

    if (lflag)
    {
	switch (freq_format)
	{
	case SPFMT_SYM_CTR:
	    xmin_def = dflag ? delta_h : 0.5*delta_h;
	    break;
	case SPFMT_SYM_EDGE:
	    xmin_def = dflag ? 0.5*delta_h : delta_h;
	    break;
	default:
	    xmin_def = ((dflag && num_freqs%2 == 0)
			|| (!dflag && num_freqs%2 == 1)) ?
		    delta_h : 0.5*delta_h;
	}
	xmin = xmin_def;
	xmax = freqmax;
	frange_switch(xrange, &xmin, &xmax);
	if (xmin == xmax) xmin = xmin_def;
	spsassert(xmin > 0.0, "frequency range not positive");
    }
    else
    {
	xmin = freqmin; 
	xmax = freqmax;
	frange_switch(xrange, &xmin, &xmax);
	if (xmin == xmax)
	    switch (freq_format)
	    {
	    case SPFMT_SYM_CTR:
	    case SPFMT_SYM_EDGE:
		xmin = 0;
		break;
	    default:
		xmin = -xmax;
		break;
	    }
    }

    if (freqmin < xmin || freqmax > xmax)
	Fprintf(stderr, "frequency out of range.\n");

    if (dflag)
    {
	for (   start_plot = 0;
		start_plot <= num_freqs && dfreqs[start_plot] < xmin;
		start_plot++
	    ) {}

	for (   end_plot = num_freqs;
		end_plot >= 0 && dfreqs[end_plot] > xmax;
		end_plot--
	    ) {}
    }
    else
    {
	for (   start_plot = 0;
		start_plot < num_freqs && freqs[start_plot] < xmin;
		start_plot++
	    ) {}

	for (   end_plot = num_freqs - 1;
		end_plot >= 0 && freqs[end_plot] > xmax;
		end_plot--
	    ) {}
    }

    num_plot = end_plot - start_plot + 1;

    datamin = FLT_MAX;
    datamax = -FLT_MAX;

    skiprec(specfile, firstrec - 1, size_rec(ih));

    nrec = 0;
    while (nplots-- && (get_feaspec_rec(spec_rec, ih, specfile) != EOF))
    {
	for (j = 0; j < num_freqs; j++)
	{
	    if (Dflag && spec_type != SPTYP_DB)
	    {
		double pwr;

		rsv = spec_rec->re_spec_val[j];
		switch (spec_type)
		{
		case SPTYP_PWR:
		    pwr = rsv;
		    break;
		case SPTYP_REAL:
		    pwr = rsv * rsv;
		    break;
		case SPTYP_CPLX:
		    isv = spec_rec->im_spec_val[j];
		    pwr = rsv*rsv + isv*isv;
		    break;
		default:
		    Fprintf(stderr, "%s: unknown spectral type\n", ProgName);
		    exit(1);
		    break;
		}
		spec_rec->re_spec_val[j] =
		    (pwr != 0.0) ? 10 * log10(pwr) : - 10 * log10(FLT_MAX);
	    }
	    if (xmin <= freqs[j] && freqs[j] <= xmax)
	    {
		if (datamin > spec_rec->re_spec_val[j])
		    datamin = spec_rec->re_spec_val[j];
		if (datamax < spec_rec->re_spec_val[j])
		    datamax = spec_rec->re_spec_val[j];

		if (spec_type == SPTYP_CPLX && !Dflag)
		{
		    if (datamin > spec_rec->im_spec_val[j])
			datamin = spec_rec->im_spec_val[j];
		    if (datamax < spec_rec->im_spec_val[j])
			datamax = spec_rec->im_spec_val[j];
		    
		}
	    }
	}
	put_feaspec_rec(spec_rec, temp_hd, tempfile);
	nrec++;
    }

    if (debug_level > 2)
	Fprintf(stderr, "%s: datamin = %g, datamax = %g\n",
	ProgName, datamin , datamax);

    if (nrec == 0) 
    {
	Fprintf(stderr, "%s: no input records\n", ProgName);
	exit(1);
    }

    rewind(tempfile);
    (void)read_header(tempfile);


    if (dflag)
    {
	ymin = 0.0;
	ymax = 1.0;
	ystep = 0.1;
	ydp = 1;
    }
    else if (Fflag)
    {
	ymin = -PI;
	ymax = PI;
	ystep = PI/2;
	ydp = 4;
    }
    else
    {
	ymin = datamin; 
	ymax = datamax;
	frange_switch(yrange, &ymin, &ymax);

	if (datamin < ymin || datamax > ymax)
	    Fprintf(stderr, "data out of range.\n");

	if (yexact)
	    plotexscale(ymin, ymax, 1.0, &ymin, &ymax, &ystep, &ydp);
	else
	    plotscale(ymin, ymax, 1.0, &ymin, &ymax, &ystep, &ydp);
    }

    if (debug_level > 2)
	Fprintf(stderr, "%s: ymin = %g, ymax = %g, ystep = %g, ydp = %d\n",
	ProgName, ymin, ymax, ystep, ydp);

    if (lflag)
    {
	plotscale(log10(xmin), log10(xmax), 1.0, &xmink, &xmaxk, &xstepk, &xdp);
	for (j = start_plot; j <= end_plot; j++)
	    if (dflag)
		dfreqs[j] = log10(dfreqs[j]);
	    else
		freqs[j] = log10(freqs[j]);
    }
    else
    {
	plotscale(xmin/kHz, xmax/kHz, 1.0, &xmink, &xmaxk, &xstepk, &xdp);
	for (j = start_plot; j <= end_plot; j++)
	    if (dflag)
		dfreqs[j] = dfreqs[j]/kHz;
	    else
		freqs[j] = freqs[j]/kHz;
    }

    if (debug_level > 2)
    {
	Fprintf(stderr, "%s: xmin= %g, xmax = %g\n", ProgName, xmin, xmax);
	Fprintf(stderr, "%s: xmink = %g, xmaxk = %g, xstepk = %g, xdp = %d\n",
	ProgName, xmink, xmaxk, xstepk, xdp);
    }

    if (!sflag)
	draw_box(xmink, xmaxk, xstepk, xdp, ymin, ymax, ystep, ydp, 0);

    while (nrec-- && (get_feaspec_rec(spec_rec, temp_hd, tempfile) != EOF))
    {
	if (yflag && !dflag && !Fflag)
	    for (k = 0; k < num_freqs; k++) 
	    {
		if (spec_rec->re_spec_val[k] < ymin)
		    spec_rec->re_spec_val[k] = ymin;
		if (spec_rec->re_spec_val[k] > ymax)
		    spec_rec->re_spec_val[k] = ymax;

		if (spec_type == SPTYP_CPLX && !Dflag)
		{
		    if (spec_rec->im_spec_val[k] < ymin)
			spec_rec->im_spec_val[k] = ymin;
		    if (spec_rec->im_spec_val[k] > ymax)
			spec_rec->im_spec_val[k] = ymax;
		}
	    }

	if (dflag)
	{
	    tot_power = *spec_rec->tot_power;
	    R0 = 0.0;
	    dist_func[0] = R0;
	    for (j = 0; j < num_freqs; j++)
	    {
		rsv = spec_rec->re_spec_val[j];
		switch (spec_type)
		{
		case SPTYP_PWR:
		    delta_R = rsv;
		    break;
		case SPTYP_DB:
		    delta_R = pow(10.0, rsv/10.0);
		    break;
		case SPTYP_REAL:
		    delta_R = rsv * rsv;
		    break;
		case SPTYP_CPLX:
		    isv = spec_rec->im_spec_val[j];
		    delta_R = rsv*rsv + isv*isv;
		    break;
		default:
		    Fprintf(stderr, "%s: unknown spectral type\n", ProgName);
		    exit(1);
		    break;
		}
		switch (freq_format) 
		{
		case SPFMT_SYM_CTR:
		case SPFMT_ASYM_CTR:
		    delta_R *= delta_h;
		    break;
		case SPFMT_SYM_EDGE:
		case SPFMT_ASYM_EDGE:
		    if (j == 0 || j == num_freqs - 1)
			delta_R *= 0.5 * delta_h;
		    else
			delta_R *= delta_h;
		    break;
		case SPFMT_ARB_VAR:
		case SPFMT_ARB_FIXED:
		default:
		    exit(1);
		    break;
		}
		R0 += delta_R;
		dist_func[j+1] = R0;
	    }

	    if (debug_level > 2)
		Fprintf(stderr, "%s: R0 = %g, tot_power = %g, delta_h = %g\n",
		    ProgName, R0, tot_power, delta_h);

	    for (j = 0; j <= num_freqs; j++)
	    {
		if (tot_power != 0.0)
		    dist_func[j] /= tot_power;
		else
		    dist_func[j] /= R0;

	        if (debug_level > 4)
		    Fprintf(stderr, "%s: dist_func[%d] = %g\n",
		        ProgName, j, dist_func[j]);
	    }

	    plot2data(dfreqs + start_plot, dist_func + start_plot,
		    num_plot, clr + 2, xmink, xmaxk, 0.0, 1.0);
	}
	else if (Fflag)
	{
	    for (j = 0; j < num_freqs; j++){
	      if(spec_rec->re_spec_val[j] == 0 && 
		 spec_rec->re_spec_val[j] ==0 )
		spec_rec->im_spec_val[j] = 0;
	      else
		spec_rec->im_spec_val[j] =
		  atan2(spec_rec->im_spec_val[j], spec_rec->re_spec_val[j]);
	    }

	      plotphdata(freqs + start_plot,
			 spec_rec->im_spec_val + start_plot,
			 num_plot, clr + 2, xmink, xmaxk, ymin, ymax);
	}
	else
	{
    	    plot2data(freqs + start_plot,
		    spec_rec->re_spec_val + start_plot,
		    num_plot, clr + 2, xmink, xmaxk, ymin, ymax);

	    if (spec_type == SPTYP_CPLX && !Dflag)
		plot2data(freqs + start_plot,
		    spec_rec->im_spec_val + start_plot,
		    num_plot, clr + 2, xmink, xmaxk, ymin, ymax);
	}

	if (sflag)
	{
	    draw_box(xmink, xmaxk, xstepk, xdp, ymin, ymax, ystep, ydp, 0);

	    tk_snd_plot_cmd("c 5");
	    tk_snd_plot_cmd("m 3400 500");
	    tk_snd_plot_cmd("t 5 1");
	    sprintf(s,"File: %s, Record %d",specname,firstrec++);
	    tk_snd_plot_cmd(s);


	    iy = 3600;
	    for (j = 0; j < ntext; j++) 
	    {
		tk_snd_plot_cmd("c 5");
		tk_snd_plot_cmd_1arg("m %d 500",iy);
		tk_snd_plot_cmd("t 5 1");
		tk_snd_plot_cmd(text[j]);
		iy += 200;
	    }
	    print_time(200, 4400);

	    if (!Eflag) erase();
	}
    }

    (void) fclose(tempfile);

    if (!sflag) 
    {
	if (lflag) {
	    tk_snd_plot_cmd("c 5");
	    tk_snd_plot_cmd("m 3400 2760");
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd("Decades");
        }
	else {
	    tk_snd_plot_cmd("c 5");
	    tk_snd_plot_cmd("m 3400 2920");
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd("kHz");
	}
	
        tk_snd_plot_cmd("c 5");
	tk_snd_plot_cmd("m 3600 500");
	tk_snd_plot_cmd("t 5 1");
	sprintf(s,"File: %s", specname);
	tk_snd_plot_cmd(s);

	if (!dflag && !Fflag && spec_type == SPTYP_DB || Dflag) {
	    tk_snd_plot_cmd("c 5");
	    tk_snd_plot_cmd("m 1600 35");
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd("dB");
	}

	iy = 3800;
	for (i = 0; i < ntext; i++) 
	{
	    tk_snd_plot_cmd("c 5");
	    tk_snd_plot_cmd_1arg("m %d 500",iy);
	    tk_snd_plot_cmd("t 5 1");
	    tk_snd_plot_cmd(text[i]);
	    iy += 200;
	}
	print_time(200, 4400);
    }

    tk_start();
    exit(0);
    /*NOTREACHED*/
}

void
erase()
{
}
