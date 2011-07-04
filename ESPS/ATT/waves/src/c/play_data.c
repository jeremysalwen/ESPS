/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
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
 * will eventually contain all "standard" methods for signal reconstruction
 * and synthesis
 */

static char *sccs_id = "@(#)play_data.c	1.21 5/27/97 ERL/ATT";

#include <stdio.h>
#include <Objects.h>
#include <esps/esps.h>
#include <esps/sd.h>		/* SD file stuff */
#include <esps/fea.h>
#include <esps/feasd.h>

#define	DPR_MAX		1024
#define DB_SIZE		4096	/* integral multiple of DPR_MAX */
#define	DA_START	0
#define	DA_RUN		1
#define	DA_END		2
#define ALWAYS_LOAD     1
#define LOAD_AS_NEEDED  2
#ifndef LBIN
#define LBIN "/usr/local/bin"
#endif

int             da_done = TRUE, da_location = 0, play_pid = 0;

static char     loaded[100];
short           dsp_is_open = -1;

extern short   *dspmap();
extern int      use_dsp32;
extern int      dsp_type;
extern char     play_program[];
extern int      debug_level;
extern char    *registry_name;

short          *dpr = NULL;
static int      byt_swap;

char           *mktemp();
int             call_external_play_prog_get_pid();


/*************************************************************************/
ext_play_done()
{
   da_done = TRUE;
   if ((play_pid > 0) && (da_location > 0))
      move_view_to_loc(get_esps_callback_data(play_pid), da_location);
   da_location = 0;
   play_pid = 0;
}

/*************************************************************************/
/*
 * If D/A is in progress and play_pid is > 0, kill the current play process
 * and disable it's callback.  Indicate that D/A is in progress by setting
 * da_done FALSE.  If xwaves is not in server mode, put it into server mode.
 * Export the environment variables WAVES_PORT and WAVES_HOST so that clever
 * play programs can set da_location via send_xwaves.  The routine
 * handle_da_interruption() can kill the external play program.  When this
 * child process dies, its callback procedure will then reposition the
 * display window appropriately if da_location has been set.
 */
call_external_play_prog(command, input, output, n_outputs, do_display, callback)
   char           *command, *input, *output;
   int             n_outputs, (*callback) (), do_display;
{

   if (play_program && *play_program &&
       (play_pid > 0)) {	/* a play program seems to be running */
      if (debug_level)
	 fprintf(stderr, "Trying to kill %s (%d)\n", play_program, play_pid);
      reset_esps_callback(play_pid, NULL);
      kill(play_pid + 1, SIGINT);	/* +1 since it's execv'd from the
					 * fork() */
   }
   da_done = TRUE;
   da_location = 0;
   if ((play_pid = run_esps_prog_get_pid(command, input, output,
				     n_outputs, do_display, ext_play_done)))
      da_done = FALSE;
   return ((play_pid > 0) ? 0 : -1);
}

/*************************************************************************/
get_estimated_signal_maximum(s)
   Signal         *s;
{
   double          amax = 0.0, amin = 0.0, get_genhd_val_array();
   extern int      sig_max_override;

   if (sig_max_override > 0)
      return (sig_max_override);

   if (s->header && (s->header->magic == SIGNAL_MAGIC)) {	/* get from header */
      head_scanf(s->header, "maximum", &amax);
      head_scanf(s->header, "minimum", &amin);
      if (amax < (-amin))
	 amax = -amin;
   } else if (s->header->magic == ESPS_MAGIC) {
      if (s->dim == 1)
         amax = get_genhd_val("max_value", s->header->esps_hdr, 32767.0);
      else
	 amax = MAX(get_genhd_val_array("max_value", 0, 
					 s->header->esps_hdr, 32767.0),
	            get_genhd_val_array("max_value", 1, 
					 s->header->esps_hdr, 32767.0));
	
   }
   return ((int) ((amax > 128) ? amax : 32767));	/* don't take a chance
							 * on small values! */
}

