/*
 * This material contains proprietary software of Entropic Research Laboratory,
 * Inc. Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Research Laboratory, Inc. is strictly 
 * prohibited. Any public distribution of copies of this work authorized in 
 * writing by Entropic Research Laboratory, Inc. must bear the notice
 *
 *  "Copyright (c) 1990 Entropic Research Laboratory, Inc.; All rights reserved"
 *
 * Program: erldemo.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This is wavesdemo - interactive front end waves+/ESPS demo
 */

#ifndef lint
static char *sccs_id = "@(#)erldemo.c	1.49 4/3/92	ERL";
#endif

#define VERSION "1.49"
#define DATE "4/3/92"

#define NO_AUDIO 		0
#define SUN_SPARC_AUDIO 	1
#define SUN_VME_AUDIO 		2
#define MC5600_AUDIO 		3
#define SONY_AUDIO     		4
#define NETWORK_AUDIO 		5
#define SGI_AUDIO               6

#define REPEAT_BUTTON_KEY 1785 /*XV_KEY for repeat button*/

/* exists if this or another copy of erldemo is running */

#define ERLDEMO_RUNNING_FLAG  "/tmp/ERLDEMO.lock"

/* exists if the demo is running */

#define DEMO_RUNNING_FLAG  "/tmp/erldemo.lock"

/* exists if waves+ is running from demo */

#define WAVES_RUNNING_FLAG  "/tmp/ewaves.lock"

/* exists if slide show is running (currently not exploited)*/ 

#define SLIDES_RUNNING_FLAG  "/tmp/eslides.lock"

/* exists if wtry is running */

#define WTRY_RUNNING_FLAG "/tmp/erlwtry.lock"

/* exists if a single demo is needed */

#define SINGLE_DEMO_FLAG  "/tmp/erldemo.single"

/* The existence of one or the other or both of these controls whether
 * the demo consists of a live waves+ intro, a slide show, or both
 */

#define WAVES_DEMO_FLAG   "/tmp/erldemo.live"
#define SLIDE_SHOW_FLAG   "/tmp/erldemo.slides"

/* The following define the top level scripts that are called when the
 * buttons "start demo" and "interactive waves+" are called; they contain 
 * all of the logging and locking logic and should not change.  In turn
 * they call the next group of defines, which implement the actual demos
 */

#define TOP_DEMO_SCRIPT "erldemo.sh"
#define ONE_DEMO_SCRIPT "one_demo.sh"
#define TOP_WTRY_SCRIPT "erlwtry.sh"

/* The following define the demo scripts that actually implement the 
 * three possible active demos: the introductory (live running) waves 
 * demo, the slide show, and the interactive waves demo.  The first two 
 * are called from TOP_DEMO_SCRIPT (invoked by "Start demo" button), and
 * the third is called from the TOP_WTRY_SCRIPT (invoked by "Interactive
 * waves" button).  The actual scripts are copied to these files by 
 * erldemo when it starts up (with the sources determined by command-line
 * options)
 */

#define WAVES_DEMO_SCRIPT "/tmp/e_wdemo.sh"
#define SLIDE_SHOW_SCRIPT "/tmp/e_slides.sh"
#define WTRY_SCRIPT "/tmp/e_wtry.sh"

/*
 * system include files
 */
#include <stdio.h>
#include <signal.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/tty.h>
#include <xview/text.h>
#include <xview/cms.h>
#include <xview/notice.h>
/*
 * esps include files
 */
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/exview.h>
/*
 * defines
 */
#define Fprintf (void)fprintf
#define Fflush (void)fflush

#define PROG Fprintf(stderr, "%s: ", ProgName)

#define DEBUG(n) if (debug_level >= n) Fprintf

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); SYNTAX; }

#define EXIT Fprintf(stderr, "\n"); remove_file(ERLDEMO_RUNNING_FLAG); exit(1);

#define ERROR_EXIT1(fmt,a) {PROG; Fprintf(stderr, (fmt), (a)); EXIT}

#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num),sizeof(type))) == NULL) \
    ERROR_EXIT1("Can't allocate memory--%s", (msg))}

#define SYNTAX Fprintf(stderr,"erldemo [-m io_type] [-s] [-W wavesintro] [-S slideshow]\n\t[-T wavestry] [-I more_info_file] [-A about_file] [-t window_title]\n\t[-X x_loc] [-Y y_loc] [-x debug_level]"); EXIT

/*
 * system functions and variables
 */
int getopt ();
extern  optind;
extern	char *optarg;
/*
 * external ESPS functions
 */

/*
 * global declarations
 */
char *mach_io_types[] = {
  "NONE",
  "SUN_SPARC",
  "SUN_VME",
  "MASSCOMP",
  "SONY",
  "NETWORK",
  "SGI",
  NULL};

/* these are ugly as globals, but hey...*/

Frame mainframe = NULL;		/* top level frame - owns everything */

Panel_item feedback = NULL;	/*PANEL_MESSAGE item for feedback messages*/

