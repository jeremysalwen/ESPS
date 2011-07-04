/*=============================================================================
	RingBuffer.cpp
	
=============================================================================*/

#include "RingBuffer.h"

RingBuffer::RingBuffer(UInt32 bytesPerSample, UInt32 capacityFrames, bool isInterleaved, int nChannels) :
  mBuffer(NULL)
{
  mInterleavedChannels = isInterleaved;
  mNChannels = nChannels;
  Allocate(bytesPerSample, capacityFrames, nChannels);
}

RingBuffer::~RingBuffer()
{
  if (mBuffer)
    free(mBuffer);
}

void	RingBuffer::Allocate(UInt32 bytesPerSample, UInt32 capacityFrames, int nChannels)
{
  if (mBuffer)
    free(mBuffer);
	
  mBytesPerFrame = bytesPerSample * nChannels;
  mCapacityFrames = capacityFrames;
  mCapacityBytes = mBytesPerFrame * capacityFrames;
  mBuffer = (Byte *)malloc(mCapacityBytes);
  Clear();
}

void	RingBuffer::Clear()
{
  memset(mBuffer, 0, mCapacityBytes);
  mStartStorage = 0;
  mStartFetch = 0;
  mWrapped = 0;
}

UInt32 RingBuffer::FramesInRing()
{
  if(mStartStorage > mStartFetch)
    return( (mStartStorage - mStartFetch)/(sizeof(short)*mNChannels));
  if((mStartStorage == mStartFetch) && ( ! mWrapped ))
     return(0);
  return( (mCapacityBytes - mStartFetch + mStartStorage)/(sizeof(short)*mNChannels));
}

// Transfer shorts from input buffer into the ring.
SInt32	RingBuffer::Store(const short *data, UInt32 nItems)
{
  // $$$ we have an unaddressed critical region here
  // reading and writing could well be in separate threads
	
  int shortsPerItem = (mInterleavedChannels)? mNChannels : 1;
  int shortsToCopy = nItems * shortsPerItem;
  if(shortsToCopy > mCapacityBytes/sizeof(short))
    return( -nItems );

  int shortsAvailable, retVal;
  if(mStartFetch > mStartStorage) {
    shortsAvailable = (mStartFetch - mStartStorage)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal < 0) {
      shortsToCopy = shortsAvailable;
    }
    memcpy(mBuffer + mStartStorage, (char*)data, shortsToCopy * sizeof(short));
  } else {
    if((mStartStorage == mStartFetch) && mWrapped) { // There is no space available.
      return( -nItems );
    }
    shortsAvailable = ((mCapacityBytes - mStartStorage) + mStartFetch)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal < 0) {
      shortsToCopy = shortsAvailable;
    }
    int count1 = (mCapacityBytes - mStartStorage);
    if((count1/sizeof(short)) >= shortsToCopy) {
      memcpy(mBuffer + mStartStorage, (char*)data, shortsToCopy * sizeof(short));
    } else {
      int count2 = (shortsToCopy * sizeof(short)) - count1;
      memcpy(mBuffer + mStartStorage, (char*)data, count1);
      memcpy(mBuffer, ((char*)data) + count1, count2);
    }
  }
  mStartStorage = (mStartStorage + (shortsToCopy * sizeof(short))) % mCapacityBytes;

  if(shortsToCopy > 0)
    mWrapped = 1;

  return(retVal);
}

static void _copyFloats( short *dest, const float *src, UInt32 nItems)
{
  int i;
  float scale = 32768.0;

  for(i=0; i < nItems ; i++) {
    if(src[i] >= 0.0)
      dest[i] = (short)(0.5 + (src[i] * scale));
    else
      dest[i] = (short)((src[i] * scale) - 0.5);
  }
}

// Transfer floats from input to shorts in the ring.
SInt32	RingBuffer::StoreFloatsAsShorts(const float *data, UInt32 nItems)
{
  // $$$ we have an unaddressed critical region here
  // reading and writing could well be in separate threads
	
  int shortsPerItem = (mInterleavedChannels)? mNChannels : 1;
  int shortsToCopy = nItems * shortsPerItem;
  if(shortsToCopy > mCapacityBytes/sizeof(short))
    return( -nItems );

  int shortsAvailable, retVal;
  if(mStartFetch > mStartStorage) {
    shortsAvailable = (mStartFetch - mStartStorage)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal < 0) {
      shortsToCopy = shortsAvailable;
    }
    _copyFloats((short*)(mBuffer + mStartStorage), data, shortsToCopy);
  } else {
    if((mStartStorage == mStartFetch) && mWrapped) { // There is no space available.
      return( -nItems );
    }
    shortsAvailable = ((mCapacityBytes - mStartStorage) + mStartFetch)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal < 0) {
      shortsToCopy = shortsAvailable;
    }
    int count1 = (mCapacityBytes - mStartStorage)/sizeof(short);
    if(count1 >= shortsToCopy) {
      _copyFloats((short*)(mBuffer + mStartStorage), data, shortsToCopy);
    } else {
      int count2 = shortsToCopy - count1;
      _copyFloats((short*)(mBuffer + mStartStorage), data, count1);
      _copyFloats((short*)mBuffer, data + count1, count2);
    }
  }
  mStartStorage = (mStartStorage + (shortsToCopy * sizeof(short))) % mCapacityBytes;

  if(shortsToCopy > 0)
    mWrapped = 1;

  return(retVal);
}