/*************************************************************************/
static Signal  *this_play = NULL;

Signal *get_playing_signal()
{
  return(this_play);
}

/*
 * This is the routine called by all requests for INTERNAL plays by xwaves.
 * See call_external_play_prog() to explicitly request play by external
 * programs.  Note that some internal requests are sent to external play
 * programs when it seems implssible to execute them internally.
 */
play_file(s, start, end)
   Signal         *s;
   double          start, end;
{
   static int      fd;
   int             sam1, nsamps, startsamp;
   char            play_command[256];	/* for ESPS play command */
   double          end_time, freq;
   Header         *h = NULL;
   struct header  *ehead = NULL;

   if (debug_level)
      (void) fprintf(stderr, "xwaves: entered play_file\n");
   if (s) {
      if ((start < SIG_END_TIME(s)) && (end > s->start_time)) {
	 this_play = s;
	 h = s->header;
	 byt_swap = h && (h->magic == RHEADR_MAGIC);

	 /* Limit play request to signal limits. */
	 if ((sam1 = (start - s->start_time) * s->freq) < 0) {
	    sam1 = 0;
	    start = s->start_time;
	 }
	 if (end > (end_time = s->start_time + ((double) s->file_size) / s->freq))
	    end = end_time;

	 /*
	  * If there is built-in hardware and the signal can be read and
	  * converted for its use...
	  */
	 if (use_dsp32 && ereader_can_handle(s, &ehead)) {
	    if (((!s->name) ||
		 ((s->file < 0) && (s->file != SIG_CLOSED))) &&
		(s->buff_size != s->file_size)) {
	       (void) fprintf(stderr, "xwaves(play_file): can't play data\n");
	       return (FALSE);
	    }
	    if (!da_done)
	       stop_da(NULL);

	    /*
	     * If it's all in memory and it's 1-dimensional, just play from
	     * the display buffer.
	     */
	    if (((type_of_signal(s) & VECTOR_SIGNALS) == P_SHORTS) &&
		(s->dim == 1) && (start >= BUF_START_TIME(s)) &&
		(end <= BUF_END_TIME(s))) {
	       return (play_signal(s, start, end));
	    }
	    if (s->file == SIG_NEW)
	       put_signal(s);

	    /* It's playable by the built-in hardware. */
	    return (dac_file(s, ehead, start, end));

	 } else if (play_program && *play_program) {
/* PUNT: TRY TO USE AN EXTERNAL "PLAY" * PROGRAM */

	    if ((nsamps = (end - start) * s->freq) <= 0)
	       return (FALSE);

	    if ((s->file == SIG_NEW) && (s->dim < 3))
	       put_signal(s);

	    startsamp = sam1 + 1;	/* ESPS numbering starts at 1, not 0 */

	    (void) sprintf(play_command, "%s -r%d:+%d",
			   play_program, startsamp, nsamps - 1);
	    (void) call_external_play_prog(play_command, s->name, "", 0, NO, NULL);

	    return (TRUE);
	 } else {
            if (!use_dsp32 && !play_program || ! *play_program)
	      sprintf(notice_msg, "Cannot play %s. No play program is specfied (play_prog is not set)", s->name);
            else
	      sprintf(notice_msg, "Cannot play %s due to its type", s->name);
            show_notice(1, notice_msg);
         }
      } else {
	 sprintf(notice_msg, "Play region requested not in file %s", s->name);
	 show_notice(1, notice_msg);
      }
   } else			/* else (NULL signal argument to play_file() */
      show_notice(1, "No playable signal passed to play_file");
   return (FALSE);
}

