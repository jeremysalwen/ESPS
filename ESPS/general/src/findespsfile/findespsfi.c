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
 * Written by:  Ken Nelson
 * Checked by:
 * Revised by:
 *
 * Brief description: finds a file along a path, kind of like
 * 		      a "esps which command"
 *
 */

static char *sccs_id = "@(#)findespsfi.c	1.2	5/28/91	ERL";

/* INCLUDE FILES */

#include <esps/esps.h>

#include <stdio.h>

/* GLOBAL VARIABLES */

int debug_level = 0;

/* LOCAL FUNCTION DECLARATIONS */

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "5/28/91"
#define EC_SCCS_VERSION "1.2"

/* LOCAL TYPEDEFS AND STRUCTURES */

/* LOCAL MACROS */

static Usage(progname)
  char *progname;
{
  printf("Usage: %s filename defpath [environment_variable]\n",progname);
  exit(1);
}

int
main(argc, argv)
     int  argc;
     char **argv;
{
 char *path;

 if (getenv("DEBUGON") != NULL)
   debug_level = 2;

 if (argc < 3 || argc > 4)
 {
   Usage(argv[0]);
 }
 else
 {
   if (argc == 3)
     path = find_esps_file(NULL,argv[1],argv[2],NULL);
   else
     path = find_esps_file(NULL,argv[1],argv[2],argv[3]);

   if (path != NULL)
   {
     printf("%s\n",path);
     exit(0);
   }
   else
   {
    exit(1);
   }
 }

}


