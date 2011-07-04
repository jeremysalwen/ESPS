/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|         "Copyright (c) 1988, 1989, 1990 Entropic Speech, Inc.         |
|                         All rights reserved."		                |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: image.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)image.c	1.13 8/31/95 ESI";
#endif
#define VERSION "8/31/95"
#define DATE "1.13"

#include <stdio.h>
#include <ctype.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include "image.h"

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("image [-{ef} range]... [-l range][-o][-{prs} range][-t text]\n [-x debug_level][-A algorithm][-B scale][-C colormap][-D][-F function][-G range]\n [-L{prs}][-M mag][-P param][-S width[:height]][-T device][-V text] [-W geometry] file") \
 ;
/* Delete ; when esps.h fixed */

#define DEF_FIELD "re_spec_val"

#ifdef XWIN
#define DEV_DEFAULT DEV_X11
#else
#define DEV_DEFAULT DEV_MCD
#endif

#define DATA_CHUNK 500

double	log10(), log(), exp(), sqrt(), pow();

void	lrange_switch();
void	frange_switch();
long	*grange_switch();
long    get_fea_siz();
long	get_fea_element();
long	get_rec_len();
char    *savestring();
int	get_gen_recd();
char	*get_cmd_line();
char	*arr_alloc();

void	addstr();
long	*fld_range_switch();

int	depth();
int	(*read_cmap())[3];
void	get_default_size();
void	get_scale();
void	set_margins();
long	read_and_count();
long	*fea_range();
long	read_with_tag();
void	put_with_tag();
char	*allo_buf();
void	reallo_data();
void	interp(), pick_nearest();
void	gain_adj();
void	dev_init(), dev_fin();
void	dot_init(), dot_row(), dot_fin();
void	text();
void	axes_and_titles();
void	pr_larray();

extern void
	(*dev_plotline)();
extern void
	(*dev_initbits)();

char	*ProgName = "image";
char	*Version = VERSION;
char	*Date = DATE;

int	debug_level = 0;
long	lmarg,
	rmarg,
	tmarg,
	bmarg;	/* left, right, top, bottom margins */
int	scale;
long    width, height;	/* dimensions of pixel array */
long    nrows, ncols;	/* width, height in default orientation;
			   height, width in rotated orientation */

int	oflag = NO;	/* -o option specified? */
int	dev;		/* output device code */
static char *devices[] = DEV_NAMES;
int	alg;		/* halftone algorithm */
static char *algorithms[] = HT_NAMES;
int	fun;		/* preprocessing function */
static char *functions[] = FN_NAMES;

int	Dflag = NO;	/* -D option specified? */

int 	Mflag = NO;	/* -M option specified? */
int	mag = 1;	/* pixel magnification */

int	gray_bits;	/* number of bits gray-scale resolution required */
int	Bflag = NO;	/* B option specified? */

int	(*cmap)[3];	/* array of RGB triples */
int	cmap_len;	/* number of colormap entries */

int	Lflag = NO;	/* -L option specified? */
char	lbl_units;	/* mark axis in samples, records, or seconds? */
long    startrec;	/* first record to plot */
long    endrec;		/* last record to plot */
double  src_sf;		/* sampling freq. of source sampled data --
			 * used in relating tags to times.
			 */
int	src_sf_def = NO;/* have valid value for src_sf? */
double  start_time;	/* starting time of file */
double  record_freq;	/* record freq of source file --
			 * used in relating record numbers to times.
			 */
int	rec_fr_def = NO;/* have valid value for record_freq? */
long    *elem_array;	/* numbers of record elements to display */


int	Gflag = NO;
double	gainlolim, gainhilim;
int	pwr_flag = NO;	/* convert FEA_SPEC data to power */
int	spectype;

float   **data;		/* data array */
long    data_size;	/* number of data array rows allocated */
long    data_dim[2];	/* dimensions of chunk of data array */
double  *x, *y;		/* row and col coordinates of data array */
double	xmin, xmax;	/* coords of ends of axis */
long    nrecs;		/* number of records to plot */
long    nelem;		/* number of elements to plot in each record */
double  zmin, zmax;	/* limits of data range for full gray-scale */

char    *h_ttl_text;	/* text string for labeling horizontal axis */
char    *v_ttl_text;	/* text string for labeling vertical axis */

char    *geometry;      /* holds geomtry string from command line(imdevx11.c)*/
int     Wflag = 0;      /* W option flag (imdevx11.c)*/

int	Argc;		/* make available in imdevx11.c */
char	**Argv;		/* make available in imdevx11.c */

/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int
	    optind;		/* for use of getopt() */
    extern char
	    *optarg;		/* for use of getopt() */
    int	    ch;			/* command-line option letter */

    int	    eflag = NO;		/* -e option specified? */
    char    *erange = NULL;	/* element range ("grange" form) */
    char    **fea_fields;	/* field specification given with -e */
    int	    nfld = 0;		/* number of named feature fields */

    int	    lflag = NO;		/* -l option specified? */
    char    *lrange;		/* data range for full gray-scale */

    int	    pflag = NO;		/* -p option specified? */
    char    *prange;		/* range of tags to plot */
    long    starttag;		/* first tag to plot */
    long    endtag;		/* last tag to plot */
    
    int	    rflag = NO;		/* -r option specified? */
    char    *rrange;		/* range of records to plot */

    int	    sflag = NO;		/* -s option specified? */
    char    *srange;		/* range of times to plot */
    double  starttim;		/* first time to plot */
    double  endtim;		/* last time to plot */
    
    int	    tflag = NO;		/* -t option specified? */

    int	    Aflag = NO;		/* -A option specified? */

    int	    Cflag = NO;		/* -C option specified? */
    char    *cmap_filename;	/* name of colormap file */

    int	    Fflag = NO;		/* -F option specified? */

    char    *param_name = NULL;
				/* parameter file name */

    int	    Sflag = NO;		/* -S option specified? */
    char    *Srange;		/* arguments to -S option */

    int	    Tflag = NO;		/* -T option specified? */

    int	    Vflag = NO;		/* -V option specified? */

    char    *iname;		/* input file name */
    FILE    *ifile;		/* input stream */
    struct header
	    *ihd;		/* input file header */

    double  datamin, datamax;	/* max and min data values */
    long    i, j;		/* data indices */

