/*----------------------------------------------------------------------+
|									
|   This material contains proprietary software of Entropic Speech,	
|   Inc.  Any reproduction, distribution, or publication without the	
|   prior written permission of Entropic Speech, Inc. is strictly	
|   prohibited.  Any public distribution of copies of this work		
|   authorized in writing by Entropic Speech, Inc. must bear the	
|   notice								
|									
|   "Copyright (c) 1988,1991 Entropic Speech, Inc.
|    All rights reserved."	
|									
+-----------------------------------------------------------------------+
|									
|  Program: xacf.c
|
|  User interface for acoustic feature program acf.  Xacf is a menu'd
|  interface for creating parameter files for acf.
|
|  Author: Bill Byrne
|									
+----------------------------------------------------------------------*/
#ifndef lint
  static char *sccs_id = "@(#)xacf.c	1.9 07 Apr 1993 ERL";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>

#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/exview.h>

#include "xacf.h"

#ifndef NULL
#define NULL 0
#endif

/*
#define FCS(item) \
  xv_set(item, \
	 WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_GRAY, \
	 WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,\
	 NULL);
*/
#define FCS(item)

/*
#define PICS(item) \
  xv_set(item, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
*/
#define PICS(item)

#define PCHK(item) \
if (symtype(item)==ST_UNDEF) { \
Fprintf(stderr, "%s: can't get parameter %s.\n", PROG, item); \
exit(1);}

#define Fprintf (void)fprintf
#define SYNTAX {\
Fprintf(stderr, "%s: [-x debug_level] [-P params] acf_params\n", PROG); \
exit(1);}

int debug_level=0;
Attr_attribute INSTANCE;
int do_color=1;
extern optind;
extern char *optarg;
char  *pfile=NULL;       /* destination parameter file */
extern int fullscreendebug;	/* defined in xview library */

int getopt();

static void copysym();
void text_pi_serv();
void shared_text_pi_serv();
void choice_pi_serv();
void flag_pi_serv();
void set_defaults();
void serve_default_button();
static void done();
char **symchoices();
void get_nrows_ncols();
char **symlist();
char **symchoices();
static char *getsymdef_string();
char *get_helpfile();
char *symprompt();
char getsymdef_c();
double getsymdef_d();
char* getsymdef_s();
long strtol();
double strtod();
char *savestring();
static int param_checkwrite(param, strval, newval, parfile);


void main(argc, argv)
     int argc;
     char **argv;
{

  Xv_opaque		main_window_image;

  char  c;
  char  *ipfile="Pxacf";      /* initial parameter file */
  int   rp_ret;            /* read params return val */

  int atoi();

  void create_helpframe();
  void create_frameparams();
  void create_acfopts();

  fullscreendebug = 1;		/* prevents server grabs that crash SGIs */
  xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

  while ( (c=getopt(argc, argv, "x:P:")) != EOF )
    switch (c) {
    case 'x':
      debug_level = (int) atoi(optarg);
      Fprintf(stderr, "%s: debug level %d\n", PROG, debug_level);
      break;
    case 'P':
      ipfile = optarg;
      break;
    default:
      SYNTAX;
    }

  if ( argc != (optind + 1) )
    SYNTAX;
  pfile = savestring(argv[optind++]);
  if ( pfile == NULL ) {
    Fprintf(stderr, "must specifiy output file.\n");
    SYNTAX;
  }

  /* get parameters */
  if (ipfile != NULL)
    rp_ret = read_params(ipfile, SC_NOCOMMON, (char *)NULL);
  else
    rp_ret = read_params(XACFPARAMFILE, SC_NOCOMMON, (char *)NULL);
  symerr_exit();
  if ( rp_ret < 0 ) {
    Fprintf(stderr, "%s: can't read parameter file %s.\n",
	    PROG, ipfile);
    exit(1);
  }

  xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, 0);
  INSTANCE = xv_unique_key();
  EXVK_FIELD_NAME = xv_unique_key();

  main_window = xv_create((Xv_opaque) NULL, FRAME,
		XV_KEY_DATA, INSTANCE, (Xv_opaque) NULL,
		XV_WIDTH, 632,
		XV_HEIGHT, 770,
		XV_LABEL, "xacf: acoustic feature parameter maker",
		FRAME_CLOSED, FALSE,
		FRAME_SHOW_FOOTER, TRUE,
		FRAME_SHOW_RESIZE_CORNER, TRUE,
		NULL);
/*  e_cms = exv_init_colors(main_window); */
  create_helpframe(main_window);
  create_frameparams(main_window);
  create_acfopts(main_window);
  (void) exv_attach_icon( main_window, ERL_NOBORD_ICON, "xacf", TRANSPARENT);

  set_defaults();

  xv_main_loop(main_window);

  exit(0);

}

