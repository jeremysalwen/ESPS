/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1988-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore (based on Rod Johnson's "cross_cor.c")
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This program takes a single channel FEA_SD file, reads frames  	
 * (possibly overlapping), applies an optional window to the data,      
 * and puts out FEA file records containing one frame of sampled data   
 * per record.                                                          
 */

static char *sccs_id = "@(#)frame.c	1.11	1/22/97	ESI/ERL";

#define VERSION "1.11"
#define DATE "1/22/97"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/fea.h>
#include <esps/window.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("frame [-f sd_field_name] [-l frame_len] [-p range] [-w window_type]\n \
 [-x debug_level] [-P param] [-S step] input.sd output.fea") ;

#define WT_PREFIX "WT_"

void	lrange_switch();
int	get_sd_orecd();
char	*get_cmd_line();
double  get_genhd_val();

long	n_rec();
void	pr_farray();

char	*ProgName = "frame";
char	*Version = VERSION;
char	*Date = DATE;
char	sbuf[256];		/* to hold comment */
int     debug_level = 0;		/* debug level; global for library*/

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
    long    frame_len;		/* length of each frame */
    int	    pflag = 0;		/* -p option specified? */
    int	    lflag = 0;		/* -l option specified? */
    long    step;		/* shift between successive frame positions */
    int	    Sflag = 0;		/* -S option specified? */
    char    *prange;		/* arguments of -p option */
    long    start;		/* starting points in the input files */
    long    nan;		/* total number of samples to analyze */
    long    last;		/* end of range */
    char    *window_type;	/* name of type of window to apply to data */
    char    *pref_w_type;	/* window type name with added prefix */
    int	    wflag = 0;		/* -w option specified? */
    int	    win;		/* window type code */
    extern char
	    *window_types[];    /* standard window type names */
    char    *param_name = NULL;
				/* parameter file name */
    char    *iname;		/* input file name */
    FILE    *ifile;		/* input stream */
    struct header
	    *ihd;		/* input file header */
    float   *x;			/* sampled data from the input file */
    char    *oname;		/* output file name */
    FILE    *ofile;		/* output stream */
    struct header
	    *ohd;		/* output file header */
    struct fea_data
	    *fea_rec;		/* output record */
    char    *sd_name;		/* name for output sd field */
    int	    fflag = 0;		/* flag for -f option */
    long    n_frames;		/* number of frames to process */
    int	    more = 1;		/* flag to say more frames out there */
    int	    first;		/* flag for initial call of get_sd_orecd() */
    float   *sd;		/* pointer to sd field in FEA record */
				/* also used as destination of windowing */
    long    k;			/* loop index */

/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "f:l:p:r:w:x:P:S:")) != EOF)
        switch (ch)
	{
	case 'f':
	    sd_name = optarg;
	    fflag++;
	    break;
	case 'l':
	    frame_len = atol(optarg);
	    lflag++;
	    break;
	case 'p':
	case 'r':
	    prange = optarg;
	    pflag++;
	    break;
	case 'w':
	    window_type = optarg;
	    wflag++;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	case 'S':
	    step = atol(optarg);
	    Sflag++;
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
    iname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, FEA_SD, &ihd, &ifile);
    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);
    if (debug_level)
	Fprintf(stderr, "Input file: %s\nOutput file: %s\n",
	    iname, oname);

    REQUIRE(get_fea_siz("samples", ihd, (short) NULL, (short) NULL) == 1,
	    "sorry, can't deal with multi-channel files");

    REQUIRE(!is_field_complex(ihd, "samples"), 
	    "sorry, can't deal with complex data"); 

/* Get parameter values. */

    if (ifile != stdin)
	(void) read_params(param_name, SC_CHECK_FILE, iname);
    else
        (void) read_params(param_name, SC_NOCOMMON, iname);

    if (!lflag)
	frame_len =
	    (symtype("frame_len") != ST_UNDEF)
	    ? getsym_i("frame_len")
	    : 0;

    if (!fflag)
	sd_name =
	    (symtype("sd_field_name") != ST_UNDEF)
	    ? getsym_s("sd_field_name")
	    : "sd";
  
    if (debug_level)
	Fprintf(stderr, "frame_len: %ld\n", frame_len);

    if (pflag) {
	start = 1;
	last = LONG_MAX;
	lrange_switch(prange, &start, &last, 0);
	REQUIRE(start >= 1,
	    "can't start before beginning of input file");
	REQUIRE(last >= start,
	    "empty range specified for input file");
    }
    else {
	start =
	    (symtype("start") != ST_UNDEF)
	    ? getsym_i("start")
	    : 1;

	if (symtype("nan") != ST_UNDEF && getsym_i("nan") != 0)
	    last = start + getsym_i("nan") - 1;
	else 
	    last = LONG_MAX;
    }

    /* don't call n_rec unless we have to (cause we may be on a pipe)*/

    if (frame_len == 0) {
        if (last == LONG_MAX)
	  last = n_rec(&ifile, &ihd);
	frame_len = nan = last - start + 1;
	if (debug_level) 
	    Fprintf(stderr, "%s: frame_len changed to %ld\n",
		ProgName, frame_len);
    }
    else
	nan = last - start + 1;

    if (debug_level) {
	Fprintf(stderr, "start: %ld\n", start);
	Fprintf(stderr, "nan: %ld\n", nan);
    }

    if (!wflag)
	window_type =
	    (symtype("window_type") != ST_UNDEF)
	    ? getsym_s("window_type")
	    : "RECT";

    pref_w_type =
	malloc((unsigned)(strlen(WT_PREFIX) + strlen(window_type) + 1));
    REQUIRE(pref_w_type, "can't allocate space for window type name");
    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);
    win = lin_search(window_types, pref_w_type);
    if (win <= 0) {
      fprintf(stderr, "frame: window type not recognized, using RECT");
      win = WT_RECT;
    }

    if (debug_level)
	Fprintf(stderr, "window_type: %s\nwin: %d\n",
	    window_type, win);

    if (!Sflag)
	step =
	    (symtype("step") != ST_UNDEF && getsym_i("step") != 0)
	    ? getsym_i("step")
	    : frame_len;

    if (debug_level)
	Fprintf(stderr, "step: %ld\n", step);

