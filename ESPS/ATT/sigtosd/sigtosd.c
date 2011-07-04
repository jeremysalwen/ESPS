/*	Copyright (c)  1988, 1989 AT&T and Entropic Speech, Inc.*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*      AND ENTROPIC SPEECH, INC.                               */
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

#ifndef lint
static char *sccs_id = "@(#)sigtosd.c	1.3 1/9/90 AT&T, ESI";
#endif

#define VERSION "1.3"
#define DATE "1/9/90"

#include <Objects.h>
#include <sgtty.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/sd.h>
#include <esps/feasd.h>

#define SYNTAX \
USAGE("sigtosd [-x debug_level] [-H] input.sig output.esps\n")

char *get_cmd_line();
void e_make_sd_hdr();

int debug_level = 0;
int debug = 0;
int w_verbose = 0;
long max_buff_bytes = 2000000;  /* limit on size of signal data buffers */


  
/*      ----------------------------------------------------------      */

main(argc,argv)
int	argc;
char	**argv;
{

  int i,j;
  short *data;
  int Hflag = 0;		/* flag for -H option */

  char *sp, *oname;
  Signal *spsig;
  FILE *outstrm = NULL;
  char *command;		/* function command line */

  char *sigheader;		/* copy of SIGnal header with nulls stripped */
  int sigheadsiz;		/* size of sigheader */
  extern int	optind;
  extern char	*optarg;	
  int     	ch;

  command = get_cmd_line(argc, argv);

  while ((ch = getopt(argc, argv, "x:H")) != EOF)
    switch (ch)  {
    case 'x':
      debug_level = debug = atoi(optarg);
      break;
    case 'H':
      Hflag++;
      break;
    default: 
      SYNTAX
	;
      break;
    }
  
    if (optind < argc)
	sp = argv[optind++];
    else {
	Fprintf(stderr, "sigtosd: no input SIGnal file specified.\n");
	SYNTAX
	;
    }
    if (debug_level)
	Fprintf(stderr, "Input file is %s\n", sp);
    if (optind < argc)
	oname = argv[optind++];
    else {
	Fprintf(stderr, "sigtosd: no output file specified.\n");
	SYNTAX
	;
    }
  if (debug_level)
    Fprintf(stderr, "Output file is %s\n", oname);
  
  spsig = NULL;
  if((spsig = get_signal(sp,0.0,-1.0,NULL))) {
    if (debug_level) fprintf(stderr, "sigtosd: got input SIGnal file\n");
    spsig->name = oname;

    /* add esps fea_sd header to SIGnal file */
    spsig->header->e_scrsd = 1;
    e_make_sd_hdr(spsig, command);

    
    if (debug_level) fprintf(stderr, "sigtosd: creating SD header\n");
    if (!Hflag) {
      if (debug_level) fprintf(stderr, 
			       "sigtosd: adding SIGnal header as comment\n");
      add_comment(spsig->header->esps_hdr,
		  "Here is a copy of the input SIGnal header:\n");
      /*copy SIG header, strip out nulls*/
      sigheadsiz = 1 + spsig->header->nbytes * sizeof(char);
      sigheader = malloc(sigheadsiz);
      j = 0;
      for (i = 0; i < spsig->header->nbytes; i++) {
	if (spsig->header->header[i] != '\0') {
	  sigheader[j] = spsig->header->header[i];
	  j++;
	}
      }
      sigheader[sigheadsiz - 1] = '\0';
      add_comment(spsig->header->esps_hdr, sigheader);
    }
    
    spsig->header->magic = ESPS_MAGIC;
    TRYOPEN ("sigtosd", spsig->name, "w", outstrm);
    spsig->header->strm = outstrm;
    if (debug_level) fprintf(stderr, "sigtosd: writing SD file\n");
    put_signal(spsig);
    
  } else {
    printf("Can't access sampled speech signal: %s\n",sp);
    exit(-1);
  }
  if(spsig)free_signal(spsig);
}

void
e_make_sd_hdr(sig, command_line)
Signal *sig;
char *command_line;
{
  struct header *newhd;
  newhd = new_header(FT_FEA);
 (void) init_feasd_hd(newhd, SHORT, (int) 1, 
		      &sig->start_time, (int) 0, (double) sig->freq);
  add_comment(newhd, command_line);
  strcpy (newhd->common.prog, "sigtosd");
  strcpy (newhd->common.vers, VERSION);
  strcpy (newhd->common.progdate, DATE);
  sig->header->esps_hdr = newhd;
}
void
set_pvd(hdr)
    struct header   *hdr;
{
/*this function does nothing - it's needed owing to the 
  convoluted waves library, which needs to be straightened out*/
}