void
create_helpframe(main_window)
     Xv_opaque main_window;
{
  caddr_t ip = NULL;

  helpframe = xv_create(main_window, PANEL,
			XV_KEY_DATA, INSTANCE, ip,
			XV_X, 0,
			XV_Y, 0,
			XV_WIDTH, WIN_EXTEND_TO_EDGE,
			XV_HEIGHT, 28,
			WIN_BORDER, TRUE,
			NULL);
  FCS(helpframe);

  man = xv_create(helpframe, PANEL_BUTTON,
			XV_KEY_DATA, INSTANCE, ip,
			XV_KEY_DATA, EXVK_HELP_TITLE, "xacf: acf man page",
			XV_KEY_DATA, EXVK_HELP_NAME, "acf",
			PANEL_NOTIFY_PROC, exv_get_help,
			XV_X, 280,
			XV_Y, 4,
			XV_WIDTH, 109,
			XV_HEIGHT, 19,
			PANEL_LABEL_STRING, "Acf Man Page...",
		        PANEL_NOTIFY_PROC, exv_get_help,
			NULL);
  PICS(man);

  help = xv_create(helpframe, PANEL_BUTTON,
		   XV_KEY_DATA, INSTANCE, ip,
		   XV_X, 408,
		   XV_Y, 4,
		   XV_WIDTH, 55,
		   XV_HEIGHT, 19,
		   XV_KEY_DATA, EXVK_HELP_TITLE, "xacf: xacf man page",
		   XV_KEY_DATA, EXVK_HELP_NAME, "xacf",
		   PANEL_NOTIFY_PROC, exv_get_help,
		   PANEL_LABEL_STRING, "Help...",
		   NULL);
  PICS(help);

#if !defined(M5600)
  defaults = xv_create(helpframe, PANEL_BUTTON,
		       XV_KEY_DATA, INSTANCE, ip,
		       XV_X, 480,
		       XV_Y, 4,
		       XV_WIDTH, 130,
		       XV_HEIGHT, 19,
		       PANEL_LABEL_STRING, "Set Default Values",
		       PANEL_NOTIFY_PROC, serve_default_button,
		       NULL);
  PICS(defaults);
#endif

  quit = xv_create(helpframe, PANEL_BUTTON,
		   XV_KEY_DATA, INSTANCE, ip,
		   XV_X, 8,
		   XV_Y, 4,
		   XV_WIDTH, 55,
		   XV_HEIGHT, 19,
		   PANEL_NOTIFY_PROC, done,
		   PANEL_LABEL_STRING, "Done",
		   NULL);
  PICS(quit);

  return;
}


static
void
done(item, event)
     Panel_item item;
     Event      *event;
{
  /*This function gets called when the user is done, so all it has to do
    is destroy the main window, causing the termination of the notifier
    loop; note that this is ok since the panel notify procedures do the
    parameter-file writing directly; hence the file is up to date when
    done is called.
    */
/*
  Frame frame = xv_get(item, PANEL_CLIENT_DATA);
  xv_destroy_safe(frame); */
  xv_destroy_safe(main_window);
  return;
}


void
serve_default_button(item, event)
Panel_item item;
Event event;
{
  set_defaults();
}

