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
 * Brief description: returns the ESPS base directory.
 *
 */

static char *sccs_id = "@(#)getespsbas.c	1.3	2/20/96	ERL";

/* INCLUDE FILES */

#include <stdio.h> 
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/spsassert.h>

/*
 *  Get max length of path definitions.
 *  If not found then define it as 255.
 */

#include <sys/types.h>
#ifndef APOLLO_68K
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#ifndef MAXNAMLEN
# define MAXNAMLEN 255
#endif

/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "2/20/96"
#define EC_SCCS_VERSION "1.3"

/* LOCAL TYPEDEFS AND STRUCTURES */

#define DEF_ESPS_BASE "/usr/local/esps"


/*
 * get_esps_base(base_name)
 *  puts the ESPS base path in the string provided. Allocates
 *  new string if string is NULL. Uses ESPS_BASE environment
 *  variable if present and not empty.
 *
 */

char *get_esps_base(base_name)
     char *base_name;
{

  char *pathstring;  /* A new string to be allocated if none provided */

  /* First get a string to put it in. */

  if (base_name == NULL)  /* User wants a new string */
  {
    pathstring = (char *) calloc(MAXNAMLEN,sizeof(char));
    spsassert(pathstring != NULL,"Could not allocate memory.");
  }
  else /* User provided a string. */
  {
   pathstring = base_name;
  }


  /* Check ESPS_BASE environment variable first */

  if (getenv("ESPS_BASE") == NULL)
  {
    strcpy(pathstring,DEF_ESPS_BASE);
  }
  else
  {
    strcpy(pathstring,getenv("ESPS_BASE"));
    if (strlen(pathstring) == 0)  /* Handle empty environment variable */
    {
       strcpy(pathstring,DEF_ESPS_BASE);
    }  
  }

  return pathstring;

}

