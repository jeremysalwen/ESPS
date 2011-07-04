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
 * Brief description:
 *
 */

static char *sccs_id = "@(#)malloc.c	1.2	12/1/93	ERL";

#include <stdio.h>
#ifdef DEC_ALPHA
char *malloc();
char *calloc();
#endif

#ifndef IBM_RS6000
char *ecalloc(nelem, elsize)
#else
void *ecalloc(nelem, elsize)
#endif
unsigned nelem, elsize;
{


#ifdef DEBUG
fprintf(stderr,"calloc: nelem: %d, elsize: %d\n",nelem,elsize);
#endif

if (elsize && nelem)
  return (calloc(nelem, elsize));
else
  return 1;
}


#ifndef IBM_RS6000
char *emalloc(size)
#else
void *emalloc(size)
#endif
unsigned size;
{

#ifdef DEBUG
fprintf(stderr,"malloc: size: %d\n",size);
#endif

if (size)
  return (malloc(size));
else
  return 1;
}
