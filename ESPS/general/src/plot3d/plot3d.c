/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1990 Entropic Speech, Inc.  All rights reserved."   |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: plot3d							|
|									|
|  This program makes a 3-d plot (perspective drawing with hidden	|
|  lines removed) of data from a FEA file.				|
|									|
|  Module: plot3d.c							|
|									|
|  Main program.  Interpret command line; initialize parameters; read	|
|  data.
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)plot3d.c	1.17	1/24/97	ESI";
#endif
#define VERSION "1.17"
#define DATE "1/24/97"

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/constants.h>
#include "plot3d.h"
#include <sys/param.h>

#define REQUIRE(test,text) {if (!(test)) {(void) Fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("plot3d [-w] [-d depth][-f \"field[range]\"][-o orientation][-r range]\n  [-x debug_level][-B length,width,height][-I{aAbBcCpP}...]\n  [-P param][-R bearing,elevation,rotation][-S h_skew:v_skew] [-M] file")


/* ESPS functions */

void	lrange_switch();
void	frange_switch();
long	*grange_switch();
long	get_fea_siz();
long	get_fea_element();
char	*savestring();
char	*get_cmd_line();
char	*arr_alloc();
void	addstr();
long	*fld_range_switch();

/* plot3d functions */

extern void	gr_init();
extern void	interact();
extern void	set_box_len(),	set_box_wid(),	set_box_hgt();
extern void	set_finv(),	set_hskew(),	set_vskew();
extern void	set_ori();
extern void	set_rot(),	set_bear(),	set_elev();

extern char	*read_data();

int		enable_waves_mode=0;	

static int	fea_range();
static void	i3range_switch(), f3range_switch();
void		getsym();
void		pr_larray();

char	*ProgName = "plot3d";
char	*Version = VERSION;
char	*Date = DATE;

int	debug_level = 0;
int     force_monochrome_plot = 0;

/*!*//* Shouldn't be global.  Make separate param-file module. */
char	*onames[] = { "L", "R", NULL };

/*!*/
char		*iname;			/* input file name */
char            *start_rec_string;      
char            *end_rec_string;
char            *start_item_string;
char            *end_item_string;
char            *farg_string;
char            *dir_string;
FILE		*ifile;			/* input stream */
struct header	*ihd =  NULL;		/* input file header */
char		*targ = NULL;		/* y-axis title text */
char		*Harg = NULL;		/* x-axis title text */
char		*Varg = NULL;		/* z-axis title text */
int             data_loaded=0;          /* flag for starting up without feafile specified */
char		*waves_name = "xwaves";


