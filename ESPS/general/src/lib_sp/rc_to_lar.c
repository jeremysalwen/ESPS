/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*/
#ifndef lint
	static char *sccs_id = "@(#)rc_to_lar.c	1.7 2/8/88 ESI";
#endif

#include <stdio.h>
#include <esps/limits.h>
#define log10(x) log(x)/log((double)10) 


int
rc_to_lar(rc,lar)
double rc;
float *lar;

{
  double x, log();
  if(rc >= 1.0) {
  	*lar = FLT_MAX;
	return 1;}

  if(rc <= -1.0) {
  	*lar = - FLT_MAX;
	return 1;}

  x = (1.0+rc) / (1.0-rc);
  *lar = log10(x);
  return 0;

}