/* make argc and argv available in imdevx11.c */

    Argc = argc;
    Argv = argv;

/* Parse command-line arguments */

    fea_fields = (char **) malloc(sizeof(char *));
    spsassert(fea_fields, "can't allocate space for field name list");
    fea_fields[0] = NULL;

    while ((ch = getopt(argc, argv, "e:f:l:op:r:s:t:x:A:B:C:DF:G:L:M:P:S:T:V:W:")) != EOF)
	switch (ch)
	{
	case 'e':
	case 'f':
	    eflag = YES;
	    if (isalpha(optarg[0]))
	    {
		nfld++;
		addstr(optarg, &fea_fields);
	    }
	    else erange = optarg;
	    break;
	case 'l':
	    lflag = YES;
	    lrange = optarg;
	    break;
	case 'o':
	    oflag = YES;
	    break;
	case 'p':
	    pflag = YES;
	    prange = optarg;
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 's':
	    sflag = YES;
	    srange = optarg;
	    break;
	case 't':
	    tflag = YES;
	    h_ttl_text = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'A':
	    Aflag = YES;
	    alg = lin_search2(algorithms, optarg);
	    break;
	case 'B':
	    Bflag = YES;
	    scale = atoi(optarg);
	    break;
	case 'C':
	    Cflag = YES;
	    cmap_filename = optarg;
	    break;
	case 'D':
	    Dflag = YES;
	    break;
	case 'F':
	    Fflag = YES;
	    fun = lin_search2(functions, optarg);
	    break;
	case 'G':
	    Gflag = YES;
	    frange_switch(optarg, &gainlolim, &gainhilim);
	    break;
	case 'L':
	    Lflag = YES;
	    lbl_units = *optarg;
	    break;
	case 'M':
	    Mflag = YES;
	    mag = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'S':
	    Sflag = YES;
	    Srange = optarg;
	    break;
	case 'T':
	    Tflag = YES;
    	    dev = lin_search2(devices, optarg);
	    break;
	case 'V':
	    Vflag = YES;
	    v_ttl_text = optarg;
	    break;
        case 'W':
	    Wflag++;
	    geometry = optarg;
	    break;
	default:
	    SYNTAX
	    break;
	}

/* Process file name and open file. */

    if (argc - optind > 1)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 1)
    {
	Fprintf(stderr,
	    "%s: no input file name specified.\n", ProgName);
	SYNTAX
    }

    iname = eopen(ProgName, argv[optind], "r", NONE, NONE, &ihd, &ifile);

    if (debug_level)
	Fprintf(stderr, "Input file: %s\n", iname);

/* Consistency checks */

    REQUIRE(pflag + rflag + sflag <= 1,
		"only one of -p, -r, and -s may be used at one time");
    REQUIRE(nfld == 0 || ihd->common.type == FT_FEA,
		"named fields allowed only with FEA files");
    REQUIRE(nfld == 0 || erange == NULL,
		"if any -e option argument has a field name, all must.");

