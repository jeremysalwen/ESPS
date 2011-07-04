/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)strlist.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS programs that use NULL-terminated string arrays.
 *
 */

#ifndef strlist_H
#define strlist_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

void
addstr ARGS((char *str, char ***arr));

int
lin_search ARGS((char **array, char *string));

int
lin_search2 ARGS((char **array, char *string));

int
strlistlen ARGS((char **strlist));

int
idx_ok ARGS((int code, char **array));


#ifdef __cplusplus
}
#endif

#endif /* strlist_H */

