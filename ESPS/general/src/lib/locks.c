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
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)locks.c	1.11	7/15/96	ESI/ERL";



/*----------------------------------------------------------------------+
|									|
|  Module: locks.c							|
|									|
|  These functions create and remove a lock file to help waves+ and	|
|  other functions that use the dsp board keep from interfering with	|
|  each other.								|
|									|
 -----------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#if defined(MACII) || defined(SG) || defined(HP) 
#include <sys/types.h>
#endif
#ifdef OS5
#include <sys/fcntl.h>
#endif
#include <sys/file.h>
#include <esps/locks.h>

#define MAXHOSTSIZE 31
#define LCK_DIR "/tmp/"

#ifndef DEC_ALPHA
char	    *malloc();
#endif

extern int  errno;

static char *initpath();


static int 
locking_enabled()
{
  return (getenv("DSP_NO_LOCK") == NULL);
}

int
set_lock(wait, prefix)
    int     wait;
    char    *prefix;
{
    int	    flags = O_RDONLY|O_CREAT|O_EXCL;
    int	    mask = 0;
    char    *path = initpath(prefix);
    int	    opn;


   if (locking_enabled())
   {
    while ((opn = open(path, flags, mask)) == -1
	   && errno == EEXIST
	   && wait > 0)
    {
	sleep(1);
	wait--;
    }

    if (opn == -1)		/* open failed */
	switch (errno)
	{
	case EEXIST:		/* lock file exists */
	    (void) fprintf(stderr, "%s: lock file %s exists.\n",
			   "set_lock", path);
	    return LCKFIL_INUSE;
	    break;
	default:		/* error */
	    (void) fprintf(stderr, "%s: can't create lock file %s.\n",
			   "set_lock", path);
	    perror("set_lock");
	    return LCKFIL_ERR;
	    break;
	}
    else			/* open succeeded */
    if (close(opn) == -1)	/* close failed */
    {
	(void) fprintf(stderr, "%s: can't close lock file %s.\n",
		       "set_lock", path);
	perror("set_lock");
	return LCKFIL_ERR;
    }
    else			/* close succeeded */
	return LCKFIL_OK;
   }
   else  /* DSP Locking disabled */
   {
     return LCKFIL_OK;
   }
}


int
rem_lock(prefix)
    char    *prefix;
{
    char    *path = initpath(prefix);

    if (locking_enabled())
    {
      if (unlink(path) == -1)	/* failure */
      {
	  (void) fprintf(stderr, "%s: can't remove lock file %s.\n",
		         "rem_lock", path);
	  perror("rem_lock");
	  return LCKFIL_ERR;
      }
      else			/* success */
	  return LCKFIL_OK;
    }
    else
    {
      return LCKFIL_OK;
    }
}

static char *
initpath(prefix)
    char    *prefix;
{
    char    *path = malloc(strlen(LCK_DIR) + strlen(prefix) + MAXHOSTSIZE + 1);

    strcpy(path, LCK_DIR);
    strcat(path, prefix);
    gethostname(path + strlen(path), MAXHOSTSIZE + 1);
    return path;
}
