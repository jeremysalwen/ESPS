/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Ajaipal S. Virdy, CSPL University of Maryland, College Park.
 * Checked by:
 * Revised by:  Alan Parker for ESPS 3.0
 *              John Shore for ESPS 4.0
 *
 * Brief description:  convert headerless binary sampled data
 *              to an ESPS FEA_SD file
 *
 */

static char *sccs_id = "@(#)btosps.c	3.26	9/9/97	CSPL/ESI/ERL";
#ifndef lint
#define VERSION "3.26"
#define DATE "9/9/97"
#endif
#ifndef VERSION
#define VERSION "debug"
#endif
#ifndef DATE
#define DATE "none"
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>
#include <esps/func.h>

#define SIZE 1024
#define LINE_SIZE 256
#define COMMENT_LINE_SIZE 256

#define SYNTAX USAGE ("btosps: [-P param_file] [-f sample_freq] [-T start_time] \n\t[-n nchan] [-t data_type] [-S skip_bytes] [-F] [-E ] [-c comment] \n\t[-C comment_file] [-x debug_level] infile outfile")

extern  int optind;
extern  char *optarg;
int debug_level = 0;
char *savestring();
char *add_lf();

void get_comment();
double fabs();
char *get_cmd_line();
void lrange_switch();
void set_sd_type();
char *type_convert();

