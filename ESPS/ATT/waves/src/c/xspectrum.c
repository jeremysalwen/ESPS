/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  David Talkin, ATT
 * Checked by:
 * Revised by:  Rod Johnson, John Shore, Alan Parker, David Talkin ERL
 *
 * FURTHER MODIFIED BY:
 *              LEE HETHERINGTON, MIT-LCS-SLS
 *
 * Modified a LOT MORE by:
 *              David Talkin, ERL
 *
 *  xspectrum.c
 *  a spectrum analysis and inverse filter attachment for "xwaves"
 */

static char *sccs_id = "@(#)xspectrum.c	1.37	1/18/97	ATT/ESI/ERL";

/* This program, apart from providing some useful functions, serves as a
   reasonably well documented example of an attachment for the "xwaves"
   program.  See $WAVES_DOC/attachments.help for a general discussion.

   Spectrum computes and displays log magnitude spectra (of time series
   and inverse LPC models).  Optionally, a reference spectrum may be saved
   for comparison purposes and the (possibly integrated) residual
   resulting from the application of an inverse LPC model may be computed
   and displayed (see the section on "spectrum" in
   $WAVES_DOC/attachments.help).
 */

#include <stdio.h>
#include <xview/font.h>
#if !defined(APOLLO_68K) && !defined(DS3100) && !defined(LINUX)
#include <malloc/malloc.h>
#endif
#include <xview/notice.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/ana_methods.h>
#include <esps/window.h>
#include <esps/fea.h>
#include <sys/param.h>
#include <Objects.h>
#include <esps/exview.h>
#define _NO_PROTO
#include <xprint_util.h>

/* display scale types */
#define	SCALE_DB    0
#define	SCALE_MAG   1
#define	SCALE_PWR   2

/* max number of (current & reference) traces */
#define TRACE_MAX   5
/* max number of vertical (harmonic) cursors */
#define X_CURS_MAX  20
/* max number of horizontal (current & reference) cursors */
#define Y_CURS_MAX  TRACE_MAX

#define NOT_SET -1e-30

int	    debug_level = 0;
int	    command_paused = 0;		/* referenced in globals.c */
int	    use_dsp32, da_location = 0;
extern      int w_verbose;
extern      int fea_sd_special;
double	    image_clip = 7.0, image_range = 40.0;
double      range_db = 200.0, reference_level = 0.0;
extern char *Version;

static int fft_tablesize = 1048576;     /* limit to avoid excessive comp. times */
static char *wave_pro = ".wave_pro";	/* .wave_pro file name from command line */

#define MAX_LPC_ORDER 200

#ifdef STARDENT_3000
extern char WAVES_MISC[];
#endif

char *savestring();
Signal  *get_any_signal();/* see copheader.c, signal.c and read_data.c */

/*global font declarations*/
Xv_Font	    def_font;
int	    def_font_height, def_font_width;

/* These globals are a means of control via the .wave_pro */
int xsp_datwin_height = -1;	/* initial height of xspectrum plot window */
int xsp_datwin_width = -1;	/* initial width of xspectrum plot window */
int xsp_datwin_y = -1;	/* initial Y pos of (1st ob) xspectrum plot window */
int xsp_datwin_x = -1;	/* initial X pos of (1st ob) xspectrum plot window */
int xsp_ctlwin_y = -1;		/* initial Y pos of xspectrum control window */
int xsp_ctlwin_x = -1;		/* initial X pos of xspectrum control window */
int xsp_datwin_forw = 1;   /* bring plot window forward on new plot; 0: don't*/
int xsp_max_lpc_order = 200;  /* maximum permitted LPC order */

extern	int	 print_graphic_printer;
extern	int	 print_graphic_resolution;
extern	char	 print_graphic_orientation[];
extern	char	 print_graphic_type[];
extern	char	 print_graphic_file[];
extern	char	 print_graphic_command[];
extern	double	 print_graphic_scale;

/* Other globals */
    static double   m_time, rstart, rend, sec_cm, start;
    static int	    color, width, height, loc_x, loc_y;
    static char	    name[NAMELEN], file[NAMELEN], signame[NAMELEN];

/* +++++++ Global analysis parameters. +++++++ */
double	w_size = .025,		/* window duration in sec. */
	preemp = .0,		/* H(z) = 1 - preemp*z^(-1);  preemphasis */
	i_f_dur = .05,		/* duration of segment to inverse filter */
	i_f_int = .99;		/* integrator for residual */
int	scale_type = SCALE_DB,	/* scaling for display */
	weight_type = 3, 	/* temporal weighting type (3 = Hanning) */
                                /* given that default func is dft */
	order = 18,		/* order of analysis for LPC, etc. */
	do_region = 0,		/* initialize with window limits set locally */
	reticle_on = 1,		/* initialize reticle to be on */
	hcurs_on = 0,		/* initialize harmonic cursors to be off */
        horiz_c_on = 1,		/* horizontal (ampl.) cursors */
        f_autozoom = 1,		/* auto. zoom freq. if markers are changed */
        a_autozoom = 1,		/* auto. zoom ampl. if markers are changed */
	fb_flag = 0,		/* initialize formant/bandwidth printing off */
        def_fft_size = 1024,
        def_order = 10;

int    current_analysis_type = 0; /* default: number of log_mag_dft */

/* MIT */
double  cep_cutoff = 0.006, cep_trans = .008;
int cep_liftering = 1; /* no liftering, 1 = lowpass, 2 = highpass */

static Selector
  g72 = {"xspectrum_max_lpc_order", "%d", (char *) &xsp_max_lpc_order,NULL},
  g71 = {"xspectrum_datwin_forward", "%d", (char *) &xsp_datwin_forw, &g72},
  g68 = {"xspectrum_datwin_height", "%d", (char *) &xsp_datwin_height, &g71},
  g67 = {"xspectrum_datwin_width", "%d", (char *) &xsp_datwin_width, &g68},
  g66 = {"xspectrum_datwin_y", "%d", (char *) &xsp_datwin_y, &g67},
  g65 = {"xspectrum_datwin_x", "%d", (char *) &xsp_datwin_x, &g66},
  g64 = {"xspectrum_ctlwin_y", "%d", (char *) &xsp_ctlwin_y, &g65},
  g63 = {"xspectrum_ctlwin_x", "%d", (char *) &xsp_ctlwin_x, &g64},
  gg15 = {"xspectrum_cep_liftering", "%d", (char*)&cep_liftering, &g63},
  gg14b = {"xspectrum_cep_trans", "%lf", (char*)&cep_trans, &gg15},
  gg14 = {"xspectrum_cep_cutoff", "%lf", (char*)&cep_cutoff, &gg14b},
  gg13 = {"xspectrum_window_type", "%d", (char*)&weight_type, &gg14},
  gg11 = {"xspectrum_lpc_order", "%d", (char*)&order, &gg13},
  gg10 = {"xspectrum_window_size", "%lf", (char*)&w_size, &gg11},
  gg9  = {"xspectrum_inv_filt_dur", "%lf", (char*)&i_f_dur, &gg10},
  gg8  = {"xspectrum_inv_filt_integ", "%lf", (char*)&i_f_int, &gg9},
  gg7  = {"xspectrum_preemp", "%lf", (char*)&preemp, &gg8},
  gg6  = {"xspectrum_analysis_type", "%d", (char*)&current_analysis_type, &gg7},
  gg5e = {"xspectrum_reference_level", "%lf", (char*)&reference_level, &gg6},
  gg5d  = {"xspectrum_range_db", "%lf", (char*)&range_db, &gg5e},
  gg5c  = {"xspectrum_max_fft_size", "%d", (char*)&fft_tablesize, &gg5d},
  gg5  = {"xspectrum_formants", "%d", (char*)&fb_flag, &gg5c},
  gg4b  = {"xspectrum_horiz_cursors", "%d", (char*)&horiz_c_on, &gg5},
  gg4  = {"xspectrum_reticle", "%d", (char*)&reticle_on, &gg4b},
  gg3  = {"xspectrum_plot_scale", "%d", (char*)&scale_type, &gg4},
  gg2c = {"xspectrum_a_autozoom", "%d", (char*)&a_autozoom, &gg3},
  gg2b = {"xspectrum_f_autozoom", "%d", (char*)&f_autozoom, &gg2c},
  gg2  = {"xspectrum_harmonic_cursors", "%d", (char*)&hcurs_on, &gg2b},
  g1  = {"xspectrum_window_limits", "%d", (char*)&do_region, &gg2},

  /* The duplicate forms below are retained for backwards compatibility. */
  g15 = {"cep_liftering", "%d", (char*)&cep_liftering, &g1},
  g14 = {"cep_cutoff", "%lf", (char*)&cep_cutoff, &g15},
  g13 = {"window_type", "%d", (char*)&weight_type, &g14},
  g11 = {"lpc_order", "%d", (char*)&order, &g13},
  g10 = {"window_size", "%lf", (char*)&w_size, &g11},
  g9  = {"inv_filt_dur", "%lf", (char*)&i_f_dur, &g10},
  g8  = {"inv_filt_integ", "%lf", (char*)&i_f_int, &g9},
  g7  = {"preemp", "%lf", (char*)&preemp, &g8},
  g6  = {"analysis_type", "%d", (char*)&current_analysis_type, &g7},
  g5d  = {"range_db", "%lf", (char*)&range_db, &g6},
  g5c  = {"max_fft_size", "%d", (char*)&fft_tablesize, &g5d},
  g5  = {"formants", "%d", (char*)&fb_flag, &g5c},
  g4b  = {"horiz_cursors", "%d", (char*)&horiz_c_on, &g5},
  g4  = {"reticle", "%d", (char*)&reticle_on, &g4b},
  g3  = {"plot_scale", "%d", (char*)&scale_type, &g4},
  g2c = {"a_autozoom", "%d", (char*)&a_autozoom, &g3},
  g2b = {"f_autozoom", "%d", (char*)&f_autozoom, &g2c},
  g2  = {"harmonic_cursors", "%d", (char*)&hcurs_on, &g2b},
  gg1  = {"window_limits", "%d", (char*)&do_region, &g2};

    /* This selector list specifies all keywords to be recognized for
       the make and mark commands. */
    static Selector a9 = {"sec/cm", "%lf", (char*)&sec_cm, NULL},
		    a8 = {"start", "%lf", (char*)&start, &a9},
		    a7 = {"width", "%d", (char*)&width, &a8},
		    a6 = {"height", "%d", (char*)&height, &a7},
		    a5 = {"loc_y", "%d", (char*)&loc_y, &a6},
		    a4 = {"loc_x", "%d", (char*)&loc_x, &a5},
		    a3 = {"color", "%d", (char*)&color, &a4},
		    a2c = {"rstart", "%lf", (char*)&rstart, &a3},
		    a2b = {"rend", "%lf", (char*)&rend, &a2c},
		    a2 = {"time", "%lf", (char*)&m_time, &a2b},
		    a1 = {"name", "#qstr", name, &a2},
		    a0b = {"signal", "#qstr", file, &a1},
		    a0 = {"file", "#qstr", file, &a0b};

/* This "Objects" structure can be defined as you like.  It should have at
   least "name," "next" and "methods" fields. */

typedef struct {
    int		n;		/* number of data elements */
    double	*data;		/* spectral amplitude data pointer */
    double	*freqs;		/* NULL or n freq values */
    double	band_low,	/* lower frequency limit of data */
		band;		/* width of data frequency band */
    int		scale_type;	/* DB, MAG, or PWR? */
} Trace;

typedef struct objects {
    char	*name;		/* the (ASCII) name of the object */
    Canvas	view;		/* a display handle */
    Methods	*methods;	/* a list of things this object can do */
    Signal	*sig;		/* the Signal being analyzed */
    Trace	*trace[TRACE_MAX];
				/* spectral traces ([0] current, [1]... ref) */
    int		trace_num;
    double	data2[300];	/* LPC coefficients, if any */
    Reticle	*ret;		/* reticle definition for the display */
    int		vers;		/* i.f. segment name distinguisher */
    int         init_located;	/* Has been positioned relative to host sig. */
    double	xhigh, xlow;	/* frequency display limits */
    double	yhigh, ylow;	/* amplitude display limits */
    int		x_off, y_off,
		xloc, yloc,
		width, height;	/* etc., etc. */
    double	sec_cm, start, time;
    double      l_marker, r_marker, /* freq. and ampl. markers */
                t_marker, b_marker;
    double	cursorx[X_CURS_MAX],
		cursory[Y_CURS_MAX];
    int		x_curs_num, y_curs_num;
    int         current_harmonic;
    int         first_display;
    struct objects
		*next;		/* linkage to next list member */
} Objects;

Trace	    *new_trace();
Objects	    *objlist = NULL; /* head of the list of objects to receive commands */
Notify_value destroy_func();
Notify_value sigint_func();
Notify_value sigtrm_func();
Notify_value sigbus_func();
Notify_value sigfpe_func();
Notify_value sigill_func();
Notify_value sigseg_func();

Menu	    menu, make_menu();

/* declarations for menu callback functions */

static void e_ref_save(), e_inv_filt(), e_clear_traces(), e_zoom_out(),
            e_bracket_markers(), e_clear_non_ref_traces(), e_set_ref_level(),
 	    e_print_graphic();

static void print_EPS_temp();

char        *Ok="ok", *Null="null";

/*********************************************************************/
/* Here are the definitions of items which appear in the control panel: */
Panel	    panel;
Frame	    frame;
Frame	    daddy = XV_NULL; /* this global is used by some common routines */

Panel_item  newFunct_item,	order_item,	reticle_item,
	    weightType_item,	wsize_item,	fb_item,
	    limits_item,
	    preemp_item,	hcurs_item,  horiz_c_item,
	    fsize_item,		scale_item,
	    int_c_item,		man_item,	quit_item;
/* MIT */
Panel_item  cep_trans_item, cep_cutoff_item, cep_liftering_item;

/* Here are some procedures invoked by manipulating the panel items: */
void	    quit_proc(),	double_proc(),	int_proc(),
	    newFunction(),	newWindType();

int	    exv_get_help();

/* available analysis functions */
int	    log_mag_dft(),
	    log_mag_dftr(),
	    use_sig_data();
int         esps_compute_spect();

/* MIT */
int         log_cepstrally_smoothed();

/* a structure for relating function names to program entry points */

typedef struct funlist {
    char	    *name;
    int		    (*funct)();
    int		    esps_method;
    struct funlist  *next;
} Funlist;

/* In the following, the original LPC autocorrelation and covariance
   methods (DFT-LPC and DFT-CLPS) have been removed, with their functions
   taken over withing the ESPS compute_rc().  Also, the latter were
   easier to generalize for non-SHORT input
*/

/* Enter any new functions you want to add here (and in main()).
   fun0 should remain at the head of the list. */
Funlist fun12 = {"data-from-signal", use_sig_data, AM_NONE, NULL},
        fun13 = {"CEPST", log_cepstrally_smoothed, AM_NONE, &fun12},
        fun11 = {"STRCOV1", esps_compute_spect, AM_STRCOV1, &fun13},
        fun10 = {"STRCOV", esps_compute_spect, AM_STRCOV, &fun11},
	fun9 =	{"VBURG", esps_compute_spect, AM_VBURG, &fun10},
	fun8 =	{"FBURG", esps_compute_spect, AM_FBURG, &fun9},
	fun7 =	{"MBURG", esps_compute_spect, AM_MBURG, &fun8},
	fun6 =	{"BURG", esps_compute_spect, AM_BURG, &fun7},
	fun5 =	{"COV", esps_compute_spect, AM_COV, &fun6},
	fun4 =	{"AUTOC", esps_compute_spect, AM_AUTOC, &fun5},
	fun1 =	{"DFTR", log_mag_dftr, AM_NONE, &fun4},
	fun0 =	{"DFT", log_mag_dft, AM_NONE, &fun1};



/* We make the default window (weight) type to be Hanning for DFT,
  DFTR, AUTOC, and COV,  but rectangular for all other (Burg and
  maxent) analysis methods; however, we reset the
   default each time a weight_type is changed -- thus, user settings
   are "sticky" with respect to analysis type.  */

int    def_weight_type[11] = {3,3,3,3,0,0,0,0,0,0,3};


/* generic spectral analysis function pointer function(ob);
   sets up ob->trace for plotting; uses global parameters. */


int	(*function)() = log_mag_dft; /* init. value should match panel item */
int	esps_spect_method = AM_NONE;


#define SYNTAX USAGE("xspectrum [-w wave_pro] [-n<waves or other host program>] [-c<registry name of host>] ");

char *host = "waves", *thisprog = "xspectrum";

static void doit();	/* the procedure which handles mouse interaction */
static void	repaint();
static Notify_value iocatcher(); /* handles pipe I/O for host communication*/

/*********************************************************************/
doing_cepstral_smoothing()
{
  return(function == log_cepstrally_smoothed);
}

/*********************************************************************/

