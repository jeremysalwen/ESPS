/*---------------------------------------------------------------------------+

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
  f_copy.  Copy an array of floats
+---------------------------------------------------------------------------*/

#ifndef lint
	static char *sccsid = "@(#)fcopy.c	1.2 7/17/87 ESI";
#endif

void
f_copy (out, in, n)
float *out, *in;
int n;
{

#ifdef UE
    spsassert(out != NULL, "f_copy: out is NULL");
    spsassert(in != NULL, "f_copy: in is NULL");
    spsassert(n < 0, "f_copy: n < 0");
#endif

    while (n-- > 0) *out++ = *in++;
    return;
}