/* Get parameter values. */

    (void) read_params(param_name, SC_NOCOMMON, (char *) NULL);

    if (!Fflag)
    {
	if (symtype("function") != ST_UNDEF)
	{
	    fun = lin_search2(functions, getsym_s("function"));
	    Fflag = YES;
	}
	else
	    fun = NONE;
    }

    switch (fun)
    {
    case NONE:
    case FN_LOG:
    case FN_EXP:
    case FN_SQ:
    case FN_SQRT:
	break;
    default:
	Fprintf(stderr, "%s: unrecognized function name.\n",
		ProgName);
	exit(1);
	break;
    }

    if (eflag && nfld == 0)
    {
	REQUIRE(!is_file_complex(ihd),
	"-e[grange] not supported with complex files\n - real FEA fields may be specified by name\n");
	elem_array = grange_switch(erange, &nelem);
    }
    else
    if (!eflag && symtype("elements") != ST_UNDEF)
    {
	int	*elems = malloc_i((unsigned) get_rec_len(ihd));

	spsassert(elems,
	    "can't allocate space for element array from symbol table");
	nelem = getsym_ia("elements", elems, (int) get_rec_len(ihd));
	spsassert(nelem,
	    "can't get \"elements\" parameter--may be too long");
	REQUIRE(!is_file_complex(ihd),
	"\"elements\" parameter not supported with complex files\n - real FEA fields may be specified by name\n");
	elem_array = malloc_l((unsigned) nelem);
	spsassert(elem_array,
	    "can't allocate space for array of element numbers");
	for (i = 0; i < nelem; i++)
	    elem_array[i] = elems[i];
	free((char *) elems);
    }
    else
    {
	switch (ihd->common.type)
	{
	case FT_FEA:
	    if (ihd->hd.fea->fea_type == FEA_SPEC && !eflag)
	    {
		pwr_flag = YES;

		REQUIRE(genhd_type("spec_type", (int *) NULL, ihd) == CODED,
			"header item \"spec_type\" undefined or not CODED");
		spectype = *get_genhd_s("spec_type", ihd);
		switch (spectype)
		{
		case SPTYP_PWR:
		case SPTYP_DB:
		case SPTYP_REAL:
		case SPTYP_CPLX:
		    break;
		default:
		    Fprintf(stderr, "%s: unknown spectral type\n", ProgName);
		    exit(1);
		    break;
		}

		if (!Fflag) fun = FN_LOG;

		REQUIRE(genhd_type("num_freqs", (int *) NULL, ihd) == LONG,
			"header item \"num_freqs\" undefined or not LONG");
		nelem = *get_genhd_l("num_freqs", ihd);

	    }
	    else
	    {
		if (!eflag)
		{
		    nfld = 1;
		    addstr(DEF_FIELD, &fea_fields);
		}
		elem_array = fea_range(fea_fields, ihd, &nelem);
		break;
	    }
	    break;
	default:
	    Fprintf(stderr,
		"%s: use -e[grange] option unless file is FEA file.\n",
		ProgName);
	    exit(1);
	    break;
	}
    }

    if (!pwr_flag && debug_level > 1)
    {
	fprintf(stderr, "%ld elements:\n", nelem);
	pr_larray("elements", nelem, elem_array);
    }

    if (!Lflag && symtype("label_units") != ST_UNDEF)
    {
	Lflag = YES;
	lbl_units = getsym_c("label_units");
    }
    if (Lflag)
	switch (lbl_units)
	{
	case 'p':
	case 'r':
	case 's':
	    break;
	default:
	    Fprintf(stderr,
		    "%s: axis label units must be given as p, r, or s.\n",
		    ProgName);
	    exit(1);
	}
    else if (pflag) lbl_units = 'p';
    else if (rflag) lbl_units = 'r';
    else if (sflag) lbl_units = 's';
    else lbl_units = 'r';

    if (pflag || lbl_units == 'p')
	REQUIRE(ihd->common.tag,
		"-p option or axis labeling in points requires tagged file");
	
    if (sflag || lbl_units == 's')
    {
	if (ihd->common.tag
	    && ((src_sf = get_genhd_val("src_sf", ihd, 0.0)) != 0.0
		|| (ihd->common.type == FT_FEA
		    && ihd->hd.fea->fea_type == FEA_SPEC
		    && (src_sf = get_genhd_val("sf", ihd, 0.0)) != 0.0)))
	{
	    start_time = 0.0;
	    src_sf_def = YES;
	}
	else if ((record_freq = get_genhd_val("record_freq", ihd, 0.0)) != 0.0)
	{
	    start_time = get_genhd_val("start_time", ihd, 0.0);
	    rec_fr_def = YES;
	}

	REQUIRE(src_sf_def || rec_fr_def,
		"-s option or axis labeling in seconds\n\trequires nonzero sampling or record frequency.");
    }

    if (pflag)
    {
	starttag = 1;
	endtag = LONG_MAX;
	lrange_switch(prange, &starttag, &endtag, 0);
	REQUIRE(starttag >= 1, "initial tag not positive");
	REQUIRE(starttag < endtag, "initial tag not less than final tag");

	if (debug_level > 1)
	    Fprintf(stderr, "Initial and final tags from -p: %d, %d\n",
		    starttag, endtag);
    }
    else
    if (rflag)
    {
    	startrec = 1;
	endrec = LONG_MAX;
	lrange_switch(rrange, &startrec, &endrec, 0);
	REQUIRE(startrec >= 1, "initial record number not positive");
	REQUIRE(startrec < endrec,
	    "initial record number not less than final record number");

	if (debug_level > 1)
	    Fprintf(stderr,
		"inital and final record numbers from -r: %d, %d\n",
		 startrec, endrec);
    }
    else
    if (sflag)
    {
	starttim = start_time;
	endtim =
	    src_sf_def ? LONG_MAX/src_sf
		: start_time + LONG_MAX/record_freq;
	frange_switch(srange, &starttim, &endtim);
	REQUIRE(starttim >= start_time, "starting time before start of file");
	REQUIRE(starttim < endtim,
		  "starting time not less than ending time");

	if (src_sf_def)
	{
	    starttag = 1 + LROUND(starttim * src_sf);
	    endtag =
		(1.5 + endtim * src_sf >= LONG_MAX)
		? LONG_MAX
		: 1 + LROUND(endtim * src_sf);
	}
	else
	{
	    startrec = 1 + LROUND((starttim - start_time) * record_freq);
	    endrec =
		(1.5 + (endtim - start_time) * record_freq >= LONG_MAX)
		? LONG_MAX
		: 1 + LROUND((endtim - start_time) * record_freq);
	}

	if (debug_level > 1)
	{
	    Fprintf(stderr,
		    "initial and final times from -s: %f, %f\n",
		    starttim, endtim);
	    if (src_sf_def)
		Fprintf(stderr,
			"initial and final tags from times: %d, %d\n",
			starttag, endtag);
	    else
		Fprintf(stderr,
			"initial and final records from times: %d, %d\n",
			startrec, endrec);
	}
    }
    else
    {
	rflag = YES;
	startrec =
	    (symtype("startrec") != ST_UNDEF)
	    ? getsym_i("startrec")
	    : 1;
	REQUIRE(startrec >= 1, "starting record number not positive");
	endrec =
	    (symtype("nrecs") != ST_UNDEF && getsym_i("nrecs") != 0)
	    ? startrec - 1 + getsym_i("nrecs")
	    : LONG_MAX;

	if (debug_level > 1)
	    Fprintf(stderr,
		"initial and final records from param or default: %d, %d\n",
		startrec, endrec);
    }

    if (!tflag)
	h_ttl_text =
	    (symtype("x_text") != ST_UNDEF)
	    ? getsym_s("x_text")
	    : "";

    if (!Vflag)
	v_ttl_text =
	    (symtype("y_text") != ST_UNDEF)
	    ? getsym_s("y_text")
	    : "";

    if (!Tflag)
	dev =
	    (symtype("device") != ST_UNDEF)
	    ? lin_search2(devices, getsym_s("device"))
	    : DEV_DEFAULT;


    if (dev == DEV_TYPE) oflag = YES;
    if (!oflag && symtype("orientation") != ST_UNDEF)
    {
	char	*or = getsym_s("orientation");

	if (strcmp(or, "rotated") == 0) oflag = YES;
	else REQUIRE(strcmp(or, "default") == 0,
		"unrecognized value for \"orientation\" parameter");
    }

    if (!Aflag)
	alg =
	    (symtype("algorithm") != ST_UNDEF)
	    ? lin_search2(algorithms, getsym_s("algorithm"))
	    : (ihd->common.type == FT_FEA
		    && ihd->hd.fea->fea_type == FEA_SPEC)
	    ? ( (depth() < 4)
		? HT_FS2
		: HT_16OD1_2 )
	    : (depth() < 4)
	    ? HT_FS
	    : HT_16OD1;

    switch (alg)
    {
    case HT_16LVL:
    case HT_16OD1:
    case HT_16OD1_2:
    case HT_16OD1_3:
	gray_bits = 4;
	break;
    case HT_OD1:
    case HT_OD2:
    case HT_OD3:
    case HT_OD4:
    case HT_FS:
    case HT_FS2:
    default:
	gray_bits = 1;
	break;
    }

    cmap_len = ROUND(pow(2.0, (double) gray_bits));
    if (!Cflag)
	cmap_filename =
	    (symtype("colormap_file") == ST_UNDEF) ? NULL
	    : getsym_s("colormap_file");
    cmap = read_cmap(cmap_filename, cmap_len);


    if (!Mflag)
    {
	if (symtype("magnification") != ST_UNDEF)
	    mag = getsym_i("magnification");
	else
	    mag = 1;
    }
    if (mag < 1)
    {
	Fprintf(stderr, "%s: magnification must be positive.\n",
		ProgName);
	exit(1);
    }

    if (!Bflag)
    {
	if (symtype("b_scale") != ST_UNDEF)
	{
	    scale = getsym_i("b_scale");
	    Bflag = YES;
	}
	else
	    scale = 0;
    }

    get_scale(&scale);
    set_margins(Bflag, scale, &lmarg, &rmarg, &tmarg, &bmarg);

    get_default_size(&width, &height);
    if (Sflag)
	lrange_switch(Srange, &width, &height, 0);
    else
    {
	if (symtype("width") != ST_UNDEF)
	    width = getsym_i("width");
	if (symtype("height") != ST_UNDEF)
	    height = getsym_i("height");
    }

    if (oflag)
    {
	nrows = height;
	ncols = width;
    }
    else
    {
	nrows = width;
	ncols = height;
    }

    data_dim[0] = data_size = DATA_CHUNK;
    data_dim[1] = nelem;
    data = (float **) arr_alloc(2, data_dim, FLOAT, 0);

    x = (double *) arr_alloc(1, &data_dim[0], DOUBLE, 0);
    y = (double *) arr_alloc(1, &nelem, DOUBLE, 0);
    
    if (pwr_flag)
    {
	int 	freq_format;
	double  sf;		/* sampling frequency -- used in determining
				 * band limits & scale on freq. axis
				 */
	float   *freqs;		/* frequencies from header for case ARB_FIXED
				 */

	REQUIRE(genhd_type("freq_format", (int *) NULL, ihd) == CODED,
		"header item \"freq_format\" undefined or not CODED");
	freq_format = *get_genhd_s("freq_format", ihd);

	switch (freq_format)
	{
	case SPFMT_SYM_CTR:
	case SPFMT_SYM_EDGE:
	case SPFMT_ASYM_CTR:
	case SPFMT_ASYM_EDGE:
	    REQUIRE(genhd_type("sf", (int *) NULL, ihd) == FLOAT,
		    "header item \"sf\" undefined or not FLOAT");
	    sf = *get_genhd_f("sf", ihd);
	    break;
	case SPFMT_ARB_VAR:
	    Fprintf(stderr, "%s: freq format ARB_VAR not yet supported.\n",
		ProgName);
	    exit(1);
	    break;
	case SPFMT_ARB_FIXED:
	    REQUIRE(genhd_type("freqs", (int *) NULL, ihd) == FLOAT,
		    "header item \"freqs\" undefined or not FLOAT");
	    freqs = get_genhd_f("freqs", ihd);
	    break;
	default:
	    Fprintf(stderr, "%s: unrecognized freq format %d.\n",
		ProgName, (int) freq_format);
	    exit(1);
	    break;
	}

	switch (freq_format)
	{
	case SPFMT_SYM_CTR:
	    for (j = 0; j < nelem; j++)
		y[j] = (j + 0.5) * sf / (2 * nelem);
	    break;
	case SPFMT_SYM_EDGE:
	    for (j = 0; j < nelem; j++)
		y[j] = j * sf / (2 * (nelem - 1));
	    break;
	case SPFMT_ASYM_CTR:
	    for (j = 0; j < nelem; j++)
		y[j] = -0.5 * sf + (j + 0.5) * sf / nelem;
	    break;
	case SPFMT_ASYM_EDGE:
	    for (j = 0; j < nelem; j++)
		y[j] = -0.5 * sf + j * sf / (nelem - 1);
	    break;
	case SPFMT_ARB_FIXED:
	    for (j = 0; j < nelem; j++)
		y[j] = freqs[j];
	}
    }
    else
    {
	for (j = 0; j < nelem; j++)
	    y[j] = j;
    }

    nrecs = read_and_count(ifile, ihd, rflag || sflag && rec_fr_def,
			    &startrec, &endrec, &starttag, &endtag);

    if (debug_level > 1)
	Fprintf(stderr, "%ld %s\n%s: %ld, %ld\n%s %ld, %ld\n",
		nrecs, "returned from read_and_count",
		"initial and final records", startrec, endrec,
		"initial and final tags", starttag, endtag);

    REQUIRE(nrecs > 1,
	"span of records in file doesn't overlap given range");

    switch (lbl_units)
    {
    case 'p':
	xmin = (pflag || sflag && src_sf_def) ? starttag : x[0];
	xmax = (pflag || sflag && src_sf_def) ? endtag : x[nrecs - 1];
	break;
    case 'r':
	xmin = x[0];
	xmax = x[nrecs - 1];
	break;
    case 's':
	xmin = (pflag || sflag && src_sf_def) ? (starttag - 1)/src_sf : x[0];
	xmax = (pflag || sflag && src_sf_def) ? (endtag - 1) / src_sf : x[nrecs - 1];
    }

    if (debug_level > 1)
	Fprintf(stderr, "xmin and xmax %g, %g\n", xmin, xmax);

    datamin = FLT_MAX;
    datamax = -FLT_MAX;
    for (i = 0; i < nrecs; i++)
	for (j = 0; j < nelem; j++)
	{
	    if (data[i][j] < datamin) datamin = data[i][j];
	    if (data[i][j] > datamax) datamax = data[i][j];
	}

    if (pwr_flag)
    {
	switch (fun)
	{
	case NONE:
	    zmax = datamax*0.199526;
	    zmin = zmax/10000;
	    break;
	case FN_LOG:
	    zmax = datamax - 7.0;
	    zmin = zmax - 40.0;
	    break;
	case FN_EXP:
	    zmax = pow(datamax, 0.199526);
	    zmin = pow(zmax, 1.0e-4);
	    break;
	case FN_SQ:
	    zmax = datamax*0.0398107;
	    zmin = zmax/10.e8;
	    break;
	case FN_SQRT:
	    zmax = datamax*0.446684;
	    zmin = zmax/100.0;
	    break;
	}

	if (debug_level)
	    Fprintf(stderr, "Limits computed from max data value: %g, %g.\n",
		zmin, zmax);
    }
    else
    {
	zmin = datamin;
	zmax = datamax;

	if (debug_level)
	    Fprintf(stderr, "Extreme data values: %g, %g.\n", zmin, zmax);
    }

    if (lflag)
	frange_switch(lrange, &zmin, &zmax);
    else
    {
	if (symtype("minlevel") != ST_UNDEF) zmin = getsym_d("minlevel");
	if (symtype("maxlevel") != ST_UNDEF) zmax = getsym_d("maxlevel");
    }

    if (!Gflag)
    {
	if (symtype("gain_low_lim") != ST_UNDEF
		|| symtype("gain_high_lim") != ST_UNDEF)
	{
	    REQUIRE(symtype("gain_low_lim") != ST_UNDEF
			&& symtype("gain_high_lim") != ST_UNDEF,
		"just one of \"gain_low_lim\" and \"gain_high_lim\" defined")
	    Gflag = YES;
	    gainlolim = getsym_d("gain_low_lim");
	    gainhilim = getsym_d("gain_high_lim");
	    
	}
    }

    dev_init();

    exit(0);
    /*NOTREACHED*/
}


