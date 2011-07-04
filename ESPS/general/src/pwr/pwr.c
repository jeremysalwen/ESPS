/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988,1989,1990 Entropic Speech, Inc.                 |
|    All rights reserved."	                                        |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: pwr.c							|
|									|
|  This program computes the power in sampled-data records in a FEA	|
|  file.  
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|  John Shore, modified for -l option, modified for FEA_SD              |
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)pwr.c	1.11	8/31/95	ESI";
#endif
#define VERSION "1.11"
#define DATE "8/31/95"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("pwr [-l] [-f sd_field [-f pwr_field]] [-r range] [-x debug_level]\n \
 [-P params] input.fea output.fea") \
 ;

char		*get_cmd_line();
void		lrange_switch();
void		copy_fea_rec();
void            set_seg_lab();
double          get_genhd_val();
double          log10();
char            *type_convert();

char	*ProgName = "pwr";
char	*Version = VERSION;
char	*Date = DATE;
int	debug_level = 0;	/* debug level, global for library*/

/*array of field names to copy over from input to output (these are for 
 *copying over the segment labelling*/

char   *seg_fields[] = {"source_file", "segment_start", "segment_length", NULL};

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

    int		    fflag = 0;	/* -f option specified 0, 1, or 2 times? */
    int		    lflag = 0;	/* flag for -l (log power) option */
    char            *pow_funct = "none";
				/* name of power function */
    char	    *sd_name;	/* field name for input sampled data */
    int		    sd_type;	/* data type of sampled-data field */
    double 	    sf;		/* sampling frequency of original data*/
    char	    *pwr_name;	/* field name for output power */

    int		    rflag = NO;	/* -r option specified? */
    char	    *rrange;	/* arguments of -r option */
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
    char	    *sd_ptr;	/* pointer (untyped) to sampled-data
				    field in input record */
    long	    *frame_len;	/* pointer to frame size
				    in input header or record */
    long	    size;	/* size of input sampled-data field */
    double	    *data;	/* input data as doubles */

    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    double	    *power;	/* pointer to power field
				    in output record */
    int		    i;		/* loop index */
	/* don't try log10 for values below this */
    double          log10lim = 5/DBL_MAX;
    double          start_time; /*used in calculation of start_time offset*/
    short           tagged=0;	/* flag to indicate input is tagged */
    short           seglab=0;	/* to indicate segmenta labelled input */
    short           file_ind;	/* index for location of source_file field */
    int		    dsize;	/* dummy variable for genhd_type call */
    int      have_src_sf = 1;	/* is src_sf an input generic? */
/*
 * parse command-line options.
 */

    while ((ch = getopt(argc, argv, "lf:r:x:P:")) != EOF)
        switch (ch)
	{
	case 'f':
	    switch (fflag)
    	    {
	    case 0:
		sd_name = optarg;
		fflag++;
		break;
	    case 1:
		pwr_name = optarg;
		fflag++;
		break;
	    default:		
		Fprintf(stderr, 
		    "%s: -f option may be specified at most twice.\n",
		    ProgName);
		exit(1);
	    }
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'l':
	    lflag++;
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

    if (fflag < 1)
	sd_name =
	    (symtype("sd_field_name") != ST_UNDEF)
	    ? getsym_s("sd_field_name")
	    : "sd";

    if (fflag < 2)
	pwr_name =
	    (symtype("power_field_name") != ST_UNDEF)
	    ? getsym_s("power_field_name")
	    : (lflag ? "log_power" : "raw_power");

    if (debug_level)
	Fprintf(stderr,
	    "%s: sd_field_name = \"%s\".  power_field_name = \"%s\".\n",
	    ProgName, sd_name, pwr_name);

    sd_type = get_fea_type(sd_name, ihd);
    REQUIRE(sd_type != UNDEF, "sampled-data field not in input file");

    REQUIRE(!is_field_complex(ihd, sd_name), 
	    "sorry, can't deal with complex data"); 

    if (rflag)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rrange, &start_rec, &end_rec, 0);
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

    if (debug_level)
	Fprintf(stderr,"%s: start_rec = %ld, end_rec =  %ld,  num_recs = %ld,\n",
	    ProgName, start_rec, end_rec, num_recs);

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

     if (!lflag) {
       if (symtype("power_function") != ST_UNDEF) {
	 pow_funct = getsym_s("power_function");
	 if (strcmp(pow_funct, "log") == 0) {
	   lflag++;
	 }
	 else 
	   if (strcmp(pow_funct,"none") != 0) {
	     Fprintf(stderr, 
		     "%s: don't recognize power_function, will use none.\n",
		     ProgName);
	   }
       }
     }

