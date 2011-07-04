/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1997 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by: David Burton (as result of code review)
 *
 * Brief description: 
 *
 *      This module links the old ESPS plot programs to
 * a tcl/tk interpreter.   The actual plotting is done by the tcl script.
 * This file does not use ANSI prototypes since it becomes a module to
 * an existing pre-ANSI program.  Probably would be best to convert the
 * whole set of files to ANSI.
 *
 */

static char *sccs_id = "@(#)tk.c	1.3	7/21/98	ERL";

#include <stdio.h>
#include <values.h>
#include <unistd.h>
#include <tcl.h>
#include <tk.h>
#include <tix/tix.h>
#include <esps/unix.h>
#include <esps/esps.h>


static Tk_Window mainWindow;
static Tcl_Interp *interp;
static char *display = NULL;
static int init_done = 0;
static char *program_name;
static long count = 0;	     /* count of the number of plot commands */

char *getenv();
int   putenv();

#define TCL_PROGRAM_FILE "plot.tcl"


/* tk_init() is called by the plot program main (plotsd.c, plotspec.c, 
   aplot.c, etc).    It initializes the interpreters and also handles
   the plot program W option to specify geometry.
*/

void
tk_init(program, filename, Wflag, title)
char *program, *filename, *Wflag, *title;
{

/* 
 program  -  the name of the program for use in error messages
 filename -  the name of the ESPS file being plotted
 Wflag    -  contains the geometry option from the main command line
 title    -  contains title, if specified by user
*/

        char  *esps_base;      /* points to $ESPS_BASE */
        static char 
	  *tcl_library,        /* holds tcl library path */
	  *tk_library,         /* holds tk library path */
	  *tix_library;        /* holds tix library path */
	short ebase_length;    /* holds string length of ESPS_BASE value*/

	spsassert(!init_done, "tk_init already called");
	spsassert(program, "program NULL");

        interp = NULL;
	program_name = program;
	if (Wflag == NULL)
		Wflag = " ";
	if (title == NULL)
	        title = "none";
	/*
	   No filename is input from a pipe
	 */
	if (filename == NULL)
		filename = " ";

	/*
	   Point tcl/tk/tix library environment variables at the product 
	   provided script libraries ($ESPS_BASE/lib/{tcl,tk,tix}).  A 
	   customer may have an incompatible version installed; 
	   we want to make sure that we  don't use it by mistake.
	 */
	esps_base = getenv("ESPS_BASE");
	if (esps_base == NULL) {
	    Fprintf(stderr, 
              "%s requires that the ESPS_BASE environment variable be set.\n",
	       program_name);
	    exit(1);
	}
	if (access(esps_base, R_OK) != 0) {
	    Fprintf(stderr, "ESPS_BASE (%s) is not a readable directory.\n",
	      esps_base);
	    exit(1);
	}
	ebase_length = strlen(esps_base);
	/* 
	   Allocate enough room for library directory name and value. Fill in
	   name and value, and put in environment. Make sure to include 1 
	   for terminating \0.
	*/
	tk_library = (char *) malloc((unsigned) (11 + ebase_length + 7 + 1));
	if (tk_library == NULL) {
	    Fprintf(stderr, "%s: Couldn't allocate memory for tk_library\n",
		    program_name);
	    exit(1);
	} else {
	    sprintf(tk_library, "TK_LIBRARY=%s/lib/tk", esps_base);
	    if(putenv(tk_library) != 0) {
		Fprintf(stderr, "Failed to set TK_LIBRARY in environment\n");
		exit(1);
	    }
	}
	tix_library = (char *) malloc((unsigned) (12 + ebase_length + 8 + 1));
	if (tix_library == NULL) {
	    Fprintf(stderr, "%s: Couldn't allocate memory for tix_library\n",
		    program_name);
	    exit(1);
	} else {
	    sprintf(tix_library, "TIX_LIBRARY=%s/lib/tix", esps_base);
	    if(putenv(tix_library) !=0) {
		Fprintf(stderr, "Failed to set TIX_LIBRARY in environment\n");
		exit(1);
	    }	
	}
	tcl_library = (char *) malloc((unsigned) (12 + ebase_length + 8 + 1));
	if (tcl_library == NULL) {
	    Fprintf(stderr, "%s: Couldn't allocate memory for tcl_library\n",
		    program_name);
	    exit(1);
	} else {
	    sprintf(tcl_library, "TCL_LIBRARY=%s/lib/tcl", esps_base);
	    if(putenv(tcl_library)  != 0) {
		Fprintf(stderr, "Failed to set TCL_LIBRARY in environment\n");
		exit(1);
	    }
	}

	/* Create interpreter */
	interp = Tcl_CreateInterp();
	if (interp == NULL) {
	        Fprintf(stderr, "%s: Failed to create a Tcl interpreter.\n",
			program_name);
		exit(1);
	}
	mainWindow = Tk_CreateMainWindow(interp, display, program, "Tk");
	if (mainWindow == NULL) {
		Fprintf(stderr,"%s: %s\n",program_name, interp->result);
		exit(1);
	}

	/*
	   ready to initialize interpreter
	 */
	if (Tcl_Init(interp) == TCL_ERROR) {
                Fprintf(stderr, "Tcl_Init failed: %s\n", interp->result);
		exit(1);
        }
	if (Tk_Init(interp) == TCL_ERROR) {
                Fprintf(stderr, "Tk_Init failed: %s\n", interp->result);
		exit(1);
        }
        if (Tix_Init(interp) == TCL_ERROR) {
                Fprintf(stderr, "Tix_Init failed: %s\n", interp->result);
		exit(1);
        }

	/* 
	   pass these some variables to the tcl script via these global names
	 */
	if((Tcl_SetVar(interp,"start_geometry",Wflag,TCL_GLOBAL_ONLY)==NULL) ||
	   (Tcl_SetVar(interp,"program_name",program,TCL_GLOBAL_ONLY)==NULL) ||
	   (Tcl_SetVar(interp,"filename",filename,TCL_GLOBAL_ONLY)==NULL)    ||
	   (Tcl_SetVar(interp,"plot_title",title,TCL_GLOBAL_ONLY)==NULL) )
	{
	  Fprintf(stderr, "%s: Trouble setting values in interpreter\n",
		  program_name);
	  exit(1);
	}

	init_done++;
}


