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

static char *sccs_id = "@(#)audio_linux.c	1.1 03/21/98 ERL";

#include <stdio.h>
#include <linux/soundcard.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#define AUDIODEV	"/dev/dsp"

char audio_error_msg[100];


static int play_fd = -1, rec_fd = -1;	
static unsigned long sample_rate, rec_sample_rate;
static int channels, rec_channels, rec_buffsize;
static int playPortBufSize;
static struct timeval playBgnTime;
static unsigned long samps_sent;
static int abuf_size;
static int rbuf_size;

static int sample_rates[] = {48000, 44100, 37800, 32000, 22050, 18900, 16000, 
			   11025, 9600, 8000, 0};

extern int debug_level;


static double
TimeDiff (a, b)
     struct timeval *a, *b;
{
   double at, bt;
   bt = b->tv_sec + b->tv_usec * 1E-6;
   at = a->tv_sec + a->tv_usec * 1E-6;

   return (bt - at);
}

static void
nap (usec)
     int usec;
{
   struct timeval timer_val;

   timer_val.tv_sec = 0;
   timer_val.tv_usec = usec;
   select (0, NULL, NULL, NULL, &timer_val);
}


int
PortIsAvailable ()
{
   if (play_fd > 0)
      return 0;

   if ((play_fd = open (AUDIODEV, O_RDWR)) < 0)
     {
	sprintf (audio_error_msg, "Cannot open Linux audio port.\n");
	return 1;
     }
   else
     {
	if (close (play_fd) < 0)
	   perror ("PortIsAvailable");
	play_fd = -1;
	return 0;
     }
}


