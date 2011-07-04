/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: support for eddy (EnSig)
 *
 */

static char *sccs_id = "@(#)eddy.c	1.1	3/18/96	ERL";

#define INSIDE_ENSIG "SON_OF_EDDY"
char *getenv();

int
child_of_ensig()
{
	return (int)getenv(INSIDE_ENSIG);
}
