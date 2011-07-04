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
 * %W% %G% ESI/ERL
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: include file with libxv func definitions
 *
 */

#ifndef exv_func_H
#define exv_func_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

Cms
exv_create_cms ARGS((void));

Cms
exv_init_colors ARGS((Xv_opaque win));

char *
exv_cms_name ARGS((void));

int
exv_color_status ARGS((Frame frame));


#ifndef HP400
Icon
exv_attach_icon ARGS((Frame frame, int icon_id, char *label, int trans));

#endif

int
exv_get_help ARGS((Panel_item item, Event *event));

Frame
exv_make_text_window ARGS((Frame owner, char *window_title, char *icon_title,
			   char *textfile, int find, int indep));

char *
exv_get_helpfile ARGS((char *progfile));

void
exv_free_data ARGS((Xv_object object, int key, caddr_t data));

int
exv_prompt_params ARGS((char *param_file, int flag, char *check_file,
			char *outfile, char *program, int x_pos, int y_pos));

bbox_par *
exv_bbox ARGS((bbox_par *params, Frame *bbox_frame, Panel *bbox_panel));

void
print_bbox_par ARGS((bbox_par *but_def));

menudata *
read_olwm_menu ARGS((char *menu_file));

void
print_olwm_menu ARGS((menudata *men));


#ifdef __cplusplus
}
#endif

#endif /* exv_func_H */
