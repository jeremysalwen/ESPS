/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
*/
#ifndef lint
static char *sccs_id = "@(#)lar_to_rc.c	1.3 11/19/87 EPI";
#endif

#include <math.h>

float
lar_to_rc(lar)
double lar;

{
  double exp(), log();
  float x;

  
  x = exp(lar*log((double)10));
  return( (x-1)/(x+1) );

}