void
create_frameparams(main_window)
     Xv_opaque main_window;
{
  caddr_t ip = NULL;

  frameparams = xv_create(main_window, PANEL,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 0,
		XV_Y, 24,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, 131,
		WIN_BORDER, TRUE,
		NULL);
  FCS(frameparams);

  frameparamsmsg = xv_create(frameparams, PANEL_MESSAGE,
			     XV_KEY_DATA, INSTANCE, ip,
			     XV_X, 8,
			     XV_Y, 8,
			     XV_WIDTH, 125,
			     XV_HEIGHT, 13,
			     PANEL_LABEL_STRING, "Frame Parameters",
			     PANEL_LABEL_BOLD, TRUE,
			     NULL);
  PICS(frameparamsmsg);

  sdfieldname = xv_create(frameparams, PANEL_TEXT,
			  XV_KEY_DATA, INSTANCE, ip,
			  XV_KEY_DATA, EXVK_FIELD_NAME, savestring("sd_field_name"),
			  XV_X, 20,
			  XV_Y, 32,
			  XV_WIDTH, 249,
			  XV_HEIGHT, 15,
			  PANEL_LABEL_STRING, "Input Field Name:",
			  PANEL_VALUE_X, 149,
			  PANEL_VALUE_Y, 32,
			  PANEL_LAYOUT, PANEL_HORIZONTAL,
			  PANEL_VALUE_DISPLAY_LENGTH, 15,
			  PANEL_VALUE_STORED_LENGTH, 80,
			  PANEL_READ_ONLY, FALSE,
			  PANEL_NOTIFY_PROC, text_pi_serv,
			  PANEL_VALUE, getsymdef_string("sd_field_name"),
			  NULL);

  window = xv_create(frameparams, PANEL_CHOICE, PANEL_DISPLAY_LEVEL, PANEL_CURRENT,
		     XV_KEY_DATA, INSTANCE, ip,
		     XV_KEY_DATA, EXVK_FIELD_NAME, savestring("window_type"),
		     XV_X, 84,
		     XV_Y, 76,
		     XV_WIDTH, 190,
		     XV_HEIGHT, 23,
		     PANEL_VALUE_X, 149,
		     PANEL_VALUE_Y, 76,
		     PANEL_LAYOUT, PANEL_HORIZONTAL,
		     PANEL_CHOICE_NROWS, 1,
		     PANEL_LABEL_STRING, "Window:",
		     PANEL_CHOICE_STRINGS,
		     "RECT",
		     "HAMMING",
		     "HANNING",
		     "TRIANG",
		     "COS4",
		     NULL,
		     PANEL_NOTIFY_PROC, choice_pi_serv,
		     NULL);
  PICS(window);

  preemp = xv_create(frameparams, PANEL_TEXT,
		     XV_KEY_DATA, INSTANCE, ip,
		     XV_KEY_DATA, EXVK_FIELD_NAME, savestring("preemphasis"),
		     XV_X, 48,
		     XV_Y, 56,
		     XV_WIDTH, 166,
		     XV_HEIGHT, 15,
		     PANEL_LABEL_STRING, "Preemphasis:",
		     PANEL_VALUE_X, 150,
		     PANEL_VALUE_Y, 56,
		     PANEL_LAYOUT, PANEL_HORIZONTAL,
		     PANEL_VALUE_DISPLAY_LENGTH, 8,
		     PANEL_VALUE_STORED_LENGTH, 80,
		     PANEL_READ_ONLY, FALSE,
		     PANEL_NOTIFY_PROC, text_pi_serv,
		     PANEL_VALUE, getsymdef_string("preemphasis"),
		     NULL);
  PICS(preemp);

  units = xv_create(frameparams, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("units"),
		XV_X, 320,
		XV_Y, 24,
		XV_WIDTH, 241,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 429,
		PANEL_VALUE_Y, 24,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Framing Units:",
		PANEL_CHOICE_STRINGS,
			"samples",
			"seconds",
		        NULL,
		PANEL_NOTIFY_PROC, choice_pi_serv,
		NULL);


  framelen = xv_create(frameparams, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("frame_len"),
		XV_X, 324,
		XV_Y, 56,
		XV_WIDTH, 169,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Frame Length:",
		PANEL_VALUE_X, 429,
		PANEL_VALUE_Y, 56,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("frame_len"),
		NULL);

  start = xv_create(frameparams, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("start"),
		XV_X, 515,
		XV_Y, 56,
		XV_WIDTH, 108,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Start:",
		PANEL_VALUE_X, 559,
		PANEL_VALUE_Y, 56,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("start"),
		NULL);
  PICS(start);

  step = xv_create(frameparams, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("step"),
		XV_X, 388,
		XV_Y, 80,
		XV_WIDTH, 106,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Step:",
		PANEL_VALUE_X, 430,
		PANEL_VALUE_Y, 80,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("step"),
		NULL);
  PICS(step);

  nan = xv_create(frameparams, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("nan"),
		XV_X, 521,
		XV_Y, 80,
		XV_WIDTH, 103,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Nan:",
		PANEL_VALUE_X, 560,
		PANEL_VALUE_Y, 80,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("nan"),
		NULL);
    PICS(nan);

  sd_flag = xv_create(frameparams, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("sd_flag"),
		XV_X, 24,
		XV_Y, 100,
		XV_WIDTH, 270,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 225,
		PANEL_VALUE_Y, 100,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Store windowed data frame:",
		PANEL_NOTIFY_PROC, flag_pi_serv,
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		NULL);

  sd_fname = xv_create(frameparams, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("sd_fname"),
		XV_X, 348,
		XV_Y, 108,
		XV_WIDTH, 152,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 104,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("sd_fname"),
		NULL);



}

void create_acfopts(main_window)
     Xv_opaque main_window;

