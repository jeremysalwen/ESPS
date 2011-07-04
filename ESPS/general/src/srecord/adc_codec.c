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

static char *sccs_id = "@(#)adc_codec.c	1.1	11/10/92	ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>
#include <malloc.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sun/audioio.h>
#include <stropts.h>

#define BUF_SIZE 4000

int ad_done = FALSE, codec_sent = 0, error_at = -1, chan_config = 0;
static int maxsamps;
FILE *file_output = NULL;
void (*sig_save[3])();
extern int debug_level;
double amax, amin;
char *input_buffer;	          /* codec output buffer */
short *file_buffer;                /* mu-law covnerted input_buffer */

static int Audio_fdr;                     /* read codec file descriptor */
static int Audio_fdw;                     /* write codec file descriptor */

/*---------------------------------------------------------------------*/
void
  stop_codec_ad(sig)
int sig;
{
  int err;
  if((err = ioctl(Audio_fdr, I_FLUSH, FLUSHRW)) < 0){
    fprintf(stderr, "Error flushing device\n");
    exit(-1);
  }
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d)\n",sig);
  ad_done = TRUE;
}

/*---------------------------------------------------------------------*/
void
  stop_codec_ad_send_xwaves(sig)
int sig;
{
  int err;
  extern int send_display_file;
  if((err = ioctl(Audio_fdr, I_FLUSH, FLUSHRW)) < 0){
    fprintf(stderr, "Error flushing device\n");
    exit(-1);
  }
  if(debug_level)
    fprintf(stderr,"Caught a send_xwaves signal (%d)\n",sig);
  ad_done = TRUE;
  send_display_file = TRUE;
}



/*---------------------------------------------------------------------*/
record_file(outfd, samples, srate, channel, sigmax, echo_to_da)
     FILE *outfd;
     int samples, channel;
     double *srate, *sigmax;
     int echo_to_da;
{
  int samples_left = 0, written = 0;
  char *board_code_path=NULL;
  extern char *pstring;
  extern int do_prompt;

  if(!(file_output = outfd)) {
    fprintf(stderr,"Null FILE passed to record_file!\n");
    return(0);
  }

  chan_config = channel;

  if((maxsamps = samples) <= 0) {
    printf("Bogus sample count sent to codec_record_file(%d)\n",samples);
    return(0);
  }

  if(debug_level)
    fprintf(stderr,
    "record_file:outfd=%x samples=%d sigmax=%f\n", outfd, samples, *sigmax);

  /* allocate data buffer */
  input_buffer = (char *) calloc((unsigned) BUF_SIZE, sizeof(char));
  spsassert(input_buffer != NULL, "adc_codec: Can't allocate input_buffer");

  file_buffer = (short*) calloc ((unsigned) BUF_SIZE, sizeof(short));
  spsassert(file_buffer != NULL, "adc_codec: Can't allocate file_buffer");

  /* Install a signal handler to deal with interrupts and quits. */
  sig_save[1] = signal( SIGINT, stop_codec_ad);
  sig_save[2] = signal( SIGQUIT, stop_codec_ad);
  sig_save[3] = signal( SIGUSR1, stop_codec_ad_send_xwaves);
  maxsamps = samples;
  *sigmax = amax = amin = 0.0;
  error_at = -1;		/* if >= 0, here is where a problem occurred */

  ad_done = FALSE;

  /* open audio device and initialize */
  if(( Audio_fdr = open("/dev/audio", O_RDONLY)) < 0 ){
    fprintf(stderr,"adc_codec: Can't open A/D codec\n");
    exit(-1);
  }
  if(echo_to_da)
    if(( Audio_fdw = open("/dev/audio", O_WRONLY))< 0){
      fprintf(stderr,"adc_codec: Can't open D/A codec\n");
      exit(-1);
    }

  if(do_prompt && pstring && *pstring) /* optional user-start prompt */
    fprintf(stderr, "%s\n",pstring);

  while((!ad_done) && (error_at < 0)) {
    codec_handle(echo_to_da);
  }
  if((*sigmax = amax) < -amin)
    *sigmax = -amin;

  close(Audio_fdr);
  close(Audio_fdw);

  /* Restore the original signal handlers. */
  signal(SIGINT,sig_save[1]);
  signal(SIGQUIT,sig_save[2]);
  signal(SIGUSR1,sig_save[3]);

  if(error_at >= 0)
    return(error_at);
  else
    return(codec_sent);

}

/*************************************************************************/
get_buff_maxmin(buffer,samples,chans,smax,smin)
     int chans;
     register short *buffer;
     register long samples;
     double *smax, *smin;
{
  if(samples > 0) {
    register int lmax = *smax, lmin = *smin, t;

    if(chans > 1)			/* 0 ==> stereo */
      samples *= 2;
    
    do {
      if((t = *buffer++) > lmax)
	lmax = t;
      else
	if(t < lmin) lmin = t;
    } while( --samples );
    *smax = lmax;
    *smin = lmin;
  }
}


/*************************************************************************/
static codec_handle(echo_to_da)
int echo_to_da;
{
  long canread = BUF_SIZE;   /* samples left to read */
  long got=0;                /* samples read so far */
  long toread;               /* samples to read for this frame */
  long wrote;                /* samples to write */
  long i,error;
  
  if((canread+codec_sent) > maxsamps) canread = maxsamps - codec_sent;

  while(canread > 0) {
    if((toread = canread) > BUF_SIZE)
      toread = BUF_SIZE;
    toread = read(Audio_fdr, input_buffer, toread * sizeof(char));
    if(echo_to_da) write(Audio_fdw, input_buffer, toread *sizeof(char));

    mu_to_linear(input_buffer, file_buffer, toread);
    get_buff_maxmin(file_buffer,toread,chan_config,&amax,&amin);
    if((wrote = fwrite(file_buffer, sizeof(short), toread, file_output)) != toread) {
      fprintf(stderr,"Problems writing samples at sample number %d\n",codec_sent);
      error_at = codec_sent;
      return(got);
    }
    got += toread;
    canread -= toread;
    codec_sent += toread;
  }
    
  if(codec_sent >= maxsamps) {	/* Got all the samples requested? */
    if(debug_level)
      fprintf(stderr,"codec_handle(): reached requested sample count.\n");
    ad_done = TRUE;
  }
  return(got);
}

