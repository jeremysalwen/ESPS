#include "macAudioDevice.h"

#include "AudioDevice.h"
#include "AudioInput.h"
#include "AudioOutput.h"
#include "RingBuffer.h"
#include "FdFilter.h"

// NOTE: In the following, 'nFrames' will refer to the number of
// multi-channel samples being measured or transferred.  On G4 Macs
// with OS-x, there are two channels by default.  Thus, nFrames will
// generally mean that ( sizeof(short) * 2 * nFrames ) bytes of data
// are being transferred.  The default hardware supp;ies only two
// channels, and these are always interleaved on a per-sample basis.

static void *_openOutput(char *name, float sampleRate)
{
  AudioDeviceID devID;
  UInt32 propsize = sizeof(AudioDeviceID);
  OSStatus retVal = 10;
  AudioOutput *aOutput = NULL;

    retVal = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propsize, &devID);
    if(retVal) {
      fprintf(stderr,"Failure opening output\n");
      return(NULL);
    }
    aOutput = new AudioOutput;
    aOutput->SetBufferSize(512);
    aOutput->SetOutputDevice(devID, sampleRate);
    return(aOutput);
}

static void *_openInput(char *name, float sampleRate)
{
  AudioDeviceID devID;
  UInt32 propsize = sizeof(AudioDeviceID);
  OSStatus retVal = 10;
  AudioInput *aInput = NULL;

    retVal = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propsize, &devID);
    if(retVal) {
      fprintf(stderr,"Failure opening input\n");
      return(NULL);
    }
    aInput = new AudioInput;
    aInput->SetBufferSize(512);
    aInput->SetInputDevice(devID, sampleRate);
    return(aInput);
}

void *audioOpenDefaultDevice(char *name, int isInput, float sampleRate)
{
  if(isInput)
    return(_openInput(name, sampleRate));
  else
    return(_openOutput(name, sampleRate));
}

int audioFramesAvailable(void *device)
{ 
  AudioInput *ai = (AudioInput*)device;
  if(ai) {
    int n  = ai->mInputBuffer->FramesInRing();
    return(n);
  } else
    return(0);
}

int audioFramesBuffered(void *device)
{ 
  AudioOutput *ao = (AudioOutput*)device;
  if(ao) {
    int n  = ao->mOutputBuffer->FramesInRing();
    return(n);
  } else
    return(0);
}

void audioGetFrames(void *device, short *destBuffer, int nFrames)
{
  AudioInput *ai = (AudioInput*)device;
  if(ai) {
    ai->mInputBuffer->Fetch(destBuffer,nFrames);
  }
}

void audioPutFrames(void *device, short *sourceBuffer, int nFrames)
{
  AudioOutput *ao = (AudioOutput*)device;
  if(ao) {
    int nc = ao->mNConverters;
    if(nc > 0) {
      int i, nSamples = nFrames;
      int inLoc = 0;
      while(nSamples > 0) {
	int doInit = ao->mFirstFilterPass;
	int nOutput;
	int nInput = ao->mConverterBufferSize;
	if(nInput > nSamples)
	  nInput = nSamples;
	for(i=0; i < nc; i++) { // outer loop over channels
	  int j,k;
	  for(j=0, k=i+inLoc; j < nInput; j++, k += nc)
	    ao->mIOBuffer[j] = sourceBuffer[k];
	  nOutput = ao->mRateConverters[i]->filterArray(ao->mIOBuffer,nInput,doInit,0,ao->mIOBuffer,ao->mIOBufferSize);
	  for(j= 0, k=i; j < nOutput; j++, k += nc)
	    ao->mWorkBuffer[k] = ao->mIOBuffer[j];
	}
	ao->mOutputBuffer->Store(ao->mWorkBuffer,nOutput);
	ao->mFirstFilterPass = 0;
	nSamples -= nInput;
	inLoc += nInput*nc;
      }
      return;
    }
    ao->mOutputBuffer->Store(sourceBuffer,nFrames);
  }
}

void audioStartOutput(void *device)
{
  AudioOutput *ao = (AudioOutput*)device;
  if(ao) {
    ao->StartOutput();
    ao->EnableOutput(1);
  }
}

void audioStartInput(void *device)
{
  AudioInput *ai = (AudioInput*)device;
  if(ai) {
    ai->StartInput();
    ai->EnableInput(1);
  }
}

void audioCloseOutputDevice(void *device)
{
  AudioOutput *ao = (AudioOutput*)device;
  if(ao) {
    ao->StopOutput();
    delete ao;
  }
  return;
}

void audioCloseInputDevice(void *device)
{
  AudioInput *ai = (AudioInput*)device;
  if(ai) {
    ai->StopInput();
    delete ai;
  }
  return;
}

