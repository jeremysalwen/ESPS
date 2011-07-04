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
 * Compute the critical band filter weight correponding to a Bark value.
 * 0 Bark corresponds to the filter peak. 
 *
 */


#ifndef lint
static char *sccs_id = "@(#)crit_band_filt_wt.c	1.1 9/4/98 ERL";
#endif


/*
 * Local defines
 */
#define LOG10_BY_10 0.230258509299404568402
                                /* log(10.0)/10.0 */

 
/*
 * System Includes
 */
#include <math.h>
 

/*
 * Compute the value of a critical-band weighting function at a point,
 * given the distance b of the point from the peak (in barks)
 */
double
crit_band_filt_wt(barkValue)
  double barkValue;             /* Input Bark value - 
				   specifies offset from filter peak */
{
    double  weight;             /* return value (dB) */
 
    barkValue -= 0.210;         /* adjust for peak at 0 bark;
                                 * not 0.215; see man page.
                                 */
                                /* the other magic numbers below come
                                 * come from Wang, Sekey, & Gersho [1] and
                                 * Sekey & Hanson [2]; see the man page
                                 * for full references.
                                 */
    weight = 7.0 - 7.5*barkValue - 17.5*sqrt(0.196 + barkValue*barkValue);
 
    return exp(LOG10_BY_10 * weight);  /* convert from dB */
}


/* Simple skeleton test program */
/* Remove the comment beginning and end to test
main()
{
#include <errno.h>

double bigNumber = 100000, bark, weight;

errno = 0;
weight = crit_band_filt_wt(bigNumber);
if (errno)
  perror("crit_band_filt_wt() error");
errno = 0;


bark = -6.465;
weight = crit_band_filt_wt(bark);
printf("-60 dB filter wt = %f\n", weight);
bark = 0.41;
weight = crit_band_filt_wt(bark);
printf("-3 dB filter wt = %f\n", weight);


}

*/
