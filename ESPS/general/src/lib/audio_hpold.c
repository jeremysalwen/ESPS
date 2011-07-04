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

static char    *sccs_id = "%W% %G% ERL";

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
static AInputChMask InputChannelsSupported;

/* record */
static AudioAttrMask recordAttribsMask;
static AudioAttributes recordAttribs;
static SSRecordParams recordStreamParams;
static SStream  recordStream;
static ATransID recordXid;
static int      recordSocket = -1;
static int      recordGainR, recordGainL;
static AGainEntry recordGainEntry[4];
static int      recordDestSet = 0;
static int      recordGainSet = 0;
static double   recordRate;
static int      recordChannel;
static int      recordNSampsPerChan;
static          recordInitCalled = 0;

/* play */
static AudioAttrMask playSrcAttribsMask;
static AudioAttrMask playAttribsMask;
static AudioAttributes playSrcAttribs;
static AudioAttributes playAttribs;
static SSPlayParams playStreamParams;
static SStream  playStream;
static ATransID playXid = NULL;
static int      playSocket = -1;/* <0 when audio is in close state */
#ifndef WRITE
static FILE    *playFP = NULL;
#endif
static int      playChan = 1;
static int      playPortBufSize;
static int      fileRate;
static unsigned long     playSamps = 0;
static long     playPauseCount = 0;
static int      playPause = 0;
static struct timeval playBgnTime;
static AGainEntry playGainEntry[4];
static int      playGainSet = 0;
static int      playGainR, playGainL;
static int      playDrain = 0;
static double   playRate;
static int      playNSampsPerChan;
static int      playInitCalled = 0;
static int      useIntSpeaker = 0;
static AErrorHandler prevHandler;
#ifdef WRITE_TEST
static FILE *      test_fp = NULL;
#endif
static int      first_write=0;

char            audio_error_msg[512];
#define perror private_perror
extern int      sys_nerr;
extern char    *sys_errlist[];
int             closest_srate();

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


#define PLAYHOLD 300000

static int
GetFilled(fd)
   int             fd;
{
   int             canread;
   if (ioctl(fd, FIONREAD, &canread) < 0) {
      perror("error in GetFilled");
      return (0);
   }
   return canread / sizeof_sample;
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

   audio = AOpenAudio(server, &status);
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
   InputChannelsSupported = AInputChannels(audio);
   return (0);
}


/*
 * ----------------------------------------------------------------- Record
 * -----------------------------------------------------------------
 */



int
AudioMaxChannels()
{

   /*
    * InputChannelsSupported is a bit mask that represents the available A/D
    * input channels on the hardware. Here we use that fact that mono
    * capabilities is numerically equal to 1 and that the upper limit is two
    * channels.
    */

   if ((InputChannelsSupported & AMonoInputChMask) &&
       !(InputChannelsSupported & ALeftInputChMask) &&
       !(InputChannelsSupported & ARightInputChMask))
      return 1;
   else if ((InputChannelsSupported & ALeftInputChMask) &&
	    (InputChannelsSupported & ARightInputChMask))
      return 2;
   else
      return 0;
}


int            *
AudioGetRates()
{
   return (sampleRates);
}