{
  caddr_t ip = NULL;

  acfopts = xv_create(main_window, PANEL,
		      XV_KEY_DATA, INSTANCE, ip,
		      XV_X, 0,
		      XV_Y, 152,
		      XV_WIDTH, WIN_EXTEND_TO_EDGE,
		      XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		      WIN_BORDER, TRUE,
		      NULL);
  FCS(acfopts);

  acfmsg = xv_create(acfopts, PANEL_MESSAGE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 8,
		XV_Y, 8,
		XV_WIDTH, 196,
		XV_HEIGHT, 13,
		PANEL_LABEL_STRING, "Acoustic Feature Parameters",
		PANEL_LABEL_BOLD, TRUE,
		NULL);

  pwr_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("pwr_flag"),
		XV_X, 170,
		XV_Y, 32,
		XV_WIDTH, 126,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 32,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Power:",
		PANEL_NOTIFY_PROC, flag_pi_serv,
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		NULL);

  pwr_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("pwr_fname"),
		XV_X, 348,
		XV_Y, 32,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 32,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("pwr_fname"),
		NULL);


  zc_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("zc_flag"),
		XV_X, 108,
		XV_Y, 64,
		XV_WIDTH, 188,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 64,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Zero Crossings:",
		PANEL_NOTIFY_PROC, flag_pi_serv,
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		NULL);

  zc_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("zc_fname"),
		XV_X, 348,
		XV_Y, 64,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 64,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("zc_fname"),
		NULL);

  ac_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("ac_flag"),
		XV_X, 99,
		XV_Y, 96,
		XV_WIDTH, 197,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 96,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Auto Correlation:",
		PANEL_NOTIFY_PROC, flag_pi_serv,
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		NULL);

  ac_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("ac_fname"),
		XV_X, 348,
		XV_Y, 96,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 96,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("ac_fname"),
		NULL);

  ac_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("ac_order"),
		XV_X, 348,
		XV_Y, 120,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 120,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("ac_order"),
		NULL);

  rc_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
                XV_KEY_DATA, EXVK_FIELD_NAME, savestring("rc_flag"),
		XV_X, 53,
		XV_Y, 144,
		XV_WIDTH, 243,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 144,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Reflection Coefficients:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);
  rc_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("rc_fname"),
		XV_X, 348,
		XV_Y, 144,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 144,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("rc_fname"),
		NULL);


  rc_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("rc_order"),
		XV_X, 348,
		XV_Y, 168,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 168,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("ac_order"),
		NULL);

  lpc_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpc_flag"),
		XV_X, 6,
		XV_Y, 196,
		XV_WIDTH, 290,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 196,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Linear Prediction Coefficients:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  lpc_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpc_fname"),
		XV_X, 348,
		XV_Y, 196,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 196,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("lpc_fname"),
		NULL);

  lpc_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpc_order"),
		XV_X, 348,
		XV_Y, 216,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 216,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("ac_order"),
		NULL);

  lar_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
                XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lar_flag"),
		XV_X, 105,
		XV_Y, 312,
		XV_WIDTH, 191,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 312,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Log Area Ratios:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  lar_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lar_fname"),
		XV_X, 348,
		XV_Y, 312,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 312,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("lar_fname"),
		NULL);

  lar_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lar_order"),
		XV_X, 348,
		XV_Y, 332,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 332,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
		PANEL_VALUE, getsymdef_string("ac_order"),
		NULL);

  lsf_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lsf_flag"),
		XV_X, 34,
		XV_Y, 244,
		XV_WIDTH, 262,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 244,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "Line Spectral Frequencies:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  lsf_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lsf_fname"),
		XV_X, 348,
		XV_Y, 244,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 244,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("lsf_fname"),
		NULL);

  lsf_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lsf_order"),
		XV_X, 348,
		XV_Y, 264,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 264,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("ac_order"),
		NULL);

  lsf_freq_res = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lsf_freq_res"),
		XV_X, 348,
		XV_Y, 284,
		XV_WIDTH, 150,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Resolution:",
		PANEL_VALUE_X, 434,
		PANEL_VALUE_Y, 284,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                getsymdef_string("lsf_freq_res"),
		NULL);

  lpccep_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpccep_flag"),
		XV_X, 115,
		XV_Y, 368,
		XV_WIDTH, 181,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 368,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "LPC Cepstrum:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  lpccep_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpccep_fname"),
		XV_X, 348,
		XV_Y, 368,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 436,
		PANEL_VALUE_Y, 368,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("lpccep_fname"),
		NULL);

  lpccep_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpccep_order"),
		XV_X, 348,
		XV_Y, 412,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 399,
		PANEL_VALUE_Y, 412,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
                PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("lpccep_order"),
		NULL);

  lpccep_deriv = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("lpccep_deriv"),
		XV_X, 348,
		XV_Y, 392,
		XV_WIDTH, 273,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Element Subrange:",
		PANEL_VALUE_X, 485,
		PANEL_VALUE_Y, 392,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 17,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
                PANEL_NOTIFY_PROC, text_pi_serv,
		PANEL_VALUE, getsymdef_string("lpccep_deriv"),
		NULL);

  lpccep_warp = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("warp_param"),
		XV_X, 348,
		XV_Y, 432,
		XV_WIDTH, 207,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Warping Parameter:",
		PANEL_VALUE_X, 491,
		PANEL_VALUE_Y, 432,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
                PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("warp_param"),
		NULL);

  fftcep_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fftcep_flag"),
		XV_X, 116,
		XV_Y, 460,
		XV_WIDTH, 179,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 223,
		PANEL_VALUE_Y, 460,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "FFT Cepstrum:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  fftcep_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fftcep_fname"),
		XV_X, 347,
		XV_Y, 460,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Field Name:",
		PANEL_VALUE_X, 435,
		PANEL_VALUE_Y, 460,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("fftcep_fname"),
		NULL);

  fftcep_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fftcep_order"),
		XV_X, 347,
		XV_Y, 480,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 398,
		PANEL_VALUE_Y, 480,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("fftcep_order"),
		NULL);

  fftcep_deriv = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fftcep_deriv"),
		XV_X, 347,
		XV_Y, 500,
		XV_WIDTH, 273,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Element Subrange:",
		PANEL_VALUE_X, 484,
		PANEL_VALUE_Y, 500,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 17,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("fftcep_deriv"),
		NULL);

  fftcep_warp = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("warp_param"),
		XV_X, 347,
		XV_Y, 524,
		XV_WIDTH, 207,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Warping Parameter:",
		PANEL_VALUE_X, 490,
		PANEL_VALUE_Y, 524,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, shared_text_pi_serv,
                PANEL_VALUE, getsymdef_string("warp_param"),
		NULL);

  fft_flag = xv_create(acfopts, PANEL_CHOICE,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fft_flag"),
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 187,
		XV_Y, 556,
		XV_WIDTH, 108,
		XV_HEIGHT, 23,
		PANEL_VALUE_X, 222,
		PANEL_VALUE_Y, 556,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LABEL_STRING, "FFT:",
		PANEL_CHOICE_STRINGS,
			"NO",
			"YES",
			0,
		PANEL_NOTIFY_PROC, flag_pi_serv,
		NULL);

  fft_order = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_KEY_DATA, EXVK_FIELD_NAME, savestring("fft_order"),
		XV_X, 347,
		XV_Y, 580,
		XV_WIDTH, 115,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, "Order:",
		PANEL_VALUE_X, 398,
		PANEL_VALUE_Y, 580,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, text_pi_serv,
                PANEL_VALUE, getsymdef_string("fft_order"),
		NULL);

  fft_fname = xv_create(acfopts, PANEL_TEXT,
		XV_KEY_DATA, INSTANCE, ip,
		XV_X, 347,
		XV_Y, 556,
		XV_WIDTH, 272,
		XV_HEIGHT, 15,
		PANEL_LABEL_STRING, savestring("Field Name:"),
		PANEL_VALUE_X, 435,
		PANEL_VALUE_Y, 556,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE_DISPLAY_LENGTH, 23,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_VALUE, "re_spec_val",
		PANEL_READ_ONLY, TRUE,
		NULL);
}




