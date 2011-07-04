/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * This program computes cross correlations between two sampled-data
 * files and writes the results in a FEA file.
 *
 */

static char *sccs_id = "@(#)cross_cor.c	1.16	1/21/97	ESI/ERL";

#define VERSION "1.16"
#define DATE "1/21/97"

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/fea.h>
#include <esps/window.h>

#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num), sizeof(type))) == NULL) \
    {Fprintf(stderr, "%s: can't allocate memory--%s", ProgName, (msg)); \
    exit(1);}}

#define REQUIRE(test, text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("cross_cor [-l frame_len][-o range][-{pr} range][-w window_type]\n [-x debug_level][-P param][-S step] [-N] input1.sd input2.sd output.fea") \
 ;
/* Delete ; when esps.h fixed */

#define WT_PREFIX "WT_"

void	lrange_switch();
int	get_sd_orecd();
char	*get_cmd_line();

void	d_cross_cor();
void	pr_darray();
long	n_rec();

char	*ProgName = "cross_cor";
char	*Version = VERSION;
char	*Date = DATE;

int debug_level = 0;

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
    int	    lflag = 0;		/* -l option specified? */
    long    step;		/* shift between successive frame positions */
    int	    Sflag = 0;		/* -S option specified? */
    long    minlag, maxlag;	/* least and greatest lags for cross-cor */
    char    *orange;		/* arguments of -o option */
    int	    oflag = 0;		/* -o option specified? */
    char    *prange[2];		/* arguments of -p option */
    int	    pnum = 0;		/* number of -p options */
    long    start1, start2;	/* starting points in the input files */
    long    nan;		/* total number of samples to analyze */
    long    last;		/* end of range */
    long    nan1, nan2;		/* tentative values for nan */
    char    *window_type;	/* name of type of window to apply to data */
    char    *pref_w_type;	/* window type name with added prefix */
    int	    wflag = 0;		/* -w option specified? */
    int	    win;		/* window type code */
    extern char
	    *window_types[];    /* standard window type names */
    int	    debug = 0;		/* debug level */
    char    *param_name = NULL;
				/* parameter file name */
    char    *iname1, *iname2;	/* input file names */
    FILE    *ifile1, *ifile2;	/* input streams */
    struct header
	    *ihd1, *ihd2;	/* input file headers */
    double  *x, *y;		/* sampled data from the two input files */
    double  *wx, *wy;		/* windowed sampled data */
    char    *oname;		/* output file name */
    FILE    *ofile;		/* output stream */
    struct header
	    *ohd;		/* output file header */
    struct fea_data
	    *fea_rec;		/* output record */
    long    *tag1, *tag2;	/* pointers to fields in output record */
    float   *c_cor;		/* pointer to field in output record */
    double  *d_c_cor;		/* computed cross correlation */
    long    n_frames;		/* number of frames to process */
    int	    first;		/* flag for initial call of get_sd_orecd() */
    int     nsrflag = 0;        /* no shift reference flag */

/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "l:o:p:r:w:x:P:S:N")) != EOF)
        switch (ch)
	{
	case 'l':
	    frame_len = atol(optarg);
	    lflag++;
	    break;
	case 'o':
	    orange = optarg;
	    oflag++;
	    break;
	case 'p':
	case 'r':
	    REQUIRE(pnum < 2, "at most two -p or -r options allowed");
	    prange[pnum++] = optarg;
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
        case 'N':
	    nsrflag++;
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
    iname1 = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, FEA_SD, &ihd1, &ifile1);
    iname2 = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, FEA_SD, &ihd2, &ifile2);
    REQUIRE(ifile1 != stdin || ifile2 != stdin,
	"input files must not both be standard input");
    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);
    if (debug_level)
	Fprintf(stderr, "Input files: %s, %s\nOutput file: %s\n",
	    iname1, iname2, oname);

