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
|   Module:	    zero_fill.c						|
|   									|
|   Written by: Rodney Johnson						|
|   Checked by: 							|
|   									|
|   Fill a newly allocated or existing numeric array with zeros		|
|   of a given numeric type.						|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)zero_fill.c	1.2	12/8/89	ESI";
#endif

/*
 * Include files.
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

/*
 * ESPS functions
 */

int	typesiz();


/*
 * FUNCTION DEFINITION
 */

char *
zero_fill(num, type, buf)
    long    num;
    int	    type;
    char    *buf;
{
    long    i;


    if (!buf)
    {
	buf = malloc((unsigned) num * typesiz(type));
	spsassert(buf, "zero_fill: can't allocate space for array.");
    }

    switch(type)
    {
    case BYTE: {
	char	*ptr = (char *) buf;

	for (i = 0; i < num; i++) ptr[i] = 0;
	break;}
    case SHORT: {
	short	*ptr = (short *) buf;

	for (i = 0; i < num; i++) ptr[i] = 0;
	break;}
    case LONG: {
	long	*ptr = (long *) buf;

	for (i = 0; i < num; i++) ptr[i] = 0;
	break;}
    case FLOAT: {
	float	*ptr = (float *) buf;

	for (i = 0; i < num; i++) ptr[i] = 0.0;
	break;}
    case DOUBLE: {
	double	*ptr = (double *) buf;

	for (i = 0; i < num; i++) ptr[i] = 0.0;
	break;}
    case BYTE_CPLX: {
	byte_cplx   *ptr = (byte_cplx *) buf;

	for (i = 0; i < num; i++) ptr[i].real = ptr[i].imag = 0;
	break;}
    case SHORT_CPLX: {
	short_cplx   *ptr = (short_cplx *) buf;

	for (i = 0; i < num; i++) ptr[i].real = ptr[i].imag = 0;
	break;}
    case LONG_CPLX: {
	long_cplx   *ptr = (long_cplx *) buf;

	for (i = 0; i < num; i++) ptr[i].real = ptr[i].imag = 0;
	break;}
    case FLOAT_CPLX: {
	float_cplx   *ptr = (float_cplx *) buf;

	for (i = 0; i < num; i++) ptr[i].real = ptr[i].imag = 0.0;
	break;}
    case DOUBLE_CPLX: {
	double_cplx   *ptr = (double_cplx *) buf;

	for (i = 0; i < num; i++) ptr[i].real = ptr[i].imag = 0.0;
	break;}
    default:
	Fprintf(stderr, "zero_fill: type code %d not recognized.\n",
		type);
	exit(1);
	break;
    }

    return buf;
}