/* Create output-file header */

    ohd = new_header(FT_FEA);
    add_source_file(ohd, iname, ihd);
    ohd->common.tag = YES;
    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);
    ohd->variable.refer = iname;
    add_comment(ohd, get_cmd_line(argc, argv));

    *add_genhd_f("src_sf", (float *) NULL, 1, ohd) = 
      get_genhd_val("record_freq", ihd, 0);
    *add_genhd_l("frmlen", (long *) NULL, 1, ohd) = frame_len;
    *add_genhd_l("nan", (long *) NULL, 1, ohd) = nan;
    *add_genhd_l("start", (long *) NULL, 1, ohd) = start;
    *add_genhd_l("step", (long *) NULL, 1, ohd) = step;
    if (win != WT_RECT)
	*add_genhd_e("window_type", (short *) NULL, 1, 
				    window_types, ohd) = win;

    REQUIRE( add_fea_fld(sd_name, frame_len, (short) 1, (long *) NULL,
			    FLOAT, (char **) NULL, ohd) != -1,
		"can't create sd_field_name field in output file header" );

    update_waves_gen(ihd, ohd, (float) start, (float) step);

    if (debug_level)
	Fprintf(stderr, "writing output header to file\n");

    write_header(ohd, ofile);

/* Allocate buffer and set up output record. */

    x = (float *) calloc((unsigned) frame_len, sizeof(float));
    REQUIRE( x != NULL, "can't allocate memory for input frame");

    fea_rec = allo_fea_rec(ohd);
    sd = (float *) get_fea_ptr(fea_rec, sd_name, ohd);

/* Main read-write loop */

    n_frames =
	(nan == 0) ? 0
	: (nan <= frame_len) ? 1
	: 2 + (nan - frame_len - 1) / step;

    if (n_frames > 0 && (n_frames - 1) * step + frame_len > nan)
	Fprintf(stderr,
	    "%s: WARNING - last frame will exceed specified range.\n",
	    ProgName);

    if (debug_level)
	Fprintf(stderr, "n_frames: %ld\n", n_frames);

    fea_skiprec(ifile, start - 1, ihd);
    first = 1;
    fea_rec->tag = start;

    for (k = 0; k < n_frames && more; k++) 
    {
	long	actsize;	/* actual frame length */

	if (debug_level) 
	    Fprintf(stderr, "%s: frame %ld at tag %ld\n", ProgName, k + 1, 
		fea_rec->tag);

	if (win == WT_RECT) /*avoid intermediate buffer*/
	    actsize = get_sd_orecf(sd,
			(int) frame_len, (int) step, first, ihd, ifile);
	else /*read into windowing buffer*/
	    actsize = get_sd_orecf(x,
			(int) frame_len, (int) step, first, ihd, ifile);
	first = 0;
	if (actsize == 0) break;
	more = (actsize == frame_len);

	if (actsize < frame_len) {
	    Fprintf(stderr, 
		"%s: WARNING - only %ld points in frame %ld at tag %ld (zero filled)\n",
		ProgName, actsize, k + 1, fea_rec->tag);
	}
	if (debug_level >= 2) {
	    if (win == WT_RECT) 
		pr_farray("frame from input.sd", actsize, sd);
	    else
		pr_farray("frame from input.sd", actsize, x);
	}
	/* Window data.  (Not in place, since x must be preserved
	   for get_sd_orec.) We just put it directly in the FEA record;
	   if no windowing, data was already in FEA record*/

	if (win != WT_RECT) 
	    (void) window(frame_len, x, sd, win, (double *) NULL);
	if (debug_level >= 2)
	    pr_farray("windowed frame from input.sd", frame_len, sd);
	
	put_fea_rec(fea_rec, ohd, ofile);
	fea_rec->tag += step;
    }

    exit(0);
    /*NOTREACHED*/
}    


/*
 * For debug printout of float arrays
 */

void pr_farray(text, n, arr)
    char    *text;
    long    n;
    float  *arr;
{
    int	    i;

    Fprintf(stderr, "%s -- %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}

#define BUFSIZE 1000

long
n_rec(file, hd)
    FILE **file;
    struct header **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe or has
				     * variable record length. */
    {
	FILE	*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	static double
		buf[BUFSIZE];
	int	num_read;
	long	ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);

	do
	{
	    num_read = get_sd_recd(buf, BUFSIZE, *hd, *file);
	    if (num_read != 0) put_sd_recd(buf, num_read, tmphdr, tmpstrm);
	    ndrec += num_read;
	} while (num_read == BUFSIZE);
	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}

