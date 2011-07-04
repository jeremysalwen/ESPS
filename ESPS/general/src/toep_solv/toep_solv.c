/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by: Bill Byrne to write feafilt files
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)toep_solv.c	1.6	8/8/91	ESI/ERL";
int	    debug_level = 0;		/* debug level */



/*----------------------------------------------------------------------+
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: toep_solv.c							|
|									|
|  This program real symmetric Toeplitz systems of linear equations	|
|  whose coefficients are given in an input FEA file (typically a	|
|  FEA_ANA file containing autocorrelation coefficients) and whose	|
|  right-hand sides (typically cross-correlation vectors) are given in	|
|  a second FEA file.  The solutions are written to a FEAFILT file.	|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/

#define VERSION "8/8/91"
#define DATE "1.6"

#include <stdio.h>

#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feafilt.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("toep_solv [-f crsfield][-x debug_level][-F autfield][-P param]\n\
 [-W pwrfield] autocor.fea crosscor.fea output.filt") \
 ;


char	*get_cmd_line();

char	*ProgName = "toep_solv";
char	*Version = VERSION;
char	*Date = DATE;


/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int	    argc;
    char    **argv;
{
    extern int
	    optind;		/* for use of getopt() */
    extern char
	    *optarg;		/* for use of getopt() */
    int	    ch;			/* command-line option letter */

    int	    fflag = NO;		/* -f option specified? */
    char    *crsfield;		/* field name for cross correlations */

    int	    Fflag = NO;		/* -F option specified? */
    char    *autfield;		/* field name for autocorrelations */

    int	    Wflag = NO;		/* -W option specified? */
    char    *pwrfield;		/* field name for power */

    char    *param_name = NULL;
				/* parameter file name */
    char    *autname, *crsname;	/* input file names */
    FILE    *autfile, *crsfile;	/* input streams */
    struct header
	    *authd, *crshd;	/* input file headers */
    struct fea_data
	    *autrec,
	    *crsrec;		/* input records  */
    float   *autptr;		/* pointer to autocorrelation field
							in input record */
    float   *pwrptr;		/* pointer to power field
							in input record */
    float   *crsptr;		/* pointer to cross_correlation field
							in input record */
    double  *autvec, *crsvec;	/* vectors of values from input records */
    long    autsize, pwrsize,
	    crssize;		/* input field sizes */
    int	    order;		/* order of solution obtained */
    char    *oname;		/* output file name */
    FILE    *ofile;		/* output stream */
    struct header
	    *ohd;		/* output file header */
    struct feafilt
	    *orec;		/* output record */
    filtdesparams  
	    *fdp;               /* FEA_FILT header info */
    double  *ovec;		/* solution vector */
    long    i;			/* loop index */
    long    num_recs;		/* output record counter */

/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "f:x:F:P:W:")) != EOF)
        switch (ch)
	{
	case 'f':
	    fflag = YES;
	    crsfield = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'F':
	    Fflag = YES;
	    autfield = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'W':
	    Wflag = YES;
	    pwrfield = optarg;
	    break;
	default:
	    SYNTAX
	    break;
	}
/* Process file names and open files. */

    if (argc - optind > 3)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 3)
    {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }
    autname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, NONE, &authd, &autfile);
    crsname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, NONE, &crshd, &crsfile);
    REQUIRE(autfile != stdin || crsfile != stdin,
	"input files must not both be standard input");
    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);
    if (debug_level)
	Fprintf(stderr, "Input files: %s, %s\nOutput file: %s\n",
	    autname, crsname, oname);

/* Get parameter values. */

    (void) read_params(param_name, SC_NOCOMMON, (char *) NULL);

    if (!fflag)
	crsfield =
	    (symtype("crsfield") != ST_UNDEF)
	    ? getsym_s("crsfield")
	    : "cross_cor";

    if (!Fflag)
	autfield =
	    (symtype("autfield") != ST_UNDEF)
	    ? getsym_s("autfield")
	    : "spec_param";

    if (!Wflag)
	pwrfield =
	    (symtype("pwrfield") != ST_UNDEF)
	    ? getsym_s("pwrfield")
	    : "raw_power";