void sig_handler();		/* interrupt handler routine */
#if !defined(hpux) && !defined(IBM_RS6000) && !defined(SG)
char *getwd();
char *sprintf();
char *calloc();
#endif
char *savestring();
Panel create_info_panel();
Panel create_control_panel();
void link_file();
void set_flag_file();
void remove_file();
void set_play_command();
void set_sparc_output();
void set_sun4_output();
void set_mc_output();
void set_sony_output();
void set_sgi_output();
void set_net_output();
int file_exists();
int run_demo();
void start_demo();
void select_repeat();
void select_demo();
void try_waves();
void exit_proc();
void display_text();
void display_info();
void display_about();
char *make_temp();
void stop_demo();
void feedback_message();
Notify_value destroy_func();
Notify_value sigint_func();

int		    debug_level = 0;
int                 do_color = 0;
char		    *ProgName = "erldemo";

/*
 * main program
 */
main (argc, argv)
int argc;
char **argv;
{
 /*
  * setup and initialization
  */
  int		    c;		/* for getopt return */
  Frame frame = NULL;		/* the main frame */
  Panel demo_panel;
  char *waves_demo_command = NULL;
  char *slideshow_command = NULL;
  char *waves_try_command = NULL;
  int  Wflag = 0;
  int  Sflag = 0;
  int  Tflag = 0;
  int demo_choices;		/* allow intro, slides, or both */
  char *frame_title = "Entropic Demo"; 
  char *info_file = "edemo.info";
  char *about_file = "edemo.about";
  int  x_loc = 670;		/* X location of command window */
  int  y_loc = 0;		/* Y location of command window */
  int  mach_io_type = SUN_SPARC_AUDIO;
  int  silent_demo = 0;		/* true to force silent demos */
  int  frame_height;		/* height of command window */
  FILE *tfd;			/* for testing /tmp write permission */

  static Xv_singlecolor  ffg = {0x00, 0x00, 0x00};
  static Xv_singlecolor  fbg = {0xf0, 0xff, 0xff};


  if (file_exists(ERLDEMO_RUNNING_FLAG)) {
    char e_message[80];		/* error message buffer */
    sprintf(e_message, "         If not, please \"rm %s\".\n", 
	    ERLDEMO_RUNNING_FLAG);
    Fprintf(stderr, 
	    "\nerldemo: It appears that another copy of the demo is running.\n");
    Fprintf(stderr, e_message);
    exit(1);
  }

  set_flag_file(ERLDEMO_RUNNING_FLAG);


  xv_init (XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);
  

  /*
   * process command line options
   */

  while ((c = getopt (argc, argv, "m:sW:S:T:I:A:t:X:Y:x:")) != EOF) {
    switch (c) {

    case 'm':
      mach_io_type = lin_search(mach_io_types, optarg);
      if (mach_io_type == -1) 
	ERROR_EXIT1("erldemo: unknown machine type %s", optarg);
      break;
    case 's':
      silent_demo = 1;
      break;
    case 'W':
      waves_demo_command = optarg;
      Wflag++;
      break;
    case 'S':
      slideshow_command = optarg;
      Sflag++;
      break;
    case 'T':
      waves_try_command = optarg;
      Tflag++;
      break;
    case 'I':
      info_file = optarg;
      break;
    case 'A':
      about_file = optarg;
      break;
    case 't':
      frame_title = optarg;
      break;      
    case 'X':
      x_loc = atoi(optarg);
      break;
    case 'Y':
      y_loc = atoi(optarg);
      break;
    case 'x': 
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX;
    }
  }
  /* check to make sure demo commands are supplied */

  if (!(Wflag || Sflag))
    ERROR_EXIT("erldemo: can't run without at leasts one of -W and -S");

  /* make sure can read the demo commands */

  if (Wflag && !file_exists(waves_demo_command)) 
    ERROR_EXIT1("erldemo: can't read waves+ intro command %s", 
		waves_demo_command);

  if (Sflag && !file_exists(slideshow_command)) 
    ERROR_EXIT1("erldemo: can't read slideshow command %s", 
		slideshow_command);

  if (Tflag && !file_exists(waves_try_command)) 
    ERROR_EXIT1("erldemo: can't read interactive waves+ command %s", 
		waves_try_command);

  /* make sure can read the wired-in scripts */

  if (!file_exists(TOP_DEMO_SCRIPT))
    ERROR_EXIT1("erldemo: can't read top level demo script %s", 
		TOP_DEMO_SCRIPT);

  if (!file_exists(ONE_DEMO_SCRIPT)) 
    ERROR_EXIT1("erldemo: can't read the single loop demo script %s", 
		ONE_DEMO_SCRIPT);

  if (Tflag && !file_exists(TOP_WTRY_SCRIPT)) 
    ERROR_EXIT1("erldemo: can't read top level interactive waves+ script %s", 
		TOP_WTRY_SCRIPT);

  /* make sure can read and write in /tmp */ 

  if ((tfd = fopen("/tmp/e_test_write", "w")) == NULL)
    ERROR_EXIT("erldemo: must have read/write permission in /tmp to run; exiting")
  else {
    fclose(tfd);
    unlink("/tmp/e_test_write"); 
  }
  /* Set initial value for panel item controlling the choice of 
   * intro and slide show;  This is determined by which combination
   * of -W and -S was set on startup (at least one was).
   */
  if (Wflag && Sflag) /*both waves intro and slides are allowed*/
   {
    demo_choices = 3;
    set_flag_file(WAVES_DEMO_FLAG);
    set_flag_file(SLIDE_SHOW_FLAG);
   }
  else if (Sflag) /* only slideshow is allowed */
   {
    demo_choices = 2;
    set_flag_file(SLIDE_SHOW_FLAG);
   }
  else if (Wflag) /* only waves intro is allowed */
   {
    demo_choices = 1;
    set_flag_file(WAVES_DEMO_FLAG);
   }

  DEBUG(1)(stderr, "erldemo: setting up files and links in /tmp\n");

  /*default initial demo repeat is single */

  set_flag_file(SINGLE_DEMO_FLAG);

  /* setup up command scripts for waves+ intro, slide show, and 
   * interactive waves+
   */

  if (Wflag) 
    link_file(waves_demo_command, WAVES_DEMO_SCRIPT);

  if (Sflag) 
    link_file(slideshow_command, SLIDE_SHOW_SCRIPT);

  if (Tflag)
    link_file(waves_try_command, WTRY_SCRIPT);

  DEBUG(1)(stderr, "erldemo: creating base frame (command window)\n");

  /* Since the -m option yields different sized stacks of I/O buttons, 
   * we set the overall height of the window accordingly.  Also 
   * affected by whether or not there's a tryout button (-T). 
   */

  switch (mach_io_type) {
  case SUN_SPARC_AUDIO:
    if (Tflag)
      frame_height = 287;
    else 
      frame_height = 266;
    break;
  case SUN_VME_AUDIO:
  case MC5600_AUDIO:
  case SONY_AUDIO:
  case NETWORK_AUDIO:
  case SGI_AUDIO:
  case NO_AUDIO:
  default:
    frame_height = 266;
    break;
    }

  frame = (Frame)xv_create(NULL, FRAME,	
			   FRAME_LABEL,    frame_title,
			   XV_X,     x_loc,
			   XV_Y,     y_loc,
			   XV_WIDTH, 375,
			   XV_HEIGHT, frame_height,
/* Next 3 lines fix colormap problem on Sony-- don't know why. */
                           FRAME_INHERIT_COLORS,       FALSE,
                           FRAME_FOREGROUND_COLOR,     &ffg,
                           FRAME_BACKGROUND_COLOR,     &fbg,
			   FRAME_CLOSED, FALSE,
			   FRAME_SHOW_FOOTER, FALSE,
			   FRAME_SHOW_RESIZE_CORNER, TRUE,
			   NULL);

  if (frame == NULL) {
    Fprintf(stderr, "%s: couldn't create base frame\n");
    EXIT;
  }

  notify_interpose_destroy_func(frame, destroy_func);
  notify_set_signal_func(frame, sigint_func, SIGINT, NOTIFY_SYNC);

  /* set global for color treatment*/
  
  do_color = exv_color_status(frame);

  demo_panel = create_info_panel(frame, about_file, info_file);

  demo_panel = create_control_panel(frame, TOP_DEMO_SCRIPT, TOP_WTRY_SCRIPT, 
				    mach_io_type, demo_choices, 
				    Tflag, silent_demo); 
  window_fit(frame);

  (void) exv_attach_icon(frame, ERL_NOBORD_ICON, "demo", TRANSPARENT);

  /*set global*/

  mainframe = frame;

  xv_main_loop(frame);

  exit(0);
}

