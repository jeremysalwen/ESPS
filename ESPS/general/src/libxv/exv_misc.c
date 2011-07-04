/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: xhelp.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * functions for creating help window
 */

#ifndef lint
static char *sccs_id = "@(#)exv_misc.c	1.2	9/26/90	ESI";
#endif

/*
 * system include files
 */
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/cms.h>

#include <esps/exview.h>

/* this function is used as the callback procedure for the
   XV_KEY_DATA_REMOVE_PROC attribute in xview calls; this will free
   the space used by objects that are passed to callback procedures 
   via XV_KEY_DATA, at the time the objects are destroyed; purpose is 
   to avoid core leakage.  Taken from Dan Heller (O'Reilly p. 173) 
*/

/*
  note that it appears for now that XV_KEY_DATA_REMOVE_PROC is 
  broken in Xview, so do not use 
*/

void
exv_free_data(object, key, data)
Xv_object object; 
int       key;
caddr_t   data;
{
  fprintf(stderr, "freeing object in exv_free_data\n"); 
  free(data);
}


