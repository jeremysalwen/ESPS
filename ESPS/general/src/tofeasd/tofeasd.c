/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1989-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This program takes the data from a given FEA field in an input file
 * and and produces a FEA_SD output file with the input field providing
 * the data for the field "samples". 
 */

static char *sccs_id = "@(#)tofeasd.c	1.4	1/20/97	ESI/ERL";

#define VERSION "1.4"
#define DATE "1/20/97"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feasd.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX USAGE("tofeasd [-P params] [-f fea_field] [-r record_range] \n\t [-R record_freq] [-x debug_level] input output");

/* default upper and lower limits; only use 128-13 values 
   since top 13 levels of color map are used for things other than
   data values in waves+*/

#define HIGH_OUT 50
#define LOW_OUT -64.0

double          getsym_d();
char		*get_cmd_line();
void		lrange_switch();
void		copy_fea_fields();
void		copy_fea_rec();
struct feasd    *allo_feasd_recs();
void            print_fea_rec();

char	*ProgName = "tofeasd";
char	*Version = VERSION;
char	*Date = DATE;
int	debug_level = 0;	/* debug level, global for library*/

/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int	    optind;	/* for use of getopt() */
    extern char	    *optarg;	/* for use of getopt() */
    int		    ch;		/* command-line option letter */
    char	    *field_name;/* name of input field to transform */
    int		    field_type;	/* data type of input field */
    int		    rflag = NO;	/* -r option specified? */
    int             fflag = NO; /* -f option specified? */
    double          rec_freq = -1;	/* override for record frequency */
    char	    *rec_range;	/* arguments of -r option */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				    (0 means all up to end of file) */
    long	    num_read;	/* number of records actually read */

    char	    *param_name = NULL;
				/* parameter file name */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */
    char	    *f_ptr;	/* pointer (untyped) to data
				    field in input record */
    long	    size;	/* size of input data field */
    short           rank = 0;	/* rank of input data field */
    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct feasd    *orec;	/* output record */
    double          dstart_time;/* dummy start_time pointer*/
    char            *fields[2];	/* needed for debugging */

/*
 * Parse command-line options.
 */

    while ((ch = getopt(argc, argv, "f:r:x:P:R:")) != EOF)
        switch (ch)
	{
	case 'f':
	     field_name = optarg;
	     fflag++;
	     break;
	case 'r':
	    rflag = YES;
	    rec_range = optarg;
	    break;
	case 'R':
	    rec_freq = (double) atof(optarg);
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	default:
	    SYNTAX
	    break;
	}

/*
 * Process file names and open files.
 */

    if (argc - optind > 2)
    {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }

    iname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, NONE, &ihd, &ifile);

    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);

    if (debug_level)
	Fprintf(stderr, "%s: Input file: %s, Output file: %s\n",
	    ProgName, iname, oname);
/*
 * Get parameter values.
 */

    if (ifile != stdin)
	(void) read_params(param_name, SC_CHECK_FILE, iname);
    else
	(void) read_params(param_name, SC_NOCOMMON, iname);

    if (!fflag)
	field_name =
	    (symtype("field_name") != ST_UNDEF)
	    ? getsym_s("field_name")
	    : "samples";

    if (debug_level)
	Fprintf(stderr,
	    "%s: field_name is \"%s\".\n", ProgName, field_name);

    /* prepare field array for debug printout*/ 

    if (debug_level > 2) {
	      fields[0] = field_name;
	      fields[1] = NULL;
	    }

    if (rflag)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rec_range, &start_rec, &end_rec, 0);
	num_recs =
	    (end_rec != LONG_MAX)
	    ? end_rec - start_rec + 1
	    : 0;
    }
    else
    {
	start_rec =
	    (symtype("start") != ST_UNDEF)
	    ? getsym_i("start")
	    : 1;
	num_recs =
	    (symtype("nan") != ST_UNDEF)
	    ? getsym_i("nan")
	    : 0;
	end_rec = (num_recs != 0)
	    ? start_rec - 1 + num_recs
	    : LONG_MAX;
    }

    if (rec_freq < 0)    /*i.e., unless -R option*/ 
      if (symtype("record_freq") != ST_UNDEF)
	rec_freq = getsym_d("record_freq");

    if (debug_level)
	Fprintf(stderr, "%s: start_rec = %ld, end_rec = %ld, num_recs = %ld\n",
	    ProgName, start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

/*
 * Get info on input field
 */

    field_type = get_fea_type(field_name, ihd);

    REQUIRE(field_type != UNDEF, "specified field not in input file");

    size = get_fea_siz(field_name, ihd, &rank, (long **) NULL);

    REQUIRE(size != 0,
	"size == 0, bad data field definition in input file");

    irec = allo_fea_rec(ihd);

    f_ptr = get_fea_ptr(irec, field_name, ihd);

    REQUIRE(f_ptr, "NULL pointer returned from get_fea_ptr");

/*
 * Create and write output-file header
 */

    ohd = new_header(FT_FEA);

    /* no point in handling start_time and record_freq, as update_waves_gen
       will do it
     */

    dstart_time = 0.0;   /*stupid requirement*/

    if (init_feasd_hd(ohd, field_type, size, &dstart_time, NO, (double) 0.0)  
	!= 0) { 

      Fprintf(stderr, "%s: Error filling FEA_SD header\n", ProgName);
      exit(1);
    }

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));

    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec;

    update_waves_gen(ihd, ohd, (float) start_rec, (float) 1);

    if (rec_freq > 0) /*possible override*/
      *add_genhd_d("record_freq", (double *) NULL, 1, ohd) = rec_freq;

    if (debug_level)
	Fprintf(stderr, "%s: Writing output header to file.\n", ProgName);

    write_header(ohd, ofile);

    /*just hook the records up directly*/

    orec = allo_feasd_recs(ohd, field_type, (long) 1, f_ptr, NO);

/*
 * Main read-write loop
 */

    /* now read/write loop for putting out FEA_SPEC file*/

    num_read = start_rec - 1,
    fea_skiprec(ifile, num_read, ihd);

    for (   ;
	    num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	    num_read++
	)
    {
      if (debug_level > 1) {
	    Fprintf(stderr, "Record number %ld read.\n", num_read + 1);
	    if (debug_level > 2)
	      print_fea_recf(irec, ihd, stderr, fields);
	  }
      (void) put_feasd_recs(orec, 0L, 1L, ohd, ofile);
    }

    if (num_read < end_rec && num_recs != 0)
	Fprintf(stderr, "tofspec: Only %ld records read.\n", 
		num_read - start_rec + 1);
 /*
 * Write common
 */

    if (strcmp(oname, "<stdout>") != 0 && strcmp(iname, "<stdin>") != 0) {
	(void) putsym_s("filename", iname);
	(void) putsym_s("prog", ProgName);
	(void) putsym_i("start", (int) start_rec);
	(void) putsym_i("nan", (int) num_recs);
    }

    exit(0);
    /*NOTREACHED*/
}