main (argc, argv)
int     argc;
char  **argv;
{
    int     c;
    int     getopt ();

    FILE    *in = stdin;
    FILE    *out = stdout;
    struct  header  *oh, *ih;

    int     i = 0;
    long     n;
    long     n_read;
    int	    fflag = 0;		/* flag for -f option (sampling rate) */
    int	    nflag = 0;		/* flag for -n option (number channels) */
    int	    Tflag = 0;		/* flag for -T option (start time) */
    int     tflag = 0;		/* flag for -t option (data type) */
    int     Sflag = 0;		/* flag for -S option (bytes to skip) */
    int     nchan = 1;		/* number of data channels */
    double  maxval = 0;		/* maximum data value  */
    double  absval;		/* absolute value of data value */
    long    total_recs=0;
    struct  feasd *sdrecs = NULL; /* FEA_SD record structure */

    char    *ifile = NULL;
    char    *ofile;
    char    *combuf;
    double  *sbuf;		/* buffer for converting data to double */
    char    *param = NULL;

    double  sf = 0;		/* sampling rate of input data */
    int	    skip = 0;		/* number of bytes to skip */
    int	    native_mode = 1;
    int     foreign_header = 0; /* if 1, then keep foreign header when
				   adding esps header */
    long    foreign_hd_ptr;
    char *ProgName = *argv;
    double start_time=0.0;		/* dummy */
    int nopipe = 1;		/* true if output not a pipe */
    int use_max = 0;		/* write max_value generic */
    int data_type = SHORT;		/* data type from -t option */
    char   *comment = NULL;
    char   *comment_file = NULL;

    while ((c = getopt (argc, argv, "f:T:n:P:t:S:x:Ec:C:F")) != EOF) {
	switch (c) {
	    case 'c':
		comment = add_lf(optarg);
		break;
	    case 'C':
	        comment_file = optarg;
		break;
	    case 'x':
		debug_level = atoi (optarg);
		break;
	    case 'f':
		sf = atof (optarg);
		fflag = 1;
		break;
	    case 'n':
		nchan = atoi (optarg);
		nflag = 1;
		break;
	    case 'T':
		start_time = atof (optarg);
		Tflag = 1;
		break;
	    case 't':
		tflag++;
		data_type = lin_search(type_codes, optarg);
		break;
	    case 'P':
		param = optarg;
		break;
	    case 'S':
		skip = atoi (optarg);
		Sflag++;
		if (skip < 0) {
			Fprintf(stderr,"s option argument must be > 0\n");
			SYNTAX;
		}
		break;
	    case 'E':
		native_mode = 0;
		break;
	    case 'F':
		if (Sflag)
			foreign_header++;
		else {
			fprintf(stderr,
			"btosps: S option must be used with F option.\n");
			SYNTAX;
		}
		break;
	    default:
		SYNTAX;
		break;
	}
    }

    if (argc - optind != 2)
	SYNTAX;

    if (comment != NULL && comment_file != NULL) {
      Fprintf(stderr, "btosps: cannot give both -C and -c options\n");
      exit(1);
    }

/* process input file
*/
    ifile = argv[optind++];
    if (strcmp (ifile, "-") == 0) {
	ifile = "<stdin>";
	in = stdin;
    }
    else
	TRYOPEN (argv[0], ifile, "r", in);

/* create output ESPS file
*/

    ofile = eopen(ProgName,argv[optind],"w",FT_FEA,FEA_SD,&oh,&out);

    if (strcmp(ofile, "<stdout>") == 0)
	nopipe = 0;

/* read parameter file
*/
    (void)read_params(param, SC_NOCOMMON, (char *)NULL);

#define MSG1 "Headerless file %s converted to ESPS FEA_SD.\n"

    combuf = (char *)malloc(strlen(ifile)+strlen(MSG1)+1);
    (void) sprintf (combuf, MSG1, ifile);

    if (!fflag)
      sf = (symtype("sampling_rate") != ST_UNDEF)
	   ? getsym_d("sampling_rate")
	   : 8000;

    if (!Tflag)
      start_time = (symtype("start_time") != ST_UNDEF)
	   ? getsym_d("start_time")
	   : 0.0;

    if (!nflag)
      nchan = (symtype("nchan") != ST_UNDEF)
	   ? getsym_i("nchan")
	   : 1;

    if (!tflag)
      data_type = (symtype("data_type") != ST_UNDEF)
	          ? lin_search(type_codes, getsym_s("data_type"))
		    : SHORT;

    if (data_type == -1) {
      Fprintf(stderr, "btosps: unknown data type specified for output\n");
      exit(1);
    }

    if (!Sflag)
      skip = (symtype("skip_bytes") != ST_UNDEF)
	   ? getsym_i("skip_bytes")
	   : 0;

    if (!(comment || comment_file) && symtype ("comment") == ST_STRING)
      comment = getsym_s ("comment");

    if (debug_level) {
      Fprintf(stderr, "btosps: assumptions are type = %s\n",
	      type_codes[data_type]);
      Fprintf(stderr, "\tnchan = %d\n", nchan);
      Fprintf(stderr, "\tstart_time = %g\n", start_time);
      Fprintf(stderr, "\tsampling rate = %g\n", sf);
      Fprintf(stderr, "\tbytes to skip = %d\n", skip);
    }

    oh = new_header(FT_FEA);
    (void) strcpy (oh -> common.prog, ProgName);
    (void) strcpy (oh -> common.vers, VERSION);
    (void) strcpy (oh -> common.progdate, DATE);
    oh -> common.tag = NO;

    (void) init_feasd_hd(oh, data_type, nchan,
		       &start_time, (int) 0, sf);

    (void) add_comment (oh, get_cmd_line(argc,argv));

    (void) add_comment (oh, combuf);

    if (comment_file)
	get_comment(comment_file, oh);
    else if (comment)
	add_comment (oh, comment);
    else if (strcmp("<stdin>",ifile) != 0)
	get_comment("<stdin>", oh);
    else {
      Fprintf(stderr,
	      "btosps: you must give a comment on the command line or in the parameter file\n");
      exit(1);
    }

    *add_genhd_d("record_freq", (double *) NULL, 1, oh) = sf;

    /* add max_value to header if it will be filled in later*/

    if (nopipe) {
      *add_genhd_d("max_value", (double *) NULL, 1, oh) = 0;
      use_max = 1;
    }

/*
  if foreign_header is defined, then read the foreign_header size
  given by the skip option into a buffer and then save a pointer
  to it as the generic foreign_hd_ptr
*/

    if(foreign_header) {
    	*add_genhd_l("foreign_hd_length", (long *)NULL, 1, oh) = skip;
#if defined(IBM_RS6000) || defined(DS3100) || defined(SG) || defined(HP400) || defined(M5600) || defined(OS5) || defined(LINUX)
	foreign_hd_ptr = malloc((unsigned)skip);
#else
	(char *)foreign_hd_ptr = malloc((unsigned)skip);
#endif
	spsassert(foreign_hd_ptr, "btosps: malloc failed");
	*add_genhd_l("foreign_hd_ptr", (long *)NULL, 1, oh) =
			(long)foreign_hd_ptr;
	if(!fread((char *)foreign_hd_ptr,1,skip,in)) {
	  fprintf(stderr,"btosps: cannot read foreign header!\n");
	  exit(1);
	}
    }

    (void) write_header (oh, out);
    ih = copy_header(oh);
    ih->common.edr = !native_mode;

    sdrecs = allo_feasd_recs(ih, data_type, (long) SIZE, (char *) NULL, NO);

    if (skip && !foreign_header) {
      if (debug_level) {
	Fprintf(stderr, "btosps: skipping %d bytes\n", skip);
      }
    skiprec(in, (long)skip, 1);
    }

/* Read the next block of n samples. If fewer
   than n remain to be read, then read the smaller number
*/

    n = SIZE;
    sbuf = (double *) malloc(SIZE*nchan*sizeof(double));
    while ((n_read = get_feasd_recs(sdrecs, 0L, n, ih, in)) == n) {
      if (debug_level > 1)
	Fprintf(stderr, "btosps: read %ld records \n", n_read);
      if (use_max) {
	(void) arr_func(FN_ABS, n*nchan, sdrecs->data, data_type,
			(char *) sbuf, DOUBLE);
	for (i=0; i< n*nchan; i++) {
	  if(sbuf[i] > maxval)
	    maxval = sbuf[i];
	}
      }
      (void) put_feasd_recs (sdrecs, 0L, n, oh, out);
      total_recs += n;
    }

    if (debug_level > 1)
      Fprintf(stderr, "btosps: read %ld records \n", n_read);

    if (use_max) {
      (void) arr_func(FN_ABS, n_read*nchan, sdrecs->data, data_type,
		      (char *) sbuf, DOUBLE);
      for (i=0; i< n_read*nchan; i++) {
	if(sbuf[i] > maxval)
	  maxval = sbuf[i];
      }
    }

    if (n_read)
	(void) put_feasd_recs (sdrecs, 0L, n_read, oh, out);

    total_recs += n_read;

/* if the output is not a pipe, then rewind and write the header
   again to get max_value -- and also write common.
*/


    if (use_max) {
      *add_genhd_d("max_value", (double *) NULL, 1, oh) = maxval;
      (void) rewind(out);
      (void) write_header (oh, out);
    }
    if (nopipe) {
      (void) putsym_s("filename",ofile);
      (void) putsym_s("prog",argv[0]);
      (void) putsym_i("start",1);
      (void) putsym_i("nan",(int)total_recs);
    }

    return 0;
}

void
get_comment (file, hd)
char   *file;
struct header  *hd;
{
    FILE * strm;
    char   *buffer = malloc ((unsigned) COMMENT_LINE_SIZE);

    spsassert (buffer, "btosps: malloc failed!");
    if (strcmp ("<stdin>", file) == 0) {
	Fprintf (stderr,
	 "btosps: Please enter comment, ended with blank line:\n");
	strm = stdin;
    }
    else
    if ((strm = fopen (file, "r")) == NULL) {
      Fprintf(stderr, "btosps: can't open comment file %s\n", file);
      exit(1);
    }

    while (fgets (buffer, COMMENT_LINE_SIZE, strm) != NULL) {
	if (strm == stdin && strlen (buffer) == 1)
	    return;
	add_comment (hd, buffer);
    }
    free (buffer);
}

char *
add_lf(line)
char *line;			/* line to add a line feed to */
{
  char *newline = malloc((unsigned) strlen(line) + 2);
  return(savestring(strcat(strcpy(newline, line),"\n")));
}
