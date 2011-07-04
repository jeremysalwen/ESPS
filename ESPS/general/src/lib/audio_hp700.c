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

static char    *sccs_id = "@(#)audio_hp700.c	1.8 10/3/96 ERL";

#include <esps/esps.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <audio/Alib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/time.h>

/*
 * ---------------------------------------------------------------
 * Machine-dependent declaration
 * ---------------------------------------------------------------
 */

static int     *sampleRates = NULL;
static AGainDB  maxIn;
static AGainDB  minIn;
static AGainDB  maxOut;
static AGainDB  minOut;
static Audio   *audio = NULL;
static int      sizeof_sample;

static AudioAttrMask playSrcAttribsMask;
static AudioAttrMask playAttribsMask;
static AudioAttributes playSrcAttribs;
static AudioAttributes playAttribs;
static SSPlayParams playStreamParams;
static SStream  playStream;
static ATransID playXid = NULL;
static int      playSocket = -1;/* <0 when audio is in close state */
static int      playChan = 1;
static int      playPortBufSize;
static int      fileRate;
static unsigned long playSamps = 0;
static struct timeval playBgnTime;
static AGainEntry playGainEntry[4];
static int      playGainSet = 0;
static int      playGainR, playGainL;
static double   playRate;
static int      playNSampsPerChan;
static int      useIntSpeaker = 0;
static AErrorHandler prevHandler;
static int      first_write = 0;

char            audio_error_msg[512];
#define perror private_perror
extern int      sys_nerr;
extern char    *sys_errlist[];
int             closest_srate();
char           *getenv(), *malloc();
void            free();

extern int	debug_level;

static void
private_perror(s)
   char           *s;
{
   if (errno < sys_nerr)
      sprintf(audio_error_msg, "%s: %s\n", s, sys_errlist[errno]);
   else
      sprintf(audio_error_msg, "%s: Unknown ioctl error\n", s);
}

static long
ErrorHandler(audio, err_event)
   Audio          *audio;
   AErrorEvent    *err_event;
{
   char            errorbuff[132];
   AGetErrorText(audio, err_event->error_code, errorbuff, 131);
   fprintf(stderr, "HP Audio Error: %s\n", errorbuff);
   return 0;
}

int
PortIsAvailable()
{
   long            status;
   char            server[MAXHOSTNAMELEN], *s = NULL;
   unsigned long  *srates;
   long            nRates, i;

   if (audio)
      return 0;

   s = getenv("AUDIO");
   if (s)
      strcpy(server, s);
   else
      strcpy(server, ":0");

   prevHandler = ASetErrorHandler(ErrorHandler);

   audio = AOpenAudio(NULL, NULL);
   if (status) {
      sprintf(audio_error_msg,
	      "Trouble opening audio, error code: %ld\n", status);
      return (1);
   }
   nRates = ANumSamplingRates(audio);
   if (!nRates) {
      sprintf(audio_error_msg,
	      "Trouble opening audio, ANumSamplingRates returns 0.\n");
      return 1;
   }
   srates = ASamplingRates(audio);
   if (sampleRates)
      free(sampleRates);
   sampleRates = (int *) malloc(sizeof(int) * (nRates + 1));
   sampleRates[nRates] = 0;
   for (i = 0; i < nRates; i++)
      sampleRates[i] = srates[i];
   maxIn = AMaxInputGain(audio);
   maxOut = AMaxOutputGain(audio);
   minIn = AMinInputGain(audio);
   minOut = AMinOutputGain(audio);
   return (0);
}

static double
TimeDiff(a, b)
   struct timeval *a, *b;
{
   double          d;
   d = (b->tv_sec + b->tv_usec * 1E-6) - (a->tv_sec + a->tv_usec * 1E-6);
   if (d > 0)
      return (d);
   else
      return (0);
}

static int
OpenPlayPort()
{
   int             stat;
   /*
    * create a stream socket
    */
   spsassert(audio, "Audio structure not open\n");
   if (playSocket > -1)
      return 0;

   playSocket = socket(AF_INET, SOCK_STREAM, 0);

   if (playSocket < 0) {
      perror("Socket creation failed");
      return 1;
   }
   /*
    * connect the stream socket to the audio stream port
    */
   stat = connect(playSocket, (struct sockaddr *) & playStream.tcp_sockaddr,
		  sizeof(struct sockaddr_in));

   if (stat < 0) {
      perror("Connect failed");
      close(playSocket);
      playSocket = -1;
      return 1;
   }
   return (0);
}

