/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
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
 * Compute distortion measures between two ESPS files.
 *
 */

static char *sccs_id = "@(#)distort.c	3.9	06/14/99	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/anafea.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/spec.h>
#include <esps/unix.h>
#include <esps/feaspec.h>

#ifdef ESI
#include <esps/ana.h>
#include <esps/pitch.h>
#endif

#define SYNTAX	USAGE ("distort [-x debug-level] [-f range] [-e range] [-E] [-r] [-s] file1 file2")

#define	  ABS(x)	((x) > 0 ? (x) : (-x))
#define   SQUARE(x)	(x) * (x)

extern	char	*optarg;
extern	int 	optind;

/*
 * U N I X
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

/* done via <esps/unix.h>*/


/*
 * E S P S
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

char	*get_cmd_line();
short	get_rec_len();
void	lrange_switch();
int	getopt();

/* GLOBAL VARIABLES */

char		*f1str, *f2str;		/* string pointers */
FILE		*f1p, *f2p;		/* file pointers */
struct header	*f1h, *f2h;		/* pointers to structures */

int		debug_level = 0;	/* flag for debug level */
int		eflag = 0;		/* range of elements flag */
int		Eflag = 0;		/* E option flag */
int		rflag = 0;		/* r flag */
int		sflag = 0;		/* Itakura-Saito distortion flag */
int		nflag = 0;		/* suppress output for SPEC files */

long		s_rec;			/* starting record position */
long		e_rec;			/* ending record position */
long		s_ele;			/* starting element position */
long		e_ele;			/* ending element position */



