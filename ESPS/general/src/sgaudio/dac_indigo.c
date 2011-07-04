/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* dac_indigo.c */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written By David Talkin
 *
 * Brief description: play routines for Indigo native DAC
 *
 */

#ifdef SG

static char *sccs_id = "@(#)dac_indigo.c	1.5	4/4/96	ATT/ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <esps/epaths.h>
#include <malloc.h>
#include <audio.h>

int indigo_maxsamps, indigo_sent=0, indigo_error_at;

extern int channel, debug_level;
static short *input_buffer = NULL;
static int buff_bytes = 0, buff_samps = 0, shifts, min_transfer;
extern int da_done, da_location;
static void *inhead=NULL;
static FILE *instream=NULL;
static int  infd = -1, obuffersize, sizeof_sample = 2, amax, amin;
static short *inbuff=NULL;

static int indigo_rates[] = {48000, 44100, 32000, 29400, 24000, 22050, 16000, 14700,
			       12000, 11025, 10667, 9800, 9600, 8820, 8000, 7350,
			       6857, 6400, 6300, 6000, 5880, 5512, 5333, 4900,
			       4571, 4200, 4000, 3675, 0};

static ALport alport = NULL;		/* Audio library port for output. */


/*************************************************************************/
close_alport()
{
  if(alport)
    ALcloseport(alport);
  alport = NULL;
}

/*************************************************************************/
/* Return gracefully if we're running on anything besides a
 * 4D/35 or an Indigo. */
int sg_audio_is_available()
{
  int fd;
  if ((fd = open("/dev/hdsp/hdsp0master", O_RDWR)) >= 0)   {
    close(fd);
    return(1);
  } else
    return(0);
}

/*************************************************************************/
static scale(data, nsamps, channel, shift)
     register int nsamps, channel, shift;
     short *data;
{
  if(nsamps > 0) {
    register short *to;

    to = data;
    if(!channel)		/* stereo */
      do {
	(*to++) <<= shift;

	(*to++) <<= shift;
      } while(--nsamps);
    else			/* mono */
      do {
	(*to++) <<= shift;
      } while(--nsamps);
  }
}

/*************************************************************************/
int da_samps_remaining()
{
  if(alport)
    return(ALgetfilled(alport));
  else
    return(0);
}

/*************************************************************************/
indigo_handle(not_first_call)
     int not_first_call;
{
  static int draining = -1;
  int canwrite, sams_out;

  if(indigo_sent >= indigo_maxsamps) { /* Wait for the last samples to drain. */
    if(draining < 0) {
      if(debug_level)
	fprintf(stderr,"indigo_handle(): draining D/A buffer\n");
      draining = 300;
    }
    if((draining-- > 0)  && ((sams_out = ALgetfilled(alport)) > 0)) {
#ifndef SGPLAY
      show_play_position(da_location - sams_out);
#endif
      return(0);
    } else {
      if(draining > 0)
#ifndef SGPLAY
	show_play_position(da_location);
#endif
      indigo_completion();
      return(indigo_sent);
    }
  } else
    draining = -1;
  
  /* Be sure that D/A hasn't lost real time. */
  canwrite = ALgetfillable(alport);
#ifndef SGPLAY
  show_play_position(da_location - obuffersize + canwrite);
#endif
  if(canwrite < min_transfer)
    return(0);
  if(not_first_call && (indigo_sent < indigo_maxsamps) &&
     (obuffersize <= canwrite)) {
    fprintf(stderr,"Lost real time operation at sample %d\n",indigo_sent);
    indigo_error_at = indigo_sent;
    indigo_completion();
    return(0);
  }
  if((sams_out = (indigo_maxsamps - indigo_sent)) > buff_samps)
    sams_out = buff_samps;
  if(sams_out > canwrite)
    sams_out = canwrite; /* return(0);	 Indicate no output was done. */
  if(debug_level)
    fprintf(stderr,"cw:%d ",canwrite);
  if(instream || (infd >= 0)) { /* samples from unix file? */
    int nread;

    if((nread = read_any_file(input_buffer, sizeof_sample, sams_out,
			      instream, inhead, infd)) != sams_out) {
      if(debug_level)
	fprintf(stderr,"Read less than requested at %d(%d:%d)\n",
        indigo_sent,sams_out,nread);
      sams_out = nread;
      indigo_maxsamps = indigo_sent+sams_out; /* To force buffer drain and termination. */
      if(debug_level)
	fprintf(stderr,"Reached file end at sample %d\n",indigo_sent);
    }
    scale(input_buffer,sams_out,channel,shifts);
  } else {			/* must come from a memory buffer */
    register int i, nshifts=shifts;
    register short *from, *to, smax, smin;

    if(channel)	{		/* mono */
      i=sams_out;
      to = input_buffer;
      from = inbuff;
      for(; i--; ) {
	*to++ = (*from++)<<nshifts;
      }
    } else {			/* stereo */
      for(i=sams_out,	to = input_buffer,
	  from = inbuff; i--; ) {
	*to++ = (*from++)<<nshifts;
	*to++ = (*from++)<<nshifts;
      }
    }
  }
  ALwritesamps(alport, input_buffer, sams_out*(sizeof_sample/sizeof(short)));
  indigo_sent += sams_out;
  da_location += sams_out;
  if(inbuff)
    inbuff += (sams_out * (sizeof_sample/sizeof(short)));
  return(sams_out);
}

