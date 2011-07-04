 	

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
 * Written by: Alan Parker
 * Checked by:
 * Revised by: Ken Nelson
 *
 * Brief description: 
 *
 */

#define MAXIFILES 15		/* maximum number of input files */
#define MAXWFIELDS 50		/* maximum number of fields on a WITH */
#define PAGER "more"

#ifndef PSPS
#define PSPS (char*) build_esps_path("bin/psps")
#endif

#ifndef HELPFILE
#define HELPFILE (char *) build_esps_path("lib/select.hlp")
#endif

#define PROG "select"
#define NUM_ERR 2
#define LINSIZ 512

#include <stdio.h>
#include "unix.h"
#include <esps/esps.h>
#include <esps/fea.h>
#include "message.h"
#include <math.h>
#include <errno.h>













