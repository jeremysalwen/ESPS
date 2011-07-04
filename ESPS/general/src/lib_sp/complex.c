/*----------------------------------------------------------------------------
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	

			COMPLEX

subroutines for operations in the complex domain

    cadd : returns the sum of the 2 arguments
    csub : returns the difference of the 2 arguments
    cmult : returns the product of the 2 arguments
    cdiv : returns the complex division of the first argument by the second
    conj : returns the complex conjugate of its argument
    realmult : returns the product of the first argument by the second, assumed
		to be a real number
    cmultacc : returns the product of the first 2 arguments accumulated with 
		the third
    modul : returns the module of its argument
    csqrt : returns the sqrt of its argument
    realadd : returns the sum of a real and a complex

Written by: Bernard G. Fraenkel; csqrt and ESPS modification by D. Burton
----------------------------------------------------------------------------*/



#ifndef lint
	static char *sccs_id = "@(#)complex.c	1.13 12/19/95 ERL";
#endif

#include <math.h>
#include <stdio.h>
#include <esps/esps.h>
#include <esps/complex.h>


/* ------------------------------------------------------------------------- */

/* NOTE: the return values are declared 'static' in order to improve the
   efficiency of the code; it is not necessary though. */


COMPLEX cadd (x, y)
COMPLEX x, y;
{
    static COMPLEX r;
    r.real = x.real + y.real;
    r.imag = x.imag + y.imag;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX csub (x, y)
COMPLEX x, y;
{
    static COMPLEX r;
    r.real = x.real - y.real;
    r.imag = x.imag - y.imag;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX conj (x)
COMPLEX x;
{
    static COMPLEX r;
    r.real = x.real;
    r.imag = -x.imag;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX cmult (x, y)
COMPLEX x, y;
{
    static COMPLEX r;
    r.real = x.real * y.real - x.imag * y.imag;
    r.imag = x.real * y.imag + x.imag * y.real;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX realmult (x, y)
COMPLEX x;
double  y;
{
    static COMPLEX r;
    r.real = x.real * y;
    r.imag = x.imag * y;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX realadd (x, y)
COMPLEX x;
double y;
{
  static COMPLEX r;
  r.real = x.real + y;
  r.imag = x.imag;
  return (r);
}

/* ------------------------------------------------------------------------- */

double  modul (x)
        COMPLEX x;
{
    static double  temp;
    temp = x.real * x.real + x.imag * x.imag;
    temp = sqrt (temp);
    return (temp);
}

/* ------------------------------------------------------------------------- */

COMPLEX cdiv (x, y)
COMPLEX x, y;
{
    static COMPLEX r;
    double  mody, modul ();
    mody = modul (y);		/* |y| */
    if (mody <= 0.0){
	(void) fprintf (stderr, 
			"cdiv: can't do a complex division by 0.0\n");
	exit(-1);
    }
    mody = 1.0 / mody;
    mody *= mody;		/* mody = 1.0/(|y|**2) */
    r.real = (x.real * y.real + x.imag * y.imag) * mody;
    r.imag = (x.imag * y.real - x.real * y.imag) * mody;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX cmultacc (x, y, r)
COMPLEX x, y, r;

/* multiply/accumulate :   r <- x*y+r   */

{
    r.real += x.real * y.real - x.imag * y.imag;
    r.imag += x.real * y.imag + x.imag * y.real;
    return (r);
}

/* ------------------------------------------------------------------------- */

COMPLEX csqrt (x)
COMPLEX x;

/* returns positive real part square root of x */

{
    double angle, magnitude;
    COMPLEX y;

/* following avoids atan2 error message when finding atan2(0,0) */
    if (x.imag == 0.0 && x.real == 0.0) 
    	angle = 0.0;
    else
    	angle = atan2(x.imag, x.real);
    magnitude = sqrt(modul(x));

    y.real = magnitude*cos(angle/2);
    y.imag = magnitude*sin(angle/2);

    return(y);
}

/*--------------------------------------------------------------------------*/