/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int		argc;
    char	**argv;
{
    extern int	optind;			/* for use of getopt() */
    extern char	*optarg;		/* for use of getopt() */
    extern int fullscreendebug;		/* inside of xview library */
    int		ch;			/* command-line option letter */
    char	*cp;			/* pointer into command-line arg */

    char	*farg = NULL;		/* field specification given with -f */
    char	*rarg = NULL;		/* range of records to plot */
    char	*param_name = NULL;	/* parameter file name */
    int		init_axes = YES;	/* draw axes on startup? */
    int		init_box = YES;		/* draw box on startup? */
    int		init_cpanel = YES;	/* put up control panel on startup? */
    int		init_plot = YES;	/* draw plot on startup? */

    char	*darg = NULL;		/* perspective depth */
    char	*oarg = NULL;		/* orientation ("L" or "R") */
    char	*Barg = NULL;		/* box dimensions */
    char	*Rarg = NULL;		/* rot. angles */
    char	*Sarg = NULL;		/* skew factors */
    char	*Xarg = NULL;		/* x-axis limits */
    char	*Yarg = NULL;		/* y-axis limits */
    char	*Zarg = NULL;		/* z-axis limits */

    char	*fieldname;
    long	startitem;		/* starting element in field */
    long	enditem;		/* last element to plot */
    long	startrec;		/* starting record number */
    long	endrec;			/* last record to plot */
    double  	datamin, datamax;   	/* max and min data values */
    int		length, width, height;	/* dimensions of box */
    double	depth;			/* inv. dist. to viewpoint */
    double	alpha, beta;		/* horiz & vert skew factors */
    int		orientation;		/* ORI_LEFT or ORI_RIGHT */
    char	*ostr = NULL;		/* orientation ("L" or "R") */
    double	theta, phi, psi;	/* orientation of box */
    long	i, j;			/* data indices */
    int		ok, err;		/* error codes returned by functions */
    char	*errmsg;		/* error message ret. by function */
    char        *slash = "/";
    char	env_string[MAXPATHLEN+9];

/* Initialize window system */

    fullscreendebug = 1;	/* this global prevents server grabs that
				   crash SGIs */
    gr_init(&argc, argv);

/* set Xprinter env variable */
    sprintf(env_string,"XPPATH=%s/lib/Xp",get_esps_base(NULL));
    putenv(env_string);

/* Parse command-line arguments */

    while ((ch = getopt(argc, argv, "wd:f:o:r:t:x:B:H:I:P:R:S:V:X:Y:Z:M")) != EOF)
	switch (ch)
	{
	case 'd':
	    darg = optarg;
	    break;
	case 'f':
	    farg = optarg;
	    break;
	case 'o':
	    oarg = optarg;
	    break;
	case 'r':
	    rarg = optarg;
	    break;
	case 't':
	    targ = optarg;
	    break;
	case 'w':
	    enable_waves_mode=1;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'I':
/*!*//* Move out of command-line parsing loop. */
	    for (cp = optarg; *cp; cp++)
	    switch (*cp)
	    {
	    case 'a':
		init_axes = YES;
		break;
	    case 'A':
		init_axes = NO;
		break;
	    case 'b':
		init_box = YES;
		break;
	    case 'B':
		init_box = NO;
		break;
	    case 'c':
		init_cpanel = YES;
		break;
	    case 'C':
		init_cpanel = NO;
		break;
	    case 'p':
		init_plot = YES;
		break;
	    case 'P':
		init_plot = NO;
		break;
	    default:
		Fprintf(stderr,
			"%s: unrecognized letter `%c' in -I argument\n",
			ProgName, *cp);
		SYNTAX;
		break;
	    }
	    break;
	case 'B':
	    Barg = optarg;
	    break;
	case 'H':
	    Harg = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'R':
	    Rarg = optarg;
	    break;
	case 'S':
	    Sarg = optarg;
	    break;
	case 'V':
	    Varg = optarg;
	    break;
        case 'X':
	    Xarg = optarg;
	    break;
        case 'Y':
	    Yarg = optarg;
	    break;
        case 'Z':
	    Zarg = optarg;
	    break;
        case 'M':
	    force_monochrome_plot = 1;
	    break;
	case 'n':
	    waves_name = optarg;
	    break;
	default:
	    SYNTAX;
	    break;
	}


/* Process file name and open file. */

    if (argc - optind > 1)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX;
    }

    /* Get parameter values and parse options arguments. */
    (void) read_params(param_name, SC_NOCOMMON, (char *) NULL);

    if (argc - optind == 1) {

	data_loaded++;

	iname = eopen(ProgName, argv[optind], "r", FT_FEA, NONE, &ihd, &ifile);

	if (debug_level)
	    Fprintf(stderr, "Input file: %s\n", iname);

	/* Field name and item range */
	if (!farg)
	    {
		farg = DEF_FIELD;
	    }
	ok = fea_range(farg, &fieldname, &startitem, &enditem, ihd);
	REQUIRE(ok, "field not defined in header");
	REQUIRE(startitem >= 0, "initial item number negative");
	REQUIRE(enditem >= startitem, "Empty range of items");
	REQUIRE(!is_field_complex(ihd, fieldname),
		"complex fields not yet supported");
	
	if (debug_level)
	    Fprintf(stderr,
		    "inital and final item numbers: %d, %d\n",
		    startitem, enditem);

	/* Record range */
	
	startrec = 1;
	endrec = LONG_MAX;
	lrange_switch(rarg, &startrec, &endrec, 0);
	REQUIRE(startrec >= 1, "initial record number not positive");
	REQUIRE(endrec >= startrec, "empty range of records specified");

	if (debug_level)
	    Fprintf(stderr,
		    "inital and final record numbers: %ld, %ld\n",
		    startrec, endrec);


	errmsg = read_data(ifile, ihd, fieldname,
			   startitem, enditem, &startrec, &endrec);
	REQUIRE(!errmsg, errmsg);

	start_rec_string = (char *) malloc(20);
	end_rec_string = (char *) malloc(20);
	start_item_string = (char *) malloc(20);
	end_item_string = (char *) malloc(20);
	spsassert(start_rec_string!=NULL && end_rec_string!=NULL && start_item_string!=NULL && end_item_string!=NULL,
		  "internal error - can't allocate memory for load data menu strings.");
	sprintf( start_rec_string, "%d", startrec);
	sprintf( end_rec_string, "%d", endrec);
	sprintf( start_item_string, "%d", startitem);
	sprintf( end_item_string, "%d", enditem);
	farg_string = savestring(farg);

	dir_string = NULL;
	j = strlen(iname);
	for (i=j-1; i>=0; i--) 
	    if ( slash[0] == iname[i] ) {
		dir_string = (char *) calloc( (i+5), sizeof(char));
		spsassert(dir_string!=NULL, "can't allocate memory for directory string name.");
		strncpy( dir_string, iname, (i+1));
		strcpy( iname, iname+i+1);
		break;
	    }
	if ( dir_string == NULL ) {
	    dir_string = savestring("./"); 
	}
    }

    /* Box geometry */

    length = 400;
    width = 250;
    height = 150;
    if (!Barg)
    {
	getsym("box_length", ST_INT, (char *) &length);
	getsym("box_width", ST_INT, (char *) &width);
	getsym("box_height", ST_INT, (char *) &height);
    }
    else
	i3range_switch(Barg, &length, &width, &height);
    set_box_len(length);
    set_box_wid(width);
    set_box_hgt(height);

    depth = 50.0;
    if (!darg)
	getsym("depth", ST_FLOAT, (char *) &depth);
    else
	depth = atof(darg);
    set_finv(depth);

    alpha = 0.0;
    beta = 0.0;
    if (!Sarg)
    {
	getsym("horizontal_skew", ST_FLOAT, (char *) &alpha);
	getsym("vertical_skew", ST_FLOAT, (char *) &beta);
    }
    else
	frange_switch(Sarg, &alpha, &beta);
    set_hskew(alpha);
    set_vskew(beta);
    
    ostr = "L";
    if (!oarg)
	getsym("orientation", ST_STRING, (char *) &ostr);
    else
	ostr = oarg;
    orientation = lin_search(onames, ostr);
    if (orientation == -1)
    {
	Fprintf(stderr,
		"%s: orientation \"%s\" not recognized.\n",
		ProgName, ostr);
	orientation = ORI_LEFT;
    }
    set_ori(orientation);

    theta = 45.0;
    phi = atan(sqrt(0.5))*180.0/PI;
    psi = 0.0;
    if (!Rarg)
    {
	getsym("bearing", ST_FLOAT, (char *) &theta);
	getsym("elevation", ST_FLOAT, (char *) &phi);
	getsym("rotation", ST_FLOAT, (char *) &psi);
    }
    else
	f3range_switch(Rarg, &theta, &phi, &psi);
    set_bear(theta*PI/180.0);
    set_elev(phi*PI/180.0);
    set_rot(psi*PI/180.0);
    
    /* Axis limits */

    if (Xarg)
    {
    }

    if (Yarg)
    {
    }

    if (Zarg)
    {
    }

    interact(init_axes, init_box, init_cpanel, init_plot);

    exit(0);
    /*NOTREACHED*/
}


