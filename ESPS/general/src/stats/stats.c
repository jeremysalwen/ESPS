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
 * Written by:  Joe Buck
 * Checked by:
 * Revised by: Dave Burton, Ken Nelson
 *
 * Brief description: compute statistics on ESPS files
 *
 */

static char *sccs_id = "@(#)stats.c	3.9	9/24/98	ESI/ERL";
int debug_level = 0;

/*
  stats - compute statistics on ESPS files				     
  Joseph T. Buck							    
 									   
  stats considers an ESPS file to be a series of vectors. It computes mean, 
  standard deviation, maximum, and minimum values on an element-by-element 
  basis. The number of vectors used may be controlled with the -e switch.  
  The -n switch suppresses the header.					   
  									   
  From SDS version 1.3 of 10/28/85					   

  Joe Buck started converting it to SPS; Alan Parker finsished it.
  Dave Burton made ESPS modifications.

*/

#include <stdio.h>
#include <math.h>
#define SYNTAX USAGE ("stats [-x debug_level] [-r rec_range] [-e el_range] [-m magnitude] [-n] file")
#define MAGNITUDE(x, m) (((x-m) > (m-x)) ? (x-m) : (m-x))

#include <esps/esps.h>
#include <esps/unix.h>

void lrange_switch (), range_switch ();
short   get_rec_len ();
char   *eopen ();
char   *get_sphere_hdr();


