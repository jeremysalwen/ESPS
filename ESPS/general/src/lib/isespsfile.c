/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  (unknown)
 * Checked by:
 * Revised by:
 *
 * Brief description:  determine whether a given string is the name
 *	of a valid (and readable) ESPS file
 */

static char *sccs_id = "@(#)isespsfile.c	1.2	21 Mar 1998	ERL";

#include <stdio.h>
#include <esps/esps.h>

int
is_esps_file(name)
char *name;
{
	FILE *fp;
	struct header *hd;
	unsigned int flags;
	char *ptr;

	fp = fopen(name,"r");
	if(fp) {
		hd = read_header(fp);
		close(fp);
		if(hd) {
			free_header(hd,flags,ptr);
			return 1;
		}
	}
	return 0;
}
		
