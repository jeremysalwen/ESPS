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

static char    *sccs_id = "@(#)audio_iris.c	1.5 3/2/98 ERL";

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <stropts.h>
#include <errno.h>

#include <audio.h>

/*
 * ---------------------------------------------------------------
 * Machine-dependent declaration
 * ---------------------------------------------------------------
 */

ALport          recordPort = NULL;	/* audio port for record */
ALport          playPort = NULL;/* audio port for play */
static ALconfig recordPortConfig = NULL;
static ALconfig playPortConfig = NULL;
/* static int      sampleRates[] = {48000, 44100, 32000, 22050, 16000, 11025, 8000, 0};
*/
/* sampling rates available */
static int sampleRates[] = {48000, 44100, 32000, 29400, 24000, 22050, 
			    16000, 14700, 12000, 11025, 10667, 9800, 9600, 
	 	            8820, 8000, 7350 , 6857, 6400, 6300, 6000, 5880, 
			    5512, 5333, 4900, 4571, 4200, 4000, 3675, 0};


char            audio_error_msg[100];
#define perror private_perror
extern char    *strerror();

static void
private_perror(s)
   char           *s;
{
   sprintf(audio_error_msg, "%s: %s", s, strerror(errno));
}

int
AudioMaxChannels()
{
   return 2;
}



/*
 * -----------------------------------------------------------------
 * Machine-dependent defintions:
 * 
 * Static:
 * 
 * GetFillable GetFilled PortOverflow PortIsAvailable
 * 
 * Extern:
 * 
 * InitAudioRecord CloseAudioRecord StartAudioRecord RecordAudio SetVolRecord
 * 
 * InitAudioPlay CloseAudioPlay StartAudioPlay PlayAudio SetVolPlay
 * -----------------------------------------------------------------
 */

static int
GetFillable(ALport alPort)
{
   int             canread;
   canread = ALgetfillable(alPort);
   return (canread);
}

static int
GetFilled(ALport alPort)
{
   int             canread;
   canread = ALgetfilled(alPort);

   return canread;
}

static int
PortOverflow(ALport alPort)
{
   int             i;
   if ((i = ALgetfillable(alPort)) < 10) {
      perror("error in PortOverflow");
      return (-1);
   }
   return (0);
}


int
PortIsAvailable()
{
   int             fd;
   if ((fd = open("/dev/hdsp/hdsp0master", O_RDWR)) >= 0) {
      close(fd);
      return (0);
   } else {
      sprintf(audio_error_msg,
      "SGI audio device is not available; cannon open /dev/hdsp/hdsp0master");
      return (1);
   }
}


/*****************************************************************
 *
 *  Functions for Record
 *
 *****************************************************************/


int            *
AudioGetRates()
{
   int            *s;
   s = sampleRates;
   return (s);
}

int 
InitAudioRecord(double srate, int channel, int nSampsPerChan)
{
   long            buflen;
   long            pvbuf[4];

   if (PortIsAvailable()) {
      sprintf(audio_error_msg, "Audio device not available on this hardware");
      return (1);
   }
   if (recordPortConfig)
      ALfreeconfig(recordPortConfig);
   recordPortConfig = NULL;

   /* Set the sample rate. */
   pvbuf[0] = AL_INPUT_RATE;
   pvbuf[1] = closest_srate(srate, sampleRates);
   buflen = 2;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

   if (!recordPortConfig)
      recordPortConfig = ALnewconfig();
   ALsetwidth(recordPortConfig, AL_SAMPLE_16);
   ALsetchannels(recordPortConfig, (channel == 1) ? AL_MONO : AL_STEREO);
   ALsetqueuesize(recordPortConfig, nSampsPerChan);

   return (0);
}

int 
CloseAudioRecord()
{
   if (recordPort) {
      ALcloseport(recordPort);
   }
   recordPort = NULL;
   return (0);
}

int 
PauseAudioRecord()
{
   CloseAudioRecord();
   return (0);
}