int
do_function(ob)
    Objects *ob;
{
    return ((ob->sig->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM
	    && use_sig_data(ob))
	   || (*function)(ob);
}

/*********************************************************************/
/* The interface routines get_receiver_name(), get_methods() and get_receiver()
give the message library access to your locally-defined "Objects."  The
routine make_new_object() must exist, but may simply return NULL.
Alternately, make_new_object() can try to create an object based on the
first (blank- or newline-separated) string in the character array which
is pointed to by its argument. */
/*********************************************************************/
/* Return a pointer to the name of the object. */
char *
get_receiver_name(ob)
     Objects *ob;
{
  return(ob->name);
}

/*********************************************************************/
/* Return the head of the methods list for the object. */
char *
get_methods(ob)
     Objects *ob;
{
  extern Methods base_methods;

  if(ob) return((char*)(ob->methods));

  return((char*)&base_methods);	/* assumes the program's methods are wanted */
}

/*********************************************************************/
window_wash(handle)
    caddr_t handle;
{
    if (handle) return;
    fprintf(stderr, "Can't create window---exiting.\n");
    fprintf(stdout,"Can't create window---exiting.\n");

    /* Clean up before exiting. */
    cleanup();
    kill_proc();
}

/*********************************************************************/
/* Return a pointer to the object whose name (str) is specified. */
char *
get_receiver(str)
     char *str;
{
  Objects *ob;
  static  char name[50];

  ob = objlist;
  if(str && strlen(str)) {
    sscanf(str,"%s",name);
    while(ob) {
      if(ob->name &&
	 (! strcmp(ob->name, name))) {
	   return((char*)ob);
      }
      ob = ob->next;
    }
  }
  return(NULL);
}


/*********************************************************************/

#define SET_PANEL_DOUBLE(item,val)   sprintf(panel_string, "%lf", val);\
  xv_set(item, PANEL_VALUE, panel_string, 0);

#define SET_PANEL_INT(item,val)   sprintf(panel_string, "%d", val);\
  xv_set(item, PANEL_VALUE, panel_string, 0);

#define LIM_LOW(x,y) (x = (x < y) ? y : x)
#define LIM_HIGH(x,y) (x = (x > y) ? y : x)

#define SET_ABS(x) (x = (x > 0) ? x : -x)

char *
meth_set(ob, args)
     Objects *ob;
     char *args;
{
  char *panel_string = malloc(100);

  get_args(args, &gg1);

  if (do_region != 0) do_region = 1;

  xv_set(limits_item, PANEL_VALUE, do_region, 0);

  if (hcurs_on != 0) hcurs_on = 1;

  xv_set(hcurs_item, PANEL_VALUE, !hcurs_on, 0);

  if (horiz_c_on != 0) horiz_c_on = 1;

  xv_set(horiz_c_item, PANEL_VALUE, !horiz_c_on, 0);

  if (reticle_on != 0) reticle_on = 1;

  xv_set(reticle_item, PANEL_VALUE, !reticle_on, 0);

  if (fb_flag != 0) fb_flag = 1;

  xv_set(fb_item, PANEL_VALUE, !fb_flag, 0);

  LIM_LOW(scale_type, 0);
  LIM_HIGH(scale_type, 2);

  xv_set(scale_item, PANEL_VALUE, scale_type, 0);

  SET_PANEL_DOUBLE(preemp_item, preemp);

 /* MIT */
   SET_ABS(cep_trans);
   SET_PANEL_DOUBLE(cep_trans_item, cep_trans);
   SET_ABS(cep_cutoff);
   SET_PANEL_DOUBLE(cep_cutoff_item, cep_cutoff);
   LIM_HIGH(cep_cutoff, 1);
   if (cep_cutoff > 1 || cep_cutoff < 0)
       cep_cutoff = 0;
   xv_set(cep_liftering_item, PANEL_VALUE, cep_liftering, 0);


  SET_ABS(i_f_dur);

  SET_PANEL_DOUBLE(fsize_item, i_f_dur);

  SET_ABS(i_f_int);

  SET_PANEL_DOUBLE(int_c_item, i_f_int);

  SET_ABS(w_size);

  SET_PANEL_DOUBLE(wsize_item, w_size);

  if (xsp_max_lpc_order > MAX_LPC_ORDER)
    xsp_max_lpc_order = MAX_LPC_ORDER;

  LIM_LOW(order, 1);

  LIM_HIGH(order, xsp_max_lpc_order);

  SET_PANEL_INT(order_item, order);

  LIM_LOW(weight_type, 0);
  LIM_HIGH(weight_type, 3);

  /* we reset the method-associated window weighting type.  We `
     don't need to reset the weightType_item on the panel, as
     the call to newFunct_item will take care of that */

  def_weight_type[current_analysis_type] = weight_type;

  LIM_LOW(current_analysis_type, 0);
  LIM_HIGH(current_analysis_type, 10);

  xv_set(newFunct_item, PANEL_VALUE, current_analysis_type, 0);

  /* this sets the analysis funtion() and also calls recompute_all()*/

  newFunction(NULL, current_analysis_type, NULL);

  return(Ok);
}

/*********************************************************************/
char *
make_new_object(str)
     char *str;
{
  char name[NAMELEN], command[NAMELEN], *meth_make_object();

  sscanf(str,"%s",name);  /* get the name of the "object" to be created. */
  if(strlen(name)) {
    sprintf(command,"name %s",name); /* put it into a message */
    if(!strcmp(Ok,meth_make_object(NULL,command))) {
      return(get_receiver(name));
    }
  }
  return(NULL);
}
/*************************************************************************/

/* The following are procedures for retrieving data from the control panel
   created in main(). */
/*************************************************************************/
void
double_proc(item, event)	/* accept numeric input from  a panel< item */
     Panel_item item;
     Event *event;
{
  double val;

#define PNZ_VAL(x) (x = (val > 0.0)? val : x)

  sscanf((char *) xv_get(item, PANEL_VALUE), "%lf", &val);

  if(item == wsize_item) /* Examples of vectoring the input values */
   PNZ_VAL(w_size);

  if(item == preemp_item)
    preemp = val;

  if(item == int_c_item)
    i_f_int = val;

  if(item == fsize_item)
    i_f_dur = val;

 /* MIT */
  if(item == cep_cutoff_item) {
    cep_cutoff = val;
    if(!doing_cepstral_smoothing())
      return;
  }
  if(item == cep_trans_item) {
    cep_trans = val;
    if(!doing_cepstral_smoothing())
      return;
  }

  recompute_all();		/* redo all spectra with the new parameter */
#undef PNZ_VAL
}

/*************************************************************************/
void
int_proc(item, event)	/* accept numeric input from  a panel item */
     Panel_item item;
     Event *event;
{
  int val;
  char junk[10];

#define PNZ_VAL(x) (x = (val > 0)? val : x)

  sscanf((char *) xv_get(item, PANEL_VALUE), "%d", &val);

  if(item == order_item) /* Example of vectoring an input value */

  if (xsp_max_lpc_order > MAX_LPC_ORDER)
    xsp_max_lpc_order = MAX_LPC_ORDER;

    if(PNZ_VAL(order) > xsp_max_lpc_order) {
      order = xsp_max_lpc_order;
      sprintf(junk,"%d",order);
      xv_set(item, PANEL_VALUE, junk, 0);
    }

  recompute_all();		/* redo all spectra with the new parameter */
#undef PNZ_VAL
}

/*************************************************************************/
void
newFunction(item, value, event)
     Panel_item item;
     int value;
     Event *event;
{
  int (*fun)();
  Funlist *fp = &fun0;
  int func_id = value;

  /* set corresponding window weighting */

  current_analysis_type = value;
  weight_type = def_weight_type[current_analysis_type];

  xv_set(weightType_item, PANEL_VALUE, weight_type, 0);

  while(fp && value--) fp = fp->next;

  if(fp->funct) {
    function = fp->funct;
    esps_spect_method = fp->esps_method;
    recompute_all();		/* redo all spectra with the new parameter */
  }
  return;
}

/*************************************************************************/
void newWindType(item, value, event)
     Panel_item	item;
     int		value;
     Event	*event;
{

  if(item == weightType_item) {
    weight_type = value;
    def_weight_type[current_analysis_type] = weight_type;
  }
  else
    if(item == limits_item) {
      do_region = value;
      return;
    } else
      if(item == hcurs_item)
	hcurs_on = !value;
      else
	if(item == horiz_c_item)
	  horiz_c_on = !value;
	else
	  if(item == scale_item)
	    scale_type = value;
	  else
	    if(item == reticle_item)
	      reticle_on = !value;
	    else
	      if(item == fb_item)
		fb_flag = !value;
	      else
		if(item == cep_liftering_item) {  /* MIT */
		  cep_liftering = value;
		  if(!doing_cepstral_smoothing())
		    return;
		}

  recompute_all();		/* redo all spectra with the new parameter */
}

/*********************************************************************/
void
quit_proc(item, event)
     Panel_item item;
     Event *event;
{
  cleanup();
  kill_proc();
}

/*********************************************************************/
char *generate_startup_command(registry_name)
     char *registry_name;
{
  static char com[MES_BUF_SIZE];

  sprintf(com,"add_op name %s op #send function %s registry %s command _name mark signal _file time _cursor_time rstart _l_marker_time rend _r_marker_time",
	   basename(thisprog), thisprog, registry_name);
  return(com);
}

/*********************************************************************/

/*********************************************************************/
/* This program is designad to be called as an "attachment" to programs
  like "xwaves," but can be run stand-alone for debugging and other purposes. */

extern int  optind;		/* for use of getopt() */
extern char *optarg;		/* for use of getopt() */
extern int fullscreendebug;

static char env_str[MAXPATHLEN+20];

main(ac, av)
     int ac;
     char **av;
{
  Objects  *new_objects();
  extern Methods base_methods;	/* a list of things that the program can do */
  char mess[100];
  extern char default_header[];	/* dummy header to use if signals have none
				   (in copheader.c) */
  int rem_args; /* no. of args after getopt processing */
  int i;
  int		ch;		/* option letter read by getopt */

  char           *server_name = "xwaves";
  extern Display *X_Display;
  extern Window   comm_window;
  extern char    *registry_name;
  extern Frame    daddy; /* global reference in xprint_setup.c and xnotice.c */


  fullscreendebug = 0; /* this global inhibits server grabs that cause
			  problems on the SGI */
  thisprog = av[0];

  while ((ch = getopt(ac, av, "w:n:c:")) != EOF)
    switch (ch)
      {
      case 'n':
	host = optarg;
	break;
      case 'w':
	wave_pro = optarg;
	break;
      case 'c':
        server_name = optarg;
	break;
      default:
	SYNTAX
	  if(debug_level)
	    for(ch = 0; ch < ac; ch++)
	      fprintf(stderr,"%s ",av[ch]);
	fprintf(stderr,"\n");
	exit(-1);
      }

  attachment = TRUE;	/* make cmap create static colormap segment */

  def_font = (Xv_Font) xv_find(XV_NULL, FONT,
			FONT_FAMILY,	FONT_FAMILY_DEFAULT_FIXEDWIDTH,
			0);

  def_font_width = (int) xv_get(def_font, FONT_DEFAULT_CHAR_WIDTH);
  def_font_height = (int) xv_get(def_font, FONT_DEFAULT_CHAR_HEIGHT);

  get_all_globals(wave_pro, &gg1);
  sprintf(env_str,"XPPATH=%s/lib/Xp",get_esps_base(NULL));
  putenv(env_str);

  /* just in case fea_sd_special is set in the .wave_pro, we
     reset it here; we want all data types read in as they are
     in the file
   */
  fea_sd_special = 0;

  get_color_depth();
  setup_colormap();

  menu = make_menu();

  /* Create a control panel. */

  /* initial position can be specified via globals (.wave_pro) */

  daddy =	/* global reference in xprint_setup.c and xnotice.c */
      frame = xv_create(XV_NULL, FRAME,
		XV_X,	(xsp_ctlwin_x >= 0) ? xsp_ctlwin_x : 590,
		XV_Y,	(xsp_ctlwin_y >= 0) ? xsp_ctlwin_y : 0,
		XV_LABEL,		    "Spectrum Analyzer",
		FRAME_INHERIT_COLORS,	    FALSE,
		WIN_IGNORE_EVENTS,
		    WIN_ASCII_EVENTS,
		    0,
		0);

  notify_interpose_destroy_func(frame, destroy_func);

  /* set up communications with the host (xwaves) */
  if (!setup_attach_comm(frame, server_name, "xspectrum")) {
     fprintf(stderr, "Failed to setup ipc communications\n");
     exit(0);
  }
  sprintf(mess, "Spectrum Analyzer %s (%s)", Version, registry_name);
  xv_set(frame, XV_LABEL, mess, NULL);
  if (debug_level)
     fprintf(stderr, "registry name: %s\n", registry_name);

  send_start_command(generate_startup_command(registry_name));

  set_blowup_op();


  window_wash(frame);

  panel = xv_create(frame, PANEL,
		XV_X,			    0,
		XV_Y,			    0,
		0);
  window_wash(panel);

  newFunct_item = xv_create(panel, PANEL_CYCLE,
		XV_X,			    8,
		XV_Y,			    16,
		PANEL_LABEL_STRING,	    "Analysis type:",
		PANEL_CHOICE_STRINGS,
		    fun0.name,
		    fun1.name,
/*		    fun2.name,*/
/*		    fun3.name,*/
		    fun4.name,
		    fun5.name,
		    fun6.name,
		    fun7.name,
		    fun8.name,
		    fun9.name,
		    fun10.name,
		    fun11.name,
/* MIT */
                    fun13.name,
		    0,
		PANEL_VALUE,		    current_analysis_type,
		PANEL_NOTIFY_PROC,	    newFunction,
		0);

  weightType_item = xv_create(panel, PANEL_CYCLE,
		XV_X,			    8,
		XV_Y,			    40,
		PANEL_LABEL_STRING,	    "Window type:",
		PANEL_CHOICE_STRINGS,
		    "rectangular",
		    "Hamming",
		    "cos^4",
		    "Hanning",
		    0,
		PANEL_VALUE,		    weight_type,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  limits_item = xv_create(panel, PANEL_CHOICE,
		XV_X,			    8,
		XV_Y,			    68,
		PANEL_LABEL_STRING,	    "Window limits from:",
		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
		PANEL_CHOOSE_ONE,	    TRUE,
		PANEL_CHOICE_STRINGS,
		    "Cursor +- size/2",
		    "Markers",
		    0,
		PANEL_VALUE,		    do_region,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  sprintf(mess,"%6.4f",preemp);
  preemp_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    8,
		XV_Y,			    101,
		PANEL_LABEL_STRING,	    "Preemphasis coeff:",
		PANEL_VALUE,		    mess,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_NOTIFY_PROC,	    double_proc,
		0);

  sprintf(mess,"%7.6f",i_f_dur);
  fsize_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    8,
		XV_Y,			    125,
		PANEL_LABEL_STRING,	    "Inverse filter intvl. (sec):",
		PANEL_VALUE,		    mess,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_NOTIFY_PROC,	    double_proc,
		0);

  sprintf(mess,"%6.4f",i_f_int);
  int_c_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    8,
		XV_Y,			    149,
		PANEL_LABEL_STRING,	    "Integration coeff:",
		PANEL_VALUE,		    mess,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_NOTIFY_PROC,	    double_proc,
		0);

  sprintf(mess,"%d",order);
  order_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    224,
		XV_Y,			    21,
		PANEL_LABEL_STRING,	    "order:",
		PANEL_VALUE,		    mess,
		PANEL_VALUE_DISPLAY_LENGTH, 3,
		PANEL_NOTIFY_PROC,	    int_proc,
		0);

  sprintf(mess,"%7.6f",w_size);
  wsize_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    224,
		XV_Y,			    45,
		PANEL_LABEL_STRING,	    "size (sec):",
		PANEL_VALUE,		    mess,
		PANEL_VALUE_DISPLAY_LENGTH, 8,
		PANEL_NOTIFY_PROC,	    double_proc,
		0);

  hcurs_item  = xv_create(panel, PANEL_CHOICE,
		XV_X,			    328,
		XV_Y,			    100,
		PANEL_LABEL_STRING,	    "Harmonic cursors:",
		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
		PANEL_CHOOSE_ONE,	    TRUE,
		PANEL_CHOICE_STRINGS,
		    "On",
		    "Off",
		    0,
		PANEL_VALUE,		    !hcurs_on,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  scale_item = xv_create(panel, PANEL_CYCLE,
		XV_X,			    328,
		XV_Y,			    125,
		PANEL_LABEL_STRING,	    "Plot scale:",
		PANEL_CHOICE_STRINGS,
		    "log pwr (dB)",
		    "magnitude",
		    "power (sq mag)",
		    0,
		PANEL_VALUE,		    scale_type,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  man_item = xv_create(panel, PANEL_BUTTON,
 	      XV_X,			    8,
 	      XV_Y,			    173,
		PANEL_LABEL_STRING,	    "xspectrum manual",
		XV_KEY_DATA,
		    EXVK_HELP_NAME,	    FIND_WAVES_LIB(NULL,
							   "xspectrum.man"),
		XV_KEY_DATA,
		    EXVK_HELP_TITLE,        "xspectrum manual page",
		PANEL_NOTIFY_PROC,	    exv_get_help,
		0);

  quit_item = xv_create(panel, PANEL_BUTTON,
		XV_X,			    160,
		XV_Y,			    173,
		PANEL_LABEL_STRING,	    "QUIT",
		PANEL_NOTIFY_PROC,	    quit_proc,
		0);

  horiz_c_item  = xv_create(panel, PANEL_CHOICE,
		XV_X,			    332,
		XV_Y,			    13,
		PANEL_LABEL_STRING,	    "Horizontal cursor:",
		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
		PANEL_CHOOSE_ONE,	    TRUE,
		PANEL_CHOICE_STRINGS,
		    "On",
		    "Off",
		    0,
		PANEL_VALUE,		    !horiz_c_on,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  reticle_item = xv_create(panel, PANEL_CHOICE,
		XV_X,			    404,
		XV_Y,			    42,
		PANEL_LABEL_STRING,	    "Reticle:",
		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
		PANEL_CHOOSE_ONE,	    TRUE,
		PANEL_CHOICE_STRINGS,
		    "On",
		    "Off",
		    0,
		PANEL_VALUE,		    !reticle_on,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

  fb_item = xv_create(panel, PANEL_CHOICE,
		XV_X,			    387,
		XV_Y,			    71,
		PANEL_LABEL_STRING,	    "Formants:",
		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
		PANEL_CHOOSE_ONE,	    TRUE,
		PANEL_CHOICE_STRINGS,
		    "On",
		    "Off",
		    0,
		PANEL_VALUE,		    !fb_flag,
		PANEL_NOTIFY_PROC,	    newWindType,
		0);

 /* MIT */
   sprintf(mess,"%6.4f", cep_cutoff);
   cep_cutoff_item = xv_create(panel, PANEL_TEXT,
			       XV_X,			    230,
		XV_Y,			    154,
 	      PANEL_LABEL_STRING,	    "Cep. cut (sec):",
 	      PANEL_VALUE,		    mess,
 	      PANEL_VALUE_DISPLAY_LENGTH, 8,
 	      PANEL_NOTIFY_PROC,	    double_proc,
 	      0);
   sprintf(mess,"%6.4f", cep_trans);
   cep_trans_item = xv_create(panel, PANEL_TEXT,
		XV_X,			    400,
		XV_Y,			    154,
 	      PANEL_LABEL_STRING,	    "Cep. trans:",
 	      PANEL_VALUE,		    mess,
 	      PANEL_VALUE_DISPLAY_LENGTH, 8,
 	      PANEL_NOTIFY_PROC,	    double_proc,
 	      0);
   cep_liftering_item = xv_create(panel, PANEL_CHOICE,
 		XV_X,			    275,
 		XV_Y,			    175,
 		PANEL_LABEL_STRING,	    "Liftering:",
 		PANEL_LAYOUT,		    PANEL_HORIZONTAL,
 		PANEL_CHOOSE_ONE,	    TRUE,
 		PANEL_CHOICE_STRINGS,
 		    "None",
 		    "Lowpass",
                     "Highpass",
 		    0,
 		PANEL_VALUE,		    cep_liftering,
 		PANEL_NOTIFY_PROC,	    newWindType,
 		0);

  /* (XView bug?) Attaching the icon earlier screws up the panel dimensions
   * and the placement of panel items.
   */
  (void) exv_attach_icon(frame, ERL_NOBORD_ICON, "spectrum", TRANSPARENT);

  window_fit(panel);
  window_fit(frame);

  newFunction(NULL, current_analysis_type, NULL);

  /* This program will be the first entry on the list of objects which may
     receive messages. */
  objlist = new_objects(av[0]);	/* It will have the program's name */
  objlist->methods = &base_methods; /* and a special set of methods. */

  if(!debug_level) {
    /*
      notify_set_signal_func(frame, sigint_func, SIGINT, NOTIFY_SYNC);
      notify_set_signal_func(frame, sigtrm_func, SIGTERM, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigfpe_func, SIGFPE, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigbus_func, SIGBUS, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigseg_func, SIGSEGV, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigill_func, SIGILL, NOTIFY_ASYNC);
      */
    signal( SIGINT, sigint_func);
    signal( SIGTERM, sigtrm_func);
    signal( SIGFPE, sigfpe_func);
    signal( SIGBUS, sigbus_func);
    signal( SIGSEGV, sigseg_func);
    signal( SIGILL, sigill_func);
  }
  window_main_loop(frame);	/* Relinquish control to "notifier." */
}

/*********************************************************************/
set_blowup_op()
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"set blowup_op %s",basename(thisprog));
  mess_write(mess);
}

/*************************************************************************/
/* These translate things to and from screen to data coordinates. */
/*************************************************************************/
static double
x_to_frequency(ob, x)
    Objects *ob;
    int	    x;
{
    Rect    *rec;
    Reticle *r;
    double  width;

    if (!ob) return 0.0;

    if (r = ob->ret)
    {
	x -= r->bounds.left;
	width = r->bounds.right - r->bounds.left;
    } else {
	rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
	x -= ob->x_off;
	width = rec->r_width - 1 - ob->x_off;
    }

    return (x >= width) ? ob->xhigh
	    : (x <= 0) ? ob->xlow
	    : ob->xlow + x * (ob->xhigh - ob->xlow) / width;
}

/*************************************************************************/
static int
frequency_to_x(ob, freq)
    Objects *ob;
    double  freq;
{
    Rect    *rec;
    Reticle *r;
    double  width, xoff;
    int	    x;

    if (ob)
    {
 	if (r = ob->ret)
	{
	    width = r->bounds.right - r->bounds.left;
	    xoff = 0.5 + r->bounds.left;	/* 0.5 for rounding */
	} else {
	    rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
	    width = rec->r_width - 1 - ob->x_off;
	    xoff = 0.5 + ob->x_off;		/* 0.5 for rounding */
	}

	x = (int) (xoff + width * (freq - ob->xlow) / (ob->xhigh - ob->xlow));

	return x;
    }

    return 0;
}

/*************************************************************************/
static int
x_to_index(ob, x, t)
    Objects *ob;
    int	    x;
    int	    t;
{
    Rect    *rec;
    Trace   *trace;
    Reticle *r;
    double  width;
    int	    n, index;
    double  *freqs;

    if (ob && (trace = ob->trace[t]) && trace->band > 0.0 && (n = trace->n) > 0)
    {
	if (freqs = trace->freqs)
	{
	    double  f = x_to_frequency(ob, x);
	    int	    i, j, k;

	    if (f < freqs[i = 0])
		return i;
	    else if (f > freqs[j = n-1])
		return j;
	    else
	    {
		while (j - i > 1)
		{
		    if (f < freqs[k = (i+j)/2])
			j = k;
		    else
			i = k;
		}

		return (f - freqs[i] < freqs[j] - f) ? i : j;
	    }
	}
	else
	{
	    if (r = ob->ret)
	    {
		x -= r->bounds.left;
		width = r->bounds.right - r->bounds.left;
	    } else {
		rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
		x -= ob->x_off;
		width = rec->r_width - 1 - ob->x_off;
	    }

	    if (x > 0 && width > 0.0)
	    {
		index = (int) (0.5 + ((n-1)/trace->band)
				      * (ob->xlow - trace->band_low
					 + x*(ob->xhigh - ob->xlow)/width));

		return (index < 0) ? 0 : (index < n) ? index : (n-1);
	    }
	}
    }

    return 0;
}

/*********************************************************************/
amplitude_to_y(ob, amp)
    Objects *ob;
    double  amp;
{
    Rect    *rec;
    Reticle *r;
    double  height, yscale, yoff;
    int	    y;

    if (ob)
    {
	if (r = ob->ret)
	{
	    height = r->bounds.bottom - r->bounds.top;
	    yoff = 0.5 + r->bounds.bottom;	/* 0.5 for rounding */
	} else {
	    rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
	    height = rec->r_height - 1 - ob->y_off;
	    yoff = 0.5 + height;		/* 0.5 for rounding */
	}

	yscale = -height/(ob->yhigh - ob->ylow);
	y = (int) (yoff + yscale * (amp - ob->ylow));

	return (y <= yoff) ? y : yoff;
    }

    return 0;
}

/*********************************************************************/
double y_to_amplitude(ob, y)
     Objects *ob;
     int y;
{
  Rect    *rec;
  Reticle *r;
  double  height, yscale, yoff, amp;

  if(ob) {
    if(r = ob->ret) {
      height = r->bounds.bottom - r->bounds.top;
      yoff = 0.5 + r->bounds.bottom; /* 0.5 for rounding */
    } else {
      rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
      height = rec->r_height - 1 - ob->y_off;
      yoff = 0.5 + height;	/* 0.5 for rounding */
    }

    yscale = -height/(ob->yhigh - ob->ylow);

    amp = ob->ylow + ((y - yoff)/yscale);

    return (amp);
  }

  return(0.0);
}

/*********************************************************************/

int
trace_color(i)
    int		i;
{
    static int	*c_vars[] = {&TEXT_COLOR, &CURSOR_COLOR, &YA1_COLOR,
			     &YA2_COLOR,  &YA3_COLOR,    &YA4_COLOR,
			     &YA5_COLOR},
		n = sizeof(c_vars)/sizeof(c_vars[0]) - 1;

    return (i <= n) ? *c_vars[i] : *c_vars[1 + i%n];
}

/*********************************************************************/

static void
plotcursors(ob)
    Objects	*ob;
{
    Xv_Window	pw;
    Rect	*rec;
    int		i, j, n;
    int		x, y, y0[Y_CURS_MAX];
    double	curs;
    int		color;

    if (!ob || ob->view == XV_NULL
	|| (pw = canvas_paint_window(ob->view)) == XV_NULL) return;

    rec = (Rect *) xv_get(pw, WIN_RECT);

    for (i = 0; i < ob->x_curs_num; i++)
	if ((curs = ob->cursorx[i]) >= ob->xlow && curs <= ob->xhigh)
	{
	    x = frequency_to_x(ob, curs);
	    pw_vector(pw, x, 0, x, rec->r_height - 1,
		      PIX_COLOR(CURSOR_COLOR)|(PIX_SRC^PIX_DST), CURSOR_COLOR);
	}

    if(horiz_c_on) {
      n = 0;
      for (i = 0; i < ob->y_curs_num; i++)
	if ((curs = ob->cursory[i]) >= ob->ylow && curs <= ob->yhigh)
	  {
	    y = amplitude_to_y(ob, curs);

	    for (j = 0; j < n && y0[j] != y; j++) { }

	    if (j == n)		/* New y value; won't overwrite another cusor. */
	      {
		y0[n++] = y;	/* New distinct value. */
		color = (i == 0) ? RETICLE_COLOR : trace_color(i);
		pw_vector(pw, 0, y, rec->r_width - 1, y,
			  PIX_COLOR(color)|(PIX_SRC^PIX_DST), color);
	      }
	  }
    }
}

/*********************************************************************/
static void
printcursors(ob)
    Objects	*ob;
{
    Xv_Window	pw;
    char	mess[200];
    int		i;
    int		color;

    if (!ob || ob->view == XV_NULL
	|| (pw = canvas_paint_window(ob->view)) == XV_NULL
	|| ob->cursorx[0] < ob->xlow || ob->cursorx[0] > ob->xhigh) return;

    if (ob->trace[0])
    {
	color = trace_color(0);
	sprintf(mess, "%7.1fHz   ", ob->cursorx[0]);
	pw_text(pw, 50, 11, PIX_COLOR(color)|PIX_SRC, def_font, mess);
    }

    for (i = 0; i < TRACE_MAX && ob->trace[i]; i++)
    {
	color = trace_color(i);
	sprintf(mess, "%5.1f%s", ob->cursory[i],
		(ob->trace[i]->scale_type == SCALE_DB) ? "dB" : "  ");
	pw_text(pw, 146 + 64*i, 11,
		PIX_COLOR(color)|PIX_SRC, def_font, mess);
    }
}


/*********************************************************************/
send_curs_freq(name, freq)
     char *name;
     double freq;
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"%s cursor frequency %f",name,freq);
  mess_write(mess);
}