/*************************************************************************/
int still_in_que()
{
  if(alport)
    return(ALgetfilled(alport));
  else
    return(0);
}

/*************************************************************************/
indigo_completion()
{

  if(!da_done) {
    if(debug_level) fprintf(stderr,"\n");
    da_done = TRUE;
    close_alport();
    clear_dac_callbacks();
  }

}

/*************************************************************************/
closest_srate(freq, rates)
     double *freq;
     register int *rates;
{
  if(freq && rates && (*rates > 0)) {
    register int i, ifr = *freq + 0.5, best, min;

    best = *rates++;
    min = abs(best-ifr);
    while(*rates) {
      if((i = abs(*rates - ifr)) < min) {
	best = *rates;
	if((min = i) == 0) break;
      }
      rates++;
    }
    if(best != ifr) {
      fprintf(stderr,"Closest rate available to that requested (%f) is %d\n",
	      *freq,best);
      *freq = best;
    }
    return(best);
  } else
    fprintf(stderr,"Bad args to closest_srate()\n");
  return(0);
}

/*************************************************************************/
dac_indigo(inputfile, inbuffer, sig_size, freq, sigmax)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
{
  return(dacmaster_indigo(inputfile, inbuffer, sig_size, freq,
			  sigmax, NULL, NULL));
}

/*************************************************************************/
dacmaster_indigo(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
     FILE *stream;
     void *eheader;
{
  static ALconfig audio_port_config = NULL;
  int pvbuf[4], buflen, samps_per_frame, frames_per_sec;

  if (!sg_audio_is_available())  {
    fprintf(stderr, 
	    "%s: can't play audio data on this hardware platform\n",
	    "dacmaster_indigo");
    return(-1);
  }

  indigo_sent = 0;
  indigo_error_at = -1;
  shifts = 0;
  if(!sigmax) sigmax = 32767;
  if(sigmax < 16384)
    while((sigmax << shifts) < 16384) shifts++;

  if(channel)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);

  samps_per_frame = sizeof_sample/sizeof(short);

  if((indigo_maxsamps = sig_size) <= 0) {
    printf("Bogus sample count sent to dac(%d)\n",sig_size);
    return(-1);
  }

  if((inputfile >= 0) || stream) {  /* input from either a file OR a memory buffer */
    infd = inputfile;
    instream = stream;
    inhead = eheader;
    inbuff = NULL;
  } else
    if(inbuffer) {
      infd = -1;
      inbuff = inbuffer;
      instream = NULL;
      inhead = NULL;
    } else {
      printf("Bogus input file and buffer specified to dacmaster_indigo()\n");
      return(-1);
    }
  
  /* Set the sample rate. */
  pvbuf[0] = AL_OUTPUT_RATE;
  pvbuf[1] = frames_per_sec = closest_srate(freq, indigo_rates);
  buflen   = 2;
  ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

  buflen = sizeof_sample * frames_per_sec; /* 1-second read buffer */
  if(buflen > buff_bytes) {
    if(buff_bytes) {
      free(input_buffer);
      input_buffer = NULL;
      buff_bytes = 0;
    }
    if(!(input_buffer = (short*)malloc(buflen))) {
      fprintf(stderr,"Allocation problems in dacmaster_indigo()\n");
      return(-1);
    }
    buff_bytes = buflen;
  }
  buff_samps = buff_bytes/sizeof_sample;

  /* Configure and open the audio port.    */
  if(!audio_port_config)
    audio_port_config = ALnewconfig();
  ALsetwidth(audio_port_config, AL_SAMPLE_16);
  ALsetchannels(audio_port_config, channel? AL_MONO : AL_STEREO);
  ALsetqueuesize(audio_port_config, frames_per_sec); /* one-second buffer size */
  /* Initialize the port */
  close_alport();       /* D/A may still be in progress. */
  if(!(alport = ALopenport("xwaves output","w",audio_port_config))) {
    fprintf(stderr,"Can't open an Indigo audio output port.\n");
    return(-1);
  }
  obuffersize = ALgetfillable(alport);
  min_transfer = obuffersize/10;
  da_done = FALSE;
  /* Send first buffer to dsp and start interrupts. */

  indigo_handle(0);
  if(!da_done)
    set_dac_callbacks(50000);	/* 50 ms timer interrupts */

  if(debug_level)
    fprintf(stderr,"tot_samples:%d D/A_buf_size:%d input_buffer:%x buff_bytes:%d frames_per_sec:%d buflen:%d sizeof_sample:%d\n",
	  sig_size, obuffersize,input_buffer,buff_bytes, frames_per_sec,buflen,sizeof_sample);

}

#else

/*************************************************************************/
int da_samps_remaining()
{
    return(0);
}


#endif