long *
fea_range(fields, hd, n_ele)
    char	    **fields;
    struct header   *hd;
    long	    *n_ele;
{
    int	    i, j;
    char    *field_name;	/* string to hold field name */
    long    *item_array;	/* array of items given in gen_range */
    long    n_item;		/* number of items given in gen_range */
    long    *evec;		/* vector of element numbers */
    long    total_len = 0;	/* total length of fields */
    long    indx = 0;		/* index into evec */

    evec = malloc_l((unsigned) 1);
    for (i = 0; fields[i] != NULL; i++)
    {
	item_array = fld_range_switch(fields[i], &field_name, &n_item, hd);
	if (!item_array)
	{
	    Fprintf(stderr,
		    "%s: field \"%s\" not defined in feature file header\n",
		    ProgName, field_name);
	    exit(1);
	}

	REQUIRE(!is_field_complex(hd, field_name),
		"complex fields not yet supported");

	total_len += n_item;

	evec = (long *) realloc((char *) evec,
				(unsigned) total_len * sizeof (long));
	spsassert(evec,
		"can't reallocate space for vector of element numbers");

	for (j = 0; j < n_item; j++)
	{
	    evec[indx] = get_fea_element(field_name, hd) + item_array[j];
	    indx++;
	}

	free((char *) item_array);

    }	/* end for (i ...) */

    *n_ele = total_len;
    return evec;
}

