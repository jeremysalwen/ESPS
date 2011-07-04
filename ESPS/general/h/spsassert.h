/*
 * @(#)spsassert.h	1.7 7/23/96 ESI
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987-1990 Entropic Speech, Inc.
 *     Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * Written by:  John Shore
 *
 * SPS version of assert macro; prints a message as well
 */

#ifndef spsassert_H
#define spsassert_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#include <assert.h>

#ifndef lint
#define _spsassert(ex,ms) {if (!(ex)){fprintf(stderr,"SPS assertion failed: %s\n",(ms));assert(0);exit(1);}}
#define spsassert(ex,ms) {if (!(ex)){fprintf(stderr,"SPS assertion failed: %s\n",(ms));assert(0);exit(1);}}
#else
#define _spsassert(ex,ms) ;
#define spsassert(ex,ms) ;
#endif


#ifdef __cplusplus
}
#endif

#endif /* spsassert_H */
