/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1989  Entropic Speech, Inc.; 
 *                   All rights reserved"
 *
 * Program:	cover_type()	
 *
 * Written by:  John Shore
 *
 * This routine finds and returns the type that "covers" the two 
 * input types, i.e., can contain all the values.  
 */

#ifndef lint
    static char *sccs_id = "@(#)cover_type.c	1.1	10/17/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

short
  cover_type(t1, t2)
short t1, t2; /*two ESPS data types defined in esps.h*/
{
  /* first check arguments */
  
  switch(t1) {
  case DOUBLE_CPLX:
  case FLOAT_CPLX:
  case LONG_CPLX:
  case SHORT_CPLX:
  case BYTE_CPLX:
  case DOUBLE:
  case FLOAT:
  case LONG:
  case SHORT:
  case BYTE:
    break;
  default:
    spsassert(0, "cover_type: argument 1 is not a valid ESPS data type");
  }
  
  switch(t1) {
  case DOUBLE_CPLX:
  case FLOAT_CPLX:
  case LONG_CPLX:
  case SHORT_CPLX:
  case BYTE_CPLX:
  case DOUBLE:
  case FLOAT:
  case LONG:
  case SHORT:
  case BYTE:
    break;
  default:
    spsassert(0, "cover_type: argument 2 is not a valid ESPS data type");
  }
  
  /* return immediately for trivial case of both equal*/
  
  if (t1 == t2) 
    return t1;
  
  /* handle all other cases (some redundant) */
  
  switch(t1) {
  case DOUBLE_CPLX:
    return DOUBLE_CPLX;
  case FLOAT_CPLX:
    if (t2 == DOUBLE_CPLX || t2 == DOUBLE)
      return DOUBLE_CPLX;
    else
      return FLOAT_CPLX;
  case LONG_CPLX:
    switch(t2) {
    case DOUBLE_CPLX:
    case DOUBLE:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
    case FLOAT:
      return FLOAT_CPLX;
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
    case LONG:
    case SHORT:
    case BYTE:
      return LONG_CPLX;
    }
  case SHORT_CPLX:
    switch(t2) {
    case DOUBLE_CPLX:
    case DOUBLE:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
    case FLOAT:
      return FLOAT_CPLX;
    case LONG_CPLX:
    case LONG:
      return LONG_CPLX;
    case SHORT_CPLX:
    case BYTE_CPLX:
    case SHORT:
    case BYTE:
      return SHORT_CPLX;
    }
  case BYTE_CPLX:
    switch(t2) {
    case DOUBLE_CPLX:
    case DOUBLE:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
    case FLOAT:
      return FLOAT_CPLX;
    case LONG_CPLX:
    case LONG:
      return LONG_CPLX;
    case SHORT_CPLX:
    case SHORT:
      return SHORT_CPLX;
    case BYTE_CPLX:
    case BYTE:
      return BYTE_CPLX;
    }
  case DOUBLE:
    switch(t2) {
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
      return DOUBLE_CPLX;
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
      return DOUBLE;
    }
  case FLOAT:
    switch(t2) {
    case DOUBLE_CPLX:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
      return FLOAT_CPLX;
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
      return FLOAT_CPLX;
    case DOUBLE:
      return DOUBLE;
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
      return FLOAT;
    }
  case LONG:
    switch(t2) {
    case DOUBLE_CPLX:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
      return FLOAT_CPLX;
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
      return LONG_CPLX;
    case DOUBLE:
      return DOUBLE;
    case FLOAT:
      return FLOAT;
    case LONG:
    case SHORT:
    case BYTE:
      return LONG;
    }
  case SHORT:
    switch(t2) {
    case DOUBLE_CPLX:
      return DOUBLE_CPLX;
    case FLOAT_CPLX:
      return FLOAT_CPLX;
    case LONG_CPLX:
      return LONG_CPLX;
    case SHORT_CPLX:
    case BYTE_CPLX:
      return SHORT_CPLX;
    case DOUBLE:
      return DOUBLE;
    case FLOAT:
      return FLOAT;
    case LONG:
      return LONG;
    case SHORT:
    case BYTE:
      return SHORT;
    }
  case BYTE:
    return t2;
  }
}
  
  