long
read_and_count(file, hd, rfl, startrec, endrec, starttag, endtag)
    FILE	    *file;
    struct header   *hd;
    int		    rfl;
    long	    *startrec, *endrec, *starttag, *endtag;
{
    long    	nrec;
    char	*buf0 = allo_buf(hd),
    	    	*buf1 = allo_buf(hd);

    spsassert(buf0 && buf1,
	      "can't allocate space for buffers to make temp file");

    if (debug_level > 1)
	Fprintf(stderr, "%s\n%s: %ld, %ld\n%s %ld, %ld\n",
		"Beginning of read_and_count",
		"initial and final records", *startrec, *endrec,
		"initial and final tags", *starttag, *endtag);

    if (rfl)
    {
	long	tag0;

	fea_skiprec(file, *startrec - 1, hd);
	for (nrec = 0;
	     nrec < *endrec - *startrec + 1
	     && (tag0 = read_with_tag(buf0, hd, file)) != LONG_MAX;
	     (void) put_with_tag(tag0, buf0, nrec++)
	     ) { }

	*endrec = nrec + *startrec - 1;
    }
    else
    {
	long    tag0, tag1;

	tag0 = read_with_tag(buf0, hd, file);
	nrec = 1;		/* record num in tag0, buf0 */
	if (tag0 > *starttag)
	{
	    REQUIRE(tag0 < *endtag, 
		    "first record doesn't precede end of range");
	    *starttag = tag0;
	}

	tag1 = read_with_tag(buf1, hd, file);
	while (tag1 <= *starttag)
	{
	    char	*tmp = buf0;

	    buf0 = buf1;	tag0 = tag1;
	    nrec++;
	    buf1 = tmp;
	    tag1 = read_with_tag(buf1, hd, file);
	}

	*startrec = nrec;

	nrec = 0;		/* num of records in data array */
	put_with_tag(tag0, buf0, nrec++);

	while (tag1 < *endtag)
	{
	    put_with_tag(tag1, buf1, nrec++);
	    tag0 = tag1;
	    tag1 = read_with_tag(buf1, hd, file);
	}

	if (tag1 == LONG_MAX)
	{
	    REQUIRE(tag0 > *starttag,
		    "last record doesn't follow beginning of range");
	    *endtag = tag0;
	}
	else
	    put_with_tag(tag1, buf1, nrec++);

	*endrec = *startrec - 1 + nrec;
    }

    return nrec;
}

