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
 * Compute the sone equivalent of a loudness level in phons
 *
 */


#ifndef lint
static char *sccs_id = "@(#)phon_to_sone.c	1.1 9/4/98 ERL";
#endif


/*
 * Local defines
 */
#define LOG2_OVER_10  (0.0693147180559945309417)    /* log(2)/10.0 */

 
/*
 * System Includes
 */
#include <math.h>
 

/*
 * Convert phon level in dBs to sone level
 */
double
phon_to_sone(phonLevel)
  double phonLevel;             /* Input phon value - in dBs */

{
    return  exp((phonLevel- 40) * LOG2_OVER_10);
}


/* Simple skeleton test program */
/* Remove the comment beginning and end to test
main()
{
#include <errno.h>

double bigNumber = 100000, phone, sone;


errno = 0;
sone = phon_to_sone(bigNumber);
if (errno)
  perror("phon_to_sone() error");
errno = 0;


phone = 100.;
sone = phon_to_sone(phone);
printf("sone level corresponding to phon = 100 is %f\n", sone);
}

*/

