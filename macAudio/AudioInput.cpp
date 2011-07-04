/*=============================================================================
	AudioInput.cpp
	
=============================================================================*/

#include "AudioInput.h"
#include "RingBuffer.h"
#include "fdFilter.h"
#include <unistd.h>

#define kSecondsInRingBuffer 2.0

AudioInput::AudioInput() : 
	mRunning(false),
	mInputing(false),
	mBufferSize(512)
{
	mErrorMessage[0] = '\0';
	mInputBuffer = NULL;
	mNChannels = 0;
	int i;
	for(i=0;i<2;i++) {
	  mRateConverters[i] = NULL;
	}
	mIOBuffer = NULL;
	mWorkBuffer = NULL;
	mConverterBufferSize = 0;
	mNConverters = 0;
}

AudioInput::~AudioInput()
{
  StopInput();
  delete mInputBuffer;
  int i;
  for(i=0; i < mNConverters; i++) {
    delete mRateConverters[i];
  }
  if(mNConverters > 0) {
    delete mWorkBuffer;
    delete mIOBuffer;
  }
}

float   AudioInput::SetupRateConverter(float nativeRate, float desiredRate)
{
  int i;
  float obtainedRate = nativeRate;
  for(i = 0; i < mNConverters; i++) {
    delete mRateConverters[i];
    mRateConverters[i] = NULL;
  }
  mNConverters = 0;
  if(mIOBuffer)
    delete mIOBuffer;
  mIOBuffer = NULL;
  if(mWorkBuffer)
    delete mWorkBuffer;
  mWorkBuffer = NULL;
  mFirstFilterPass = 1;
  if((desiredRate > 0.0) && ((fabs(desiredRate - nativeRate)/desiredRate) > .01)) {
    for(i=0; i < mNChannels; i++) {
      mRateConverters[i] = new FdFilter(nativeRate, desiredRate, 0, 0.01, 1);
      obtainedRate = mRateConverters[i]->getActualOutputFreq();  // Actual output sample rate.
    }
    mNConverters = mNChannels;
    mConverterBufferSize = mRateConverters[0]->getMaxInputSize();
    mIOBufferSize = mConverterBufferSize;
    if(obtainedRate > nativeRate)
      mIOBufferSize = (int)(mIOBufferSize*(0.5 + (obtainedRate/nativeRate)));
    mIOBuffer = new short [mIOBufferSize];
    int workBuffSize = mIOBufferSize * mNChannels;
    mWorkBuffer = new short [workBuffSize];
    return(obtainedRate);
  }
  return(nativeRate);
}

void	AudioInput::SetInputDevice(AudioDeviceID input, float sampleRate)
{
  if (input == kAudioDeviceUnknown) {
    fprintf(stderr,"SetInputDevice called with unknown device\n");
    return;
  }
  StopInput();
  mInputDevice.Init(input, true);
  mNChannels = mInputDevice.CountChannels();
  SetBufferSize(mBufferSize);
  if ( mInputDevice.mFormat.mSampleRate <= 0.0) {
    fprintf(stderr, "Error - Bizarre sample rate: %f\n",
	    mInputDevice.mFormat.mSampleRate);
    return;
  }

  mSampleRate = SetupRateConverter(mInputDevice.mFormat.mSampleRate, sampleRate);

  // Arrange for a ring buffer with capacity of about kSecondsInRing seconds.
  UInt32 rbSize = (UInt32)( 0.5 + ( kSecondsInRingBuffer * mSampleRate));
  if(  mInputBuffer )
    delete mInputBuffer;
  mInputBuffer = new RingBuffer(sizeof(short),rbSize,mInputDevice.IsInterleavedChannels(),mNChannels);
  //  fprintf(stderr,"Set up input: Fs=%f Frames=%d Channels=%d IsInterleaved=%d\n",
  //	  mSampleRate,mInputBuffer->mCapacityFrames,mInputBuffer->mNChannels,mInputBuffer->mInterleavedChannels);

}

void	AudioInput::SetBufferSize(UInt32 size)
{
	bool wasRunning = StopInput();
	mBufferSize = size;
	mInputDevice.SetBufferSize(size);
	if (wasRunning) StartInput();
}

