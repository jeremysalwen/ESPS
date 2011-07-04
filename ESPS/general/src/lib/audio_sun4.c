/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1995-1996 Entropic Research Laboratory, Inc.
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

static char    *sccs_id = "@(#)audio_sun4.c	1.5 12/18/96 ERL";

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef OS5
#include <sys/audioio.h>
#else
#include <sun/audioio.h>
#endif

/*
 * for select
 */
#include <sys/time.h>
#include <sys/types.h>

/*
 * ---------------------------------------------------------------
 * Machine-dependent declaration
 * ---------------------------------------------------------------
 */

static int      recordPort = -1;/* audio port */

static char    *audio_dev = "/dev/audio";
static char    *audio_ctl = "/dev/audioctl";
FILE           *playPortFp = NULL;
static int      playPort = -1;	/* audio port */
static int      playChan = 1;
static int      amdDeviceMu = 0;
static char    *muPlayData, *muRecordData;
static short   *resampBuffer;
static struct timeval playBgnTime;
static long     playSamps = 0;
static double   fileRate, resampRatio;
static int      sampleRates[] = {48000, 44100, 37800, 32000, 22050, 18900,
16000, 11025, 9600, 8000, 0};
/* sampling rates available */
static int      amdSampleRate[] = {8000, 0};
static int      sizeof_sample;
static int      PortBufSize;
static int      playPortBufSize;
static double   playRateSave;
static int      playChannelSave;
static int      playNSampsPerChanSave;
static int      ctl_port = 0;

#ifndef MAX_AUDIO_DEV_LEN
/*
 * Paramter for the AUDIO_GETDEV ioctl to determine current audio device
 */
#define MAX_AUDIO_DEV_LEN       (16)
typedef struct audio_device {
   char            name[MAX_AUDIO_DEV_LEN];
   char            version[MAX_AUDIO_DEV_LEN];
   char            config[MAX_AUDIO_DEV_LEN];
}               audio_device_t;
#endif

#ifndef AUDIO_GETDEV
#define AUDIO_GETDEV    _IOR(A, 4, audio_device_t)
#endif

static audio_info_t recordPortInfo;
static audio_info_t playPortInfo;

char            audio_error_msg[100];
#define perror private_perror
extern int sys_nerr;
extern char *sys_errlist[];

static void
private_perror(s)
   char           *s;
{
   if (errno < sys_nerr)
      sprintf(audio_error_msg, "%s: %s", s, sys_errlist[errno]);
   else
      sprintf(audio_error_msg, "%s: Unknown ioctl error", s);
}


static int
GetFillable(fd)
   int             fd;
{
   int             canread;
   if (ioctl(fd, I_NREAD, &canread) < 0) {
      perror("error in GetFillable");
      return (0);
   }
   return (PortBufSize - canread) / sizeof_sample;
}

static int
GetFilled(fd)
   int             fd;
{
   int             canread;
   if (ioctl(fd, I_NREAD, &canread) < 0) {
      perror("error in GetFilled");
      return (0);
   }
   return canread / sizeof_sample;
}

static int
PortOverflow(fd)
   int             fd;
{
   audio_info_t    info;
   AUDIO_INITINFO(&info);
   if (ioctl(fd, AUDIO_GETINFO, &info) < 0) {
      perror("error in PortOverflow");
      return (-1);
   }
   return (info.record.error);
}


int
PortIsAvailable()
{
   int             port, dev, c;
#ifdef OS5
   audio_device_t  adev;
#endif

   if (!ctl_port) {
      if ((ctl_port = open(audio_ctl, O_RDWR)) < 0) {
	 sprintf(audio_error_msg, 
      "Sparc audio device is not available; cannot open audio control device");
	 return 1;
      }
      amdDeviceMu = 0;
#ifdef OS5
      if (ioctl(ctl_port, AUDIO_GETDEV, &adev) < 0) {
	 perror("PortIsAvailable ioctl:");
	 return 1;
      }
      if (!strcmp(adev.name, "SUNW,am79c30")) {
	 amdDeviceMu = 1;
      }
#else
      if (ioctl(ctl_port, AUDIO_GETDEV, &dev) < 0) {
	 perror("PortIsAvailable ioctl:");
	 return 1;
      }
      if (AUDIO_DEV_AMD == dev) {
	 amdDeviceMu = 1;
      }
#endif
   }
   return 0;
}


/*
 * -----------------------------------------------------------------
 * Record
 * -----------------------------------------------------------------
 */

int            *
AudioGetRates()
{
   int            *s;
   s = (amdDeviceMu) ? amdSampleRate : sampleRates;
   return (s);
}