/*
 tk_snd_plot_cmd() is called by the C plot programs to send a plot 
 command (such as "m 500 600" to move to coordinate 500,600)
*/

void
tk_snd_plot_cmd(command)
char *command;
{
        char s[20];         /* big enough to hold MAXLONG */
	spsassert(command, "argument NULL");
	spsassert(init_done,"tk_init not called");

	sprintf(s,"%ld",count);
#ifdef DEBUG
	Fprintf(stderr,"%s -->%s<--\n",s,command);
#endif

	/* 
	   This sets the plot command into the element named by the value of 
	   "s" (the count) into the array input_data.
	 */
	Tcl_SetVar2(interp, "input_data", s, command, TCL_GLOBAL_ONLY);
	count++;
	if(MAXLONG == count) {
	    Fprintf(stderr, "%s: Too many commands to plot.\n", program_name);
	    exit(1);
	}
}

/*
 tk_snd_plot_cmd_1arg() is like tk_snd_plot_cmd() above, except that it
 takes an additional argument and prepends that to the plot command
*/

void
tk_snd_plot_cmd_1arg(command,arg)
char *command;
int arg;
{
	char s[20];     /* big enough to hold MAXLONG */
	char s2[100];	/* I know that the arguments are much smaller
			   than this, but probably not good form anyway
			*/
	spsassert(command, "argument NULL");
	spsassert(init_done, "tk_init not called");


	sprintf(s,"%ld",count);
	sprintf(s2,command,arg);
#ifdef DEBUG
	Fprintf(stderr,"%s -->%s<--\n",s,s2);
#endif
	Tcl_SetVar2(interp, "input_data", s, s2, TCL_GLOBAL_ONLY);
	count++;
	if(MAXLONG == count) {
	    Fprintf(stderr, "%s: Too many commands to plot.\n", 
		    program_name);
	    exit(1);
	}

}

	
/*
 tk_start() is called by the plot program after all plot commands have
 been sent.  It sets a final count and starts tcl script
*/

void
tk_start()
{
	int code;
	char s[20];
        char *tcl_filename = FIND_ESPS_LIB(NULL,TCL_PROGRAM_FILE);

	spsassert(init_done, "tk_init not called");

	if (tcl_filename == NULL) {
		Fprintf(stderr,"%s: Cannot find TCL script file %s.\n",
			program_name, TCL_PROGRAM_FILE);
		exit(1);
	}
	
	sprintf(s,"%ld",count);
	Tcl_SetVar (interp, "line_num", s, TCL_GLOBAL_ONLY);
	code = Tcl_EvalFile(interp, tcl_filename);
	if (code != TCL_OK) {
                Fprintf(stderr, "Tcl_EvalFile failed: %s\n", interp->result);
                exit(1);
        }

	Tk_MainLoop();
}

	

