/* dac.c */

/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1995 Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Checked by: Revised by:
 * 
 * Brief description:
 * 
 */

static char    *sccs_id = "@(#)dac.c	1.8 10/30/96 ERL/ATT";

#include <Objects.h>
#include <sys/ioctl.h>
#include <esps/epaths.h>

int             indigo_maxsamps, indigo_sent = 0, indigo_error_at;

extern int      channel, debug_level, hold_audio_port;
static int      numChannel;
static short   *input_buffer = NULL;
static int      buff_bytes = 0, buff_samps = 0, shifts, min_transfer;
extern int      da_done, da_location;
static void    *inhead = NULL;
static FILE    *instream = NULL;
static int      infd = -1, obuffersize, sizeof_sample = 2, amax, amin;
static short   *inbuff = NULL;
extern int      play_buff_factor; /* from globals.c */


extern char     audio_error_msg[];

#if defined(HP700) || defined(LINUX_OR_MAC)
#define DRAIN_LIMIT 50
#else
#define DRAIN_LIMIT 0
#endif

/*************************************************************************/
close_alport()
{
   CloseAudioPlay(hold_audio_port);
}

/*************************************************************************/
int
sg_audio_is_available()
{
   if (PortIsAvailable() == 1) {
      fprintf(stderr, "%s\n", audio_error_msg);
      return 0;
   } else
      return 1;
}

/*************************************************************************/
static
scale(data, nsamps, channel, shift)
   register int    nsamps, channel, shift;
   short          *data;
{
   ScaleData(data, NULL, nsamps, channel, 0, shift, 0);
}

/*************************************************************************/
int
da_samps_remaining()
{
   return (GetPlayFilled() / numChannel);
}

