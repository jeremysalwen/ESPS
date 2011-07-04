/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright (c) 1990 Entropic Speech, Inc."
 *
 * Program:	mu2esps
 *
 * Written by:  David Burton
 *
 */


static char *sccs_id = "@(#)mu2esps.c	1.8 7/10/96	 ESI";

/*
 * include files
 */
#include <stdio.h>
#include <fcntl.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>

#if defined(SUN4) && !defined(OS5)
#include <multimedia/libaudio.h>
#include <multimedia/audio_filehdr.h>
#endif
/*
 * defines
 */

#define SYNTAX USAGE ("mu2esps [-s] [-x debug_level] infile outfile")
#define BUFFER 8192	/*Max. buffer length for transfering data*/

/*
 * system functions and variables
 */

extern  optind;
extern	char *optarg;
int atoi();
/* SDI 28/6/06 added LINUX to avoid compiler error */
#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char *calloc();
#endif
 
/*
 * external ESPS functions
 */

int getopt ();
char *eopen();
int mu_to_linear();
struct feasd *allo_feasd_recs();
void add_comment();
char *get_cmd_line();

/*
 * global declarations
 */
int debug_level = 0;

/*
 * main program
 */
main (argc, argv)
     int argc;
     char **argv;
{
  /*
   * setup and initialization
   */
  
  int c;			    /*for getopt return*/
  int i;			    /*loop counter*/
  char *infile = NULL;	    /*file name for input data file*/
  FILE *istrm;
  int ifd;
  struct header *oh;	    /*pointer to SD file header*/
  char *sd_file = NULL;	    /*file name for output SD file*/
  FILE *ostrm = stdout;	    /*output SD file stream*/
  short *sdata;		    /*array to hold output data*/
  int buffer_size = BUFFER;    /*number of items retrieved from input sampled
				 data file*/
  int points;		    /* number of points returned by get_data*/
  char *mudata;               /* buffer to hold mu-encoded data */
  double start_time;          /* holds start_time generic value for header*/
  struct feasd *data_rec;     /*data structure*/

  char *VERSION = "1.8";
  char *DATE    = "7/10/96";
  char *PROG    = "mu2esps";
  int sflag = 0;
  
  /*
   * process command line options
   */
  
  while ((c = getopt (argc, argv, "x:s")) != EOF) {
    switch (c) {
#if defined(SUN4) && !defined(OS5)
    case 's':
      sflag++;
      break;
#endif
    case 'x': 
      debug_level = atoi (optarg);
      break;
    default:
      SYNTAX;
    }
  }
  /*
   * process file arguments and open input and output files
   */
  
  /*
   * First get input file
   */
  if(optind < argc) {
    infile = argv[optind++];
    if (strcmp(infile, "-") != 0){
      if ((ifd = open(infile, O_RDONLY, 0)) == -1){
	Fprintf(stderr, "mu2esps: can't open ");
	perror(infile);
	exit(1);
      }
      istrm = fdopen( ifd, "r");
    }
    else{
      infile = "<stdin>";
      istrm = stdin;
      ifd = 0;
    }
    if(debug_level > 0)
      Fprintf(stderr, "mu2esps: input file is %s\n", infile);
  }
  else{
    Fprintf(stderr, "mu2esps: No input file specified.\n");
    SYNTAX;
    exit(1);
  }
  
  /*
   * Now output file 
   */
  
  if (optind < argc) {
    sd_file = eopen("mu2esps", argv[optind], "w", FT_FEA, FEA_SD, 
		    (struct header **)NULL, &ostrm); 
  }
  else {
    Fprintf(stderr, "mu2esps: no output file name specified.\n");
    SYNTAX;
  }
  
  if(debug_level)
    Fprintf(stderr, "mu2esps: Output file is %s\n", sd_file);
  
  /*
   * allocate memory for data transfer
   */
  
  mudata = (char *) calloc ((unsigned) buffer_size, sizeof(char));
  spsassert(mudata != NULL, "Cannot allocate needed space for output buffer");
  sdata = (short*) calloc ((unsigned) buffer_size, sizeof(short));
  spsassert(sdata != NULL, "Cannot allocate needed space for input");
  
  /*
   * set up output file header
   */
  oh = new_header(FT_FEA);
  (void) strcpy (oh->common.prog, PROG);
  (void) strcpy (oh->common.vers, VERSION);
  (void) strcpy (oh->common.progdate, DATE);
  (void) add_comment (oh, get_cmd_line(argc,argv));
  oh->common.tag = NO;
  
  start_time = 0;
  if((init_feasd_hd(oh, SHORT, 1, &start_time, NO , (double)8000)) != 0){
    Fprintf(stderr, "mu2esps: Couldn't allocate FEA_SD header - exiting.\n");
    exit(1);
  }
  (void) write_header(oh, ostrm);    
  
  /*
   * allocate the fea_sd dta structure
   */
  
  data_rec = allo_feasd_recs(oh, SHORT, (long)buffer_size, (char *)NULL, NO);
  sdata = (short *)data_rec->data;

  /*
   * get SunOS4.1 audio file header
   */
#if defined(SUN4) && !defined(OS5)
  if(sflag){
    char *hdrchar;
    int size, err;
    Audio_hdr Hdr;
    err = audio_read_filehdr( ifd, &Hdr, (char *)NULL, 0);
    if( err != AUDIO_SUCCESS){
      fprintf(stderr,"%s: %s does not have a valid SUNOS 4.1 audio file header\n", argv[0], infile);
      exit(1);
    }
  }  
#endif
  /*
   * get input data, convert to linear, and write output file
   */
  
  
  if (debug_level> 0) Fprintf(stderr, 
	      "mu2esps:reading blocks of %d points from data file %s\n", 
			      buffer_size,  infile);
  
  /* 
   * Now pack data array and write output file
   */
  while ((points = fread(mudata, sizeof(*mudata), buffer_size, istrm)) != 0){
    if(debug_level > 1)
      Fprintf(stderr, "mu2esps: Got %d points from input\n", points);
    
    /*convert to linear data*/
    (void)mu_to_linear(mudata, sdata, (long)points);
    if (debug_level >2){
      for(i=0;i<points; i++){
	Fprintf(stderr,"%d", sdata[i]);
	Fprintf(stderr,"\n");
      }
    }
    (void) put_feasd_recs(data_rec, 0L, (long) points, oh, ostrm);
  }
  /*
   * clean up and exit
   */
  exit(0);
  /*NOTREACHED*/
}
