/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

*/
 	



#include <stdio.h>

#ifndef lint
	static char *sccsid = "@(#)faterr.c	1.2	7/17/87 ESI";
#endif

faterr(sub,messag,n)
char *sub,*messag;
int n;
{
/*    Subroutine for fatal error messages */

fprintf(stderr,"			FATAL ERROR in : %s \n",sub);
fprintf(stderr,"%s\n",messag);
exit(n);
}

/*    ---------------------------------------------------------    */

