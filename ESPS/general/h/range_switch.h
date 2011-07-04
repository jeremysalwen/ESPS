/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)range_switch.h	1.1 2/20/96 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS programs that use various range_switch functions.
 *
 */

#ifndef range_switch_H
#define range_switch_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

void
range_switch ARGS((char *text, int *startp, int *endp, int us));

void
lrange_switch ARGS((char *text, long int *startp, long int *endp, int us));

void
frange_switch ARGS((char *text, double *startp, double *endp));

long *
grange_switch ARGS((char *text, long int *n_ele));

long *
fld_range_switch ARGS((char *text, char **name,
		       long int *array_len, struct header *hd));

void
trange_switch ARGS((char *text, struct header *hd,
		    long int *startp, long int *endp));


#ifdef __cplusplus
}
#endif

#endif /* range_switch_H */