Panel
create_info_panel(frame, about_file, info_file)
Frame  frame;			/* the main frame */
char *about_file;		/* file for "about this demo" */
char *info_file;		/* file for "for more info" */
{
  /*
  create basic frame with titles and buttons for information 
  about the demo and more info about the products.
  */
  Panel  con_panel;		/* panel for control buttons */
  Panel_item button;		/* temp handle */
  Panel_item message;		/* temp handle */

  DEBUG(1) (stderr, "Entered create_info_panel\n");

  con_panel = (Panel)xv_create(frame, PANEL, 
			       OPENWIN_SHOW_BORDERS, TRUE,
			       WIN_BORDER, TRUE,
			       XV_X, 0,
			       XV_Y, 0,
			       XV_WIDTH, WIN_EXTEND_TO_EDGE,
			       XV_HEIGHT, 101,
			       NULL);
  if (do_color) {

    (void) exv_init_colors(con_panel); 

    xv_set(con_panel, 
	   WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_GRAY,
	   WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
	   NULL);
  }
  /* put up a two lines of text*/

  message = xv_create(con_panel, PANEL_MESSAGE,
		      XV_X, 64,
		      XV_Y, 8,
		      XV_WIDTH, 233,
		      XV_HEIGHT, 13,
		      PANEL_LABEL_STRING, "Entropic Research Laboratory, Inc.",
		      PANEL_LABEL_BOLD, TRUE,
		      NULL);

  if (do_color) xv_set(message, 
		       PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_RED,
		       NULL);

  message = xv_create(con_panel, PANEL_MESSAGE,
		      XV_X, 20,
		      XV_Y, 32,
		      XV_WIDTH, 327,
		      XV_HEIGHT, 13,
		      PANEL_LABEL_STRING, "Unix Signal Processing Software Demonstration",
		      PANEL_LABEL_BOLD, TRUE,
		      NULL);

  if (do_color) xv_set(message, 
		       PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_FIREBRICK,
		       NULL);

  /* put up top panel buttons */

  button = (Panel_item) xv_create(con_panel, PANEL_BUTTON,
				      XV_X, 11,
				      XV_Y, 60,
				      XV_WIDTH, 127,
				      XV_HEIGHT, 25,
				      PANEL_LABEL_STRING, "About this demo...",
				      PANEL_CLIENT_DATA, about_file,
				      PANEL_NOTIFY_PROC, display_about,
				      NULL);

  if (do_color) 
    xv_set(button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

  button = (Panel_item) xv_create(con_panel, PANEL_BUTTON,
				      XV_X, 155,
				      XV_Y, 60,
				      XV_WIDTH, 156,
				      XV_HEIGHT, 25,
				      PANEL_LABEL_STRING, "For more information...",
				      PANEL_CLIENT_DATA, info_file,
				      PANEL_NOTIFY_PROC, display_info,
				      NULL);
  if (do_color) 
    xv_set(button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

  button = xv_create(con_panel, PANEL_BUTTON,
		       XV_X, 320,
		       XV_Y, 60,
		       XV_WIDTH, 44,
		       XV_HEIGHT, 25,
		       PANEL_CLIENT_DATA, frame,
		       PANEL_LABEL_STRING,  "Exit",
		       PANEL_NOTIFY_PROC,   exit_proc,
		       NULL);

  if (do_color) 
    xv_set(button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

/*  window_fit_height(con_panel);*/

  DEBUG(1) (stderr, "Leaving create_info_panel\n");

  return(con_panel);

}

Panel
create_control_panel(frame, demo_com, wtry_com, 
		     mach_type, demo_choices, tryflag, silent_demo)
Frame frame;			/* owner */
char *demo_com;			/* demo command script */
char *wtry_com;			/* interactive waves+ script */
int  mach_type;			/* code for machine type */
int  demo_choices;		/* allow intro, slides, or both */
                                /* 1 = waves only; 2 = slides only;*/
				/* 3 = both OK; */
int  tryflag;			/* TRUE if try waves+ enabled */
int  silent_demo;		/* TRUE for forced silence */
{
  Panel demo_panel;		/* panel with all the buttons on it */
  Panel_item demo_button;	/* temp handle */
  Panel_item repeat_button;	/* handle for single/continous button */
  Panel_item message;		/* temp handle */
  int feedback_y;		/* vertical position of feedback message */

  DEBUG(1) (stderr, "Entered create_control_panel\n");

  spsassert(frame != NULL, "null frame passed to create_control_panel");

  demo_panel = (Panel)xv_create(frame, PANEL, 
				XV_X, 0,
				XV_WIDTH, WIN_EXTEND_TO_EDGE,
				XV_HEIGHT, WIN_EXTEND_TO_EDGE,
				OPENWIN_SHOW_BORDERS, TRUE,
				WIN_BORDER, TRUE,
				NULL);
  if (do_color) {

    (void) exv_init_colors(demo_panel); 

    xv_set(demo_panel, 
	   WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_LIGHT_YELLOW,
	   WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
	   NULL);
  }

  /*set demo options */

  message  =  xv_create(demo_panel, PANEL_MESSAGE,
			XV_X, 233,
			XV_Y, 18,
			XV_WIDTH, 102,
			XV_HEIGHT, 13,
			PANEL_LABEL_STRING, "Demo Options:",
			PANEL_LABEL_BOLD, TRUE,
			NULL);

  if (do_color) xv_set(message,
		       PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
		       NULL);

  repeat_button = xv_create(demo_panel, PANEL_CHOICE,
			  XV_WIDTH, 95,
			  XV_HEIGHT, 46,
			  PANEL_VALUE_X, 233,
			  PANEL_VALUE_Y, 42,
			  PANEL_LAYOUT, PANEL_VERTICAL,
			  PANEL_CHOICE_NCOLS, 1,
			  PANEL_CHOICE_STRINGS,
			         "Single",
			         "Continuous",
			         0,
			  PANEL_VALUE,         0,
			  PANEL_CHOOSE_ONE,    TRUE,
			  PANEL_NOTIFY_PROC, select_repeat,
			  NULL);

  if (do_color) 
      xv_set(repeat_button, 
	     PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

  demo_button = xv_create(demo_panel, PANEL_CHECK_BOX,
			  XV_WIDTH, 115,
			  XV_HEIGHT, 52,
			  PANEL_VALUE_X, 233,
			  PANEL_VALUE_Y, 99,
			  PANEL_CHOICE_NCOLS, 1,
			  PANEL_CHOICE_STRINGS,
			       "waves+ intro",
			       "slide show",
			  0,
			  PANEL_VALUE,       demo_choices,
			  PANEL_CLIENT_DATA, demo_choices,		  
			  PANEL_NOTIFY_PROC, select_demo,
			  NULL);
  if (do_color) 
      xv_set(demo_button, 
	     PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

  /* now set the startup buttons*/ 

  demo_button = xv_create(demo_panel, PANEL_BUTTON,
			  XV_X, 20,
			  XV_Y, 18,
			  XV_WIDTH, 85,
			  XV_HEIGHT, 19,
			  PANEL_LABEL_STRING, "Start demo",
			  PANEL_NOTIFY_PROC, start_demo,
			  PANEL_CLIENT_DATA, demo_com,
			  XV_KEY_DATA, REPEAT_BUTTON_KEY, repeat_button,
			  NULL);
  if (do_color) 
      xv_set(demo_button, 
	     PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);

  /* we do not put up the interactive waves+ button unless -T option
   * was used (passed via tryflag).  Note, however, that we could 
   * still put it up, using NULL as the PANEL_CLIENT_DATA (the 
   * script to call), since try_waves will just put up a message 
   * if pressed.  
   */

  if (tryflag){
      demo_button = xv_create(demo_panel, PANEL_BUTTON,
			  XV_X, 20,
			  XV_Y, 48,
			  XV_WIDTH, 133,
			  XV_HEIGHT, 19,
			  PANEL_LABEL_STRING, "Interactive waves+",
			  PANEL_NOTIFY_PROC, try_waves,
			  PANEL_CLIENT_DATA, wtry_com,
			  NULL);
      if (do_color) 
          xv_set(demo_button, 
	     PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
  }

  /* set audio output options */

  demo_button = xv_create(demo_panel, PANEL_CHOICE,
			  XV_WIDTH, 152,
			  XV_HEIGHT, 86,
			  PANEL_VALUE_X, 20,
			  PANEL_VALUE_Y, (tryflag ? 97 : 70),
			  PANEL_CLIENT_DATA, silent_demo, 
			  PANEL_LAYOUT, PANEL_VERTICAL,
			  PANEL_LABEL_STRING, "Audio Output:",
			  PANEL_CHOOSE_ONE,    TRUE,
			  PANEL_CHOICE_NCOLS, 1,
			  NULL);

  switch (mach_type) {

  case SUN_SPARC_AUDIO:
    {
    int a_fd; 		/*temp file descriptor*/
    int audio_dev;  /*is there /dev/audio?*/

    /*select default for audio*/
    if((a_fd = open("/dev/audio", 1)) < 0)
      audio_dev = 0;	 
    else {
      audio_dev = 1;
      close(a_fd);
    }
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"SPARCstation Internal",
		"SPARCstation External",
		NULL,
	   PANEL_VALUE,         audio_dev,
	   PANEL_NOTIFY_PROC, set_sparc_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_sparc_output(demo_button, NULL);
    }
    break;

  case SUN_VME_AUDIO:
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"VME (AT&T DSP32)",
		NULL,
	   PANEL_VALUE,  0,
	   PANEL_NOTIFY_PROC, set_sun4_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_sun4_output(demo_button, NULL);
    break;

  case MC5600_AUDIO:
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"Concurrent D/A",
		NULL,
	   PANEL_VALUE,  0,
	   PANEL_NOTIFY_PROC, set_mc_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_mc_output(demo_button, NULL);
    break;

  case SONY_AUDIO:
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"Sony D/A",
		NULL,
	   PANEL_VALUE,  0,
	   PANEL_NOTIFY_PROC, set_sony_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_sony_output(demo_button, NULL);
    break;

  case NETWORK_AUDIO:
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"Network play",
		NULL,
	   PANEL_VALUE,  0,
	   PANEL_NOTIFY_PROC, set_net_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_net_output(demo_button, NULL);
    break;

  case SGI_AUDIO:
    xv_set(demo_button, 
	   PANEL_CHOICE_STRINGS,
	        "None",
		"SGI D/A",
		NULL,
	   PANEL_VALUE,  1, /* set up internal audio as default*/
	   PANEL_NOTIFY_PROC, set_sgi_output,
	   NULL);

    /* invoke call-back directly to get default behavior */

    set_sgi_output(demo_button, NULL);
    break;

  case NO_AUDIO:
  default:
    link_file("play_scripts/play.none", "/tmp/play");
    break;
  }

  if (mach_type != NO_AUDIO) {
    if (do_color) 
      xv_set(demo_button, 
	     PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
  }

  /*set global item for feedback messages; since the overall size of the
   *panel varies (with -m option causing different length of I/O options), 
   *we calculate the Y position to be at the bottom of the panel 
   */

  feedback_y = xv_get(demo_panel, XV_HEIGHT) - 17;

  feedback = xv_create(demo_panel, PANEL_MESSAGE,
		       XV_X, 3,
		       XV_Y, feedback_y,
		       XV_WIDTH, 233,
		       XV_HEIGHT, 13,
		       PANEL_LABEL_BOLD, TRUE,
		       NULL);

  if (do_color) xv_set(feedback,
		       PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_RED,
		       NULL);

  DEBUG(1) (stderr, "Leaving create_control_panel\n");

  return(demo_panel);
}

void
exit_proc(item, event)
    Panel_item item;
    Event      *event;
{
  /*This function gets called when the user is done, so all it has to do
    is destroy the main window, causing the termination of the notifier
    loop; before doing so, we also run the stop_waves stuff*/

    /* if we think a demo is running, stop it*/
    DEBUG(1) (stderr,"Inside of exit_proc.\n");

    if (file_exists(DEMO_RUNNING_FLAG) || file_exists(WTRY_RUNNING_FLAG))
      stop_demo();

    remove_file(ERLDEMO_RUNNING_FLAG);
    remove_file(WAVES_DEMO_FLAG);
    remove_file(SINGLE_DEMO_FLAG);
    remove_file(SLIDE_SHOW_FLAG);
    remove_file(WAVES_DEMO_SCRIPT);
    remove_file(SLIDE_SHOW_SCRIPT);
    remove_file(WTRY_SCRIPT);
    remove_file("/tmp/play");
    remove_file("/tmp/ENTROPIC.log");
    remove_file("/tmp/erldemo.log");
    

    xv_destroy_safe(mainframe);
    return;
}

Notify_value
destroy_func(client, status)
Notify_client client;
Destroy_status status;
{

    DEBUG(1) (stderr,"Inside of destroy_func.\n");
    if (status == DESTROY_CHECKING) {
	}
    else  if (status == DESTROY_CLEANUP) {
    DEBUG(1) (stderr,"Inside of destroy_func. CLEANUP\n");
	if (file_exists(DEMO_RUNNING_FLAG) || file_exists(WTRY_RUNNING_FLAG))
      		stop_demo();

	remove_file(ERLDEMO_RUNNING_FLAG);
        return notify_next_destroy_func(client, status);
	}
    else if (status == DESTROY_SAVE_YOURSELF) {
	}
    else {
    DEBUG(1) (stderr,"Inside of destroy_func. DEATH\n");
	if (file_exists(DEMO_RUNNING_FLAG) || file_exists(WTRY_RUNNING_FLAG))
      		stop_demo();

	remove_file(ERLDEMO_RUNNING_FLAG);
	}
	
    return NOTIFY_DONE;
}

Notify_value
sigint_func(client, sig, when)
Notify_client client;
int sig, when;
{ 
    DEBUG(1) (stderr,"Killed due to signal; cleaning up...\n");
    if (file_exists(DEMO_RUNNING_FLAG) || file_exists(WTRY_RUNNING_FLAG))
      stop_demo();

    remove_file(ERLDEMO_RUNNING_FLAG);

    xv_destroy_safe(mainframe);
    return NOTIFY_DONE;
}

	
void
stop_demo()
{
/* called when EXIT button pressed and a demo or tryout is running */

  if (file_exists(WTRY_RUNNING_FLAG)) {
    /*wtr is running, must quit that by hand*/

    feedback_message("When finished with waves+ tryout, click on QUIT.", 3);
  }
  else {  /*automated demo running, setup for quickest stop*/

    set_flag_file(SINGLE_DEMO_FLAG);

    remove_file(WAVES_DEMO_FLAG);

    remove_file(SLIDE_SHOW_FLAG);

    feedback_message("Demo will stop after waves+ intro or slide show.", 3);

    if (file_exists(WAVES_RUNNING_FLAG)) 
      feedback_message("For faster exit, click on QUIT from waves+.", 3);
  }
  return;
}

void
select_repeat(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  switch (xv_get(item, PANEL_VALUE)) {
  case 0: /*single*/

    if (file_exists(SINGLE_DEMO_FLAG)) /*don't bother resetting*/
      break;
    else {
      set_flag_file(SINGLE_DEMO_FLAG);

      if (file_exists(DEMO_RUNNING_FLAG)) {
	/* reset the "Stop Continuous" button to "Single" */

	xv_set(item, PANEL_CHOICE_STRINGS,
	                "Single",
	                "Continuous",
	                0,
	             NULL);

	feedback_message("Demo will stop after current cycle.", 3);
      }
      if (file_exists(WAVES_RUNNING_FLAG)) 
	feedback_message("For faster stop, click on QUIT from waves+.", 3);
    }
    break;
  case 1: /*continuous*/
    if (!file_exists(SINGLE_DEMO_FLAG)) /*don't bother resetting*/
      break;
    else {
      remove_file(SINGLE_DEMO_FLAG);

      if (file_exists(DEMO_RUNNING_FLAG)) {

	/* reset the repeat button */

	xv_set(item, PANEL_CHOICE_STRINGS,
	                 "Stop Continuous",
	                 "Continuous",
	                 0,
	             NULL);
	feedback_message("Resetting to continuous play.", 3);
      }
    }
    break;
  }
}

void
select_demo(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int item_value, demo_choices;

  item_value = xv_get(item, PANEL_VALUE);
  DEBUG(1) (stderr, "select_repeat: panel value is %d\n", item_value);

  demo_choices = xv_get(item, PANEL_CLIENT_DATA); 

  if (demo_choices != 3) /* if both not allowed */ {
    if (item_value != demo_choices) {
      feedback_message("Sorry, that choice isn't supported on this system.",1);
      xv_set(item, PANEL_VALUE, demo_choices, NULL);
    }
    return;
  }
  switch (item_value) {
  case 1: /*waves+ only*/
    set_flag_file(WAVES_DEMO_FLAG);
    remove_file(SLIDE_SHOW_FLAG);
    break;
  case 2: /*slides only*/
    remove_file(WAVES_DEMO_FLAG);
    set_flag_file(SLIDE_SHOW_FLAG);
    break;
  case 0: /*none set, so reset to both*/
    feedback_message("Must have one or both of live waves+ and slide show.",1);
#ifdef SONY_RISC
    fprintf(stderr,"after case 0 feedback\n");
#endif
    xv_set(item, PANEL_VALUE, 3, NULL);
    /*fall through to next case*/
  case 3: /*both waves+ and slides*/
    set_flag_file(WAVES_DEMO_FLAG);
    set_flag_file(SLIDE_SHOW_FLAG);
    break;
  }
  return;
}

void
set_sparc_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.int"); 
    break;
  case 2:
    set_play_command(item, "play_scripts/play.ext"); 
    break;
  }
}

void
set_sun4_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.vme"); 
    break;
  }
}

void
set_mc_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.mc"); 
    break;
  }
}

void
set_sony_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.sony"); 
    break;
  }
}

