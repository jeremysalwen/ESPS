/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)exv_bbox.h	1.6 2/20/96 ERL
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: include file for x buttons
 *
 */

#ifndef exv_bbox_H
#define exv_bbox_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * bbox_par is the structure of parameters for exv_bbox()
 */

#include <esps/exv_olwm.h>

typedef struct _bbox_par {
  char **but_labels;		/* button labels */
  char **but_data;		/* button data strings */
  char *menu_file;		/* olwm menu file name */
  void (*but_data_proc)();	/* execution function for button data */
  int quit_button;		/* if true, include QUIT button on panel */
  char *quit_label;		/* if non-NULL, name for QUIT button */
  char *quit_data;		/* QUIT button data string */
  void (*quit_data_proc)();	/* execution function for QUIT button data */
  int n_per_row;		/* number of buttons per row */
  int button_choice;		/* if true, show submenus as panel choice 
				   items instead of button menus */
  int choice_horizontal;	/* if true, panel choice items are 
				   displayed horizontally */
  Frame owner;			/* owner of button panel (may be NULL) */
  char *title;			/* title for button panel window */
  char *icon_title;		/* icon title for button panel window */
  int show;			/* if true, set XV_SHOW new frame */
  int x_pos;			/* if positive, sets x position of new frame */
  int y_pos;			/* if positive, sets y position of new frame */
} bbox_par;


#ifdef __cplusplus
}
#endif

#endif /* exv_bbox_H */
