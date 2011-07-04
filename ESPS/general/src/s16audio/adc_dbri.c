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

static char *sccs_id = "@(#)adc_dbri.c	1.4 9/18/97 ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/ss.h>
#include <Objects.h>
#include <malloc.h>
#ifdef OS5
#include <sys/audioio.h>
#else
#include <sun/audioio.h>
#endif
#include <stropts.h>

#define BUF_SIZE 8192

int input_buffer[BUF_SIZE];	/* working buffer for sample output and max/min */

int ad_done = FALSE, sent = 0, error_at = -1, chan_config = 0;
static int  obuffersize, sizeof_sample = 2, maxsamps;
FILE *output = NULL;
int audio_port = NULL;		
int audio_port_ctl = NULL;		
void (*sig_save[3])();
extern int debug_level;
double amax, amin;
int dbri_input_size;

/*---------------------------------------------------------------------*/
void
  stop_dbri_ad(sig)
int sig;
{
  if(debug_level)
    (void)fprintf(stderr,"Caught a signal (%d)\n",sig);
  ad_done = TRUE;
}

/*---------------------------------------------------------------------*/
void
  stop_dbri_ad_send_xwaves(sig)
int sig;
{
  extern int send_display_file;

  if(debug_level)
    (void)fprintf(stderr,"Caught a send_xwaves signal (%d)\n",sig);
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
  long buflen, samps_per_frame, frames_per_sec;
  audio_info_t audio_info;
  int junk, junk1, junk3;

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
   extern int speaker_box;
   int input_port;
   char *getenv();

  if(!(output = outfd)) {
    (void)fprintf(stderr,"Null FILE passed to record_file!\n");
    return(0);
  }
  if(!dbri_audio_is_available()) {
    (void)fprintf(stderr,"Sorry, this record program requires dbri audio.\n");
    return(0);
  }

  if(channel < 2)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);

  samps_per_frame = sizeof_sample/sizeof(short);

  chan_config = channel;

  if((maxsamps = samples) <= 0) {
    printf("Bogus sample count sent to dbri_record_file(%d)\n",samples);
    return(0);
  }
  /* Set the sample rate. */
  AUDIO_INITINFO(&audio_info);
  audio_info.record.sample_rate = frames_per_sec = closest_srate(&req_rate);
  *srate = req_rate/1000.0;
  audio_info.record.channels = (channel < 2)? 1: 2;
  audio_info.record.precision = 16;
  audio_info.record.encoding = 3; /* LINEAR */
  audio_info.record.error = 0;
  dbri_input_size = audio_info._yyy[1] = frames_per_sec * sizeof_sample;
  audio_info.play.pause = 1;
  audio_info.record.pause = 1;

  buflen   = 2;

  if(debug_level)
    (void)fprintf(stderr,
    "record_file:outfd=%x samples=%d srate=%f channel=%d sigmax=%f frames_per_sec=%d\n",
	    outfd, samples, *srate, channel, *sigmax, frames_per_sec);

  if(do_prompt && pstring && *pstring) /* optional user-start prompt */
    (void)fprintf(stderr, "%s\n",pstring);

  /* Initialize the port.  This is probably where A/D actually starts. */

  /* open the audio device */
  if((audio_port = open("/dev/audio", O_RDONLY)) < 0) {
        perror("Cannot open audio device for input");
        return(-1);
  }
 
  if(ioctl(audio_port, AUDIO_SETINFO, &audio_info) < 0) {
        perror("error setting audio port mode");
        return(-1);
  }
  ioctl(audio_port, I_FLUSH, FLUSHRW);
  audio_info.record.pause = 0;
  if(ioctl(audio_port, AUDIO_SETINFO, &audio_info) < 0) {
        perror("error setting audio port mode");
        return(-1);
  }
/*
  junk3=ioctl(audio_port, I_NREAD, &junk);
  fprintf(stderr,"%d bytes of junk to skip, %d\n",junk,junk3);
  junk1 = read(audio_port, input_buffer, junk);
  fprintf(stderr,"%d bytes of junk read\n",junk);
*/

  obuffersize = dbri_getfillable(audio_port);
  ad_done = FALSE;

  sleep_time = (1000000.0*obuffersize)/(frames_per_sec * 50);

  /* Install a signal handler to deal with interrupts and quits. */
  sig_save[1] = signal( SIGINT, stop_dbri_ad);
  sig_save[2] = signal( SIGQUIT, stop_dbri_ad);
  sig_save[3] = signal( SIGUSR1, stop_dbri_ad_send_xwaves);
  maxsamps = samples;
  *sigmax = amax = amin = 0.0;
  ad_done = FALSE;
  error_at = -1;		/* if >= 0, here is where a problem occurred */

  while((!ad_done) && (error_at < 0)) {
    dbri_handle();
    usleep(sleep_time);
  }
  if((*sigmax = amax) < -amin)
    *sigmax = -amin;

  if(error_at >= 0)
    return(error_at);
  else
    return(sent);
}

/*************************************************************************/
void
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
static dbri_handle()
{
  long canread, got=0, toread, wrote;
  int i,ret;
  static int first_time=1;
  
  

  /* Be sure that A/D hasn't lost real time. */
  if(dbri_overflow(audio_port) || dbri_getfillable(audio_port) < 10) {
    (void)fprintf(stderr,"Lost real time operation at sample %d\n",sent);
    error_at = sent;
    return(got);
  }
  canread = dbri_getfilled(audio_port);
  if((canread+sent) > maxsamps) canread = maxsamps - sent;
  while(canread > 0) {
    if((toread = canread) > BUF_SIZE)
      toread = BUF_SIZE;
    (void)read(audio_port, input_buffer, toread*sizeof_sample);
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
      (void)fprintf(stderr,"dbri_handle(): reached requested sample count.\n");
    ad_done = TRUE;
  }
  return(got);
}

int
dbri_getfillable(fd)
int fd;
{
	int canread;
	if(ioctl(fd, I_NREAD, &canread) < 0) {
	  perror("error in dbri_getfillable");
	  return(0);
	}
	return (dbri_input_size - canread)/sizeof_sample;
}


int
dbri_getfilled(fd)
int fd;
{
	int canread;
	if(ioctl(fd, I_NREAD, &canread) < 0) {
	  perror("error in dbri_getfilled");
	  return(0);
	}
	return canread/sizeof_sample;
}


int 
dbri_overflow(fd)
int fd;
{
	
  audio_info_t audio_info;
  if(ioctl(fd, AUDIO_GETINFO, &audio_info) < 0) {
	perror("error in dbri_overflow");
	return(-1);
  }
  return (audio_info.record.error);
}