/*
 * Create Output file header
 */

    ohd = new_header(FT_FEA);
    if (ihd->common.tag == YES) {
        tagged++;
	ohd->common.tag = YES;
	ohd->variable.refer = ihd->variable.refer;
    }

    ohd->variable.refhd = ihd->variable.refhd;

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    add_source_file(ohd, iname, ihd);
    add_comment(ohd, get_cmd_line(argc, argv));

    (void) copy_genhd(ohd, ihd, "src_sf");

    if (genhd_type("src_sf", &dsize, ihd) == HD_UNDEF) {
      Fprintf(stderr, "%s: WARNING - can't find src_sf in input header\n", 
	      ProgName);
      Fprintf(stderr, 
	      "%s: start_time will not be displaced by 1/2 the frmlen\n", 
	      ProgName);

      add_comment(ohd, 
	      "pwr: src_sf unknown, start_time not displaced\n");
      have_src_sf = 0;
    }
    else
      sf = get_genhd_val("src_sf", ihd, 1.0);

    REQUIRE(	add_fea_fld(pwr_name, 1L, 0, (long *) NULL,
			    DOUBLE, (char **) NULL, ohd) != -1,
		"can't create power field in output file header" );

    *add_genhd_l("nan", (long *) NULL, 1, ohd) = num_recs;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start_rec;

/*
 * Allocate records and set up pointers
 */

    irec = allo_fea_rec(ihd);

    sd_ptr = get_fea_ptr(irec, sd_name, ihd);
    REQUIRE(sd_ptr, "can't locate sampled-data field in input record");

    size = get_fea_siz(sd_name, ihd, (short *) NULL, (long **) NULL);
    REQUIRE(size != -1,
	"bad sampled-data field definition in input file");

    data = calloc_d((unsigned) size);
    REQUIRE(data,
	"can't allocate space for array of data as doubles");

    if (ihd->hd.fea->segment_labeled)
    {
	frame_len = (long *) get_fea_ptr(irec, "segment_length", ihd);
	REQUIRE(frame_len, "no segment_length field in input file header");

	if (debug_level)
	    Fprintf(stderr, "%s: Input file segment_labeled.\n", ProgName);
    }
    else
    {
	frame_len = &size;

	if (debug_level)
	    Fprintf(stderr,
		"%s: Input file not segment_labeled.  Frame length: %ld.\n",
		ProgName, *frame_len);
    }

    /*revise waves generics, deal with segment labelling, and write header*/

    update_waves_gen(ihd, ohd, (float) start_rec, 1.0);

    /*offset the start_time to the middle of the frame size processed
     *so that waves displays come out right; we do this even for segment
     *labelled files even though the displays would be screwy*/

    start_time = get_genhd_val("start_time", ohd, 0.0); 

    if (have_src_sf) 
      *add_genhd_d ("start_time", (double *) NULL, 1, ohd) = 
	start_time + size / (2 * sf);

    /*frmlen is size of sd field, even if segment labelled*/

    (void) add_genhd_l("frmlen", &size, 1, ohd);

    if (ihd->hd.fea->segment_labeled) {
      if (debug_level) 
	Fprintf(stderr, "%s: input is segment labelled, make output same\n",
	      ProgName);

      file_ind = lin_search2(ihd->hd.fea->names,"source_file");

      set_seg_lab(ohd, ihd->hd.fea->enums[file_ind]);

      seglab = 1;
    }

    if (debug_level)
	Fprintf(stderr, "%s: Writing output header to file.\n", ProgName);
    
    write_header(ohd, ofile);

    orec = allo_fea_rec(ohd);

    power = (double *) get_fea_ptr(orec, pwr_name, ohd);
    REQUIRE(power, "can't locate power field in output record");

/*
 * Main read-write loop
 */

    num_read = start_rec - 1,
    fea_skiprec(ifile, num_read, ihd);

    for (   ;
	    num_read < end_rec && get_fea_rec(irec, ihd, ifile) != EOF;
	    num_read++
	)
    {
	if (debug_level > 1)
	    Fprintf(stderr, "%s: Record number %ld read.\n",
		    ProgName, num_read + 1);

	if (tagged)
	  orec->tag = irec->tag;
	else if (seglab)
	  copy_fea_rec(irec, ihd, orec, ohd, seg_fields, (short **) NULL);

	(void) type_convert(*frame_len, (char *) sd_ptr, sd_type, 
			    (char *) data, DOUBLE, (void (*)())NULL);

	*power = 0;
	for (i = 0; i < *frame_len; i++) *power += data[i]*data[i];

	*power /= *frame_len;

	if (debug_level > 1) 
	  Fprintf(stderr, "%s, power value = %g\n", ProgName, *power);

	if (lflag) {
	  if (*power < log10lim) {
	    *power = log10(log10lim);
	    if (debug_level) 
	      Fprintf(stderr, "%s: limited log10 for low power value\n",
		      ProgName);
	  }
	  else
	    *power = log10(*power);
	}
	
	if (debug_level > 1) 
	  Fprintf(stderr, "%s: value stored = %g\n", ProgName, *power);

	put_fea_rec(orec, ohd, ofile);
    }

    if (num_read < end_rec && num_recs != 0)
	Fprintf(stderr, "Only %ld records read.\n", num_read - start_rec + 1);

/*
 * Write common
 */

    if (ofile != stdout)
    {
	REQUIRE(putsym_i("start", (int) start_rec) != -1,
		"can't write \"start\" to Common");
	REQUIRE(putsym_i("nan", (int) num_recs) != -1,
		"can't write \"nan\" to Common");
	REQUIRE(putsym_s("prog", ProgName) != -1,
		"can't write \"prog\" to Common");
	REQUIRE(putsym_s("filename", iname) != -1,
		"can't write \"filename\" to Common");
    }

    exit(0);
    /*NOTREACHED*/
}