int
InitAudioPlay (srate, channel, nSampsPerChan)
     double srate;
     int channel;
     int nSampsPerChan;
{
   unsigned long sample_size = 16;
   int stereo = (channel == 2);

   if (debug_level)
      fprintf (stderr,
	       "InitAudioPlay: srate: %lf, channel: %d, nSampsPerChan: %d\n",
	       srate, channel, nSampsPerChan);
   channels = channel;
   playPortBufSize = nSampsPerChan * sizeof (short) * channel;
   sample_rate = srate;

   if (play_fd < 0)
     {
	if ((play_fd = open (AUDIODEV, O_WRONLY)) < 0)
	  {
	     sprintf (audio_error_msg, "Cannot open audio port in Init\n");
	     fprintf (stderr, "Cannot open audio port in Init\n");
	     return 1;
	  }
     }

   if (ioctl (play_fd, SNDCTL_DSP_GETBLKSIZE, &abuf_size) < 0)
     {
	sprintf (audio_error_msg, "Unable to get buffer size\n");
	close (play_fd);
	play_fd = -1;
	return 1;
     }

   if (ioctl (play_fd, SNDCTL_DSP_SAMPLESIZE, &sample_size) < 0)
     {
	sprintf (audio_error_msg, "Unable to set sample size\n");
	close (play_fd);
	play_fd = -1;
	return 1;
     }
   if (ioctl (play_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
     {
	sprintf (audio_error_msg, "Unable to set stereo flag\n");
	close (play_fd);
	play_fd = -1;
	return 1;
     }
   if (ioctl (play_fd, SNDCTL_DSP_SPEED, &sample_rate) < 0)
     {
	sprintf (audio_error_msg, "Unable to set sample rate\n");
	close (play_fd);
	play_fd = -1;
	return 1;
     }

   return (0);
}

int
GetPlayFilled ()
{
   int samps_not_played, samps_played;
   struct timeval current_time;

   gettimeofday (&current_time, NULL);
   samps_played = sample_rate * TimeDiff (&playBgnTime, &current_time) * channels;
   samps_not_played = samps_sent - samps_played;
   if (samps_not_played < 0)
      samps_not_played = 0;
   return (samps_not_played);
}

int
CanWriteAudio ()
{
   int samps_not_played;
   int can_write_samps;

   can_write_samps = playPortBufSize / sizeof (short) - GetPlayFilled ();
   if (can_write_samps < 0)
      can_write_samps = 0;
   return (can_write_samps);

}


int
PlayAudioDrain ()
{
   char *io_buff, *calloc ();
   int filled = GetPlayFilled () * sizeof (short);
   int buf_size = abuf_size - filled;

   if (play_fd < 0)
      return 0;
   if (buf_size < 0)
      return 0;
   io_buff = calloc (buf_size, sizeof (char));
   write (play_fd, io_buff, buf_size);
   free (io_buff);
   return 0;
}

int
CloseAudioPlay (hold_port)
     int hold_port;		/* if non-zero, hold port open */
{
   if (play_fd < 0)
      return 0;
   if (debug_level)
      fprintf (stderr, "CloseAudioPlay: remaining: %d", GetPlayFilled ());
   if (close (play_fd) < 0)
      perror ("close");
   if (debug_level)
      fputc ('.', stderr);
   play_fd = -1;
   return 0;
}

int
StartAudioPlay ()
{
   samps_sent = 0;
   gettimeofday (&playBgnTime, NULL);
   return 0;
}

int
PlayAudio (buffer, nSamps)
     short *buffer;
     int nSamps;
{
   int i, n;

   if (debug_level)
      fprintf (stderr, "PlayAudio: %d\n", nSamps);
   if (play_fd < 0)
      return 0;

   n = nSamps * sizeof (short);
   if (write (play_fd, buffer, n) != n)
      fprintf (stderr, "write error to audio dev\n");

   samps_sent += nSamps;
   return 0;
}

void
SendAudioZeros ()
{
   static short buff[400];

   if (play_fd > -1)
      write (play_fd, buff, 800);
}

int
InitAudioRecord (srate, channel, buffsize)
     double srate;
     int channel;
     int buffsize;

{
   long buflen;
   unsigned long sample_size = 16;
   int stereo = (channel == 2);


   if (debug_level)
      fprintf (stderr,
	       "InitAudioRecord: srate: %lf, channel: %d, buffsize: %d\n",
	       srate, channel, buffsize);


   if (rec_fd < 0)
     {
	if ((rec_fd = open (AUDIODEV, O_RDONLY)) < 0)
	  {
	     sprintf (audio_error_msg, "Cannot open audio port in Init\n");
	     fprintf (stderr, "Cannot open audio port in Init\n");
	     return 1;
	  }
     }

   rec_channels = channel;
   rec_sample_rate = srate;
   rec_buffsize = buffsize;

   if (ioctl (rec_fd, SNDCTL_DSP_SAMPLESIZE, &sample_size) < 0)
     {
	sprintf (audio_error_msg, "Unable to set sample size\n");
	close (rec_fd);
	rec_fd = -1;
	return 1;
     }
   if (ioctl (rec_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
     {
	sprintf (audio_error_msg, "Unable to set stereo flag\n");
	close (rec_fd);
	rec_fd = -1;
	return 1;
     }
   if (ioctl (rec_fd, SNDCTL_DSP_SPEED, &rec_sample_rate) < 0)
     {
	sprintf (audio_error_msg, "Unable to set sample rate\n");
	close (rec_fd);
	rec_fd = -1;
	return 1;
     }

   if (ioctl (rec_fd, SNDCTL_DSP_GETBLKSIZE, &rbuf_size) < 0)
     {
	sprintf (audio_error_msg, "Unable to get buffer size\n");
	close (play_fd);
	play_fd = -1;
	return 1;
     }


   return 0;
}

int
StartAudioRecord ()
{
   return 0;
}

int
CloseAudioRecord ()
{
   if (rec_fd < 0)
      return 0;

   if (close (rec_fd) < 0)
      perror ("close");

   rec_fd = -1;
   return 0;
}

int *
AudioGetRates()
{
   return sample_rates;
}

int
AudioMaxChannels()
{  
   return 2;
}

int
CanRead()
{
	static struct timeval timeout = {0L, 0L};
	static fd_set fds;

	FD_ZERO(&fds);
	FD_SET(rec_fd, &fds);
	return(select(rec_fd+1, &fds, NULL, NULL, &timeout));
}

int
PauseAudioRecord()
{
	return 1;
}

int
ContAudioRecord()
{
	return 1;
}

int
RecordAudio(buffer)
short *buffer;
{
	int haveread;

	haveread = read(rec_fd, buffer, rbuf_size)/sizeof(short);
	return haveread;
}