void
set_net_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.net"); 
    break;
  }
}

void
set_sgi_output(item, event)
     Panel_item item;		/* parameter panel item */
     Event      *event;
{
  int play_choice = xv_get(item, PANEL_VALUE);
  switch (play_choice) {
  case 0:
    set_play_command(item, "play_scripts/play.none"); 
    break;
  case 1:
    set_play_command(item, "play_scripts/play.sgi"); 
    break;
  }
}

void
start_demo(item, event)
     Panel_item item;
     Event      *event;
{
  Panel_item repeat_button;	/* handle for single/continous button */
  if (run_demo(item) == -1)
    return;

  if (file_exists(SINGLE_DEMO_FLAG)) {
    feedback_message("starting single demo . . . .", 2);
  }
  else {
    /* turn the "Single" button into "Stop Continuous" */

    repeat_button = (Panel_item) xv_get(item, XV_KEY_DATA, REPEAT_BUTTON_KEY);
    xv_set(repeat_button, PANEL_CHOICE_STRINGS,
	                    "Stop Continuous",
	                    "Continuous",
	                    0,
	                    NULL);
    feedback_message("starting continuous demo . . . .", 2);
  }
}

void
try_waves(item, event)
     Panel_item item;
     Event      *event;
{
  char *trywaves_com = (char *) xv_get(item, PANEL_CLIENT_DATA);

  if (trywaves_com == NULL) {  /*no -T option was supplied*/
    feedback_message("Sorry, waves+ tryout is not available on this system.",3);
    return;
  }

  if (run_demo(item) == -1)
    return;

  feedback_message("starting waves+  tryout;  stop with quit from waves+.", 2);

}

