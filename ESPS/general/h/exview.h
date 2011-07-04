/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *     Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)exview.h	1.9 2/20/96 ESI/ERL
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: main include file for ESPS libxv
 *
 */

#ifndef exview_H
#define exview_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* XVIEW XV_KEY_DATA item defs */

#include <esps/exv_keys.h>

/* for icons */

#undef EXV_ICON_BITMAPS
#include <esps/exv_icon.h>

/* for colormaps*/ 

#include <esps/exv_colors.h>

/* for button boxes */

#include <esps/exv_bbox.h>

/* function types*/  

#include <esps/exv_func.h>

/*misc. defines*/

/*used in calls to exv_make_text_window() */

#define WITH_FIND_BUTTON 1      
#define WITHOUT_FIND_BUTTON 0

#define USE_FRAME_CMD 0
#define USE_FRAME_INDEP 1

#ifdef __cplusplus
}
#endif

#endif /* exview_H */
