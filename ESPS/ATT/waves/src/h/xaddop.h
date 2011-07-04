/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)xaddop.h	1.3 9/26/95 ATT/ERL/ESI */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: include file for add_op GUI
 *
 */

/*
 * addop_par is the structure of parameters for xaddop()
 */

typedef struct _addop_par {
  Frame owner;			/* owner of main panel  */
  char **global_vars;		/* NULL-terminated string array of global
				   variables */
  char **view_vars;		/* NULL-terminated string array of 
				   view_specific variables */
  char **operations;		/* NULL-terminated string array of current ops */
  int (*apply_add_op)();	/* function to call to execute the add_op  */
  char **(*get_attr_list)();	/* function to call to get list of 
				   operation's attributes */
  char *helpfile;		/* full path to help file */
  int addop_x_pos;		/* x position of main add_op window  */
  int addop_y_pos;		/* y position of main add_op window */
  int show_vars_win;		/* whether or not to show vars popup at start */
  int vars_x_pos;		/* x position of variables popup  */
  int vars_y_pos;		/* y position of variables popup */
  int show_ops_win;		/* whether or not to show operations popup */
  int ops_x_pos;		/* x position of operations popup */
  int ops_y_pos;		/* y position of operations popup */
} addop_par;



  