int
InitAudioRecord(srate, channel, nSampsPerChan)
   double          srate;
   int             channel;
   int             nSampsPerChan;
{
   long            status;
   if (PortIsAvailable()) {
      (void) sprintf(audio_error_msg, "Sorry, this record program requires hp700 audio.\n");
      return (1);
   }
   recordChannel = channel;
   recordNSampsPerChan = nSampsPerChan;
   sizeof_sample = sizeof(short);

   /* Set the sample rate. */
   recordAttribs = *ABestAudioAttributes(audio);
   recordRate = recordAttribs.attr.sampled_attr.sampling_rate =
      closest_srate(srate, sampleRates);

   /*
    * HP's audio server has a bufer of limited audio->block_size size. At
    * least on HP700, this is 4096 (N) bytes.  Number of data points
    * transferred must not exceed  N bytes in every consecutive read;
    * otherwise, overflow occurs, unread data gets overwritten in the buffer.
    * The result is samples lost during recording.  Here I use a rule of data
    * transfered every some sec must not exceed N bytes (enough so that
    * single channel 48 Hz is recordable)
    */
   if (0.08 * srate * channel * sizeof_sample > audio->block_size) {
      sprintf(audio_error_msg,
	      "InitAudioRecord: Sampling rate too high or too many channels specified.\n");
      ACloseAudio(audio, NULL);
      return (1);
   }
   /* Set up defaults in the HP audio attrib structure */
   recordAttribsMask = 0;
   recordAttribsMask = (recordAttribsMask | ASSamplingRateMask |
			ASChannelsMask | ASDataFormatMask);
   recordAttribs.attr.sampled_attr.channels = (channel < 2) ? 1 : 2;
   recordAttribs.attr.sampled_attr.data_format = ADFLin16;
   recordAttribs.attr.sampled_attr.bits_per_sample = 16;

   switch (recordAttribs.attr.sampled_attr.channels) {
   case 1:
      recordGainEntry[0].u.i.in_ch = AICTMono;
      recordGainEntry[0].gain = AUnityGain;
      break;
   case 2:
   default:
      recordGainEntry[0].u.i.in_ch = AICTLeft;
      recordGainEntry[1].u.i.in_ch = AICTRight;
      recordGainEntry[0].gain = AUnityGain;
      recordGainEntry[1].gain = AUnityGain;
      break;
   }

   recordStreamParams.event_mask = 0;
   recordStreamParams.gain_matrix.type = AGMTInput;
   recordStreamParams.gain_matrix.num_entries =
      recordAttribs.attr.sampled_attr.channels;
   recordStreamParams.gain_matrix.gain_entries = recordGainEntry;
   recordXid = ARecordSStream(audio, ~0, &recordAttribs, &recordStreamParams,
			      &recordStream, &status);
   if (status) {
      sprintf(audio_error_msg,
	      "InitAudioRecord: can't open audio stream, code %ld.\n",
	      status);
      ACloseAudio(audio, NULL);
      return 1;
   }
   recordInitCalled = 1;
   return (0);
}

int
CloseAudioRecord()
{
   long            status;
   if (recordSocket < 0)
      return (0);
   close(recordSocket);
   recordSocket = -1;
   recordInitCalled = 0;

   ACloseAudio(audio, &status);
   audio = NULL;
   return (status);
}

int
PauseAudioRecord()
{
   (void) CloseAudioRecord();
   return (0);
}

int
StartAudioRecord()
{
   long            status;
   recordXid = ARecordSStream(audio, ~0, &recordAttribs, &recordStreamParams,
			      &recordStream, &status);
   if (status) {
      sprintf(audio_error_msg,
	      "StartAudioRecord: can't open audio stream, code %ld.\n",
	      status);
      (void) ACloseAudio(audio, NULL);
      return 1;
   }
   recordSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (recordSocket < 0) {
      perror("StartAudioRecord: Socket creation failed");
      return 1;
   }
   status = connect(recordSocket,
		    (struct sockaddr *) & recordStream.tcp_sockaddr,
		    sizeof(struct sockaddr_in));
   if (status < 0) {
      perror("StartAudioRecord: Connect failed");
      return (1);
   }
   if (recordGainSet) {
      switch (recordAttribs.attr.sampled_attr.channels) {
      case 1:
	 ASetChannelGain(audio, recordXid, ACTMono, recordGainL, &status);
	 if (status) {
	    sprintf(audio_error_msg,
		    "StartAudioRecord: can't set channel gain, code %ld.\n",
		    status);
	    (void) ACloseAudio(audio, NULL);
	    return 1;
	 }
	 break;
      case 2:
      default:
	 ASetChannelGain(audio, recordXid, ACTLeft, recordGainL, &status);
	 ASetChannelGain(audio, recordXid, ACTRight, recordGainR, &status);
	 if (status) {
	    sprintf(audio_error_msg,
		    "StartAudioRecord: can't set channel gain, code %ld.\n",
		    status);
	    (void) ACloseAudio(audio, NULL);
	    return 1;
	 }
	 break;
      }
   }
   return (0);
}

int
ContAudioRecord()
{
   int status;
   status = InitAudioRecord(recordRate, recordChannel, recordNSampsPerChan);
   if (status) 
	return 1;
   status = StartAudioRecord();
   if (status) 
	return 1;
   return (0);
}


/*
 * in case of STEREO recording, RecordAudio returns total number of samples
 * in both channels
 */

