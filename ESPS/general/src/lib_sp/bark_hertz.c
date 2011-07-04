/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1998 Entropic, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson/David Burton
 * Checked by:
 * Revised by: converted to library function by david burton
 *
 * Compute the Bark equivalent of a frequency in Hz, and the frequency (Hertz)
 * equivalent of a Bark value.
 *
 */


#ifndef lint
static char *sccs_id = "@(#)bark_hertz.c	1.1 9/4/98 ERL";
#endif
 
/*
 * System Includes
 */
#include <math.h>
 
/* 
 * Large input Bark values could result in a return value that is out of range
 * for the bark_to_hz conversion, but no reasonable sampling frequency
 * should produce this error. Users should validate input check for errors.
 * 
 * asinh() in hz_to_bark is valid for all input arguments.
 */


/*
 * Convert a Bark-scale value b to the equivalent linear-scale
 * frequency (in hertz).
 */

double 
bark_to_hz(barkValue)
  double   barkValue;
{
  return 600.0*sinh(barkValue/6.0);
}


/*
 * Convert a linear-scale frequency f (in hertz) to the equivalent
 * Bark-scale value.
 */

double hz_to_bark(freqValue)
  double freqValue;
{
  return 6.0*asinh(freqValue/600.0);
}



/* Simple skeleton test program */

/* Remove comment begginning and end to test
main()
{
#include <errno.h>
#include <values.h>

double bigNumber = MAXFLOAT, hertz, bark;

errno = 0;
hertz = bark_to_hz(bigNumber);
if (errno)
  perror("bark_to_hz error");
errno = 0;


hertz = 5000.0;
bark = hz_to_bark(hertz);
printf("bark equivalent of 5000 Hz is %f Bark\n", bark);


hertz = bark_to_hz(bark);
printf("bark converted back to hertz is %f Hz.\n", hertz);
}

*/
