#ifndef __MAC_AUDIO_DEVICE
#define  __MAC_AUDIO_DEVICE


#ifdef __cplusplus
extern "C" {
#endif

static void *_openOutput(char *name, float sampleRate);

static void *_openInput(char *name, float sampleRate);

void *audioOpenDefaultDevice(char *name, int isInput, float sampleRate);

int audioFramesAvailable(void *device);

int audioFramesBuffered(void *device);

void audioGetFrames(void *device, short *destBuffer, int nShorts);

void audioPutFrames(void *device, short *sourceBuffer, int nShorts);

void audioStartOutput(void *device);

void audioStartInput(void *device);

void audioCloseOutputDevice(void *device);

void audioCloseInputDevice(void *device);


#ifdef __cplusplus
}
#endif

#endif
