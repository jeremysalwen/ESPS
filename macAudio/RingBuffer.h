/*=============================================================================
	RingBuffer.h
	
=============================================================================*/

#ifndef __RingBuffer_h__
#define __RingBuffer_h__

#include <CoreServices/CoreServices.h>

//  Note: In the following, 'bytesPerFrame' will indicate the actual
//  nymber of Bytes required for each frame stored, A 'frame' is
//  defined as the collection of samples across all active channels
//  (e.g. for mono, a frame has one sample; stereo has two, etc.)  //
//  Thus, a frame of stereo short-integer samples uses four bytes.

// For now, data will always be stored in the ring as 16-bit signed
// integers (shorts).  It will always be retrieved from the ring as
// single-precision floats.  Unless otherwise specified, the shorts
// will be converted to floats using float[i] = short[i]/32768.0

// In all cases, a 'sample' in the ring uses two bytes, and is a
// 16-bit signed integer.  A frame can have any number of samples.
// Frames occur at the sample rate.


class RingBuffer {
 public:
  RingBuffer(UInt32 bytesPerSample, UInt32 capacityFrames, bool isInterleaved, int nChannels);
  ~RingBuffer();
  void	Allocate(UInt32 bytesPerSample, UInt32 capacityFrames, int nChannels);
  void	Clear();

  // Note: Store() and Fetch() need be called only once per
  // buffer with interleaved samples data, but will need to be
  // called for each channel independently for interleaved
  // buffer formats.  This avoids the necessity of having
  // RingBufffer concerned with buffer sizes, etc.

  // Return the number of items of storage available after the call.
  // If negative, means buffer overflow.
  SInt32	Store(const short *data, UInt32 nItems);
  SInt32	StoreFloatsAsShorts(const float *data, UInt32 nItems);
  // Return the number of items of data left in the ring adfter the call.
  // If negative, means underflow.
  SInt32	Fetch( short *data, UInt32 nItems);
  SInt32	FetchShortsAsFloats( float *data, UInt32 nItems);
  // Note that for interleaved data, nItems is identical to the number
  // of frames.  Otherwise, nItems is the number of samples for each
  // channel.  In the latter case, Store() and Fetch() must be called
  // for each channel.  This is because the non-interleaved data
  // buffers from the AudioDeviceIOProcs are not contiguous in memory!

  // Return the number of frames that can currently be added to the ring.
  UInt32 GetSpaceAvailableFrames();
  // Return the number of frames of valid data currently in the ring.
  UInt32 FramesInRing();


  bool      mInterleavedChannels;
  UInt32    mNChannels;
  UInt32		mBytesPerFrame;
  UInt32		mCapacityFrames;
  UInt32		mCapacityBytes;
  UInt32		mStartStorage;
  UInt32		mStartFetch;
  bool                  mWrapped;
  Byte *		mBuffer;
};

#endif // __RingBuffer_h__
