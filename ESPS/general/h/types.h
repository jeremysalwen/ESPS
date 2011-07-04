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
 * @(#)types.h	1.2 5/16/97 ERL
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for functions that deal with ESPS data types.
 *
 */

#ifndef types_H
#define types_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Function declarations.
 */

char *
type_convert ARGS((long int num, char *source, int src_type,
		   char *destination, int dest_type, void (*clip_action)()));

void
warn_on_clip ARGS((long int i, double_cplx val));

int
is_field_complex ARGS((struct header *h, char *name));

int
is_file_complex ARGS((struct header *h));

int
is_type_complex ARGS((int type));

int
is_type_numeric ARGS((int type));

short
cover_type ARGS((int t1, int t2));
/* t1 and t2 are specified as short in pre-ANSI style in cover_type.c;
 * therefore they are passed as int.
 */

int
typesiz ARGS((int type));


#ifdef __cplusplus
}
#endif

#endif /* types_H */
