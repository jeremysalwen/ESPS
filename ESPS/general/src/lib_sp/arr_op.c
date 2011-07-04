/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Element-by-element binary operations on pairs of numeric arrays.
 *
 */

static char *sccs_id = "@(#)arr_op.c	1.2	12/1/93	ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/func.h>
#include <esps/op.h>

extern char *type_convert();
extern void warn_on_clip();
extern char *arr_func();

/* Keep following strings consistent with symbols in esps/op.h. */

char *operation_names[] = {
     "ADD",
     "SUB",
     "MUL",
     "DIV",
     "PWR",
     "CPLX",
     NULL};

/*!*//* Scalar first or second arg?  Outer "prod"?  Non-unit strides? */

char *
arr_op(op, num, src1, src1_type, src2, src2_type, dest, dest_type)
    int		op;
    long	num;
    char	*src1;
    int		src1_type;
    char	*src2;
    int		src2_type;
    char	*dest;
    int		dest_type;
{
    long	i;
    char	*result;
    double_cplx	*cdat1, *cdat2;
    double	*data1, *data2;
    double	x1, y1, x2, y2, r, s;

    switch (op)
    {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
	if (is_type_complex(src1_type) || is_type_complex(src2_type))
	{
/*!*//* Could handle 1 real and 1 complex operand separately. */

	    /* Avoid second conversion when dest_type is DOUBLE_CPLX:
	       develop result directly in dest (supplied or malloc'ed). */

	    cdat1 =
		(double_cplx *)
		    type_convert(num, src1, src1_type,
				 (dest_type == DOUBLE_CPLX)
				 ? dest : (char *) NULL,
				 DOUBLE_CPLX, (void (*)()) NULL);

	    cdat2 =
		(double_cplx *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE_CPLX, (void (*)()) NULL);

	    switch (op)
	    {
	    case OP_ADD:
		for (i = 0; i < num; i++)
		{
		    cdat1[i].real += cdat2[i].real;
		    cdat1[i].imag += cdat2[i].imag;
		}
		break;
	    case OP_SUB:
		for (i = 0; i < num; i++)
		{
		    cdat1[i].real -= cdat2[i].real;
		    cdat1[i].imag -= cdat2[i].imag;
		}
		break;
	    case OP_MUL:
		for (i = 0; i < num; i++)
		{
		    x1 = cdat1[i].real;
		    y1 = cdat1[i].imag;
		    x2 = cdat2[i].real;
		    y2 = cdat2[i].imag;

		    cdat1[i].real = x1*x2 - y1*y2;
		    cdat1[i].imag = x1*y2 + y1*x2;
		}
		break;
	    case OP_DIV:
		for (i = 0; i < num; i++)
		{
		    x1 = cdat1[i].real;
		    y1 = cdat1[i].imag;
		    x2 = cdat2[i].real;
		    y2 = cdat2[i].imag;
			
/*!*//* Denominator may underflow or overflow.  Check and scale if necessary. */

		    r = x2*x2 + y2*y2;
		    cdat1[i].real = (x1*x2 + y1*y2)/r;
		    cdat1[i].imag = (y1*x2 - x1*y2)/r;
		}
		break;
	    }

	    if (dest_type == DOUBLE_CPLX)
		result = (char *) cdat1;
	    else
	    {
		result = type_convert(num, (char *) cdat1, DOUBLE_CPLX,
				      dest, dest_type, warn_on_clip);
		free((char *) cdat1);
	    }

	    free((char *) cdat2);
	}
	else /* both arguments real */
/*!*//* Should handle (integer, integer) -> integer separately
	for ADD, SUB, and MUL. */
	{
	    data1 =
		(double *)
		    type_convert(num, src1, src1_type,
				 (dest_type == DOUBLE)
				 ? dest : (char *) NULL,
				 DOUBLE, (void (*)()) NULL);
	    data2 = 
		(double *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE, (void (*)()) NULL);

	    switch (op)
	    {
	    case OP_ADD:
		for (i = 0; i < num; i++)
		{
		    data1[i] += data2[i];
		}
		break;
	    case OP_SUB:
		for (i = 0; i < num; i++)
		{
		    data1[i] -= data2[i];
		}
		break;
	    case OP_MUL:
		for (i = 0; i < num; i++)
		{
		    data1[i] *= data2[i];
		}
		break;
	    case OP_DIV:
		for (i = 0; i < num; i++)
		{
		    data1[i] /= data2[i];
		}
		break;
	    }

	    if (dest_type == DOUBLE)
		result = (char *) data1;
	    else
	    {
		result = type_convert(num, (char *) data1, DOUBLE,
				      dest, dest_type, warn_on_clip);
		free((char *) data1);
	    }

	    free((char *) data2);
	}
	break;
    case OP_PWR:
	if (is_type_complex(src1_type) || is_type_complex(src2_type))
	{
	    /* Avoid second conversion when dest_type is DOUBLE_CPLX:
	       develop result directly in dest (supplied or malloc'ed). */

	    cdat1 =
		(double_cplx *)
		    arr_func(FN_LOG, num, src1, src1_type,
			     (dest_type == DOUBLE_CPLX)
			     ? dest : (char *) NULL,
			     DOUBLE_CPLX);

	    cdat2 =
		(double_cplx *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE_CPLX, (void (*)()) NULL);

	    for (i = 0; i < num; i++)
	    {
		x1 = cdat1[i].real;
		y1 = cdat1[i].imag;
		x2 = cdat2[i].real;
		y2 = cdat2[i].imag;

		if (x2 == 0.0 && y2 == 0.0)
		{
		    cdat1[i].real = 1.0;
		    cdat1[i].imag = 0.0;
		}
		else if (x1 == FN_LOG_ZERO && x2 > 0.0)
		{
		    cdat1[i].real = 0.0;
		    cdat1[i].imag = 0.0;
		}
		else
		{
		    r = exp(x1*x2 - y1*y2);

		    if (r == 0.0)
		    {
			cdat1[i].real = 0.0;
			cdat1[i].imag = 0.0;
		    }
		    else
		    {
			s = x1*y2 + y1*x2;

			cdat1[i].real = r*cos(s);
			cdat1[i].imag = r*sin(s);
		    }
		}
	    }

	    if (dest_type == DOUBLE_CPLX)
		result = (char *) cdat1;
	    else
	    {
		result = type_convert(num, (char *) cdat1, DOUBLE_CPLX,
				      dest, dest_type, warn_on_clip);
		free((char *) cdat1);
	    }

	    free((char *) cdat2);
	}
	else /* both arguments real */
/*!*//* should break out case real^integer */
	{
	    /* Avoid second conversion when dest_type is DOUBLE_CPLX:
	       develop result directly in dest (supplied or malloc'ed). */

	    cdat1 =
		(double_cplx *)
		    arr_func(FN_LOG, num, src1, src1_type,
			     (dest_type == DOUBLE_CPLX)
			     ? dest : (char *) NULL,
			     DOUBLE_CPLX);

	    data2 = 
		(double *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE, (void (*)()) NULL);

	    for (i = 0; i < num; i++)
	    {
		x1 = cdat1[i].real;
		y1 = cdat1[i].imag;
		x2 = data2[i];

		if (x2 == 0.0)
		{
		    cdat1[i].real = 1.0;
		    cdat1[i].imag = 0.0;
		}
		else if (x1 == FN_LOG_ZERO && x2 > 0.0)
		{
		    cdat1[i].real = 0.0;
		    cdat1[i].imag = 0.0;
		}
		else
		{
		    r = exp(x1*x2);

		    if (r == 0.0)
		    {
			cdat1[i].real = 0.0;
			cdat1[i].imag = 0.0;
		    }
		    else
		    {
			s = y1*x2;
			cdat1[i].real = r*cos(s);
			cdat1[i].imag = r*sin(s);
		    }
		}
	    }

	    if (dest_type == DOUBLE_CPLX)
		result = (char *) cdat1;
	    else
	    {
		result = type_convert(num, (char *) cdat1, DOUBLE_CPLX,
				      dest, dest_type, warn_on_clip);
		free((char *) cdat1);
	    }

	    free((char *) data2);
	}
	break;
    case OP_CPLX:
	/* Avoid second conversion when dest_type is DOUBLE_CPLX:
	   develop result directly in dest (supplied or malloc'ed). */

	cdat1 =
	    (double_cplx *)
		type_convert(num, src1, src1_type,
			     (dest_type == DOUBLE_CPLX)
			     ? dest : (char *) NULL,
			     DOUBLE_CPLX, (void (*)()) NULL);

	if (is_type_complex(src1_type) || is_type_complex(src2_type))
	{
	    cdat2 =
		(double_cplx *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE_CPLX, (void (*)()) NULL);

	    for (i = 0; i < num; i++)
	    {
		cdat1[i].real -= cdat2[i].imag;
		cdat1[i].imag += cdat2[i].real;
	    }

	    free((char *) cdat2);
	}
	else /* both arguments real */
	{
	    data2 = 
		(double *)
		    type_convert(num, src2, src2_type, (char *) NULL,
				 DOUBLE, (void (*)()) NULL);

	    for (i = 0; i < num; i++)
	    {
		cdat1[i].imag = data2[i];
	    }

	    free((char *) data2);
	}

	if (dest_type == DOUBLE_CPLX)
	    result = (char *) cdat1;
	else
	{
	    result = type_convert(num, (char *) cdat1, DOUBLE_CPLX,
				  dest, dest_type, warn_on_clip);
	    free((char *) cdat1);
	}

	break;
    default:
	Fprintf(stderr, "%s: unknown operation code %d.\n", "arr_op", op);
	return NULL;
	break;
    }

    return result;
}
