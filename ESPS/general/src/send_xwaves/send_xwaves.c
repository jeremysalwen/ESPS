/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1993  Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  David Talkin
 * 
 * Brief description:  send commands to an xwaves+ server vis RPC
 * 
 */

/* send_xwaves.c */

static char    *sccs_id = "@(#)send_xwaves.c	1.16 1/4/96 ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/xwaves_ipc.h>

#define SYNTAX \
"usage: send_waves [-h display host] [-n destination name] [-x] [-B] [-d timeout delay] [-D startup delay] [commands]\n \
      if [commands] is missing, then standard input is read.\n \
      -B disables blocking mode; -d sets blocking delay [2000sec].\n"

int             debug_level;

has_content(str)
   char           *str;
{
   if (str && *str) {
      register char   c;

      while ((c = *str++))
	 if ((c != '\t') && (c != '\n') && (c != ' '))
	    return (1);
   }
   return (0);
}

main(ac, av)
   int             ac;
   char          **av;
{
   char            in[1001], *pin = in;
   char           *result;
   extern int      optind;
   extern char    *optarg;
   int             ch, delay = 200000, startup_delay = 10, 
                   no_wait = FALSE, com_line;
   int             res, nstr, n;
   char           *program;
   char           *dest = "xwaves";
   char	          *display_name=NULL;
   Sxptr          *sxptr;
   int	          atoi();

   debug_level = 0;


   program = av[0];

   while ((ch = getopt(ac, av, "Bd:D:x:h:n:?")) != EOF)
      switch (ch) {
      case 'x':
	 debug_level = atoi(optarg);
	 break;
      case 'B':
	 no_wait = TRUE;
	 break;
      case 'd':
	 delay = atoi(optarg) * 1000;
	 if (delay < 1000)
	    delay = 1000;
	 break;
      case 'D':
	 startup_delay = atoi(optarg);
	 break;
      case 'h':
	 display_name = optarg;
	 break;
      case 'n':
	 dest = optarg;
         break;
      case '?':
      default:
	 fprintf(stderr, SYNTAX);
	 exit(-1);
      }

   av += optind;
   ac -= optind;
   *in = 0;
   if (ac > 0) {
      nstr = strlen(*av);
      while (ac-- > 0) {
	 if (*in)
	    strcat(in, " ");
	 nstr += strlen(*av);
	 if (nstr >= 1000) {
	    fprintf(stderr, " Line too long in %s.\n", av[0]);
	    exit(-1);
	 }
	 strcat(in, *av);
	 av++;
      }
      com_line = TRUE;		/* input from command line */
   } else
      com_line = FALSE;		/* input from stdin */


   do {
      if (sxptr = OpenXwaves(display_name, dest, program)) {
	 break;
      }
      if (startup_delay > 0)
	 sleep(1);
   } while (startup_delay-- > 0);

   if (sxptr) {
      if (no_wait) {
	 if (!com_line)
	    while (fgets(in, 999, stdin) != NULL) {
	       if (has_content(in)) {
		  res = SendXwavesNoReply(NULL, NULL, sxptr, pin);
		  if (!res)
		     exit(-1);
	       }
	    }
	 else {
	    res = SendXwavesNoReply(NULL, NULL, sxptr, pin);
	    if (!res)
	       exit(-1);
	 }
	 exit(0);
      } else {
	 int             rv = 0;

	 if (!com_line)
	    if (fgets(in, 999, stdin) == NULL)
	       exit(-1);

	 do {
	    if (has_content(in)) {
	       result = SendXwavesReply(NULL, NULL, sxptr, pin, delay);
	       if (!result) {
		  fprintf(stderr, "Error getting return from %s\n", dest);
                  CloseXwaves(sxptr);
                  exit(-1);
	       }
	       nstr = 0;
	       if (!strncmp("returned", result, (n = strlen("returned")))) {
		  nstr = n + 1;	/* skip space */
		  fprintf(stdout, "%s\n", &result[nstr]);
	       } else {
		  if (strncmp("ok", result, 2))
		     rv = -1;
		  if (debug_level)
		     fprintf(stderr, "%s\n", &result[nstr]);
	       }
	    }
	 } while (!com_line && (fgets(in, 999, stdin) != NULL));
         CloseXwaves(sxptr);
	 exit(rv);
      }
   } else {
      fprintf(stderr, "Cannot open a connection to %s\n", dest);
      exit(-1);
   }
}
