
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

static char    *sccs_id = "@(#)adc_hpaudiolib.c	1.3 7/25/96 ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>
#include <Objects.h>
#include <malloc.h>
#include <audio/Alib.h>

#define BUF_SIZE 4096

int             input_buffer[BUF_SIZE];	/* working buffer for sample output
					 * and max/min */

int             ad_done = FALSE, sent = 0, error_at = -1, chan_config = 0;
static int      sizeof_sample = 2, maxsamps;
FILE           *output = NULL;
void            (*sig_save[3]) ();
extern int      debug_level;
double          amax, amin;
int             hp_input_size;
int             hp_sampling_rates[32];
int             streamSocket;

/*---------------------------------------------------------------------*/
void
stop_hp_ad(sig)
   int             sig;
{
   if (debug_level)
      (void) fprintf(stderr, "Caught a signal (%d)\n", sig);
   ad_done = TRUE;
}

/*---------------------------------------------------------------------*/
void
stop_hp_ad_send_xwaves(sig)
   int             sig;
{
   extern int      send_display_file;

   if (debug_level)
      (void) fprintf(stderr, "Caught a send_xwaves signal (%d)\n", sig);
   ad_done = TRUE;
   send_display_file = TRUE;
}

/*---------------------------------------------------------------------*/
closest_srate(freq, rates)
   double         *freq;
   register int   *rates;
{
   if (freq && rates && (*rates > 0)) {
      register int    i, ifr = *freq + 0.5, best, min;

      best = *rates++;
      min = abs(best - ifr);
      while (*rates) {
	 if ((i = abs(*rates - ifr)) < min) {
	    best = *rates;
	    if ((min = i) == 0)
	       break;
	 }
	 rates++;
      }
      if (best != ifr) {
	 fprintf(stderr, "Closest rate available to that requested (%f) is %d\n",
		 *freq, best);
	 *freq = best;
      }
      return (best);
   } else
      fprintf(stderr, "Bad args to closest_srate()\n");
   return (0);
}
/*---------------------------------------------------------------------*/
record_file(outfd, samples, srate, channel, sigmax)
   FILE           *outfd;
   int             samples, channel;
   double         *srate, *sigmax;
{
   int             fd, num_out, num_to_pass, num_trans;
   int             smallest_chunk, i, j;
   int             samples_left = 0, written = 0;
   char           *path = NULL;
   extern char    *pstring;
   extern int      do_prompt;
   short          *buf = NULL;
   double          req_rate = *srate * 1000.0;
   long            buflen, samps_per_frame, frames_per_sec;
   int             junk, junk1, junk3;
   unsigned long  *rates;
   AudioAttrMask   AttribsMask;
   AudioAttributes Attribs;
   long            status;
   SSRecordParams  streamParams;
   Audio          *audio;
   ATransID        xid;
   SStream         audioStream;
   struct timeval  sleep_time;
   AGainEntry      gainEntry[4];
   int             useLineIn = 0;

   int             input_port;
   char           *getenv();
   extern	   Iflag;

   useLineIn = Iflag;
   AttribsMask = 0;

   if (!(output = outfd)) {
      (void) fprintf(stderr, "Null FILE passed to record_file!\n");
      return (0);
   }
   audio = AOpenAudio(NULL, NULL);

   rates = ASamplingRates(audio);

   for (i = ANumSamplingRates(audio) - 1, j = 0; i >= 0; i--)
      hp_sampling_rates[j++] = rates[i];

   hp_sampling_rates[j] = 0;
   Attribs = *aBestAudioAttributes(audio);
   frames_per_sec = Attribs.attr.sampled_attr.sampling_rate =
      closest_srate(&req_rate, hp_sampling_rates);
   if (channel < 2)
      sizeof_sample = sizeof(short);
   else
      sizeof_sample = sizeof(int);

   samps_per_frame = sizeof_sample / sizeof(short);

   chan_config = channel;

   if ((maxsamps = samples) <= 0) {
      printf("Bogus sample count sent to hp_record_file(%d)\n", samples);
      return (0);
   }
   /* Set up defaults in the HP audio attrib structure */
   AttribsMask = (AttribsMask | ASSamplingRateMask | ASChannelsMask |
		  ASDataFormatMask);
   Attribs.attr.sampled_attr.channels = (channel < 2) ? 1 : 2;
   Attribs.attr.sampled_attr.data_format = ADFLin16;
   Attribs.attr.sampled_attr.bits_per_sample = 16;
   /*
    * setup the record parameters
    */
   switch (Attribs.attr.sampled_attr.channels) {
   case 1:
      gainEntry[0].u.i.in_ch = AICTMono;
      gainEntry[0].gain = AUnityGain;
      gainEntry[0].u.i.in_src = (useLineIn) ? AISTMonoAuxiliary : AISTMonoMicrophone;
      break;
   case 2:
   default:			/* assume no more than 2 channels */
      gainEntry[0].u.i.in_ch = AICTLeft;
      gainEntry[0].gain = AUnityGain;
      gainEntry[0].u.i.in_src = (useLineIn) ? AISTLeftAuxiliary : AISTLeftMicrophone;
      gainEntry[1].u.i.in_ch = AICTRight;
      gainEntry[1].gain = AUnityGain;
      gainEntry[1].u.i.in_src = (useLineIn) ? AISTRightAuxiliary : AISTRightMicrophone;
      break;
   }
   streamParams.gain_matrix.type = AGMTInput;
   streamParams.gain_matrix.num_entries = Attribs.attr.sampled_attr.channels;
   streamParams.gain_matrix.gain_entries = gainEntry;
   streamParams.record_gain = AUnityGain;	/* record gain */
   streamParams.event_mask = 0;	/* don't solicit any events */

   buflen = 2;

   if (debug_level)
      (void) fprintf(stderr,
		     "record_file:outfd=%x samples=%d srate=%f channel=%d sigmax=%f frames_per_sec=%d\n",
		  outfd, samples, *srate, channel, *sigmax, frames_per_sec);

   if (do_prompt && pstring && *pstring)	/* optional user-start prompt */
      (void) fprintf(stderr, "%s\n", pstring);

   /* Initialize the port.  This is probably where A/D actually starts. */

   /* open the audio device */

   xid = ARecordSStream(audio, ~0, &Attribs, &streamParams,
			&audioStream, NULL);

   streamSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (streamSocket < 0) {
      perror("Socket creation failed");
      exit(1);
   }
   status = connect(streamSocket, (struct sockaddr *) & audioStream.tcp_sockaddr,
		    sizeof(struct sockaddr_in));
   if (status < 0) {
      perror("Connect failed");
      exit(1);
   }
   ad_done = FALSE;

   sleep_time.tv_sec = 0;
   sleep_time.tv_usec = (1000000.0) / (frames_per_sec * 50);

   /* Install a signal handler to deal with interrupts and quits. */
   sig_save[1] = signal(SIGINT, stop_hp_ad);
   sig_save[2] = signal(SIGQUIT, stop_hp_ad);
   sig_save[3] = signal(SIGUSR1, stop_hp_ad_send_xwaves);
   maxsamps = samples;
   *sigmax = amax = amin = 0.0;
   ad_done = FALSE;
   error_at = -1;		/* if >= 0, here is where a problem occurred */

   AResumeAudio(audio, xid, NULL, NULL);
   while ((!ad_done) && (error_at < 0)) {
      hp_handle();
   }
   if ((*sigmax = amax) < -amin)
      *sigmax = -amin;

   if (error_at >= 0)
      return (error_at);
   else
      return (sent);
}

