/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson, ESI
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * xversion -- version information for program "xwaves"
 */

static char *sccs_id = "@(#)version.c	1.1 4/4/96 ERL";

#include <stdio.h>
#include <esps/esps.h>

char	*ProgName = "s16record";
char	*Version = "5.1 4/4/96";
char	*Date = "4/4/96";

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}
