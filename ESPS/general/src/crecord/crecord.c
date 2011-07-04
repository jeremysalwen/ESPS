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
 * Convex changes by: Dale Lancaster, Convex Computer Corporation
 *
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
       static char *sccsid = "%W% %G% ESI";
#define DATE "%G%"
#define VERSION "%I%"
#endif

#ifndef DATE
#define DATE ""
#endif

#ifndef VERSION
#define VERSION "nosccs"
#endif


#include <stdio.h>

#ifdef CONVEX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include </sys/dev_iop/adcommon.h>
extern int errno ;
#endif
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
#ifdef CONVEX
#define ADDEV "/dev/ad0"
#else
#define ADDEV "/dev/dacp0/adf0"
#endif
#endif

#ifndef MAXAD
#define MAXAD 2047
#endif

#ifndef ADTYPE
#ifdef  CONVEX
#define ADTYPE  "RTI-732"
#else
#define ADTYPE  "EF12M"
#endif
#endif

/* ************************************************************************ */

#define DEF_SAMPLE_RATE 8000		/* default sample rate / sec */
#define PROGRAM "record"		/* name of this program */

#define SOURCE "analog data digitized by ESPS record\n"
#define fperror (void)perror

#ifdef  CONVEX
#define SYNTAX USAGE \
("record [-x debug] [-[spf] range] [-w frame size] [-c a2d channel] [-r sampling rate] [-g gain] [-t \"comment\"] [-C clock] [-D a/d device] file");
#else
#define SYNTAX USAGE \
("record [-x debug] [-[spf] range] [-w frame size] [-c a2d channel] [-r sampling rate] [-k clock#] [-g gain] [-t \"comment\"] [-C clock] [-D a/d device] file");
#endif

/* Externals */
int debug_level = 0;
extern int   optind;
extern char *optarg;
char *get_cmd_line();

main (argc, argv)
int argc;
char **argv;
{
#ifdef CONVEX
    int ioctl_data ;
#endif

    int     c,
            clkpn = -1,
            clkn = 1,		/* default clock is clock 1 */
            dacppn = -1,
            fchan = 0,
            incr = 0,
            nchans = 1,
            num_samps = 0,
            sizlckd,
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


    int    sam_rate = DEF_SAMPLE_RATE;

    


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
#ifndef CONVEX
	    case 'k': 
		clkn = atoi (optarg);
		break;
#endif
	    case 'r': 
		sam_rate = atoi (optarg);
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
#ifndef CONVEX
	    case 'C':
		clkdev = optarg;
		break;
#endif
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
        Fprintf(stderr, "record: sampling rate (%d) must be > 0\n", sam_rate);
        exit(1);
   }

/* Allocate memory for data */
    if ((data = (short *) malloc (num_samps * BPS)) == NULL) {
	Fprintf (stderr,
	"%s: could not allocate memory for data.\n",
	argv[0]);
	exit (1);
    }



/* open the clock and the a/d */

#ifndef CONVEX
    (void)sprintf(clockdev, "%s%d", clkdev, clkn);
    (void)mropen (&clkpn, clockdev, EX_RDWR);
    (void)mropen (&dacppn, dacpdev, EX_RDWR); 
#endif
    dacppn = open (dacpdev, O_RDWR); 
    
    if (debug_level > 0) {
	Fprintf (stderr,"%s opened, pathno = %d, errno = %d\n", 
		 dacpdev, dacppn,errno);
#ifndef CONVEX
	Fprintf (stderr,"%s opened, pathno = %d\n", clockdev, clkpn);
#endif
    }

/* set up the clock */

#ifdef CONVEX
    /* Make sure that the "software clock" is set to the proper value */
    ioctl_data = DEFAULT_DELAY ;
    ioctl(dacppn,ADSETDELAY,&ioctl_data) ;
