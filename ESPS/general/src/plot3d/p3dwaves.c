/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: functions to link cursors with xwaves
 *
 */

static char *sccs_id = "@(#)p3dwaves.c	1.5 1/24/97 ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <esps/xwaves_ipc.h>
#include "plot3d.h"



int		    send_to_waves=0;
int		    send_time, send_freq;
extern Menu_item    waves_on, waves_off;
extern int	    debug_level;
Sxptr		    *sxptr;
char 		    buffer[80];
extern Panel	    panel;
extern int	    enable_waves_mode;

void
send_waves_cursor(x,y)
double x,y;
{
        (void)strcpy(buffer,". cursor ");
	if (send_time)
	  (void)sprintf(buffer+strlen(buffer)," time %f",x); 
	if (send_freq)
	  (void)sprintf(buffer+strlen(buffer)," freq %f",y);
	(void)strcat(buffer,"\n");
        SendXwavesNoReply(NULL, NULL, sxptr, buffer);
}

Xv_opaque
enable_waves_cursors(menu, item)
   Menu	       menu;
   Menu_item   item;
{
   extern void make_x_rtime();
   extern void make_y_freq();
   extern int  time_ok_for_x(),
	       freq_ok_for_y();
   extern int  xv_not_running;
   extern char *waves_name;
   char        *ptr;

   if (debug_level)
	fprintf(stderr, "enable_waves_cursors\n");

   if (ptr = getenv("WAVES_NAME"))
	waves_name = ptr;

   if((sxptr = OpenXwaves(NULL, waves_name, "plot3d")) == NULL) { 
	  char s[80];
	  sprintf(s,"at registry name %s.\n",waves_name);
	  if(xv_not_running) {
	   fprintf(stderr,"Error: cannot connect to an xwaves server ");
	   fprintf(stderr,s);
	   fprintf(stderr,"Plot3d will continue without sending ");
	   fprintf(stderr,"commands to xwaves.\n");
 	  }
	  else
   	  (void)xv_create(panel, NOTICE,
	  NOTICE_MESSAGE_STRINGS, "Error: cannot connect to an xwaves server",
				  s,
				  "Plot3d will continue without sending",
				  "commands to xwaves.",
				NULL,
	  NOTICE_BUTTON_YES,	"Continue", 
          XV_SHOW, 		TRUE, NULL);
	return XV_NULL;
   }
	
   if(freq_ok_for_y()) {
   	make_y_freq();
	send_freq = 1;
   }
   send_time = 0;
   switch (time_ok_for_x()) {
    case XTIME_FROM_BOTH:
    case XTIME_FROM_REC:
      make_x_rtime();
      send_time = 1;
      break;
    case XTIME_FROM_TAG:
      make_x_ttime();
      send_time = 1;
      break;
   }
   if (!xv_not_running) {
   xv_set(waves_on,
			MENU_INACTIVE,		    1,
			0);
   xv_set(waves_off,
			MENU_INACTIVE,		    0,
			0);
   }
  send_to_waves = 1;
  return XV_NULL;
}

Xv_opaque
disable_waves_cursors(menu, item)
   Menu	       menu;
   Menu_item   item;
{
   if (debug_level)
	fprintf(stderr, "disable_waves_cursors\n");
   send_to_waves = 0;
   enable_waves_mode = 0;
   CloseXwaves(sxptr);
   xv_set(waves_off,
			MENU_INACTIVE,		    1,
			0);
   xv_set(waves_on,
			MENU_INACTIVE,		    0,
			0);
  return XV_NULL;
}
