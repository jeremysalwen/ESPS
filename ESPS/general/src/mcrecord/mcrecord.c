/* mcrecord - sample data from Masscomp A/D converted and store in ESPS SD file
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Syntax: record [options] file
 *   Options:
 *	-g gain
 *		gain is a number between from 0 to 3. 
 *		This is a magic cookie; the gain factor used depends
 *		on the particular A/D converter being used.
 *		(See below.)
 *	-p range
 *		range is a number of points to capture.
 *	-f range
 *		range is a number of frames to capture (default frame size
 *                = 100)
 *	-w width
 *		changes the frame size
 *	-s range
 *		range is the number of seconds of data to capture.
 *	-r sampling rate
 *		number of samples per second
 *	-c a2d channel
 *		input channel number
 *	-k clock#
 *		the number of the clock unit that drives the conversion
 *	-x debug_level
 *		0 => no debug; 1 => print debug messages
 *	-t comment
 *		comment text to add to header
 *	-d adtype
 *		this overrides the compiled in default.  This doesn't
 *		affect processing; it just goes into the header.
 *	-C
 *		alternate clock device
 *	-D
 *		alternate A/D device
 *
 * Written by: David Burton
 * Checked by: Alan Parker
 * Modified for ESPS by Alan Parker
 * Additional modifications by Ajaipal S. Virdy
 *
 * Purpose: Digitize analog data using Masscomp A/D converter
 *
 *
 *       ---------------------------------------------------
 *       |Gain settings for different Masscomp A/D devices:|
 *       |						   |
 *       |Factor	AD12F	AD12FA	AD12FA*	EF12M	   |
 *       |  0	 	x1	  x1	  x1	 x1 	   |
 *       |  1	 	x4	  x2	  x4	 x2	   |
 *       |  2	 	x16	  x4	  x16	 x4	   |
 *       |  3	 	x64	  x8	  x64	 x8	   |
 *       ---------------------------------------------------
 *
 * NOTE: default clock for record is 1.  Default clock for play is 0.
 */

#ifndef lint
       static char *sccsid = "@(#)mcrecord.c	3.13 3/3/89	ESI";
#define DATE "3/3/89"
#define VERSION "3.13"
#endif

#ifndef DATE
#define DATE ""
#endif

#ifndef VERSION
#define VERSION "nosccs"
#endif


#include <stdio.h>

/* ESPS includes */
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/sd.h>

#define BPS		sizeof (short)	/* number of bytes per short int */
#define EX_RDWR		1	/* write only mode for the dacp device */
#define	EX_WRIT		0	/* write only mode for the dacp device */
#define NEAREST		1	/* choose the nearest freq. > desired freq. */
#define	SIMUL		1	/* do simultaneous conversions */
#define SQUARE		4	/* make the clock pulses square */
#define LOW		0	/* start the clock pulses low */
#define OFLAGS		(O_WRONLY | O_TRUNC | O_CREAT)

#define SINGLE_ENDED	0	/* single ended mode on A/D */
#define DIFFERENTIAL	1	/* differential mode */

#define BIPOLAR		0	
#define UNIPOLAR	1

/* ************************************************************************ */
/*

	CHECK THESE DEFAULTS.  Normally these values come from the 
			       install script.
*/

#ifndef CLKDEV
#define CLKDEV "/dev/dacp0/clk"
#endif

#ifndef ADDEV
#define ADDEV "/dev/dacp0/adf0"
#endif

#ifndef MAXAD
#define MAXAD 2047
#endif

#ifndef MINAD
#define MINAD -2048
#endif

#ifndef ADTYPE
#define ADTYPE  "EF12M"
#endif

/* ************************************************************************ */

#define DEF_SAMPLE_RATE 8000		/* default sample rate / sec */
#define PROGRAM "record"		/* name of this program */

#define SOURCE "analog data digitized by ESPS record\n"
#define fperror (void)perror
#define SYNTAX USAGE \
("record [-x debug] [-[spf] range] [-w frame size] [-c a2d channel] [-r sampling rate] [-k clock#] [-g gain] [-t \"comment\"] [-C clock] [-D a/d device] file");

