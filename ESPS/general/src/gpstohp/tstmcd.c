/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)tstmcd.c	1.2	10/4/91	ESI/ERL";

/*----------------------------------------------------------------------+
|									|
|  tstmcd--generate test pattern for mcd				|
|									|
|  Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/

#include <stdio.h>

main()
{
    nullcomment();
    box(250, 250, 750, 750);
    box(5250, 250, 5750, 750);
    box(5250, 2750, 5750, 3250);
    box(250, 2750, 750, 3250);
    box(500, 500, 5500, 3000);
    text(1000, 1850, 850, "Mcd OK.");

    exit(0);
    /*NOTREACHED*/
}

/* The following are macros and functions for writing Masscomp Unix GPS
    commands. */

/* Macros to pack data into the three graphic primitive word types that
   occur in a GPS metafile.  Byte order is not necessarily correct for
   machines other than Masscomp.  (Documentation on the GPS format is
   not clear on that point.) */

#define PUT1(A,B)  {short sh_P = (A)<<12 | (B)&0x0fff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT2(A)	   {short sh_P = (A); \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT3(A,B)  {short sh_P = (A)<<8 | (B)&0x00ff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}

nullcomment()
{
    PUT1(15, 1)
}

box(x1, y1, x2, y2)
    int	    x1, y1, x2, y2;
{
    PUT1(0, 12)
    PUT2(x1) PUT2(y1)
    PUT2(x2) PUT2(y1)
    PUT2(x2) PUT2(y2)
    PUT2(x1) PUT2(y2)
    PUT2(x1) PUT2(y1)
    PUT3(20, 0)
}

text(u0, v0, s, t)
    int u0, v0;			/* GPS universe coords of first char.	*/
    int s;			/* Size,				*/
    char *t;			/* Text string.				*/
{
    int n = (2 + strlen(t))/2;	/* 2 chars per word; 1 or 2 nulls	*/
    PUT1(2, 5 + n)		/* Command code & length.		*/
    PUT2(u0) PUT2(v0)		/* Starting coords.			*/
    PUT3(20, 0)			/* Bundle number.			*/
    PUT3((s+2)/5, 0)		/* Size & rotation.			*/
				/* Text characters.			*/
    while (n-- > 1)
    {
	putchar(*t++);
	putchar(*t++);
    }
    {
	putchar(*t);		/* Last char if length is odd; null if	*/
				/*  even.				*/
	putchar('\0');
    }
}