/* Are input files single-channel and real? */

    if (get_fea_siz("samples", ihd1, (short *) NULL, (long **) NULL) != 1
        || get_fea_siz("samples", ihd2, (short *) NULL, (long **) NULL) != 1)
    {
	Fprintf(stderr, "%s: Multichannel data not supported yet.\n",
		ProgName);
	exit(1);
    }

    if (is_field_complex(ihd1, "samples") || is_field_complex(ihd2, "samples"))
    {
	Fprintf(stderr, "%s: Complex data not supported yet.\n",
		ProgName);
	exit(1);
    }

/* Get parameter values. */

    (void) read_params(param_name, SC_NOCOMMON, (char *) NULL);

    if (!lflag)
	frame_len =
	    (symtype("frame_len") != ST_UNDEF)
	    ? getsym_i("frame_len")
	    : 0;

    if (debug_level)
	Fprintf(stderr, "frame_len: %ld\n", frame_len);

    if (oflag)
    {
	minlag = -10; maxlag = 10;
	lrange_switch(orange, &minlag, &maxlag, 0);
    }
    else
    {
	minlag =
	    (symtype("minlag") != ST_UNDEF)
	    ? getsym_i("minlag")
	    : -10;
	maxlag =
	    (symtype("maxlag") != ST_UNDEF)
	    ? getsym_i("maxlag")
	    : 10;
    }

    if (debug_level)
	Fprintf(stderr, "minlag: %ld;  maxlag: %ld\n", minlag, maxlag);

    REQUIRE(minlag <= maxlag, "maxlag less than minlag.");

    if (debug_level)
    {
	int i;

	Fprintf(stderr, "%d -p options.\n", pnum);
	for (i = 0; i < pnum; i++)
	    Fprintf(stderr, "  prange[%d]: %s\n", i, prange[i]);
    }

    switch (pnum)
    {
    case 0:
	start1 =
	    (symtype("start1") != ST_UNDEF)
	    ? getsym_i("start1")
	    : 1;
	start2 =
	    (symtype("start2") != ST_UNDEF)
	    ? getsym_i("start2")
	    : 1;
	if (symtype("nan") != ST_UNDEF && getsym_i("nan") != 0)
	    nan1 = nan2 = getsym_i("nan");
	else
	{
	    nan1 = n_rec(&ifile1, &ihd1) - start1 + 1;
	    nan2 = n_rec(&ifile2, &ihd2) - start2 + 1;
	}
	break;
    case 1:
	prange[1] = prange[0];
	/* FALL THROUGH */
    case 2:
	start1 = 1;
	last = LONG_MAX;
	lrange_switch(prange[0], &start1, &last, 0);
	REQUIRE(start1 >= 1,
	    "can't start before beginning of first input file");
	REQUIRE(last >= start1,
	    "empty range specified for first input file");
	nan1 = ((last == LONG_MAX) ? n_rec(&ifile1, &ihd1) : last)
		- start1 + 1;

	start2 = 1;
	last = LONG_MAX;
	lrange_switch(prange[1], &start2, &last, 0);
	REQUIRE(start2 >= 1,
	    "can't start before beginning of second input file");
	REQUIRE(last >= start2,
	    "empty range specified for second input file");
	nan2 = ((last == LONG_MAX) ? n_rec(&ifile2, &ihd2) : last)
		- start2 + 1;

	break;
    default:
	Fprintf(stderr, "%s: at most two -p options allowed", ProgName);
	exit(1);
	break;
    }
    nan = (nan1 < nan2 || nsrflag) ? nan1 : nan2;

    if (debug_level)
    {
	Fprintf(stderr, "start1: %ld\nstart2: %ld\n", start1, start2);
	Fprintf(stderr, "nan1: %ld;  nan2: %ld\n", nan1, nan2);
	Fprintf(stderr, "nan: %ld\n", nan);
	if (frame_len == 0)
	    Fprintf(stderr, "frame_len changed to %ld\n", nsrflag?nan2:nan);
    }

    REQUIRE(frame_len >= 0, "Negative frame length.");

    if (frame_len == 0) frame_len = nan;

    if (nsrflag) {
	frame_len = nan2;
	REQUIRE( nan1>=nan2, "test file cannot have less records than reference.");

	maxlag = minlag = 0;
	if (debug_level)
	    Fprintf(stderr, "maxlag and minlag set to 0\n");
    }

    if (minlag <= -frame_len)
	Fprintf(stderr,
	    "%s: WARNING - first lag (%ld) is -frame_len or less.\n",
	    ProgName, minlag);
    
    if (maxlag >= frame_len)
	Fprintf(stderr,
	    "%s: WARNING - last lag (%ld) is frame_len or greater.\n",
	    ProgName, maxlag);
    
    if (!wflag)
	window_type =
	    (symtype("window_type") != ST_UNDEF)
	    ? getsym_s("window_type")
	    : "RECT";

    TRYALLOC(char, strlen(WT_PREFIX) + strlen(window_type) + 1,
	     pref_w_type, "window type name");

    (void) strcpy(pref_w_type, WT_PREFIX);
    (void) strcat(pref_w_type, window_type);

    win = lin_search2(window_types, pref_w_type);

    REQUIRE(win > -1, "window type not recognized");

    if (debug_level)
	Fprintf(stderr, "window_type: %s\nwin: %d\n",
	    window_type, win);

    if (!Sflag)
	step = (symtype("step") != ST_UNDEF && getsym_i("step") != 0) ? 
	    getsym_i("step") : nsrflag ? 1 : frame_len;

    if (debug_level)
	Fprintf(stderr, "step: %ld\n", step);

    REQUIRE(step > 0, "Step size must be positive.");

