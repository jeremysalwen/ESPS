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
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description: prompts for parameters and then runs program
 *
 */

static char *sccs_id = "@(#)xpromptrun.c	1.1	6/7/91	ERL";

/* INCLUDE FILES */

#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>

#include <esps/esps.h>
#include <esps/unix.h>

/* LOCAL FUNCTION DECLARATIONS */

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "6/7/91"
#define EC_SCCS_VERSION "1.1"

/* LOCAL TYPEDEFS AND STRUCTURES */

/* LOCAL MACROS */

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

#define SYNTAX USAGE ("xpromptrun [-P param_file] [-h help_name] [-n] [-c checkfile]\n\t [-x debug_level] [-z] command_line")

/*
 * system functions and variables
 */
int getopt ();
extern  optind;
extern	char *optarg;

/*
 * global function declarations
 */

char *calloc();
char *e_temp_name();
char *find_esps_file();

/*
 * global variable declarations
 */

int		    debug_level = 0;
int                 do_color = 0;
char		    *ProgName = "xpromptrun";

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
  int               zflag = 0;	/* flag for silent operation */

  int nflag = SC_CHECK_FILE;	/* for getopt return */
  int i;			/* loop index */
  int num_files = 0;		/* number of file arguments */
  char *paramfile = "params";
  char *param_out = e_temp_name(NULL);	/* output parameter file name */
  FILE *outstrm = NULL;		/* output parameter file stream */
  char *checkfile = NULL;
  char *help_name = NULL;	/* name of program for eman call */
  int ret;			/* return code for param_prompt*/
  int sret;			/* return code for system() */
  char *command_line = malloc(500); 

  void   done();

  /*try to make this conditional on X windows*/
  /*what if work is in lib routine -- 
    can it know whether or not init was done? */

  xv_init (XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

  /*
   * process command line options
   */

  while ((c = getopt (argc, argv, "x:P:nc:h:z")) != EOF) {
    switch (c) {
    case 'x': 
      debug_level = atoi (optarg);
      break;
    case 'P':
      paramfile = optarg;
      break;
    case 'n':
      nflag = SC_NOCOMMON;
      break;
    case 'c':
      checkfile = optarg;
      break;
    case 'h':
      help_name = optarg;
      break;
    case 'z':
      zflag++;
      break;
    default:
      SYNTAX;
    }
  }

  if ((nflag == SC_NOCOMMON) && (checkfile != NULL)) 
    ERROR_EXIT("can't specify checkfile (-c) if common disabled (-n)");

  /* Get name of output parameter file */
  num_files = argc - optind;

  if (num_files == 0)
    SYNTAX;

  /* make sure can write the output parameter file; this also clears it*/

  if ((outstrm = fopen(param_out, "w")) == NULL)
    ERROR_EXIT1("can't write temporary parameter file %s\n", param_out)
  else
    fclose(outstrm); 

  /* build argument list for command line*/

  (void) sprintf(command_line, "%s -P %s ", argv[optind++], param_out); 

  for (i = optind++; i < argc; i++)  {
      /* should make sure that no -P option is used */
      (void) strcat(command_line, argv[i]);
      (void) strcat(command_line, " ");
    }

  /* do all the work */

  ret = 
    prompt_params(paramfile, nflag, checkfile, param_out, help_name);

  if (ret == -1 && !zflag) 
    Fprintf(stderr, 
	    "%s: no parameters or no indefinite parameters\n", ProgName);

  /*run program */

  if (debug_level) 
    Fprintf(stderr, 
	    "%s: running this command:\n%s\n", ProgName, command_line);

  ret = system(command_line); 

  if (debug_level) 
    Fprintf(stderr, 
	    "%s: command completed; system() returned %d\n", ProgName, ret);

  unlink(param_out); 

  (void) exit(ret);
}



