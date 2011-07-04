/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1991-1993 Entropic Research Laboratory, Inc.
 *     All rights reserved."
 *
 * Program: mbuttons.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * functions for popping up buttons that invoke programs on the named file
 */

#ifndef lint
static char *sccs_id = "@(#)mbuttons.c	1.15     7/8/96     ERL";
#endif

#define VERSION "  "
#define DATE "  "

/*
 * system include files
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#ifndef APOLLO_68K
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/icon_load.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>

/*
 * esps include files
 */

#include <esps/esps.h>
#include <esps/unix.h>

#include <esps/exview.h>

/*
 * defines
 */

#ifndef NULL
#define NULL
#endif

#define LIST_INC 50
#define DEF_REXP "f*.sd"

#define Fprintf (void)fprintf
#define Fflush (void)fflush

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"mbuttons: %s - exiting\n", text); exit(1);}}

#define PROG Fprintf(stderr, "%s: ", ProgName)

#define DEBUG(n) if (debug_level >= n) Fprintf

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); SYNTAX; exit(1);}

#define EXIT Fprintf(stderr, "\n"); exit(1);

#define ERROR_EXIT1(fmt,a) {PROG; Fprintf(stderr, (fmt), (a)); EXIT}

#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num),sizeof(type))) == NULL) \
    ERROR_EXIT1("Can't allocate memory--%s", (msg))}

#define SYNTAX USAGE ("mbuttons [-b but_per_row] [-w window_title] \
[-i icon_title] [-c] [-h]\n\t [-q quit_button] [-Q quit_label] \
[-l quit_command] \n\t [-X x_pos] [-Y y_pos] [-x debug_level] menu_file")

/*
 * system functions and variables
 */

int getopt ();
extern  optind;
extern	char *optarg;
extern int fullscreendebug;

/*
 * global function declarations
 */
/* SDI 28/6/06 added LINUX to avoid compiler error */
#if !defined(hpux) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *calloc();
#endif
char *savestring();
void print_but_def();
char *re_comp();
long exv_init();

/*
 * global variable declarations
 */

int	debug_level = 0;
int     do_color = 0;
char	*ProgName = "mbuttons";
static void    *reaper();

/*
 * main program
 */

main (argc, argv)
int argc;
char **argv;
{
  int c;			/* for getopt return */
  char *menu_file;		/* file containing olwm-format menu */
  char *title = "ESPS Button Panel";   /* title for window */
  char *icon_title = "mbuttons"; /* icon title */
  int q_button = 0;		/* force quit button? (-q) */
  char *q_label = "QUIT";	/* label for quit button */
  char *q_command = NULL;	/* command on QUIT */
  int but_per_row = 10;		/* buttons per row */
#ifndef HP700
  Frame frame = (Frame *)NULL;		/* main frame */
#else
  Frame frame = 0;		/* main frame */
#endif
  Panel file_panel;		/* panel with file buttons */
  int rem_args;			/* remaining no. args on command line */
  int button_choice = 0;	/* if 1, panel_choices instead of menu button*/
  int choice_horiz = 0;		/* if 1, panel_choices are horizontal */
  int x_pos = -1;
  int y_pos = -1;
  bbox_par *but_params = NULL;

  fullscreendebug = 1;		/* prevents server grabs that crash SGI */
  if (xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, 0) == NULL) {
    Fprintf(stderr, "%s: XView init failed; is DISPLAY set?\n", ProgName);
    exit(1);
  }

  while ((c = getopt (argc, argv, "x:w:i:b:q:Q:l:X:Y:ch")) != EOF) {

    switch (c) {
    case 'x': 
      debug_level = atoi(optarg);
      break;
    case 'b': 
      but_per_row = atoi(optarg);
      break;
    case 'w':
      title = optarg;
      break;
    case 'i':
      icon_title = optarg;
      break;
    case 'q':
      /* note that BBOX_QUIT_BUTTON environment variable forces 
	 a quit button in a global way */
      q_button = atoi(optarg);
      break;
    case 'Q':
      q_label = optarg;
      break;
    case 'l':
      q_command = optarg;
      break;
    case 'X':
      x_pos = atoi(optarg);
      break;
    case 'Y':
      y_pos = atoi(optarg);
      break;
    case 'c':
      button_choice = 1;
      break;
    case 'h':
      choice_horiz = 1;
      break;
    default:
      SYNTAX;
    }
  }

  /*
   * Setup signal handler to reap zombie children
   */

  signal(SIGCHLD, reaper);

  rem_args = argc - optind;

  if (rem_args != 1) {
    if (rem_args == 0) 
      ERROR_EXIT("a menu file is required");
    if (rem_args > 1) 
      ERROR_EXIT("only one menu file can be specified");
  }

  if ((menu_file = FIND_ESPS_MENU(NULL, argv[optind])) == NULL) {

      Fprintf(stderr, "%s: can't find or read menu button file %s\n", 
	      ProgName, argv[optind]);

      SYNTAX;
    }

  if (choice_horiz == 1 && button_choice == 0)
    Fprintf(stderr, "%s: ignoring -h since -c was not used.\n", ProgName);
  
  if (debug_level) 
    Fprintf(stderr, "%s: using menu file %s for mbuttons\n", 
	    ProgName, menu_file);

  /* get default parameter set for button box */

  but_params = exv_bbox((bbox_par *)NULL, &frame, &file_panel);

  /* change the relevant defaults */

  but_params->menu_file = menu_file;
  but_params->n_per_row = but_per_row;
  but_params->quit_button = q_button;
  but_params->quit_data = q_command;
  but_params->quit_label = q_label;
  but_params->title = title;
  but_params->icon_title = icon_title;
  but_params->show = 1;
  but_params->button_choice = button_choice;
  but_params->choice_horizontal = choice_horiz;
  but_params->x_pos = x_pos;
  but_params->y_pos = y_pos;

  /* create button box */

  (void) exv_bbox(but_params, &frame, &file_panel);

  if (file_panel == (Panel *)NULL)
    ERROR_EXIT("Couldn't create buttons");

  if (frame != (Frame *)NULL) 
    xv_main_loop(frame);

  (void) exit(0);
}

/*
 * reaper - signal handler to reap zombie children
 */
static void *reaper() {
        int status;
#ifndef OS5
	wait3(NULL, WNOHANG, NULL);
#else
	waitpid((pid_t)-1, status, WNOHANG);
#endif
	signal(SIGCHLD, reaper);
}