int
run_demo(item)
    Panel_item item;
{
  Frame owner;
  char *demo_com = (char *) xv_get(item, PANEL_CLIENT_DATA);
  char demo_command[100];
  int status = 0;
  int x,y;			/* coordinates of panel item */

  owner = (Frame) xv_get((Frame) xv_get(item, XV_OWNER, NULL), 
			 XV_OWNER, 
			 NULL);
  if (file_exists(DEMO_RUNNING_FLAG)) {
    if (file_exists(WAVES_RUNNING_FLAG))
      feedback_message(
	      "waves+ intro already running; if not, exit and restart.",3);
    else if (file_exists(SLIDES_RUNNING_FLAG))
	feedback_message(
	      "slide show already running; if not, exit and restart.",3);
    else
      feedback_message(
	      "Demo already running; if not, please exit and restart.", 3);
    return(-1);
  }

  if (file_exists(WTRY_RUNNING_FLAG)) {
    feedback_message("Interactive waves+ is already running.", 2);
    feedback_message("Click on QUIT from interactive waves+.", 2);
    feedback_message("If it isn't running, please exit and restart.", 3);
    return(-1);
  }
  sprintf(demo_command, "%s &", demo_com);

  DEBUG(1) (stderr, "run_demo: running shell command: %s\n", demo_command);

  status = system(demo_command);

  if (status != 0) {

    x = (int) xv_get(item, PANEL_ITEM_X, NULL);
    y = (int) xv_get(item, PANEL_ITEM_Y, NULL);

    (void)notice_prompt(owner, NULL,
			NOTICE_MESSAGE_STRINGS, "Problem running demo.", NULL,
			NOTICE_FOCUS_XY,        x, y,
			NOTICE_BUTTON_YES,      "Continue",
			NULL);
    return(-1);
  }
  return(0);
}