/*********************************************************************/
static void
setcursors(ob, curs_freq)
    Objects *ob;
    double  curs_freq;		/* cursor frequency */
{
    int	    x;
    double  m;
    int	    i;

    if (!ob) return;
    send_curs_freq(ob->name,curs_freq);
    if(hcurs_on && curs_freq != 0.0) {
      m = ((curs_freq > 0.0) ? ob->xhigh : ob->xlow) / curs_freq;
      ob->x_curs_num =
	(m >= X_CURS_MAX) ? X_CURS_MAX : (m <= 1) ? 1 : (int) m;
    } else
	ob->x_curs_num = 1;

    for (i = 0; i < ob->x_curs_num; i++)
	ob->cursorx[i] = (i+1)*curs_freq;

    x = frequency_to_x(ob, curs_freq);

    for (i = 0; i < TRACE_MAX && ob->trace[i] && ob->trace[i]->data; i++)
	ob->cursory[i] = ob->trace[i]->data[x_to_index(ob, x, i)] -
	  ((scale_type == SCALE_DB)? reference_level : 0.0);

    ob->y_curs_num = i;
}

/*********************************************************************/

static void
save_as_ref(ob)
    Objects *ob;
{
    Trace   *cur_trace, *ref_trace;
    int	    n, t, x;
    double  *dp, *dq;

    if (!(cur_trace = ob->trace[0]) || !cur_trace->data) return;
    n = cur_trace->n;

    t = ob->trace_num;
    ref_trace =
	ob->trace[t] = new_trace(ob->trace[t], n, !!cur_trace->freqs);

    if (ref_trace);
    {
	ob->trace_num = (t < TRACE_MAX - 1) ? t + 1 : 1;

	ref_trace->band_low = cur_trace->band_low;
	ref_trace->band = cur_trace->band;
	ref_trace->scale_type = cur_trace->scale_type;
	if ((dp = cur_trace->data) && (dq = ref_trace->data))
	    for (x = 0; x < n; x++)
		*dq++ = *dp++;
	if ((dp = cur_trace->freqs) && (dq = ref_trace->freqs))
	    for (x = 0; x < n; x++)
		*dq++ = *dp++;
    }
}

/*********************************************************************/
static void
set_ref_level(ob, x)
    Objects *ob;
     int x;
{
    Trace   *cur_trace;

    if (!(cur_trace = ob->trace[0]) || !cur_trace->data) return;

    reference_level = cur_trace->data[x_to_index(ob, x, 0)];

    if(debug_level)
      fprintf(stderr, "Setting reference level to %f dB\n", reference_level);

}


