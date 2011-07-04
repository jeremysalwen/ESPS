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

static char *sccs_id = "@(#)wsystem.c	1.3	9/9/91	ERL";
int    debug_level = 0;

#include <stdio.h>

#define EC_SCCS_DATE "9/9/91"
#define EC_SCCS_VERSION "1.3"

int
main(argc, argv)
     int  argc;
     char **argv;
{

  char xrelease[100];

  if (checkForSunview() && !checkForX(xrelease))
  {
    puts("Sunview");
    exit(1);
  }
  else
  if (checkForX(xrelease))
  {
    puts(xrelease);
    exit(1);
  }
  else
  {
    puts("none");
  }
}


