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
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: pops up text window contain ascii output of 
 *                    arbitrary command
 */

static char *sccs_id = "@(#)xtext.c	1.11	4/7/93	ERL";

#define VERSION "1.11"
#define DATE "4/7/93"

/*
 * system include files
 */
#include <stdio.h>
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
#define NULL 0
#endif

#define Fprintf (void)fprintf

#define Fflush (void)fflush

#define PROG Fprintf(stderr, "%s: ", ProgName)

#define DEBUG(n) if (debug_level >= n) Fprintf

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); SYNTAX; exit(1);}

#define EXIT Fprintf(stderr, "\n"); exit(1);

#define ERROR_EXIT1(fmt,a) {PROG; Fprintf(stderr, (fmt), (a)); EXIT}

#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num),sizeof(type))) == NULL) \
    ERROR_EXIT1("Can't allocate memory--%s", (msg))}

#define SYNTAX USAGE ("xtext [-t window_title] [-i icon_title] [-F file_name] \n\t[-L] [-x debug_level] command_line")
/*
 * system functions and variables
 */

int getopt ();
extern  optind;
extern	char *optarg;
/*
 * global function declarations
 */

char *savestring();
char *e_temp_name();
/*
 * global variable declarations
 */

int		    debug_level = 0;
int                 do_color = 0;
char		    *ProgName = "xtext";

extern int	    fullscreendebug; /* inside of xview */

/*
 * main program
 */
main (argc, argv)
int argc;
char **argv;
{

  int c;			/* for getopt return */
  char *template = "xtextXXXXXX"; /* template for temp fie */
  char command_line[200];	/* for program command line */
  char title[200];		/* title for window */
  char *icon_title;		/* icon title */
  char *tempfile;		/* temp file for program output */
  int ret = 1;			/* return from system call */
  int iflag = 0, tflag = 0;	/* option flags */
  int Fflag = 0, Lflag = 0;
  Frame frame;		   /* main frame */
  int i;

  fullscreendebug = 1;		/* this global prevents server grabs that
				   crash the SGIs */
  xv_init (XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

  if (argc == 1) 
    SYNTAX;

  while ((c = getopt (argc, argv, "LF:x:t:i:")) != EOF) {

    switch (c) {
    case 'L':
      Lflag++;
      break;
    case 'x': 
      debug_level = atoi (optarg);
      break;
    case 't':
      sprintf(title, "%s", optarg);
      tflag++;
      break;
    case 'i':
      icon_title = optarg;
      iflag++;
      break;
    case 'F':
      tempfile = find_esps_file((char *)NULL, optarg, ".", (char *) NULL);
      if (!tempfile) 
	ERROR_EXIT1("Couldn't find readable file %s", optarg); 
      Fflag++;
      break;
    default:
      SYNTAX;
    }
  }

  if (!iflag) {
      if (Fflag) 
	icon_title = tempfile;
      else
	icon_title = savestring(argv[optind]);
    }

  if (Fflag) {
    if (debug_level) 
      Fprintf(stderr, "%s: used -F, will display contents of %s\n", 
	      ProgName, tempfile);
    if (optind != argc) {
      ERROR_EXIT("Can't give command line with -F option")
      }

    if (!tflag) 
      sprintf(title, "%s", tempfile);

    Lflag = 1; /*always leave file in place (don't unlink)*/
  }
  else { /* want to display output of command_line */

    /* build argument list for command line*/

    sprintf(command_line, "%s ", argv[optind++]); 

    for (i = optind++; i < argc; i++)  {
      (void) strcat(command_line, argv[i]);
      (void) strcat(command_line, " ");
    }

    /*use the command line as the window title*/
  
    tempfile = e_temp_name(template); 

    if (!tflag) 
      sprintf(title, "%s (%s)", command_line, tempfile);

    /* add redirection to tempfile onto command line*/

    (void) strcat(command_line, " > ");
    (void) strcat(command_line, tempfile);

    /* run command */

    if (debug_level)
    Fprintf(stderr, "Running shell command:\n%s\n", command_line);    

    ret = system(command_line); 

    if (ret != 0) 
      Fprintf(stderr, "%s: trouble running command:\n\t%s\n", ProgName, command_line); 

    if (debug_level)
      Fprintf(stderr, "%s: displaying text output\n", ProgName); 
  }

  /* now pop-up the display (even if psps failed, as we will get 
     an XView notice 
     */

  frame = exv_make_text_window((Frame) NULL, title, icon_title, tempfile, 
			       WITH_FIND_BUTTON, USE_FRAME_INDEP);

  if (frame != NULL) 
    xv_main_loop(frame);

  /* be nice, and remove the temp file containing command output */

  if (!Lflag) {
    if (debug_level)
      Fprintf(stderr, 
	    "%s: unlinking temp text file %s\n", ProgName, tempfile); 

    unlink(tempfile);
  }
  (void) exit(0);
}


