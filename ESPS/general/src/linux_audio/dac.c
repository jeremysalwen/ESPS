/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1986-1990  Entropic Speech, Inc. "Copyright (c) 1990-1993
 * Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written By David Talkin and Alan Parker
 * 
 * 
 */


static char    *sccs_id = "@(#)dac.c	1.3 5/6/97 ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <esps/epaths.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int             maxsamps, samps_sent = 0, samps_error_at;
static int channel = 1;
static int da_done = 1;

extern int      errno;
extern int      debug_level;
static short   *input_buffer = NULL;
static int      buff_bytes = 0, buff_samps = 0, shifts;
extern int      da_location;
static void    *inhead = NULL;
static FILE    *instream = NULL;
static int      infd = -1, obuffersize, sizeof_sample = 2, amax, amin;
static short   *inbuff = NULL;
void (*sig_save[3])();

/*************************************************************************/

static int
check_underflow()
{
   return 0;
}

static
scale(data, nsamps, channel, shift)
   register int    nsamps, channel, shift;
   short          *data;
{
   if (nsamps > 0) {
      register short *to;

      to = data;
      if (!channel) {		/* stereo */
	 do {
	    (*to++) <<= shift;

	    (*to++) <<= shift;
	 } while (--nsamps);
      } else			/* mono */
	 do {
	    (*to++) <<= shift;
	 } while (--nsamps);
   }
}

/*************************************************************************/
linux_handle(not_first_call, canwrite_arg)
   int             not_first_call;
   long            canwrite_arg;
{
   long            sams_out;
   static long     canwrite;

   if (not_first_call && (CanWriteAudio() < 100)) {
      return 0;
   }
   /* Be sure that D/A hasn't lost real time. */
   if (not_first_call && check_underflow()) {
      fprintf(stderr, "Lost real time operation at sample %d\n", samps_sent);
      samps_error_at = samps_sent;
      linux_completion();
      return (0);
   }
   if ((sams_out = (maxsamps - samps_sent)) > buff_samps)
      sams_out = buff_samps;

   if (instream || (infd >= 0)) {	/* samples from unix file? */
      int             nread;

      if ((nread = read_any_file(input_buffer, sizeof_sample, sams_out,
				 instream, inhead, infd)) != sams_out) {
	 if (debug_level)
	    fprintf(stderr, "Read error at %d(%d:%d)\n", samps_sent, sams_out, nread);
	 samps_error_at = samps_sent;
	 sams_out = nread;
	 maxsamps = samps_sent + sams_out;	/* To force buffer drain
							 * and termination. */
	 if (debug_level)
	    fprintf(stderr, "Reached file end at sample %d\n", samps_sent);
      }
      scale(input_buffer, sams_out, channel, shifts);
   } else {			/* must come from a memory buffer */
      register int    i, nshifts = shifts;
      register short *from, *to, smax, smin;

      if (channel) {		/* mono */
	 i = sams_out;
	 to = input_buffer;
	 from = inbuff;
	 for (; i--;) {
	    *to++ = (*from++) << nshifts;
	 }
      } else {			/* stereo */
	 for (i = sams_out, to = input_buffer,
	      from = inbuff; i--;) {
	    *to++ = (*from++) << nshifts;
	    *to++ = (*from++) << nshifts;
	 }
      }
   }
   PlayAudio(input_buffer, sams_out * (channel ? 1 : 2));
   samps_sent += sams_out;
   da_location += sams_out;
   if (inbuff)
      inbuff += (sams_out * (sizeof_sample / sizeof(short)));
   if (samps_sent >= maxsamps) {/* Wait for the last samples to drain. */
      {
	 short          *p1;
	 int             n = 0;
	 float           v1, v2, co = .9;
	 v1 = input_buffer[(sams_out - 1)*(sizeof_sample / sizeof(short))];
	 v2 = input_buffer[(sams_out - 1)*(sizeof_sample / sizeof(short)) + 1];
	 p1 = input_buffer;
	 while ((fabs(v1) > 1.0) || (fabs(v2) > 1.0)) {
	    v1 *= co;
	    v2 *= co;
	    *p1++ = v1;
	    *p1++ = v2;
	    n++;
	 }
	 if (n < 1) {
	    PlayAudio(input_buffer, n * (channel ? 1 : 2));
	 }
      }
      linux_completion();
   }
   return (sams_out);
}


/*************************************************************************/
linux_completion()
{
   if (!da_done) {
      da_done = TRUE;
      clear_dac_callbacks();
      PlayAudioDrain();
      CloseAudioPlay(0);
   }
}

/*************************************************************************/
dac_linux(inputfile, inbuffer, sig_size, freq, sigmax)
   int             inputfile, sig_size, sigmax;
   short          *inbuffer;
   double         *freq;
{
   return (dacmaster_linux(inputfile, inbuffer, sig_size, freq,
			   sigmax, NULL, NULL));
}