void
display_info(item, event)
     Panel_item item;
     Event      *event;
{
  Frame owner = (Frame) xv_get((Frame) xv_get(item, XV_OWNER, NULL));

  char *textfile = (char *) xv_get(item, PANEL_CLIENT_DATA);
#ifdef SG
{
  char Command[100]; /* big enough for known command and filename*/
  sprintf(Command, "xterm -T \"More Information About Entropic Software\" -e more %s", textfile);
  exec_command(Command);
}
#else
  display_text(owner, "More Information About Entropic Software", textfile);
#endif
}

void
display_about(item, event)
     Panel_item item;
     Event      *event;
{
  Frame owner = (Frame) xv_get((Frame) xv_get(item, XV_OWNER, NULL));

  char *textfile = (char *) xv_get(item, PANEL_CLIENT_DATA);

#ifdef SG
{
  char Command[100];/* big enough for known command and filename*/
  sprintf(Command, "xterm -T \"About The Entropic Demo\" -e more %s", textfile);
  exec_command(Command);
}
#else
  display_text(owner, "About the Entopic Demo", textfile);
#endif
}

void
display_text(frame, title, file)
Frame frame;			/* owner of popup window */
char *title;			/* popup title */
char *file;			/* popup file */
{
  Frame popup;
  Textsw textw;

  DEBUG(1) (stderr, "Entered display_text, textfile is %s\n", file);

  /*busy out the frame*/

  xv_set(frame, FRAME_BUSY, TRUE, NULL);

  if (!file_exists(file)) {
    feedback_message("Sorry, can't open text file.", 3);
    return;
  }
  popup = xv_create(frame, FRAME_CMD,
		    XV_LABEL, title,
		    XV_SHOW, FALSE,
		    FRAME_SHOW_FOOTER, FALSE,
		    FRAME_SHOW_RESIZE_CORNER, TRUE,
		    FRAME_CMD_PUSHPIN_IN, TRUE,
		    WIN_ROWS,     33,
		    WIN_COLUMNS,  70,
		    0);
  textw = (Textsw) xv_create(popup, TEXTSW,
			     XV_X,         0,
			     XV_Y,         0,
			     OPENWIN_SHOW_BORDERS, TRUE,
			     TEXTSW_FILE, file,
			     TEXTSW_BROWSING, TRUE,
			     TEXTSW_DISABLE_LOAD, TRUE,
			     TEXTSW_DISABLE_CD, TRUE,
			     NULL);
  if (do_color) {

    (void) exv_init_colors(textw);

    xv_set(textw, WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_GRAY,
	   WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
	   NULL);
  }
  xv_set(popup, XV_SHOW, TRUE, NULL);

  xv_set(frame, FRAME_BUSY, FALSE, NULL); 
}

