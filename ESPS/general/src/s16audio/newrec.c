/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 *  newrec.c
 *  a flexible single/dual-channel A/D program
 *
 */
static char *sccs_id = "@(#)newrec.c	1.9 9/18/97 ERL";

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>
#include <Objects.h>

#ifdef SG
#define SYNTAX { \
fprintf(stderr,"Usage: %s [-f<sample frequency>] [-c<channel> (1 or 2)] [-x<debug level>] [-H] [-P] [-p<prompt string>] [-S] [-W<for waves make>] [-s<duration>] [[-o]<output file>]\n",av[0]); \
  fprintf(stderr, "If no channel is specified, output will be single channel.\n"); \
  fprintf(stderr, "If channel = 2, output will be 2-channel (stereo).\n"); \
  fprintf(stderr, "If no output file is specified, output will go to stdout.\n"); \
  fprintf(stderr, "Duration is in seconds and defaults to %f.\n",duration); \
  fprintf(stderr, "-H suppresses header creation, else an ESPS header is prepended.\n"); \
  fprintf(stderr, "-P enables prompting at start of A/D conversion\n"); \
  fprintf(stderr, "-p specifies the prompt to be used at start of A/D conversion\n"); \
  fprintf(stderr, "-S enables sending of the waves load file on completion.\n"); \
  fprintf(stderr, "The argument of -W gets appended to the send_xwaves make command.\n");  \
  fprintf(stderr, "Use the Indigo \"apanel\" program to control input gain, etc.\n"); \
      exit(ERR); \
}		  
#else
#define SYNTAX { \
fprintf(stderr,"Usage: %s [-f<sample frequency>] [-c<channel> (0, 1 or 2)] [-x<debug level>] [-H] [-P] [-p<prompt string>] [-S] [-W<for waves make>] [-s<duration>] [[-o]<output file>]\n",av[0]); \
  fprintf(stderr, "If no channel is specified, output will be from channel B.\n"); \
  fprintf(stderr, "If no output file is specified, output will go to stdout.\n"); \
  fprintf(stderr, "Duration is in seconds and defaults to %f.\n",duration); \
  fprintf(stderr, "-H suppresses header creation, else an ESPS header is prepended.\n"); \
  fprintf(stderr, "-P enables prompting at start of A/D conversion\n"); \
  fprintf(stderr, "-p specifies the prompt to be used at start of A/D conversion\n"); \
  fprintf(stderr, "-S enables sending of the waves load file on completion.\n"); \
  fprintf(stderr, "The argument of -W gets appended to the send_xwaves make command.\n");  \
  fprintf(stderr, "-I selects line-in jack on HP700 (higher quality than mic).\n"); \
      exit(ERR); \
}		  
#endif

/*
  This program records using A/D hardware, and writes the ouput to stdout or file.
  Samples alternate A and B.  Optionally outputs only one or the other channel.
  The output file can optionally have no header or an ESPS header.
*/


#define ERR 4

/* BUF_SIZE is the maximum size of each UNIX I/O operation.  Below,
   this buffer size is tied to the physical space available on the S-bus
   board.  */

#define BUF_SIZE NELEMS

int debug_level = 0, send_display_file = FALSE, do_prompt = FALSE;
char *pstring = "START RECORDING NOW ...";
char *waves_name = "xwaves";
int  Iflag = 0;

/*---------------------------------------------------------------------*/
output_the_header( stream, freq, amax, samples, channel)
     FILE *stream;
     double amax, freq;
     int samples, channel;
{
  double start_time = 0.0;
  struct header *head;

  if(stream && (freq > 0.0) && (samples > 0)) {
    head = new_header(FT_FEA);
    (void)init_feasd_hd(head, SHORT, (channel < 2)?1:2, &start_time, NO, freq);
    if(stream != stdout) {
      *add_genhd_d("max_value", (double *)NULL, 1, head) = amax;
      *add_genhd_f("samples", (double *)NULL, 1, head) = samples;
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
  int  channel = 1;
  double srate = 16.0, duration = 10.0, freq = srate * 1000.0;
  char *ofile = NULL;
  char display_stuff[500];
  FILE *output = stdout;
  int c, do_header = TRUE;
  extern int optind;
  extern char *optarg;

  display_stuff[0]=0;
  while((c = getopt(ac,av,"r:f:c:x:o:s:W:p:ShPHg:w:k:t:d:C:D:n:I?")) != EOF) {
    switch(c) {
    case 'f':
    case 'r':
      srate = atof(optarg);
      if(srate > 96.0)		/* Assume user means Hz. */
	srate /= 1000.0;
      freq = srate * 1000.0;
      break;
    case 'I':
      Iflag = 1;;   /* used on HP700 to indicate line in */
      break;
    case 'g':
    case 'w':
    case 'k':
    case 't':
    case 'd':
    case 'C':
    case 'D':
      fprintf(stderr,"Option %c is obsolete; %s ignored.\n",c,optarg);
      break;
    case 'S':
      send_display_file = TRUE;
      break;
    case 'W':
      strcpy(display_stuff, optarg);
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
    case 'n':
      waves_name = optarg;
      break;
    case 'c':
      channel = atoi(optarg);
      if((channel > 2) || (channel < 0)) {
	fprintf(stderr,"Bad channel specification (%d)\n",channel);
	exit(ERR);
      }
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
    case 'h':
    case '?':
    default:
      SYNTAX
    }
  }
  if(ac < 2) SYNTAX

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

#ifdef HP700
  if (ofile == NULL)
      output = fdopen(dup(1),"w");
#endif
    
 {
   int samples_requested = 0.5 + (duration*freq), got;
   double amax = 0.0;

   if(do_header)
       output_the_header(output, freq, amax, samples_requested, channel);
   
   got = record_file(output, samples_requested, &srate, channel, &amax);
   if(got) {
#ifdef HP700
     if(do_header && (ofile != NULL)) { /* rewind and update header values */
#else
     if(do_header && (output != stdout)) { /* rewind and update header values */
#endif
       fseek(output, 0, 0);
       freq = srate * 1000.0;
       output_the_header(output, freq, amax, got, channel);
       fseek(output, 0, 2);
     }
     fclose(output);
     if(ofile && (output != stdout) && send_display_file) {
       char mess[500];
       sprintf(mess,"make file %s %s\n", ofile, display_stuff);
       if (!SendXwavesNoReply(NULL, waves_name, NULL, mess))
	fprintf(stderr,"Could not communicate with %s.\n",waves_name);
     }
     exit(0);
   } else
     fprintf(stderr,"No samples collected!\n");
   exit(ERR);
 }
}
