/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|   xmcd - gps filter for X windows
|									|
|   Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
static char    *sccs_id = "@(#)mcd.c	1.3	6/6/89	ESI";
#endif
#define VERSION "3.1"
#define DATE "10/7/87"

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/spsassert.h>
#include <esps/constants.h>

#define SYNTAX	    USAGE("mcd [-r region][-u][-x level] [-(file.gps stdin)]")
#define EOF_ERR	    {Fprintf(stderr, "%s: unexpected end of file.\n", \
		    ProgName); exit(1);}
#define READ	    fread((char *) &input_word, sizeof input_word, 1, infile)

#define SAVE	    (void) fwrite((char *) &input_word, sizeof input_word, 1, tfile)
#define GET1(code, length) (READ && (code = (input_word>>12) & 0xf, \
		    length = input_word & 0xfff, 1))
#define GET2(coord) {if (READ) coord = input_word; else EOF_ERR}
#define GET3(m,n)   {if (READ) {m = (input_word>>8) & 0xff; \
		    n = input_word & 0xff;} else EOF_ERR}
#define GETCHRS(ch1,ch2) {if (READ) {ch1 = input_chr[0]; \
		    ch2 = input_chr[1];} else EOF_ERR}

#define L_ROUND(n) ((n)>=0 ? (long)((n)+0.5) : -(long)(-(n)+0.5))

#define LINES	    0
#define ARC	    3
#define TEXT	    2
#define HARDWARE    4
#define COMMENT	    15

#define BUFSIZE	    2046
#define TXTSIZE	    8185
#define TEXT_OFFSET   20	/* space to save for text when using range */

double          sin(), cos();

int 
init_dev(), erase(), label(), line(), circle(),
arc(), move(), cont(), point(), linemod(),
space(), closepl();
int             text();
int             plotline();
void            do_plot();
void            start_plot();

char           *ProgName = "mcd";
char           *Version = VERSION;
char           *Date = DATE;

short           input_word;
char           *input_chr = (char *) &input_word;
int             c;
extern          optind;
extern char    *optarg;
int             e_flag = NO;
int             u_flag = NO;
int             region = 0;
int             debug_level = 0;
FILE           *infile;
char           *inname;
FILE           *tfile;
char           *tmpname;
int             code;
int             length;
static int      xdata[BUFSIZE], ydata[BUFSIZE];
static char     chdata[TXTSIZE];
static int      region_x[] = {-32767, -20479, -8191, 4096, 16384};
static int      region_y[] = {-32767, -19660, -6553, 6553, 19660};
int             x, y;
int             bundle;
int             unused;
int             size;
int             rotation;
double          cosrot, sinrot;
int             i;
int             x0 = 32767, z0 = 32767, x1 = -32767, z1 = -32767;
char 		*title = "ESPS Plot";
char		*icon_name = "SINE";

char            *geom = NULL;
Widget          toplevel;

