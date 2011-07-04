/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1989-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:  Alan Parker
 *
 * Brief description: utility functions for parameter prompting
 *
 */

static char *sccs_id = "@(#)xpromptpar.c	1.25	2/21/96	ESI/ERL";

/*
 * system include files
 */
#include <stdio.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/tty.h>
#include <xview/text.h>
#include <xview/cms.h>
#include <xview/notice.h>
#include <xview/scrollbar.h>


/*
 * esps include files
 */

#include <esps/esps.h>
#if !defined(DS3100) && !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/exview.h>
 
#ifndef NULL
#define NULL 0
#endif

/*
 * defines
 */
#define Fprintf (void)fprintf
#define Fflush (void)fflush

#define DEBUG(n) if (debug_level >= n) Fprintf

#define PARAM_OK      0
#define PARAM_MAX     1		/* parameter was greater than maximum */
#define PARAM_MIN     2		/* parameter was less than minimum */
#define PARAM_CHOICE  3		/* illegal discrete choice */
#define PARAM_FORMAT  4		/* parameter string value has bad format */
#define PARAM_NO_WRITE 5        /* couldn't write parameter */

#define DONE_OK  0
#define BAD_PARAMS 1
#define DONE_NOT_OK  2
#define EXIT_NOT_DONE  3

/*
 * global variable declarations
 */


static int frame_exit_status; /* for exit status of main frame */

extern int debug_level;
extern int do_color; 

#if !defined(hpux) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *calloc();
#endif
char *savestring();

char **symlist();
char **symchoices();
char *get_helpfile();
char *symprompt();
char getsymdef_c();
double getsymdef_d();
char* getsymdef_s();
long strtol();
double strtod();


static int	sh_fill_param_file();
static void	copysym();
       char	*getsymdef_string();
static int	param_checkwrite();
static int	x_fill_param_file();
       Frame	create_param_frame();
static Panel	create_param_panels();
static void	set_defaults();
static void	set_default_item();
static int	write_param();
static void	raise_param_notice();
static void	done();
       void	get_nrows_ncols();

/*
   This routine (exv_prompt_params) right now is the only visible one from this
   module; it fills in a parameter file via window or shell prompts based on 
   input parameter file; definite parameters are copied from input 
   to output file; prompts are used for the indefinite parameters 
   before putting them in output file
*/

