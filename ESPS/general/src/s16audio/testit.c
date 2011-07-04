/*

  Call this file testit.c and compile it as follows:

 cc -o testit testit.c -lm

 If you run this program like this:

	 testit 8000 770

repeatedly, you will observe that it seriously truncates the output on
alternate runs.  This is on SUNOS 4.1.3_U1 running on a sparc-5.

*/

#include <sys/ioctl.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/file.h>
#include <sun/audioio.h>

#ifndef AUDIO_DEV_CS4231
#define AUDIO_DEV_CS4231	(5)
#endif

#define SRATE 8000.0;

static int  infd = -1, obuffersize, sizeof_sample = sizeof(short), amax, amin;
static short *inbuff=NULL;

static int sparc_rates[] = {48000, 44100, 37800, 32000,  22050, 18900,
				16000, 11025, 9600, 8000, 0};

int audio_port = -1;		/* audio port */
int audio_port_ctl = -1;	/* control port */
int debug_level = 0;

audio_info_t audio_info;

#ifndef MAX_AUDIO_DEV_LEN
/*
 * Paramter for the AUDIO_GETDEV ioctl to determine current audio device
*/
#define MAX_AUDIO_DEV_LEN       (16)
typedef struct audio_device {
        char    name[MAX_AUDIO_DEV_LEN];
        char    version[MAX_AUDIO_DEV_LEN];
        char    config[MAX_AUDIO_DEV_LEN];
} audio_device_t;
#endif

#ifndef AUDIO_GETDEV
#define AUDIO_GETDEV    _IOR(A, 4, audio_device_t)
#endif


/*************************************************************************/
close_alport()
{
  if(audio_port >= 0) {
    ioctl(audio_port,AUDIO_DRAIN, NULL);
    close(audio_port);
  }
  if(audio_port_ctl >= 0)
    close(audio_port_ctl);

  audio_port = -1;
  audio_port_ctl = -1;
}

/*************************************************************************/
/* Return gracefully if we're running on anything besides a
 * Sparc with 2 channel 16 bit audio */
int sparc_audio_is_available()
{
  int audio_dev;

  if ((audio_port_ctl = open("/dev/audioctl", O_RDWR)) >= 0)   {

    if(ioctl(audio_port_ctl, AUDIO_GETDEV, &audio_dev) < 0) {
	perror("error getting audio device characteristics");
	return(0);
    }
    if(audio_dev != AUDIO_DEV_SPEAKERBOX && audio_dev != AUDIO_DEV_CODEC &&
       audio_dev != AUDIO_DEV_CS4231){
	fprintf(stderr,"This program requires SUN dbri audio.\n");
	return(0);
    }
    return 1;
  }
  return 0;
}

/*************************************************************************/
closest_srate(freq, rates)
     double *freq;
     register int *rates;
{
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


main(ac,av)
     char **av;
     int ac;
{
  short samps[10000];
  double arg,  srate;
  int nsamps, i;
  
  if(ac != 3) {
    fprintf(stderr,"USAGE:%s srate #_samples\b",av[0]);
    exit(-1);
  }

  srate = atof(av[1]);
  nsamps = atoi(av[2]);
  if(nsamps > 10000) nsamps = 10000;

  if (!sparc_audio_is_available())  {
    fprintf(stderr, 
	    "%s: can't play audio data on this hardware platform\n",
	    "dacmaster_sparc");
    exit(-1);
  }

  for(i=0, arg = 300.0*3.1415927*2.0/srate; i < nsamps; i++)
    samps[i] = 10000 * sin(arg * i);

  close_alport();       /* D/A may still be in progress. */

  /* Set the sample rate. */
  AUDIO_INITINFO(&audio_info);
  audio_info.play.sample_rate =  closest_srate(&srate, sparc_rates); 
  audio_info.play.precision = 16;
  audio_info.play.channels = 1;
  audio_info.play.encoding = 3; /* LINEAR */
  audio_info.play.error = 0;
  audio_info._yyy[0] = 0;

  /* open the audio device */
  if((audio_port = open("/dev/audio", O_WRONLY)) < 0) {
	perror("Cannot open audio device for output");
	return(-1);
  }
  
  if(ioctl(audio_port, AUDIO_SETINFO, &audio_info) < 0) {
	perror("xwaves output: error setting audio port mode");
	return(-1);
  }

  write(audio_port, (char *)samps, (nsamps-87)*(sizeof_sample));

  write(audio_port, (char *)(samps + (nsamps-87)), 87*(sizeof_sample));

  close_alport();

  exit(0);
}
