/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)srecord.c	1.7	6/23/93	ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>

#define SPARC_SAMPLE_FREQ 8000
#define SIZ_CHANNEL 1
#define SYNTAX { \
fprintf(stderr,"Usage: %s [-x<debug level>] [-H] [-P] [-p<prompt string>] [-S] [-W<for waves make>] [-s<duration>] [-e] [[-o] output file]\n",av[0]); \
  fprintf(stderr, "Duration is in seconds and defaults to %f.\n",duration); \
  fprintf(stderr, "-H suppresses header creation, else an ESPS header is prepended.\n"); \
  fprintf(stderr, "-P enables prompting at start of A/D conversion\n"); \
  fprintf(stderr, "-p specifies the prompt to be used at start of A/D conversion\n"); \
  fprintf(stderr, "-S enables sending of the waves load file on completion.\n"); \
  fprintf(stderr, "The argument of -W gets appended to the send_xwaves make command.\n");  \
  fprintf(stderr, "-e enables echo of a/d input to d/a.\n"); \
}		  

/*
  This program records using SUN codec, and writes the ouput to stdout or file.
  The output file can optionally have no header or an ESPS header.
*/


#define ERR 4

/* BUF_SIZE is the maximum size of each UNIX I/O operation.  Below,
   this buffer size is tied to the physical space available on the S-bus
   board.  */

#define BUF_SIZE NELEMS

int debug_level = 0, send_display_file = FALSE, do_prompt = FALSE;
char *pstring = "START RECORDING NOW ...";

/*---------------------------------------------------------------------*/
output_the_header( stream, freq, amax, samples, channel)
     FILE *stream;
     double amax, freq;
     int samples, channel;
{
  double start_time = 0.0;
  struct header *head;
  void set_pvd();

  if(stream && (freq > 0.0) && (samples > 0)) {
    head = new_header(FT_FEA);
    set_pvd(head);
    (void)init_feasd_hd(head, SHORT, (channel < 2)?1:2, &start_time, NO, freq);
    if(stream != stdout) {
      *add_genhd_d("max_value", (double *)NULL, 1, head) = amax;
      *add_genhd_f("samples", (float *)NULL, 1, head) = samples;
    }
    (void)write_header(head, stream);
  } else
    fprintf(stderr,"Bad args to output_the_header(stream=%x freq=%f samples=%d\n",
	      stream, freq, samples);
  return(NULL);
}


/*---------------------------------------------------------------------*/
main(ac, av)
     int ac;
     char **av;
{
  int  channel = SIZ_CHANNEL;
  int echo_to_da = 0;
  double duration = 10.0, freq = SPARC_SAMPLE_FREQ;
  char *ofile = NULL;
  char *display_stuff = NULL;
  FILE *output = stdout;
  int c, do_header = TRUE;
  extern int optind;
  extern char *optarg;
  int i;

  while((c = getopt(ac,av,"x:o:s:W:p:SPH:?e")) != EOF) {
    switch(c) {
    case 'S':
      send_display_file = TRUE;
      break;
    case 'W':
      display_stuff = optarg;
      break;
    case 'P':
      do_prompt = TRUE;
      break;
    case 'p':
      pstring = optarg;
      do_prompt = TRUE;
      break;
    case 's':
      duration = atof(optarg);
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    case 'o':
      ofile = optarg;
      if(!(output = fopen(ofile,"w"))) {
	fprintf(stderr,"Can't open %s for output.\n",ofile);
	exit(ERR);
      }
      break;
    case 'H':
      do_header = FALSE;
      break;
    case 'e':
      echo_to_da = 1;
      break;
    case '?':
    default:
      SYNTAX
    }
  }
  if(ac < 2){
    SYNTAX
    exit(1);
  }

  if(ac > optind) {
    ofile = av[optind];
    if(strcmp(ofile,"-")) {
      if(!(output = fopen(ofile,"w"))) {
	fprintf(stderr,"Can't open %s for output.\n",ofile);
	exit(ERR);
      }
    } else {
      optind++;
      ofile = NULL;
      output = stdout;
    }
  }
    
 {
   int samples_requested = 0.5 + (duration*freq), got;
   double amax = 0.0;

   if(do_header)
       output_the_header(output, freq, amax, samples_requested, channel);
   
   got = record_file(output, samples_requested, &freq, channel, 
	&amax, echo_to_da);
   if(got) {
     if(do_header && (output != stdout)) { /* rewind and update header values*/
       fseek(output, 0, 0);
       output_the_header(output, freq, amax, got);
       fseek(output, 0, 2);
     }
     fclose(output);
     if(ofile && (output != stdout) && send_display_file) {
       char mess[500];
       if(display_stuff != NULL)
	 sprintf(mess,"make file %s %s\n", ofile, display_stuff);
       else
	 sprintf(mess,"make file %s\n", ofile);
       send_xwaves2(NULL, NULL, mess, debug_level);
     }
     exit(0);
   } else
     fprintf(stderr,"No samples collected!\n");
   exit(ERR);
 }
}


char	*ProgName = "srecord";
char	*Version = "1.3";
char	*Date = "6/10/93";

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}