int
exv_prompt_params(param_file, flag, check_file, outfile, program, x_pos, y_pos)
char *param_file;		/* input parameter file */
int flag;			/* flag for Common processing */
char *check_file;		/* check file for Common processing */
char *outfile;			/* output parameter file */
char *program;			/* intended program for param file*/
int x_pos, y_pos;		/* if non-zero, locates (possible) X window */
{
  char *funcname = "exv_prompt_params";
  char **allparams = NULL;	/* list of parameters */
  char **indef_params = NULL;	/* list of indefinite parameters */
  char *param = NULL;		/* specific parameter */
  int nparam = 0;		/* total number of parameters */
  int i, j;			/* loop indices */
  int ret;			/* a return code variable */
  int rp_ret;			/* for read_params return */
  int xwin;			/* flag for X Windows */

  spsassert(outfile != NULL, "exv_prompt_params() called with null outfile");
  DEBUG(2) (stderr, "Entered promp_params\n");
  if (debug_level > 1) {
    Fprintf(stderr, "%s: read_params called with:\n", funcname);
    Fprintf(stderr, "\t\tparam_file = %s\n", param_file);
    Fprintf(stderr, "\t\tflag = %s\n", 
	    (flag == SC_NOCOMMON ? "SC_NOCOMMON" : "SC_CHECK_FILE"));
    Fprintf(stderr, "\t\tcheck_file = %s\n", 
	    (check_file == NULL ? "NULL" : check_file));
  }
	  
  rp_ret = read_params(param_file, flag, check_file);

  symerr_exit();

  if ((rp_ret == -1) || (rp_ret == -3)) {
    Fprintf(stderr, 
	  "%s: read_params couldn't read parameter file\n", 
	    funcname, param_file);
    return(1);
  }

  if (debug_level && (rp_ret == -2))
    Fprintf(stderr, "%s: read_params couldn't read Common\n", funcname); 

  /*get list of parameters*/

  if ((allparams = symlist(&nparam)) == NULL) {
    Fprintf(stderr, "%s: no parameters in symbol table\n", funcname);
    return(BAD_PARAMS);
  }

  if (debug_level > 1) 
    Fprintf(stderr, "%s: %d parameters reported by symlist()\n",
	    funcname, nparam);

  /*allocate (excess) memory to duplicate the param list*/
  
  indef_params = (char **) calloc((unsigned) (nparam + 1), sizeof(char *));
  if(indef_params == NULL) {
    fprintf(stderr, 
	    "%s:couldn't allocate memory for indefinite parameter list\n",
	    funcname);
    return(BAD_PARAMS);
    }

  /* copy all parameters and get list of indefinite parameters*/
  /* note: we copy them all, since defaults may be taken without change*/

  if (debug_level > 1) 
    Fprintf(stderr, "%s: copying parameters to file %s\n",
	    funcname,  outfile);

  j = 0;

  for (i = 0; i < nparam; i++) {
    param = allparams[i];
    copysym(param, outfile);
    if (!symdefinite(param)) {
      indef_params[j] = savestring(param); /*create new one for safety*/
      j++;
    }
  }
  indef_params[j] = NULL;

  if (debug_level > 1) 
    Fprintf(stderr, "%s: found %d indefinite parameters\n", funcname, j);

  if (j == 0) {
    if (debug_level) 
      Fprintf(stderr, "%s: no indefinite parameters in parameter file %s\n",
	      funcname, param_file);
    return(BAD_PARAMS);
  }

  /* prompt for the indefinite parameters and fill in the rest of the file*/
  /* right now wire for X; later go with detection or environment variable*/

  xwin = 1;

  if (!xwin) 
    ret = sh_fill_param_file(indef_params, outfile);
  else
    ret = x_fill_param_file(indef_params, outfile, program, x_pos, y_pos);

  return(ret);
}