/*************************************************************************/
indigo_handle(not_first_call)
   int             not_first_call;
{
   static int      draining = -1;
   int             canwrite, sams_out;
   extern	   int stop_play_error;

   if (debug_level) fprintf(stderr,"in _handle\n");
   if (indigo_sent >= indigo_maxsamps) {
      if (draining < 0) {
	 if (debug_level)
	    fprintf(stderr, "_handle(): draining D/A buffer\n");
	 draining = 300;
/* this is called to write zeros to the audio device to keep data flowing
   during the drain.  On Linux and HP this is required, otherwise the
   playback stops before all data has drained from the buffer.  And no,
   the close operation doesn't drain the data either.   For other archs
   this function should do nothing.
*/
#if defined(HP700) || defined(LINUX_OR_MAC)
	 SendAudioZeros();
#endif
      }
      if ((draining-- > 0) && ((sams_out = GetPlayFilled() / numChannel) > DRAIN_LIMIT)) {
         if(debug_level) fprintf(stderr,"draining: %d\n",draining);
	 show_play_position(da_location - sams_out);
#if defined(HP700) || defined(LINUX_OR_MAC)
	 SendAudioZeros();
#endif
	 return (0);
      } else {
	 if (draining > 0)
	    show_play_position(da_location);
	 PlayAudioDrain();
	 indigo_completion();
	 return (indigo_sent);
      }
   } else
      draining = -1;

   /* Be sure that D/A hasn't lost real time. */
   canwrite = CanWriteAudio() / numChannel;
   show_play_position(da_location - obuffersize + canwrite);
   if (canwrite < min_transfer)
      return (0);
   if (not_first_call && (indigo_sent < indigo_maxsamps) &&
       (obuffersize <= canwrite)) {
      sprintf(notice_msg,
	      "Lost real time operation at sample %d\n", indigo_sent);
      if (stop_play_error) {
      	show_notice(1, notice_msg);
      	indigo_error_at = indigo_sent;
      	indigo_completion();
      	return (0);
      } else
        fputs(notice_msg,stderr);
   }
   if ((sams_out = (indigo_maxsamps - indigo_sent)) > obuffersize / numChannel)
      sams_out = obuffersize / numChannel;
   if (sams_out > canwrite)
      sams_out = canwrite;	/* return(0);   Indicate no output was done. */
   if (debug_level)
      fprintf(stderr, "cw:%d ", canwrite);
   if (instream || (infd >= 0)) {	/* samples from unix file? */
      int             nread;
      if ((nread = read_any_file(input_buffer, sizeof_sample, sams_out,
				 instream, inhead, infd)) != sams_out) {
	 if (debug_level)
	    fprintf(stderr, "Read error at %d(%d:%d)\n", indigo_sent, sams_out, nread);
	 indigo_error_at = indigo_sent;
	 sams_out = nread;
	 indigo_maxsamps = indigo_sent + sams_out;	/* To force buffer drain
							 * and termination. */
	 if (debug_level)
	    fprintf(stderr, "Reached file end at sample %d\n", indigo_sent);
      }
      scale(input_buffer, sams_out, numChannel, shifts);
   } else {			/* must come from a memory buffer */
      register int    i, nshifts = shifts;
      register short *from, *to, smax, smin;

      if (numChannel) {		/* mono */
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
   PlayAudio(input_buffer, sams_out * numChannel);
   indigo_sent += sams_out;
   da_location += sams_out;
   if (inbuff)
      inbuff += sams_out;
   return (sams_out);
}

/*************************************************************************/
int
still_in_que()
{
   return (GetPlayFilled() / numChannel);
}

/*************************************************************************/
indigo_completion()
{
   if (!da_done) {
      if (debug_level)
	 fprintf(stderr, "**completion**\n");
      da_done = TRUE;
      CloseAudioPlay(hold_audio_port);
      clear_dac_callbacks();
      free(input_buffer);
   }
}

/*************************************************************************/
dac_indigo(inputfile, inbuffer, sig_size, freq, sigmax)
   int             inputfile, sig_size, sigmax;
   short          *inbuffer;
   double         *freq;
{
   return (dacmaster_indigo(inputfile, inbuffer, sig_size, freq,
			    sigmax, NULL, NULL));
}

/*************************************************************************/
dacmaster_indigo(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
   int             inputfile, sig_size, sigmax;
   short          *inbuffer;
   double         *freq;
   FILE           *stream;
   void           *eheader;
{
   if (PortIsAvailable()) {
      fprintf(stderr, "%s\n", audio_error_msg);
      return (-1);
   }
   numChannel = (channel == 0) ? 2 : 1;
   indigo_sent = 0;
   indigo_error_at = -1;
   shifts = 0;
   if (!sigmax)
      sigmax = 32767;
   if (sigmax < 16384)
      while ((sigmax << shifts) < 16384)
	 shifts++;

   sizeof_sample = sizeof(short) * numChannel;

   CloseAudioPlay(hold_audio_port);
   obuffersize = ((int) *freq) / (2 * numChannel);
   if (play_buff_factor > 0)
      obuffersize = obuffersize*play_buff_factor;
   if (InitAudioPlay(*freq, numChannel, obuffersize) == 1) {
      show_notice(1, audio_error_msg);
      return (-1);
   }
   if ((indigo_maxsamps = sig_size) <= 0) {
      fprintf(stderr, "Bogus sample count sent to dac(%d)\n", sig_size);
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
      fprintf(stderr, "Bogus input file and buffer specified to dacmaster_()\n");
      return (-1);
   }

   if (!(input_buffer = (short *) malloc(obuffersize * sizeof(short)))) {
      fprintf(stderr, "Allocation problems in dacmaster_()\n");
      return (-1);
   }
   if (StartAudioPlay() == 1) {
      show_notice(1, audio_error_msg);
      return (-1);
   }
   min_transfer = obuffersize / (10 * numChannel);
   da_done = FALSE;
   /* Send first buffer to dsp and start interrupts. */

   indigo_handle(0);
   if (!da_done)
      set_dac_callbacks(10000);	/* 10 ms timer interrupts */

}
