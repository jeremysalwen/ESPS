/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: xprompt.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This is xprompt - fills in parameter file prompts via X Window
 */

#ifndef lint
static char *sccs_id = "@(#)xprompt.c	1.6	10/31/90	ESI";
#endif

#define VERSION "1.6"
#define DATE "10/31/90"

/*
 * system include files
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>

/*
 * esps include files
 */

#include <esps/esps.h>
#include <esps/unix.h>
/*
 * defines
 */
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

#define SYNTAX USAGE ("xprompt [-P param_file] [-h help_name] [-n] [-c checkfile]\n\t [-x debug_level] [-z] param_out")
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
/*
 * global variable declarations
 */

int		    debug_level = 0;
int                 do_color = 0;
char		    *ProgName = "xprompt";

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
  int num_files = 0;		/* number of file arguments */
  char *paramfile = "params";


  char *param_out = NULL;	/* output parameter file name */
  FILE *outstrm = NULL;		/* output parameter file stream */
  char *checkfile = NULL;
  char *help_name = NULL;	/* name of program for eman call */
  int ret;			/* return code for param_prompt*/
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
  if (num_files != 1)
    SYNTAX;
  param_out = argv[optind];

  /* make sure can write the output parameter file; this also clears it*/

  if ((outstrm = fopen(param_out, "w")) == NULL)
    ERROR_EXIT1("can't write output file %s\n", param_out)
  else
    fclose(outstrm); 

  /* do all the work */

  ret = 
    prompt_params(paramfile, nflag, checkfile, param_out, help_name);

  if (ret == -1 && !zflag) 
    Fprintf(stderr, "%s: no parameters or no indefinite parameters\n", ProgName);

  (void) exit(ret);
}