/*********************************************************************/
plot_v_marker(ob, x)
     Objects *ob;
     int x;
{
  Xv_Window	pw;
  Rect	*rec;

  if (ob && ob->view &&
      (pw = canvas_paint_window(ob->view))) {
    rec = (Rect *) xv_get(pw, WIN_RECT);
    pw_vector(pw, x, 0, x, rec->r_height - 1,
	      PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
  }
}

/*********************************************************************/
move_v_markers(o,event)
     Objects *o;
     Event *event;
{
  int x = event_x(event), y = event_y(event);
  double freq = x_to_frequency(o,x);

  if(event_is_down(event)) {	/* move the left marker */
    plot_v_marker(o, frequency_to_x(o, o->l_marker));
    o->l_marker = freq;
    plot_v_marker(o, x);
  } else {
    plot_v_marker(o, frequency_to_x(o, o->r_marker));
    if(freq < o->l_marker) {
      o->r_marker = o->l_marker;
      o->l_marker = freq;
    } else
      o->r_marker = freq;
    if(f_autozoom)
      redo_display(o);
    else
      plot_v_marker(o, x);
  }

}

/*********************************************************************/
plot_h_marker(ob, y)
     Objects *ob;
     int y;
{
  Xv_Window	pw;
  Rect	*rec;

  if (ob && ob->view &&
      (pw = canvas_paint_window(ob->view))) {
    rec = (Rect *) xv_get(pw, WIN_RECT);
    pw_vector(pw, ob->x_off, y, rec->r_width - 1, y,
	      PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
  }
}

/*********************************************************************/
move_h_markers(o,event)
     Objects *o;
     Event *event;
{
  int x = event_x(event), y = event_y(event);
  double ampl = y_to_amplitude(o,y);

  if(event_is_down(event)) {	/* move the top marker */
    plot_h_marker(o, amplitude_to_y(o,o->t_marker));
    o->t_marker = ampl;
    plot_h_marker(o, y);
  } else {
    plot_h_marker(o, amplitude_to_y(o,o->b_marker));
    if(ampl > o->t_marker) {
      o->b_marker = o->t_marker;
      o->t_marker = ampl;
    } else
      o->b_marker = ampl;
    if(a_autozoom)
      redo_display(o);
    else
      plot_h_marker(o, y);
  }

}

/*********************************************************************/
plot_markers(ob)
     Objects *ob;
{
  if(ob->l_marker != ob->r_marker) {
    plot_v_marker(ob, frequency_to_x(ob, ob->l_marker));
    plot_v_marker(ob, frequency_to_x(ob, ob->r_marker));
  }
  if(ob->t_marker != ob->b_marker) {
    plot_h_marker(ob, amplitude_to_y(ob, ob->t_marker));
    plot_h_marker(ob, amplitude_to_y(ob, ob->b_marker));
  }
}

/*********************************************************************/
move_markers(ob, event)
     Objects *ob;
     Event *event;
{
  if(event_shift_is_down(event))
    move_h_markers(ob,event);
  else
    move_v_markers(ob,event);
}

/*********************************************************************/
/* This is the event handler, which deals with mouse
   interactions in windows created by meth_make_object(). */
static void
doit(pw, event, arg)
    Xv_Window	pw;
    Event	*event;
    caddr_t	arg;
{
    Objects	*ob;
    int		x, y;
    Canvas	canvas = xv_get(pw, CANVAS_PAINT_CANVAS_WINDOW);
    Menu	me;
    Menu_item   inv_item;

/* This is how "user" data associated with a window may be retrieved: */
  if(!(canvas && (ob = (Objects *) xv_get(canvas, WIN_CLIENT_DATA))))
    return;

  x = event_x(event);		/* mouse coordinates within the window */
  y = event_y(event);

  switch(event_id(event)){	/* identify nature of interaction: */

  case MS_LEFT:	    /* copy current spectrum to reference spectrum array */

    move_markers(ob,event);
    break;

  case MS_MIDDLE:   /* currently, just a means of gettting out of wind w/o
		       cursor movement */
    break;

  case MS_RIGHT:    /* bring up menu */

    if (event_is_up(event)) return;	/* Ignore button release. */

    me = (Menu) xv_get(canvas, WIN_MENU);

    /* inverse filtering is not an option for non-LPC methods */

    inv_item = (Menu_item) xv_find(me, MENUITEM,
				  MENU_VALUE, e_inv_filt, 0);

    if (function != esps_compute_spect)
      xv_set(inv_item, MENU_INACTIVE, TRUE, 0);
    else
      xv_set(inv_item, MENU_INACTIVE, FALSE, 0);

    menu_show(me, canvas, event, 0);

    break;

  case LOC_MOVE:		/* "locator move" with or without button press */
  case LOC_DRAG:
    if(event_middle_is_down(event)) /* to escape the window and leave cursor */
      break;
/*
IFF hcurs_on: Translate the current x position into a frequency.  This
frequency is then taken as that of the current_harmonic and is divided
accordingly to get the fundamental.  This frequency then becomes the
argument of the setcursors() call.  If the current harmonic is more
than the fundamental away from the hightest plottable frequency, and
it is also less than the max harmonic number plottable, increase the current
harmonic by one.  If the harmonic is bumped, the cursor location is
forced to the corresponding location.
*/
    if(XEventsQueued(xv_get(canvas, XV_DISPLAY),QueuedAlready) <= 0) {
      plotcursors(ob);		/* XOR off old cursors */

      /* if there's data to interact with... */
      if (ob->trace[0] && ob->trace[0]->data && (ob->trace[0]->n > 0)) {
	if(hcurs_on) {
	  double high_h = x_to_frequency(ob, x);
	  double fund = high_h/ob->current_harmonic;

	  /* To avoid instability, only process the most recent event. */
	  setcursors(ob, fund);
	  if((ob->current_harmonic < X_CURS_MAX) &&
	     (ob->xhigh - high_h) > (2.0*fund)) {
	    int newx = frequency_to_x(ob, high_h + fund);
	    xv_set(ob->view, WIN_MOUSE_XY, newx, y, 0);
	    ob->current_harmonic++;
	  } else
	    if((ob->current_harmonic > 1) &&
	       (ob->xhigh - high_h) < (0.5*fund)) {
	      int newx = frequency_to_x(ob, high_h - fund);
	      xv_set(ob->view, WIN_MOUSE_XY, newx, y, 0);
	      ob->current_harmonic--;
	    }
	} else			/* no harmonic cursors */
	  setcursors(ob,x_to_frequency(ob,x));
      } else			/* no data to display */
	ob->x_curs_num = ob->y_curs_num = 0;

      plotcursors(ob);		/* draw new cursor positions */
      printcursors(ob);		/* print frequency and amplitude */
    }

    break;
  default:
    break;
  }
  return;
}

/*********************************************************************/
/* Plot the spectrum (and reference spectrum, if any) */
/* pointed to by ob->data; (ob->n points) */

plot_data(ob)
    Objects	*ob;
{
  if(ob && ob->view) {
    Xv_Window	pw;
    Rect	*rec;
    int		xoff, yoff;
    double	xscale, yscale, width, height;
    Reticle	*r;
    int		i;

    if (debug_level)
      fprintf(stderr, "entered plot_data()\n");

    if (!ob) return FALSE;

    pw = canvas_paint_window(ob->view);
    if((r = ob->ret)) {
      height = r->bounds.bottom - r->bounds.top;
      width = r->bounds.right - r->bounds.left;
      xoff = r->bounds.left + 0.5; /* 0.5 for rounding */
      yoff = r->bounds.bottom + 0.5; /* 0.5 for rounding */
    } else {
      rec = (Rect *) xv_get(pw, WIN_RECT);
      height = rec->r_height - 1 - ob->y_off;
      width = rec->r_width - 1 - ob->x_off;
      xoff = ob->x_off + 0.5;	/* 0.5 for rounding */
      yoff = rec->r_height - 1 - ob->y_off + 0.5; /* 0.5 for rounding */
    }

    xscale = width / (ob->xhigh - ob->xlow);
    yscale = -height/(ob->yhigh - ob->ylow);

    if (!plot_trace(pw, xoff, yoff, xscale, yscale, ob->xlow, ob->xhigh,  ob->ylow,
		    ob->trace[0], trace_color(0)))
      return FALSE;

    for (i = 1; i < TRACE_MAX && ob->trace[i]; i++)
      (void) plot_trace(pw, xoff, yoff, xscale, yscale, ob->xlow, ob->xhigh, ob->ylow,
			ob->trace[i], trace_color(i));

    return TRUE;
  }
  return(FALSE);
}

/*********************************************************************/
char *meth_print_data(ob, str)
     Objects *ob;
     char *str;
{
  static char output[NAMELEN];
  static Selector s1 = {"output", "#qstr", output, NULL};
  FILE *fp = NULL;
  struct header *hdr;
  int i;
  double freq;

  *output = 0;
  get_args(str, &s1);

  if(*output && ob->view && ob->trace[0] && (ob->trace[0]->n > 1)) {
    if((fp = fopen(output, "w"))) {

      if(ob->sig && ob->sig->name) {
	fprintf(fp,"signal %s\n",ob->sig->name);
	fprintf(fp,"time %f\n",ob->time);
	if((ob->sig->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM)
	  freq = ob->sig->freq;
	else
	  if((hdr = ob->sig->header->esps_hdr))
	    freq = get_genhd_val("src_sf",hdr,
				 get_genhd_val("sf",hdr,(double)-1.0));
	if(freq > 0.0)
	  fprintf(fp,"src_sf %f\n",freq);

	fprintf(fp,"num_freqs %d\n", ob->trace[0]->n);

	output_variables_to_file(fp, &g1);
	if(ob->trace[0]->freqs)
	  for(i=0 ; i < ob->trace[0]->n; i++)
	    fprintf(fp,"%f	%f\n",
		    ob->trace[0]->freqs[i],ob->trace[0]->data[i]);
	else {
	  double df = (ob->trace[0]->band - ob->trace[0]->band_low) /
			(ob->trace[0]->n - 1);
	  for(i=0; i < ob->trace[0]->n; i++)
	    fprintf(fp,"%f	%f\n",
		    ob->trace[0]->band_low + (df * i),
		    ob->trace[0]->data[i]);
	}
	Fclose(fp);
	return(Ok);
      } else {
	sprintf(notice_msg, "Can't open %s for output in xspectrum.",
		output);
	show_notice(1, notice_msg);
      }
    }
  }
  return(Null);
}


#define scoff(x) ((int)(yoffs + ((double)(x - ydboff) * scale)))
plot_a_trace(val, s_data, index, line_type, xoffset, scale, yoffs, incr, npix, nsam, pw, ymin)
     Xv_Window	pw;
     int val, index, npix, line_type, nsam;
     double *s_data, xoffset, scale, yoffs, incr;
{
  register int i, j, k, y, y2;
  register double *q, *r, *p, imin, sump, imax, x, ydboff = 0.0;;

  if(scale_type == SCALE_DB) ydboff = reference_level;

  switch(line_type) {
  case 1:			/* standard solid line type */
  default:
    if(incr < 1.0){		/* must do max-min computation? */
      for(k=0, j = xoffset, sump=0.0,
	  p = (double *) s_data + index ;    k < npix;
	  j++, k++, sump -= 1.0) {
	for(imax = imin = *p; sump < 1.0; sump += incr ) {
	  if(*++p > imax)imax = *p;
	  else
	    if(*p < imin)imin = *p;
	}
	pw_vector(pw, j, scoff(imax), j, scoff(imin),
		  PIX_COLOR(val)|PIX_SRC, val);
      }
    } else {			/* no need for max-min */
      for(k = xoffset, x = xoffset + incr,
	  p = (double *) s_data + index,
	  r = p + 1,  q = p + nsam - 1; p < q; x += incr) {
	j = x;
	pw_vector(pw, k, scoff(*p++), j, scoff(*r++),
		  PIX_COLOR(val)|PIX_SRC, val);
	k = j;
      }
    }
    break;
  case 2:			/* histogram-style */
    if(incr < 1.0){		/* must do max-min computation? */
      for(k=0, j = xoffset, sump=0.0,
	  y = scoff(ymin),
	  p = (double *) s_data + index ;    k < npix;
	  j++, k++, sump -= 1.0) {
	for(imax = *p; sump < 1.0; sump += incr )
	  if(*++p > imax)imax = *p;
	pw_vector(pw, j, scoff(imax), j, y,
		  PIX_COLOR(val)|PIX_SRC, val);
      }
    } else {			/* no need for max-min */
      for(k = xoffset, x =  xoffset + incr,
	  y2 = scoff(ymin),
	  p = (double *) s_data + index,
	  r = p + 1,  q = p + nsam; p < q; x += incr) {
	j = x;
	pw_vector(pw, k, (y = scoff(*p++)), j, scoff(*r++),
		  PIX_COLOR(val)|PIX_SRC, val);
	pw_vector(pw, k, y, k, y2, PIX_COLOR(val)|PIX_SRC, val);
	k = j;
      }
    }
    break;
  }
}

/*********************************************************************/

int
plot_trace(pw, xoff, yoff, xscale, yscale, xlow, xhi, ylow, trace, color)
    Xv_Window	pw;
    int		xoff, yoff;
    double	xscale, yscale, xlow, xhi, ylow;
    Trace	*trace;
    int		color;
{
  int		i, n, xo, yo, x, y, index, npix, ndata;
  double	*freqs, *data, hoff, voff, ydboffset = 0.0;
  Rect *rec = (Rect *) xv_get(pw, WIN_RECT);

  if (!(trace && (data = trace->data) && (n = trace->n)))
    return FALSE;

  if(trace->scale_type == SCALE_DB)
    ydboffset = reference_level;

  if(freqs = trace->freqs) {	/* enumerated frequency points */
    hoff = 0.5 + xoff - xscale * xlow; /* 0.5 for rounding */
    voff = 0.5 + yoff - yscale * ylow; /* 0.5 for rounding */

    xo = hoff + xscale * *freqs++;
    if ((yo = voff + yscale * (*data++ - ydboffset)) > yoff) yo = yoff;
    for(i = 1; (i < n) && (xo < rec->r_width); i++)	{
      x = hoff + xscale * *freqs++;
      if ((y = voff + yscale * (*data++ - ydboffset)) > yoff) y = yoff;
      if(xo >= xoff)
	pw_vector(pw, xo, yo, x, y, PIX_COLOR(color)|PIX_SRC, color);
      xo = x; yo = y;
    }
  } else {     /* linearly-spaced frequency points */
    double diff, incr, dhi = trace->band_low + trace->band;

    if(xlow <= trace->band_low) {
      hoff = xoff + ((rec->r_width - xoff)*
		     (trace->band_low - xlow)/(xhi - xlow));
      index = 0;
      if(xhi >= dhi) {
	ndata = n;
	npix = 0.5 + (trace->band * (rec->r_width - xoff)/(xhi - xlow));
      } else {
	ndata = 0.5 + (n * (xhi - trace->band_low)/trace->band);
	npix = rec->r_width - hoff;
      }
    } else {
      index = 0.5 + (n*(xlow - trace->band_low)/trace->band);
      hoff = xoff;
      if(xhi >= dhi) {
	ndata = n - index;
	npix = 0.5 + ((rec->r_width - xoff)*(dhi - xlow)/(xhi - xlow));
      } else {
	ndata = 0.5 + (n * (xhi - xlow)/trace->band);
	npix = rec->r_width - xoff;
      }
    }
	
/*
    if((diff = trace->band_low - xlow) <= 0.0) {
      index = diff*(1-n)/trace->band;
      hoff = xoff;
      ndata = 1 + (n*(xhi - xlow)/trace->band);
      if(ndata > n)
	ndata = n;
    } else {
      ndata = n;
      hoff = 0.5 + xoff + xscale * (trace->band_low - xlow);
      index = 0;
    }
    npix = rec->r_width - hoff;
*/
    incr = ((double)npix)/ndata;

    voff = 0.5 + yoff - yscale * ylow; /* 0.5 for rounding */

    plot_a_trace(color, data, index, 1, hoff, yscale,
		 voff, incr, npix, ndata, pw, ylow);
    }

  return TRUE;
}

/*********************************************************************/
/* Note that this reads n+1 samples so that an extra is available for preemp. */
Signal *
read_signal_segment(ob, w_size, n) /* see Signals.h and Utils.h */
    Objects *ob;
    double  w_size;
    int     *n;			/* # samples for w_size; (n+1 are read) */
{
  Signal *s;
  double stime;

  if(ob && ob->name && ob->name[0]) {
    if(! (s = ob->sig)) {	/* should have been created already... */
      sprintf(notice_msg,
	      "No signal present for object %s in read_signal_segment.",
	      ob->name);
      show_notice(1, notice_msg);
      return(NULL);
    }
    stime = ob->time - (0.5 * w_size); /* center window on cursor */

    /* we process periodic shorts for SIGnal files or any FEA_SD file */

    if ((s->dim == 1) && IS_GENERIC(s->type) &&
        (((s->type & VECTOR_SIGNALS) == P_SHORTS) || is_feasd(s)))
    {
      *n = (w_size * s->freq) + .5;

      if (debug_level > 1)
	fprintf(stderr,
		"read_signal_segment: stime = %g, w_size = %g, n = %d\n",
		stime, w_size, *n);

      if ((BUF_START_TIME(s) != stime ||
	   stime + w_size + 1.0/s->freq > BUF_END_TIME(s)) &&
	  ! s->utils->read_data(s, stime, w_size + 1.0/s->freq))
      {
	sprintf(notice_msg, "Problems in read_data(%d, %f, %f).",
		s, ob->time, w_size + 1.0/s->freq);
	show_notice(1, notice_msg);
	return(NULL);
      }
      close_sig_file(s);	/* close file to conserve file descriptors */
      return(s);
    }
    else if ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM)
    {
      /* get spectral slice for one time period */

      if (s->start_time <= ob->time
	  && ob->time + 1.0/s->freq <= s->start_time + s->file_size/s->freq
	  && s->utils->read_data(s,ob->time,1.4/s->freq))
      {
	*n = s->buff_size;
	close_sig_file(s);
	return(s);
      } else {
	sprintf(notice_msg,
		"Problems in read_data(sig=%s,time=%f,dur=%f).",
		s->name, ob->time, 1.4/s->freq);
	show_notice(1, notice_msg);
      }
    } else {
      sprintf(notice_msg, "Can't process data type %x yet!", s->type);
      show_notice(1, notice_msg);
    }
  } else
    show_notice(1, "Bad arguments to read_signal_segment().");

  return(NULL);
}

/*********************************************************************/
/* Compute the residual time series resulting from applying the current set of
   LPC coefficients to a region of the original waveform around the point
   at which the LP model was estimated. */
invfil(o)
     Objects *o;
{
  Signal *s, *sn;
  register int i, j, k, l, ordc;
  register double sum, *dp, *dp2, *de, out, sum2;
  register short *r;
  double *scr,amax,amin,scale;
  int n;
  short **spp, *p, *q, *datas;
  char newn[500];
  Signal *sig_in;
  struct header *ehd;
  double stime = 0.0;
  short in_type;

  sig_in = o->sig;


  /* since much of this routine assumes SHORT sampled data, we force
     FEA_SD files to be read in as such; we switch back before returning
  */

  if((sig_in) &&
     (sig_in->dim == 1) &&
     (((sig_in->type & VECTOR_SIGNALS) == P_SHORTS) || is_feasd(sig_in)) &&
     (s = read_signal_segment(o,i_f_dur,&n))) {
      o->vers++;
      if((spp = (short**)malloc(sizeof(short*))) &&
	 (datas = (short*)malloc(sizeof(short)*n)) &&
	 (*spp = (short*)malloc(sizeof(short)*n)) &&
	 (scr = (double*)malloc(sizeof(double)*n))) {
	  sprintf(newn,"%s%d",s->name,o->vers);

	  /* adjust for output_dir, if it exists */

	  setup_output_dir(newn);

	  if((sn = new_signal(newn,SIG_NEW,dup_header(s->header),spp,n,
			      s->freq,1))) {

	      if (is_feasd(sig_in)) {
		  /* if the input was FEA_SD, we go through some
		     contortions; converting to SHORT and then
		     setting enough things so that put_signal
		     writes shorts
		     */

		  if (debug_level > 1)
		    fprintf(stderr,
			    "invfil: creating new FEA_SD header for output\n");

		  ehd = new_header(FT_FEA);
		  (void) strcpy(ehd->common.prog, "xspectrum");
		  add_comment(ehd, "inverse filter output");

		  stime = BUF_START_TIME(s);

		  init_feasd_hd(ehd, SHORT, (int) 1, &stime, (int) 0,
				(double) sig_in->freq);

		  sn->header->esps_hdr = ehd;

		  if (!get_genhd_val("encoding",sig_in->header->esps_hdr,(double)0))
		    in_type = get_fea_type("samples", sig_in->header->esps_hdr);
	 	  else
		    in_type = SHORT;

		  if (debug_level > 1)
		    (void) fprintf(stderr,
				   "invfil: converting from type %d to %d\n",
				   in_type, SHORT);

		  type_convert((long) (n), ((char**)s->data)[0], in_type,
			       datas, SHORT, NULL);

		  sn->type = P_SHORTS;

		  if (in_type != SHORT)
		    sn->types[0] = P_SHORTS;

		  q = datas;

	      }
	      else
		  q = *((short**)s->data);

	      if (debug_level > 1) {
		(void) fprintf(stderr,
		       "invfil: first and last points are %d and %d\n",
			q[0], q[n-1]);
	      }

	      /* Compute the inverse-filtered signal: */
	      for(i=0, dp2=scr; i < order; i++) *dp2++ = 0;
	      for(out = 0.0, ordc = order, de = o->data2+order,amax = -2.0e30,
		  amin = -amax, sum2 = 0.0 ; i < n; i++) {
		  for(j=0, sum=0.0, dp=de, r=q++; j < ordc; j++)
		    sum += *dp-- * *r++;
		  sum += *r;
		  *dp2++ = out = (sum + i_f_int * out);
		  sum2 += out;
		  if(out > amax) amax = out;
		  else if(out < amin) amin = out;
	      }


	      sum2 /= n;
	      /* Arbitrarily scale output to cover specified range. */
	      scale = 16000.0/(amax-amin);
	      for(i=0,dp2=scr, p = *spp; i < n; i++) {
		  *p++ = 0.5 + scale*(*dp2++ - sum2);
	      }
	      free(scr);
	      get_maxmin(sn);
	      /* Save it in a SIGnal file: */

	      if (debug_level > 1) {
		(void) fprintf(stderr,
		       "invfil: 10th and 11th postfiltered points are %d and %d\n",
			p[9], p[10]);
	      }

	      head_printf(sn->header,"samples",&n);
	      sn->start_time = BUF_START_TIME(s);
/*	      sn->start_time = 0.0;*/
	      sn->end_time = sn->start_time + BUF_DURATION(sn);
	      head_printf(sn->header,"start_time",&(sn->start_time));
	      head_printf(sn->header,"end_time",&(sn->end_time));
	      sprintf(newn,
		"invfil: order %d preemp %6.4f wsize %f time %f i_f_int %6.4f",
		      order,preemp,w_size,o->time,i_f_int);
	      head_printf(sn->header,"operation",newn);
	      head_printf(sn->header,"maximum",sn->smax);
	      head_printf(sn->header,"minimum",sn->smin);
	      put_signal(sn);
	      /* Send a message to waves to display it: */
	      sprintf(newn,"%s make name %s file %s\n", host, o->name, sn->name);
	      mess_write(newn);
	      free_signal(sn);
	      return(TRUE);
	  } else {
	    sprintf(notice_msg, "Can't make new signal %s.", newn);
	    show_notice(1, notice_msg);
	  }
      } else
	show_notice(1, "Allocation failure in invfil().");
  } else {
    sprintf(notice_msg, "Problems reading %s (%f %f).",
	    sig_in->name, o->time, i_f_dur);
    show_notice(1, notice_msg);
  }
  return(FALSE);
}
	
/*********************************************************************/
/* Free a Trace structure */

void
free_trace(tr)
    Trace   *tr;
{
    if (!tr) return;
    if (tr->data) free((char *) tr->data);
    if (tr->freqs) free((char *) tr->freqs);
    free((char *) tr);
}

/* Create an output Trace structure and return a pointer to it.
   Reuse existing structure if any.  NULL on error. */

Trace *
new_trace(tr, n, want_freqs)
    Trace   *tr;
    int	    n;
    int	    want_freqs;
{
    if (!tr)
    {
	tr = (Trace *) malloc(sizeof(Trace));
	if (!tr) return NULL;
	tr->data = NULL;
	tr->freqs = NULL;
    }

    if (tr->freqs)
    {
	free((char *) tr->freqs);
	tr->freqs = NULL;
    }

    if (tr->n != n)
    {
	if (tr->data)
	{
	    free((char *) tr->data);
	    tr->data = NULL;
	}
	if (tr->freqs)
	{
	    free((char *) tr->freqs);
	    tr->freqs = NULL;
	}
    }

    if (!tr->data
	&& !(tr->data = (double *) malloc(n * sizeof(double))))
    {
	free_trace(tr);
	return NULL;
    }

    if (want_freqs && !tr->freqs
	&& !(tr->freqs = (double *) malloc(n * sizeof(double))))
    {
	free_trace(tr);
	return NULL;
    }
    if (!want_freqs && tr->freqs)
    {
	free((char *) tr->freqs);
	tr->freqs = NULL;
    }

    tr->n = n;
    return tr;
}

/* Create an output Trace structure and set ob's pointer to it.
   Also return a pointer to the structure.  NULL on error. */

Trace *
make_output_array(ob, out, want_freqs)
    Objects *ob;
    int	    out;
    int	    want_freqs;
{
    return ob->trace[0] = new_trace(ob->trace[0], out, want_freqs);
}


/*
 * compute max entropy spectrum via reflection coeffients computed by
 * various ESPS methods  - j. shore
 */

esps_compute_spect(ob)
     Objects *ob;
{
  int n, i, j, out, ffto, pow2, ncpx;
  char *datai;
  short in_type;
  Trace  *trace;
  double *datao;
  double *x, *y, form[30],band[30], err, energy, rms;
  static double *r;
  static float *rc, *xf, *dataf, *sdataf;
  Signal *s;
  int strcov_iter = 20;
  double strcov_conv = 1e-5;
  int  sincn = 0;
  float gain;
  static int last_n = 0;
  static int last_order = 0;

  if (debug_level)
    fprintf(stderr, "entered esps_compute_spect()\n");

  if((s = read_signal_segment(ob,w_size,&n))) {

    if (order >= n) {
	sprintf(notice_msg, "%s: %s %d %s\n%s (%d).",
		"xspectrum", "Frame size", n, "too small -",
		"require more points than order", order);
	show_notice(1, notice_msg);

	return(FALSE);
    }

    ffto = 0;			/* fft order */
    pow2 = 1;			/* resultant exponential */
    make_arrays(def_fft_size,&ffto,&pow2,&x,&y);
    out = (pow2/2) + 1;

    datai = ((char **)s->data)[0];

    if ((trace = make_output_array(ob, out, NO)) && (datao = trace->data)) {
      if(got_enough(s->buff_size, &n)) {
	  if (n > last_n) {
	      if(dataf)
		free(dataf);
	      if(sdataf)
		free(sdataf);
	      dataf = sdataf = NULL;
	      if(!((dataf = (float*)malloc(sizeof(float)*(n+1))) &&
		   (sdataf = (float*)malloc(sizeof(float)*(n+1)))))
	      {
		  show_notice(1, "Allocation problems in esps_comput_spect.");

		  if (debug_level)
		  {
		      fprintf(stderr,
			      "esps_compute_spect: Allocation problems -\n");
		      fprintf(stderr, "\tdataf = %d, sdataf = %d.\n",
			      sizeof(dataf), sizeof(sdataf));
		  }
		  return(FALSE);
	      }
	      last_n = n;
	  }

	  if (order > last_order) {

	      if(rc)
		free(rc);
	      if(r)
		free(r);
	      if(xf)
		free(xf);
	      r = NULL; rc = xf = NULL;
	      if (debug_level)
		(void) fprintf(stderr, "re-allocating arrays for order\n");

	      if(!((rc = (float*)malloc(sizeof(float)*(order + 1))) &&
		   (r = (double*)malloc(sizeof(double)*(order + 1))) &&
		   (xf = (float*)malloc(sizeof(float)*(order + 1))))) {
		  show_notice(1,
			      "Allocation problems in esps_compute_spect.");
		  return(FALSE);
	      }
	      last_order = order;
	  }

	  if (is_feasd(s)
	      && !get_genhd_val("encoding",s->header->esps_hdr,(double)0))
	    in_type = get_fea_type("samples", s->header->esps_hdr);
	  else
	    in_type = SHORT;

	  if (debug_level > 1)
	    (void) fprintf(stderr,
		   "esps_compute_spect: converting type %d to %d\n",
			   in_type, FLOAT);

	  type_convert((long) (n+1), datai, in_type, dataf, FLOAT, NULL);

	  if (debug_level > 1)
	    (void) fprintf(stderr,
		"esps_compute_spect: sd[0] = %g, sd[n-1] = %g, last_n = %d\n",
			  dataf[0], dataf[n-1], last_n);

	  fwindow_f(dataf, sdataf, n, preemp, weight_type);
	
	  if (debug_level)
	    fprintf(stderr, "calling compute_rc()\n");

	  if (compute_rc(sdataf, n, esps_spect_method, 0, WT_RECT, order,
			 sincn, strcov_iter, strcov_conv, rc, r, &gain) == -1)
	  {
	      sprintf(notice_msg,
		      "%s: problem computing reflection coefficients.",
		      "xspectrum");
	      show_notice(1, notice_msg);
	      return(FALSE);
	  }

	  rctoc(rc, order, xf, &gain);
                                /* rctoc indices start at 1 */

	  /* correct sign convention, etc. */

	  xf[0] = 1.0;
	  for(i=1; i <= order; i++)
	    xf[i] = -xf[i];

	  if (debug_level > 2) {
	    pr_farray(xf, order+1, "esps_c:");
	    (void) fprintf(stderr, "r[0] = %g, gain = %g\n",
			   r[0], gain);
	  }

	  for(i=0; i <= order; i++) {
	    x[i] = (double) xf[i];
	    y[i] = x[i];
	  }

	  /* formant/bandwidth estimating and printing */
	  if(fb_flag == 1) {
	      if (order <= 29) {
		  formant(order,s->freq,y,&ncpx,form,band,1);
		  printf("\nFREQUENCIES	BANDWIDTHS (%s - order %d)\n",
			 ana_methods[esps_spect_method], order);
		  for(i=0; i < ncpx; i++)
		    printf("%9.3f	%9.3f\n",form[i],band[i]);
	      }
	      else
	      {
		  sprintf(notice_msg,
			  "%s: can't do formants for order > 29.",
			  "xspectrum");
		  show_notice(0, notice_msg);
	      }
	  }

	  for(i=0; i <= order; i++) {
	      y[i] = 0.0;
	      ob->data2[i] = x[i];
	    }
	  for( ; i < pow2; i++) x[i] = y[i] = 0.0;
	  get_rfftd(x, y, ffto);

	  scale_mag(x, y, datao, out, r[0]*(n-order)*gain, scale_type, YES);

	  trace->band_low = 0.0;
	  trace->band = s->freq/2.0;
	  trace->scale_type = scale_type;
	  return(TRUE);
	} else {
	    show_notice(1, "Error reading data in esps_compute_spect.");
	    if (debug_level)
		fprintf(stderr,
			"%s: Error reading data; n:%d; s->buff_size:%d.",
			"esps_compute_spect", n, s->buff_size);
	}
    } else
      show_notice(1, "Allocation problems in esps_compute_spect.");
  }
  return(FALSE);
}

/*********************************************************************/
/* Simple log-magnitude DFT (done the hard way). */
log_mag_dftr(ob)
     Objects *ob;
{
  int n, i, j, k, out;
  char *datai;
  short in_type;
  Trace  *trace;
  double *datao, sum, dif;
  static double *datad=NULL, *x=NULL, *re=NULL, *im=NULL;
  static int size=0;
  Signal *s, *read_signal_segment();

  if((s = read_signal_segment(ob,w_size,&n))) {
    if(n > 1025) {
      sprintf(notice_msg,
	      "Requested window size (%d) unreasonably large for straight DFT.", n);
      show_notice(1, notice_msg);
      return(FALSE);
    }
    if (is_feasd(s) && !get_genhd_val("encoding",s->header->esps_hdr,(double)0))
      in_type = s->header->esps_hdr->hd.fea->types[0];
    else
      in_type = SHORT;

    out = (n/2) + 1;		/* # of spectral points (includes 0Hz) */
    if(size < n) {
      if(x) {
	free(x);
	if(re)
	  free(re);
	if(im)
	  free(im);
	x = re = im = NULL;
	size = 0;
      }
      if(!((x = (double*)malloc(sizeof(double)*n)) &&
	   (datad = (double*)malloc(sizeof(double)*n)) &&
	   (re = (double*)malloc(sizeof(double)*out)) &&
	   (im = (double*)malloc(sizeof(double)*out)))) {
	show_notice(1, "Allocation problems in log_mag_dftr.");
	return(FALSE);
      }
      size = n;
    }
    datai = ((char**)s->data)[0];

    type_convert((long) (n), datai, in_type, datad, DOUBLE, NULL);

    trace = make_output_array(ob, out, NO);
    if(datai && trace && (datao = trace->data)) {
      if(got_enough(s->buff_size, &n)) {

	  fwindow_d(datad, x, n, preemp, weight_type);
	  dft(n,x,re,im);
	  scale_mag(re, im, datao, out, 1.0, scale_type, NO);

	  trace->band_low = 0.0;
	  trace->band = s->freq/2.0;
	  trace->scale_type = scale_type;
	  return(TRUE);
	} else {
	    show_notice(1, "Error reading data in log_mag_dftr.");
	    if (debug_level)
		fprintf(stderr,
			"%s: Error reading data; n:%d; s->buff_size:%d\n",
			"log_mag_dftr", n, s->buff_size);
	}
    } else
      show_notice(1, "Allocation problems in log_mag_dftr.");
  } else
      show_notice(1, "Problems with read_signal_segment() in log_mag_dftr.");
  return(FALSE);
}

/*********************************************************************/
/* Simple log-magnitude DFT using radix-2 FFT of def_fft_size points or more. */
log_mag_dft(ob)
     Objects *ob;
{
  int n, i, j, k, out;
  char *datai;
  short in_type;
  Trace  *trace;
  double *datad, *datao;
  double *x, *y, z[4096];
  Signal *s, *read_signal_segment();
  FILE *fopen(), *fd;
  int m;

  if((s = read_signal_segment(ob,w_size,&n))) {
    i=def_order;			/* start with minimum FFT size of def_fft_size */
    j= def_fft_size;
    if(make_arrays(n,&i,&j,&x,&y)) { /* get size of fft required. */

      datad = (double*)malloc(sizeof(double)*n);

      while(j < n) {
	j *= 2;
	i++;
      }
      out = (j/2) + 1;		/* # of spectral points (includes 0Hz) */

      datai = ((char**)s->data)[0];

      trace = make_output_array(ob, out, NO);
      if(datai && trace && (datao = trace->data)) {
	if(got_enough(s->buff_size, &n)) {
	  for(k=0; k < j ; k++)  x[k] = y[k] = 0.0;

	  if (is_feasd(s) && !get_genhd_val("encoding",s->header->esps_hdr,(double)0))
	    in_type = s->header->esps_hdr->hd.fea->types[0];
	  else
	    in_type = SHORT;	

	  type_convert((long) (n), datai, in_type, datad, DOUBLE, NULL);

	  fwindow_d(datad, x, n, preemp, weight_type);

	  get_rfftd(x,y,i);
/*	  phase(x,y,z,out); */

	  scale_mag(x, y, datao, out, 1.0, scale_type, NO);

/*
	  if((fd = fopen("polar.out","w"))) {
	    double fac = log(10.0)/20.0;
	    for(k=0;k<out;k++)
	      fprintf(fd,"%f %f\n",datao[k]*fac,z[k]);
	    fclose(fd);
	  }
*/
	  trace->band_low = 0.0;
	  trace->band = s->freq/2.0;
	  trace->scale_type = scale_type;
	  return(TRUE);
	} else {
	    show_notice(1, "Error reading data in log_mag_dft.");
	    if (debug_level)
		fprintf(stderr,
			"%s: Error reading data; n:%d; s->buff_size:%d\n",
			"log_mag_dft", n, s->buff_size);
	}
      } else
	  show_notice(1, "Allocation problems in log_mag_dft.");
    }
  }
  return(FALSE);
}

/* phase(x,y,z,n)
     register double *x, *y, *z;
     register int n;
{
  for(; n-- ;) *z++ = atan2(*y++,*x++);
}
*/


/* MIT */
/*********************************************************************/
/* Cepstrally smoothed log-magnitude. */
/* Simple log-magnitude DFT using radix-2 FFT of def_fft_size points or more. */
log_cepstrally_smoothed(ob)
     Objects *ob;
{
  int n, i, j, k, out, iord;
  char *datai;
  short in_type;
  Trace  *trace;
  double *datad, *datao;
  double *x, *y, z[4096];
  Signal *s, *read_signal_segment();
  FILE *fopen(), *fd;

  if((s = read_signal_segment(ob,w_size,&n))) {
    iord=def_order;		/* start with minimum FFT size of def_fft_size */
    j= def_fft_size;
    if(make_arrays(n,&iord,&j,&x,&y)) { /* get size of fft required;
					     max size set by "thetable.c"
					     (easily changed) */

      datad = (double*)malloc(sizeof(double)*n);

      while(j < n) {
	j *= 2;
	i++;
      }
      out = (j/2) + 1;		/* # of spectral points (includes 0Hz) */

      datai = ((char**)s->data)[0];

      trace = make_output_array(ob, out, NO);
      if(datai && trace && (datao = trace->data)) {
	if(got_enough(s->buff_size, &n)) {
	  for(k=0; k < j ; k++)  x[k] = y[k] = 0.0;

	  if (is_feasd(s) && !get_genhd_val("encoding",s->header->esps_hdr,(double)0))
	    in_type = s->header->esps_hdr->hd.fea->types[0];
	  else
	    in_type = SHORT;	

	  type_convert((long) (n), datai, in_type, datad, DOUBLE, NULL);

	  fwindow_d(datad, x, n, preemp, weight_type);

          {
	      float *xf, *yf;
	      int cep_cutoff_i = cep_cutoff * s->freq;
	      int trans_i = cep_trans * s->freq;
	      int zeros, nzeros;

	      if(cep_cutoff_i > j/2)
		cep_cutoff_i = j/2;

	      if(trans_i/2 > cep_cutoff_i)
		trans_i = 2*cep_cutoff_i;
	      if(trans_i/2 > ((j/2) - cep_cutoff_i - 1))
		trans_i = 2*((j/2) - cep_cutoff_i - 1);

	      xf = (float *) malloc(j * sizeof(float));
	      yf = (float *) malloc(j * sizeof(float));
	
	      if (!xf || !yf) {
		  printf("couldn't allocate float arrays\n");
		  return FALSE;
	      }
	
	      for (k = 0; k < j; k++) {
		  xf[k] = (float) x[k];
		  yf[k] = (float) y[k];
	      }
	
	      fft_cepstrum_r(xf, yf, iord);
	
	      /* cepstral liftering */
	      switch (cep_liftering) {	
	      case 1: /* lowpass */
		{
		 int zeros = j - (2*cep_cutoff_i) - 1 - trans_i;
		 if(debug_level)
		   fprintf(stderr,
			   "zeros:%d trans_i:%d j:%d cep_cutoff_i:%d\n",
			   zeros,trans_i,j,cep_cutoff_i);
		 k = cep_cutoff_i + 1 - (trans_i)/2;
		  if(trans_i > 0) {
		    double arg = M_PI/trans_i, fac;
		    for(i=0; i < trans_i; i++, k++) {
		      fac = .5 + (.5 * cos(arg * i));
		      xf[k] *= fac;
		      yf[k] *= fac;
                      xf[j - k - 1] *= fac;
                      yf[j - k - 1] *= fac;
		    }
		  }
		 for(i=0; i < zeros; i++, k++)
		      xf[k] = yf[k] = 0.0;
		  break;
		}
	      case 2: /* highpass */
		{
		  int zeros = cep_cutoff_i - (trans_i/2);

		  if(debug_level)
		    fprintf(stderr,
			    "zeros:%d trans_i:%d j:%d cep_cutoff_i:%d\n",
			    zeros,trans_i,j,cep_cutoff_i);
		  for (k = 0; k < zeros; k++) {
		      xf[k] = yf[k] = 0.0;
		      xf[j - k - 1] = yf[j - k - 1] = 0.0;
		    }
		  if(trans_i > 0) {
		    double arg = M_PI/trans_i, fac;
		    for(i=0; i < trans_i; i++, k++) {
		      xf[k] *= (fac = .5 - (.5 * cos(arg * i)));
		      yf[k] *= fac;
		      xf[j - k - 1] *= fac;
		      yf[j - k - 1] *= fac;
		    }
		  }
		  break;
		}
	      default:
		  break;
	      }
	
	      get_fft(xf, yf, iord);

	      /* log-spectrum -> spectrum because we may take log later */
	      for (k = 0; k < j; k++) {
		  x[k] = exp(xf[k]);
		  y[k] = 0;
	      }

	      free(xf);
	      free(yf);
	  }

	  scale_mag(x, y, datao, out, 1.0, scale_type, NO);

	  trace->band_low = 0.0;
	  trace->band = s->freq/2.0;
	  trace->scale_type = scale_type;
	  return(TRUE);
	} else
	  printf("Error reading data; n:%d; s->buff_size:%d\n",n,s->buff_size);
      } else
	printf("Couldn't allocate working array\n");
    }
  }
  return(FALSE);
}


/*********************************************************************/
scale_mag(x, y, z, n, scale, type, invert)
    double  *x, *y, *z;
    int	    n;
    double  scale;
    int	    type, invert;
{
    int	    i;
    double  xi, yi, pwr;

    if (!x || !y || !z || !n || !scale) return FALSE;

    switch (type)
    {
    case SCALE_DB:
	scale = 10.0 * log10(scale);
	if (invert){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale - 10.0 * log10(pwr)) : (scale + 200.0);
	  }
	}
	else if (scale != 0.0){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale + 10.0 * log10(pwr)) : (scale - 200.0);
	  }
	}
	else{
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (10.0 * log10(pwr)) : -200;
	  }
	}
	if (debug_level > 1)
	    fprintf(stderr, "log pwr = %g dB\n", scale);
	break;
    case SCALE_MAG:
	scale = sqrt(scale);
	if (invert){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale / sqrt(pwr)) : DBL_MAX;
	  }
	}
	else if (scale != 1.0){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale * sqrt(pwr)) : 0.0;
	  }
	}
	else{
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (sqrt(pwr)) : 0.0;
	  }
	}
	if (debug_level > 1)
	    fprintf(stderr, "sqrt(pwr) = %g dB\n", scale);
	break;
    case SCALE_PWR:
	if (invert){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale / pwr) : DBL_MAX;
	  }
	}
	else if (scale != 1.0){
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? (scale * pwr) : 0.0;
	  }
	}
	else{
	  for(i=0; i<n; i++){
	    pwr = x[i] * x[i] + y[i] * y[i];
	    z[i] = (pwr) ? pwr : 0.0;
	  }
	}
	if (debug_level > 1)
	    fprintf(stderr, "pwr = %g dB\n", scale);
	break;
    default:
	return FALSE;
    }

    return TRUE;
}

