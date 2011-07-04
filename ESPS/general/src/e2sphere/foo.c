/*
 * This material contains proprietary software of Entropic Research 
 * Laboratory, Inc. Any reproduction, distribution, or publication 
 * of this work must be authorized in writing by Entropic Research 
 * Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 */

/* PROGRAM:           e2sphere - convert FEA_SD to NIST Sphere form
 *  
 * Author:            John Shore (based on setmax, and byrne's btosphere)
 * Maintained by:     Shore
 */

#ifndef lint
static char *sccs_id = "%W%	%G%	ERL";
#endif

#define VERSION "%I%"
#define DATE "%G%"

#define BUFSIZE 8192

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/feasd.h>
#include <esps/fea.h>

/* sphere include files */

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}
#define SYNTAX \
USAGE("e2sphere [-b sample_byte_format] [-x debug_level] in.fea_sd out.sphere") ;

struct header_t *sd_to_sphere();

char	*ProgName = "e2sphere";
char	*Version = VERSION;
char	*Date = DATE;
int     debug_level = 0;	/* debug level; global for library*/

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
    char    *iname;		/* input file name */
    FILE    *ifile = stdin;	/* input stream */
    char    *oname;		/* output file name */
    FILE    *ofile = stdout;	/* output stream */
    char *byte_format = "10"; 


/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "x:b:")) != EOF)
        switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'b':
	    byte_format = optarg;
	    if(! (  (strcmp(byte_format, "10") == 0)
		  ||(strcmp(byte_format, "01") == 0))) {

	       Fprintf(stderr, 
		 "%s: sample_byte_format (-b) must be '01' or '10'.\n",
		       ProgName);
	       SYNTAX
	       exit(1);
	      }
	    break;
	default:
	    SYNTAX
	    break;
	}

/* Process file names and open files. */

    if (argc - optind > 2) {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 2) {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }

    iname = argv[optind++];

    if (strcmp( "-", iname) != 0 ) 
      ifile = fopen(iname, "r");  
    
    REQUIRE(ifile != NULL, "couldn't open input file.\n");
    
    oname = argv[optind++];

    if ( strcmp( "-", oname) != 0 ) 
	ofile = fopen( oname, "w");

    REQUIRE(ifile != NULL, "couldn't open output file.\n");

    if (debug_level)
	Fprintf(stderr, "%s: Input file = %s\n\tOutput file = %s\n",
	    ProgName, iname, oname);
    (void) sd_to_sphere(ifile, ofile, byte_format);

    Fclose(ofile);
    Fclose(ifile);

    exit(0);
    /*NOTREACHED*/
}    

