/* Copyright (c) 1995 Entropic Research Laboratory, Inc.  All rights reserved.
 */
/* @(#)xprint_util.h	1.1 11/13/95 ATT/ERL/ESI */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description: print_graphics definitions
 *
 */

#include <X11/Xlib.h>

extern int doing_print_graphic;

extern Display	*get_xp_display();
extern Display	*setup_xp_from_globals();
extern Display	*setup_xp_EPS_temp();
extern void	start_xp();
extern void	finish_xp();
