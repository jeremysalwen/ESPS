/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)window.h	1.8 24 Mar 1997 ESI/ERL
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This file should be included in all ESPS programs that call window()
 */

#ifndef window_H
#define window_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/* Add new symbols at the end of this list,
 * keep the numbering consecutive,
 * don't change the numbering of existing symbols,
 * and keep the definition of window_types in
 * lib_sp/window.c consistent with this list, so that
 * window_types[WT_<name>] is always "WT_<name>".
 */

#define WT_NONE 0
#define WT_RECT 1
#define WT_HAMMING 2
#define WT_TRIANG 3
#define WT_HANNING 4
#define WT_COS4 5
#define WT_KAISER 6
#define WT_ARB 7
#define WT_SINC 8
#define WT_SINC_C4 9

#ifndef LIB
extern char *window_types[];
#endif

int
window ARGS((long nsamp, float *in, float *out, int type, double *p));

int
windowd ARGS((long nsamp, double *in, double *out, int type, double *p));

int
windowcf ARGS((long nsamp,
	       float_cplx *in, float_cplx *out, int type, float_cplx *p));

int
windowcd ARGS((long nsamp,
	       double_cplx *in, double_cplx *out, int type, double_cplx *p));

int
win_type_from_name ARGS((char *name));


#ifdef __cplusplus
}
#endif

#endif /* window_H */
