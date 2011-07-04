/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1988-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by: Ken Nelson, John Shore
 *
 * Brief description:
 *  This program puts in common a sampled data range (in points) given
 *  a sampled data range in seconds.  
 */

#ifndef lint
static char *sccs_id = "@(#)setrange.c	1.14	06 Oct 1999 ESI";
#endif

int debug_level = 0;

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("setrange [-{prs} range] [-x debug_level] [-z] sdfile") ;

struct header  *sdtofea();
void	frange_switch();
char	*ProgName = "setrange";

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int
	    optind;		/* for use of getopt() */
    extern char
	    *optarg;		/* for use of getopt() */
    int	    ch;			/* command-line option letter */
    char    *srange;		/* range string from -s */
    int	    sflag = 0;		/* -s option specified? */
    int	    pflag = 0;		/* -p option specified? */
    int	    zflag = 0;		/* -z option (suppress) specified? */
    double  start_s;		/* start second in the input file */
    double  nan_s;		/* total time of range */
    double  end_s;		/* end of range in seconds*/
    double  start_time = 0;     /* start_time generic */
    long    start_p;		/* starting sample */
    long    end_p;		/* last sample */
    long    nrec;		/* number of records in file */
    long    nan;		/* total number of samples to analyze */
    float   isf = 0;		/* input file sampling frequency */
    char    *iname;		/* input file name */
    FILE    *ifile;		/* input stream */
    char    *param_name = NULL;
				/* parameter file name */
    struct header
	    *hd;		/* input file header */
/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "P:p:r:s:x:z")) != EOF)
        switch (ch)
	{
	case 'P':
	    param_name = optarg;
	    break;
	case 's':
	    srange = optarg;
	    sflag++;
	    break;
	case 'p':
	case 'r':
	    srange = optarg;
	    pflag++;
	    break;
	case 'z':
	    zflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	default:
	    SYNTAX
	    break;
	}

    if (argc - optind > 1) {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }

    if (argc - optind < 1) {
	Fprintf(stderr,
	    "%s: must specify a FEA file name.\n", ProgName);
	SYNTAX
    }

    /*we open the file; note, we don't care what file type it is*/

    iname = argv[optind++];

    TRYOPEN (ProgName, iname, "r", ifile);

    if (!(hd = read_header (ifile)))
	NOTSPS (ProgName, iname);

    if (debug_level)
	Fprintf(stderr, "Input file: %s\n", iname);

    /*convert to FEA_SD if FT_SD*/

    if (hd->common.type == FT_SD)
	hd = sdtofea(hd);

    isf = get_genhd_val("record_freq", hd, 0.0);

    if (debug_level)
	Fprintf(stderr, "sampling rate = %g\n", isf);

    REQUIRE(isf > 0,
	    "generic record_freq (sample fequency) is <= 0 or not found");

    nrec = hd->common.ndrec;
    if (nrec == -1)
    {
	struct fea_data	*rec;

	REQUIRE(hd->common.type == FT_FEA, "non-FEA file can't be on a pipe");

	/* Count records by reading */

	rec = allo_fea_rec(hd);
	for (nrec = 0; get_fea_rec(rec, hd, ifile) != EOF; nrec++)
	{ }

	free_fea_rec(rec);
    }

    (void) read_params(param_name, SC_NOCOMMON, iname);

/*
 * Get start_time from input file - defaults to zero if not there
 */
    start_time = get_genhd_val("start_time", hd, (double)0);

/*
 * Process range option
 */
    if (pflag) {
	start_p = 1;
	end_p = LONG_MAX;
	lrange_switch(srange, &start_p, &end_p);
	REQUIRE(end_p >= start_p, "end can't be before the start");
    	if (end_p > nrec) {
	    end_p = nrec;
	    Fprintf(stderr,
		"%s: Warning - range extends beyond file; truncated\n",
		ProgName);
	}
	start_s = start_time + (start_p - 1)/isf;
	end_s = start_time + (end_p - 1)/isf;
	nan_s = end_s - start_s;
	nan = end_p - start_p + 1;

    }

    if (sflag) {
	start_s = 0;
	end_s = DBL_MAX;
	frange_switch(srange, &start_s, &end_s);
    }
    if (!sflag && !pflag) {
	start_s =
	    (symtype("start_s") != ST_UNDEF)
	    ? getsym_d("start_s")
	    : 0;

	if (symtype("nan_s") != ST_UNDEF && getsym_d("nan_s") != 0)
	    end_s = start_s + getsym_d("nan_s");
	else
	    end_s = DBL_MAX;

	sflag = 1;
    }
    if (sflag) {
	REQUIRE(end_s >= start_s, "end can't be before the start");

	start_p = (start_s - start_time)*isf + 1;

	if (start_p < 1) {
	    start_p = 1;
	    Fprintf(stderr,
		    "%s: Warning - range starts before file; truncated\n",
		    ProgName);
	    start_s = start_time;

	}

	if (end_s == DBL_MAX)
	    end_p = nrec;
	else
	    end_p = end_s * isf + 1;

	if (end_p > nrec) {
	    end_p = nrec;
	    end_s = start_time + (end_p - 1)/isf;
	    Fprintf(stderr,
		    "%s: Warning - range extends beyond file; truncated\n",
		    ProgName);
	}

	nan = end_p - start_p + 1;
	nan_s = end_s - start_s;
    }

    if (debug_level) {
	Fprintf(stderr,
		"%s: start_s = %g, end_s = %g\n", ProgName, start_s, end_s);
	Fprintf(stderr,
		"%s: start_p = %ld, end_p = %ld, nan = %ld\n",
		ProgName, start_p, end_p, nan);
    }

    if (!zflag) {
	Fprintf(stdout,
		"%s: starting record = %ld, number of records = %ld\n",
		ProgName, start_p, nan);
    }
/*
 * put info in ESPS common
 */
    if (end_s == DBL_MAX)
	end_s = FLT_MAX;
    if (nan_s == DBL_MAX)
	nan_s = FLT_MAX;

    (void) putsym_s("filename", iname);
    (void) putsym_s("prog", ProgName);
    (void) putsym_i("start", (int) start_p);
    (void) putsym_i("nan", (int) nan);
    (void) putsym_f("start_s", (float) start_s);
    (void) putsym_f("end_s", (float) end_s);
    (void) putsym_f("nan_s", (float) nan_s);

    exit(0);
    /*NOTREACHED*/
}