void
feedback_message(message, seconds)
char *message;			/* message text */
int seconds;			/* number of seconds to leave up */
{
  xv_set(feedback, PANEL_LABEL_STRING, message, NULL);

  xv_set(mainframe, FRAME_BUSY, TRUE, NULL); 

  xv_set(mainframe, XV_SHOW, TRUE, NULL);

  sleep((unsigned) seconds); 

  xv_set(mainframe, FRAME_BUSY, FALSE, NULL); 

  xv_set(feedback, PANEL_LABEL_STRING, " ", NULL);
}

int
file_exists(name)
char *name;			/* name of file to check */
{
  FILE *tfd;
  if ((tfd = fopen(name, "r")) == NULL)
    return(0);
  else {
    fclose(tfd);
    return(1);
  }
}

void
set_play_command(item, play_command)
Panel_item item;
char *play_command;
{
/* this function copies the appropriate play_command to /tmp.  The 
 *  item handle is passed so that forced silence can be handled; 
 *  We assument that "None" is always the first choice on the Audio
 *  output list
 */
  int silent_demo = xv_get(item, PANEL_CLIENT_DATA); 
  int play_choice = xv_get(item, PANEL_VALUE); 

  if (silent_demo && play_choice != 0) {
    feedback_message("Sorry, audio output is not available.", 2);
    xv_set(item, PANEL_VALUE, 0, NULL); 
  }
  else {
    link_file(play_command, "/tmp/play");
  }
  return;
}

