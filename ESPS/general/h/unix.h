/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Speech, Inc.
                     All rights reserved"
  
  Alan Parker, ESI

  sccs: @(#)unix.h	3.31 2/21/96 ERL

*/

#ifndef unix_H
#define unix_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>
 				
/* This file contains declarations for the Unix functions */
/* It doesn't list everthing, just those functions that are common to
   all supported Unix versions (supported by us) and those used by esps */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef MACOS
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#ifdef __cplusplus
}
#endif

#endif /* unix_H */
