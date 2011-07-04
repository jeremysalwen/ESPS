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

static char *sccs_id = "@(#)fab2_stubs.c	1.1 1/15/93 ATT/ERL";

/* fab2_stubs.c */

/* Stubs and covering calls to make the old FAB2 DAC compatible with waves */

#include <stdio.h>
#include <signal.h>
#include <esps/esps.h>

extern int da_done;
extern int fab2_sent, fab2_error_at, fab2_maxsamps, da_location, debug_level;

int dsp32_wait = 5;	/* how long to try for board access */

void (*sig_save[3])();

fab2_play_from_file(istr, length, srate, chan, amp, ehead)
     FILE *istr;
     double *srate;
     int chan,	 /* ignored */
       amp;	/* Max sample value in the file or region. */
     void *ehead;
{
  double frequency = *srate * 1000.0;
  extern int use_dsp32;

  if(chan < 0) {
    fprintf(stderr, "Dual-channel output is not provided by the FAB2 board.\n");
    return(0);
  }
  use_dsp32 = 1;
  if(dacmaster_32(-1, NULL, length, &frequency, amp, istr, ehead)) {
    *srate = frequency/1000.0;

    while(!da_done && (fab2_error_at < 0)) {
      usleep(100000);		/* sleep 100 ms */
    }
  }
  fab2_completion();
  if(fab2_error_at >= 0)
    return(fab2_error_at);
  else
    return(fab2_sent);
}

#ifdef OLD_WAITER
  while( atime ) {
      pause();
    if ( debug_level )
      fprintf(stderr, " fab2_maxsamps %d icnt %d qcnt %d\n", fab2_maxsamps, icnt, qcnt);
    }
  dsprg(0,C_STOP);		/* halt dsp#0 */
  dsprg(0,C_CLRSIG);		/* clear pending signals */
  signal( SIGALRM, sigsav[0]);	/* restore signal handlers */
  signal( SIGINT, sigsav[1]);
  signal( SIGQUIT, sigsav[2]);
  *amax = smax;
  *amin = smin;
  if (locked)
  {
      DSP_UNLOCK;
      locked = 0;
  }
  return(size-icnt);		/* returns number of samples written */
#endif

/*---------------------------------------------------------------------*/
void stop_fab2_da_waves(sig)
     int sig;
{
  int number_left;

  number_left = fab2_maxsamps;

  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); number_left:%d\n",
	    sig, number_left);
  fab2_completion();
  if(number_left > 0) {
    char str[200];
    sprintf(str,"set da_location %d\n", da_location);
    send_xwaves2(NULL, NULL, str, debug_level);
    /* This pause seems to be necessary to prevent the possibility that
       the child's death will be registered before the send_xwaves is! */
    usleep(200000);
  }
  exit(0);
}

/*************************************************************************/
void interrupt_fab2_da(sig)
     int sig;
{
  fab2_error_at = fab2_sent;
  fab2_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); da_location:%d\n",
	    sig, da_location);
  exit(0);
}

set_fab2_dac_callbacks(interval_us)
     long interval_us;
{
  /* Setup the signal handlers. */
  sig_save[0] = signal( SIGINT, interrupt_fab2_da);
  sig_save[1] = signal( SIGQUIT, interrupt_fab2_da);
  sig_save[2] = signal( SIGUSR1, stop_fab2_da_waves);
}

clear_fab2_dac_callbacks()
{
  /* Restore the original signal handlers. */
  signal(SIGINT,sig_save[0]);
  signal(SIGQUIT,sig_save[1]);
  signal(SIGUSR1,sig_save[2]);
}
