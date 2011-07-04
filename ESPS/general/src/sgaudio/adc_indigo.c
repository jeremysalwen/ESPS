
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)adc_indigo.c	1.1 4/4/96 ERL";

/* adc_indigo.c */

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>
#include <Objects.h>
#include <malloc.h>
#include <audio.h>

#define BUF_SIZE 8192

int input_buffer[BUF_SIZE];	/* working buffer for sample output and max/min */

int ad_done = FALSE, sent = 0, error_at = -1, chan_config = 0;
static int  obuffersize, sizeof_sample = 2, maxsamps;
FILE *output = NULL;
ALport alport = NULL;		/* Audio library port for output. */
void (*sig_save[3])();
extern int debug_level;
double amax, amin;

/*---------------------------------------------------------------------*/
void
  stop_indigo_ad(sig)
int sig;
{
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d)\n",sig);
  ad_done = TRUE;
}

/*---------------------------------------------------------------------*/
void
  stop_indigo_ad_send_xwaves(sig)
int sig;
{
  extern int send_display_file;

  if(debug_level)
    fprintf(stderr,"Caught a send_xwaves signal (%d)\n",sig);
  ad_done = TRUE;
  send_display_file = TRUE;
}

/*---------------------------------------------------------------------*/
record_file(outfd, samples, srate, channel, sigmax)
     FILE *outfd;
     int samples, channel;
     double *srate, *sigmax;
{
  int fd, num_out, num_to_pass, num_trans;
  int sleep_time, smallest_chunk;
  int samples_left = 0, written = 0;
  char  *path = NULL;
  extern char *pstring;
  extern int do_prompt;
  short *buf = NULL;
  double req_rate = *srate * 1000.0;
#if !defined(SG)
  return(0);
#else
  static ALconfig audio_port_config = NULL;
  long pvbuf[4], buflen, samps_per_frame, frames_per_sec;

  if(!(output = outfd)) {
    fprintf(stderr,"Null FILE passed to record_file!\n");
    return(0);
  }
  if(!sg_audio_is_available()) {
    fprintf(stderr,"Sorry, this record program doesn't work on this machine.\n");
    return(0);
  }

  if(channel < 2)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);

  samps_per_frame = sizeof_sample/sizeof(short);

  chan_config = channel;

  if((maxsamps = samples) <= 0) {
    printf("Bogus sample count sent to indigo_record_file(%d)\n",samples);
    return(0);
  }

  /* Set the sample rate. */
  pvbuf[0] = AL_INPUT_RATE;
  pvbuf[1] = frames_per_sec = closest_srate(&req_rate);
  buflen   = 2;
  ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

  if(debug_level)
    fprintf(stderr,
    "record_file:outfd=%x samples=%d srate=%f channel=%d sigmax=%f frames_per_sec=%d\n",
	    outfd, samples, *srate, channel, *sigmax, frames_per_sec);
  /* Configure and open the audio port.    */
  if(!audio_port_config)
    audio_port_config = ALnewconfig();
  ALsetwidth(audio_port_config, AL_SAMPLE_16);
  ALsetchannels(audio_port_config, (channel < 2)? AL_MONO : AL_STEREO);
  ALsetqueuesize(audio_port_config, frames_per_sec); /* one-second buffer size */

  if(do_prompt && pstring && *pstring) /* optional user-start prompt */
    fprintf(stderr, "%s\n",pstring);

  /* Initialize the port.  This is probably where A/D actually starts. */
  if(!(alport = ALopenport("waves+ input", "r", audio_port_config))) {
    fprintf(stderr,"Can't open an Indigo audio input port.\n");
    return(-1);
  }
  obuffersize = ALgetfillable(alport);
  ad_done = FALSE;

  sleep_time = (100*obuffersize)/(frames_per_sec * 50); /* assumes SGI
						       clock at 100Hz and check
						       every 50th of a buffer. */

  /* Install a signal handler to deal with interrupts and quits. */
  sig_save[1] = signal( SIGINT, stop_indigo_ad);
  sig_save[2] = signal( SIGQUIT, stop_indigo_ad);
  sig_save[3] = signal( SIGUSR1, stop_indigo_ad_send_xwaves);
  maxsamps = samples;
  *sigmax = amax = amin = 0.0;
  ad_done = FALSE;
  error_at = -1;		/* if >= 0, here is where a problem occurred */

  while((!ad_done) && (error_at < 0)) {
    indigo_handle();
    sginap(sleep_time);
  }
  if((*sigmax = amax) < -amin)
    *sigmax = -amin;

  if(error_at >= 0)
    return(error_at);
  else
    return(sent);
#endif
}

/*************************************************************************/
get_buff_maxmin(buffer,samples,chans,smax,smin)
     register short *buffer, samples;
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
static indigo_handle(first_time)
     int first_time;
{
#if !defined(SG)
  return;
#else
  long canread, got=0, toread, wrote;
  
  /* Be sure that A/D hasn't lost real time. */
  if(ALgetfillable(alport) < 10) {
    fprintf(stderr,"Lost real time operation at sample %d\n",sent);
    error_at = sent;
    return(got);
  }
  canread = ALgetfilled(alport);
  if((canread+sent) > maxsamps) canread = maxsamps - sent;
  while(canread > 0) {
    if((toread = canread) > BUF_SIZE)
      toread = BUF_SIZE;
    ALreadsamps(alport, input_buffer, toread*(sizeof_sample/sizeof(short)));
    get_buff_maxmin(input_buffer,toread,chan_config,&amax,&amin);
    if((wrote = fwrite(input_buffer, sizeof_sample, toread, output)) != toread) {
      fprintf(stderr,"Problems writing samples at sample number %d\n",sent);
      error_at = sent;
      return(got);
    }
    got += toread;
    canread -= toread;
    sent += toread;
  }
    
  if(sent >= maxsamps) {	/* Got all the samples requested? */
    if(debug_level)
      fprintf(stderr,"indigo_handle(): reached requested sample count.\n");
    ad_done = TRUE;
  }
  return(got);
#endif
}
