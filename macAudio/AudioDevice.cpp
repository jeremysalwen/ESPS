/*=============================================================================
	AudioDevice.cpp
	
=============================================================================*/

#include "AudioDevice.h"

void	AudioDevice::Init(AudioDeviceID devid, bool isInput)
{
	mID = devid;
	mIsInput = isInput;
	UInt32 stuff[2];
	if (mID == kAudioDeviceUnknown) return;
	
	UInt32 propsize;
	
	propsize = sizeof(UInt32);
	verify_noerr(AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertySafetyOffset, &propsize, &mSafetyOffset));
	
	propsize = sizeof(UInt32);
	verify_noerr(AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertyBufferFrameSize, &propsize, &mBufferSizeFrames));
	
	propsize = sizeof(AudioStreamBasicDescription);
	verify_noerr(AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertyStreamFormat, &propsize, &mFormat));
	stuff[0] = mFormat.mFormatID;
	stuff[1] = 0;
	//	fprintf(stderr,"%d::isIn:%d FormID:%s, FormFlags:%x B/pack:%d F/pack:%d B/frame:%d chan/frame:%d b/chan:%d\n",mID,
	//		isInput, (char*)stuff,mFormat.mFormatFlags,mFormat.mBytesPerPacket,mFormat.mFramesPerPacket,
	//		mFormat.mBytesPerFrame,mFormat.mChannelsPerFrame,mFormat.mBitsPerChannel);

}

bool   AudioDevice::IsInterleavedChannels()
{
	if (mID == kAudioDeviceUnknown)
	  return FALSE;

	return( ! ( mFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) );
}

Float64 AudioDevice::GetSampleRate()
{
	if (mID == kAudioDeviceUnknown)
	  return 0.0;
	return(mFormat.mSampleRate);
}

  
void	AudioDevice::SetBufferSize(UInt32 size)
{
	UInt32 propsize = sizeof(UInt32);
	verify_noerr(AudioDeviceSetProperty(mID, NULL, 0, mIsInput, kAudioDevicePropertyBufferFrameSize, propsize, &size));

	propsize = sizeof(UInt32);
	verify_noerr(AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertyBufferFrameSize, &propsize, &mBufferSizeFrames));
	//	fprintf(stderr,"AudioDevice::SetBufferSize:%d\n",mBufferSizeFrames);
}

int		AudioDevice::CountChannels()
{
	OSStatus err;
	UInt32 propSize;
	int result = 0;
	
	err = AudioDeviceGetPropertyInfo(mID, 0, mIsInput, kAudioDevicePropertyStreamConfiguration, &propSize, NULL);
	if (err) return 0;

	AudioBufferList *buflist = (AudioBufferList *)malloc(propSize);
	err = AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertyStreamConfiguration, &propSize, buflist);
	if (!err) {
		for (UInt32 i = 0; i < buflist->mNumberBuffers; ++i) {
			result += buflist->mBuffers[i].mNumberChannels;
		}
	}
	free(buflist);
	return result;
}

char *	AudioDevice::GetName(char *buf, UInt32 maxlen)
{
	verify_noerr(AudioDeviceGetProperty(mID, 0, mIsInput, kAudioDevicePropertyDeviceName, &maxlen, buf));
	return buf;
}
