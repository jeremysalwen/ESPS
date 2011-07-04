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
 * Written by: Ken Nelson 
 * Checked by:
 * Revised by:
 *
 * Brief description: Appends the ESPS base dir and your string.
 *
 */

static char *sccs_id = "@(#)buildespsp.c	1.1	5/30/91	ERL";

/* INCLUDE FILES */

#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif

/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "5/30/91"
#define EC_SCCS_VERSION "1.1"

/* LOCAL MACROS */

char *build_esps_path(append_string)
	char *append_string;
{
	char pathstring[255];
	char *savestring();

	spsassert(append_string != NULL,"eman: couldn't build esps path");
	get_esps_base(pathstring);

	/* Append a / if one isn't there */
	if ( append_string[0] != '/' )
	 strcat(pathstring,"/");

	 strcat(pathstring,append_string);

	return savestring(pathstring);
}
