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
 * Brief description: Tells what Window system you are using
 *
 */

static char *sccs_id = "@(#)chkx.c	1.2	9/9/91	ERL";

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

int checkForX(release)
  char *release;
{
  Display  *display;
  int result = 0;

  freopen("/dev/null","a",stderr);
  if ( (display=XOpenDisplay(NULL)) != NULL)
  {
    sprintf(release,"X11R%d",VendorRelease(display));
    XCloseDisplay(display);
    result = 1;
  }
  else
  {
    result =  0;
  }
  fclose(stderr);
  return result;
}



