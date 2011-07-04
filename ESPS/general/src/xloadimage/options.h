/* options.h:
 *
 * optionNumber() definitions
 *
 * jim frost 10.03.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

/* @(#)options.h	1.2  10/14/90 */

#include "copyright.h"

#define OPT_NOTOPT   -1
#define OPT_BADOPT   -2
#define OPT_SHORTOPT -3

int optionNumber(); /* options.c */
