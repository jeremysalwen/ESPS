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
 * Written by:  David Talkin Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)dbri_stubs.c	1.3 6/11/96 ERL";


/* Stubs and covering calls to make the Sparc10/LX DAC compatible with
 * surf, etc. */

#include <stdio.h>
#include <signal.h>

int da_done = 1;
int channel = 1;
extern int sparc_sent, sparc_error_at, sparc_maxsamps, da_location, debug_level;
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
  dacmaster_sparc(-1, NULL, length, &frequency, amp, istr, ehead);
  *srate = frequency/1000.0;

  while(!da_done && (sparc_error_at < 0)) {
    
    if(!sparc_handle(1,-1))	/* returns 0 if couldn't write a bufferfull */
      usleep(10000);		/* sleep 10 ms */
  }
  
  sparc_completion();
  if(sparc_error_at >= 0)
    return(sparc_error_at);
  else
    return(sparc_sent);
}


/*---------------------------------------------------------------------*/
void stop_sparc_da_waves(sig)
     int sig;
{
  int number_left;
  extern char *display_name, *waves_name;

  da_location -= sparc_samps_sent();

  number_left = sparc_maxsamps - sparc_sent;

  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); number_left:%d\n",
	    sig, number_left);
  if(number_left > 0) {
    char str[200];
    sprintf(str,"set da_location %d\n", da_location);
    SendXwavesNoReply(display_name, waves_name, NULL, str);
    /* This pause seems to be necessary to prevent the possibility that
       the child's death will be registered before the send_xwaves is! */
    usleep(20000);
  }
  exit(0);
}

/*************************************************************************/
void interrupt_sparc_da(sig)
     int sig;
{
  sparc_error_at = sparc_sent;
  sparc_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); da_location:%d\n",
	    sig, da_location);
  exit(0);
}

set_dac_callbacks(interval_us)
     long interval_us;
{

  if (debug_level) fprintf(stderr,"setting signals\n");
  /* Setup the signal handlers. */
  sig_save[1] = signal( SIGINT, interrupt_sparc_da);
  sig_save[2] = signal( SIGQUIT, interrupt_sparc_da);
  sig_save[3] = signal( SIGUSR1, stop_sparc_da_waves);
}

clear_dac_callbacks()
{
  /* Restore the original signal handlers. */
#ifdef FOR_STANDALONE
  if (debug_level) fprintf(stderr,"clearing signals\n");
  signal(SIGINT,sig_save[1]);
  signal(SIGQUIT,sig_save[2]);
  signal(SIGUSR1,sig_save[3]);
#endif
}