void set_defaults()
{

#if defined(M5600)
/* disable xv_set */
#define lxv_set( x, y, z)
#else
#define lxv_set( x, y, z) xv_set(x,y,z)
#endif

  PCHK("sd_field_name");
  lxv_set(sdfieldname, PANEL_VALUE, getsymdef_string("sd_field_name"));
  copysym( "sd_field_name", pfile);

  PCHK("preemphasis");
  lxv_set(preemp, PANEL_VALUE, getsymdef_string("preemphasis"));
  copysym( "preemphasis", pfile);

  PCHK("frame_len");
  lxv_set( framelen, PANEL_VALUE, getsymdef_string("frame_len"));
  copysym( "frame_len", pfile);

  PCHK("start");
  lxv_set( start, PANEL_VALUE, getsymdef_string("start"));
  copysym( "start", pfile);

  PCHK("step");
  lxv_set( step, PANEL_VALUE, getsymdef_string("step"));
  copysym( "step", pfile);

  PCHK("nan");
  lxv_set( nan, PANEL_VALUE, getsymdef_string("nan"));
  copysym( "nan", pfile);

  PCHK("window_type");
  lxv_set( window, PANEL_VALUE, 0);
  copysym( "window_type", pfile);

  PCHK("units");
  lxv_set( units, PANEL_VALUE, 0);
  copysym( "units", pfile);

  PCHK("sd_flag");
  lxv_set( sd_flag, PANEL_VALUE, 0);
  copysym( "sd_flag", pfile);

  PCHK("sd_fname");
  lxv_set(sd_fname, PANEL_VALUE, getsymdef_string("sd_fname"));
  copysym( "sd_fname", pfile);

  PCHK("pwr_flag");
  lxv_set( pwr_flag, PANEL_VALUE, 0);
  copysym( "pwr_flag", pfile);

  PCHK("pwr_fname");
  lxv_set(pwr_fname, PANEL_VALUE, getsymdef_string("pwr_fname"));
  copysym( "pwr_fname", pfile);

  PCHK("zc_flag");
  lxv_set( zc_flag, PANEL_VALUE, 0);
  copysym( "zc_flag", pfile);

  PCHK("zc_fname");
  lxv_set( zc_fname, PANEL_VALUE, getsymdef_string("zc_fname"));
  copysym( "zc_fname", pfile);

  PCHK("ac_flag");
  lxv_set( ac_flag, PANEL_VALUE, 0);
  copysym( "ac_flag", pfile);

  PCHK("ac_fname");
  lxv_set( ac_fname, PANEL_VALUE, getsymdef_string("ac_fname"));
  copysym( "ac_fname", pfile);

  PCHK("ac_order");
  lxv_set(ac_order, PANEL_VALUE, getsymdef_string("ac_order"));
  lxv_set(rc_order, PANEL_VALUE, getsymdef_string("ac_order"));
  lxv_set(lpc_order, PANEL_VALUE, getsymdef_string("ac_order"));
  lxv_set(lar_order, PANEL_VALUE, getsymdef_string("ac_order"));
  lxv_set(lsf_order, PANEL_VALUE, getsymdef_string("ac_order"));
  copysym( "ac_order", pfile);

  PCHK("rc_flag");
  lxv_set( rc_flag, PANEL_VALUE, 0);
  copysym( "rc_flag", pfile);

  PCHK("rc_fname");
  lxv_set( rc_fname, PANEL_VALUE, getsymdef_string("rc_fname"));
  copysym( "rc_fname", pfile);

  PCHK("lpc_flag");
  lxv_set( lpc_flag, PANEL_VALUE, 0);
  copysym( "lpc_flag", pfile);

  PCHK("lpc_fname");
  lxv_set( lpc_fname, PANEL_VALUE, getsymdef_string("lpc_fname"));
  copysym( "lpc_fname", pfile);

  PCHK("lar_flag");
  lxv_set( lar_flag, PANEL_VALUE, 0);
  copysym( "lar_flag", pfile);

  PCHK("lar_fname");
  lxv_set( lar_fname, PANEL_VALUE, getsymdef_string("lar_fname"));
  copysym( "lar_fname", pfile);

  PCHK("lsf_flag");
  lxv_set( lsf_flag, PANEL_VALUE, 0);
  copysym( "lsf_flag", pfile);

  PCHK("lsf_fname");
  lxv_set( lsf_fname, PANEL_VALUE, getsymdef_string("lsf_fname"));
  copysym( "lsf_fname", pfile);

  PCHK("lsf_freq_res");
  lxv_set( lsf_freq_res, PANEL_VALUE, getsymdef_string("lsf_freq_res"));
  copysym( "lsf_freq_res", pfile);

  PCHK("lpccep_flag");
  lxv_set( lpccep_flag, PANEL_VALUE, 0);
  copysym( "lpccep_flag", pfile);

  PCHK("lpccep_fname");
  lxv_set( lpccep_fname, PANEL_VALUE, getsymdef_string("lpccep_fname"));
  copysym( "lpccep_fname", pfile);

  PCHK("lpccep_order");
  lxv_set( lpccep_order, PANEL_VALUE, getsymdef_string("lpccep_order"));
  copysym( "lpccep_order", pfile);

  PCHK("lpccep_deriv");
  lxv_set( lpccep_deriv, PANEL_VALUE, getsymdef_string("lpccep_deriv"));
  copysym( "lpccep_deriv", pfile);

  PCHK("warp_param");
  lxv_set( lpccep_warp, PANEL_VALUE, getsymdef_string("warp_param"));
  lxv_set( fftcep_warp, PANEL_VALUE, getsymdef_string("warp_param"));
  copysym( "warp_param", pfile);

  PCHK("fftcep_flag");
  lxv_set( fftcep_flag, PANEL_VALUE, 0);
  copysym("fftcep_flag", pfile);

  PCHK("fftcep_fname");
  lxv_set( fftcep_fname, PANEL_VALUE, getsymdef_string("fftcep_fname"));
  copysym( "fftcep_fname", pfile);

  PCHK("fftcep_order");
  lxv_set( fftcep_order, PANEL_VALUE, getsymdef_string("fftcep_order"));
  copysym( "fftcep_order", pfile);

  PCHK("fftcep_deriv");
  lxv_set( fftcep_deriv, PANEL_VALUE, getsymdef_string("fftcep_deriv"));
  copysym( "fftcep_deriv", pfile);

  PCHK("fft_flag");
  lxv_set( fft_flag, PANEL_VALUE, 0);
  copysym( "fft_flag", pfile);

  PCHK("fft_order");
  lxv_set( fft_order, PANEL_VALUE, getsymdef_string("fft_order"));
  copysym( "fft_order", pfile);

  return;
}