char *
allo_buf(hd)
    struct header   *hd;
{
    char    *buf;

    if (pwr_flag)
    {
	buf = (char *) allo_feaspec_rec(hd, FLOAT);
	spsassert(buf, "can't allocate space for input FEA_SPEC record");
    }
    else
    {
	buf = (char *) malloc_d((unsigned) get_rec_len(hd));
	spsassert(buf, "can't allocate space for input data vector");
    }
    return buf;
}

long
read_with_tag(buf, hd, strm)
    char	    *buf;
    struct header   *hd;
    FILE	    *strm;
{
    int	    nread;
    long    tag;

    if (pwr_flag)
    {
	struct feaspec	*specrec = (struct feaspec *) buf;

	nread = get_feaspec_rec(specrec, hd, strm);
	if (nread != EOF) tag = (specrec->tag) ? *specrec->tag : 0;
    }
    else
    {
	double	*genrec = (double *) buf;

	nread = get_gen_recd(genrec, &tag, hd, strm);
    }
    if (nread == EOF) tag = LONG_MAX;
    return tag;
}

void
put_with_tag(tag, buf, i)
    long    tag;
    char    *buf;
    long    i;
{
    long    j;

    if (i >= data_size) reallo_data();

    if (pwr_flag)
    {
	struct feaspec	*specrec = (struct feaspec *) buf;

	for (j = 0; j < nelem; j++)
	{
	    double	rsv, isv;

	    rsv = specrec->re_spec_val[j];

	    if (spectype == SPTYP_DB && fun == FN_LOG
		    || spectype == SPTYP_REAL && fun == FN_SQRT)
		data[i][j] = rsv;
	    else
	    {
		switch (spectype)
		{
		case SPTYP_PWR:
		    break;
		case SPTYP_DB:
		    rsv = pow(10.0,rsv/10.0);
		    break;
		case SPTYP_REAL:
		    rsv = rsv*rsv;
		    break;
		case SPTYP_CPLX:
		    isv = specrec->im_spec_val[j];
		    rsv = rsv*rsv + isv*isv;
		    break;
		}

		switch (fun)
		{
		case NONE:
		    data[i][j] = rsv;
		    break;
		case FN_LOG:
		    data[i][j] = 10.0 * log10(rsv);
		    break;
		case FN_EXP:
		    data[i][j] = exp(rsv);
		    break;
		case FN_SQ:
		    data[i][j] = rsv*rsv;
		    break;
		case FN_SQRT:
		    data[i][j] = sqrt(rsv);
		    break;
		}
	    }
	}
    }
    else
    {
	double	*genrec = (double *) buf;

	for (j = 0; j < nelem; j++)
	    switch (fun)
	    {
	    case NONE:
		data[i][j] = genrec[- 1 + elem_array[j]];
		break;
	    case FN_LOG:
		data[i][j] = log(genrec[- 1 + elem_array[j]]);
		break;
	    case FN_EXP:
		data[i][j] = exp(genrec[- 1 + elem_array[j]]);
		break;
	    case FN_SQ:
		data[i][j] = genrec[- 1 + elem_array[j]]
				* genrec[- 1 + elem_array[j]];
		break;
	    case FN_SQRT:
		data[i][j] = sqrt(genrec[- 1 + elem_array[j]]);
		break;
	    default:
		break;
	    }
    }

    switch (lbl_units)
    {
    case 'p':
	x[i] = tag;
	break;
    case 'r':
	x[i] = i + startrec;
	break;
    case 's':
	x[i] = src_sf_def ? (tag - 1)/src_sf
	    : start_time + (startrec + i - 1)/record_freq;
	break;
    }
}

void
reallo_data()
{
    float   **new_chunk;
    long    i, old_size;

    old_size = data_size;
    data_size += DATA_CHUNK;
    x = (double *) realloc((char *) x,
			   (unsigned) data_size * sizeof(double));
    REQUIRE(x, "can't reallocate space for x coordinate values");

    data = (float **) realloc((char *) data,
			      (unsigned) data_size * sizeof(float *));
    new_chunk = (float **) arr_alloc(2, data_dim, FLOAT, 0);
    REQUIRE(data && new_chunk, "can't extend storage for data array");
    for (i = 0; i < DATA_CHUNK; i++)
	data[old_size + i] = new_chunk[i];
    free((char *) new_chunk);
}

