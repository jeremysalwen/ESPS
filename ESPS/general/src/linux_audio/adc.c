
/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1987-1990  AT&T, Inc. "Copyright (c) 1986-1990  Entropic
 * Speech, Inc. "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc.
 * All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Checked by: Revised by:
 * 
 * Brief description:
 * 
 */

static char    *sccs_id = "@(#)adc.c	1.2 11/11/96 ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/audio.h>
#include <esps/ss.h>
#include <malloc.h>

#define BUF_SIZE 8192

int             input_buffer[BUF_SIZE];	/* working buffer for sample output
					 * and max/min */

int             ad_done = FALSE, sent = 0, error_at = -1, chan_config = 0;
static int      sizeof_sample = 2, maxsamps;
FILE           *output = NULL;
int             audio_port = NULL;
int             audio_port_ctl = NULL;
void            (*sig_save[3]) ();
extern int      debug_level;
double          amax, amin;
int             input_size;

/*************************************************************************/
void
get_buff_maxmin(buffer, samples, chans, smax, smin)
   register short *buffer, samples;
   double         *smax, *smin;
   int chans;
{
   if (samples > 0) {
      register int    lmax = *smax, lmin = *smin, t;

      if (chans > 1)		/* 0 ==> stereo */
	 samples *= 2;

      do {
	 if ((t = *buffer++) > lmax)
	    lmax = t;
	 else if (t < lmin)
	    lmin = t;
      } while (--samples);
      *smax = lmax;
      *smin = lmin;
   }
}


/*************************************************************************/
void
stop_ad(sig)
   int             sig;
{
   if (debug_level)
      (void) fprintf(stderr, "Caught a signal (%d)\n", sig);
   ad_done = TRUE;
}

/*************************************************************************/
void
stop_ad_send_xwaves(sig)
   int             sig;
{
   extern int      send_display_file;

   if (debug_level)
      (void) fprintf(stderr, "Caught a send_xwaves signal (%d)\n", sig);
   ad_done = TRUE;
   send_display_file = TRUE;
}

/*************************************************************************/
static void
handle()
{
   long            sample_count, wrote;

   sample_count = RecordAudio(input_buffer);
   if (sent + sample_count > maxsamps)
      sample_count = maxsamps - sent;
   get_buff_maxmin(input_buffer, sample_count, chan_config, &amax, &amin);
   if ((wrote = fwrite(input_buffer, sizeof(short), sample_count, output))
       != sample_count) {
      fprintf(stderr, "Problems writing samples at sample number %ld\n",
	      sent + wrote);
      error_at = sent + wrote;
      return;
   }
   sent += wrote;

   if (sent >= maxsamps) {	/* Got all the samples requested? */
      if (debug_level)
	 (void) fprintf(stderr, "handle(): reached requested sample count.\n");
      ad_done = TRUE;
   }
   return;
}

/*************************************************************************/
int
record_file(outfd, samples, srate, channel, sigmax)
   FILE           *outfd;
   int             samples, channel;
   double         *srate, *sigmax;
{
   extern char    *pstring;
   extern int      do_prompt;
   double          req_rate = *srate * 1000.0;
   long            buflen, frames_per_sec;
   int            *AudioGetRates();

   if (!(output = outfd)) {
      (void) fprintf(stderr, "Null FILE passed to record_file!\n");
      return (0);
   }
   if (PortIsAvailable()) {
      (void) fprintf(stderr, "Sorry, this record program requires audio hardware.\n");
      return (0);
   }
   if (channel < 2)
      sizeof_sample = sizeof(short);
   else
      sizeof_sample = sizeof(int);

   chan_config = channel;

   maxsamps = samples * channel;
   frames_per_sec = closest_srate(req_rate, AudioGetRates());
   *srate = (double)frames_per_sec/1000.0;
   input_size = frames_per_sec * sizeof_sample;
   buflen = 2;

   if (debug_level)
      (void) fprintf(stderr,
		     "record_file:outfd=%d samples=%d srate=%f channel=%d sigmax=%f frames_per_sec=%d\n",
		  outfd, samples, *srate, channel, *sigmax, frames_per_sec);

   if (do_prompt && pstring && *pstring)	/* optional user-start prompt */
      (void) fprintf(stderr, "%s\n", pstring);

   /* open the audio device */

   InitAudioRecord((double) frames_per_sec, channel, input_size);
   StartAudioRecord();


   ad_done = FALSE;

   /* Install a signal handler to deal with interrupts and quits. */
   sig_save[1] = signal(SIGINT, stop_ad);
   sig_save[2] = signal(SIGQUIT, stop_ad);
   sig_save[3] = signal(SIGUSR1, stop_ad_send_xwaves);
   *sigmax = amax = amin = 0.0;
   ad_done = FALSE;
   error_at = -1;		/* if >= 0, here is where a problem occurred */

   while ((!ad_done) && (error_at < 0)) {
      handle();
      E_usleep(10000);
   }
   CloseAudioRecord();
   if ((*sigmax = amax) < -amin)
      *sigmax = -amin;

   if (error_at >= 0)
      return (error_at);
   else
      return (sent);
}

