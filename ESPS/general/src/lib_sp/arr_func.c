/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Element-by-element functions on real and complex arrays.
 *
 */

static char *sccs_id = "@(#)arr_func.c	1.6	6/28/93	ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/func.h>
#include <esps/constants.h>

#define HYPOT_AVAILABLE

#if !defined(M5600) && !defined(hpux)
#define ATANH_AVAILABLE
#endif

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern char *type_convert();
extern void warn_on_clip();

#ifdef HYPOT_AVAILABLE
double hypot();
#define GET_CABS(x, y, r) {(r) = hypot((x), (y));}
#else
#define GET_CABS(x, y, r) {double X = ABS(x), Y = ABS(y); \
    if (X > Y) {Y /= X; (r) = X * sqrt(1 + Y*Y);} \
    else if (Y == 0.0) (r) = 0.0; \
    else {X /= Y; (r) = Y * sqrt(1 + X*X);}}
#endif
/* {r = sqrt(x*x + y*y);} is simpler than the above, but more prone to
 * overflow or underflow in extreme cases.
 */

#ifdef ATANH_AVAILABLE
double atanh();
#else
static double
atanh(x)
    double  x;
{
    if (ABS(x) > 0.33) /* precise value not critical */
    {
	/* theoretically correct, but loses precision when x is small */

	return 0.5 * log((1+x)/(1-x));
    }
    else
    {
	double	sq, n, p, s, s0;

	/* use power-series expansion when x is small */
	sq = x*x;
	n = 1.0;
	p = x;
	s = 0.0;	/* will add in first term (x) at end */
	do
	{
	    s0 = s;
	    n += 2.0;
	    p *= sq;
	    s += p/n;
	} while (s != s0);

	return x + s;
    }
}
#endif

/*!*//* Cf. M_LOG10_E and M_LN10 in math.h */
#define LOG10_E 0.43429448190325182765
#define LOGE_10 2.30258509299404568402

/* Keep following strings consistent with symbols in esps/func.h. */

char *function_types[] = {
     "NONE",
     "ABS",
     "ARG",
     "ATAN",
     "CONJ",
     "COS",
     "EXP",
     "IM",
     "LOG",
     "LOG10",
     "RE",
     "RECIP",
     "SGN",
     "SIN",
     "SQR",
     "SQRT",
     "TAN",
     "EXP10",
     NULL};


