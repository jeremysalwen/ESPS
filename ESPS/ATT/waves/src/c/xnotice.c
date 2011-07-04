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
 * Written by:   Alan Parker Checked by: Revised by:
 * 
 * Brief description:
 * 
 */

static char    *sccs_id = "@(#)xnotice.c	1.5	3/19/96	ERL";

#include <xview/xview.h>
#include <xview/notify.h>
#include <xview/notice.h>
#include <sys/time.h>

extern Frame    daddy;

char            notice_msg[256];

#define ITIMER_NULL ((struct itimerval *)0)

/* ********************************************************************** */
/* These routines are used to support warning and error messages in a     */
/* notice box.								  */

static Xv_notice notice_obj = NULL;

Notify_value
kill_notice(client, which)
   Notify_client   client;
   int             which;
{
   struct itimerval timer;

   timer.it_interval.tv_usec = 0;
   timer.it_interval.tv_sec = 0;
   timer.it_value.tv_usec = 0;
   timer.it_value.tv_sec = 0;

   notify_set_itimer_func(client, NULL,
			  ITIMER_REAL, &timer, ITIMER_NULL);

   if (daddy)
      xv_set(daddy, FRAME_BUSY, FALSE, NULL);
   if (notice_obj)
      xv_destroy_safe(notice_obj);

   notice_obj = NULL;
   return NOTIFY_DONE;
}

notice_proc(notice, value, event)
   Xv_Notice       notice;
   int             value;
   Event          *event;
{
   return kill_notice(kill_notice, 1);
}


/*
 * Display a notice.   A timer is started so that the message is taken down
 * if the user doesn't dismiss it within 10 seconds.  This is to prevent
 * blocking of xwaves when being driven by external commands
 */

void
show_notice(type, message)
   int             type;
   char           *message;
{
   struct itimerval timer;
   char           *type_msg;
   extern int      show_error_gui;

   if (message) {

      if (type)
	 type_msg = "Error:";
      else
	 type_msg = "Warning:";

      if (show_error_gui) {
	 timer.it_interval.tv_usec = 0;
	 timer.it_interval.tv_sec = 0;
	 timer.it_value.tv_usec = 0;
	 timer.it_value.tv_sec = 10;


	 if (notice_obj != NULL)
	    kill_notice(kill_notice, 1);

	 if (daddy) {
	    notice_obj = xv_create(daddy, NOTICE,
				   NOTICE_LOCK_SCREEN, FALSE,
				   NOTICE_BLOCK_THREAD, FALSE,
		       NOTICE_MESSAGE_STRINGS, type_msg, " ", message, NULL,
				   NOTICE_BUTTON, "Dismiss", 1,
				   XV_SHOW, TRUE,
				   NOTICE_NO_BEEPING, TRUE,
				   NOTICE_EVENT_PROC, notice_proc,
				   NULL);
	 }
	 if (notice_obj)
	    notify_set_itimer_func(kill_notice, kill_notice,
				   ITIMER_REAL, &timer, ITIMER_NULL);
	 else
	    fprintf(stderr, "Couldn't create notice panel:\n%s\n", message);
      } else
	 fprintf(stderr, "%s %s\n", type_msg, message);
   }
}