static
int
sh_fill_param_file(params, outfile)
char ** params;			/* list of indefinite parameters */
char *outfile;			/* output parameter file */
/* use shell prompts to fill out the indefinite parameters
   from a parameter file; we assume that read_params has already been 
   called, and that the output param file already has the definite 
   parameters
*/
{
  spsassert(outfile != NULL, "sh_fill_param_file: null outfile");
  spsassert(outfile != NULL, "sh_fill_param_file: no params (null)");

  DEBUG(2) (stderr, "Entered sh_fill_param_file\n");

  /*use common routines with x_fill_param:

    read default numeric val and return as string
    convert string to numberic value and putsym
    check limits on numeric items
    */
  /* also need routine to check string choice item (use linsearch)*/

  Fprintf(stderr, "sh_fill_param_file: function not implemented yet\n"); 

  return(0);
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
    fputsym_f(param, getsymdef_d(param), file);
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


char *
getsymdef_string(param)
char *param;
{
  /* return default value as a string */
  int ptype;			/* data type of parameter */
  char *retstr;			/* actual return string */
  char strval[100];		/* space to construct return value */

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
    sprintf(strval, "%g", getsymdef_d(param));
    retstr = savestring(strval); 
    break;
  case ST_IARRAY: 
    Fprintf(stderr, "getsymdef_string: ST_IARRAY not supported yet\n");
    retstr = NULL;

/*    size = symsize(param);
    
    TRYALLOC(int, size, iarray, "iarray");

    lim = getsymdef_ia(param, iarray, size);

    for (i = 0; i < lim; i++)
	Fprintf(stdout,"%d ", iarray[i]);
    Fprintf(stdout, "\n");

    if ((lim != size) && !zflag) {
	Fprintf(stderr, 
		"%s: WARNING - inconsistent size of int array\n", ProgName);
	return(2);
      }

    else 
      return(0);
    }

*/
    break;
  case ST_FARRAY: 
    
    Fprintf(stderr, "getsymdef_string: ST_FARRAY not supported yet\n");
    retstr = NULL;

/*
    size = symsize("param");
    
    TRYALLOC(float, size, farray, "farray");

    lim = getsymdef_ia(param, iarray, size);

    for (i = 0; i < lim; i++)      for (i = 0; i < size; i++)
      Fprintf(stdout,"%g ", farray[i]);
    Fprintf(stdout, "\n");

    if ((lim != size) && !zflag) {
      Fprintf(stderr, 
	      "%s: WARNING - inconsistent size of float array\n", ProgName);
      return(2);
    }
    else 
      return(0);
      }
*/
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
    putsymret = fputsym_f(param, dval, parfile);
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

  if (putsymret == -1) {
      ret = PARAM_NO_WRITE;
      Fprintf(stderr, 
	    "param_checkwrite: trouble writing parameter %s\n", param);
    }
  return(ret);
}

/*all of the functions after this point are ones that really depend 
on XView -- might want to move them later; the ones above are ones that
could go into the regular ESPS library*/


static 
int
  x_fill_param_file(params, outfile, helpname, x_pos, y_pos)
char ** params;			/* list of indefinite parameters */
char *outfile;			/* output parameter file */
char *helpname;			/* intended user of outfile (for help) */
int x_pos, y_pos;		/* if non-zero, locates X prompt window */
/* pop up a window with prompts to fill out the indefinite parameters
   from a parameter file; we assume that read_params has already been 
   called, and that the output param file already has the definite 
   parameters; 
*/
{
  Frame frame = NULL;
  DEBUG(2) (stderr, "Entered x_fill_param_file\n");
  spsassert(outfile != NULL, "x_fill_param_file: null outfile");
  spsassert(outfile != NULL, "x_fill_param_file: no params (null)");

  frame = create_param_frame((Frame) NULL, params, outfile, 
			     helpname, x_pos, y_pos);
  if (frame == NULL) 
    return(EXIT_NOT_DONE);

  /* set global to indicate frame exit via quit (rather than DONE button) 
   * this is reset appropriately in done() */

  frame_exit_status = EXIT_NOT_DONE;

  xv_main_loop(frame);

  return(frame_exit_status); 

}


Frame
create_param_frame(parent, params, outfile, helpname, x_pos, y_pos)
Frame parent;			/* optional parent frame (may be NULL) */
char **params;			/* list of indefinite parameters */
char *outfile;			/* output parameter file */
char *helpname;			/* intended user of outfile (for help) */
int x_pos, y_pos;		/* if non-zero, locates (possible) X window */
{
  /*
  create basic frame with title and buttons for done, help (man page) if 
  program non-NULL, defaults, and items for each parameter
  This routine also sets the do_color global attribute 
   for access by other- modules
  */

/*pass outfile name as client data or perhaps as key data*/

  Frame  frame;			/* the main frame */
  Panel  con_panel;		/* panel for control buttons */
  Panel  par_panel;		/* panel for parameters */
  Panel_item def_button;	/* handle for default button */
  Panel_item done_button;	/* handle for Done button */
  Panel_item button;		/* temp handle */
  Cms    e_cms;			/* colormap segment */
  char *title;			/* title for prompt frame */
  char *help_title;		/* title for help window */

  spsassert(outfile != NULL, "create_param_frame: null outfile");
  spsassert(outfile != NULL, "create_param_frame: no params (null)");
  DEBUG(2) (stderr, "Entered create_param_frame\n");

  /* create custom frame titles if ESPS program name*/

  title = malloc(80); 
  help_title = malloc(80);

  if ((helpname != NULL) && (helpname[0] != '.') && (helpname[0] != '/')) {
    (void) sprintf(title, "ESPS Parameter Request for %s", helpname);
    (void) sprintf(help_title, "ESPS man page for %s", helpname);
  }
  else {
    (void) sprintf(title, "ESPS Parameter Request");
    (void) sprintf(help_title, "ESPS Help Window");
  }

  if ((x_pos >= 0) && (y_pos >= 0))  {

    frame = (Frame)xv_create(parent, FRAME,
			     XV_LABEL,    title,
			     FRAME_SHOW_FOOTER, FALSE,
			     FRAME_SHOW_RESIZE_CORNER, TRUE,
			     XV_X, x_pos,
			     XV_Y, y_pos,
			     NULL);
  }
  else {
      frame = (Frame)xv_create(parent, FRAME,
			       XV_LABEL,    title,
			       FRAME_SHOW_FOOTER, FALSE,
			       FRAME_SHOW_RESIZE_CORNER, TRUE,
			       NULL);
    }

  /* set global for color treatment*/

  do_color = exv_color_status(frame);

  /* create control panel with buttons*/ 

  con_panel = (Panel)xv_create(frame, PANEL, 
				 OPENWIN_SHOW_BORDERS, TRUE,
				 PANEL_LAYOUT, PANEL_HORIZONTAL,
				 XV_X,         0,
				 NULL);

/*
  if (do_color)  {

    e_cms = exv_init_colors(con_panel);


    xv_set(con_panel, 
	   WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_GRAY,
	   WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
	   NULL);
  }

*/

  done_button = xv_create(con_panel, PANEL_BUTTON,
			  PANEL_ITEM_Y,           10,
			  PANEL_NEXT_COL,      -1,  
			  PANEL_LABEL_STRING,  "Done",
			  PANEL_NOTIFY_PROC,   done,
			  NULL);
/*
  if (do_color) 
    xv_set(done_button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
*/

  def_button = (Panel_item) xv_create(con_panel, PANEL_BUTTON,
	    PANEL_ITEM_Y,           10,
	    PANEL_NEXT_COL,      -1,  
	    PANEL_LABEL_STRING,  "Defaults",
	    PANEL_NOTIFY_PROC,   set_defaults,
	    NULL);
/*

  if (do_color) 
    xv_set(def_button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
*/

  if (helpname != NULL) {
    button = xv_create(con_panel, PANEL_BUTTON,
		       PANEL_ITEM_Y,           10,
		       PANEL_NEXT_COL,      -1,  
		       XV_KEY_DATA, EXVK_HELP_TITLE,  help_title,
		       XV_KEY_DATA, EXVK_HELP_NAME, helpname,
		       PANEL_LABEL_STRING,  "Help",
		       PANEL_NOTIFY_PROC,   exv_get_help,
		       NULL);

/*
    if (do_color) 
      xv_set(button, PANEL_ITEM_COLOR, CMS_CONTROL_COLORS + EC_BLACK, NULL);
*/  }

  window_fit_height(con_panel); 

/* create panel with items for all the parameters*/ 

  DEBUG(2) (stderr,
	  "create_param_frame: make parameter items\n");

  par_panel = create_param_panels(frame, params, outfile);

  /* give the default and done buttons a pointer to the parameter panel*/

  xv_set(def_button, PANEL_CLIENT_DATA, par_panel, NULL);

  xv_set(done_button, PANEL_CLIENT_DATA, par_panel, NULL);

  /*finish fitting things*/

  xv_set(con_panel, XV_WIDTH, xv_get(par_panel, XV_WIDTH, NULL), NULL);

  window_fit(frame);

  DEBUG(2) (stderr,
	  "create_param_frame: set icon\n");

  (void) exv_attach_icon(frame, ERL_NOBORD_ICON, "params", TRANSPARENT);

  DEBUG(2) (stderr,
	  "create_param_frame: set defaults\n");

  return(frame);
}

static
Panel
create_param_panels(frame, param_list, outfile)
Frame frame;
char **param_list;
char *outfile;
/*This function takes an input panel and creates a panel
  item for each ESPS parameter in param_list; each time the user 
  modifies a parameter in the panel, the value is written out
  to the parameter file outfile in a form suitable for later 
  use by ESPS programs (i.e., we use fputsym()).  We assume that 
  param_list was derived from an input parameter file after calling
  read_params (i.e., so that info is available on each parameter).  
  Parameters that have (optional) discrete choices associated with
  them (in the original parameter file) are displayed as a
  PANEL_CHOICE.  All other parameters are displayed as 
  PANEL_TEXT items.  (XView has int support, but not for other 
  types, so the only way to get a uniform interface is to do the
  conversions ourselves!)  
*/
{

  Cms  e_cms;			/* ESPS colormap segment */
  char **choices;		/* NULL terminated array of strings */
  int nchoices;			/* number of choices*/
  char *param;			/* specific parameter */
  Panel con_panel;		/* handle for parameter panel */
  Panel par_panel;		/* handle for CHECK_CHOICE */
  char *prompt_string;		/* prompt string for paramter */
  char *ext_prompt_string = NULL;/* prompt_string + ":" */
  int i,j;			/* loop indices */
  int nrows, ncols;		/* number of PANEL_CHOICE rows and cols */
  Scrollbar vert_scrollbar, horiz_scrollbar; /* panel scrollbars */
   
  spsassert(frame != NULL, "create_param_panels: null frame");
  spsassert(outfile != NULL, "create_param_panels: null param_list");
  DEBUG(2) (stderr, "Entered create_param_panels\n");

  /* create parameter control panel*/

  con_panel = (Panel)xv_create(frame, SCROLLABLE_PANEL, 
			   OPENWIN_SHOW_BORDERS, TRUE,
			   XV_X,         0,
			   NULL);

  vert_scrollbar = xv_create(con_panel, SCROLLBAR,
			     SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
			     SCROLLBAR_SPLITTABLE, TRUE,
			     NULL);

  horiz_scrollbar = xv_create(con_panel, SCROLLBAR, 
			      SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
			      SCROLLBAR_PIXELS_PER_UNIT, 30,
			      NULL);

/*
  if (do_color) {

    e_cms = exv_init_colors(con_panel);

    xv_set(con_panel, WIN_BACKGROUND_COLOR, CMS_CONTROL_COLORS + EC_LIGHT_YELLOW,
		      WIN_FOREGROUND_COLOR, CMS_CONTROL_COLORS + EC_BLACK,
	              NULL);
  }
*/

  /*create panel items*/

  i = 0;

  while ((param = param_list[i]) != NULL) {
    
    /* create ":" terminated prompt string if parameter has a prompt string*/

    if ((prompt_string = symprompt(param)) != NULL) {

      if (ext_prompt_string != NULL) 
	free(ext_prompt_string);

      ext_prompt_string = malloc((unsigned) strlen(prompt_string) + 2);
      ext_prompt_string = 
	strcat(strcpy(ext_prompt_string, prompt_string), ":");

    }

    if ((choices = symchoices(param, &nchoices)) != NULL) {

      if (prompt_string) 
	par_panel = (Panel) xv_create(con_panel, PANEL_MESSAGE,
				      PANEL_NEXT_ROW,      -1,
				      PANEL_LABEL_STRING,  ext_prompt_string,
				      NULL);

      get_nrows_ncols(choices, nchoices, &nrows, &ncols);

      par_panel = (Panel) xv_create(con_panel, PANEL_CHOICE,
				      PANEL_NEXT_ROW,      -1,  
				      PANEL_CHOICE_NROWS,   nrows,
				      PANEL_CHOICE_NCOLS,   ncols,
				      PANEL_CHOOSE_ONE,    TRUE,
				      PANEL_LABEL_STRING,  param,
				      PANEL_CLIENT_DATA,   outfile,
				      PANEL_NOTIFY_PROC,   write_param,
				      NULL);

      for (j = 0; j < nchoices; j++) 
	xv_set(par_panel, PANEL_CHOICE_STRING, j, choices[j], NULL);

    }
    else {  /*parameter does not have discrete choices*/ 

      /* XV doesn't have float item so use strings for everything else*/

      if (prompt_string) 
	par_panel = (Panel) xv_create(con_panel, PANEL_MESSAGE,
				      PANEL_NEXT_ROW,      -1,
				      PANEL_LABEL_STRING,  ext_prompt_string,
				      NULL);

      par_panel = (Panel) xv_create(con_panel, PANEL_TEXT,
				    PANEL_NEXT_ROW,      -1,
				    /*set WIDTH to normalize largest*/
				    PANEL_VALUE_DISPLAY_LENGTH, 15,
				    PANEL_LABEL_STRING,  param,
				    PANEL_CLIENT_DATA,   outfile,
				    PANEL_NOTIFY_PROC,   write_param,
				    NULL);
    }
    /*set default*/ 
    set_default_item(par_panel); 
    i++;
  }
  window_fit(con_panel); 
  return(con_panel);
}

static
void
set_defaults(item, event) 
     Panel_item item;
     Event      *event;
{
    Panel panel = (Panel) xv_get(item, PANEL_CLIENT_DATA, NULL);
    
  if (debug_level > 1) 
    Fprintf(stderr, "set_defaults: setting all params to defaults\n");

  PANEL_EACH_ITEM(panel, item)
    
    /*skip any prompt strings*/

    if (((Panel_item_type) 
	 xv_get(item, PANEL_ITEM_CLASS, NULL)) != PANEL_MESSAGE_ITEM)

      set_default_item(item); 
    
  PANEL_END_EACH

  return;
}

static void
set_default_item(item)
Panel_item item;
{
  /* set default value for specific panel item */

  char *param;		/* parameter name */
  char **choices;		/* (optional) choices for param*/
  char *def_val_string;	/* default value in string representation */
  
  /* the label string is the parameter name*/

  param = (char *) xv_get(item, PANEL_LABEL_STRING, NULL);

  def_val_string = getsymdef_string(param);

  if ((choices = symchoices(param, (int *)NULL)) == NULL) /*string used*/ 
	
    xv_set(item, PANEL_VALUE, def_val_string, NULL); 

  else /*choice item*/ 

    xv_set(item, PANEL_VALUE, lin_search(choices, def_val_string), NULL);

}

static
int
write_param(item, event)
     Panel_item item;		/* parameter panel item */
     Event *event;			/* input event */
{
  /* update parameter when changed in panel (assume that parameter names 
     are the labels used in the panel? may not be possible, since might 
     want to use the parameter prompt string); use client_data or key_data to 
     attach info such as type or name; just do a putsym
     have different routine for choices and non-choices so that non-choices
     handles string conversions and also checks ranges(later)  */
  
  /*if limits, check them*/
  /*if out of bounds, change to min or max and pop message*/
  /*otherwise write the symbol*/
  /*have function that checks format, checks limits, and writes -- 
    if doesn't write (and returns various error codes) if either format 
    is wrong or limits are violated -- have calling routine handle the 
    two kinds of errors or just return if all is ok; this could be used
    also for straight prompting when ESPS_XWIN == no.*/

  char **choices;		/* parameter value choices */
  char *param_strval;		/* param value (as string) to be stored */
  char *newval;			/* param override value when error */
  char *paramfile;		/* name of parmeter file */
  short write_ok = 1;		/* was write OK? */

  /*parameter name is the item label*/

  char *param = (char *) xv_get(item, PANEL_LABEL_STRING, NULL); 

  /* item_class distinguishes choice items, etc. */

  Panel_item_type item_class = (Panel_item_type) 
                                  xv_get(item, PANEL_ITEM_CLASS, NULL);

  /* hardwire the parameter file name for now*/

  paramfile = (char *) xv_get(item, PANEL_CLIENT_DATA, NULL);

  if (debug_level > 1) 
    Fprintf(stderr, "write_param: new value for %s\n", param);

  if (item_class == PANEL_CHOICE_ITEM) {

    choices = symchoices(param, (int *) NULL); 

    param_strval = choices[(int) xv_get(item, PANEL_VALUE, NULL)];

  }
  else  /*these are all string items*/ 

    param_strval = (char *) xv_get(item, PANEL_VALUE, NULL);

  if (debug_level > 1) 
    Fprintf(stderr, "write_param: check and write value %s\n", param_strval);

  switch (param_checkwrite(param, param_strval, &newval, paramfile)) {
  case PARAM_MAX:
    xv_set(item, PANEL_VALUE, newval, NULL); 
    write_ok = 0;
    raise_param_notice(param, PARAM_MAX, newval, item, event);
    break;
  case PARAM_MIN:
    xv_set(item, PANEL_VALUE, newval, NULL); 
    write_ok = 0;
    raise_param_notice(param, PARAM_MIN, newval, item, event);
    break;
  case PARAM_FORMAT:
    set_default_item(item);
    write_ok = 0;
    newval = (char *) xv_get(item, PANEL_VALUE, NULL);
    raise_param_notice(param, PARAM_FORMAT, newval, item, event);
    break;
  case PARAM_NO_WRITE:
    write_ok = 0;
    break;
  case PARAM_OK:
    break;
  }
  if (write_ok)
    return(0);
  else
    return(-1);
}
/*   xread_params; identical to read_params but with extra arg for progname 
     (check old notes on param file changes)  -  if environment variable
     X_PROMT is set, a command panel is popped up for all "?=" parameters;
     a revised param file is then processed via read_params;
     (this routine uses the ones described above); try to do it in such
     a way that one binary distribution can suffice for all (i.e., X_PROMPT
     has no effect if X not available); also with ifdef on XVIEW
*/



static
void
raise_param_notice(param, type, newstrval, item, event)
char *param;			/* name of parameter */
int type;			/* type of parameter error */
char *newstrval;		/* string giving value changed to */
Panel_item item;		/* item for which error was made */
Event *event;			/* event pointer for input */
{
  /*raises an XV notice warning of the type of parameter entry error
    and the value that was chosen as the replacement*/

  Panel       owner;		/* parameter panel */
  char        notice_message[100]; /* the actual text of the notice */

  int         x = 0, y = 0;

  /* get the parameter panel */

  owner = (Panel) xv_get(item, XV_OWNER, NULL);

  /*get coordinates of the value string*/

  x = (int) xv_get(item, PANEL_VALUE_X, NULL);
  y = (int) xv_get(item, PANEL_VALUE_Y, NULL);

  if (debug_level > 2) 
    Fprintf(stderr, "\tx = %d, y = %d\n", x, y);

  switch(type) {
  case PARAM_MAX:
    sprintf(notice_message, "Value for %s is too large; reset to %s",
	    param, newstrval);
    break;
  case PARAM_MIN:
    sprintf(notice_message, "Value for %s is too small; reset to %s",
	    param, newstrval);
    break;
  case PARAM_FORMAT:
    sprintf(notice_message, 
	    "Format error for %s; reset to default: %s",
	    param, newstrval);
    break;
  default:
    Fprintf(stderr, "raise_param_notice: unsupported notice type\n");
    return;
  }
    (void)notice_prompt(owner, NULL,
			NOTICE_MESSAGE_STRINGS, notice_message, NULL,
			NOTICE_FOCUS_XY,        x, y,
			NOTICE_BUTTON_YES,      "Continue",
			NULL);

}

static
void
done(item, event)
     Panel_item item;
     Event      *event;
{
  /*This function gets called when the user is done; first we write out
    all the parameters (in case people didn't RETURN in the panel); 
    then we destroy the main window, causing the termination of the notifier
    loop; 
    */

    Frame frame;		/* main frame */
    Panel param_panel;		/* the panel with all the parameter settings */
    short all_ok = 1;		/* did all writes go ok? */

    param_panel = (Panel) xv_get(item, PANEL_CLIENT_DATA, NULL);

    frame = xv_get(param_panel, XV_OWNER); 

    if (debug_level > 1) 
      Fprintf(stderr, "done: writing all params\n");

    PANEL_EACH_ITEM(param_panel, item)
    
      /*skip any prompt strings*/

      if (((Panel_item_type) 
	   xv_get(item, PANEL_ITEM_CLASS, NULL)) != PANEL_MESSAGE_ITEM) {

	  if (write_param(item, event) != 0) 
	    all_ok = 0; 
      }
    PANEL_END_EACH

    if (all_ok) 
      /*set global indicating good output parameter file*/
      frame_exit_status = DONE_OK;
    else
      frame_exit_status = DONE_NOT_OK;

    xv_destroy_safe(frame);

    return;
}

/* use some heuristics to provide the number of rows and columns for 
   PANEL_CHOICEs
*/
#define TOT_CHAR_LIM 70

void
get_nrows_ncols(choices, nchoices, rows, cols)
     char **choices;		/* NULL terminated array of strings */
     int  nchoices;
     int  *rows, *cols;		/* to be computed here */
{
  int tot_char = 0;
  int i;
  int max_len = 0;			/* longest choice string */
  int len;

  for (i = 0; i < nchoices; i++) {
    len = strlen(choices[i]);
    tot_char += 2 + len;
    if (len > max_len) 
      max_len = len;
  }

  if (tot_char <= TOT_CHAR_LIM) {
    *rows = 1;
    *cols = nchoices;
  }
  else {
    /* should make this smarter; so that we avoid the last row being
       much shorter than the others */
    
    *cols = (int) TOT_CHAR_LIM / (max_len + 2);
    *rows = (int) nchoices / *cols;
  }
}       
