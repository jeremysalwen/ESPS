/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:  John Shore 
 *
 * Brief description:
 *        
 *   addstr will allocate and append to a string array
 *   strlistlen returns the current array length
 *
 */

static char *sccs_id = "@(#)addstr.c	1.2	4/21/91	ESI/ERL";

#include <stdio.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/esps.h>
 
void
addstr(str, arr)
    char    *str;
    char    ***arr;
{
    int n;

    spsassert(str,"str is NULL");
    spsassert(arr,"arr is NULL");
    
    if (*arr == NULL) {

	*arr = (char **) malloc(sizeof(char *));
	spsassert(*arr, "can't allocate space for array of file names");
	(*arr)[0] = NULL;
      }

    for (n = 0; (*arr)[n] != NULL; n++)
    { }
    *arr =
	(char **) realloc((char *) *arr, (unsigned) (n + 2) * sizeof(char *));
    spsassert(*arr, "can't reallocate memory for string array");
    (*arr)[n] = str;
    (*arr)[n+1] = NULL;
}

int
strlistlen(strlist)
char **strlist;
{
  int i = 0;

  if (*strlist == NULL) 
    return 0;

  while (strlist[i] != NULL) 
    i++;
  return i;
}
