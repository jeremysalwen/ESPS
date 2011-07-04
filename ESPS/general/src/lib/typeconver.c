/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1989 Entropic Speech, Inc. All rights reserved."	|
| 									|
+-----------------------------------------------------------------------+
|									|
|   Module:	    typeconver.c					|
|   									|
|   Written by: Rodney Johnson						|
|   Checked by: 							|
|   									|
|   Function to convert data types of numeric arrays.			|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)typeconver.c	1.3	10/23/89	ESI";
#endif

/*
 * Include files.
 */

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/fea.h>


static int  numeric();		/* is a given type numeric? */
void	    warn_on_clip();


static long	    last_i = -1; /* last index for which clip_action called */
static double_cplx  clipped_val = { 0.0, 0.0 };	/* value that was clipped */

/* What to do if clipping occurs ... */

		/* ... on conversion of real type to any type. */

#define CLIP \
if (clip_action) \
{ clipped_val.real = src[i]; (* clip_action)(i, clipped_val); }

		/* ... on conversion of complex to real. */

#define CLIP_CX \
if (clip_action) \
{ clipped_val.real = src[i].real; clipped_val.imag = src[i].imag; \
  (* clip_action)(i, clipped_val); }

		/* ... on conversion of complex to complex.  */
		/* Set last_i to remember clipping of real part */
		/* so that clipping of imag part won't invoke */
		/* clip_action a second time. */

#define CLIP_RE \
if (clip_action) \
{ clipped_val.real = src[i].real; clipped_val.imag = src[i].imag; \
  last_i = i; (* clip_action)(i, clipped_val); }

#define CLIP_IM \
if (clip_action && last_i != i) \
{ clipped_val.real = src[i].real; clipped_val.imag = src[i].imag; \
  (* clip_action)(i, clipped_val); }

		/* ... on conversion of complex_double.  */
		/* Like 3 cases for general complex types above, /*
		/* but needn't use clipped_val since elements of src array */
		/* can be passed directly to clip_action. */

#define CLIP_DBL_CX \
if (clip_action) \
{ (* clip_action)(i, src[i]); }

#define CLIP_DBL_RE \
if (clip_action) \
{ last_i = i; (* clip_action)(i, src[i]); }

#define CLIP_DBL_IM \
if (clip_action && last_i != i) \
{ (* clip_action)(i, src[i]); }

/*
 * Convert an array of data from one numeric type to another.
 */

