/*=============================================================================
	AudioOutput.h
	
=============================================================================*/

#ifndef __AudioOutput_h__
#define __AudioOutput_h__

#include "AudioDevice.h"

class FdFilter;
class RingBuffer;

class AudioOutput {
 public:
  AudioOutput();
  ~AudioOutput();
	
  void	SetOutputDevice(AudioDeviceID output, float sampleRate);
	
  void	StartOutput();
  bool	StopOutput();
  void	EnableOutput(bool enable) { mOutputing = enable; }
  void	SetBufferSize(UInt32 size);
	
  double	GetOutputTime() { return mOutputTime; }
	
  char *	GetErrorMessage() { return mErrorMessage; }
  RingBuffer * GetRingBuffer() { return mOutputBuffer; }

 protected:
  enum IOProcState {
    kOff,
    kStarting,
    kRunning,
    kStopRequested
  };
	
  static OSStatus OutputIOProc(	AudioDeviceID			inDevice,
				const AudioTimeStamp*	inNow,
				const AudioBufferList*	inInputData,
				const AudioTimeStamp*	inInputTime,
				AudioBufferList*		outOutputData,
				const AudioTimeStamp*	inOutputTime,
				void*					inClientData);

  // Return the actual sample rate achieved.
  float SetupRateConverter(float nativeRate, float desiredRate);

 public:
  FdFilter          *mRateConverters[6];
  short             *mIOBuffer, *mWorkBuffer;
  int               mNConverters;
  int               mConverterBufferSize;
  int               mIOBufferSize;
  int               mFirstFilterPass;
  AudioDevice		mOutputDevice;
  bool			mRunning;
  bool			mOutputing;
  IOProcState		mOutputProcState;
  Float64			mLastInputSampleCount, mIODeltaSampleCount;
  UInt32			mBufferSize;
  UInt32                        mNChannels;
  Float64			mSampleRate;
  RingBuffer                    *mOutputBuffer;
  UInt64			mOutputTime;
  char			mErrorMessage[128];
  SInt32 erf;
};


#endif // __AudioOutput_h__
