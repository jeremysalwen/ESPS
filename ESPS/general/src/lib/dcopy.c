/*---------------------------------------------------------------------------+
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				


  d_copy - copy an array of doubles - Joseph T. Buck			     |
+---------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)dcopy.c	1.4	7/29/88 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

void
d_copy (out, in, n)
double *out, *in;
int n;
{
#ifdef UE
    spsassert(out != NULL,"d_copy: out is NULL");
    spsassert(in != NULL,"d_copy: in is NULL");
#endif

    while (n-- > 0) *out++ = *in++;
    return;
}
