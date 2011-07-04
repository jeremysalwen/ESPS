#include <iostream>
#include "AudioOutput.h"
#include "AudioInput.h"
#include "RingBuffer.h"
#include "AudioDevice.h"
#include "macAudioDevice.h"
#include <getopt.h>

static void	*aOutput = NULL;
AudioInput	*aInput = NULL;

void exit_gracefully(int status)
{
  // Do any necessary cleanup like stopping A/D and D/A
	if (aOutput)
		audioCloseOutputDevice(aOutput);
	exit(status);
}

void	CheckErr(OSStatus err)
{
	if (err) {
		fprintf(stderr, "Error %-4.4s %ld\n", (char *)&err, err);
		exit_gracefully(1);
	}
}

// This is a preliminary implementation.  Presently, it assumes 2-channel interleaved samples.
static UInt32 writeSamples(AudioInput *ai, int nChan, SInt32 nAvailable)
{
  RingBuffer *rb = ai->GetRingBuffer();
  static short *input = NULL, *work;
  static UInt32 inSize = 0;
  UInt32 toMove = nAvailable * rb->mNChannels;
  UInt32 bufSize = ai->mBufferSize * rb->mNChannels;
  int nWritten = 0;
  int i = nChan, j, toSkip = 1, shortsPerFrame = rb->mNChannels;

  if((nChan == 1) && (rb->mNChannels > 1)) {
    toSkip = rb->mNChannels;
    shortsPerFrame = 1;
  }

  if(inSize < bufSize) {
    if(input)
      delete input;
    if(work)
      delete work;
    input = new short [bufSize];
    work = new short [bufSize];
    inSize = bufSize;
  }
  while(nAvailable >= ai->mBufferSize) {
    rb->Fetch(input, ai->mBufferSize);
    if(toSkip > 1)
      for(i=0, j=0; i < ai->mBufferSize; i++, j += toSkip) {
	input[i] = input[j];
      }
    nWritten += fwrite(input, sizeof(short)*shortsPerFrame, ai->mBufferSize, stdout);
    nAvailable -= ai->mBufferSize;
  }
  return(nWritten);
}

static UInt32 stuffit(void *ao, int nChan)
{
  static short *input = NULL, *work;
  static UInt32 inSize = 0;
  UInt32 bufferSize = 512;
  UInt32 toMove = bufferSize * 2;  // Assume two channels for output device.
  int nRead = 0;
  int i,j;

  if(inSize < toMove) {
    if(input)
      delete input;
    if(work)
      delete work;
    input = new short [toMove];
    work = new short [toMove];
    inSize = toMove;
  }

  nRead = fread(input,sizeof(short)*nChan, bufferSize, stdin);
  if(nRead <= 0)
    return(nRead);
  
  //   if( ! rb->mInterleavedChannels) {
  if(0) {
      fprintf(stderr,"Must use interleaved-channel format\n"); // Until we do this right...	
      return(0);
#ifdef OBSOLETE
      int i, lim = ao->mBufferSize * nChan;
      for(i=nRead; i < lim; i ++) {
	input[i] = 0; // Pad last buffer with 0, as needed
      }
      if(nChan == 1) {
	rb->Store(input,ao->mBufferSize);
	rb->Store(input,ao->mBufferSize);
      } else {
	for(i=0, j=0; i < ao->mBufferSize; i++, j += 2)
	  work[i] = input[j];
	rb->Store(work,ao->mBufferSize);
	for(i=0, j=1; i < ao->mBufferSize; i++, j += 2)
	  work[i] = input[j];
	rb->Store(work,ao->mBufferSize);
      }
#endif
    } else {
      if(nChan == 1) {
	for(j = (nRead*2)-1, i = nRead -1; i >= 0; i--, j -= 2) {
	  input[j] = input[i];  // Replicate mono across both channels.
	  input[j-1] = input[i];
	}
      }
      audioPutFrames(ao,input,nRead);
    }
   return(nRead);
}

#define USAGE { fprintf(stderr,"Usage: %s [-n<1|2>]\n%s reads stdin and plays using Mac audio output.\n"); exit(-1); }

int main(int ac, char *av[])
{
  AudioDeviceID inputDevice;
  UInt32 propsize, inRing, framesOut, nIOChannels = 1;
  RingBuffer *rb = NULL;
  propsize = sizeof(AudioDeviceID);
  int c, isRecorder = 0;
  float sampleRate = 0.0;

  while((c = getopt(ac,av,"-n:f:r")) != EOF) {
    switch(c) {
    case 'n':
      nIOChannels = atoi(optarg);
      if((nIOChannels < 1) || (nIOChannels > 2)) {
	fprintf(stderr,"Can only process i- or 2-channel, signed 16-bit integer samples.\n");
	USAGE;
      }
      break;
    case 'f':
      sampleRate = (float)atof(optarg);
      break;
    case 'r':
      isRecorder = 1;
      break;
    default:
      USAGE;
    }
  }

  if( ! isRecorder ) {
    aOutput = audioOpenDefaultDevice("audioM", 0, sampleRate);
    stuffit(aOutput,nIOChannels);
    stuffit(aOutput,nIOChannels);
    stuffit(aOutput,nIOChannels);
    stuffit(aOutput,nIOChannels);
    audioStartOutput(aOutput);
    while(1) {
      int nb;
      if((nb = audioFramesBuffered(aOutput)) < 10000) {
	framesOut = stuffit(aOutput,nIOChannels);
      }
      usleep(5000);
      if(framesOut <= 0) {
	while((framesOut = audioFramesBuffered(aOutput)) > 0) {
	  usleep(1000);
	}
	audioCloseOutputDevice(aOutput);
	exit(0);
      }
    }
  } else { // This is the recording mode.
    SInt32 available;
    CheckErr (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propsize, &inputDevice));
    aInput = new AudioInput;
    aInput->SetBufferSize(512);
    aInput->SetInputDevice(inputDevice,sampleRate);
    rb = aInput->GetRingBuffer();
    aInput->StartInput();
    aInput->EnableInput(1);
    while(1) {
      if(( available = rb->FramesInRing()) > 0) {
	framesOut = writeSamples(aInput, nIOChannels, available);
      } else
	usleep(10000);
    }
  }
  return(0);
}

