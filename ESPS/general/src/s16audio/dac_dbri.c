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
 * Written By David Talkin Alan Parker
 *
 * Brief description: play routines for sparc dbri
 *
 */


static char *sccs_id = "@(#)dac_dbri.c	1.2 11/15/95 ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <esps/epaths.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef OS5
#include <std.h>
#include <sys/audioio.h>
#else
#include <sun/audioio.h>
#endif
int sparc_maxsamps, sparc_sent=0, sparc_error_at;

#define AUDIO_DEV_CS4231	(5)

extern int errno;

extern int channel, debug_level;
static short *input_buffer = NULL;
static int buff_bytes = 0, buff_samps = 0, shifts;
extern int da_done, da_location;
static void *inhead=NULL;
static FILE *instream=NULL;
static int  infd = -1, obuffersize, sizeof_sample = 2, amax, amin;
static short *inbuff=NULL;

static int sparc_rates[] = {48000, 44100, 37800, 32000,  22050, 18900,
				16000, 11025, 9600, 8000, 0};

int audio_port = -1;		/* audio port */
int audio_port_ctl = -1;	/* control port */
audio_info_t audio_info;

#ifndef MAX_AUDIO_DEV_LEN
/*
 * Paramter for the AUDIO_GETDEV ioctl to determine current audio device
*/
#define MAX_AUDIO_DEV_LEN       (16)
typedef struct audio_device {
        char    name[MAX_AUDIO_DEV_LEN];
        char    version[MAX_AUDIO_DEV_LEN];
        char    config[MAX_AUDIO_DEV_LEN];
} audio_device_t;
#endif

#ifndef AUDIO_GETDEV
#define AUDIO_GETDEV    _IOR(A, 4, audio_device_t)
#endif

#define AUDIOCTL "/dev/audioctl"
#define AUDIODEV "/dev/audio"

/*************************************************************************/
close_alport()
{
  if(audio_port >= 0) {
    ioctl(audio_port,AUDIO_DRAIN, NULL);
    close(audio_port);
  }
  if(audio_port_ctl >= 0)
    close(audio_port_ctl);

  audio_port = -1;
  audio_port_ctl = -1;
}

