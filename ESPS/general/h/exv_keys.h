/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)exv_keys.h	1.7 2/20/96 ESI/ERL
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: defines for XV_KEY_DATA items
 *
 */

#ifndef exv_keys_H
#define exv_keys_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/* used in exv_get_help()*/

#define EXVK_HELP_NAME      201 /*full path to help file or name of ESPS prog*/
#define EXVK_HELP_TITLE     202 /*title of help window */
#define EXVK_FIRST	    203 /*used by search routines */
#define EXVK_FIND_STRING    204 /*used by search routines */

/* used in exv_bbox() */

#define EXVK_BUT_DATA_PROC    205 /*used by exv_bbox*/

#ifdef __cplusplus
}
#endif

#endif /* exv_keys_H */