/*********************************************************************/
/* Display slice spectrogram, eih or other SIG_SPECTROGRAM-type signal */
use_sig_data(ob)
    Objects *ob;
{
    Signal  *s;
    int	    n, count;
    Trace   *trace;
    double  *dst, *dsrc;
    char    **src;

    if (ob && ob->sig)
    {
	if ((s=read_signal_segment(ob,w_size,&n)) && n>0)
	{
	  if (debug_level)
	    (void) fprintf(stderr,
			   "xspectrum/use_sig_data: %s, nsamp %d, dim %d\n",
			   s->name,n,s->dim);

			/* Include a freqs array in new trace if signal
			   has non-NULL y_dat.  If read_esps_hdr ever
			   starts supplying y_dat unnecessarily, will
			   need a better test here to avoid needless
			   labor later.  */
	    if ((trace = make_output_array(ob, s->dim, !!s->y_dat))
		&& (dst = trace->data)
		&& (src = (char **)s->data))
	    {
		for (count = s->dim; count--; )
		    *dst++ = (double)**(src++);
		if ((dst = trace->freqs) && (dsrc = s->y_dat))
		    for (count = s->dim; count--; )
			*dst++ = *dsrc++;
	    }
	    trace->band_low = s->band_low;
	    trace->band = s->band;
/*!*//* Fix for other ESPS spec types */
	    trace->scale_type = SCALE_DB;
	    convert_trace(trace, scale_type);
	    return TRUE;
	}
    }
    else
	show_notice(1, "No object or no signal in use_sig_data.");
    return FALSE;
}