/* Check consistency of the input files. */

    REQUIRE(get_fea_type(autfield, authd) == FLOAT,
	"autocorrelation field not of type FLOAT");
    REQUIRE(get_fea_type(pwrfield, authd) == FLOAT,
	"total_power field not of type FLOAT");
    REQUIRE(get_fea_type(crsfield, crshd) == FLOAT,
	"cross-correlation field not of type FLOAT");

    autsize =
	get_fea_siz(autfield, authd, (short *) NULL, (long **) NULL);
    pwrsize =
	get_fea_siz(pwrfield, authd, (short *) NULL, (long **) NULL);
    crssize =
	get_fea_siz(crsfield, crshd, (short *) NULL, (long **) NULL);

    REQUIRE(autsize > 0,
	"autocorrelation field not present in first input file");
    REQUIRE(pwrsize > 0,
	"total-power field not present in first input file");
    REQUIRE(crssize > 0,
	"cross-correlation field not present in second input file");
    REQUIRE(autsize + 1 == crssize,
	"autocorrelation and cross-correlation fields have different sizes");
    REQUIRE(crssize <= SHRT_MAX,
	"input fields too large");

/* Create output-file header. */

    ohd = new_header(FT_FEA);

    ohd->common.tag = YES;
    add_source_file(ohd, autname, authd);
    add_source_file(ohd, crsname, crshd);
    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);
    add_comment(ohd, get_cmd_line(argc, argv));

    fdp = (filtdesparams *) calloc(1, sizeof(filtdesparams));
    REQUIRE( fdp != NULL, "can't allocate header info.");    
    fdp->type = FILT_ARB;
    fdp->method = PZ_PLACE;
    init_feafilt_hd( ohd, (long) crssize, (long) 0, fdp);

    (void) add_genhd_c("crsfield", crsfield, 0, ohd);
    (void) add_genhd_c("autfield", autfield, 0, ohd);
    (void) add_genhd_c("pwrfield", pwrfield, 0, ohd);

    if (debug_level)
	Fprintf(stderr, "writing output header to file\n");

    write_header(ohd, ofile);

/* Allocate records. */

    autrec = allo_fea_rec(authd);
    autptr = (float *) get_fea_ptr(autrec, autfield, authd);
    pwrptr = (float *) get_fea_ptr(autrec, pwrfield, authd);

    crsrec = allo_fea_rec(crshd);
    crsptr = (float *) get_fea_ptr(crsrec, crsfield, crshd);

    orec = allo_feafilt_rec(ohd);

    *orec->num_size = crssize;
    *orec->denom_size = 0;

    autvec = malloc_d((unsigned) crssize);
    spsassert(autvec,
	"can't allocate space for autocorrelation vector");
    crsvec = malloc_d((unsigned) crssize);
    spsassert(crsvec,
	"can't allocate space for cross-correlation vector");
    ovec = malloc_d((unsigned) crssize);
    spsassert(ovec,
	"can't allocate space for output vector");

/* Main loop. */

    num_recs = 0;
    while ( (get_fea_rec(autrec, authd, autfile) != EOF)
	&& (get_fea_rec(crsrec, crshd, crsfile) != EOF) )
    {
	autvec[0] = *pwrptr;
	crsvec[0] = crsptr[0];
	for (i = 1; i < crssize; i++)
	{
	    autvec[i] = autptr[i-1]*autvec[0];
	    crsvec[i] = crsptr[i];
	}
	order = stsolv(autvec, crsvec, (int) crssize - 1, ovec);
	if (order != crssize - 1)
	{
	    if (order == -1)
		Fprintf(stderr, "%s: zero total power in record %ld.\n",
		    ProgName, num_recs);
	    else
	        Fprintf(stderr,
		    "%s: solution with reduced order %d in record %ld.\n",
		    ProgName, order, num_recs);
	}
	for (i = 0; i <= order; i++)
	    orec->re_num_coeff[i] = ovec[i];
	for ( ; i < crssize; i++)
	    orec->re_denom_coeff[i] = 0.0;


	put_feafilt_rec(orec, ohd, ofile);
	num_recs++;
    }

    exit(0);
    /*NOTREACHED*/
}