int 
StartAudioRecord()
{
   /* open the audio device */
   if (!(recordPort = ALopenport("StartAudio", "r", recordPortConfig))) {
      sprintf(audio_error_msg, "Can't open an Indigo audio input port");
      return (1);
   }
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
RecordAudio(short *inputBuffer)
{
   int             canread;

   if (recordPort) {
      /* Be sure that A/D hasn't lost real time. */
      if (GetFillable(recordPort) < 10)
	 return (-1);

      canread = GetFilled(recordPort);
      ALreadsamps(recordPort, inputBuffer, canread);

      return (canread);
   } else
      return (0);
}

int 
SetAudioInputType(src)
   char           *src;
{
   long            pvbuf[4], buflen;

   pvbuf[0] = AL_INPUT_SOURCE;
   if (!strcmp(src, "mic"))
      pvbuf[1] = AL_INPUT_MIC;
   else
      pvbuf[1] = AL_INPUT_LINE;
   buflen = 2;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);
   return 0;
}

int 
SetAudioInputGain(gainL, gainR)	/* from 0 to 100, 100 max gain */
   int             gainL, gainR;
{
   long            pvbuf[4], buflen;

   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   gainL = 255 - gainL * 2.55;
   gainR = 255 - gainR * 2.55;
   pvbuf[0] = AL_LEFT_INPUT_ATTEN;	/* 0 - 255, 0 max gain */
   pvbuf[1] = gainL;
   pvbuf[2] = AL_RIGHT_INPUT_ATTEN;
   pvbuf[3] = gainR;
   buflen = 4;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);
   return 0;
}

/*****************************************************************
 *
 *  Functions for Play
 *
 *****************************************************************/

int 
InitAudioPlay(double srate, int channel, int nSampsPerChan)
{
   long            pvbuf[4], buflen;

   if (PortIsAvailable()) {
      sprintf(audio_error_msg, "Audio device not available on this hardware");
      return (1);
   }
   CloseAudioPlay();		/* D/A may still be in progress. */

   if (playPortConfig)
      ALfreeconfig(playPortConfig);
   playPortConfig = NULL;

   pvbuf[0] = AL_OUTPUT_RATE;
   pvbuf[1] = closest_srate(srate, sampleRates);
   buflen = 2;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);

   if (!playPortConfig)
      playPortConfig = ALnewconfig();
   ALsetwidth(playPortConfig, AL_SAMPLE_16);
   ALsetchannels(playPortConfig, (channel == 1) ? AL_MONO : AL_STEREO);
   ALsetqueuesize(playPortConfig, nSampsPerChan);

   return (0);
}

int 
CanWriteAudio()
{
   if (playPort)
      return (GetFillable(playPort));
   else
      return (0);
}

void 
PlayAudioDrain()
{
   int             iw = 300;
   if (playPort)
      while ((iw-- > 0) && GetFilled(playPort) > 0)
	 sginap(1);
}

int 
CloseAudioPlay()
{
   if (playPort)
      ALcloseport(playPort);
   playPort = NULL;
   return (0);
}


int 
StartAudioPlay()
{
   if (!(playPort = ALopenport("StartAudioPlay", "w", playPortConfig))) {
      sprintf(audio_error_msg, "Can't open an Indigo audio output port");
      return (1);
   }
   return (0);
}

int 
PauseAudioPlay()
{
   int             n;
   if (playPort) {
      n = GetFilled(playPort);
      CloseAudioPlay();
      return (n);
   } else
      return (0);
}

int 
ContAudioPlay()
{
   return StartAudioPlay();
}


/*
 * in case of STEREO recording, RecordAudio returns total number of samples
 * in both channels
 */

int 
PlayAudio(buffer, nsamps)
   short          *buffer;
   int             nsamps;
{
   if (playPort)
      ALwritesamps(playPort, buffer, nsamps);
   return (0);
}

int 
GetPlayFilled()
{
   if (playPort)
      return (GetFilled(playPort));
   else
      return (0);

}

int 
SetAudioOutputType(out)
   char           *out;
{
   sprintf(audio_error_msg, "SetAudioOutputType: Not implemented");
   return 1;
}

int 
SetAudioOutputGain(gainL, gainR)/* 0 - 100,  100 max */
   int             gainL, gainR;
{
   long            pvbuf[4], buflen;

   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   gainL = gainL * 2.55;
   gainR = gainR * 2.55;
   pvbuf[0] = AL_LEFT_SPEAKER_GAIN;	/* 0 - 255, 0 max gain */
   pvbuf[1] = gainL;
   pvbuf[2] = AL_RIGHT_SPEAKER_GAIN;
   pvbuf[3] = gainR;
   buflen = 4;
   ALsetparams(AL_DEFAULT_DEVICE, pvbuf, buflen);
   return 0;
}

void
SendAudioZeros()
{}
