/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written By David Talkin Alan Parker
 *
 * Brief description: support routines for sparc dbri
 *
 */
static char *sccs_id = "@(#)dbri_utils.c	1.2 14 Mar 1996 ERL";

#include <Objects.h>
#include <signal.h>
#include <esps/epaths.h>
#include <malloc.h>
#ifdef OS5
#include <sys/audioio.h>
#else
#include <sun/audioio.h>
#endif


extern int channel, debug_level;
extern void (*sig_save[3])();

static int sparc_rates[] = {48000, 44100, 37800, 32000,  22050, 18900,
				16000, 11025, 9600, 8000, 0};
extern int  audio_port;
extern int  audio_port_ctl;
extern int ad_done;

#define AUDIOCTL "/dev/audioctl"

/*************************************************************************/
close_alport()
{
  if(audio_port)
    close(audio_port);
  if(audio_port_ctl)
    close(audio_port_ctl);
  audio_port = NULL;
  audio_port_ctl = NULL;
}

/*************************************************************************/
int dbri_audio_is_available()
{
        if ((audio_port_ctl = open(AUDIOCTL,O_RDWR)) < 0) {
         fprintf(stderr,"Cannot open Sparc audio\n");
         return 0;
        }
        return 1;
}

/*************************************************************************/
static completion()
{
  if(!ad_done) {

    ad_done = TRUE;
    close_alport();
    /* Restore the original signal handlers. */
    signal(SIGINT,sig_save[1]);
    signal(SIGQUIT,sig_save[2]);
    signal(SIGUSR1,sig_save[3]);
  }
}

/*************************************************************************/
closest_srate(freq)
     double *freq;
{
  register int *rates = sparc_rates;

  if(freq && rates && (*rates > 0)) {
    register int i, ifr = *freq + 0.5, best, min;

    best = *rates++;
    min = abs(best-ifr);
    while(*rates) {
      if((i = abs(*rates - ifr)) < min) {
	best = *rates;
	if((min = i) == 0) break;
      }
      rates++;
    }
    if(best != ifr) {
      fprintf(stderr,"Closest rate available to that requested (%f) is %d\n",
	      *freq,best);
      *freq = best;
    }
    return(best);
  } else
    fprintf(stderr,"Bad args to closest_srate()\n");
  return(0);
}