void
plotimage()
{
    int		    u, v;	/* pixel indices */
    long	    i;
    double	    delta_x = xmax - xmin;
    double	    delta_z = zmax - zmin;
    static int	    alloc_size = 0;
    static double   *oldrow, *midrow, *newrow;
				/* "rows" of grayscale densities */
    if (alloc_size == 0)
    {
	oldrow = (double *) malloc_d((unsigned) ncols);
	midrow = (double *) malloc_d((unsigned) ncols);
	newrow = (double *) malloc_d((unsigned) ncols);
	spsassert(oldrow && midrow && newrow,
	    "can't allocate space for 3 rows of interpolated values");
	alloc_size = ncols;

	if (debug_level >= 2)
	    Fprintf(stderr,
		"Allocated space for 3 rows of %ld interpolated values.\n",
		ncols);
    }
    else
    if (alloc_size < ncols)
    {
	oldrow = (double *) realloc((char *) oldrow,
				    (unsigned) ncols * sizeof(double));
	midrow = (double *) realloc((char *) midrow,
				    (unsigned) ncols * sizeof(double));
	newrow = (double *) realloc((char *) newrow,
				    (unsigned) ncols * sizeof(double));
	spsassert(oldrow && midrow && newrow,
	    "can't reallocate space for 3 rows of interpolated values");
	alloc_size = ncols;

	if (debug_level >= 2)
	    Fprintf(stderr,
		"Reallocated space for 3 rows of %ld interpolated values.\n",
		ncols);
    }

    if (debug_level > 1)
	Fprintf(stderr, "In plot_image xmin and xmax: %g, %g\n", xmin, xmax);

    if (Bflag) axes_and_titles();
    dot_init();
    (*dev_initbits)();

    if (Dflag)
    {
	u = 0;
	for (i = 0; i < nrecs; i++)
	{
	    pick_nearest(data[i], y, nelem, midrow, ncols);

	    if (Gflag)
	    {
		double	rowmax = -FLT_MAX;
		double	rowmin;
		
		for (v = 0; v < ncols; v++)
		    if (rowmax < midrow[v]) rowmax = midrow[v];

		gain_adj(pwr_flag, &rowmin, &rowmax);

		for (v = 0; v < ncols; v++)
		    midrow[v] = (midrow[v] - rowmin)/(rowmax - rowmin);
	    }
	    else
	    {
		for (v = 0; v < ncols; v++)
		    midrow[v] = (midrow[v] - zmin) / delta_z;
	    }
	    
	    for ( ;
		 u <= nrows - 1
		 && (i == nrecs - 1
		     || u < (nrows-1) * (0.5*(x[i]+x[i+1]) - xmin) / delta_x);
		 u++
		)
	    {
		dot_row(midrow);
	    }
	}
    }
    else
    {
	interp(data[0], y, nelem, newrow, ncols);
	u = 0;
	for (i = 1; i < nrecs; i++)
	{
	    double	*tmp;

	    tmp = oldrow; oldrow = newrow; newrow = tmp;

	    interp(data[i], y, nelem, newrow, ncols);

	    for ( ;
		 u <= nrows - 1
		 && (i == nrecs - 1
		     || u <= (nrows-1) * (x[i]-xmin) / delta_x);
		 u++
		)
	    {
		double  alpha =
		    (u*delta_x/(nrows-1.0) + xmin - x[i-1]) / (x[i] - x[i-1]);
		double  beta = 1.0 - alpha;
		double  gamma;

		if (Gflag)
		{
		    double	rowmax = -FLT_MAX;
		    double	rowmin;

		    for (v = 0; v < ncols; v++)
		    {
			midrow[v] =
			    alpha*newrow[v] + beta*oldrow[v];
			if (rowmax < midrow[v]) rowmax = midrow[v];
		    }

		    gain_adj(pwr_flag, &rowmin, &rowmax);

		    for (v = 0; v < ncols; v++)
			midrow[v] = (midrow[v] - rowmin)/(rowmax - rowmin);
		}
		else
		{
		    alpha /= delta_z;
		    beta /= delta_z;
		    gamma = zmin / delta_z;

		    for (v = 0; v < ncols; v++)
			midrow[v] =
			    alpha*newrow[v] + beta*oldrow[v] - gamma;
		}

		dot_row(midrow);
	    }
	}
    }

    dot_fin();
    dev_fin();
}

/*
 * Given values dat[j] of a function at points y[j] (j = 0, ..., (nelem-1)),
 * determine values row[v] (v = 0, ..., (ncols-1)) by linear interpolation
 * at ncols other points equispaced from y[0] to y[nelem-1].
 */

void
interp(dat, y, nelem, row, ncols)
    float   *dat;
    double  *y;
    long    nelem;
    double  *row;
    long    ncols;
{
    long    j;
    long    v;
    double  delta_y;

    delta_y = y[nelem - 1] - y[0];
    v = 0;
    for (j = 1; j < nelem; j++)
    {
	for ( ; v <= (ncols-1)*(y[j]-y[0])/delta_y; v++)
	{
	    double  beta =
		(v*delta_y/(ncols-1.0) + y[0] - y[j-1]) / (y[j] - y[j-1]);

	    row[v] = beta*dat[j] + (1.0 - beta)*dat[j-1];
	}
    }
    if (v < ncols)
	row[v] = dat[j];
}

/*
 * Like  interp  above, but the value at a point in the interval
 * (y[j-1], y[j]), instead of being linearly interpolated between dat[j-1]
 * and dat[j], is simply one or the other of those values, depending on
 * which end of the interval is closer.  The interpolating function,
 * instead of being linear on the interval, is constant on each half of
 * the interval with a jump at the center.
 */

void
pick_nearest(dat, y, nelem, row, ncols)
    float   *dat;
    double  *y;
    long    nelem;
    double  *row;
    long    ncols;
{
    long    j;
    long    v;
    double  delta_y;

    delta_y = y[nelem - 1] - y[0];
    v = 0;
    for (j = 1; j < nelem; j++)
	for ( ; v < (ncols-1) * (0.5*(y[j-1]+y[j]) - y[0]) / delta_y; v++)
	    row[v] = dat[j-1];
    for ( ; v < ncols; v++)
	row[v] = dat[nelem-1];
}

