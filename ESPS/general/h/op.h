/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)op.h	1.3 2/20/96 ERL
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Include file for ESPS programs that use arr_op(3-ESPS).
 *
 */

#ifndef op_H
#define op_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * Codes for binary operations on numeric scalars.
 * Keep consistent with strings in array operation_types (see below).
 */

#define OP_ADD	 0
#define OP_SUB	 1
#define OP_MUL	 2
#define OP_DIV	 3
#define OP_PWR	 4
#define OP_CPLX	 5

/*
 * Strings corresponding to symbol names without the "OP_" prefix.
 * Defined in arr_op.c.
 */

extern char *operation_names[];

/*
 * Function declaration.
 */

char *
arr_op ARGS((int op, long int num,
	     char *src1, int src1_type, char *src2, int src2_type,
	     char *dest, int dest_type));

#ifdef __cplusplus
}
#endif

#endif /* op_H */
