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

static char *sccs_id = "@(#)surf_stubs.c	1.3	1/16/93	ERL";

/* surf_stubs.c */

/* Stubs and covering calls to make the  SURF DAC compatible with waves */

#include <stdio.h>
#include <signal.h>
#include <esps/esps.h>

int ARIEL_16 =0;
int ARIEL_HK =0;	/* these are defined in globals.c for waves */
int P8574_type =1;

int da_done = 1;
extern int channel;
extern int surf_sent, surf_error_at, surf_maxsamps, da_location, debug_level;
void (*sig_save[3])();

play_from_file(istr, length, srate, chan, amp, ehead)
     FILE *istr;
     double *srate;
     int chan,	 /* -1==>stereo; else chan == (channel number to play) */
       amp;	/* Max sample value in the file or region. */
     void *ehead;
{
  static its_surf = -1;

  if((its_surf > 0) || dsp32c_is_available()) {
    double frequency = *srate * 1000.0;

    channel = (chan < 0)? 0 : chan + 1;
    if(dacmaster_32C(-1, NULL, length, &frequency, amp, istr, ehead)) {
      *srate = frequency/1000.0;

      while(!da_done && (surf_error_at < 0)) {
      
	if(!surf_handle(1))	/* returns 0 if couldn't write a bufferfull */
	  usleep(10000);	/* sleep 10 ms */
      }
    }
    its_surf = 1;
    surf_completion();
    if(surf_error_at >= 0)
      return(surf_error_at);
    else
      return(surf_sent);
  } else
    if((its_surf == 0) || dsp32_is_available()) {
      its_surf = 0;
      return(fab2_play_from_file(istr, length, srate, chan, amp, ehead));
    } else {
      fprintf(stderr,
              "It seems you do not have the relevant hardware\n(FAB2 or SURF board) installed.\n");
      return(0);
    }
}


/*---------------------------------------------------------------------*/
void stop_surf_da_waves(sig)
     int sig;
{
  int number_left;

  number_left = surf_maxsamps - surf_sent;

  surf_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); number_left:%d\n",
	    sig, number_left);
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
void interrupt_surf_da(sig)
     int sig;
{
  surf_error_at = surf_sent;
  surf_completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal (%d); da_location:%d\n",
	    sig, da_location);
  exit(0);
}

set_dac_callbacks(interval_us)
     long interval_us;
{

  /* Setup the signal handlers. */
  sig_save[0] = signal( SIGINT, interrupt_surf_da);
  sig_save[1] = signal( SIGQUIT, interrupt_surf_da);
  sig_save[2] = signal( SIGUSR1, stop_surf_da_waves);
}

clear_dac_callbacks()
{
  /* Restore the original signal handlers. */
#ifdef FOR_STANDALONE
  signal(SIGINT,sig_save[0]);
  signal(SIGQUIT,sig_save[1]);
  signal(SIGUSR1,sig_save[2]);
#endif
}

char	*ProgName = "surf_play";
char	*Version = "3.0";
char	*Date = "1/12/93";

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}