/* Create output-file header */

    ohd = new_header(FT_FEA);
    add_source_file(ohd, iname1, ihd1);
    add_source_file(ohd, iname2, ihd2);
    ohd->common.tag = NO;
    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);
    ohd->variable.refer = NULL;
    add_comment(ohd, get_cmd_line(argc, argv));
    *add_genhd_l("frame_len", (long *) NULL, 1, ohd) = frame_len;
    *add_genhd_l("nan", (long *) NULL, 1, ohd) = nan;
    *add_genhd_l("minlag", (long *) NULL, 1, ohd) = minlag;
    *add_genhd_l("maxlag", (long *) NULL, 1, ohd) = maxlag;
    *add_genhd_l("start1", (long *) NULL, 1, ohd) = start1;
    *add_genhd_l("start2", (long *) NULL, 1, ohd) = start2;
    *add_genhd_l("step", (long *) NULL, 1, ohd) = step;
    *add_genhd_e("window_type", (short *) NULL, 1, window_types, ohd) = win;
    (void) add_fea_fld("tag1", 1L,
		0, (long *) NULL, LONG, (char **) NULL, ohd);
    (void) add_fea_fld("tag2", 1L,
		0, (long *) NULL, LONG, (char **) NULL, ohd);
    (void) add_fea_fld("cross_cor", maxlag - minlag + 1,
		1, (long *) NULL, FLOAT, (char **) NULL, ohd);

    if (debug_level)
	Fprintf(stderr, "writing output header to file\n");

    (void)update_waves_gen(ihd1, ohd,
			   (float)(start1 + (frame_len+1)/2), (float) step);

    write_header(ohd, ofile);