main (argc, argv)
int     argc;
char  **argv;
{
    extern  optind;
    extern char *optarg;
    int     l_rec;
    int     c,
            hflag = 1,
            mflag = 0;
    long    s_rec = 1,
            e_rec = LONG_MAX,
            i_rec,
            i,
            tag;
    char   *r_range = NULL,
           *erange = NULL,
           *filename = NULL;
    double *data,
           *mins,
           *maxs,
           *avgs,
           *devs,
           *pow_ptr,
           *mag;
    int    *minl,
           *maxl;
    int     s_ele,
            e_ele;
    double  scale,
            bias,
            sqrt ();
    FILE * istrm = stdin;
    struct header  *h;
    int     nofile = 0,
            filecount = 0,
	    multi_file=0;

    while ((c = getopt (argc, argv, "x:nmr:e:")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi(optarg);
		break;
	    case 'n': 
		hflag = 0;
		break;
	    case 'm': 
		mflag++;
		break;
	    case 'r': 
		r_range = optarg;
		break;
	    case 'e': 
		erange = optarg;
		break;
	    default: 
		SYNTAX;
	}
    }

    if (optind >= argc)
	nofile = 1;
    else
	filename = argv[optind];
    if (argc > optind+1)
	multi_file = 1;

    if (!multi_file)
    	(void) read_params ((char *) NULL, SC_CHECK_FILE, filename);

    if (nofile) {
	if (symtype ("filename") == ST_UNDEF) {
	    Fprintf (stderr, "stats: no input file.\n");
	    SYNTAX;
	}
	filename = getsym_s ("filename");
    }

    while (optind < argc || nofile) {
	filecount++;
	e_rec = LONG_MAX;
	if (!nofile)
	    filename = argv[optind];

	filename = eopen ("stats", filename, "r", NONE, NONE, &h, &istrm);

        /* bail out if this is a sphere file */
        if ( get_sphere_hdr(h) ) {
            (void) fprintf(stderr, "stats: Input file (%s) is a Sphere file",
                       filename);
            (void) fprintf(stderr, " - use fea_stats(1-ESPS).\n");
            exit (1);
        }      

	if (!r_range && !multi_file) {
	    if (symtype ("start") != ST_UNDEF)
		s_rec = getsym_i ("start");
	    if (symtype ("nan") != ST_UNDEF)
		e_rec = s_rec + getsym_i ("nan") - 1;
	}
	else
	    lrange_switch (r_range, &s_rec, &e_rec, 1);

	if (e_rec == 0)
	    e_rec = LONG_MAX;

/* initialize the element range counters */
	s_ele = 1;
	e_ele = l_rec = get_rec_len (h);
				/* this returns the record length */

/* allocate the data arrays */
	data = calloc_d ((unsigned) l_rec);
	spsassert (data, "stats: calloc failed!");

	mins = calloc_d ((unsigned) l_rec);
	spsassert (mins, "stats: calloc failed!");

	maxs = calloc_d ((unsigned) l_rec);
	spsassert (maxs, "stats: calloc failed!");

	avgs = calloc_d ((unsigned) l_rec);
	spsassert (avgs, "stats: calloc failed!");

	devs = calloc_d ((unsigned) l_rec);
	spsassert (devs, "stats: calloc failed!");

	pow_ptr = calloc_d ((unsigned) l_rec);
	spsassert (pow_ptr, "stats: calloc failed!");

	mag = calloc_d ((unsigned) l_rec);
	spsassert (mag, "stats: calloc failed!");

	minl = calloc_i ((unsigned) l_rec);
	spsassert (minl, "stats: calloc failed!");

	maxl = calloc_i ((unsigned) l_rec);
	spsassert (maxl, "stats: calloc failed!");

	range_switch (erange, &s_ele, &e_ele, 1);
	if (e_ele > l_rec) {
	    Fprintf (stderr, "stats: only %d elements per record\n", l_rec);
	    exit (1);
	}
/* Initialize statistics */
	for (i = s_ele - 1; i < e_ele; i++) {
	    avgs[i] = pow_ptr[i] = devs[i] = 0.0;
	    mins[i] = FLT_MAX;
	    maxs[i] = -FLT_MAX;
	    mag[i] = 0.;
	}
	fea_skiprec (istrm, s_rec - 1, h);
	for (i_rec = s_rec;
		i_rec <= e_rec && get_gen_recd (data, &tag, h, istrm) != EOF;
		i_rec++) {

	    for (i = s_ele - 1; i < e_ele; i++) {
		if (data[i] < mins[i]) {
		    mins[i] = data[i];
		    minl[i] = i_rec;
		}
		if (data[i] > maxs[i]) {
		    maxs[i] = data[i];
		    maxl[i] = i_rec;
		}
		avgs[i] += data[i];
		pow_ptr[i] = devs[i] += data[i] * data[i];
	    }
	}

	e_rec = i_rec - 1;
	if (!multi_file && strcmp ("filename", "<stdin>") != 0) {
	    (void) putsym_s ("filename", filename);
	    (void) putsym_s ("prog", "stats");
	    (void) putsym_i ("start", (int) s_rec);
	    (void) putsym_i ("nan", (int) (e_rec - s_rec + 1));
	}

	scale = 1.0 / (double) (e_rec - s_rec + 1);

/* bias is the factor that converts the biased estimate of the variance to
   the unbiased estimate: (e_rec-s_rec+1)/((e_rec-s_rec)-1).
*/
	bias = 0.0;
	if ((e_rec - s_rec + 1) > 1)
	    bias = (double) (e_rec - s_rec + 1) / (double) (e_rec - s_rec);
	for (i = s_ele - 1; i < e_ele; i++) {
	    avgs[i] *= scale;
	    pow_ptr[i] *= scale;

/*
 * for some cases this expression yields a negative number very close
 * zero that should be treated as zero.
 */
	    if ((bias * (devs[i] * scale - avgs[i] * avgs[i])) <= 0)
		devs[i] = 0;
	    else
		devs[i] = sqrt (bias * (devs[i] * scale - avgs[i] * avgs[i]));
	}

/*
 * Now compute average adjusted magnitude, if required.
 * This cannot be done if the input is a pipe.
*/
	if (mflag) {
	    if (h -> common.ndrec == -1) {
		Fprintf (stderr,
			"stats: cannot use the m option when the input is a pipe.\n");
		exit (1);
	    }
	    rewind (istrm);
	    h = read_header (istrm);
	    fea_skiprec (istrm, s_rec - 1, h);
	    for (i_rec = s_rec; i_rec <= e_rec &&
		    get_gen_recd (data, &tag, h, istrm) != EOF; i_rec++) {
		for (i = s_ele - 1; i < e_ele; i++) {
		    mag[i] += MAGNITUDE (data[i], avgs[i]);
		}
	    }
	    /* divide by number of records */
	    for (i = s_ele-1; i < e_ele; i++)
		mag[i] *= scale;
	}


	if (hflag) {
	    if (multi_file)
	    	(void) printf ("%s:\n",filename);
	    (void) printf ("Element     Minimum    at     Maximum    at      Mean       Std. Dev    %s", (mflag ? "Magnitude\n" : "Power\n"));
	}

	for (i = s_ele - 1; i < e_ele; i++) {
	    (void) printf ("%5d%13g%6d%13g%6d%13.4g%12.4g", i + 1, mins[i], minl[i],
		    maxs[i], maxl[i], avgs[i], devs[i]);
	    if (!mflag)
		(void) printf ("%12.4g", pow_ptr[i]);
	    if (mflag)
		(void) printf ("%12.4g", mag[i]);
	    (void) printf ("\n");
	}

	if (nofile)
	    break;
	optind++;
	free((char *)data);
	free((char *)mins);
	free((char *)maxs);
	free((char *)avgs);
	free((char *)devs);
	free((char *)pow_ptr);
	free((char *)mag);
	free((char *)minl);
	free((char *)maxl);
    }


    return 0;
}
