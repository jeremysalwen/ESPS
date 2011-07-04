/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.
 *     Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * Program: xecolors.h  @(#)xecolors.h	1.4	2/20/96	ESI
 *
 * Written by:  John Shore
 * Checked by:
 *
 * pre-defined single colors for XView programs
 */

#ifndef xecolors_H
#define xecolors_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


#define NUM_COLORS 11

#define EC_WHITE   0
#define EC_RED     1
#define EC_GREEN   2
#define EC_BLUE    3
#define EC_YELLOW  4
#define EC_BROWN   5
#define EC_GRAY    6
#define EC_BLACK   7

#define EC_ORANGE  8
#define EC_AQUA    9
#define EC_PINK    10


/*extern Xv_singlecolor *ecolors[];*/


#ifdef __cplusplus
}
#endif

#endif /* xecolors_H */