char *
type_convert(num, source, src_type, destination, dest_type, clip_action)
    long    num;
    char    *source;
    int	    src_type;
    char    *destination;
    int	    dest_type;
    void    (*clip_action)();
{
    long    i;			/* loop index */

    spsassert(source, "type_convert: source is NULL.");
    spsassert(numeric(src_type), "type_convert: source type not numeric.");
    spsassert(numeric(dest_type),
	      "type_convert: destination type not numeric.");

    if (!destination)
    {
	destination = malloc((unsigned)(num * typesiz(dest_type)));
	spsassert(destination, "type_convert: destination is NULL.");
    }

    switch (src_type)
    {
    case BYTE: {
	char	*src = (char *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	}
	break;}
    case SHORT: {
	short	*src = (short *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP;

		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP;
		}
		else dest[i] = src[i];
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP;
		}
		else dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	}
	break;}
    case LONG: {
	long	*src = (long *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP;
		}
		else dest[i] = src[i];
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP;
		}
		else dest[i] = src[i];
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP;
		}
		else dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP;
		}
		else dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	}
	break;}
    case FLOAT: {
	float	*src = (float *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < LONG_MIN)
		{
		    dest[i] = LONG_MIN;
		    CLIP;
		}
		else if (src[i] > LONG_MAX)
		{
		    dest[i] = LONG_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < LONG_MIN)
		{
		    dest[i].real = LONG_MIN;
		    CLIP;
		}
		else if (src[i] > LONG_MAX)
		{
		    dest[i].real = LONG_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	}
	break;}
    case DOUBLE: {
	double	*src = (double *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < LONG_MIN)
		{
		    dest[i] = LONG_MIN;
		    CLIP;
		}
		else if (src[i] > LONG_MAX)
		{
		    dest[i] = LONG_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i] = src[i] + 0.5;
		else dest[i] = - (0.5 - src[i]);
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < - FLT_MAX)
		{
		    dest[i] = - FLT_MAX;
		    CLIP;
		}
		else if (src[i] > FLT_MAX)
		{
		    dest[i] = FLT_MAX;
		    CLIP;
		}
		else dest[i] = src[i];
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP;
		}
		else if (src[i] > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP;
		}
		else if (src[i] > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < LONG_MIN)
		{
		    dest[i].real = LONG_MIN;
		    CLIP;
		}
		else if (src[i] > LONG_MAX)
		{
		    dest[i].real = LONG_MAX;
		    CLIP;
		}
		else if (src[i] > 0) dest[i].real = src[i] + 0.5;
		else dest[i].real = - (0.5 - src[i]);
		dest[i].imag = 0;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i] < - FLT_MAX)
		{
		    dest[i].real = - FLT_MAX;
		    CLIP;
		}
		else if (src[i] > FLT_MAX)
		{
		    dest[i].real = FLT_MAX;
		    CLIP;
		}
		else dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i];
		dest[i].imag = 0.0;
	    }
	    break;}
	}
	break;}
    case BYTE_CPLX: {
	byte_cplx   *src = (byte_cplx *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	}
	break;}
    case SHORT_CPLX: {
	short_cplx   *src = (short_cplx *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP_CX;
		}
		else dest[i] = src[i].real;
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP_RE;
		}
		else dest[i].real = src[i].real;

		if (src[i].imag < CHAR_MIN)
		{
		    dest[i].imag = CHAR_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > CHAR_MAX)
		{
		    dest[i].imag = CHAR_MAX;
		    CLIP_IM;
		}
		else dest[i].imag = src[i].imag;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	}
	break;}
    case LONG_CPLX: {
	long_cplx   *src = (long_cplx *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP_CX;
		}
		else dest[i] = src[i].real;
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP_CX;
		}
		else dest[i] = src[i].real;
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP_RE;
		}
		else dest[i].real = src[i].real;

		if (src[i].imag < CHAR_MIN)
		{
		    dest[i].imag = CHAR_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > CHAR_MAX)
		{
		    dest[i].imag = CHAR_MAX;
		    CLIP_IM;
		}
		else dest[i].imag = src[i].imag;
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP_RE;
		}
		else dest[i].real = src[i].real;

		if (src[i].imag < SHRT_MIN)
		{
		    dest[i].imag = SHRT_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > SHRT_MAX)
		{
		    dest[i].imag = SHRT_MAX;
		    CLIP_IM;
		}
		else dest[i].imag = src[i].imag;
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	}
	break;}
    case FLOAT_CPLX: {
	float_cplx   *src = (float_cplx *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < LONG_MIN)
		{
		    dest[i] = LONG_MIN;
		    CLIP_CX;
		}
		else if (src[i].real > LONG_MAX)
		{
		    dest[i] = LONG_MAX;
		    CLIP_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < CHAR_MIN)
		{
		    dest[i].imag = CHAR_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > CHAR_MAX)
		{
		    dest[i].imag = CHAR_MAX;
		    CLIP_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < SHRT_MIN)
		{
		    dest[i].imag = SHRT_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > SHRT_MAX)
		{
		    dest[i].imag = SHRT_MAX;
		    CLIP_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < LONG_MIN)
		{
		    dest[i].real = LONG_MIN;
		    CLIP_RE;
		}
		else if (src[i].real > LONG_MAX)
		{
		    dest[i].real = LONG_MAX;
		    CLIP_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < LONG_MIN)
		{
		    dest[i].imag = LONG_MIN;
		    CLIP_IM;
		}
		else if (src[i].imag > LONG_MAX)
		{
		    dest[i].imag = LONG_MAX;
		    CLIP_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i].real = src[i].real;
		dest[i].imag = src[i].imag;
	    }
	    break;}
	}
	break;}
    case DOUBLE_CPLX: {
	double_cplx   *src = (double_cplx *) source;

	switch (dest_type)
	{
	case BYTE: {
	    char    *dest = (char *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i] = CHAR_MIN;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i] = CHAR_MAX;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case SHORT: {
	    short   *dest = (short *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i] = SHRT_MIN;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i] = SHRT_MAX;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case LONG: {
	    long    *dest = (long *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < LONG_MIN)
		{
		    dest[i] = LONG_MIN;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > LONG_MAX)
		{
		    dest[i] = LONG_MAX;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > 0) dest[i] = src[i].real + 0.5;
		else dest[i] = - (0.5 - src[i].real);
	    }
	    break;}
	case FLOAT: {
	    float   *dest = (float *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < - FLT_MAX)
		{
		    dest[i] = - FLT_MAX;
		    CLIP_DBL_CX;
		}
		else if (src[i].real > FLT_MAX)
		{
		    dest[i] = FLT_MAX;
		    CLIP_DBL_CX;
		}
		else dest[i] = src[i].real;
	    }
	    break;}
	case DOUBLE: {
	    double  *dest = (double *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i].real;
	    }
	    break;}
	case BYTE_CPLX: {
	    byte_cplx	*dest = (byte_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < CHAR_MIN)
		{
		    dest[i].real = CHAR_MIN;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > CHAR_MAX)
		{
		    dest[i].real = CHAR_MAX;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < CHAR_MIN)
		{
		    dest[i].imag = CHAR_MIN;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > CHAR_MAX)
		{
		    dest[i].imag = CHAR_MAX;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case SHORT_CPLX: {
	    short_cplx	*dest = (short_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < SHRT_MIN)
		{
		    dest[i].real = SHRT_MIN;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > SHRT_MAX)
		{
		    dest[i].real = SHRT_MAX;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < SHRT_MIN)
		{
		    dest[i].imag = SHRT_MIN;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > SHRT_MAX)
		{
		    dest[i].imag = SHRT_MAX;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case LONG_CPLX: {
	    long_cplx	*dest = (long_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < LONG_MIN)
		{
		    dest[i].real = LONG_MIN;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > LONG_MAX)
		{
		    dest[i].real = LONG_MAX;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > 0) dest[i].real = src[i].real + 0.5;
		else dest[i].real = - (0.5 - src[i].real);

		if (src[i].imag < LONG_MIN)
		{
		    dest[i].imag = LONG_MIN;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > LONG_MAX)
		{
		    dest[i].imag = LONG_MAX;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > 0) dest[i].imag = src[i].imag + 0.5;
		else dest[i].imag = - (0.5 - src[i].imag);
	    }
	    break;}
	case FLOAT_CPLX: {
	    float_cplx	*dest = (float_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		if (src[i].real < - FLT_MAX)
		{
		    dest[i].real = - FLT_MAX;
		    CLIP_DBL_RE;
		}
		else if (src[i].real > FLT_MAX)
		{
		    dest[i].real = FLT_MAX;
		    CLIP_DBL_RE;
		}
		else dest[i].real = src[i].real;

		if (src[i].imag < - FLT_MAX)
		{
		    dest[i].imag = - FLT_MAX;
		    CLIP_DBL_IM;
		}
		else if (src[i].imag > FLT_MAX)
		{
		    dest[i].imag = FLT_MAX;
		    CLIP_DBL_IM;
		}
		else dest[i].imag = src[i].imag;
	    }
	    break;}
	case DOUBLE_CPLX: {
	    double_cplx	*dest = (double_cplx *) destination;

	    for (i = 0; i < num; i++)
	    {
		dest[i] = src[i];
	    }
	    break;}
	}
	break;}
    }

    return destination;
}


void
warn_on_clip(i, val)
    long	i;
    double_cplx	val;
{
    if (val.imag != 0)
	fprintf(stderr, "warn_on_clip: element %ld clipped from [%g, %g]\n",
		i, val.real, val.imag);
    else
	fprintf(stderr, "warn_on_clip: element %ld clipped from %g\n",
		i, val.real);
}


static int
numeric(type)
    int	    type;
{
    switch (type)
    {
    case BYTE:
    case SHORT:
    case LONG:
    case FLOAT:
    case DOUBLE:
    case BYTE_CPLX:
    case SHORT_CPLX:
    case LONG_CPLX:
    case FLOAT_CPLX:
    case DOUBLE_CPLX:
	return YES;
	break;
    default:
	return NO;
	break;
    }

    /*NOTREACHED*/
}