/*************************************************************************/
show_play_position(sample)
   int             sample;
{
   Signal         *s;
   View           *v;
   extern double play_time;
   extern int show_play_pos;

#ifdef LINUX
   return;
#endif

   if ((s = this_play) &&
       (v = s->views) && canvas_still_lives(v->canvas)) {
     if(show_play_pos) {
       plot_time_bar(v, s, sample);

       if(show_play_pos > 1) {
	 Signal *so = s;
	 long psample;

	 s = ((Object*)(s->obj))->signals;
	 while(s) {
	   if((s != this_play) && (v = s->views) &&
	      canvas_still_lives(v->canvas) &&
	      (s->freq > 0.0) && (so->freq > 0.0)) {
	     psample = (s->freq * sample)/so->freq;
	     plot_time_bar(v, s, psample);
	   }
	   s = s->others;
	 }
       }
     } else  /* set play_time here (instead of in plot_time_bar() */
       play_time = s->start_time + (((double)sample)/s->freq);
   }
}

/*************************************************************************/
void stop_da_view(v)
     View *v;
{
  if(!da_done && v && v->sig && (v->sig == this_play))
    stop_da(NULL);
}

/*************************************************************************/
play_signal(s, start, end)
   Signal         *s;
   double          start, end;
{
   int             i;
   double          stime, etime, freq;
   short          *data;
   char            play_command[256];	/* for ESPS play command options */
   struct header  *espshd = NULL;	/* SD header for ESPS file */
   int             nsamps, startsamp;

   if (debug_level)
      (void) fprintf(stderr, "xwaves: entered play_signal\n");
   if (s && IS_GENERIC(type_of_signal(s)) &&
       ((type_of_signal(s) & VECTOR_SIGNALS) == P_SHORTS) &&
       (s->dim == 1)) {

      if ((stime = BUF_START_TIME(s)) > start)
	 start = stime;
      if ((etime = BUF_END_TIME(s)) < end)
	 end = etime;

      if ((end > start) && use_dsp32) {
	 /* D/A conversion from a memory buffer. */
	 dac_mem(s, start, end);
	 return (TRUE);
      }
   }
   return (FALSE);
}

/*************************************************************************/
dac_mem(s, start, end)
   Signal         *s;
   double          start, end;
{
   if (s && (start < end) && s->data) {

      short          *data = ((short **) (s->data))[0];
      int             imax = get_estimated_signal_maximum(s), i;
      double          freq = s->freq;


      if (!data)
	 return;

      /* Board is reputed to be present, so use it. */
      for (i = 0; i < 123456; i++)
	 if (da_done)
	    break;

      /* provide for detection of location of potential interrupts */
      da_location = (start - s->start_time) * s->freq;

#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
      dacmaster_indigo(-1, (short *) ((int) ((start - BUF_START_TIME(s)) * s->freq) + data), (int) ((end - start) * s->freq), &freq, imax, NULL, NULL);
#else
      if (dsp_type == DSP32_FAB2)
	 dacmaster_32(-1, (short *) ((int) ((start - BUF_START_TIME(s)) * s->freq) + data),
		  (int) ((end - start) * s->freq), &freq, imax, NULL, NULL);
      else
	 dacmaster_32C(-1, (short *) ((int) ((start - BUF_START_TIME(s)) * s->freq) + data),
		  (int) ((end - start) * s->freq), &freq, imax, NULL, NULL);
#endif
   }
}