/*********************************************************************/
kill_proc()			/* tell host process we're leaving */
{
  char mess[100];
  extern char *registry_name;

  sprintf(mess,"%s disconnect function %s\n",host,thisprog);
  terminal_message(mess);
  terminate_communication(registry_name);
  if(debug_level)
    fprintf(stderr,"%s is now exiting.\n",thisprog);
  exit(0);
}

/*********************************************************************/
/* Send a command to the host process.  This routine might be used to
   send messages like: "foo mark file foo.d time .34 color 4"
 */
waves_send(name,file,command,color,time)
     char *name, *file, *command;
     int color;
     double time;
{
  char mes[200];

  sprintf(mes,"%s %s file %s time %f color %d\n",
	  name,command,file,time,color);
  mess_write(mes);
  return(TRUE);
}

/*********************************************************************/
/* Here is a typical method which responds to messages from the host: */
char *
meth_mark(ob, args) /* responds to the "mark" message from host to */

     Objects *ob;   /* display object. */
     char *args;
{
  rstart = rend = NOT_SET;
  m_time = 0.0;	       /* preset optional arguments to reasonable values */
  color = MARKER_COLOR;
  *file = 0;

  if (debug_level)
    (void) fprintf(stderr,
	    "Received: %s mark %s\n",ob->name,args);

  if(get_args(args, &a0)) {	/* returns # of arguments read (see parse.c) */
    if(! *file) {
      if(ob->sig && ob->sig->name)
	strcpy(file, ob->sig->name); /* assume filename == signal name */
    }
    if(do_implied_analysis(ob,file,rstart,rend,m_time))
      return(Ok);
  }
  return(Null);
}


/*********************************************************************/
do_implied_analysis(ob,file,rstart,rend,time)
     Objects *ob;
     char *file;
     double rstart, rend, time;
{
  if((rstart > NOT_SET) && (rend > NOT_SET) && (rstart < rend) && do_region) {
    char mess[20];
    ob->time = 0.5 * (rstart + rend);
    w_size = rend - rstart;
    sprintf(mess,"%7.6f",w_size);
    xv_set(wsize_item,PANEL_VALUE,mess, 0);
  } else
    ob->time = time;

  if (check_signal(ob, file) && do_function(ob)) {
    redo_display(ob);
    show_window(ob);
    return(TRUE); /* respond to host process (which is waiting) */
  }
  return(FALSE);
}

/*********************************************************************/
char *
meth_mark_obj(ob, args) /* responds to the "mark" message from host to */
     Objects *ob;  /* Program object */
     char *args;
{
  /* preset optional arguments to reasonable values */
  rstart = rend = -1.234;
  m_time = 0.0;	
  color = MARKER_COLOR;
  *file = *name = 0;

  if(get_args(args, &a0) &&
     *name && (ob = (Objects*)get_receiver(name))) {
    if(! *file) {
      if(ob->sig && ob->sig->name)
	strcpy(file, ob->sig->name); /* assume filename == signal name */
    }
    if(do_implied_analysis(ob,file,rstart,rend,m_time)) {
      if( ! ob->init_located )
	get_display_attributes(ob);
      return(Ok);
    }
  }
  return(Null);
}

/*********************************************************************/
/* Send messages to the host to move the markers to the window edges
   used in the most recent spectrum computation. */
show_window(ob)
     Objects *ob;
{
  char mess[200];
  double stime, etime;

  if (debug_level)
    (void) fprintf(stderr, "Entered show_window\n");

  stime = ob->time - w_size*0.5;
  etime = ob->time + w_size*0.5;

  sprintf(mess,"%s marker time %f do_left 1\n",ob->name,stime);
  mess_write(mess);

  sprintf(mess,"%s marker time %f do_left 0\n",ob->name,etime);
  mess_write(mess);
}

/*********************************************************************/
/* See to it that the signal named "file" is loaded. */
check_signal(ob,file)
     Objects *ob;
     char *file;
{
  if(file && *file && ob) {
    Signal *s;

    if((! ob->sig) || strcmp(ob->sig->name,file)) {
      if(ob->sig) free_signal(ob->sig);
      ob->sig = NULL;
      if(! (ob->sig = get_any_signal(file, 0.0, 0.0, NULL))) {
	return(FALSE);
      }
    }
    return(TRUE);
  }
  return(FALSE);
}


/*********************************************************************/
/* This might close any open files, etc. as needed before exit. */
cleanup()
{
  return;
}

/*********************************************************************/
/* Remove a local view of an object on command from the host process. */
char *
meth_kill(ob, args)
     Objects *ob;
     char *args;
{
  static char file[200];
  static Selector  a0 = {"file", "#qstr", file, NULL};
  char str[10];
  Frame fr;
  Objects *ob2;

  if(ob) {
    if(get_args(args,&a0)) {
      if(ob->sig && (!strcmp(file,ob->sig->name))) { /* just delete the file */
	free_signal(ob->sig);
	ob->sig = NULL;
	return(Ok);
      } else
	return(Null);
    } else {			/* assume entire object is to be removed */
      if(ob->view) {
	xv_set(ob->view, WIN_CLIENT_DATA, NULL, 0);

	fr = (Frame)xv_get(ob->view, XV_OWNER); /* kill its window */
	xv_set(fr, WIN_CLIENT_DATA, NULL, 0);
	save_shared_colormap(ob->view);
	ob->view = NULL;
	xv_set(fr, FRAME_NO_CONFIRM, TRUE, 0);
	xv_destroy_safe(fr);
      }
      if(ob->name)free(ob->name); /* free the data structures */
      ob->name = NULL;
      ob2 = objlist; /* (This needs to be redone (D.T.)); need free_object() */
      if(ob2 == ob)
	objlist = ob->next;
      else
	while(ob2->next) {
	  if(ob2->next == ob) {
	    ob2->next = ob->next;
	    break;
	  }
	  ob2 = ob2->next;
	}
      if(ob->ret) free_reticle(ob->ret);
      ob->ret = NULL;
      free(ob);
    }
  }
  return(Ok);
}

/*********************************************************************/
/* quit on command from host */
char *
meth_quit(ob, args)
     Objects *ob;
     char *args;
{
  extern char *registry_name;

  cleanup();
  terminate_communication(registry_name);
  if(debug_level)
    fprintf(stderr,"Spectrum is now exiting. (2)\n");
  exit(0);
}

/*********************************************************************/
Notify_value
kill_spectral_view(canvas, status)
     Canvas canvas;
     Destroy_status status;
{
  Objects *o;

  if(status == DESTROY_CLEANUP) {
    if((o = (Objects*)xv_get(canvas, WIN_CLIENT_DATA)))
      kill_object(o);
    xv_set(canvas, WIN_CLIENT_DATA, NULL, 0);
  }
  return(notify_next_destroy_func(canvas, status));
}

/**********************************************************************/
/* This is an example of how an interactive data display window might be set
   up.  It creates an object and places a view window on the screen which
   is spatially linked to the view in the host program.  If called with
   insufficient information, it sends a message to the host requesting
   the missing data.  If the object already exists, another one by the same
   name is not created.  In this example, the message may also contain
   a request to "mark" a certain time in the newly created object (this has
   the same affect as meth_mark()).  */
