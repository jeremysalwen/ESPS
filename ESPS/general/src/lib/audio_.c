/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)audio_.c	1.4 10/24/96 ERL";

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
/* 
 * -----------------------------------------------------------------
 * Machine-independent utility routines
 * -----------------------------------------------------------------
 */

static float gain_0 = 1.0, gain_1 = 1.0;

void DoDC(buffer, nsamps, chans, dc)
    short *buffer;
    short nsamps;
    short chans;
    short *dc;
{
    static int counter = 0, maxcount = 100000;
    static double aver[2], summer[2];
    static init = 1;
    int i;
    register short *sp;

    if(init){
	summer[0] = 0.0;
	summer[1] = 0.0;
	init = 0;
    }

    if(chans > 1){               /* stereo */
	if(counter < maxcount){
	    i = nsamps / 2;
	    sp = buffer;
	    counter += nsamps / 2;
	    
	    while(i-- > 0){
		summer[0] += *sp++;
		summer[1] += *sp++;
	    }
	    
	    if(counter > 0){
		aver[0] = summer[0]/counter;
		aver[1] = summer[1]/counter;
	    }
	}
	i = nsamps / 2;
	sp = buffer;
	while(i-- > 0){
	    *sp++ -= aver[0];
	    *sp++ -= aver[1];
	}
    } else {
	if(counter < maxcount){
	    i = nsamps;
	    sp = buffer;
	    counter += nsamps;
	    
	    while(i-- > 0)
		summer[0] += *sp++;
	    
	    if(counter > 0)
		aver[0] = summer[0]/counter;
	}
	i = nsamps;
	sp = buffer;
	while(i-- > 0)
	    *sp++ -= aver[0];
    }
    dc[0] = aver[0];
    dc[1] = aver[1];
}

void GetBuffMaxMin(buffer,samples,chans,smax,smin)
    register short *buffer;
    short samples, chans;
    double *smax, *smin;
{
    if(samples > 0) {
	register int max0, max1, min0, min1, t;
	max0 = -32767;
	max1 = -32767;
	min0 = 32767;
	min1 = 32767;
	
	if(chans > 1){			/* 0 ==> stereo */
	    do {
		if((t = *buffer++) > max0)
		    max0 = t;
		else
		    if(t < min0) min0 = t;
		--samples;
		if((t = *buffer++) > max1)
		    max1 = t;
		else
		    if(t < min1) min1 = t;
	    } while( --samples );
	    smax[0] = max0;
	    smin[0] = min0;
	    smax[1] = max1;
	    smin[1] = min1;
	} else {
	    do {
		if((t = *buffer++) > max0)
		    max0 = t;
		else
		    if(t < min0) min0 = t;
	    } while( --samples );
	    smax[0] = max0;
	    smin[0] = min0;
	}
    }
}

int closest_srate(freq, rates)
    double freq;
    int *rates;
{
    if(freq && rates && (*rates > 0)) {
	register int i, ifr = freq + 0.5, best, min;

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
	      freq,best);
      freq = best;
    }
    return(best);
  } else
    fprintf(stderr,"Bad args to closest_srate()\n");
  return(0);
}

void set_channel_gains(l_gain, r_gain)
     double l_gain, r_gain;
{
  gain_0 = l_gain;
  gain_1 = r_gain;
  return;
}

void ScaleData(indata, outdata, nsamps, channel, gflag1, shift, gain)
     register int nsamps, channel, shift;
     int gflag1;
     register float gain;
     short *indata, *outdata;
{
    register short *to, *from;
    register float gain1 = gain*gain_1,
                   gain0 = gain*gain_0;
    if(nsamps <= 0) return;
    if(!shift && !gflag1 && (channel != 2)) return;
    if( !outdata ){            /* input data is destroyed */
      to = indata;
      if(gflag1){               /* No automatic scaling */
	if(channel == 2){
	  do {
	    (*to++) *= gain0;           /* stereo */
	    (*to++) *= gain1;
	  } while(--nsamps);
	} 
	else{
	  do{
	    (*to++) *= gain;            /* mono */
	  } while(--nsamps);;
	}
      }
      else                      /* automatic scaling */
	if(channel == 2) {
	  gain0 = (1 << shift) * gain_0;
	  gain1 = (1 << shift) * gain_1;
	  do {
	    (*to++) *= gain0;
	    (*to++) *= gain1;
	  } while(--nsamps);
	}
	else	
	  do {
	    (*to++) <<= shift;
	  } while(--nsamps);
    }
    else{                     /* input data preserved */
      to = outdata;
      from = indata;
      if(gflag1){
	if(channel == 2){
	  do {
	    *to++ = (*from++) * gain0;
	    *to++ = (*from++) * gain1;
	  } while(--nsamps);
	}
	else{
	  do{
	    (*to++) = (*from++) * gain;
	  } while(--nsamps);
	}
      }
      else
	if(channel == 2) {
	  gain0 = (1 << shift) * gain_0;
	  gain1 = (1 << shift) * gain_1;
	  do {
	    *to++ = (*from++) * gain0;
	    *to++ = (*from++) * gain1;
	  } while(--nsamps);
	}
	else		
	  do {
	    *to++ = (*from++) << shift;
	} while(--nsamps);
  }
}

void
E_usleep (usec)
     unsigned int usec;
{
   struct timeval timer_val;
 
   timer_val.tv_sec = 0;
   timer_val.tv_usec = usec;
   select (0, NULL, NULL, NULL, &timer_val);
}
