/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * Search an array for a string.  Return -1 if not found
 */

static char *sccs_id = "@(#)lin_search.c	1.13	24 Mar 1997	ESI/ERL";

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#if !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif
#include <esps/esps.h>

char *savestring();

int
lin_search(array, string)
char *array[], *string;
{
  int i, j;
  int strlen(), strcmp();
  char *temp, *temp2;
  assert(array);
  assert(string);
  temp = savestring(string);
  for(i=0; i<strlen(string); i++) 
	if(islower(temp[i])) temp[i] = temp[i]-'a'+'A'; 
  for(i=0; array[i] != NULL; i++) {
	temp2 = savestring(array[i]);
  	for(j=0; j<strlen(temp2); j++) 
	   if(islower(temp2[j])) temp2[j] = temp2[j]-'a'+'A'; 
 	if(strcmp(temp,temp2) == 0)
	{
	    free(temp);
	    free(temp2);
	    return(i);
	}
	free(temp2);
  }
  free(temp);
  return(-1);
}

int
lin_search2(array, string)
char *array[], *string;
{
  int i, strcmp();
  assert(array);
  assert(string);
  for(i=0; array[i] != NULL; i++)
 	if(strcmp(string,array[i]) == 0) return(i);
  return(-1);
}

