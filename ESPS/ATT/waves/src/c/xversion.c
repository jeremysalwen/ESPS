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

static char *sccs_id = "@(#)xversion.c	2.00	5 Apr 2005	D. Talkin";

#include <stdio.h>
#include <esps/esps.h>

char	*ProgName = "xwaves";
char	*Version = "6.0";
char	*Date = "05 Apr 2005";
 
static char    *VersionFile = "version6.0";

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}

/*
 * check the version of the base esps/waves installation.  This version
 * requires ESPS 5.3 in order to work
*/

int
check_version()
{
  return(0);
}
	
	