static
void
copysym(param, file)
char *param;			/* name of parameter */
char *file;			/* output parameter file */
{
  /*reads default value from parameter file and copies to named file*/

  int ptype;			/* data type of parameter */

  ptype = symtype(param);

  switch (ptype) {
  case ST_INT:
    fputsym_i(param, getsymdef_i(param), file);
    break;
  case ST_CHAR:
    Fprintf(stderr, "copysym: can't putsym for CHAR type yet\n");
/*    fputsym_c(param, getsymdef_c(param), file);*/
    break;
  case ST_STRING:
    fputsym_s(param, getsymdef_s(param), file);
    break;
  case ST_FLOAT:
    fputsym_f(param, (float) getsymdef_d(param), file);
    break;
  case ST_IARRAY:
  case ST_FARRAY:
    Fprintf(stderr, "copysym: parameter type not supported yet\n");
    break;
  case ST_UNDEF:
    Fprintf(stderr, "copysym: undefined parameter %s\n", param);
    break;
  }
  return;
}


static
char *
getsymdef_string(param)
char *param;
{
  /* return default value as a string */
  int ptype;			/* data type of parameter */
  char *retstr;			/* actual return string */
  static char strval[100];		/* space to construct return value */
  double d_defval;

  ptype = symtype(param);

  switch (ptype) {
  case ST_INT:
    sprintf(strval, "%d", getsymdef_i(param));
    retstr = savestring(strval);
    break;
  case ST_CHAR:
    sprintf(strval, "%c", getsymdef_c(param));
    retstr = savestring(strval);
    break;
  case ST_STRING:
    retstr = savestring(getsymdef_s(param));
    break;
  case ST_FLOAT:
    d_defval = getsymdef_d(param);
    if ( d_defval == 0.0 )
      retstr = savestring("0.0");
    else {
      sprintf(strval, "%f", (float) getsymdef_d(param));
      retstr = savestring(strval);
    }
    break;
  case ST_IARRAY:
    Fprintf(stderr, "getsymdef_string: ST_IARRAY not supported yet\n");
    retstr = NULL;
    break;
  case ST_FARRAY:
    Fprintf(stderr, "getsymdef_string: ST_FARRAY not supported yet\n");
    retstr = NULL;
    break;
  case ST_UNDEF:
    Fprintf(stderr, "getsymdef_string: undefined parameter %s\n", param);
    retstr = NULL;
    break;
  }

  if (retstr == NULL)
    Fprintf(stderr,
	    "getsymdef_string: bad parameter, bad type, or couldn't allocate memory for string\n");
  return(retstr);
}