int
RecordAudio(inputBuffer)
   short          *inputBuffer;
{
   int             canread, haveread = 0;

   /*
    * Sigh, can't find a way to reliably check for overflow condition. That
    * is why a precaution check is installed in InitAudioRecord
    */

   canread = GetFilled(recordSocket);
   if (canread)
          haveread = read(recordSocket, inputBuffer, 
	           canread * sizeof_sample) / sizeof_sample;
   return (haveread);
}

int
SetAudioInputType(src)
   char           *src;
{
   int             useLineIn = 0;

   if (!strcmp(src, "mic"))
      useLineIn = 0;
   else
      useLineIn = 1;

   /*
    * Because InitAudioRecord must be called first to know how many channel
    * we have, We can't set gainEntry which setting is channel dependent
    * before the device is open.
    */
   if (recordInitCalled) {
      /*
       * close up the connection and restart.  Unfortunately HP won't let you
       * change output destination while device is open.
       */
      (void) CloseAudioRecord();
      switch (recordAttribs.attr.sampled_attr.channels) {
      case 1:
	 recordGainEntry[0].u.i.in_ch = AICTMono;
	 recordGainEntry[0].u.i.in_src = (useLineIn) ? AISTMonoAuxiliary :
	    AISTMonoMicrophone;
	 break;
      case 2:
      default:
	 recordGainEntry[0].u.i.in_ch = AICTLeft;
	 recordGainEntry[0].u.i.in_src = (useLineIn) ? AISTLeftAuxiliary :
	    AISTLeftMicrophone;
	 recordGainEntry[1].u.i.in_ch = AICTRight;
	 recordGainEntry[1].u.i.in_src = (useLineIn) ? AISTRightAuxiliary :
	    AISTRightMicrophone;
	 break;
      }
      recordDestSet = 1;
      InitAudioRecord(recordRate, recordChannel, recordNSampsPerChan);
   }
   return 0;
}

int
SetAudioInputGain(gainL, gainR)
   int             gainL, gainR;
{
   long            status;
   float           ratio;
   ratio = (maxOut - minOut) / 100.0;
   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;		/* 0 - 100, 0 min gain */
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   recordGainL = gainL * ratio + minOut;
   recordGainR = gainR * ratio + minOut;
   recordGainSet = 1;

   switch (recordAttribs.attr.sampled_attr.channels) {
   case 1:
      ASetChannelGain(audio, recordXid, ACTMono, recordGainL, &status);
      break;
   case 2:
   default:
      ASetChannelGain(audio, recordXid, ACTLeft, recordGainL, &status);
      ASetChannelGain(audio, recordXid, ACTRight, recordGainR, &status);
      break;
   }
   return 0;
}


/*
 * ----------------------------------------------------------------- Play
 * -----------------------------------------------------------------
 */

static void
takenap(usec)
   unsigned int    usec;
{
   struct timeval  timeout;
   timeout.tv_sec = usec / 1000000;
   timeout.tv_usec = usec % 1000000;
   if (select(0, 0, 0, 0, &timeout) < 0)
      perror("takenap");
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
      return 1;
   }
#ifdef WRITE_TEST
   test_fp = fopen("test_play_data","w");
#endif
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

#ifdef DEBUG
   fprintf(stderr, "InitAudioPlay: rate: %f, channel: %d, samps: %d\n",
	   srate, channel, nSampsPerChan);
#endif

   playRate = srate;
   playChan = channel;
   playNSampsPerChan = nSampsPerChan;

   if (PortIsAvailable()) {
      (void) sprintf(audio_error_msg, "Sorry, this play program requires HP audio.\n");
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
   playInitCalled = 1;

   return (0);
}

