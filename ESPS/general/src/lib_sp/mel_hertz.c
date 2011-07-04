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
 * Compute the mel equivalent of a frequency in Hz, and the frequency (Hertz)
 * equivalent of a mel value.
 *
 */

/* 
 * Large input mel values could result in a return value that is out of range
 * for the mel_to_hz conversion, but no reasonable sampling frequency
 * should produce this error. Users should validate input or check for errors.
 * 
 * The log(1+x) in hz_to_mel is valid for all positive input arguments.
 *
 * Negative input values for frequency or mel values are invalid.
 *
 */


#ifndef lint
static char *sccs_id = "@(#)mel_hertz.c	1.2 9/8/98 ERL";
#endif

 
/*
 * System Includes
 */
#include <math.h>
#include <stdio.h>

/* 
 * Local Defines 
 */
#define MEL_CONST   1127.01048033415743865
                                /* 1000.0/log(1700.0/700.0),
                                 * chosen to make hz_to_mel(1000.0)
                                 * equal to 1000.0.
                                 */

/*
 * Convert a mel-scale value melValue to the equivalent linear-scale
 * frequency (in hertz).
 */

double 
mel_to_hz(melValue)
  double   melValue;
{
  if (melValue < -1) {
      fprintf(stderr, "Invalid input value to mel_to_hz().",
	      " Input value (%f) must be >= -1\n", melValue);
      return -1;
  }
  return 700.0*(exp(melValue/MEL_CONST) - 1.0);
}


/*
 * Convert a linear-scale frequency freqValue (in hertz) to the equivalent
 * mel-scale value.
 */

double hz_to_mel(freqValue)
  double freqValue;
{
  if (freqValue < -1) {
      fprintf(stderr, "Invalid input value to hz_to_mel().",
	      " Input value (%f) must be >= -1\n", freqValue);
      return -1;
  }
  return MEL_CONST*log(1.0 + freqValue/700.0);
}



/* Simple skeleton test program */

/*  Remove comment begin and the matching one at the bottom to test
main()
{
#include <errno.h>
#include <values.h>

double bigNumber = MAXFLOAT, hertz, mel;


errno = 0;
hertz = mel_to_hz(bigNumber);
if (errno)
  perror("mel_to_hz error");
errno = 0;


mel = -2;
hertz = mel_to_hz(mel);
printf("mel = -2; mel_to_hz returns = %f\n", hertz);


hertz = 5000.0;
mel = hz_to_mel(hertz);
printf("mel equivalent of 5000 Hz. is %f mel\n", mel);


hertz = mel_to_hz(mel);
printf("mel value converted back to hertz is %f Hz.\n", hertz);
}

*/
