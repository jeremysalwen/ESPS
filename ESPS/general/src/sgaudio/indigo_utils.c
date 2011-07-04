/* indigo_utils.c */
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
 * Written By David Talkin
 *
 * Brief description: support routines for Indigo native DAC and ADC
 *
 */
static char *sccs_id = "@(#)indigo_utils.c	1.1 4/4/96 ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <esps/epaths.h>
#include <malloc.h>
#include <audio.h>


extern int channel, debug_level;
extern void (*sig_save[3])();

static int indigo_rates[] = {48000, 44100, 32000, 29400, 24000, 22050, 16000, 14700,
			       12000, 11025, 10667, 9800, 9600, 8820, 8000, 7350,
			       6857, 6400, 6300, 6000, 5880, 5512, 5333, 4900,
			       4571, 4200, 4000, 3675, 0};
extern ALport alport;
extern int ad_done;


/*************************************************************************/
close_alport()
{
  if(alport)
    ALcloseport(alport);
  alport = NULL;
}

/*************************************************************************/
/* Return gracefully if we're running on anything besides a
 * 4D/35 or an Indigo. */
int sg_audio_is_available()
{
  int fd;
  if ((fd = open("/dev/hdsp/hdsp0master", O_RDWR)) >= 0)   {
    close(fd);
    return(1);
  } else
    return(0);
}

/*************************************************************************/
static completion()
{
#if !defined(SG)
  return;
#else
  if(!ad_done) {

    ad_done = TRUE;
    close_alport();
    /* Restore the original signal handlers. */
    signal(SIGINT,sig_save[1]);
    signal(SIGQUIT,sig_save[2]);
    signal(SIGUSR1,sig_save[3]);
  }
#endif
}

/*************************************************************************/
closest_srate(freq)
     double *freq;
{
  register int *rates = indigo_rates;

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