/*************************************************************************/
int sparc_audio_is_available()
{
	if ((audio_port_ctl = open(AUDIOCTL,O_RDWR)) < 0) {
	 fprintf(stderr,"Cannot open Sparc audio\n");
	 return 0;
	}
  	if((audio_port = open("/dev/audio", O_WRONLY)) < 0) {
	 perror("Cannot open audio device for output");
	 return(0);
	}
	return 1;
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
sparc_handle(not_first_call, canwrite_arg)
     int not_first_call;
     long canwrite_arg;
{
  long  sams_out;
  static long canwrite;
  
  if(canwrite_arg > 0)
	canwrite = canwrite_arg;
  /* Be sure that D/A hasn't lost real time. */
  if(not_first_call && check_sparc_underflow()) {
    fprintf(stderr,"Lost real time operation at sample %d\n",sparc_sent);
    sparc_error_at = sparc_sent;
    sparc_completion();
    return(0);
  }
  if((sams_out = (sparc_maxsamps - sparc_sent)) > buff_samps)
    sams_out = buff_samps;

  if(instream || (infd >= 0)) { /* samples from unix file? */
    int nread;

    if((nread = read_any_file(input_buffer, sizeof_sample, sams_out,
			      instream, inhead, infd)) != sams_out) {
      if(debug_level)
	fprintf(stderr,"Read error at %d(%d:%d)\n",sparc_sent,sams_out,nread);
      sparc_error_at = sparc_sent;
      sams_out = nread;
      sparc_maxsamps = sparc_sent+sams_out; /* To force buffer drain and termination. */
      if(debug_level)
	fprintf(stderr,"Reached file end at sample %d\n",sparc_sent);
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
  write(audio_port, (char *)input_buffer, sams_out*(sizeof_sample));
/*  ioctl(audio_port,AUDIO_DRAIN, NULL); */
  if(errno <0) {
	perror("Error writing audio samples");
	return (-1);
  }
  sparc_sent += sams_out;
  da_location += sams_out;
  if(inbuff)
    inbuff += (sams_out * (sizeof_sample/sizeof(short)));
  if(sparc_sent >= sparc_maxsamps) { /* Wait for the last samples to drain. */
    {
      short *p1;
      int n = 0;
      float v1, v2, co = .9;
      v1 = input_buffer[(sams_out-1)*(sizeof_sample/sizeof(short))];
      v2 = input_buffer[(sams_out-1)*(sizeof_sample/sizeof(short)) + 1];
      p1 = input_buffer;
      while(( fabs(v1) > 1.0) || (fabs(v2) > 1.0)) {
	v1 *= co;
	v2 *= co;
	*p1++ = v1;
	*p1++ = v2;
	n++;
      }
      if(n < 1) {
	write(audio_port, (char *)input_buffer, n*(sizeof_sample));
      }
    }
    sparc_completion();
  }
  return(sams_out);
}


/*************************************************************************/
sparc_completion()
{
  if(!da_done) {
    da_done = TRUE;
    clear_dac_callbacks();
    close_alport();
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
dac_sparc(inputfile, inbuffer, sig_size, freq, sigmax)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
{
  return(dacmaster_sparc(inputfile, inbuffer, sig_size, freq,
			  sigmax, NULL, NULL));
}

/*************************************************************************/
dacmaster_sparc(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
     FILE *stream;
     void *eheader;
{
  long buflen, samps_per_frame, frames_per_sec;

  close_alport();       /* D/A may still be in progress. */

  if (!sparc_audio_is_available())  {
    fprintf(stderr, 
	    "%s: can't play audio data on this hardware platform\n",
	    "dacmaster_sparc");
    return(-1);
  }

  sparc_sent = 0;
  sparc_error_at = -1;
  shifts = 0;
  if(!sigmax) sigmax = 32767;
  if(sigmax < 16384)
    while((sigmax << shifts) < 16384) shifts++;

  if(channel)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);

  samps_per_frame = sizeof_sample/sizeof(short);

  if((sparc_maxsamps = sig_size) <= 0) {
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
      printf("Bogus input file and buffer specified to dacmaster_sparc()\n");
      return(-1);
    }



  /* Set the sample rate. */
  AUDIO_INITINFO(&audio_info);
  audio_info.play.sample_rate = 
		frames_per_sec = closest_srate(freq, sparc_rates); 
  audio_info.play.precision = 16;
  audio_info.play.channels = channel? 1 : 2;
  audio_info.play.encoding = 3; /* LINEAR */
  audio_info.play.error = 0;
  audio_info._yyy[0] = 0;

  
  if(ioctl(audio_port, AUDIO_SETINFO, &audio_info) < 0) {
	perror("error setting audio port mode");
	exit(1);
  }
/*  usleep(100000);*/		/* Let the stupid analog switch settle. */

  buflen = sizeof_sample * frames_per_sec * .5; /* 2-second read buffer */
  if(buflen > buff_bytes) {
    if(buff_bytes) {
      free(input_buffer);
      input_buffer = NULL;
      buff_bytes = 0;
    }
    if(!(input_buffer = (short*)malloc(buflen))) {
      fprintf(stderr,"Allocation problems in dacmaster_sparc()\n");
      return(-1);
    }
    buff_bytes = buflen;
  }
  buff_samps = buff_bytes/sizeof_sample;

  da_done = FALSE;
  /* Send first buffer to dsp and start interrupts. */

  set_dac_callbacks(100000);
  sparc_handle(0,buff_samps);

  if(debug_level)
    fprintf(stderr,"tot_samples:%d D/A_buf_size:%d input_buffer:%x buff_bytes:%d frames_per_sec:%d buflen:%d sizeof_sample:%d\n",
	  sig_size, obuffersize,input_buffer,buff_bytes, frames_per_sec,buflen,sizeof_sample);

}

int
sparc_samps_sent()
{
	
  if(ioctl(audio_port, AUDIO_GETINFO, &audio_info) < 0) {
	perror("xwaves output: error getting audio port mode");
	return(-1);
  }
  return (audio_info.play.samples);
}

int
check_sparc_underflow()
{
	
  if(ioctl(audio_port, AUDIO_GETINFO, &audio_info) < 0) {
	perror("xwaves output: error getting audio port mode");
	return(-1);
  }
  return (audio_info.play.error);
}