main(argc, argv)
     int             argc;
     char          **argv;
{
    int		onintr();
    char stn[40];

    /* parse the command line and remove all X Window related options
     * before calling getopt().
     */

    (void)signal(SIGTERM, onintr);	/* set up to catch SIGTERM */
    (void)signal(SIGINT, onintr);	/* set up to catch SIGINT */

    if (debug_level)
	for (i = 0; i < argc; i++)
	    fprintf (stderr, "argv[%d] = %s\n", i, argv[i]);

    /* now parse all xmcd(1-ESPS) related command options */

	while ((c = getopt(argc, argv, "T:r:ux:t:i:e")) != EOF)
		switch (c) {
		case 'T':
			break;
		case 'e':
			e_flag = YES;
			break;
		case 'r':
			region = atoi(optarg);
			u_flag = NO;
			break;
		case 'u':
			region = 0;
			u_flag = YES;
			break;
		case 'x':
			debug_level = atoi(optarg);
			break;
		case 't':
			title = optarg;
			break;
		case 'i':
			icon_name = optarg;
			break;
		default:
			SYNTAX;
			exit();
		}

    /*!!!!!!!! PATCH TO MAKE IT WORK WITH PLOTSD !!!!!!!!!!*/
    /* plotsd passes "=500x500+0+0" for geometry argument, rather than
       "-geometry =500x500+0+0" */

         if( argc-optind == 0 ){       /* no geom, no -, no filename */
	   inname = "<stdin>";
	   infile = stdin;
	 }else if( argc-optind == 1){
	   char c;
	   char *b="=";
	   c = argv[argc-1][0];
	   if( strcmp(&c, b) == 0){    /* geom, no -, no filename  */
	     geom = (char *) malloc(40*sizeof(char));
	     strcpy(geom, argv[argc-1]);
	     inname = "<stdin>";
	     infile = stdin;
	   }else if(strcmp(argv[argc-1], "-") == 0){   /* no geom, - */
	     inname = "<stdin>";
	     infile = stdin;
	   }else{                                      /* no geom, filename */
	     inname = argv[argc-1];
	     TRYOPEN(ProgName, inname, "r", infile);
	   }
	 }else if (argc - optind == 2){
	   geom = (char *) malloc(40*sizeof(char));
	   strcpy(geom, argv[argc-2]);
	   if( strcmp(argv[argc-1], "-") == 0){       /* geom, - */
	     inname = "<stdin>";
	     infile = stdin;
	   } else{
	     inname = argv[argc-1];                    /* geom, filename */
	     TRYOPEN(ProgName, inname, "r", infile);
	   }
	 }else {
	   SYNTAX; 
	   exit();
	 }

        toplevel = XtInitialize (argv[0], "XMcd", NULL, 0, &argc, argv);

	init_device ();

	if (!e_flag)
		erase();

	/*
	 * read the gps stream into a temporary file
	 */

	tmpname = mktemp("/var/tmp/mcdXXXXXX");
	TRYOPEN(ProgName, tmpname, "w", tfile);
	while (GET1(code, length)) {
		SAVE;
		if (debug_level >= 1)
			Fprintf(stderr, "Writing temporary file.\n");
		switch (code) {
		case LINES:
			if (debug_level >= 1)
				Fprintf(stderr, "lines\t(%d words)\n", length);
			spsassert(length % 2 == 0,
			 "can't draw line with odd number of coordinates.");
			spsassert(length >= 4,
				  "cant't draw line with no coordinates.");

			for (i = 0; i < (length - 2) / 2; i++) {
				GET2(x);
				SAVE;
				GET2(y);
				SAVE;
				if (x < x0)
					x0 = x;
				if (y < z0)
					z0 = y;
				if (x > x1)
					x1 = x;
				if (y > z1)
					z1 = y;
			}

			GET3(bundle, unused);
			SAVE;

			break;
		case ARC:
			if (debug_level >= 1)
				Fprintf(stderr, "arc\t(%d words)\n", length);
			spsassert(length % 2 == 0,
			  "can't draw arc with odd number of coordinates.");
			spsassert(length != 0,
				  "cant't draw arc with no coordinates.");
			for (i = 0; i < (length - 2) / 2; i++) {
				GET2(xdata[i]);
				SAVE;
				GET2(ydata[i]);
				SAVE;
			}

			GET3(bundle, unused);
			SAVE;

			break;
		case TEXT:
			if (debug_level >= 1)
				Fprintf(stderr, "text\t(%d words)\n", length);
			spsassert(length >= 6, "incomplete text command");
			GET2(x);
			SAVE;
			GET2(y);
			SAVE;
			GET3(bundle, unused);
			SAVE;
			GET3(size, rotation);
			SAVE;

			for (i = 0; i < 2 * (length - 5); i += 2) {
				GETCHRS(chdata[i], chdata[i + 1]);
				SAVE;
			}
			chdata[i] = '\0';

			cosrot = cos(PI * rotation / 128.0);
			sinrot = sin(PI * rotation / 128.0);
			{
				int xhi = x, xlo = x, yhi = y, ylo = y;
				double rgt = 
				 4.0 * size * (strlen(chdata) - 0.5), 
				 lft = 2.0 * size, top = 2.5 * size, 
				 bot = 4.5 * size;

				if (cosrot > 0) {
					xhi += rgt * cosrot;
					xlo -= lft * cosrot;
					yhi += top * cosrot;
					ylo -= bot * cosrot;
				} else {
					xhi -= lft * cosrot;
					xlo += rgt * cosrot;
					yhi -= bot * cosrot;
					ylo += top * cosrot;
				}
				if (sinrot > 0) {
					xhi += bot * sinrot;
					xlo -= top * sinrot;
					yhi += rgt * sinrot;
					ylo -= lft * sinrot;
				} else {
					xhi -= top * sinrot;
					xlo += bot * sinrot;
					yhi -= lft * sinrot;
					ylo += rgt * sinrot;
				}
				if (xlo < x0)
					x0 = xlo;
				if (ylo < z0)
					z0 = ylo;
				if (xhi > x1)
					x1 = xhi;
				if (yhi > z1)
					z1 = yhi;
			}

			break;
		case HARDWARE:
			if (debug_level >= 1)
				Fprintf(stderr, 
				 "hardware text\t(%d words)\n", length);
			spsassert(length >= 4, "incomplete hardware text command");
			GET2(x);
			SAVE;
			GET2(y);
			SAVE;
			for (i = 0; i < 2 * (length - 3); i += 2) {
				GETCHRS(chdata[i], chdata[i + 1]);
				SAVE;
			}
			chdata[i] = '\0';

			break;
		case COMMENT:
			if (debug_level >= 1)
				Fprintf(stderr,"comment\t(%d words)\n", length);
			for (i = 0; i < length - 1; i++) {
				GET2(unused);
				SAVE;
			}
			break;
		default:
			Fprintf(stderr, 
			 "%s: unimplemented plotting command (code %d).\n",
			 ProgName, code);
			exit(1);
			break;
		}
	}

	(void)fclose(tfile);

	if (u_flag) {
		space(-32767, -32767, 32767, 32767);
		if (debug_level >= 1)
			Fprintf(stderr, "space(%d, %d, %d, %d)\n",
				-32767, -32767, 32767, 32767);
	} else if (region) {
		x = region_x[(region - 1) % 5];
		y = region_y[(region - 1) / 5];
		space(x, y, x + 16383, y + 13107);
		if (debug_level >= 1)
			Fprintf(stderr, 
			 "space(%d, %d, %d, %d)\n", x, y, x + 16383, y + 13107);
	}

/* call Sunviews stuff to start the action */
	start_plot();

/* if you want to use this file as a GPS front-end to some other
   device or window system, all you have to do, is to have the start_plot
   routine call back the do_plot function.
*/
}

