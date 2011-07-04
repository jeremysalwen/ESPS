/*----------------------------------------------------------------------+
|									|
|  This material contains proprietary software of Entropic Speech,	|
|  Inc. Any reproduction, distribution, or publication without the	|
|  prior written permission of Entropic Speech, Inc. is strictly	|
|  prohibited.  Any public distribution of copies of this work		|
|  authorized in writing by Entropic Speech, Inc. must bear the notice	|
| 									|
|     "Copyright (c) 1989-1990 Entropic Speech, Inc.			|
|      Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.	|
|                   All rights reserved."				|
| 									|
+-----------------------------------------------------------------------+
|									|
|  Symbol definitions for module locks.c and its users.			|
|									|
+----------------------------------------------------------------------*/
/* @(#)locks.h	1.3	2/20/96	ESI */

#ifndef locks_H
#define locks_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#define LCKFIL_OK   	0
#define LCKFIL_INUSE	1
#define LCKFIL_ERR  	2

#ifdef __cplusplus
}
#endif

#endif /* locks_H */
