/*=============================================================================
	AudioOutput.cpp
	
=============================================================================*/

#include "AudioOutput.h"
#include "RingBuffer.h"
#include "FdFilter.h"
#include <unistd.h>

#define kSecondsInRingBuffer 2.0

AudioOutput::AudioOutput() : 
	mRunning(false),
	mOutputing(false),
	mBufferSize(512)
{
	mErrorMessage[0] = '\0';
	mOutputBuffer = NULL;
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

AudioOutput::~AudioOutput()
{
	StopOutput();
	delete mOutputBuffer;
	int i;
	for(i=0; i < mNConverters; i++) {
	  delete mRateConverters[i];
	}
	if(mNConverters > 0) {
	  delete mWorkBuffer;
	  delete mIOBuffer;
	}
}

float   AudioOutput::SetupRateConverter(float nativeRate, float desiredRate)
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
      mRateConverters[i] = new FdFilter(desiredRate, nativeRate, 0, 0.01, 1);
      obtainedRate = mRateConverters[i]->getActualOutputFreq();  // Actual output sample rate.
    }
    mNConverters = mNChannels;
    mConverterBufferSize = mRateConverters[0]->getMaxInputSize();
    mIOBufferSize = mConverterBufferSize;
    if(obtainedRate > desiredRate)
      mIOBufferSize = (int)(mIOBufferSize*(0.5 + (obtainedRate/desiredRate)));
    mIOBuffer = new short [mIOBufferSize];
    int workBuffSize = mIOBufferSize * mNChannels;
    mWorkBuffer = new short [workBuffSize];
    return(desiredRate*nativeRate/obtainedRate);
  }
  return(nativeRate);
}

void	AudioOutput::SetOutputDevice(AudioDeviceID output, float sampleRate)
{
	if (output == kAudioDeviceUnknown)
	  return;
	StopOutput();
	mOutputDevice.Init(output, false);
	mNChannels = mOutputDevice.CountChannels();
	SetBufferSize(mBufferSize);
	if ( mOutputDevice.mFormat.mSampleRate <= 0.0) {
		sprintf(mErrorMessage, "Error - Bizarre sample rate: %f\n",
			 mOutputDevice.mFormat.mSampleRate);
		return;
	}
	mSampleRate = SetupRateConverter(mOutputDevice.mFormat.mSampleRate, sampleRate);

	// Arrange for a ring buffer with capacity of about kSecondsInRing seconds.
	UInt32 rbSize = (UInt32)( 0.5 + ( kSecondsInRingBuffer * mSampleRate));
	if(  mOutputBuffer )
	  delete mOutputBuffer;
	mOutputBuffer = new RingBuffer(sizeof(short),rbSize,mOutputDevice.IsInterleavedChannels(),mNChannels);
	//	fprintf(stderr,"Set up output: Fs=%f Frames=%d Channels=%d IsInterleaved=%d\n",
	//		mSampleRate,mOutputBuffer->mCapacityFrames,mOutputBuffer->mNChannels,mOutputBuffer->mInterleavedChannels);

}

void	AudioOutput::SetBufferSize(UInt32 size)
{
	bool wasRunning = StopOutput();
	mBufferSize = size;
	mOutputDevice.SetBufferSize(size);
	if (wasRunning) StartOutput();
}

void	AudioOutput::StartOutput()
{
	if (mRunning) return;
	//	fprintf(stderr,"Starting StartOutput\n");
	mOutputTime = 0;  // Keep track of number of freames sent to output device.
	if ( !mOutputDevice.Valid()) {
		printf("Invalid output device\n");
		return;
	}
	if( ! mOutputBuffer ) {
	  fprintf(stderr,"No Ring buffer has been initialized\n");
	  return;
	}

	mErrorMessage[0] = '\0';
	erf = 0;
	mRunning = true;
		
	mOutputProcState = kStarting;
	
	verify_noerr (AudioDeviceAddIOProc(mOutputDevice.mID, OutputIOProc, this));
	verify_noerr (AudioDeviceStart(mOutputDevice.mID, OutputIOProc));
	
	while ( mOutputProcState != kRunning)
		usleep(1000);
}


// return whether we were running
bool	AudioOutput::StopOutput()
{
	if (!mRunning) return false;
	mRunning = false;
	
	mOutputProcState = kStopRequested;
	
	while ( mOutputProcState != kOff)
		usleep(5000);
	
	AudioDeviceRemoveIOProc(mOutputDevice.mID, OutputIOProc);
	
	return true;
}

// Output IO Proc
// Rendering output for 1 buffer + safety offset into the future
OSStatus AudioOutput::OutputIOProc(	AudioDeviceID			inDevice,
					const AudioTimeStamp*	inNow,
					const AudioBufferList*	inInputData,
					const AudioTimeStamp*	inInputTime,
					AudioBufferList*		outOutputData,
					const AudioTimeStamp*	inOutputTime,
					void*					inClientData)
{
  AudioOutput *This = (AudioOutput *)inClientData;
	
  switch (This->mOutputProcState) {
  case kStarting:
      This->mOutputProcState = kRunning;
      This->mIODeltaSampleCount = inOutputTime->mSampleTime - This->mLastInputSampleCount;
      break;
  case kStopRequested:
    AudioDeviceStop(inDevice, OutputIOProc);
    This->mOutputProcState = kOff;
    return noErr;
  default:
    break;
  }

  if (This->mOutputing) {
    SInt32 available;
    if(This->mOutputDevice.IsInterleavedChannels()) {
      available = This->mOutputBuffer->FetchShortsAsFloats((float *)(outOutputData->mBuffers[0].mData), This->mOutputDevice.mBufferSizeFrames);
    } else { // Assume it is interleaved buffers...
      // ALSO: Assume there are two channels for now!!!!!!!!!
      available = This->mOutputBuffer->FetchShortsAsFloats((float *)(outOutputData->mBuffers[0].mData), This->mOutputDevice.mBufferSizeFrames);
      available = This->mOutputBuffer->FetchShortsAsFloats((float *)(outOutputData->mBuffers[1].mData), This->mOutputDevice.mBufferSizeFrames);
    }
    if(available <= 0) {
      This->erf = 1;
    } else
      This->mOutputTime += This->mOutputDevice.mBufferSizeFrames;
		
  } else
    This->mOutputTime = 0;
	
  return noErr;
}

