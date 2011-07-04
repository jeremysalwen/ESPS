/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: getparam.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This is getparam, a user level version of read_params/get_sym
 */

#ifndef lint
static char *sccs_id = "@(#)getparam.c	1.4	10/20/93	ESI";
#endif
/*
 * system include files
 */
#include <stdio.h>
/*
 * system include files
 */
#include <esps/esps.h>
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

#define SYNTAX USAGE ("getparam -p param_name [-P param_file] [-n] [-c checkfile]\n\t [-x debug_level] [-z]")
/*
 * system functions and variables
 */
int getopt ();
extern  optind;
extern	char *optarg;
void exit(), perror();
/*
 * external ESPS functions
 */

/*
 * global function declarations
 */

#ifndef DEC_ALPHA
char *calloc();
#endif

/*
 * global variable declarations
 */

int		    debug_level = 0;
char		    *ProgName = "getparam";

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

    int		    c;			    /*for getopt return*/
    int flag = SC_CHECK_FILE;
    char *paramfile = NULL;
    char *checkfile = NULL;
    char *param_name = NULL;
    int size;			/* size of parameter array */
    int lim;			/* number of elements printed */
    int *iarray;		/* array for integer param array */
    float *farray;	/* array for float param array */
    int ptype;			/* type of parameter */
    int zflag = 0;		/* flag for silence (no warnings) */
    int i;

/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:P:p:nzc:")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'P':
		paramfile = optarg;
		break;
	    case 'p':
		param_name = optarg;
		break;
            case 'n':
		flag = SC_NOCOMMON;
		break;
            case 'z':
		zflag++;
		break;
	    case 'c':
		checkfile = optarg;
		break;
	    default:
		SYNTAX;
	}
    }

    if (param_name == NULL) 
      ERROR_EXIT("must give parameter name with -p option");

    if ((flag == SC_NOCOMMON) && (checkfile != NULL)) 
      ERROR_EXIT("can't specify checkfile (-c) if common disabled (-n)");

    if (debug_level) {
      Fprintf(stderr, "%s: read_params called with:\n", ProgName);
      Fprintf(stderr, "\t\tparam_file = %s\n", paramfile);
      Fprintf(stderr, "\t\tflag = %s\n", 
	      (flag == SC_NOCOMMON ? "SC_NOCOMMON" : "SC_CHECK_FILE"));
      Fprintf(stderr, "\t\tcheck_file = %s\n", 
	      (checkfile == NULL ? "NULL" : checkfile));
    }
	  
    if (read_params(paramfile, flag, checkfile) == -3) 
      return(3);

    ptype = symtype(param_name);

    switch (ptype) {
    case ST_INT:
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is int\n", ProgName);
      Fprintf(stdout,"%d\n", getsym_i(param_name));
      return(0);
      break;
    case ST_CHAR:
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is char\n", ProgName);
      Fprintf(stdout,"%c\n", getsym_c(param_name));
      return(0);
      break;
    case ST_STRING:
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is string\n", ProgName);
      Fprintf(stdout,"%s\n", getsym_s(param_name));
      return(0);
      break;
    case ST_FLOAT:
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is float\n", ProgName);
      Fprintf(stdout,"%g\n", getsym_d(param_name));
      return(0);
      break;
    case ST_IARRAY: {
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is int array\n", ProgName);

      size = symsize(param_name);

      TRYALLOC(int, size, iarray, "iarray");

      lim = getsym_ia(param_name, iarray, size);

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
      break;
    }
    case ST_FARRAY: {
      if (debug_level > 1) 
	Fprintf(stderr, "%s: parameter type is float array\n", ProgName);

      size = symsize("param_name");

      TRYALLOC(float, size, farray, "farray");

      lim = getsym_ia(param_name, iarray, size);

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
      break;
    }

    case ST_UNDEF:
      if (!zflag) 
	Fprintf(stderr, "%s: parameter %s not found\n", ProgName, param_name);
      return(3);
      break;
    }
}