static int
fea_range(text, name, start, end, hd)
    char	    *text;
    char	    **name;
    long	    *start, *end;
    struct header   *hd;
{
    char    *string;
    char    *range;
    int	    defined;

    string = savestring(text);
    spsassert(string, "can't allocate space for copy of input string");
    *name = savestring(strtok(string, "["));
    spsassert(*name, "can't allocate space for field name");
    range = strtok((char *) 0, "]");

    defined = get_fea_type(*name, hd) != UNDEF;

    *start = 0;
    if (defined)
	*end = get_fea_siz(*name, hd, (short *) NULL, (long **) NULL) - 1;
    
    if (range)
	lrange_switch(range, start, end, NO);
    free(string);
    return defined;
}


static void
f3range_switch(text, p1, p2, p3)
    char    *text;
    double  *p1, *p2, *p3;
{
    if (!text || text[0] == '\0') return;

    if (text[0] != ':')
    {
	*p1 = atof(text);
	text = strchr(text, ':');
    }

    if (!text || (++text)[0] == '\0') return;

    if (text[0] != ':')
    {
	*p2 = atof(text);
	text = strchr(text, ':');
    }

    if (!text || (++text)[0] == '\0') return;

    *p3 = atof(text);

    return;
}


static void
i3range_switch(text, p1, p2, p3)
    char    *text;
    int	    *p1, *p2, *p3;
{
    if (!text || text[0] == '\0') return;

    if (text[0] != ',')
    {
	*p1 = atoi(text);
	text = strchr(text, ',');
    }

    if (!text || (++text)[0] == '\0') return;

    if (text[0] != ',')
    {
	*p2 = atoi(text);
	text = strchr(text, ',');
    }

    if (!text || (++text)[0] == '\0') return;

    *p3 = atoi(text);

    return;
}


void
getsym(sym, type, var)
    char	*sym;
    int		type;
    char	*var;
{
    int		stype;
    static char	*sym_type_names[] =
    	{   "undefined",    "int",	    "char",	    "float",
	    "string",	    "int array",    "float array"
	};

    stype = (!sym) ? ST_UNDEF : symtype(sym);

    switch (type)
    {
    case ST_INT:
    case ST_CHAR:
    case ST_FLOAT:
    case ST_STRING:
	if (stype == ST_UNDEF)
	    return;
	else
	if (stype == type)
	    switch (type)
	    {
	    case ST_INT:
		*((int *) var) = getsym_i(sym);
		break;
	    case ST_CHAR:
		*((char *) var) = getsym_c(sym);
		break;
	    case ST_FLOAT:
		*((double *) var) = getsym_d(sym);
		break;
	    case ST_STRING:
		*((char **) var) = getsym_s(sym);
		break;
	    }
	else
	    Fprintf(stderr, "getsym: symbol %s is not of type %s.\n",
		    sym, sym_type_names[type]);
	break;
    case ST_UNDEF:
    case ST_IARRAY:
    case ST_FARRAY:
	Fprintf(stderr, "getsym: can't handle type %s.\n",
		sym_type_names[type]);
	break;
    default:
	Fprintf(stderr, "getsym: can't handle type code %d.\n", type);
	break;
    }
}


/*
 * for debug printout of arrays
 */

void
pr_darray(text, n, arr)
    char    *text;
    int     n;
    double  *arr;
{
    int     i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
        Fprintf(stderr, "%g ", arr[i]);
        if (i%5 == 4 || i == n - 1) Fprintf(stderr, "\n");
    }
}

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