#else
    (void)mrclk1 (clkpn, NEAREST, trigfreq, &retd_f, SQUARE, 0.0, &retd_w, LOW);

    if (trigfreq != retd_f) {
	Fprintf (stdout,
	"nearest freq. to % 8.2f is % 8.2f. do you want to go on? [y] ",
	trigfreq, retd_f);
	answer[0] = 'y';
	if (fgets (answer, 80, stdin) != NULL) {
	    if (answer[0] != 'y' && answer[0] != 'Y' && answer[0] != '\n') {
		Fprintf (stdout,"Exit.\n");
		(void)mrclosall ();
		exit (0);
	    }
	}
    } /* end if (trigfreq != retd_f) */
#endif

    if (debug_level > 0)
#ifdef CONVEX
	Fprintf (stderr,"AD delay set to %d iterations.\n",DEFAULT_DELAY);
#else
	Fprintf (stderr,"Clock set to %f Hz; pulse width is %f microseconds\n",
		retd_f, retd_w);
#endif

/* Setup the DACP A/D device and trigger the clock */

#ifdef CONVEX
    if (debug_level) {
	/* Turn on level 1 debug messages inside ad driver */
	ioctl_data = 1 ;
	ioctl(dacppn,ADDEBUG,&ioctl_data) ;
    } ;
	ioctl_data = fchan ;
    ioctl(dacppn,ADSETCHNL,&ioctl_data) ;
	ioctl_data = gain ;
    ioctl(dacppn,ADSETGAIN,&ioctl_data) ;
	ioctl_data = sam_rate ;
    ioctl(dacppn,ADSETHZ,&ioctl_data) ;
#else
    (void)mradmod (dacppn, BIPOLAR, DIFFERENTIAL);
    (void)mrclktrig (dacppn, 1, clkpn);
#endif

/* Prompt user for initiation of data collection */
    
    Fprintf (stdout, "record: when ready to record data, hit RETURN: ");

/* Wait for carriage return */

    while( (c = getchar()) != '\n')	/* wait for carriage return */
	;

/* lock this process's image into memory */

#ifndef CONVEX
    (void)mrlock (0, 0, &sizlckd);
    if (debug_level)
       Fprintf (stderr, "record: %d bytes locked in memory\n", sizlckd);
#endif

    Fprintf (stdout, "Starting transfer ... ");
    (void)fflush (stdout);

/* Start the Analog to Digital transfer */
#ifdef CONVEX
    read(dacppn,data,num_samps*2) ;
    if (strcmp(ADTYPE,"RTI-732") == 0) {
	int i ;
	/* Adjust the input by shifting right 4 bits */
	fprintf(stderr,"adjusting data...%d\n",num_samps) ;
	for (i=0; i < num_samps; i++) 
	    data[i] = data[i] >> 4 ;
    }
#else
    (void)mradxin (dacppn, clkpn, -1, fchan, nchans, incr, gain, num_samps, data);

/* Wait until completion of transfer */

    (void)mrevwt (dacppn, status, 0);
#endif

    Fprintf (stdout, "Done\n");

/* First open designated data file - if error occurs stop program */
 

    TRYOPEN ( PROGRAM, argv[optind], "w", ostrm );

     oh = new_header (FT_SD);
     oh->common.ndrec = num_samps;
     oh->common.tag = NO;
     set_sd_type(oh,SHORT);
     oh->hd.sd->sf = (float)sam_rate;
     oh->hd.sd->src_sf = 0;
     oh->hd.sd->synt_method = NONE;
     oh->hd.sd->q_method = NONE;
     oh->hd.sd->equip = lin_search(equip_codes,adtype);
     oh->hd.sd->max_value = MAXAD;
     oh->hd.sd->scale = 1;
     oh->hd.sd->nchan = 1;

     (void)strcpy(oh->common.prog, PROGRAM);
     (void)strcpy(oh->common.vers, VERSION);
     (void)strcpy(oh->common.progdate, DATE);

     add_comment (oh, get_cmd_line(argc,argv));
     add_comment (oh, SOURCE);
     if (comment)
     	add_comment (oh, comment);

     write_header(oh, ostrm);

    /* Then copy binary data into file */

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