void text_pi_serv(pi, event)
Panel_item pi;
Event *event;
{
  int pc_val;
  char *correct_val, *ff, *pv, *wv;

  ff = (char *) xv_get(pi, XV_KEY_DATA, EXVK_FIELD_NAME);
  pv = (char *) xv_get(pi, PANEL_VALUE);
  wv = savestring(pv);
  pc_val = param_checkwrite( ff, wv, &correct_val, pfile);
  if (pc_val == PARAM_FORMAT)
    xv_set(pi, PANEL_VALUE, correct_val, NULL);
  free(wv);

  return;
}

void shared_text_pi_serv(pi, event)
Panel_item pi;
Event *event;
{
  int pc_val;
  char *correct_val, *pv;

  pv = savestring( (char *) xv_get(pi, PANEL_VALUE) );
  if ( pv == NULL ) {
    fprintf(stderr, "%s: panel value NULL.\n", PROG);
    return;
  }

  if ( pi == fftcep_warp || pi == lpccep_warp ) {
    pc_val = param_checkwrite( "warp_param", pv, &correct_val, pfile);
    if (pc_val == PARAM_FORMAT) {
      xv_set(fftcep_warp, PANEL_VALUE, correct_val, NULL);
      xv_set(lpccep_warp, PANEL_VALUE, correct_val, NULL);
    }
    else {
      xv_set(fftcep_warp, PANEL_VALUE, pv, NULL);
      xv_set(lpccep_warp, PANEL_VALUE, pv, NULL);
    }
  }

  if ( pi == ac_order || pi == lpc_order || pi == lsf_order
      || pi == lar_order || pi == rc_order) {
    pc_val = param_checkwrite( "ac_order", pv, &correct_val, pfile);
    if (pc_val == PARAM_FORMAT) {
      xv_set( ac_order, PANEL_VALUE, correct_val, NULL);
      xv_set( lpc_order, PANEL_VALUE, correct_val, NULL);
      xv_set( lsf_order, PANEL_VALUE, correct_val, NULL);
      xv_set( lar_order, PANEL_VALUE, correct_val, NULL);
      xv_set( rc_order, PANEL_VALUE, correct_val, NULL);
    }
    else {
      xv_set( ac_order, PANEL_VALUE, pv, NULL);
      xv_set( lpc_order, PANEL_VALUE, pv, NULL);
      xv_set( lsf_order, PANEL_VALUE, pv, NULL);
      xv_set( lar_order, PANEL_VALUE, pv, NULL);
      xv_set( rc_order, PANEL_VALUE, pv, NULL);
    }
  }

  free(pv);

  return;
}

void choice_pi_serv(pi, value, event)
Panel_item pi;
int value;
Event *event;
{
  int pc_val, pv;
  char *correct_val, *ff;
  char *choice_val=NULL;

  ff = (char *) xv_get(pi, XV_KEY_DATA, EXVK_FIELD_NAME);

  pv = (int) xv_get(pi, PANEL_VALUE);

  choice_val = (char *) xv_get( pi, PANEL_CHOICE_STRING, pv);

  pc_val = param_checkwrite( ff, choice_val, &correct_val, pfile);
  if (pc_val == PARAM_FORMAT)
    xv_set(pi, PANEL_VALUE, 0, NULL);
  return;
}


void flag_pi_serv(pi, value, event)
Panel_item pi;
int value;
Event *event;
{
  int pc_val;
  char *correct_val, *ff;

  ff = (char *) xv_get(pi, XV_KEY_DATA, EXVK_FIELD_NAME);
  spsassert( value == 0 || value == 1, "bad value from flag choice.");
  pc_val = fputsym_i(ff, value, pfile);
  if (pc_val == -1)
    xv_set(pi, PANEL_VALUE, 0, NULL);
  return;
}