int
AudioMaxChannels()
{
   return (amdDeviceMu ? 1 : 2);
}

int
InitAudioRecord(srate, channel, nSampsPerChan)
   double          srate;
   int             channel;
   int             nSampsPerChan;
{
   long            buflen;
   extern int      speaker_box;

   if (PortIsAvailable()) {
      sprintf(audio_error_msg,"Audio device not available on this hardware");         return (1);
   }

   sizeof_sample = (amdDeviceMu) ? sizeof(char) : sizeof(short);

   /* Set the sample rate. */
   AUDIO_INITINFO(&recordPortInfo);
   if (amdDeviceMu && srate != 8000) {
      sprintf(audio_error_msg, 
       "This audio device only supports 8000 Hz sample rate");
      return (1);
   }
   recordPortInfo.record.sample_rate = (amdDeviceMu) ? 8000 :
      closest_srate(srate, sampleRates);

   if (amdDeviceMu && channel != 1) {
      sprintf(audio_error_msg, 
       "This audio device device only supports 1 channel, %d requested",
       channel);
      return (1);
   }
   recordPortInfo.record.channels = (channel < 2) ? 1 : 2;
   recordPortInfo.record.precision = (amdDeviceMu) ? 8 : 16;
   recordPortInfo.record.encoding =
      (amdDeviceMu) ? AUDIO_ENCODING_ULAW : AUDIO_ENCODING_LINEAR;
   recordPortInfo.record.error = 0;
   PortBufSize = recordPortInfo._yyy[1] = nSampsPerChan * 
                                          sizeof_sample * channel;
   if (amdDeviceMu) {
      muRecordData = (char *) malloc(nSampsPerChan * channel * sizeof(char));
      if (!muRecordData) {
	 sprintf(audio_error_msg, "InitAudioRecord: malloc failed.");
	 return (1);
      }
   }
   return (0);
}

int
CloseAudioRecord()
{
   if (recordPort >= 0) {
      /* save the state */
      if (ioctl(recordPort, AUDIO_GETINFO, &recordPortInfo) < 0) {
	 perror("error getting audio port mode in PauseAudioRecord");
         (void)close(recordPort);
	 return (1);
      }
      (void)ioctl(recordPort, I_FLUSH, FLUSHR);
      (void)close(recordPort);
   }
   recordPort = -1;
   return (0);
}

int
PauseAudioRecord()
{
   return CloseAudioRecord();
}

int
StartAudioRecord()
{
   /* open the audio device */
   if ((recordPort = open(audio_dev, O_RDONLY | O_NDELAY)) < 0) {
      perror("Cannot open audio device for input");
      return (1);
   }
   if (ioctl(recordPort, AUDIO_SETINFO, &recordPortInfo) < 0) {
      perror("error setting audio port mode in StartAudioRecord");
      close(recordPort);
      recordPort = -1;
      return (1);
   }
   /*
    * This ioctl is needed.  Otherwise, for some unknown reason, the
    * recording starting from now may or may not be garbage
    */
   (void)ioctl(recordPort, I_FLUSH, FLUSHR);
   return (0);
}

int
ContAudioRecord()
{
   return StartAudioRecord();
}


/*
 * in case of STEREO recording, RecordAudio returns total number of samples
 * in both channels
 */

int
RecordAudio(inputBuffer)
   short          *inputBuffer;
{
   int             canread;
   /* Be sure that A/D hasn't lost real time. */
   if (GetFillable(recordPort) < 10 || PortOverflow(recordPort)) {
      sprintf(audio_error_msg,"Lost realtime operation in RecordAudio");
      return (-1);
   }
   canread = GetFilled(recordPort);
   if (amdDeviceMu) {
      (void) read(recordPort, muRecordData, canread * sizeof_sample);
      mu_to_linear(muRecordData, inputBuffer, canread);
   } else {
      (void) read(recordPort, inputBuffer, canread * sizeof_sample);
   }

   return (canread);
}

int
SetAudioInputType(src)
   char           *src;
{
   if (!strcmp(src, "mic"))
      recordPortInfo.record.port = AUDIO_MICROPHONE;
   else
      recordPortInfo.record.port = AUDIO_LINE_IN;
   if (recordPort > 0)
      if (ioctl(recordPort, AUDIO_SETINFO, &recordPortInfo) < 0) {
	 perror("error setting audio port mode in SetAudioInputType");
         return 1;
      }
   return 0;
}