/*************************************************************************/
/* D/A from disk */
/* for files not all in memory, but playable via built-in hardware */
dac_file(s, ehead_to_use, start, end)
   Signal         *s;
   struct header  *ehead_to_use;
   double          start, end;
{
   if (s && (start < end)) {
      int             fd = -1, sam_to_skip = s->freq * (start - s->start_time);
      int             imax = 32767, nsamps;
      double          freq;
      struct header  *head = NULL;
      Header         *h = s->header;
      extern int      channel;
      char           *get_sphere_hdr();


      if (debug_level)
	 fprintf(stderr,
		 "dac_file: s: %x, ehead_to_use: %x, start: %f, end: %f\n",
		 s, ehead_to_use, start, end);

      if (is_feasd_sphere(s))
         close_sig_file(s);

      if (s->file == SIG_CLOSED) {
	 if ((fd = open(s->name, 0)) < 0) {
	    sprintf(notice_msg, "Cannot open %s for audio output", s->name);
	    show_notice(1, notice_msg);
	    return (FALSE);
	 } else {
	    s->file = fd;
	    if (h->magic == ESPS_MAGIC) {
	       char           *sp;
	       h->strm = fdopen(fd, "r");
	    }
	 }
      }
      if (h->magic == ESPS_MAGIC)
	 head = h->esps_hdr;
      if (s->file < 0)
	 return (FALSE);
      fd = s->file;		/* in case it was already open */

      /* One way or another, seek to the start of playable segment. */
      if (h->strm) {
	 if (debug_level > 2)
	    fprintf(stderr, "stream is open; skipping %d\n", s->bytes_skip);
         if (!is_feasd_sphere(s))
	    fseek(h->strm, s->bytes_skip, 0);
	 if (head) {
	    if (debug_level > 2)
	       fprintf(stderr, "using skiprec %d\n", sam_to_skip);
	    fea_skiprec(h->strm, (long) sam_to_skip, head);
	 } else {
	    if (debug_level > 2)
	       fprintf(stderr, "using fseek %d\n", (int) (sam_to_skip * sizeof(short) * s->dim));
	    fseek(h->strm, sam_to_skip * sizeof(short) * s->dim, 1);
	 }
      } else {
	 if (debug_level > 2)
	    fprintf(stderr, "No stream; skipping %d\n", s->bytes_skip + (int) (sam_to_skip * sizeof(short) * s->dim));
	 lseek(fd, s->bytes_skip + (sam_to_skip * sizeof(short) * s->dim), 0);
      }

      /* provide for detection of location of potential interrupts */
      da_location = sam_to_skip;
      imax = get_estimated_signal_maximum(s);	/* so data can be scaled for
						 * output */
      freq = s->freq;		/* actual frequency used is returned in freq */
      nsamps = s->freq * (end - start);
      if (debug_level > 2)
	 fprintf(stderr, "channel %d nsamps %d\n", channel, nsamps);
      /* D/A conversion directly from a disc file */
#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
      dacmaster_indigo(fd, (short *) 0, nsamps, &freq, imax, h->strm, ehead_to_use);
#else
      if (dsp_type == DSP32_FAB2)
	 dacmaster_32(fd, (short *) 0, nsamps, &freq, imax, h->strm, ehead_to_use);
      else
	 dacmaster_32C(fd, (short *) 0, nsamps, &freq, imax, h->strm, ehead_to_use);
#endif
   }
}

/*************************************************************************/
/*
 * Stop D/A operation.  If running under xwaves, this could mean stop an
 * external play program.  External programs are killed with the
 * distinguished SIGUSR1 so that play-programs-in-the-know can catch the
 * signal intelligently and use a send_xwaves mechanism to set da_location
 * appropriately.
 */
int
stop_da(client_data)
   caddr_t         client_data;
{
   if (debug_level)
      fprintf(stderr, "stop_da: da_done=%d play_pid=%d play_program=%s\n",
	      da_done, play_pid, play_program);
   /* external play may be in progress */
   if (play_program && *play_program && !da_done && (play_pid > 0)) {
      set_esps_callback_data(play_pid, client_data);
      kill(play_pid + 1, SIGUSR1);	/* +1 since it's execv'd from a fork */
      da_done = TRUE;		/* force it here, just in case no SIGCHILD */
      return (FALSE);		/* i.e. it's not really stopped yet! */
   } else
#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
      stop_indigo_da();
#else
#ifdef DSP_BOARD_VERSION
      switch (dsp_type) {
      case DSP32C_VME:
	 stop_32c_vme_da();
	 break;
      case DSP32_FAB2:
	 stop_fab2_da();
	 break;
      default:
	 if (debug_level)
	    fprintf(stderr, "Board type %d; (external process?)\n", dsp_type);
	 break;
      }
#endif
#endif
   return (TRUE);
}

