/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: replacement for usleep
 *
 */

static char *sccs_id = "@(#)usleep.c	1.1	21 Mar 1998	ERL";

#if defined(OS5)

#include <sys/time.h>

void
usleep(unsigned int usecs)
{
        struct timeval tval;

        tval.tv_sec = usecs / 1000000;
        tval.tv_usec = usecs % 1000000;
        select(0,NULL,NULL,NULL, &tval);
}

#endif