int (*
read_cmap(fname, len) )[3]
    char	*fname;
    int		len;
{
    char    	*grange;
    long    	*elements, num_elts;
    FILE	*file;
    int		(*map)[3];	/* colormap entries read from file */
    int		(*imap)[3];	/* interpolated colormap entries */
    int		num_lines;
    unsigned	alloc_size = 16;
    char	line[256];
    long	i, j;
    double  	alpha, ratio;

    fname = strtok(fname, "[");
    grange = strtok((char *) 0, "]");
    elements =  grange ? grange_switch(grange, &num_elts) : NULL;

    if (!fname) return NULL;

    file = fopen(fname, "r");
    if (!file)
    {
	Fprintf(stderr, "%s: can't open colormap file %s.\n", ProgName, fname);
	return NULL;
    }

    map = (int (*)[3]) malloc((unsigned) alloc_size * sizeof(int [3]));
    spsassert(map, "can't allocate space for colormap");

    for (num_lines = 0; fgets(line, 256, file); num_lines++)
    {
	if (num_lines == alloc_size)
	{
	    alloc_size *= 2;
	    map = (int (*)[3])
		  realloc((char *) map,
			  (unsigned) alloc_size * sizeof(int [3]));
	    spsassert(map, "can't reallocate space for colormap");
	}
	sscanf(line, "%d%d%d",
	       &map[num_lines][0], &map[num_lines][1], &map[num_lines][2]);
    }

    if (elements)
    {
	int 	(*submap)[3];
	long	e;

	submap = (int (*)[3]) malloc((unsigned) num_elts * sizeof(int[3]));
	spsassert(submap, "can't allocate space for subset of colormap");

	for (i = 0; i < num_elts && (e = elements[i]) < num_lines; i++)
	{
	    submap[i][0] = map[e][0];
	    submap[i][1] = map[e][1];
	    submap[i][2] = map[e][2];
	}
	if (i == num_elts)
 	    num_lines = num_elts;
	else
	{
	    Fprintf(stderr, "%s: no line %ld in colormap file.\n",
		    ProgName, e);
	    free((char *) submap);
	    submap = NULL;
	    num_lines = 0;
	}

	free((char *) map);
	map = submap;
	free((char *) elements);
    }

    if (debug_level >= 2)
    {
	Fprintf(stderr, "%d colormap entries\n", num_lines);
	for (i = 0; i < num_lines; i++)
	    Fprintf(stderr, "%ld:\t%d %d %d\n",
		    i, map[i][0], map[i][1], map[i][2]);
    }

    if (num_lines < 2)
    {
	if (map) free((char *) map);
	return NULL;
    }

    if (num_lines == len) return map;

    imap = (int (*)[3]) malloc((unsigned) len * sizeof(int [3]));
    spsassert(map, "can't allocate space to interpolate colormap");

    ratio = (double) (num_lines - 1) / (len - 1);

    for (i = 0; i < len; i++)
    {
	alpha = i * ratio;
	j = alpha;
	if (j == num_lines - 1) j--;
	alpha -= j;

	imap[i][0] = 0.5 + alpha * map[j+1][0] + (1.0 - alpha) * map[j][0];
	imap[i][1] = 0.5 + alpha * map[j+1][1] + (1.0 - alpha) * map[j][1];
	imap[i][2] = 0.5 + alpha * map[j+1][2] + (1.0 - alpha) * map[j][2];
    }

    return imap;
}

void
gain_adj(flag, rowmin, rowmax)
    int	    flag;
    double  *rowmin, *rowmax;
{
    if (flag)
    {
	switch (fun)
	{
	case NONE:
	    *rowmin = *rowmax * pow(10.0, gainlolim/10.0);
	    *rowmax = *rowmax * pow(10.0, gainhilim/10.0);
	    if (*rowmax > zmax)
	    {
		*rowmin *= zmax / *rowmax;
		*rowmax = zmax;
	    }
	    if (*rowmin < zmin)
	    {
		*rowmax *= zmin / *rowmin;
		*rowmin = zmin;
	    }
	    break;
	case FN_LOG:
	    *rowmin = *rowmax + gainlolim;
	    *rowmax = *rowmax + gainhilim;
	    if (*rowmax > zmax)
	    {
	    	*rowmin += zmax - *rowmax;
		*rowmax = zmax;
	    }
	    if (*rowmin < zmin)
	    {
		*rowmax += zmin - *rowmin;
		*rowmin = zmin;
	    }
	    break;
	case FN_EXP:
	    *rowmin = pow(*rowmax, pow(10.0, gainlolim/10.0));
	    *rowmax = pow(*rowmax, pow(10.0, gainhilim/10.0));
	    if (*rowmax > zmax)
	    {
		*rowmin = pow(zmax, pow(10.0, gainlolim/10.0
				       - gainhilim/10.0 ));
		*rowmax = zmax;
	    }
	    if (*rowmin < zmin)
	    {
		*rowmax = pow(zmin, pow(10.0, gainhilim/10.0
				       - gainlolim/10.0 ));
		*rowmin = zmin;
	    }
	    break;
	case FN_SQ:
	    *rowmin = *rowmax * pow(10.0, gainlolim/5.0);
	    *rowmax = *rowmax * pow(10.0, gainhilim/5.0);
	    if (*rowmax > zmax)
	    {
		*rowmin *= zmax / *rowmax;
		*rowmax = zmax;
	    }
	    if (*rowmin < zmin)
	    {
		*rowmax *= zmin / *rowmin;
		*rowmin = zmin;
	    }
	    break;
	case FN_SQRT:
	    *rowmin = *rowmax * pow(10.0, gainlolim/20.0);
	    *rowmax = *rowmax * pow(10.0, gainhilim/20.0);
	    if (*rowmax > zmax)
	    {
		*rowmin *= zmax / *rowmax;
		*rowmax = zmax;
	    }
	    if (*rowmin < zmin)
	    {
		*rowmax *= zmin / *rowmin;
		*rowmin = zmin;
		}
	    break;
	}
    }
    else
    {
	*rowmin = *rowmax + gainlolim;
	*rowmax = *rowmax + gainhilim;
	if (*rowmax > zmax)
	{
	    *rowmin += zmax - *rowmax;
	    *rowmax = zmax;
	}
	if (*rowmin < zmin)
	{
	    *rowmax += zmin - *rowmin;
	    *rowmin = zmin;
	}
    }
}
/*
 * for debug printout of long-integer arrays
 */

void
pr_larray(text, n, arr)
    char    *text;
    int     n;
    long    *arr;
{
    int     i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
        Fprintf(stderr, "%ld ", arr[i]);
        if (i%5 == 4 || i == n - 1) Fprintf(stderr, "\n");
    }
}

