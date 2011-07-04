/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: returns the size of an ESPS type
 *
 */

static char *sccs_id = "@(#)typesiz.c	1.1	10/4/91	ERL";

#include <stdio.h>
#include <esps/esps.h>

int
typesiz (type)
int     type;
{
    switch (type) {
	case DOUBLE: 
	    return sizeof (double);
	case FLOAT: 
	    return sizeof (float);
	case LONG: 
	    return sizeof (long);
	case SHORT: 
	    return sizeof (short);
	case CODED: 
	    return sizeof (short);
	case CHAR: 
	case BYTE: 
	case EFILE:
	case AFILE:
	    return sizeof (char);
	case DOUBLE_CPLX:
            return sizeof(double_cplx);
    	case FLOAT_CPLX:
            return sizeof(float_cplx);
    	case LONG_CPLX:
            return sizeof(long_cplx);
    	case SHORT_CPLX:
            return sizeof(short_cplx);
    	case BYTE_CPLX:
            return sizeof(byte_cplx);
    }
    spsassert(NO, "typesiz: unrecognized type code");

    return 0;	/* not reached, just to keep lint happy */
}
