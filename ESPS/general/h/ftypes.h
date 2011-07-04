/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."
 				
  @(#)ftypes.h	1.10 2/20/96 ESI
*/

#ifndef ftypes_H
#define ftypes_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/* SPS file types 
 * add new ones to the end, keep in mind that shorts are used store these
 * DO NOT alter existing ones unless you are sure of what you are doing
 */


#define FT_SPEC 5
#define FT_SPECT_DB 6
#define FT_ANA 7
#define FT_PIT 8
#define FT_SD 9
#define FT_ROS 10
#define FT_FILT 11
#define FT_SCBK 12
#define FT_FEA 13

extern char *file_type[];

#ifdef __cplusplus
}
#endif

#endif /* ftypes_H */