void	AudioInput::StartInput()
{
	if (mRunning) return;
	mInputTime = 0;  // Keep track of number of freames sent to output device.
	if ( !mInputDevice.Valid()) {
		fprintf(stderr,"Invalid input device\n");
		return;
	}
	if( ! mInputBuffer ) {
	  fprintf(stderr,"No Input Ring buffer has been initialized\n");
	  return;
	}

	mErrorMessage[0] = '\0';
	erf = 0;
	mRunning = true;
		
	mInputProcState = kStarting;
	
	verify_noerr (AudioDeviceAddIOProc(mInputDevice.mID, InputIOProc, this));
	verify_noerr (AudioDeviceStart(mInputDevice.mID, InputIOProc));
	
	while ( mInputProcState != kRunning)
		usleep(1000);
}

// return whether we were running
bool	AudioInput::StopInput()
{
	if (!mRunning) return false;
	mRunning = false;
	
	mInputProcState = kStopRequested;
	
	while ( mInputProcState != kOff)
		usleep(5000);
	
	AudioDeviceRemoveIOProc(mInputDevice.mID, InputIOProc);
	
	return true;
}


// Input IO Proc
OSStatus AudioInput::InputIOProc(	AudioDeviceID			inDevice,
					const AudioTimeStamp*	inNow,
					const AudioBufferList*	inInputData,
					const AudioTimeStamp*	inInputTime,
					AudioBufferList*		outOutputData,
					const AudioTimeStamp*	inOutputTime,
					void*					inClientData)
{
  AudioInput *This = (AudioInput *)inClientData;
	
  switch (This->mInputProcState) {
  case kStarting:
    This->mInputProcState = kRunning;
    This->mIODeltaSampleCount = inOutputTime->mSampleTime - This->mLastInputSampleCount;
    return noErr;
  case kStopRequested:
    AudioDeviceStop(inDevice, InputIOProc);
    This->mInputProcState = kOff;
    return noErr;
  default:
    break;
  }

  if (This->mInputing) {
    SInt32 available = 0;
    if(This->mInputDevice.IsInterleavedChannels()) {
      int nc = This->mNConverters;
      if(nc > 0) {
	float *sourceBuffer = (float *)(inInputData->mBuffers[0].mData);
	int i, nSamples = This->mInputDevice.mBufferSizeFrames;
	int inLoc = 0;
	while(nSamples > 0) {
	  int doInit = This->mFirstFilterPass;
	  int nInput = This->mConverterBufferSize;
	  int nOutput;
	  if(nInput > nSamples)
	    nInput = nSamples;
	  for(i=0; i < nc; i++) { // outer loop over channels
	    int j,k;
	    for(j=0, k=i+inLoc; j < nInput; j++, k += nc) {
	      if(sourceBuffer[k] >= 0.0)
		This->mIOBuffer[j] = (short)(0.5 + (sourceBuffer[k] * 32767.0));
	      else
		This->mIOBuffer[j] = (short)((sourceBuffer[k] * 32767.0) - 0.5);
	    }
	    nOutput = This->mRateConverters[i]->filterArray(This->mIOBuffer,nInput,doInit,0,This->mIOBuffer,This->mIOBufferSize);
	    for(j= 0, k=i; j < nOutput; j++, k += nc)
	      This->mWorkBuffer[k] = This->mIOBuffer[j];
	  }
	  available = This->mInputBuffer->Store(This->mWorkBuffer,nOutput);
	  This->mFirstFilterPass = 0;
	  nSamples -= nInput;
	  inLoc += nInput*nc;
	}
      }	else {
	available = This->mInputBuffer->StoreFloatsAsShorts((float *)(inInputData->mBuffers[0].mData),
							    This->mInputDevice.mBufferSizeFrames);
      }
    } else { // Assume it is interleaved buffers...
      // ALSO: Assume there are two channels for now!!!!!!!!!
      available = This->mInputBuffer->StoreFloatsAsShorts((float *)(inInputData->mBuffers[0].mData), This->mInputDevice.mBufferSizeFrames);
      available = This->mInputBuffer->StoreFloatsAsShorts((float *)(inInputData->mBuffers[1].mData), This->mInputDevice.mBufferSizeFrames);
    }
    if(available <= 0) {
      This->erf = 1;
    } else
      This->mInputTime += This->mInputDevice.mBufferSizeFrames;
		
  } else
    This->mInputTime = 0;
  return noErr;
}