int
SetAudioInputGain(gainL, gainR)
   int             gainL, gainR;
{
   /* for SUN, the left & right gains are the same */
   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;		/* 0 - 255, 0 min gain */
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   gainL = gainL * 2.55;
   gainR = gainR * 2.55;

   recordPortInfo.record.gain = (gainL > gainR) ? gainL : gainR;
   if (recordPort > 0)
      if (ioctl(recordPort, AUDIO_SETINFO, &recordPortInfo) < 0) {
	 perror("error setting audio port mode in SetAudioInputGain");
         return 1;
      }
   return 0;
}


/*
 * -----------------------------------------------------------------
 * Play
 * -----------------------------------------------------------------
 */

static double
TimeDiff(a, b)
   struct timeval *a, *b;
{
   double          at, bt;
   bt = b->tv_sec + b->tv_usec * 1E-6;
   at = a->tv_sec + a->tv_usec * 1E-6;

   return (bt - at);
}

int
InitAudioPlay(srate, channel, nSampsPerChan)
   double          srate;
   int             channel;
   int             nSampsPerChan;
{
   long            buflen;

   playRateSave = srate;
   playChannelSave = channel;
   playNSampsPerChanSave = nSampsPerChan;
   if (PortIsAvailable()) {
      sprintf(audio_error_msg,"Audio device not available on this hardware");         return (1);
   }
   sizeof_sample = (amdDeviceMu) ? sizeof(char) : sizeof(short);

   /* Set the sample rate. */
   AUDIO_INITINFO(&playPortInfo);
   playPortInfo.play.sample_rate = (amdDeviceMu) ? 
                               8000 : closest_srate(srate, sampleRates);
   fileRate = srate;
   if (amdDeviceMu)
      resampRatio = srate / 8000.0;
   playPortInfo.play.precision = (amdDeviceMu) ? 8 : 16;
   if (amdDeviceMu && channel != 1) {
      sprintf(audio_error_msg, 
       "This audio device device only supports 1 channel, %d requested",
       channel);
      return (1);
   }
   playPortInfo.play.channels = playChan = (channel < 2) ? 1 : 2;
   playPortInfo.play.encoding =
      (amdDeviceMu) ? AUDIO_ENCODING_ULAW : AUDIO_ENCODING_LINEAR;
   playPortInfo.play.error = 0;
   playPortInfo._yyy[0] = 0;
   playPortBufSize = nSampsPerChan * sizeof_sample * channel;
   if (amdDeviceMu) {
      muPlayData = (char *) malloc(nSampsPerChan * channel * sizeof(char));
      resampBuffer = (short *) malloc(nSampsPerChan * channel * sizeof(short));
      if (!muPlayData || !resampBuffer) {
	 sprintf(audio_error_msg, "InitAudioPlay: malloc failed.");
	 return (1);
      }
   }
   return (0);
}

int
GetPlayFilled()
{
   int             sampsUnPlayed, sampsPlayed;
   struct timeval  currTime;

   if (playPort < 0)
      return (0);
   gettimeofday(&currTime, NULL);
   sampsPlayed = fileRate * TimeDiff(&playBgnTime, &currTime) * playChan;
   sampsUnPlayed = playSamps - sampsPlayed;
   if (sampsUnPlayed < 0)
      sampsUnPlayed = 0;
   return (sampsUnPlayed);
}

int
CanWriteAudio()
{
   int             sampsUnplayed;
   int             canwriteSamps;
   if (playPort < 0)
      return (0);
   canwriteSamps = playPortBufSize / sizeof_sample - GetPlayFilled();
   if (canwriteSamps < 0)
      canwriteSamps = 0;
   return (canwriteSamps);
}


int
PlayAudioDrain()
{
   audio_info_t    info;
   if (playPort < 0)
      return 0;

   (void)ioctl(playPort, AUDIO_DRAIN, NULL);
   return 0;
}

int
CloseAudioPlay(hold_port)
   int             hold_port;	/* if non-zero, hold port open */
{
   audio_info_t    info;
   if (playPort >= 0) {
      ioctl(playPort, I_FLUSH, FLUSHW);
      if (!hold_port) {
	 close(playPort);
	 fclose(playPortFp);
      }
   }
   if (!hold_port) {
      playPort = -1;
      playPortFp = NULL;
   }
   return (0);
}

static int
OpenPlayPort()
{
   if (playPort == -1) {
      if ((playPort = open(audio_dev, O_WRONLY | O_NDELAY)) < 0) {
	 perror("Cannot open audio device for output");
	 return (1);
      }
      playPortFp = fdopen(playPort, "w");
   }
   if (ioctl(playPort, AUDIO_SETINFO, &playPortInfo) < 0) {
      perror("error setting audio port mode in StartAudioPlay");
      close(playPort);
      playPort = -1;
      playPortFp = NULL;
      return (1);
   }
   return (0);
}

