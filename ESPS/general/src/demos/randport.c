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
 * Brief description: provide random socket port number
 *
 */

static char *sccs_id = "@(#)randport.c	1.3	1/5/96	ERL";

#include <stdio.h>
#include <esps/esps.h>

/*
 * defines
 */

#define SYNTAX USAGE ("randport -S seed")

#define Fprintf (void)fprintf

#define ABS(a) (((a) >= 0) ? (a) : (- a))

#define BIGRAND 2147483647.0	/*maximum value returned by random()*/

#define MIN_PORT 1000
#define MAX_PORT 31768


int getopt ();
extern  optind;
extern	char *optarg;
char *strcpy();

/*
 * global declarations
 */

#ifndef DEC_ALPHA
long random();
#endif
int debug_level = 0;

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
  int c;			    /*for getopt return*/
  int seed_flag = 0;	    /*flag for -S option*/
  int seed = 0;	    /*seed for random numbers*/
  int port;			/* output port number */

/*
 * process command line options
 */
  if (debug_level) Fprintf (stderr, "testsd: processing options\n");
  while ((c = getopt (argc, argv, "S:x:")) != EOF) {
    switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'S':
		seed = atoi(optarg);
		seed_flag++;
		break;
	    default:
		SYNTAX;
	}
    }

  if (!seed_flag) {
    Fprintf(stderr, "randport: must give seed with -S\n");
    SYNTAX;
  }

  (void) srandom(seed); 

  /* we should check to see that the port can be opened */

  do 
    port = MAX_PORT*((float)random() / BIGRAND);
  while (port < MIN_PORT); 

  Fprintf(stdout, "%d\n", port);

  exit(0);
}