/*************************************************************************/
dacmaster_linux(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
   int             inputfile, sig_size, sigmax;
   short          *inbuffer;
   double         *freq;
   FILE           *stream;
   void           *eheader;
{
   long            buflen, samps_per_frame, frames_per_sec;

   CloseAudioPlay(0);

   if (PortIsAvailable() == 1) {
      fprintf(stderr,
	      "%s: can't play audio data on this hardware platform\n",
	      "dacmaster_linux");
      return (-1);
   }
   samps_sent = 0;
   samps_error_at = -1;
   shifts = 0;
   if (!sigmax)
      sigmax = 32767;
   if (sigmax < 16384)
      while ((sigmax << shifts) < 16384)
	 shifts++;

   if (channel)
      sizeof_sample = sizeof(short);
   else
      sizeof_sample = sizeof(int);

   samps_per_frame = sizeof_sample / sizeof(short);

   if ((maxsamps = sig_size) <= 0) {
      printf("Bogus sample count sent to dac(%d)\n", sig_size);
      return (-1);
   }
   if ((inputfile >= 0) || stream) {	/* input from either a file OR a
					 * memory buffer */
      infd = inputfile;
      instream = stream;
      inhead = eheader;
      inbuff = NULL;
   } else if (inbuffer) {
      infd = -1;
      inbuff = inbuffer;
      instream = NULL;
      inhead = NULL;
   } else {
      printf("Bogus input file and buffer specified to dacmaster_linux()\n");
      return (-1);
   }

   frames_per_sec = *freq;
   buflen = sizeof_sample * frames_per_sec * .5;	/* 2-second read buffer */


   if (buflen > buff_bytes) {
      if (buff_bytes) {
	 free((char *) input_buffer);
	 input_buffer = NULL;
	 buff_bytes = 0;
      }
      if (!(input_buffer = (short *) malloc(buflen))) {
	 fprintf(stderr, "Allocation problems in dacmaster_linux() %d\n", buflen);
	 return (-1);
      }
      buff_bytes = buflen;
   }
   buff_samps = buff_bytes / sizeof_sample;

   InitAudioPlay(*freq, channel ? 1 : 2, buff_samps);
   StartAudioPlay();

   da_done = FALSE;

   set_dac_callbacks();
   linux_handle(0, buff_samps);

   if (debug_level)
      fprintf(stderr, "tot_samples:%d D/A_buf_size:%d input_buffer:%x buff_bytes:%d frames_per_sec:%d buflen:%d sizeof_sample:%d\n",
	      sig_size, obuffersize, input_buffer, buff_bytes, frames_per_sec, buflen, sizeof_sample);

}

int
linux_samps_sent()
{
   int             samps_played = samps_sent - GetPlayFilled();
   if (samps_played < 0)
      return 0;
   else
      return samps_played;
}


play_from_file(istr, length, srate, chan, amp, ehead)
     FILE *istr;
     double *srate;
     int chan,	 /* -1==>stereo; else chan == (channel number to play) */
       amp;	/* Max sample value in the file or region. */
     void *ehead;
{
  double frequency = *srate * 1000.0;

  channel = (chan < 0)? 0 : channel+1;
  dacmaster_linux(-1, NULL, length, &frequency, amp, istr, ehead);
  *srate = frequency/1000.0;

  while(!da_done && (samps_error_at < 0)) {
    
    if(!linux_handle(1,-1))	/* returns 0 if couldn't write a bufferfull */
      usleep(10000);		/* sleep 10 ms */
  }
  
  linux_completion();
  if(samps_error_at >= 0)
    return(samps_error_at);
  else
    return(samps_sent);
}


/*---------------------------------------------------------------------*/
void stop_linux_da_waves(sig)
     int sig;
{
  int number_left;
  extern char *display_name, *waves_name;

  da_location -= linux_samps_sent();

  number_left = maxsamps - samps_sent;

  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); number_left:%d\n",
	    sig, number_left);
  if(number_left > 0) {
    char str[200];
    sprintf(str,"set da_location %d\n", da_location);
    SendXwavesNoReply(display_name, waves_name, NULL, str);
    /* This pause seems to be necessary to prevent the possibility that
       the child's death will be registered before the send_xwaves is! */
    usleep(20000);
  }
  exit(0);
}

/*************************************************************************/
void interrupt_linux_da(sig)
     int sig;
{
  samps_error_at = samps_sent;
  linux_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); da_location:%d\n",
	    sig, da_location);
  exit(0);
}

set_dac_callbacks()
{

  if (debug_level) fprintf(stderr,"setting signals\n");
  /* Setup the signal handlers. */
  sig_save[0] = signal( SIGINT, interrupt_linux_da);
  sig_save[1] = signal( SIGQUIT, interrupt_linux_da);
  sig_save[2] = signal( SIGUSR1, stop_linux_da_waves);
}

clear_dac_callbacks()
{
  /* Restore the original signal handlers. */
  if (debug_level) fprintf(stderr,"clearing signals\n");
  signal(SIGINT,sig_save[0]);
  signal(SIGQUIT,sig_save[1]);
  signal(SIGUSR1,sig_save[2]);
}
