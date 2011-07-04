/* 
 * This material contains proprietary software of Entropic Speech, Inc.   
 * Any reproduction, distribution, or publication without the the prior	   
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice			
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc."; All Rights Reserved
 *				
 * Written by:  John P. Burg
 * Modified by: David Burton for ESPS
 * Module:	rc_autopef.c
 *	
 * Generates autocorrelation values (r) and prediction error filter
 * coefficients (pef) from reflection coefficients (c) and the residual
 * mean square value
 * 
 *	ORDER - number of input reflection coefficients
 *	C     - input array of size ORDER+1. C[0] contains the residual;
 *		C[1] through C[ORER] contain the reflection coefficients.
 *	R     - output array of size ORDER+1. R[0] returns the energy of the
 *		the data (sum of squares); R[1] through R[ORDR] returns the
 *		rest of the autocorrelations
 *	PEF   - output array of size ORDER+1. PEF[0] contains the residual;
 *		PEF[1] through PEF[ORDER] contain prediction error filter
 *		coefficients. 
 *
 * NOTE: R[1] = R[0] * C[1] 
 *
 * If the input residual is less <= 0, then rc_autopef exits
 * with a return value = -1. If one of the reflection coefficients >= 1, 
 * rc_autopef exits with a return value = to the index of the invalid
 * RC index. Otherwise rc_autopef returns 0.
 */

#ifndef lint
static char *sccs_id = "@(#)rc_autopef.c	1.2 11/19/87 ESI";
#endif


	  rc_autopef(order, c, r, pef)
	  int    order;
	  double c[], r[], pef[];
	  {
		    
	    int    m, i;
	    double temp;

	    if (c[0] <= 0.0)             /* check that the mean square  */
	      return(-1);                 /* error is postive.           */

	    /*
	     * change sign of reflection coefficients for compatibility
	     * with ESPS conventions
	    */
		for(i=1; i <= order; i++) {
		    c[i] = -c[i];
		}

	    
            r[0] = pef[0] = c[0];	/* assign residual error */

          /* Calculate zero lag value of the autocorrelation.           */

            for (m = 1; m <= order; m++)
	    {
	      temp = 1.0 - c[m]*c[m];
	      if (temp <= 0.0)
		return(m);  /*return index of bad reflection coeff.*/
	      r[0] /= temp;	
            }

          /* Calculate the prediction error filter and autocorrelation */

	    for (m = 1; m <= order; m++)
	    {

          /* Calculate the mth order pef  */

	      pef[m] = c[m];
	      for (i = 1; i <= m/2; i++)
	      {
		temp = pef[i] + c[m]*pef[m-i];
		pef[m-i] += c[m]*pef[i];
		pef[i] = temp;
              }

          /* Calculate the mth lag value of the autocorrelation  */

	      r[m] = -r[0]*pef[m];
	      for (i = 1; i < m; i++)
	        r[m] -= r[i]*pef[m-i];	
            }
	    /*
	     * Change sign of PEFs and RCs for ESPS compatability
	    */
	    for(i = 1; i <= order; i++){
		pef[i] = -pef[i];
		c[i] = -c[i];
	    }
	    return(0);
          }