/* Allocate buffers and set up output record. */

    TRYALLOC(double, frame_len, x, "frame from first input file")
    TRYALLOC(double, frame_len, y, "frame from second input file")
    TRYALLOC(double, frame_len, wx, "windowed frame from first input file")
    TRYALLOC(double, frame_len, wy, "windowed frame from second input file")
    TRYALLOC(double, maxlag - minlag + 1, d_c_cor, "array of cross correlations")

    fea_rec = allo_fea_rec(ohd);
    tag1 = (long *) get_fea_ptr(fea_rec, "tag1", ohd);
    tag2 = (long *) get_fea_ptr(fea_rec, "tag2", ohd);
    c_cor = (float *) get_fea_ptr(fea_rec, "cross_cor", ohd);

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

    fea_skiprec(ifile1, start1 - 1, ihd1);
    fea_skiprec(ifile2, start2 - 1, ihd2);

    first = 1;
    *tag1 = start1;
    *tag2 = start2;
    while (n_frames--)
    {
	long	i;		/* loop index */

	if (debug_level)
	    Fprintf(stderr, "tag1: %ld;  tag2: %ld\n%ld frames to go.\n",
		*tag1, *tag2, n_frames+1);

	(void) get_sd_orecd(x, (int) frame_len, (int) step, first, ihd1, ifile1);
	if ( first && nsrflag || !nsrflag )
	    (void) get_sd_orecd(y, (int) frame_len, (int) step, first, ihd2, ifile2);

	if (debug_level >= 2)
	{
	    pr_darray("frame from input1.sd", frame_len, x);
	    if ( first && nsrflag || !nsrflag )
		pr_darray("frame from input2.sd", frame_len, y);
	}

	/* Window data.  (Not in place, since x & y must be preserved
	   for get_sd_orec.) */

	(void) windowd(frame_len, x, wx, win, (double *) NULL);
	if ( first && nsrflag || !nsrflag )
	    (void) windowd(frame_len, y, wy, win, (double *) NULL);

	if (debug_level >= 2)	{
	    pr_darray("windowed frame from input1.sd", frame_len, wx);
	    if ( first && nsrflag || !nsrflag )
		pr_darray("windowed frame from input2.sd", frame_len, wy);
	}

	d_cross_cor(wx, wy, frame_len, d_c_cor, minlag, maxlag);

	if (debug_level >= 2)
	    pr_darray("cross correlations", maxlag - minlag + 1, d_c_cor);

	for (i = 0; i < maxlag - minlag + 1; i++)
	    c_cor[i] = d_c_cor[i];
	put_fea_rec(fea_rec, ohd, ofile);
	*tag1 += step;
	*tag2 += step;
	first = 0;
    }

    exit(0);
    /*NOTREACHED*/
}    


/*
 * Compute cross correlations of a pair of vectors.
 * New version of d_cross_cor by David Talkin, 91 July 11, with minor
 * mod by Rod Johnson.
 */

void
d_cross_cor(x, y, len, c_cor, minlag, maxlag)
    double  *x, *y;		/* input vectors */
    register long    len;		/* length of input vectors */
    double  *c_cor;		/* result vector */
    register long    minlag, maxlag;	/* maximum and minimum lags.  Length
				   of c_cor is maxlag - minlag + 1 */
{
  if((minlag <= maxlag) && (len > 0)) {
    register long    i, j, lim;
    register double  sum, *xp, *yp, len_inv;

    spsassert(x, "d_cross_cor: first input vector is NULL");
    spsassert(y, "d_cross_cor: second input vector is NULL");
    spsassert(c_cor, "d_cross_cor: output vector is NULL");

    len_inv = 1.0/len;
    lim = (maxlag < 0) ? maxlag + 1 : 0;
    for (j = minlag; j < lim; j++)
      {
	for (i = -j, yp = y+i, xp = x, sum = 0.0; i++ < len; )
	  sum += *xp++ * *yp++;
	*c_cor++ = sum*len_inv;
      }

    for (j = (minlag < 0)? 0 : minlag; j <= maxlag; j++)
      {
	for (i = len-j, yp = y, xp = x+j, sum = 0.0; i-- > 0; )
	  sum += *xp++ * *yp++;
	*c_cor++ = sum*len_inv;
      }
  } else
    fprintf(stderr,
	    "Bad arguments passed to d_cross_cor(len:%d minlag:%d maxlag:%d)\n",
	    len,minlag,maxlag);
}


/*
 * For debug printout of double arrays
 */

void pr_darray(text, n, arr)
    char    *text;
    long    n;
    double  *arr;
{
    int	    i;

    Fprintf(stderr, "%s -- %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}


/*
 * Get number of samples in a sampled-data file.
 * Replace input stream with temporary file if input is a pipe.
 */

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