/*************************************************************************/
void
get_buff_maxmin(buffer, samples, chans, smax, smin)
   register short *buffer, samples;
   double         *smax, *smin;
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
static 
hp_handle()
{
   long            got = 0, toread, wrote, bytes, samps;
   int             i, ret;
   static struct timeval timeout = {0L, 0L};
   static fd_set   fds;


   FD_ZERO(&fds);
   FD_SET(streamSocket, &fds);
   if (select(streamSocket + 1, &fds, (fd_set *) 0, (fd_set *) 0, &timeout)) {
      toread = (sent + BUF_SIZE > maxsamps) ? maxsamps - sent : BUF_SIZE / sizeof_sample;
      bytes = read(streamSocket, input_buffer, (size_t) toread * sizeof_sample);
      samps = bytes / sizeof_sample;
      get_buff_maxmin(input_buffer, samps, chan_config, &amax, &amin);
      if ((wrote = fwrite(input_buffer, sizeof_sample, samps, output)) != samps) {
	 fprintf(stderr, "Problems writing samples at sample number %d\n", sent);
	 error_at = sent;
	 return (got);
      }
      got += samps;
      sent += samps;

      if (sent >= maxsamps) {	/* Got all the samples requested? */
	 if (debug_level)
	    (void) fprintf(stderr, "hp_handle(): reached requested sample count.\n");
	 ad_done = TRUE;
      }
   }
   return (got);
}
