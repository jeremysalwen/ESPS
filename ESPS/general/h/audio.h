/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)audio.h	1.5 10/23/96 ERL
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:  Alan Parker
 *
 * Brief description:
 *
 */

#ifndef audio_H
#define audio_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

extern char audio_error_msg[];

void
DoDC ARGS((short int *buffer,
	   int nsamps, int chans, short int *dc));
/* nsamps and chans are specified as short in pre-ANSI style
 * in audio_.c; therefore they are passed as int.
 */

void
GetBuffMaxMin ARGS((register short int *buffer,
		    int samples, int chans,
		    double *smax, double *smin));
/* samples and chans are specified as short in pre-ANSI style
 * in audio_.c; therefore they are passed as int.
 */


int
PortIsAvailable ARGS((void));

void
ScaleData ARGS((short int *indata, short int *outdata, register int nsamps,
		register int channel, int gflag1, register int shift,
		register double gain));
/* gain is specified as float in pre-ANSI style in audio_.c;
 * therefore it is passed as double.
 */

int *
AudioGetRates ARGS((void));

int
AudioMaxChannels ARGS((void));

int
InitAudioRecord ARGS((double srate, int channel, int nSampsPerChan));

int
StartAudioRecord ARGS((void));

int
CloseAudioRecord ARGS((void));

int
PauseAudioRecord ARGS((void));

int
ContAudioRecord ARGS((void));

int
RecordAudio ARGS((short int *inputBuffer));

int
SetAudioInputType ARGS((char *src));

int
SetAudioInputGain ARGS((int gainL, int gainR));

int
InitAudioPlay ARGS((double srate, int channel, int nSampsPerChan));

int
CanWriteAudio ARGS((void));

int
StartAudioPlay ARGS((void));

int
CloseAudioPlay ARGS((int hold_port));

int
PauseAudioPlay ARGS((void));

int
ContAudioPlay ARGS((void));

int
PlayAudio ARGS((short int *buffer, int nSamps));

int
GetPlayFilled ARGS((void));

int
PlayAudioDrain ARGS((void));

int
SetAudioOutputType ARGS((char *out));

int
SetAudioOutputGain ARGS((int gainL, int gainR));

int 
closest_srate ARGS((double req_rate, int *rates));

void
E_usleep ARGS((unsigned int usec));


#ifdef __cplusplus
}
#endif

#endif /* audio_H */