char *
meth_make_object(ob, args)
    Objects	    *ob;
    char	    *args;
{
    int             n_obj = 0;
    int             plot_x, plot_y, plot_height, plot_width;
    Objects	    *ob2, *ob3, *new_objects();
    Canvas	    canvas;
    Frame	    frame;
    int             analysis_req = 0;
    extern int do_color, cmap_depth;

    m_time = rstart = rend = NOT_SET;
    *name = *file = 0;
    start = sec_cm = -1.0;
    color = MARKER_COLOR;
    width = height = loc_x = loc_y = -1;


    if(debug_level && ob)
	fprintf(stderr, "xspectrum/meth_make_object: Received: %s make %s, %d objects\n",
	       ob->name,args, n_obj);

    if (get_args(args,&a0) && *name) /* if there were args. and one was name */
    {
	if(! (ob2 = (Objects *)get_receiver(name))) /* does it already exist?*/
	{
	  if (debug_level)
	    fprintf(stderr,"xspectrum/meth_make_object: Trying to create a new object: %s\n", args);
	  if((ob2 = new_objects(name)))
	  {
	    n_obj = 1;

	    /* count existing objects to get total number */

	    ob3 = objlist;
	
	    while (ob3->next) {
		n_obj++;
		ob3 = ob3->next;
	    }

	    if (debug_level)
	      (void) fprintf(stderr, "meth_make_object: %d objects\n", n_obj);

	    (void) have_all_display_attributes(ob2);

	    if(! *file)
	      strcpy(file, name);
	
	    /* if no specs in .wave_pro (or not first object),
	       we stick to the external view */
	
	    plot_x =
	      ((n_obj == 1) &&(xsp_datwin_x >= 0))
		? xsp_datwin_x
		  : ob2->xloc;
	
	    plot_y =
	      ((n_obj == 1) &&(xsp_datwin_y >= 0))
		? xsp_datwin_y
		  : ob2->yloc;

	    /* height and width from .wave_pro */

	    plot_height =
	      (xsp_datwin_height >= 0 )
		? xsp_datwin_height
		  : 250;

	    plot_width =
	      (xsp_datwin_width >= 0)
		? xsp_datwin_width
		  : 425;

	    limit_location(&plot_x, &plot_y, plot_width, plot_height);

	    if(do_color)
	      frame = xv_create(XV_NULL, FRAME,
			      XV_LABEL,	    file,
			      XV_WIDTH,	    plot_width,
			      XV_HEIGHT,	    plot_height,
			      WIN_X,		    plot_x,
			      WIN_Y,		    plot_y,
			      XV_SHOW,  FALSE,
			      FRAME_INHERIT_COLORS,	FALSE,
			      WIN_CLIENT_DATA,    ob2,
			      0);
	    else
	      frame = xv_create(XV_NULL, FRAME,
			      XV_LABEL,	    file,
			      XV_WIDTH,	    plot_width,
			      XV_HEIGHT,	    plot_height,
			      WIN_X,		    plot_x,
			      WIN_Y,		    plot_y,
			      XV_SHOW,  FALSE,
			      FRAME_INHERIT_COLORS,	TRUE,
			      WIN_CLIENT_DATA,    ob2,
			      0);

	    exv_attach_icon(frame, SPEC_ICON, basename(file), TRANSPARENT);

	    canvas = xv_create(frame, CANVAS,
			XV_X,		    0,
			XV_Y,		    0,
/*			WIN_DEPTH,              cmap_depth, */
			CANVAS_RETAINED,    FALSE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_AUTO_SHRINK, TRUE,
			CANVAS_AUTO_EXPAND, TRUE,
		       CANVAS_CMS_REPAINT, FALSE,
		       CANVAS_PAINTWINDOW_ATTRS,
		       WIN_DYNAMIC_VISUAL, TRUE,
			WIN_DEPTH,              cmap_depth,
		       0,
		       CANVAS_NO_CLIPPING,	TRUE,
			WIN_MENU,	    menu,
			WIN_CLIENT_DATA,    ob2,
			0);

	    xv_set(canvas_paint_window(canvas),
			WIN_CONSUME_EVENTS,
			    LOC_DRAG,
			    LOC_MOVE,
			    WIN_IN_TRANSIT_EVENTS,
		            LOC_WINEXIT,
		            LOC_WINENTER,
			    0,
			WIN_EVENT_PROC,	    doit,
			WIN_BIT_GRAVITY,    ForgetGravity,
			0);
	
	    window_fit(frame);

	    cmap(canvas);

	    notify_interpose_destroy_func(frame, kill_spectral_view);

	    ob2->next = objlist;	/* link in the new object */
	    objlist = ob2;
	    ob2->view = canvas;
/* Don't set repaint proc in xv_create above since call within xv_create
   might make trouble. */
	    xv_set(canvas,
		   CANVAS_REPAINT_PROC, repaint,
		   0);
	  } else {			/* Couldn't create a new Object! */
	      return(Null);
	    }
	}
	/* do analysis on newly-created or existing object */

	if (m_time > NOT_SET)
	  analysis_req = 1;

	if ((rstart > NOT_SET) && (rend > NOT_SET) && (rstart < rend)) {
	  do_region = 1;  /* set host mode */
	  xv_set(limits_item, PANEL_VALUE, do_region, 0);
	  analysis_req = 1;
	}
	
	if (analysis_req) {
	  if(!do_implied_analysis(ob2,file,rstart,rend,m_time))
	    return(Null);
	  else {
	    if(!ob2->init_located)
	      get_display_attributes(ob2);
	    return(Ok);
	  }
	} else
	  return(Ok);  /* no analysis requested; (created obj. only) */
      }
    return(Null);	/* no receiver name was specified for message */
}

/*********************************************************************/
/* Updates data and displays after some change is made to analysis parameters.
 */
recompute_all()
{
  Objects *o = objlist;

  while(o) {
    if(o->sig && do_function(o)) {
      redo_display(o);
      show_window(o);
    }
    o = o->next;
  }
}

/******************************************************************/
Reticle *
new_spectrum_reticle(ob)
     Objects *ob;
{
  Reticle *r;
  Rect *rec;
  double hif, arange;
  Bound *b;
  register double hz_per_pix, db_per_pix, lospac, hispac, range, range10,
  range5, range2;
  int right_of_d;
  char abform[20];

  if (debug_level)
    (void) fprintf(stderr, "Entered new_spectrum_reticle\n");
  if(!(ob && ob->view && ob->trace[0] && ob->trace[0]->data)) return(NULL);
  rec = (Rect *) xv_get(canvas_paint_window(ob->view), WIN_RECT);
  hif = ob->xhigh - ob->xlow;
  arange = ob->yhigh - ob->ylow;
  if(!(r = ob->ret))
    if(r = (Reticle *) calloc(1, sizeof(Reticle))) ob->ret = r;
    else return(NULL);
  /* Want at least 50 pixels between freq. numbering and no more than 100. */
  /* Assume total horizontal pixels = rec->r_width - 40. */
  hz_per_pix = hif/(rec->r_width - 40);
  lospac = 50.0*hz_per_pix;
  hispac = 100.0*hz_per_pix;
  range = pow(10.0, floor(log10(lospac)));
  range10 = 10.0*range;
  range5 = 5.0*range;
  range2 = 2.0*range;

  if (hispac >= range10)
      r->abscissa.maj.inter = range10;
  else if (lospac <= range5 && hispac >= range5)
      r->abscissa.maj.inter = range5;
  else
      r->abscissa.maj.inter = range2;

  if (r->abscissa.maj.inter <= 1.0)
      right_of_d = 1 + (int)(0.5 - log10(r->abscissa.maj.inter));
  else
      right_of_d = 0;

  sprintf(abform,"%s%df","%8.",right_of_d);
  reticle_set_absc_precision(r,abform);

  /* Want at least 20 pixels between amp. numbering and no more than 50. */
  /* Assume total vertical pixels = rec->r_height - 40. */
  db_per_pix = arange/(rec->r_height - 40);
  lospac = 20.0*db_per_pix;
  hispac = 50.0*db_per_pix;
  range = pow(10.0, floor(log10(lospac)));
  range10 = 10.0*range;
  range5 = 5.0*range;
  range2 = 2.0*range;

  if (hispac >= range10)
      r->ordinate.maj.inter = range10;
  else if((lospac <= range5) && (hispac >= range5))
      r->ordinate.maj.inter = range5;
  else
      r->ordinate.maj.inter = range2;

  if (r->ordinate.maj.inter <= 1.0)
      right_of_d = 1 + (int)(0.5 - log10(r->ordinate.maj.inter));
  else
      right_of_d = 0;

  sprintf(abform,"%s%df","%8.",right_of_d);
  reticle_set_ord_precision(r,abform);
  r->ordinate.maj.style = EDGES;
  r->ordinate.maj.length = hif/20.0;;
  r->ordinate.maj.list = NULL;
  r->ordinate.maj.num = 0;
  r->ordinate.min1.style =  EDGES;
  r->ordinate.min1.length = hif/30.0;
  r->ordinate.min1.inter = r->ordinate.maj.inter/2.0;
  r->ordinate.min1.list = NULL;
  r->ordinate.min1.num = 0;
  r->ordinate.min2.style = EDGES;
  r->ordinate.min2.length = hif/60.0;
  r->ordinate.min2.inter = r->ordinate.maj.inter/10.0;
  r->ordinate.min2.list = NULL;
  r->ordinate.min2.num = 0;
  r->ordinate.num_inter = r->ordinate.maj.inter;
  r->ordinate.num_loc = NUM_LB;
  r->abscissa.maj.style =  EDGES;
  r->abscissa.maj.length = arange/30.0;
  r->abscissa.maj.list = NULL;
  r->abscissa.maj.num = 0;
  r->abscissa.min1.style =  EDGES;
  r->abscissa.min1.length = arange/60.0;
  r->abscissa.min1.inter = r->abscissa.maj.inter/2.0;
  r->abscissa.min1.list = NULL;
  r->abscissa.min1.num = 0;
  r->abscissa.min2.style = EDGES;
  r->abscissa.min2.length = arange/100.0;
  r->abscissa.min2.inter = r->abscissa.maj.inter/10.0;
  r->abscissa.min2.list = NULL;
  r->abscissa.min2.num = 0;
  r->abscissa.num_inter = r->abscissa.maj.inter;
  r->abscissa.num_loc = NUM_LB;
  r->color = RETICLE_COLOR;
  r->linetype = 1;
  r->font = NULL;
  r->abs_label = NULL;
  r->ord_label = NULL;
  r->ordi.start = ob->ylow;
  r->ordi.end = ob->yhigh;
  r->absc.start = ob->xlow;
  r->absc.end = ob->xhigh;
  b = reticle_get_margins(r);
  r->bounds.top = 20;		/* to make room for numerical printout */
  r->bounds.bottom = rec->r_height - 1 - b->bottom;
  r->bounds.left = b->left;
  r->bounds.right = rec->r_width - 1 - b->right;
  return(r);
}

/*********************************************************************/
int
convert_trace(trace, scale_type)
    Trace   *trace;
    int	    scale_type;
{
    double  *data, lim;
    int	    n, i;

  if (debug_level)
    (void) fprintf(stderr, "Entered convert_trace()\n");

    if (!trace || !(data = trace->data)) return FALSE;
    if (scale_type == trace->scale_type) return TRUE;
    n = trace->n;

    switch (trace->scale_type)
    {
    case SCALE_DB:
	switch (scale_type)
	{
	case SCALE_MAG:
	    for (i = 0; i < n; i++)
		data[i] = (data[i] <= -200.0) ? 0.0 : pow(10.0, 0.05*data[i]);
	    break;
	case SCALE_PWR:
	    for (i = 0; i < n; i++)
		data[i] = (data[i] <= -200.0) ? 0.0 : pow(10.0, 0.1*data[i]);
	    break;
	default:
	    return FALSE;
	}
	break;
    case SCALE_MAG:
	switch (scale_type)
	{
	case SCALE_DB:
	    for (i = 0; i < n; i++)
		data[i] = (data[i] <= 0.0) ? -200.0 : 20.0*log10(data[i]);
	    break;
	case SCALE_PWR:
	    for (i = 0; i < n; i++)
		data[i] = data[i]*data[i];
	    break;
	default:
	    return FALSE;
	}
	break;
    case SCALE_PWR:
	switch (scale_type)
	{
	case SCALE_DB:
	    for (i = 0; i < n; i++)
		data[i] = (data[i] <= 0.0) ? -200.0 : 10.0*log10(data[i]);
	    break;
	case SCALE_MAG:
	    for (i = 0; i < n; i++)
		data[i] = sqrt(data[i]);
	    break;
	default:
	    return FALSE;
	}
	break;
    default:
	return FALSE;
    }

    trace->scale_type = scale_type;
    return TRUE;
}

/*********************************************************************/
void
get_trace_maxmin(trace, maxp, minp)
    Trace   *trace;
    double  *maxp, *minp;
{
    double  *data, max, min;
    int	    n, i;

  if (debug_level)
    (void) fprintf(stderr, "Entered get_trace_maxmin()\n");

    if (trace && (data = trace->data) && 0 < (n = trace->n))
    {
	max = min = data[0];
	for (i = 1; i < n; i++)
	    if (min > data[i]) min = data[i];
	    else
	    if (max < data[i]) max = data[i];
    }
    else
    {
	max = -DBL_MAX;
	min = DBL_MAX;
    }

    if(scale_type == SCALE_DB) {
     *minp = min - reference_level;
     *maxp = max - reference_level;
    } else {
      *minp = min;
      *maxp = max;
    }
}

/*********************************************************************/
void
get_spect_maxmin(ob)
    Objects *ob;
{
    double  max, min;
    int	    i;

  if (debug_level)
    (void) fprintf(stderr, "Entered get_spect_maxmin()\n");

    if (!ob) return;

    ob->ylow = DBL_MAX;
    ob->yhigh = -DBL_MAX;

    for (i = 0; i < TRACE_MAX && ob->trace[i]; i++)
    {
	get_trace_maxmin(ob->trace[i], &max, &min);
	if (ob->ylow > min) ob->ylow = min;
	if (ob->yhigh < max) ob->yhigh = max;
    }

    if (ob->ylow < ob->yhigh) return;

    if (ob->ylow > 0.0) ob->ylow = 0.0;
    if (ob->yhigh < 0.0) ob->yhigh = 0.0;
    if (ob->yhigh <= ob->ylow) ob->yhigh = 120.0;
}

/*********************************************************************/
redo_display(ob)
    Objects *ob;
{
  double end;
  Rect *rec;
  Xv_Window pw;
  int     x, y, y0;
  Frame   frame;
  Icon	  icon;
  char    mess[200];
  Signal  *s;
  double  lim;
  int	  i;

  if(ob) {
    if(!ob->init_located)
      return(get_display_attributes(ob));
    if (ob->trace[0]) {		/* Any data to be plotted? */
      if(f_autozoom &&
	 ((ob->r_marker - ob->l_marker) >
	  (ob->trace[0]->band/ob->trace[0]->n))) { /* bracket > one bin? */
	ob->xlow = ob->l_marker;
	ob->xhigh = ob->r_marker;
      } else {			/* make trace limits cover all spectra */
	ob->xlow = ob->trace[0]->band_low;
	ob->xhigh = ob->trace[0]->band_low + ob->trace[0]->band;
	for (i = 1; i < TRACE_MAX && ob->trace[i]; i++) {
	  if (ob->xlow > (lim = ob->trace[i]->band_low))
	    ob->xlow = lim;
	  if (ob->xhigh < (lim =
			   ob->trace[i]->band_low + ob->trace[i]->band))
	    ob->xhigh = lim;
	}
      }
    }

    for (i = 1; i < TRACE_MAX && ob->trace[i]; i++)
      convert_trace(ob->trace[i], scale_type);

    if(a_autozoom && (ob->t_marker - ob->b_marker > .1)) {
      ob->yhigh = ob->t_marker;
      ob->ylow = ob->b_marker;
    } else {
      get_spect_maxmin(ob);

/*      for (i = 0;
	   i < TRACE_MAX
	   && ob->trace[i]
	   && ob->trace[i]->scale_type == SCALE_DB;
	   i++) { }

      if (i < TRACE_MAX && ob->trace[i])  have found non-DB trace
*/
      if(scale_type != SCALE_DB)
	{
	  if (ob->ylow > 0.0) ob->ylow = 0.0;
	  ob->yhigh += 0.05*(ob->yhigh - ob->ylow);
	}
      else			/* all traces DB */
	{
	  ob->yhigh += 5.0;
	  ob->ylow = ob->yhigh - range_db;
	}
    }

    if(reticle_on)
      new_spectrum_reticle(ob);
    else
      if(ob->ret) {
	free_reticle(ob->ret);
	ob->ret = NULL;
      }

    if(ob->view) {
      if((s = ob->sig))	{ /* refresh the frame header and icon label */
	sprintf(mess, "%s: %fs", s->name, ob->time);

	frame = (Frame) xv_get(ob->view, XV_OWNER);

	xv_set(frame,	XV_LABEL,		mess,
	       0);

	icon = (Icon) xv_get(frame, FRAME_ICON);

	if(icon)
	  xv_set(icon,ICON_TRANSPARENT_LABEL,basename(s->name),
		 0);

	if (xsp_datwin_forw || ob->first_display) {
	  xv_set(frame, XV_SHOW, TRUE, 0);
	  ob->first_display = 0;
	}	
      }

      pw = canvas_paint_window(ob->view);
      rec = (Rect *) xv_get(pw, WIN_RECT);
      if(!doing_print_graphic)
	pw_write(pw, 0, 0, rec->r_width, rec->r_height,
	       PIX_SRC|PIX_COLOR(BACKGROUND_COLOR), NULL, 0, 0);

      draw_reticle(ob->view, ob->ret);

      if(plot_data(ob)) {
	setcursors(ob, ob->cursorx[0]);
	plotcursors(ob);
	printcursors(ob);
	plot_markers(ob);
	return(TRUE);
      } else
	ob->x_curs_num = ob->y_curs_num = 0;
    }
  }
  return(FALSE);
}

/*********************************************************************/
have_all_display_attributes(ob)
     Objects *ob;
{
  if(ob && (width > 0) &&
     (height > 0) &&
     (start != -1.0) &&
     (sec_cm > 0) &&
     (loc_x != -123) &&
     (loc_y != -123)) {
    ob->width = width;
    ob->height = height;
    ob->start = start;
    ob->sec_cm = sec_cm;
    ob->xloc = loc_x;
    ob->yloc = loc_y;
    return(TRUE);
  } else
    return(FALSE);
}

/**********************************************************************/
initial_bad_values()
{
  m_time = start = sec_cm = -1.0;
  color = -1;
  width = height = loc_x = loc_y = -123;
  *signame = 0;
}

/*********************************************************************/
char *
meth_redisplay(ob,str)
     Objects *ob;
     char *str;
{
  if(ob && str && ob->view) {
    Frame frm;
    Rect	rect;

    initial_bad_values();

    if( ! ob->init_located ) {
      if(get_args(str,&a0) &&
	 have_all_display_attributes(ob)) {
	int plot_height =
	  (xsp_datwin_height >= 0 )
	    ? xsp_datwin_height
	      : 250;
	int plot_width =
	  (xsp_datwin_width >= 0)
	    ? xsp_datwin_width
	      : 425;
	int plot_x =
	  (xsp_datwin_x >= 0) ? xsp_datwin_x : ob->xloc;
	
	int plot_y = (xsp_datwin_y >= 0)? xsp_datwin_y
	     : ob->yloc + ob->height;

	frm = (Frame) xv_get(ob->view, XV_OWNER);
	frame_get_rect(frm, &rect);
	limit_location(&plot_x,&plot_y, plot_width, plot_height);

	rect_construct(&rect, plot_x, plot_y,
		       plot_width, plot_height);
	frame_set_rect(frm, &rect);
	frame_get_rect(frm, &rect);
	ob->init_located = TRUE;
	redo_display(ob);
	return(Ok);
      } else
	get_display_attributes(ob);
    } else
      return(Ok);
  } else
    show_notice(1 ,"Null object or arg list to meth_redisplay.");
  return(Null);
}

