/* value.c:
 *
 * routines for converting byte values to long values.  these are pretty
 * portable although they are not necessarily the fastest things in the
 * world.
 *
 * jim frost 10.02.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)value.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "image.h"

unsigned long memToVal(p, len)
     byte         *p;
     unsigned int  len;
{ unsigned int  a;
  unsigned long i;

  i= 0;
  for (a= 0; a < len; a++)
    i= (i << 8) + *(p++);
  return(i);
}

void valToMem(val, p, len)
     unsigned long  val;
     byte          *p;
     unsigned int   len;
{ int a;

  for (a= len - 1; a >= 0; a--) {
    *(p + a)= val & 0xff;
    val >>= 8;
  }
}

unsigned long memToValLSB(p, len)
     byte         *p;
     unsigned int  len;
{ int val, a;

  val= 0;
  for (a= len - 1; a >= 0; a--)
    val= (val << 8) + *(p + a);
  return(val);
}

/* this is provided for orthagonality
 */

void valToMemLSB(val, p, len)
     byte          *p;
     unsigned long  val;
     unsigned int   len;
{
  while (len--) {
    *(p++)= val & 0xff;
    val >>= 8;
  }
}