void
link_file(source, dest)
char *source;
char *dest;
{
  /* we use unlink and cp rather than just cp since a change in userid 
   * since a previous run of erldemo might result in cp failing for 
   * permission reasons
   */
  char path[50];
  char fullpath[100];
  
  sprintf(fullpath, "%s/%s", getwd(path), source); 

  DEBUG(2) (stderr, "link_file: linking %s to %s\n", fullpath, dest);

  (void) unlink(dest); 
  (void) symlink(fullpath, dest); 
}

void
set_flag_file(filename)
char *filename;
{
  /* we use unlink and fopen since a change in userid 
   * since a previous run of erldemo might result in cp failing for 
   * permission reasons
   */
  FILE *tfd;
  DEBUG(2) (stderr, "set_flag_file: touching %s\n", filename);
  (void) unlink(filename);
  tfd = fopen(filename, "w");
  fclose(tfd);
}

void
remove_file(filename)
char *filename;
{
  DEBUG(2) (stderr, "remove_file: removing %s\n", filename);
  (void) unlink(filename);
}

void
sig_handler() {
  /* ok to use NULL dummies, as exit_proc doesn't use the parameters*/
  Panel_item dummy_item = NULL;
  Event      *dummy_event = NULL;
  exit_proc(dummy_item, dummy_event);
}