static void _floatCopy( float *fp, const short *sp, UInt32 nToMove)
{
  float scale = 1.0/32768.0;
  UInt32 i;
  for(i=0; i < nToMove; i++) {
    fp[i] = sp[i] * scale;
  }
}

static void _shortCopy(short *dest, short *sp, UInt32 nToMove)
{
  memcpy((char*)dest, (char*)sp, nToMove*sizeof(short) );
}

  //Fetch shorts from the ring.
SInt32	RingBuffer::Fetch(short *data, UInt32 nItems)
{
  int shortsPerItem = (mInterleavedChannels)? mNChannels : 1;
  int shortsToCopy = nItems * shortsPerItem;
  if(shortsToCopy > mCapacityBytes/sizeof(short))
    return( -nItems );

  int shortsAvailable, retVal;
  if(mStartFetch < mStartStorage) {
    shortsAvailable = (mStartStorage - mStartFetch)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal <= 0) {
      shortsToCopy = shortsAvailable;
      mWrapped = 0;
    }
    _shortCopy(data, (short*)(mBuffer + mStartFetch), shortsToCopy);
  } else {
    if((mStartStorage == mStartFetch) && ( ! mWrapped )) { // There are no samples available.
      return( -nItems );
    }
    shortsAvailable = ((mCapacityBytes - mStartFetch) + mStartStorage)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal <= 0) {
      shortsToCopy = shortsAvailable;
      mWrapped = 0;
    }
    int count1 = (mCapacityBytes - mStartFetch)/sizeof(short);
    if(count1 >= shortsToCopy) {
      _shortCopy(data,(short*)(mBuffer + mStartFetch), shortsToCopy);
    } else {
      int count2 = shortsToCopy - count1;
      _shortCopy(data,(short*)(mBuffer + mStartFetch),count1);
      _shortCopy(data+count1,(short*)mBuffer, count2);
    }
  }
  mStartFetch = (mStartFetch + (shortsToCopy * sizeof(short))) % mCapacityBytes;

  return(retVal);
}

  // Converts shorts in the ring to floats in the output buffer.
SInt32	RingBuffer::FetchShortsAsFloats(float *data, UInt32 nItems)
{
  int shortsPerItem = (mInterleavedChannels)? mNChannels : 1;
  int shortsToCopy = nItems * shortsPerItem;
  if(shortsToCopy > mCapacityBytes/sizeof(short))
    return( -nItems );

  int shortsAvailable, retVal;
  if(mStartFetch < mStartStorage) {
    shortsAvailable = (mStartStorage - mStartFetch)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal <= 0) {
      shortsToCopy = shortsAvailable;
      mWrapped = 0;
    }
    _floatCopy(data, (short*)(mBuffer + mStartFetch), shortsToCopy);
  } else {
    if((mStartStorage == mStartFetch) && ( ! mWrapped )) { // There are no samples available.
      return( -nItems );
    }
    shortsAvailable = ((mCapacityBytes - mStartFetch) + mStartStorage)/sizeof(short);
    retVal = (shortsAvailable - shortsToCopy)/shortsPerItem;
    if(retVal <= 0) {
      shortsToCopy = shortsAvailable;
      mWrapped = 0;
    }
    int count1 = (mCapacityBytes - mStartFetch)/sizeof(short);
    if(count1 >= shortsToCopy) {
      _floatCopy(data,(short*)(mBuffer + mStartFetch), shortsToCopy);
    } else {
      int count2 = shortsToCopy - count1;
      _floatCopy(data,(short*)(mBuffer + mStartFetch),count1);
      _floatCopy(data+count1,(short*)mBuffer, count2);
    }
  }
  mStartFetch = (mStartFetch + (shortsToCopy * sizeof(short))) % mCapacityBytes;

  return(retVal);
}
