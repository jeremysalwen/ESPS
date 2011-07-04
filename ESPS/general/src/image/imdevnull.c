/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1988, 1989 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: imdevnull.c							|
|									|
|  This module is for systems that don't have native graphics
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#if !defined(SP40) && !defined(SUNVUE)

#ifndef lint
static char *sccs_id = "@(#)imdevnull.c	1.3	7/31/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

void
mcd_init()
{
	spsassert(0,"image: mcd_init called in imdevnull");
}


void
mcd_fin()
{
	spsassert(0,"image: mcd_fin called in imdevnull");
}

void
mcd_default_size(w, h)
    long    *w, *h;
{
	spsassert(0,"image: mcd_default_size called in imdevnull");
}

int
mcd_depth()
{
	spsassert(0,"image: mcd_depth called in imdevnull");
}

#endif