/*************************************************************************/
int
playable_dimension(d)
   int             d;
{
   if (use_dsp32
#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
      ) {
#else
       && (dsp_type == DSP32C_VME)) {
#endif
      if ((d == 1) || (d == 2)) {
	 extern int      channel;
	 channel = (d == 1) ? 3 : 0;
	 return (TRUE);
      }
   }

   return (d == 1);
}

/*************************************************************************/
/*
 * This highly-specialized type check determines if the general-purpose
 * reading function read_any_file() can handle the signal in question.  It
 * returns TRUE or FALSE.  It also sets its required argument, ehead, to a
 * valid ESPS header, if get_sd_recs can be used, or NULL, if it can't, but
 * the file otherwise can be handled by read_any_file().
 */
ereader_can_handle(s, ehead)
   Signal         *s;
   struct header **ehead;
{
   struct header  *sdtofea();

   if (s && playable_dimension(s->dim)) {
      Header         *h = s->header;

      if (h->magic == ESPS_MAGIC) {
	 if (h->esps_hdr->common.type == FT_SD) {
	    *ehead = sdtofea(h->esps_hdr);
            return (TRUE);
         }
	 else if (h->esps_hdr->common.type == FT_FEA &&
                  h->esps_hdr->hd.fea->fea_type == FEA_SD) {
	    *ehead = h->esps_hdr;
            return (TRUE);
         }
         else if ((type_of_signal(s) & VECTOR_SIGNALS) == P_SHORTS) 
	    return (TRUE);
         else
            return (FALSE);

      } else {			/* might be a SIGnal file... */
	 if ((type_of_signal(s) & VECTOR_SIGNALS) == P_SHORTS) {
	    return (TRUE);
	 }
      }
   }
   return (FALSE);
}


#define ITIMER_NULL ((struct itimerval*)0)
/*************************************************************************/
static
xv_dac_handle(who, what)
   Panel_item      who;
   int             what;
{

   if (what == ITIMER_REAL) {
#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
      indigo_handle(1);
#else
      surf_handle(who);
#endif
   } else
      fprintf(stderr, "Unexpected interrupt in xv_dac_handle(%d)\n", what);

   return (NOTIFY_DONE);
}

/*************************************************************************/
clear_dac_callbacks()
{
   extern Frame    daddy;
   notify_set_itimer_func(daddy, xv_dac_handle, ITIMER_REAL,
			  ITIMER_NULL, ITIMER_NULL);

}

/*************************************************************************/
set_fab2_dac_callbacks()
{
   if (debug_level)
      fprintf(stderr, "set_fab2_dac_callbacks()\n");
}

/*************************************************************************/
clear_fab2_dac_callbacks()
{
   if (debug_level)
      fprintf(stderr, "clear_fab2_dac_callbacks()\n");
}

/*************************************************************************/
void
set_dac_callbacks(timer_usec)
   int             timer_usec;
{
   int             ret;
   static int      my_client_object;
   extern Frame    daddy;
   struct itimerval dac_timer;

   dac_timer.it_interval.tv_usec = timer_usec;
   dac_timer.it_interval.tv_sec = 0;
   dac_timer.it_value.tv_usec = timer_usec;
   dac_timer.it_value.tv_sec = 0;

   (void) notify_set_itimer_func(daddy, xv_dac_handle, ITIMER_REAL,
				 &dac_timer, ITIMER_NULL);


   return;
}

#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
/*************************************************************************/
stop_indigo_da()
{
   extern int      indigo_sent, da_location;
   extern int      indigo_error_at;

   indigo_error_at = indigo_sent;
   indigo_completion();
   if (debug_level)
      fprintf(stderr, "stopping indigo D/A; da_location:%d\n",
	      da_location);
}
#endif