int
StartAudioPlay()
{
   int             err;

   playSamps = 0;
   err = OpenPlayPort();
   return (err);
}

int
PauseAudioPlay()
{
   int             sampsUnPlayed;
   struct timeval  stopTime;

   if (playPort < 0)
      return (0);
   (void)CloseAudioPlay(0);

   gettimeofday(&stopTime, NULL);
   sampsUnPlayed = playSamps - fileRate * TimeDiff(&playBgnTime, &stopTime) 
                   * playChan;
   return (sampsUnPlayed);
}

int
ContAudioPlay()
{
   int err;

   err = InitAudioPlay(playRateSave, playChannelSave, playNSampsPerChanSave);
   if (err) return err;
   err = StartAudioPlay();
   if (err) return err;
}

/*************************************************************************/
/*
 * The following resample function was written by Tom Veatch, Department of
 * Linguistics, University of Pennsylvania, Philadelphia, PA 19104.  Thanks,
 * Tom.
 * 
 * Copyright (c) 1990 Thomas C. Veatch
 * 
 * Tom grants anyone the right to make or distribute verbatim copies of this
 * resample function, provided that the copyright notice and permission
 * notice are preserved, and that the distributor grants the recipient
 * permission for further redistribution as permitted by this notice.
 * 
 * modified by: Derek Lin
 */

#define SETCOEFSINDS {j=(int)i*step;k=j+1;c2=fmod(i*step,1.0);c1=1.0-c2;}

static int
resample(in, out, step, n, gain)
   register short *in;		/* input data array */
   register short *out;		/* output data array */
   register float  step;	/* step: input sf / SPARC_SAMPLE_FREQ */
   register float  gain;	/* gain factor */
   long            n;		/* size of elements in in/out */
{

   register int    i, j, k, nout;
   register float  c1, c2;

   /* 2-point interpolate samples from input to output. */
   nout = n / step;		/* max index of output samples */

   if (gain != 1.0)
      for (i = 0; i < nout; i++) {
	 SETCOEFSINDS
	    out[i] = gain * (c1 * in[j] + c2 * in[k]);
      }
   else
      for (i = 0; i < nout; i++) {
	 SETCOEFSINDS
	    out[i] = (c1 * in[j] + c2 * in[k]);
      }
   return (nout);
}
/**********************************************************************/


int
PlayAudio(buffer, nSamps)
   short          *buffer;
   int             nSamps;
{
   int             wrote;

   if (playPort < 0)
      return (0);

   if (amdDeviceMu) {
      float           nRatio;
      long            nResamps;
      nResamps = resample(buffer, resampBuffer, resampRatio, nSamps, 1.0);
      linear_to_mu(resampBuffer, muPlayData, nResamps);
      wrote = resampRatio *
	 fwrite((char *) muPlayData, sizeof(char), nResamps, playPortFp);
   } else {
      wrote = fwrite((char *) buffer, sizeof(short), nSamps, playPortFp);
   }

   if (playSamps == 0)
      gettimeofday(&playBgnTime, NULL);

   fflush(playPortFp);
   playSamps += nSamps;
   return (0);
}


int
SetAudioOutputType(out)
   char           *out;
{
   int status = 0;
   if (!strcmp(out, "speaker"))
      playPortInfo.play.port = AUDIO_SPEAKER;
   else if (!strcmp(out, "headphone"))
      playPortInfo.play.port = AUDIO_HEADPHONE;
   else if (!strcmp(out, "lineOut"))
      playPortInfo.play.port = (amdDeviceMu) ? AUDIO_HEADPHONE : AUDIO_LINE_OUT;
   else {
      sprintf(audio_error_msg,"Unknown Output destination %s, speaker used");
      playPortInfo.play.port = AUDIO_SPEAKER;
      status = 0;
   }
   if (playPort > 0)
      if (ioctl(playPort, AUDIO_SETINFO, &playPortInfo) < 0) {
	 perror("error setting audio port mode in SetAudioOutputType");
         return 1;
      }
   return status;
}

int
SetAudioOutputGain(gainL, gainR)
   int             gainL, gainR;
{
   /* for SUN, the left & right gains are the same */
   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;		/* 0 - 255, 0 min gain */
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   gainL = gainL * 2.55;
   gainR = gainR * 2.55;

   playPortInfo.play.gain = (gainL > gainR) ? gainL : gainR;
   if (playPort > 0)
      if (ioctl(playPort, AUDIO_SETINFO, &playPortInfo) < 0) {
	 perror("error setting audio port mode in SetAudioOutputGain");
         return 1;
      }
   return 0;
}

void
SendAudioZeros()
{}
