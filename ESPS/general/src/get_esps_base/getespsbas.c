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
 * Written by: Ken Nelson 
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)getespsbas.c	1.1	5/28/91	ERL";

/* INCLUDE FILES */

#include <esps/esps.h> 

#include <stdio.h>

/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "5/28/91"
#define EC_SCCS_VERSION "1.1"

int
main(argc, argv)
     int  argc;
     char **argv;
{
  puts(get_esps_base(NULL));
}