static
int
param_checkwrite(param, strval, newval, parfile)
char *param;			/* parameter name */
char *strval;			/* string value of parameter */
char **newval;			/* pointer to new value when limit error */
char *parfile;			/* parameter file name */
{
  /*convert string parameter value to proper type, check for format
   and limit errors, and write the parameter file if ok; if the input
   value strval is out of bounds or has a bad format, the value used
   is returned via newval*/

  float pmin, pmax;		/* min and max values for parameter */
  int ret;			/* return code for this function*/
  int putsymret;		/* return code from putsym calls */
  int ptype;			/* parameter data type */
  char *lastchar;		/* last scanned character */
  char *sval;			
  char cval;
  double dval;
  int ival;

  /*return PAR_FORMAT if conversion error*/
  /*if limits, check them and return appropriate codes*/
  /*if all goes well, write the parameter (where?)*/

  ptype = symtype(param);

  /* convert string to proper type and check for format errors*/

  switch (ptype) {
  case ST_INT:
    ival = (int) strtol(strval, &lastchar, 10);
    if (*lastchar != '\0') {
      ret = PARAM_FORMAT;
      ival = getsymdef_i(param);
      *newval = getsymdef_string(param);
    }
    else {
      ret = PARAM_OK;
      *newval = strval;
    }
    break;
  case ST_CHAR:
    cval = strval[0];
    if (strlen(strval) != 1) {
      ret = PARAM_FORMAT;
      cval = getsymdef_c(param);
      *newval = getsymdef_string(param);
    }
    else {
      ret = PARAM_OK;
      *newval = strval;
    }
    break;
  case ST_STRING:
    sval = strval;
    *newval = strval;
    ret = PARAM_OK;
    break;
  case ST_FLOAT:
    dval = strtod(strval, &lastchar);
    if (*lastchar != '\0') {
      ret = PARAM_FORMAT;
      dval = getsymdef_d(param);
      *newval = getsymdef_string(param);
    }
    else {
      ret = PARAM_OK;
      *newval = strval;
    }
    break;
  case ST_IARRAY:
  case ST_FARRAY:
    Fprintf(stderr, "param_checkwrite: type not supported yet\n");
    ret = PARAM_FORMAT;
    break;
  case ST_UNDEF:
    Fprintf(stderr, "param_checkwrite: undefined parameter %s\n", param);
    return(PARAM_FORMAT);
  default:
    Fprintf(stderr, "param_checkwrite: unsupported parameter type\n");
    break;
  }

  /* now check the limits (if any) for numeric types*/

  if (ptype == ST_INT) {
    if (symrange(param, &pmin, &pmax)) {
      if (ival < pmin) {
	ival = pmin;
	*newval = (char *) calloc((unsigned) 25, sizeof (char));
	sprintf(*newval, "%d", ival);
	ret = PARAM_MIN;
      }
      if (ival > pmax) {
	ival = pmax;
	*newval = (char *) calloc((unsigned) 25, sizeof (char));
	sprintf(*newval, "%d", ival);
	ret = PARAM_MAX;
      }
    }
  }

  if (ptype == ST_FLOAT) {
    if (symrange(param, &pmin, &pmax)) {
      if (dval < pmin) {
	dval = pmin;
	*newval = (char *) calloc((unsigned) 25, sizeof (char));
	sprintf(*newval, "%g", dval);
	ret = PARAM_MIN;
      }
      if (dval > pmax) {
	dval = pmax;
	*newval = (char *) calloc((unsigned) 25, sizeof (char));
	sprintf(*newval, "%g", dval);
	ret = PARAM_MAX;
      }
    }
  }

  if (debug_level > 1) {
    switch(ret) {
    case PARAM_MAX:
      Fprintf(stderr, "param_checkwrite: value greater than max value\n");
      break;
    case PARAM_MIN:
      Fprintf(stderr, "param_checkwrite: value less than min value\n");
      break;
    case PARAM_FORMAT:
      Fprintf(stderr, "param_checkwrite: parameter format error\n");
      break;
    case PARAM_OK:
      Fprintf(stderr, "param_checkwrite: parameter value OK\n");
      break;
    }
  }

  if (debug_level > 1)
    Fprintf(stderr, "param_checkwrite: writing value %s for %s\n",
	    *newval, param);

  switch (ptype) {
  case ST_INT:
    putsymret = fputsym_i(param, ival, parfile);
    break;
  case ST_CHAR:
    Fprintf(stderr, "param_checkwrite: can't putsym for CHAR type yet\n");
/*    putsymret = fputsym_c(param, cval, parfile);*/
    break;
  case ST_STRING:
    putsymret = fputsym_s(param, sval, parfile);
    break;
  case ST_FLOAT:
    putsymret = fputsym_f(param, (float) dval, parfile);
    break;
  case ST_IARRAY:
  case ST_FARRAY:
  case ST_UNDEF:
  default:
    putsymret = -1;
    Fprintf(stderr,
	    "param_checkwrite: undefined or unsupported parameter type\n");
    break;
  }

  if (putsymret == -1)
    Fprintf(stderr,
	    "param_checkwrite: trouble writing parameter %s\n", param);

  return(ret);
}
