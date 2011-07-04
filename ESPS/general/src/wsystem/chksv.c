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
 * Written by:  Ken Nelson
 * Checked by:
 * Revised by:
 *
 * Brief description: checks for sunview
 *
 */

static char *sccs_id = "@(#)chksv.c	1.4	9/3/93	ERL";

#include <stdio.h>

#if defined(sun) && !defined(OS5)

#include <suntool/sunview.h>

#endif

int checkForSunview()
{
#if defined(sun) && !defined(OS5)

  Frame frame;
  FILE *oldstderr;
  int result = 0;

  oldstderr = freopen("/dev/null","a",stderr);
  frame = window_create(NULL,FRAME,FRAME_NO_CONFIRM,TRUE,NULL);
  if (frame != NULL)
  {
    window_destroy(frame);
    result =  1;
  }
  else
  {
    result = 0;
  }

 fclose(stderr);
 return result;
 
#else
  return 0;  /* Sunview not even possible */
#endif
}


