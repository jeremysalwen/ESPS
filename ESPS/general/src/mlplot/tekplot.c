/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc.  must bear the
 * notice
 *
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 * tekplot.c - Tektronix plotting routines for mlplot(1-ESPS) and
 *	       genplot(1-ESPS) 
 *
 * Authors:  Rodney Johnson & Ajaipal S. Virdy, Entropic Speech, Inc.
 */
#ifndef lint
    static char *sccs_id = "@(#)tekplot.c	3.2	10/16/87	ESI";
#endif

#include <stdio.h>

/*
 * The following DEFINE'S are for writing to a Tektronix 4010 device.
 * See Appendix A of MASSCOMP's
 * Data Presentation Application Programming Manual.
 */

#define ESC '\033'
#define FF '\014'
#define GS '\035'
#define US '\037'
#define HI(A) (040 | ((07600 & (A)) >> 7))
#define LOY(A) (0140 | ((0174 & (A)) >> 2))
#define LOX(A) (0100 | ((0174 & (A)) >> 2))
#define EXTRA(A,B) (0140 | ((03 & (B)) << 2) | (03 & (A)))


int
tek_plotline(n, u, v)
    long    n;
    long    u[];
    long    v[];
{
    long    i;
    char    hiy, loy, hix, lox, extra;
    char    newhiy, newloy, newhix, newextra;

    if (n > 0)
    {
	putchar(GS);
	putchar(hiy = HI(*v));
	putchar(extra = EXTRA(*u, *v));
	putchar(loy = LOY(*v));
	putchar(hix = HI(*u));
	putchar(lox = LOX(*u));

	if (n == 1) putchar(lox);

	for (i=2, u++, v++; i<=n; i++, u++, v++)
	{	
	    newhiy = HI(*v);
	    if (newhiy != hiy)
		putchar(hiy = newhiy);

	    newextra = EXTRA(*u, *v);
	    newloy = LOY(*v);
	    newhix = HI(*u);

	    if (newextra != extra ||
		newloy != loy ||
		newhix != hix)
	    {
		if (newextra != extra)
		    putchar(extra = newextra);

		putchar(loy = newloy);

		if (newhix != hix)
		    putchar(hix = newhix);
	    }

	    putchar(LOX(*u));
	}

	putchar(US);
    }

}   /* end tek_plotline() */


void
tek_plotpoints(n, u, v)
    long    n;
    long    u[];
    long    v[];
{
    long    i;
    char    hiy, loy, hix, lox, extra;
    char    newhiy, newloy, newhix, newextra;

    if (n > 0)
    {
	putchar(GS);
	putchar(hiy = HI(*v));
	putchar(extra = EXTRA(*u, *v));
	putchar(loy = LOY(*v));
	putchar(hix = HI(*u));
	putchar(lox = LOX(*u));
	putchar(lox);

	for (i=2, u++, v++; i<=n; i++, u++, v++)
	{	
	    putchar(GS);
	    newhiy = HI(*v);

	    if (newhiy != hiy)
		putchar(hiy = newhiy);

	    newextra = EXTRA(*u, *v);
	    newloy = LOY(*v);
	    newhix = HI(*u);

	    if (newextra != extra ||
		newloy != loy ||
		newhix != hix)
	    {
		if (newextra != extra)
		    putchar(extra = newextra);

		putchar(loy = newloy);

		if (newhix != hix)
		    putchar(hix = newhix);
	    }

	    putchar(lox = LOX(*u));
	    putchar(lox);
	}

	putchar(US);
    }

}   /* end tek_plotpoints() */


init_tek_plot()	/* initialize the Imagen Laser Printer */
{
    (void) printf("@document(%s, %s, %s, %s)",
		    "imagespace (0 3000 0 2400)",
		    "imagesize (10.0 8.0)",
		    "window (10.5 8.0)",
		    "at (0.25 10.75 cc)"
		 );
}


tek_termpage()
{
    putchar(ESC);
    putchar(FF);
}