void
do_plot()
{
	TRYOPEN(ProgName, tmpname, "r", tfile);
	infile = tfile;
	if (debug_level >= 1)
		Fprintf(stderr, "Reading temporary file.\n");
	space(x0, z0, x1, z1);
	if (debug_level >= 1)
		Fprintf(stderr, "space(%d, %d, %d, %d)\n", x0, z0, x1, z1);
	while (GET1(code, length)) {
		switch (code) {
		case LINES:
			if (debug_level >= 1)
				Fprintf(stderr, "lines\t(%d words)\n", length);
			spsassert(length % 2 == 0,
			 "can't draw line with odd number of coordinates.");
			spsassert(length >= 4,
				  "cant't draw line with no coordinates.");

			for (i = 0; i < (length - 2) / 2; i++) {
				GET2(xdata[i])
					GET2(ydata[i])
					if (debug_level >= 2)
					Fprintf(stderr, 
					 "\t%d %d\n", xdata[i], ydata[i]);
			}

			GET3(bundle, unused)
				if (debug_level >= 2)
				Fprintf(stderr, 
				 "\tbundle %d, unused %d.\n", bundle, unused);

			move(xdata[0], ydata[0]);
			for (i = 1; i < (length - 2) / 2; i++)
				cont(xdata[i], ydata[i]);

			break;
		case ARC:
			if (debug_level >= 1)
				Fprintf(stderr, "arc\t(%d words)\n", length);
			spsassert(length % 2 == 0,
			  "can't draw arc with odd number of coordinates.");
			spsassert(length != 0,
				  "cant't draw arc with no coordinates.");
			for (i = 0; i < (length - 2) / 2; i++) {
				GET2(xdata[i])
					GET2(ydata[i])
					if (debug_level >= 2)
					Fprintf(stderr, 
					 "\t%d %d\n", xdata[i], ydata[i]);
			}

			GET3(bundle, unused)
				if (debug_level >= 2)
				Fprintf(stderr, 
				 "\tbundle %d, unused %d.\n", bundle, unused);
			break;
		case TEXT:
			if (debug_level >= 1)
				Fprintf(stderr, "text\t(%d words)\n", length);
			spsassert(length >= 6, "incomplete text command");
			GET2(x)
				GET2(y)
				if (debug_level >= 2)
				Fprintf(stderr, "\tx %d, y %d\n", x, y);
			GET3(bundle, unused)
				if (debug_level >= 2)
				Fprintf(stderr, 
				 "\tbundle %d, unused %d.\n", bundle, unused);
			GET3(size, rotation)
				if (debug_level >= 2)
				Fprintf(stderr, 
				 "\tsize %d, rotation %d.\n", size, rotation);
			for (i = 0; i < 2 * (length - 5); i += 2)
				GETCHRS(chdata[i], chdata[i + 1])
					chdata[i] = '\0';
			if (debug_level >= 2)
				Fprintf(stderr, "\t%s\n", chdata);

			cosrot = cos(PI * rotation / 128.0);
			sinrot = sin(PI * rotation / 128.0);
			text(L_ROUND(x - (2.0 * cosrot - 2.5 * sinrot) * size),
			  L_ROUND(y - (2.0 * sinrot + 2.5 * cosrot) * size),
			   (long) size * 4, ROUND(rotation * 360.0 / 256.0),
			     chdata, plotline);

			break;
		case HARDWARE:
			if (debug_level >= 1)
				Fprintf(stderr, 
				 "hardware text\t(%d words)\n", length);
			spsassert(length >= 4, 
			 "incomplete hardware text command");
			GET2(x)
				GET2(y)
				if (debug_level >= 2)
				Fprintf(stderr, "\tx %d, y %d\n", x, y);
			for (i = 0; i < 2 * (length - 3); i += 2)
				GETCHRS(chdata[i], chdata[i + 1])
					chdata[i] = '\0';
			if (debug_level >= 2)
				Fprintf(stderr, "\t%s\n", chdata);

			move(x, y);
			label(chdata);

			break;
		case COMMENT:
			if (debug_level >= 1)
				Fprintf(stderr, 
				 "comment\t(%d words)\n", length);
			for (i = 0; i < length - 1; i++) {
				GET2(unused)
					if (debug_level >= 2)
					Fprintf(stderr, "%x\n", unused);
			}
			break;
		default:
			Fprintf(stderr, 
			 "%s: unimplemented plotting command (code %d).\n",
				ProgName, code);
			exit(1);
			break;
		}
	}
	(void)fclose(tfile);
}


int
plotline(n, u, v)
	long            n;
	long           *u, *v;
{
	long            i;

	spsassert(n > 0, "cant't draw arc with no coordinates.");
	move((int) u[0], (int) v[0]);
	for (i = 1; i < n; i++)
		cont((int) u[i], (int) v[i]);
}

/* on interrupt or kill, remove the temp file
*/
onintr()
{
	(void)unlink(tmpname);
	exit(1);
}