char *
arr_func(func, num, src, src_type, dest, dest_type)
    int	    func;
    long    num;
    char    *src;
    int	    src_type;
    char    *dest;
    int	    dest_type;
{
    int	    i;
    char    *result;

/*!*//* Check for NULL src, nonnumeric types?
	(type_convert will catch these.) */

    if (is_type_complex(src_type) || is_type_complex(dest_type))
    {
	double_cplx *cdata;
	double	    x, y;
	double	    c, r, s, t;
	double	    u, v;

	/* Avoid second conversion when dest_type is DOUBLE_CPLX:
	   go direct to dest (supplied or malloc'ed). */

	cdata =
	    (double_cplx *)
		type_convert(num, src, src_type,
			     (dest_type == DOUBLE_CPLX) ? dest : (char *) NULL,
			     DOUBLE_CPLX, (void (*)()) NULL);

	switch (func)
	{
	case FN_NONE:
	    break;
	case FN_ABS:
	    for (i = 0; i < num; i++)
	    {
		GET_CABS(cdata[i].real, cdata[i].imag, cdata[i].real);
		cdata[i].imag = 0.0;
	    }
	    break;
	case FN_ARG:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		cdata[i].real = (y != 0) ? atan2(y, x) : (x >= 0) ? 0.0 : PI;
		cdata[i].imag = 0.0;
	    }
	    break;
	case FN_ATAN:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		if (ABS(x) <= 1 && ABS(y) <= 1)
		{
		    cdata[i].real = 0.5*atan2(2*x, 1 - x*x - y*y);
		    r = 2*y / (1 + x*x + y*y);
		}
		else if (ABS(x) < ABS(y))
		{
		    r = 1/y;
		    s = x*r;
		    cdata[i].real = (x != 0) ? 0.5*atan2(2*r*s, r*r - s*s - 1)
				    : (y > 0) ? -0.5*PI : 0.5*PI;
		    r = 2*r / (r*r + s*s + 1);
		}
		else
		{
		    r = 1/x;
		    s = y*r;
		    cdata[i].real = 0.5*atan2(2*r, r*r - 1 - s*s);
		    r = 2*r*s / (r*r + 1 + s*s);
		}

		if (ABS(r) < 0.33)	/* precise value not critical */
		    cdata[i].imag = 0.5*atanh(r);
		else		/* close to singularities at +i, -i */
		{
		    r = 1+y;
		    s = 1-y;
		    cdata[i].imag = 0.25*log((r*r + x*x) / (s*s + x*x));
		}
	    }
	    break;
	case FN_CONJ:
	    for (i = 0; i < num; i++)
	    {
		cdata[i].imag = - cdata[i].imag;
	    }
	    break;
	case FN_COS:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		cdata[i].real = cos(x)*cosh(y);
		cdata[i].imag = - sin(x)*sinh(y);
	    }
	    break;
	case FN_EXP:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		r = exp(x);

		if (r == 0.0)
		{		/* result 0 even if sin y or cos y is NaN */
		    cdata[i].real = 0.0;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    cdata[i].real = r*cos(y);
		    cdata[i].imag = r*sin(y);
		}
	    }
	    break;
	case FN_IM:
	    for (i = 0; i < num; i++)
	    {
		cdata[i].real = cdata[i].imag;
		cdata[i].imag = 0.0;
	    }
	    break;
	case FN_LOG:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		GET_CABS(x, y, r);
		if (r == 0.0)
		{
		    cdata[i].real = FN_LOG_ZERO;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    cdata[i].real = log(r);
		    cdata[i].imag =
			(y != 0) ? atan2(y, x) : (x >= 0) ? 0.0 : PI;
		}
	    }
	    break;
	case FN_LOG10:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		GET_CABS(x, y, r);
		if (r == 0.0)
		{
		    cdata[i].real = FN_LOG_ZERO;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    cdata[i].real = log(r)*LOG10_E;
		    cdata[i].imag =
			(y != 0) ? atan2(y, x) * LOG10_E
			: (x >= 0) ? 0.0
			: (PI * LOG10_E);
		}
	    }
	    break;
	case FN_RE:
	    for (i = 0; i < num; i++)
	    {
		cdata[i].imag = 0.0;
	    }
	    break;
	case FN_RECIP:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		/* r = x*x + y*y; u = x/r; v = -y/r; is simpler
		 * but more prone to overflow or underflow.
		 */
		if (ABS(x) > ABS(y))
		{
		    r = y/x;
		    cdata[i].imag = -r * (cdata[i].real = 1.0 / (x + y*r));
		}
		else
		{
		    r = x/y;
		    cdata[i].real = -r * (cdata[i].imag = -1.0 / (x*r + y));
		}
	    }
	    break;
	case FN_SGN:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		GET_CABS(x, y, r);
		if (r == 0.0)
		{
		    cdata[i].real = 0.0;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    cdata[i].real = x/r;
		    cdata[i].imag = y/r;
		}
	    }
	    break;
	case FN_SIN:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		cdata[i].real = sin(x)*cosh(y);
		cdata[i].imag = cos(x)*sinh(y);
	    }
	    break;
	case FN_SQR:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		cdata[i].real = x*x - y*y;
		cdata[i].imag = 2*x*y;
	    }
	    break;
	case FN_SQRT:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		GET_CABS(x, y, r);
		r = sqrt(0.5 * (r + ABS(x)));
		if (r == 0.0)
		{
		    cdata[i].real = 0.0;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    if (x >= 0.0)
		    {
			cdata[i].imag = y / (2.0 * (cdata[i].real = r));
		    }
		    else
		    {
			cdata[i].real =
			    y / (2.0 * (cdata[i].imag = (y >= 0.0) ? r : -r));
		    }
		}
	    }
	    break;
	case FN_TAN:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real;
		y = cdata[i].imag;
		t = tanh(y);
		c = cos(x);
		s = sin(x);
		r = s*t;
		if (ABS(r) < ABS(c))
		{	/*  (tan x (sech y)^2 + i (sec x)^2 tanh y)
			 *  / (1 + (tan x tanh y)^2)
			 */
		    r = r/c;
		    r = 1 + r*r;
		    u = s/c;	/* tan x. */
			    /* (sech y)^2.  Don't compute as 1/(cosh y)^2.*/
		    s = exp(-ABS(y));
		    s = 2*s/(1 + s*s);
		    s = s*s;
		    if (0.5*s == 0.0)
		    {	    /* result fixed even if sin y or cos y is NaN */
			cdata[i].real = 0.0;
			cdata[i].imag = t;
		    }
		    else
		    {
			cdata[i].real = u*s/r;
			v = t/c;
			cdata[i].imag = v/(r*c);
		    }
		}
		else
		{	/*  (cot x (csch y)^2 + i (csc x)^2 coth y)
			 *  / ((cot x coth y)^2 + 1)
			 */
		    r = c/r;
		    r = r*r + 1;
		    if (ABS(y) < 0.6)  /* "|y| small", 0.6 not critical. */
		    {
			c = c/s;	/* cot x */
			u = c/t;
					/* (csch y)^2 = 1/(tanh y)^2 - 1 */
			cdata[i].real = (u/t - c)/r;
			cdata[i].imag = 1/(r*s*s*t);
		    }
		    else		/* "|y| large" */
		    {
			    /* (csch y)^2.  Don't compute as 1/(sinh y)^2. */
			u = exp(-ABS(y));
			u = 2*u/(1 - u*u);
			u = u*u;
			if (0.5*u == 0.0)
			{    /* result fixed even if sin y or cos y is NaN */
			    cdata[i].real = 0.0;
			    cdata[i].imag = t;
			}
			else
			{
			    cdata[i].real = (c/s)*u/r;
			    cdata[i].imag = 1/(r*s*s*t);
			}
		    }
		}
	    }
	    break;
	case FN_EXP10:
	    for (i = 0; i < num; i++)
	    {
		x = cdata[i].real*LOGE_10;
		y = cdata[i].imag*LOGE_10;
		r = exp(x);

		if (r == 0.0)
		{		/* result 0 even if sin y or cos y is NaN */
		    cdata[i].real = 0.0;
		    cdata[i].imag = 0.0;
		}
		else
		{
		    cdata[i].real = r*cos(y);
		    cdata[i].imag = r*sin(y);
		}
	    }
	    break;
	default:
	    (void) fprintf(stderr, "%s: unknown function code %d.\n",
			   "arr_func", func);
	    return NULL;
	    break;
	}

	if (dest_type == DOUBLE_CPLX)
	    result = (char *) cdata;
	else
	{
	    result = type_convert(num, (char *) cdata, DOUBLE_CPLX,
				  dest, dest_type, warn_on_clip);
	    free((char *) cdata);
	}
    }
    else
    {
	double	*data;
	double	x;

	/* Avoid second conversion when dest_type is DOUBLE:
	   go direct to dest (supplied or malloc'ed). */

	data =
	    (double *)
		type_convert(num, src, src_type,
			     (dest_type == DOUBLE) ? dest : (char *) NULL,
			     DOUBLE, (void (*)()) NULL);

	switch (func)
	{
	case FN_NONE:
	    break;
	case FN_ABS:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		data[i] = (x < 0.0) ? -x : x;
	    }
	    break;
	case FN_ARG:
	    for (i = 0; i < num; i++)
	    {
		data[i] = (data[i] < 0.0) ? PI : 0;
	    }
	    break;
	case FN_ATAN:
	    for (i = 0; i < num; i++)
	    {
		data[i] = atan(data[i]);
	    }
	    break;
	case FN_CONJ:
	    break;
	case FN_COS:
	    for (i = 0; i < num; i++)
	    {
		data[i] = cos(data[i]);
	    }
	    break;
	case FN_EXP:
	    for (i = 0; i < num; i++)
	    {
		data[i] = exp(data[i]);
	    }
	    break;
	case FN_IM:
	    for (i = 0; i < num; i++)
	    {
		data[i] = 0.0;
	    }
	    break;
	case FN_LOG:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		if (x == 0.0)
		    data[i] = FN_LOG_ZERO;
		else
		    data[i] = log((x < 0.0) ? -x : x);
	    }
	    break;
	case FN_LOG10:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		if (x == 0.0)
		    data[i] = FN_LOG_ZERO;
		else
		    data[i] = log10((x < 0.0) ? -x : x);
	    }
	    break;
	case FN_RE:
	    break;
	case FN_RECIP:
	    for (i = 0; i < num; i++)
	    {
		data[i] = 1.0/data[i];
	    }
	    break;
	case FN_SGN:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		data[i] = (x < 0.0) ? -1.0
			  : (x == 0.0) ? 0.0
			  : 1.0;
	    }
	    break;
	case FN_SIN:
	    for (i = 0; i < num; i++)
	    {
		data[i] = sin(data[i]);
	    }
	    break;
	case FN_SQR:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		data[i] = x * x;
	    }
	    break;
	case FN_SQRT:
	    for (i = 0; i < num; i++)
	    {
		x = data[i];
		data[i] = (x <= 0.0) ? 0.0 : sqrt(x);
	    }
	    break;
	case FN_TAN:
	    for (i = 0; i < num; i++)
	    {
		data[i] = tan(data[i]);
	    }
	    break;
	case FN_EXP10:
	    for (i = 0; i < num; i++)
	    {
		data[i] = exp(data[i]*LOGE_10);
	    }
	    break;
	default:
	    (void) fprintf(stderr, "%s: unknown function code %d.\n",
			   "arr_func", func);
	    return NULL;
	    break;
	}

	if (dest_type == DOUBLE)
	    result = (char *) data;
	else
	{
	    result = type_convert(num, (char *) data, DOUBLE,
				  dest, dest_type, warn_on_clip);
	    free((char *) data);
	}
    }

    return result;
}
