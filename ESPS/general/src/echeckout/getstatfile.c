/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)getstatfile.c	1.4	7/23/96	ERL";

#include <sys/param.h>
#ifdef OS5
#include <netdb.h>
#endif

char *getenv();

char hostname[MAXHOSTNAMELEN];


char *
get_stat_file(code)
char code;
{
	char *path, *home;

	(void)gethostname(hostname,MAXHOSTNAMELEN-1);
	if(!hostname[0]) 
		return NULL;

	home = getenv("HOME");
	if(!home)
		return NULL;

	path = (char*)malloc(strlen(hostname)+strlen(home)+strlen("/._echeck5")+1);
	(void)sprintf(path,"%s/.%s_%ccheck5",home,hostname,code);
	return path;
}
	