int
InitAudioPlay(srate, channel, nSampsPerChan)
   double          srate;
   int             channel;
   int             nSampsPerChan;
{
   long            length, soffset;
   AByteOrder      byte_order;
   char           *c;


   if (debug_level)
      fprintf(stderr, 
              "InitAudioPlay %f %d %d %x %d\n", 
              srate, channel, nSampsPerChan, audio, playSocket);
   playRate = srate;
   playChan = channel;
   playNSampsPerChan = nSampsPerChan;

   if (!audio && PortIsAvailable()) {
      (void) sprintf(audio_error_msg,
		     "Sorry, this play program requires HP audio.\n");
      return (1);
   }
   if ((c = getenv("SPEAKER")))
      if (*c == 'E' || *c == 'e')
	 useIntSpeaker = 0;
      else if (*c == 'I' || *c == 'i')
	 useIntSpeaker = 1;

   playAttribsMask = 0;
   playSrcAttribsMask = 0;
   soffset = 0;
   sizeof_sample = sizeof(short);

   playSrcAttribsMask = (playSrcAttribsMask | ASSamplingRateMask | ASChannelsMask);
   playAttribsMask = (playAttribsMask | ASSamplingRateMask | ASChannelsMask);
   fileRate = playAttribs.attr.sampled_attr.sampling_rate =
      playSrcAttribs.attr.sampled_attr.sampling_rate =
      closest_srate(srate, sampleRates);
   playSrcAttribs.attr.sampled_attr.channels = playAttribs.attr.sampled_attr.channels = (channel < 2) ? 1 : 2;
   playSrcAttribs.attr.sampled_attr.interleave = playSrcAttribs.attr.sampled_attr.interleave = 1;

   AChooseSourceAttributes(audio, NULL, NULL, AFFRawLin16, playSrcAttribsMask,
			   &playSrcAttribs, &soffset, &length, &byte_order,
			   NULL);
   AChoosePlayAttributes(audio, &playSrcAttribs, 0,
			 &playAttribs, &byte_order, NULL);
   switch (playAttribs.attr.sampled_attr.channels) {
   case 1:
      playGainEntry[0].u.o.out_ch = AOCTMono;
      playGainEntry[0].gain = AUnityGain;
      playGainEntry[0].u.o.out_dst = useIntSpeaker ? AODTMonoIntSpeaker : AODTMonoJack;
      break;
   case 2:
   default:
      playGainEntry[0].u.o.out_ch = AOCTLeft;
      playGainEntry[1].u.o.out_ch = AOCTRight;
      playGainEntry[0].gain = AUnityGain;
      playGainEntry[1].gain = AUnityGain;
      playGainEntry[0].u.o.out_dst = useIntSpeaker ? AODTRightIntSpeaker : AODTRightJack;
      playGainEntry[1].u.o.out_dst = useIntSpeaker ? AODTRightIntSpeaker : AODTLeftJack;
      break;
   }
   playStreamParams.priority = APriorityNormal;
   playStreamParams.event_mask = 0;	/* don't solicit any events */
   playStreamParams.gain_matrix.type = AGMTOutput;	/* gain matrix */
   playStreamParams.gain_matrix.num_entries =
      playAttribs.attr.sampled_attr.channels;
   playStreamParams.gain_matrix.gain_entries = playGainEntry;
   playXid = APlaySStream(audio, ~0, &playAttribs, &playStreamParams,
			  &playStream, NULL);


   playPortBufSize = nSampsPerChan * sizeof_sample * channel;

   return (0);
}

int
GetPlayFilled()
{
   int             sampsUnPlayed, sampsPlayed;
   struct timeval  currTime;

   if (playSocket < 0)
      return 0;

   if (first_write)
      return 0;

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
   int             canwriteSamps;

   if (playSocket < 0)
      return (0);

   canwriteSamps = (playPortBufSize / sizeof_sample) - GetPlayFilled();
   if (canwriteSamps < 0)
      canwriteSamps = 0;

   return (canwriteSamps);
}


int
PlayAudioDrain()
{
   return 0;
}

int
CloseAudioPlay(hold)
   int             hold;
{
   long status;

   if (playSocket < 0)
      return (0);

   if (debug_level)
      fprintf(stderr, "Close: left: %d", GetPlayFilled());

   while (GetPlayFilled()) ;
   AStopAudio(audio, playXid, ASMLinkedTrans, NULL, NULL);

   first_write = 0;
   (void)close(playSocket);
   (void)ACloseAudio(audio, &status);
   audio = NULL;
   if(debug_level)  
      fprintf(stderr,".\n");
   playSocket = -1;
   return (0);
}


int
StartAudioPlay()
{
   int             err;
   long            status;

   playSamps = 0;
   err = OpenPlayPort();
   if (err) {
      sprintf(audio_error_msg, "StartAudioPlay: trouble OpenPlayPort()\n");
      return (1);
   }
   if (playGainSet) {
      switch (playAttribs.attr.sampled_attr.channels) {
      case 1:
	 ASetChannelGain(audio, playXid, ACTMono, playGainL, &status);
	 break;
      case 2:
      default:			/* assume no more than 2 channels */
	 ASetChannelGain(audio, playXid, ACTLeft, playGainL, &status);
	 ASetChannelGain(audio, playXid, ACTRight, playGainR, &status);
	 break;
      }
   }
   first_write = 1;
   return (0);
}

void
SendAudioZeros()
{
/*
   static short    buff[400];

   if (playSocket < 0)
      return;

   write(playSocket, buff, 400 * sizeof(short));
*/
}



int
PlayAudio(buffer, nSamps)
   char           *buffer;
   int             nSamps;
{
   int             i, n, to_write;

   if (playSocket < 0)
      return 0;

   n = nSamps * sizeof_sample;
   while (n > 0) {
      to_write = n;
      if ((i = write(playSocket, buffer, to_write)) < 0) {
	 perror("PlayAudio");
	 return 0;
      }
      if (first_write) {
	 first_write = 0;
	 gettimeofday(&playBgnTime, NULL);
	 if (playBgnTime.tv_usec > 1000000) {
	    playBgnTime.tv_sec++;
	    playBgnTime.tv_usec -= 1000000;
	 }
      }
      buffer += i;
      n -= i;

      playSamps += (i / sizeof_sample);
   }

   return (0);
}
