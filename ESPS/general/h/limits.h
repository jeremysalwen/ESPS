/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|     "Copyright (c) 1987 Entropic Speech, Inc.				|
|      Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.	|
|                   All rights reserved."				|
|									|
+-----------------------------------------------------------------------+
|									|
|  limits.h								|
|									|
|  Machine-dependent limits on the ranges of numerical data types.	|
|  In a C implementation conforming to the proposed ANSI standard,	|
|  the contents of this file could be replaced with			|
|  #include <limits.h>							|
|  #include <float.h>							|
|  as the names of the defined constants are compatible.		|
|									|
+----------------------------------------------------------------------*/
/* Sccs information:  @(#)limits.h	1.20 2/21/96 ESI */

#ifndef limits_H
#define limits_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#include <limits.h>
#if !defined(DBL_MAX) || !defined(DBL_MIN)
#if defined(SUN4) && !defined(OS5)

#define DBL_MAX         1.7976931348623157E+308 
#define DBL_MIN         2.2250738585072014E-308 

#define FLT_MAX         ((float) 3.40282347E+38)
#define FLT_MIN         ((float) 1.17549435E-38)

#else
#include <float.h>
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* limits_H */
