/*=============================================================================
	AudioInput.h
	
=============================================================================*/

#ifndef __AudioInput_h__
#define __AudioInput_h__

#include "AudioDevice.h"

class RingBuffer;
class FdFilter;

class AudioInput {
 public:
  AudioInput();
  ~AudioInput();
	
  void	SetInputDevice(AudioDeviceID input, float sampleRate);
  float SetupRateConverter(float nativeRate, float desiredRate);
  void	StartInput();
  bool	StopInput();
  void	EnableInput(bool enable) { mInputing = enable; }
  void	SetBufferSize(UInt32 size);
	
  double	GetInputTime() { return mInputTime; }
	
  char *	GetErrorMessage() { return mErrorMessage; }
  RingBuffer * GetRingBuffer() { return mInputBuffer; }

 protected:
  enum IOProcState {
    kOff,
    kStarting,
    kRunning,
    kStopRequested
  };
	
  static OSStatus InputIOProc(	AudioDeviceID			inDevice,
				const AudioTimeStamp*	inNow,
				const AudioBufferList*	inInputData,
				const AudioTimeStamp*	inInputTime,
				AudioBufferList*		outInputData,
				const AudioTimeStamp*	inInputTime,
				void*					inClientData);

 public:
  FdFilter          *mRateConverters[6];
  short             *mIOBuffer, *mWorkBuffer;
  int               mNConverters;
  int               mConverterBufferSize;
  int               mIOBufferSize;
  int               mFirstFilterPass;
  AudioDevice		mInputDevice;
  bool			mRunning;
  bool			mInputing;
  IOProcState		mInputProcState;
  Float64			mLastInputSampleCount, mIODeltaSampleCount;
  UInt32			mBufferSize;
  UInt32                        mNChannels;
  Float64			mSampleRate;
  RingBuffer *mInputBuffer;
  UInt64			mInputTime;
  char			mErrorMessage[128];
  SInt32 erf;
};


#endif // __AudioInput_h__
