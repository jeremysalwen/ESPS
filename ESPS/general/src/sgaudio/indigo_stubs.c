/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)indigo_stubs.c	1.1	4/4/96	ERL";

/* indigo_stubs.c */

/* Stubs and covering calls to make the indigo DAC compatible with
 * surf, etc. */

#include <stdio.h>
#include <signal.h>

int da_done = 1;
int channel = 1;
extern int indigo_sent, indigo_error_at, indigo_maxsamps, da_location, debug_level;
void (*sig_save[3])();

play_from_file(istr, length, srate, chan, amp, ehead)
     FILE *istr;
     double *srate;
     int chan,	 /* -1==>stereo; else chan == (channel number to play) */
       amp;	/* Max sample value in the file or region. */
     void *ehead;
{
  double frequency = *srate * 1000.0;

  channel = (chan < 0)? 0 : channel+1;
  dacmaster_indigo(-1, NULL, length, &frequency, amp, istr, ehead);
  *srate = frequency/1000.0;

  while(!da_done && (indigo_error_at < 0)) {
    
    if(!indigo_handle(1))	/* returns 0 if couldn't write a bufferfull */
      sginap(10);		/* sleep 100 ms */
  }

  indigo_completion();
  if(indigo_error_at >= 0)
    return(indigo_error_at);
  else
    return(indigo_sent);
}


/*---------------------------------------------------------------------*/
void stop_indigo_da_waves(sig)
     int sig;
{
  int number_left;
  extern char *display_name, *waves_name;

  da_location -= still_in_que();

  number_left = indigo_maxsamps - indigo_sent;

  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); number_left:%d\n",
	    sig, number_left);
  if(number_left > 0) {
    char str[200];
    sprintf(str,"set da_location %d\n", da_location);
    SendXwavesNoReply(display_name, waves_name, NULL, str);
    /* This pause seems to be necessary to prevent the possibility that
       the child's death will be registered before the send_xwaves is! */
    sginap(20);
  }
  exit(0);
}

/*************************************************************************/
void interrupt_indigo_da(sig)
     int sig;
{
  indigo_error_at = indigo_sent;
  indigo_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); da_location:%d\n",
	    sig, da_location);
  exit(0);
}

set_dac_callbacks(interval_us)
     long interval_us;
{

  /* Setup the signal handlers. */
  sig_save[1] = signal( SIGINT, interrupt_indigo_da);
  sig_save[2] = signal( SIGQUIT, interrupt_indigo_da);
  sig_save[3] = signal( SIGUSR1, stop_indigo_da_waves);
}

clear_dac_callbacks()
{
  /* Restore the original signal handlers. */
#ifdef FOR_STANDALONE
  signal(SIGINT,sig_save[1]);
  signal(SIGQUIT,sig_save[2]);
  signal(SIGUSR1,sig_save[3]);
#endif
}