/* Externals */
int debug_level = 0;
extern int   optind;
extern char *optarg;
char *get_cmd_line();

main (argc, argv)
int argc;
char **argv;
{
    int     c, i,
            clkpn = -1,
            clkn = 1,		/* default clock is clock 1 */
            dacppn = -1,
            fchan = 0,
            incr = 1,
            nchans = 1,
            num_samps = 0,
            sizlckd,
	    clipped=0,
            status[2];

    short *data;

    char   clockdev[200],
	   *clkdev = CLKDEV,
	   *comment = NULL,		/* for comment from command line */
	   *adtype = ADTYPE,
           *dacpdev = ADDEV;

    char    answer[80];			/* For getting answer */

    double  retd_f,
            trigfreq = DEF_SAMPLE_RATE,
            retd_w;


    double    sam_rate = DEF_SAMPLE_RATE;

    


    FILE *ostrm;

    int s_switch = NO, p_switch = NO, f_switch = NO;
    int    p_value = 0,  f_value = 0;
    double s_value = 0.0;

    struct header  *oh;

    int  fwidth = 100;
    int  gain = 0;


    while ((c = getopt (argc, argv, "p:s:k:r:c:w:f:g:x:t:d:C:D:")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi (optarg);
		break;
	    case 'g':
		gain = atoi (optarg);
		if (gain < 0 || gain >4) {
		 Fprintf(stderr,"record: -g value must be from 0 to 3\n");
		 exit(1);
		}
		break;
	    case 'c': 
		fchan = atoi (optarg);
		break;
	    case 'k': 
		clkn = atoi (optarg);
		break;
	    case 'r': 
		sam_rate = atof (optarg);
		trigfreq = sam_rate;
		break;
	    case 'p': 
		p_value = atoi(optarg);
		p_switch = yes;
		break;
	    case 'f':
		f_value = atoi(optarg);
		f_switch = yes;
		break;
	    case 'w':
		fwidth = atoi (optarg);
		break;
	    case 's': 
		s_value = atof(optarg);
		s_switch = yes;
		break;
	    case 't':
		comment = optarg;
		break;
	    case 'd':
		adtype = optarg;
		break;
	    case 'C':
		clkdev = optarg;
		break;
	    case 'D':
		dacpdev = optarg;
		break;
	    default: 
		SYNTAX;
	}
    }

    if (optind >= argc)
	SYNTAX;


/*
 * convert range argument to number of sample points
 */

     if (p_switch == YES) {
	num_samps = p_value;
     }
     else if (f_switch == YES) {
             num_samps = f_value * fwidth;
	  }
          else if (s_switch == YES) {
                  num_samps = (int) ( ( (float) sam_rate * s_value ) + 0.5 );
               }
	       else {
                  Fprintf(stderr,
		  "record: amount of data to record has not beeen specified\n");
		  exit(1);
	       }

     if (debug_level)
	Fprintf (stderr, "record: num_samps is %d, p is %d, f is %d, s is %f\n", 
                num_samps, p_value, f_value, s_value);

     if (num_samps == 0) {
	Fprintf (stderr, "record: number of samples must be specified.\n");
	exit(1);
     }


/* check if sampling rate is > 0 */

   if ( sam_rate <= 0 ) {
        Fprintf(stderr, "record: sampling rate (%lf) must be > 0\n", sam_rate);
        exit(1);
   }

/* First open designated data file - if error occurs stop program */
 

    if(strcmp(argv[optind],"-") == 0)
	ostrm = stdout;
    else 
    	TRYOPEN ( PROGRAM, argv[optind], "w", ostrm );

/* get a new SD header */
    oh = new_header (FT_SD);

/* Allocate memory for data */
    if ((data = (short *) malloc (num_samps * BPS)) == NULL) {
	Fprintf (stderr,
	"%s: could not allocate memory for data.\n",
	argv[0]);
	exit (1);
    }



/* open the clock and the a/d */

    (void)sprintf(clockdev, "%s%d", clkdev, clkn);
    (void)mropen (&dacppn, dacpdev, EX_RDWR); 
    (void)mropen (&clkpn, clockdev, EX_RDWR);

    if (debug_level > 0) {
	Fprintf (stderr,"%s opened, pathno = %d\n", dacpdev, dacppn);
	Fprintf (stderr,"%s opened, pathno = %d\n", clockdev, clkpn);
    }

/* set up the clock */

    (void)mrclk1 (clkpn, NEAREST, trigfreq, &retd_f, SQUARE, 0.0, &retd_w, LOW);

    if (trigfreq != retd_f) {
	Fprintf (stderr,
	"nearest freq. to %lf is %lf. do you want to go on? [y] ",
	trigfreq, retd_f);
	answer[0] = 'y';
	if (fgets (answer, 80, stdin) != NULL) {
	    if (answer[0] != 'y' && answer[0] != 'Y' && answer[0] != '\n') {
		Fprintf (stderr,"Exit.\n");
		(void)mrclosall ();
		exit (0);
	    }
	}
    } /* end if (trigfreq != retd_f) */

    if (debug_level > 0)
	Fprintf (stderr,"Clock set to %f Hz; pulse width is %f microseconds\n",
		retd_f, retd_w);

/* Setup the DACP A/D device and trigger the clock */

    (void)mradmod (dacppn, BIPOLAR, DIFFERENTIAL);
    (void)mrclktrig (dacppn, 1, clkpn);

/* Prompt user for initiation of data collection */
    
    Fprintf (stderr, "record: when ready to record data, hit RETURN: ");

/* Wait for carriage return */

    while( (c = getchar()) != '\n')	/* wait for carriage return */
	;

/* lock this process's image into memory */

    (void)mrlock (0, 0, &sizlckd);
    if (debug_level)
       Fprintf (stderr, "record: %d bytes locked in memory\n", sizlckd);

    Fprintf (stderr, "Starting transfer ... ");

/* Start the Analog to Digital transfer */
    (void)mradxin (dacppn, clkpn, -1, fchan, nchans, incr, gain, num_samps, data);

/* Wait until completion of transfer */

    (void)mrevwt (dacppn, status, 0);
    Fprintf (stderr, "Done\n");

/* Then copy binary data into file */

    oh->hd.sd->max_value = 0;
    for (i=0; i<num_samps; i++) {
	if (data[i] == MAXAD || data[i] == MINAD) clipped++;
	if (abs(data[i]) > oh->hd.sd->max_value)
		oh->hd.sd->max_value = abs(data[i]);
    }

    if (clipped)
	fprintf(stderr,"record: warning %d samples at min or max a/d value.\n"
        ,clipped);


     oh->common.ndrec = num_samps;
     oh->common.tag = NO;
     set_sd_type(oh,SHORT);
     oh->hd.sd->sf = (float)sam_rate;
     oh->hd.sd->src_sf = 0;
     oh->hd.sd->synt_method = NONE;
     oh->hd.sd->q_method = NONE;
     oh->hd.sd->equip = lin_search(equip_codes,adtype);
     oh->hd.sd->scale = 1;
     oh->hd.sd->nchan = 1;

     (void)strcpy(oh->common.prog, PROGRAM);
     (void)strcpy(oh->common.vers, VERSION);
     (void)strcpy(oh->common.progdate, DATE);

     add_comment (oh, get_cmd_line(argc,argv));
     add_comment (oh, SOURCE);
     if (comment) {
     	add_comment (oh, comment);
	add_comment (oh, "\n");
     }

     write_header(oh, ostrm);


    (void) put_sd_recs (data, num_samps, oh, ostrm);

    /* Clean up */

    (void)fclose(ostrm);
    (void)putsym_s("filename",argv[optind]);
    (void)putsym_s("prog","record");
    (void)putsym_i("start",1);
    (void)putsym_i("nan",num_samps);

    exit(0);
    /* NOTREACHED */
}
