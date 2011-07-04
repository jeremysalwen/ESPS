/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)dsplock.h	1.2 9/26/95 ATT/ERL/ESI */
/*----------------------------------------------------------------------+
|									|
|  This material contains proprietary software of Entropic Speech,	|
|  Inc. Any reproduction, distribution, or publication without the	|
|  prior written permission of Entropic Speech, Inc. is strictly	|
|  prohibited.  Any public distribution of copies of this work		|
|  authorized in writing by Entropic Speech, Inc. must bear the notice	|
| 									|
|    "Copyright (c) 1989 Entropic Speech, Inc. All rights reserved."	|
| 									|
+-----------------------------------------------------------------------+
|									|
|  For programs that use lockfiles to control contention for the dsp32  |
|  board.								|
|									|
+----------------------------------------------------------------------*/
/* dsplock.h */

#include <esps/locks.h>
#define DSP_LOCK(t) set_lock((t), "dspLCK")
#define DSP_UNLOCK rem_lock("dspLCK")