/*********************************************************************/
limit_location(x, y, width, height)
     int *x, *y, width, height;
{
  if((*x + width) > SCREEN_WIDTH)
    *x = SCREEN_WIDTH - width;

  if((*y + height) > SCREEN_HEIGHT)
    *y = SCREEN_HEIGHT - height;
}

/*********************************************************************/
/* Here is an example of two-way communication with the host process.
   This routine gets the location, size and scale factors of an object
   displayed by the host. */
/*********************************************************************/
get_display_attributes(ob)
     Objects *ob;
{
  char mes[300];
  int n, id;

  if(ob && ob->name) {
    start = sec_cm = -1.0;
    width = height = loc_x = loc_y = -1;
    id = set_return_value_callback(meth_redisplay,ob);
    if(ob->sig && ob->sig->name) {
      sprintf(mes,
	"%s get attributes display function %s file %s return_id %d\n",
	      ob->name, thisprog, ob->sig->name, id);
    } else {
      sprintf(mes,"%s get attributes display function %s return_id %d\n",
	      ob->name,
	      thisprog, id);
    }
    mess_write(mes);
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/
static void
repaint(canvas, pw, repaint_area) /* when displays are created or moved */
    Canvas	canvas;
    Xv_Window	*pw;
    Rectlist	*repaint_area;
{
    Objects	*ob;
    int hold = xsp_datwin_forw;
    Frame frm;
    Rect  rec;

    ob = (Objects *) xv_get(canvas, WIN_CLIENT_DATA);
    xsp_datwin_forw = 0;	/* prevents N-object oscillation! */
    redo_display(ob);		/* (due to asynchronous window refresh) */
    xsp_datwin_forw = hold;

    /* If the spectrum display window position and size are specified,
       then change these specifications to match the newly-redrawn window.
       This will cause subsequent window creations to match the size and
       location of the current one. */
    frm = (Frame)xv_get(canvas, XV_OWNER);
    frame_get_rect(frm, &rec);
    if(xsp_datwin_height >= 0)
      xsp_datwin_height = rec.r_height;
    if(xsp_datwin_width >= 0)
      xsp_datwin_width = rec.r_width;
    if(xsp_datwin_x >= 0)
      xsp_datwin_x = rec.r_left;
    if(xsp_datwin_y >= 0)
      xsp_datwin_y = rec.r_top;
}

/***********************************************************************/
char *
meth_return(ob,str)
    Objects *ob;
    char    *str;
{
  if(str && *str) {
    int id;
    char *get_next_item();

    sscanf(str,"%d",&id);
    do_return_callback(id,get_next_item(str));
    return(Ok);
  }
  return(Null);
}

/*********************************************************************/
char *meth_save_globals(ob,str)
     Objects *ob;
     char *str;
{
  static char name[NAMELEN];
  static Selector s1 = {"output", "#qstr", name, NULL};

  *name = 0;
  get_args(str, &s1);

  if(dump_local_variables(name, &g1))
    return(Ok);
  else
    return(Null);
}

/*********************************************************************/
char *
meth_print_EPS_temp(ob, str)
    Objects *ob;
    char    *str;
{
    static char	    filename[NAMELEN];
    static int	    id;
    static Selector s1 = {"output", "#qstr", filename, NULL},
		    s0 = {"return_id", "%d", (char *) &id, &s1};
    Frame	    frame;
    int		    x, y;
    char	    reply[200];
    Rect	    rect;

    *filename = '\0';
    id = 0;
    get_args(str, &s0);

    if (ob && ob->view && ob->init_located)
    {
	if (debug_level)
	    fprintf(stderr, "meth_print_EPS_temp(%s, \"%s\")\n",
		    (ob->name) ? ob->name : "<NULL>",
		    (str) ? str : "<NULL>");

	frame = xv_get(ob->view, XV_OWNER);

	if (*filename && id && frame)
	{
	    if (!xv_get(frame, FRAME_CLOSED))
	    {
		frame_get_rect(frame, &rect);
		x = rect.r_left;
		y = rect.r_top;

		sprintf(reply, "%s completion %d loc_x %d loc_y %d",
			host, id, x, y);

		print_EPS_temp(ob, filename);

		mess_write(reply);
		return Ok;
	    }
	}
    }

    sprintf(reply, "%s completion %d", host, id);

    if (debug_level)
	fprintf(stderr, "meth_print_EPS_temp: Null return.\n");

    mess_write(reply);
    return Null;
}

/*********************************************************************/
char *meth_print(ob, str)
     Objects *ob;
     char *str;
{

  if(ob && ob->view) {
    e_print_graphic(ob->view, NULL, ob);
    return(Ok);
  }
  return(Null);
}


/*********************************************************************/
/* The following two methods lists specify messages which may be sent by the
   host to this program (base_methods) and to data views managed by this
   program (meth1). */
/*********************************************************************/
Methods
  meth5 = {"print_EPS_temp", meth_print_EPS_temp, NULL},
  meth4 = {"kill", meth_kill, &meth5},
  meth3c = {"print", meth_print, &meth4},
  meth3b = {"save_spectrum", meth_print_data, &meth3c},
  meth3 = {"redisplay", meth_redisplay, &meth3b},
  meth2 = { "mark", meth_mark, &meth3},
  meth1 = { "quit", meth_quit, &meth2};

Methods
  bm8 = {"completion", meth_return, NULL},
  bm4 = {"set", meth_set, &bm8},
  bm4b = {"save_globals", meth_save_globals, &bm4},
  bm3 = {"mark", meth_mark_obj, &bm4b},
  bm2 = {"quit", meth_quit, &bm3},
  base_methods = {"make", meth_make_object, &bm2};

/*********************************************************************/
/* This Objects structure is simply an example.  There is nothing sacred
   about its form except that it must have name, methods and linkage
   pointers. */
Objects *
new_objects(name)
    char    *name;
{
    Objects *ob;
    char    *c;
    int	    i;

    if ((ob = (Objects*)malloc(sizeof(Objects))) &&
	(c = malloc(strlen(name) + 1))) {
	strcpy(c, name);
	ob->name = c;
	ob->view = XV_NULL;
	ob->methods = &meth1;
	ob->sig = NULL;
	ob->init_located = FALSE;
        for (i = 0; i < TRACE_MAX; i++) ob->trace[i] = NULL;
	ob->trace_num = 1;
	ob->data2[0] = 0.0;
	ob->ret = NULL;
	ob->vers = 0;
	ob->xhigh = -100.0;
	ob->xlow = 0.0;
	ob->yhigh = range_db;
	ob->ylow = 0.0;
	ob->l_marker = 0.0;
	ob->r_marker = 0.0;
	ob->t_marker = 0.0;
	ob->b_marker = 0.0;
	ob->x_off = 2;
	ob->y_off = 0;
	ob->xloc = 10;
	ob->yloc = SCREEN_HEIGHT;
	ob->width = 300;
	ob->height = 200;
	ob->sec_cm= .02;
	ob->start = 0.0;
	ob->first_display = 1;
	/* time */
	ob->cursorx[0] = ob->cursory[0]  -100.0;
	ob->x_curs_num = ob->y_curs_num = 0;
	ob->next = NULL;
	ob->current_harmonic = 1;
	return(ob);
    }
    show_notice(1 ,"Can't allocate space for another object.");
    return(NULL);
}

/************************************************************************/
kill_object(o)
    Objects *o;
{
  Objects *o2;
  extern Objects *objlist;
  int     i;

  if(o) {
    if((o2 = objlist) == o)
      objlist = o->next;
    else
      while(o2->next) {
	if(o2->next == o) {
	  o2->next = o->next;
	  break;
	}
	o2 = o2->next;
      }
    if(o->name) free(o->name);
    if(o->sig) free_signal(o->sig);
    for (i = 0; i < TRACE_MAX; i++)
      if (o->trace[i])
	free_trace(o->trace[i]);
    free(o);
    return(TRUE);
  }
  return(FALSE);
}


/* Check that requested number of points is <= max size FFT available
   generate log 2 of the size of the radix-2 FFT to use; allocate memory
   if necessary. */
make_arrays(n,log2size,size,re,im)
     int n, *log2size, *size;
     double **re, **im;
{
  static double *x=NULL, *y=NULL;
  static int oldsize = 0;

  if(get_fft_size(n,log2size,size) <= fft_tablesize*2) {
    if(*size > oldsize) {
      if(x) {
	free(x);
	if(y)
	  free(y);
	x = y = NULL;
      }
      if(!((x = (double*)malloc(sizeof(double)* *size)) &&
	   (y = (double*)malloc(sizeof(double)* *size)))) {
	show_notice(1, "Allocation problems in make_arrays.");
	return(FALSE);
      }
      oldsize = *size;
    }
    *re = x;
    *im = y;
    return(TRUE);
  } else {
    sprintf(notice_msg,
	    "FFT size requested (%d) exceeds that available (%d).\n",
	    *size, fft_tablesize*2);
    show_notice(1, notice_msg);
    return(FALSE);
  }
}

/* enter with log2size and size set to minimum acceptable FFT size */
get_fft_size(n,log2size,size)
     int n, *log2size, *size;
{
  while(*size < n) {
    *size *= 2;
    (*log2size)++;
  }
  return(*size);
}


Notify_value
destroy_func(client, status)
    Notify_client   client;
    Destroy_status  status;
{

#define DEBUG(x) if(debug_level > x) fprintf

    DEBUG(1) (stderr,"Inside of destroy_func.\n");
    if (status == DESTROY_CHECKING) {
	}
    else  if (status == DESTROY_CLEANUP) {
    DEBUG(1) (stderr,"Inside of destroy_func. CLEANUP\n");
	quit_proc();
        return notify_next_destroy_func(client, status);
	}
    else if (status == DESTROY_SAVE_YOURSELF) {
	}
    else {
    DEBUG(1) (stderr,"Inside of destroy_func. DEATH\n");
	quit_proc();
	}
	
    return NOTIFY_DONE;
}

Notify_value
sigint_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Caught an interrupt signal -- exiting.\n");
    quit_proc();
}

Notify_value
sigtrm_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Killed -- exiting.\n");
    quit_proc();
}

Notify_value
sigfpe_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Floating point exception.  Exiting.\n");
    quit_proc();
}

Notify_value
sigbus_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Bus error exception caught.  Exiting.\n");
    quit_proc();
}

Notify_value
sigill_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Illegal instruction exception caught.  Exiting.\n");
    quit_proc();
}

Notify_value
sigseg_func(client, sig, when)
    Notify_client client;
    int sig, when;
{
    fprintf(stderr, "Seg violation exception caught.  Xspectrum exiting.\n");
    quit_proc();
}

int
is_feasd(sig)
Signal *sig;
{
  struct header *esps_hdr = NULL;

  if (sig->header)
    esps_hdr = sig->header->esps_hdr;

  if (esps_hdr &&
      (esps_hdr->common.type == FT_FEA) &&
      (esps_hdr->hd.fea->fea_type == FEA_SD))
    return 1;
  else
    return 0;
}

/*********************************************************************/

got_enough(nbuf,n)
     int nbuf, *n;
{
  if(*n <= nbuf) {
    if((*n == nbuf) && (preemp != 0.0)) {
      (*n)--;
      if (w_verbose > 1)
	(void) fprintf(stderr,
	    "xspectrum: Warning, not enough data in file for requested\nwindow size with preemphasis.  Reducing window by one sample to %d\n", *n);
    }
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/

/* MENUS */
/* Mostly copied from xmenus.c */

Menuop
    op5 =   {"print graphic", e_print_graphic, NULL, NULL},
    op3 =   {"inverse filter",  e_inv_filt, NULL, &op5},
    op4c =   {"set dB reference", e_set_ref_level, NULL, &op3},
    op4b =   {"zoom to markers", e_bracket_markers, NULL, &op4c},
    op4 =   {"zoom full out", e_zoom_out, NULL, &op4b},
    op2 =   {"clear all spectra ", e_clear_traces, NULL, &op4},
    op1 =   {"clear ref spectra", e_clear_non_ref_traces, NULL, &op2},
    op0 =   {"save as ref spectrum", e_ref_save, NULL, &op1},
    *right_button_ops = &op0;

/*********************************************************************/
/* Notify_proc called when an item in the menu is selected.
   Heller shows the return type as void in version 2 of XView,
   but the code in .../lib/libxvol/menu shows the type as Xv_opaque.
   The value is irrelevant if no menu_done_proc is defined. */

Xv_opaque
do_menu(menu, item)
    Menu        menu;
    Menu_item   item;
{
    Event       *event;
    Xv_object   win;
    Canvas      canvas;
    Objects	*ob;
    void	(*proc)();

    if (debug_level)
	(void) fprintf(stderr, "do_menu: function entered\n");

    if (proc = (void (*)()) xv_get(item, MENU_VALUE))
    {
	event = (Event *) xv_get(menu, MENU_FIRST_EVENT);
	win = event_window(event);
	canvas = xv_get(win, CANVAS_PAINT_CANVAS_WINDOW);

	if (!canvas) canvas = win;	/* Workaround for possible bug
					   (cf. do_menu in xmenus.c).
					   Still needed??? */

	ob = (Objects *) xv_get(canvas, WIN_CLIENT_DATA);

        (*proc)(canvas, event, ob);
    }

    return XV_NULL;             /* Value irrelevant. */
}

/*********************************************************************/

Menu
make_generic_menu(mo)
    Menuop  *mo;
{
    Menu    menu;
    Menu_item   mitem;

    menu = xv_create(XV_NULL, MENU, 0);
    if (menu == MENU_NULL)
    {
	fprintf(stderr,"make_generic_menu:  couldn't create menu.\n");
	exit(-1);
    }

    for ( ; mo; mo = mo->next)
	if(mo->proc) {
	    mitem = xv_create(XV_NULL, MENUITEM,
			      MENU_STRING,	mo->name,
			      MENU_NOTIFY_PROC,	do_menu,
			      MENU_VALUE,		mo->proc,
			      0);
	    xv_set(menu, MENU_APPEND_ITEM, mitem, 0);
	}

    return(menu);
}

/*********************************************************************/

Menu
make_menu()
{
    return make_generic_menu(right_button_ops);
}

/*********************************************************************/

static void
e_ref_save(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
    if (!ob) return;
    save_as_ref(ob);
    redo_display(ob);
}

/*********************************************************************/
static void e_set_ref_level(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
  if(ob) {
    set_ref_level(ob, event_x(event));
    redo_display(ob);
  }
}

/*********************************************************************/
static void e_bracket_markers(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
  if(ob) {
    a_autozoom = f_autozoom = 1;
    redo_display(ob);
  }
}

/*********************************************************************/
static void e_zoom_out(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
  if(ob) {
    a_autozoom = f_autozoom = 0;
    redo_display(ob);
  }
}

/*********************************************************************/

static void
e_inv_filt(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
    if (ob && (ob->data2[0] > 0.0) && (function == esps_compute_spect))
        invfil(ob);
}

/*********************************************************************/

static void
e_clear_traces(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
    int	    i;

    if (!ob) return;

    for (i = 0; i < TRACE_MAX && ob->trace[i]; i++)
    {
	free_trace(ob->trace[i]);
	ob->trace[i] = NULL;
    }
    ob->trace_num = 1;

    redo_display(ob);
}

static void
e_clear_non_ref_traces(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
    int	    i;

    if (!ob) return;

    for (i = 1; i < TRACE_MAX && ob->trace[i]; i++)
    {
	free_trace(ob->trace[i]);
	ob->trace[i] = NULL;
    }
    ob->trace_num = 1;

    redo_display(ob);
}

/*********************************************************************/
char *exec_waves(str)
     char *str;
{
  extern char *dispatch();
  return(dispatch(str));
}

/*********************************************************************/
static void e_print_graphic(canvas, event, ob)
    Canvas  canvas;
    Event   *event;
    Objects *ob;
{
#ifdef HELL_FREEZES_OVER
	Display	*printer;


	if(!ob) return;

	printer = setup_xp_from_globals(ob->view);
	if (!printer)
	    return;

	start_xp(printer, ob->view);

	/* re-render image here */
        redo_display(ob);
	/**/
	finish_xp(printer, ob->view);
#endif
}


/*********************************************************************/
static void
print_EPS_temp(ob, filename)
    Objects *ob;
    char    *filename;
{
#ifdef HELL_FREEZES_OVER
    Display *printer;

    if (debug_level)
	fprintf(stderr, "print_EPS_temp(%s, %s)\n",
		(ob && ob->name) ? ob->name : "<NULL>",
		filename ? filename : "<NULL>");

    if (!ob) return;

    printer =
	setup_xp_EPS_temp(ob->view, filename, NULL, 0);
    if (!printer)
	return;

    start_xp(printer, ob->view);
    redo_display(ob);
    finish_xp(printer, ob->view);
#endif
}