main (argc, argv)		/*  distort program */
int argc;
char **argv;
{

int	c;
    
int	fflag = 0;		/* range of records flag */

char	*f_range;	/* range of records */
char	*e_range;	/* range of elements */
long	ndrec1, ndrec2;	/* number of records in files 1 and 2 */


void	error_ft();	/* file type error function */

#ifdef ESI
    void check_ana();
    void check_pit();
    void ana_distort();	/* external function for processing ESPS ANA files */
#endif

void	check_anafea();
void	check_spec();
void	check_sd();

void	anafea_distort(); /* external function for ESPS FEA_ANA files */
void	spec_distort();	 /* external function for ESPS SPEC files */
void	gen_distort();	/* external function for processing generic files */

/*
 *  BEGIN
 *    distort
 *      program
 */

/* get options from command line and set various flags */

    while ( (c = getopt (argc, argv, "x:f:e:Ersn")) != EOF )
	switch (c) {
	    case 'x':
		debug_level = atoi(optarg);
		break;
	    case 'f':
		f_range = optarg;
		fflag++;
		break;
	    case 'e':
		e_range = optarg;
		eflag++;
		break;
	    case 'E':
		Eflag++;
		break;
	    case 'r':
		rflag++;
		break;
	    case 's':
	        sflag++;
		break;
	    case 'n':
	        nflag++;
		break;
	    default:
		SYNTAX;
	}


    if ( argc - optind < 2 )	/* must process two files */
	SYNTAX;

    f1str = argv[optind++];
    f2str = argv[optind];

    if (debug_level > 2) 
	(void) fprintf (stderr,
	"distort: file1 = %s, file2 = %s.\n", f1str, f2str);

    if ( strcmp(f1str,"-") == 0) {
	(void) fprintf (stderr, "distort:  file1 must be an input file.\n");
	exit(1);
    } else
	TRYOPEN (argv[0], f1str, "r", f1p);

    if ( strcmp(f2str,"-") == 0) {
	(void) fprintf (stderr, "distort:  file2 must be an input file.\n");
	exit(1);
    } else
	TRYOPEN (argv[0], f2str, "r", f2p);

/* check if input files are ESPS files, exit if not */

    if ( !(f1h = read_header (f1p)) )
	 NOTSPS ("distort:", f1str);
    if ( !(f2h = read_header (f2p)) )
	 NOTSPS ("distort:", f2str);

/*
 * check if there are complex fields in the file
 * if so, abort
 */
    if(f1h->common.type == FT_FEA && f2h->common.type == FT_FEA) {
	/* only FEA files can be complex */
	if (is_file_complex(f1h) || is_file_complex(f2h)) {
	    Fprintf(stderr, 
		"distort: Doesn't yet work on files that have complex data - exiting.\n");
	    exit(1);
	}
    }

    ndrec1 = f1h->common.ndrec;
    ndrec2 = f2h->common.ndrec;

    if (debug_level)
    {
	if (ndrec1 == -1)
	    Fprintf(stderr, "distort: unknown number of ");
	else
	    Fprintf(stderr, "distort: %ld ", ndrec1);
	Fprintf(stderr, "records in file %s.\n", f1str);

	if (ndrec2 == -1)
	    Fprintf(stderr, "distort: unknown number of ");
	else
	    Fprintf(stderr, "distort: %ld ", ndrec2);
	Fprintf(stderr, "records in file %s.\n", f2str);
    }

    s_rec = 1;
    e_rec = ndrec1;

    if (fflag)
	lrange_switch(f_range, &s_rec, &e_rec, 1);

    if (e_rec == -1)	/* E.g. Esignal ASCII format. */
    {
	Fprintf(stderr, "distort: variable record length in file %s; ", f1str);
	Fprintf(stderr, "please specify an end record with -f.\n");
	exit(1);
    }

    if (s_rec > e_rec)
    {
	Fprintf(stderr, "distort: start record greater than end record.\n");
	exit (1);
    }

    if (ndrec1 != -1 && e_rec > ndrec1)
    {
	Fprintf(stderr, "distort: only %ld records in %s.\n", ndrec1, f1str);
	exit(1);
    }

    if (ndrec2 != -1 && e_rec > ndrec2)
    {
	Fprintf(stderr, "distort: only %ld records in %s.\n", ndrec2, f2str);
	exit(1);
    }

    if (s_rec == 0)
	s_rec = 1;

    if (debug_level)
	Fprintf(stderr,
		"distort: starting at record %ld, ending at %ld.\n",
		s_rec, e_rec);

    s_ele = 1;
    e_ele = MAX (get_rec_len (f1h), get_rec_len (f2h));

    if ( eflag )
       lrange_switch (e_range, &s_ele, &e_ele, 1);

    if (s_ele == 0) {
	(void) fprintf (stderr,
	"distort: cannot compute distortion for tags.\n");
	exit (1);
    }

    if ( debug_level > 2 )
	(void) fprintf (stderr,
	"distort: element range is from %d to %d.\n", s_ele, e_ele);

    (void) fprintf (stdout, get_cmd_line (argc, argv));
    (void) fprintf (stdout, "\n\n");

    if (debug_level > 1) {
	(void) fprintf (stderr,
	"distort: skipping %d record", s_rec - 1);
	(void) fprintf (stderr,
	((s_rec - 1) > 1) ? "s.\n" : ".\n");
    }

    fea_skiprec (f1p, (long) s_rec - 1, f1h);
    fea_skiprec (f2p, (long) s_rec - 1, f2h);

    if (debug_level > 1) {
        (void) fprintf (stderr,
	"distort: skipped %d record", s_rec - 1);
	(void) fprintf (stderr,
	((s_rec - 1) > 1) ? "s.\n" : ".\n");
    }

/*
 * Here is the   B I G   S W I T C H
 * to handle the file types currently supported.
 */

    switch ( f1h->common.type ) {

#ifdef ESI
	case FT_ANA:
	   if ( f2h->common.type != FT_ANA ) 
	      (void) error_ft (f1str, f2str);
	   (void) check_ana (f1h, f2h);
	   (void) ana_distort();
	break;

	case FT_PIT:
	   if ( f2h->common.type != FT_PIT ) 
	      (void) error_ft (f1str, f2str);
	   (void) check_pit (f1h, f2h);
	   (void) gen_distort();
	break;
#endif

	case FT_FEA:
	    if ( f2h->common.type != FT_FEA )
		(void) error_ft (f1str, f2str);
	    if ((f1h->hd.fea->fea_type == FEA_ANA)
			&& (f2h->hd.fea->fea_type == FEA_ANA)) {
		(void) check_anafea (f1h, f2h);
		(void) anafea_distort();
	    }
	    else if ((f1h->hd.fea->fea_type == FEA_SPEC)
		        && (f2h->hd.fea->fea_type == FEA_SPEC)){
	   (void) check_spec (f1h, f2h);
	   (void) spec_distort();
	    }
	   else if ((f1h->hd.fea->fea_type == FEA_SD)
		    && (f2h->hd.fea->fea_type == FEA_SD)) {
	            (void) check_sd(f1h, f2h);
		    (void) gen_distort();
		  }
	   else
		(void) gen_distort();
	break;

	default:
	   if (f2h->common.type != f1h->common.type)
	      (void) error_ft (f1str, f2str);
	   (void) gen_distort();
	break;

     } /* end    B I G   S W I T C H  ( f1h->common.type ) */

     (void) fclose (f1p);
     (void) fclose (f2p);
    exit(0);
    return(0);
}


void
error_ft (file1, file2)		/* print error message and exit */
char *file1;
char *file2;
{
	(void) fprintf (stderr,
	"distort: %s and %s must be of the same ESPS file type.\n",
	file1, file2);
	exit (1);
}

