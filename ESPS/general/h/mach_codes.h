/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				
 @(#)mach_codes.h	1.15 7/23/96 ESI

*/

#ifndef mach_codes_H
#define mach_codes_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* this array contains a list of supported machine types
*/
char *machine_codes[] = {
	"Invalid", 	/* 0 */
	"Concurrent", 	/* 1 */
	"Sun3", 	/* 2 */
	"Convex", 	/* 3 */
	"Sun4", 	/* 4 */
	"HP300",	/* 5 */
	"Sun386i",	/* 6 */
	"DS3100",	/* 7 */
	"Mac2",		/* 8 */
	"SGI", 		/* 9 */
	"HP800",	/* 10 */
	"DEC_Vax",	/* 11 */
	"DG_Aviion",	/* 12 */
	"Apollo_68K", /* 13 */
	"Apollo_10000", /* 14 */
	"HP400",	/* 15 */
	"Cray",		/* 16 */
	"Sony_RISC",	/* 17 */
	"Sony_68K",	/* 18 */
	"Stardent_3000", /* 19 */
	"IBM_RS6000",	/* 20 */
	"HP700",	/* 21 */
	"Alpha",	/* 22 */
	"Solaris86",	/* 23 */
	"Linux",	/* 24 */
	NULL};

#ifdef __cplusplus
}
#endif

#endif /* mach_codes_H */
