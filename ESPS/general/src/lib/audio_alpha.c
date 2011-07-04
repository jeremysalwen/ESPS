/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1995 Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Checked by: Revised by:
 * 
 * Brief description:
 * 
 */

static char    *sccs_id = "@(#)audio_alpha.c	1.1 21 Mar 1998 ERL";

char            audio_error_msg[100];

int
PortIsAvailable()
{
   return 0;
}


int
InitAudioPlay(srate, channel, nSampsPerChan)
   double          srate;
   int             channel;
   int             nSampsPerChan;
{
   return (0);
}

int
GetPlayFilled()
{
   return 0;
}

int
CanWriteAudio()
{
   return 0;
}


int
PlayAudioDrain()
{
   return 0;
}

int
CloseAudioPlay(hold_port)
   int             hold_port;	/* if non-zero, hold port open */
{
   return (0);
}

int
StartAudioPlay()
{
   return 0;
}

int
PlayAudio(buffer, nSamps)
   short          *buffer;
   int             nSamps;
{
   return (0);
}

