/* options.c:
 *
 * finds which option in an array an argument matches
 *
 * jim frost 10.03.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)options.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "options.h"

int optionNumber(arg, options)
     char *arg;
     char *options[];
{ int a, b;

  if ((*arg) != '-')
    return(OPT_NOTOPT);
  for (a= 0; options[a]; a++) {
    if (!strncmp(arg + 1, options[a], strlen(arg) - 1)) {
      for (b= a + 1; options[b]; b++)
	if (!strncmp(arg + 1, options[b], strlen(arg) - 1))
	  return(OPT_SHORTOPT);
      return(a);
    }
  }
  return(OPT_BADOPT);
}