int
GetPlayFilled()
{
   int             sampsUnPlayed, sampsPlayed;
   struct timeval  currTime;

   if (playSocket < 0)
      return 0;

   if (playInitCalled && first_write)
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
   long            status;

   if (playSocket < 0)
      return (0);
fprintf(stderr,"Close: left: %d",GetPlayFilled());

#ifdef WRITE_TEST
   if(test_fp)
      fclose(test_fp);
#endif

   if (playPause) {
      AResumeAudio(audio, playXid, NULL, NULL);
      playPause = 0;
   }

   first_write=0;
fputc('!',stderr);
   AStopAudio(audio, playXid, ASMLinkedTrans, NULL, NULL);
fputc('@',stderr);
   close(playSocket);
fputc('#',stderr);
   playSocket = -1;
   playInitCalled = 0;
   if (!hold) {
/*
      ASetCloseDownMode(audio, ADestroyAll, NULL);
*/
fputc('%',stderr);
      ACloseAudio(audio, &status);
fputc('^',stderr);
      audio = NULL;
   }
fputc('.',stderr);
fputc('\n',stderr);
   return (status);
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
#ifdef PAUSE
   /*
    * start stream paused so we can transfer enough data (PLAYHOLD seconds
    * worth) before playing starts to prevent stream from running out
    */
   APauseAudio(audio, playXid, NULL, NULL);
   playPauseCount = (PLAYHOLD / 1000000.0) * playAttribs.attr.sampled_attr.channels *
      playAttribs.attr.sampled_attr.sampling_rate;
   playPause = 1;
#endif
   first_write=1;
   return (0);
}

/*
 * HP's APauseAudio doesn't stop the audio output immediately, due to their
 * double layers of network buffering.
 */

int
PauseAudioPlay()
{
   int             sampsUnPlayed;
   struct timeval  stopTime;

   if (playSocket < 0)
      return (0);
   AStopAudio(audio, playXid, ASMLinkedTrans, NULL, NULL);
   playDrain = 0;
   CloseAudioPlay(1);
   gettimeofday(&stopTime, NULL);
   sampsUnPlayed = playSamps -
      fileRate * TimeDiff(&playBgnTime, &stopTime) * playChan;
   return (sampsUnPlayed);
}

int
ContAudioPlay()
{
   InitAudioPlay(playRate, playChan, playNSampsPerChan);
   StartAudioPlay();
   return (0);
}

void
SendAudioZeros()
{
   static short buff[400];

   if (playSocket < 0)
	return;

   write(playSocket, buff, 400*sizeof(short));
}
   


int
PlayAudio(buffer, nSamps)
   char           *buffer;
   int             nSamps;
{
   int             i, n, to_write;

   if (playSocket < 0)
	return 0;

#ifdef WRITE_TEST
   fwrite(buffer,sizeof(short),nSamps,test_fp);
#endif

   n = nSamps * sizeof_sample;
   while (n > 0) {
	 to_write = n;
      if ((i = write(playSocket, buffer, to_write)) < 0) {
	 perror("PlayAudio");
	 return 0;
      }
      if(first_write) {
      	first_write=0;
      	gettimeofday(&playBgnTime, NULL);
#ifdef PAUSE
      	playBgnTime.tv_usec += PLAYHOLD;
#endif
      	if (playBgnTime.tv_usec > 1000000) {
         	playBgnTime.tv_sec++;
         	playBgnTime.tv_usec -= 1000000;
      	}
      }
      buffer += i;
      n -= i;

      playSamps += (i / sizeof_sample);
      if (playPause) {
	 playPauseCount -= (i / sizeof_sample);
	 if ((i == 0) || (playPauseCount < 0)) {
	    AResumeAudio(audio, playXid, NULL, NULL);
	    playPause = 0;
	 }
      }
   }

   return (0);
}

int
SetAudioOutputType(out)
   char           *out;
{

   if (!strcmp(out, "speaker"))
      useIntSpeaker = 1;
   else if (!strcmp(out, "lineOut"))
      useIntSpeaker = 0;
   else {
      sprintf(audio_error_msg, "Unknown play port specified\n");
      return 0;
   }

   /*
    * Because InitAudioPlay must be called first to know how many channel we
    * have, We can't set playGainEntry which setting is channel dependent
    * before the device is open.
    */
   if (playInitCalled) {
      /*
       * close up the connection and restart.  Unfortunately HP won't let you
       * change output destination while device is open.
       */
      CloseAudioPlay(0);
      InitAudioPlay(playRate, playChan, playNSampsPerChan);
   }
   return 0;
}
int
SetAudioOutputGain(gainL, gainR)
   int             gainL, gainR;
{
   long            status;
   float           ratio;
   ratio = (maxOut - minOut) / 100.0;
   if (gainL > 100)
      gainL = 100;
   if (gainR > 100)
      gainR = 100;		/* 0 - 100, 0 min gain */
   if (gainL < 0)
      gainL = 0;
   if (gainR < 0)
      gainR = 0;
   playGainL = gainL * ratio + minOut;
   playGainR = gainR * ratio + minOut;
   playGainSet = 1;

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
   return 0;
}
