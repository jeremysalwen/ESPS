/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987-1990 Entropic Speech, Inc.
       Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
                     All rights reserved."

  @(#)constants.h	1.4 2/20/96 ESI				
*/

#ifndef constants_H
#define constants_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#define PI 3.14159265358979323846
#define M_ORDR 50
#define NPR 3 /* maximum size for zero-pole pre/de-emphasis */
#define SCOV_EPS  1.0e-8 /* convergence test factor for struct_cov */

#ifdef __cplusplus
}
#endif

#endif /* constants_H */
