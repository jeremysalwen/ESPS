/* 
 * This material contains proprietary software of Entropic Speech, Inc.   
 * Any reproduction, distribution, or publication without the the prior	   
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice			
 *
 *            "Copyright (c) 1987 Entropic Speech, Inc."; All Rights Reserved
 *				
 * Written by:  John P. Burg
 * Modified by: David Burton for ESPS
 * Module:	auto_pefrc.c
 *	
 * Generates autocorrelation values (r) and prediction error filter
 * coefficients (pef) from reflection coefficients and the residual
 * mean square value
 * 
 *	ORDER + 1 - number of input autocorrelations
 *	C     - output array of size ORDER+1. C[0] contains the residual;
 *		C[1] through C[ORER] contain the reflection coefficients.
 *	R     - input array of size ORDER+1. R[0] contains the energy of the
 *		the data (sum of squares); R[1] through R[ORDR] contains the
 *		rest of the autocorrelations
 *	PEF   - output array of size ORDER+1. PEF[0] contains the residual;
 *		PEF[1] through PEF[ORDER] contain prediction error filter
 *		coefficients. 
 *
 * NOTE: R[1] = R[0] * C[1]
 *
 * If the conversion has gone normally, that is, each reflection 
 * coefficient is less than one in magnitude and the mean square error is 
 * positive, then a zero is returned.  However, if a reflection        
 * coefficient has a magnitude of unity or greater, then the    
 * index of that coefficient is returned.  If the minimum mean  
 * square error is negative, minus one is returned.                  
 */

#include <stdio.h>
#include <esps/esps.h>

#ifndef lint
static char *sccs_id = "@(#)auto_pefrc.c	1.4 11/19/87 ESI";
#endif

	  auto_pefrc(order, r, pef, c)
	  int    order;
	  double  r[], pef[], c[];
	  {
	    double delta, temp;
	    int    i, m;

#ifdef UE
	    spsassert(r != NULL, "auto_pefrc: r is NULL");
	    spsassert(pef != NULL, "auto_pefrc: pef is NULL");
	    spsassert(c != NULL, "auto_pefrc: c is NULL");
#endif

	    for(i=0; i < order; i++)	/*zero pef[] array*/
		pef[i] = 0.;

            if (r[0] <= 0.0)      /* check that r[0] is positive       */
              return(-1);

	    c[0] = r[0];
	    for (m = 1; m <= order; m++)
	    {
	      delta = r[m];
              for (i = 1; i < m; i++)
		delta += pef[i]*r[m-i];

              c[m] = pef[m] = -delta/c[0];
              c[0] *= (1.0 - c[m]*c[m]);

	      if (c[0] <= 0.0)              /* check for negative      */
	        return(m);                  /* mean square error.      */

              pef[0] = c[0];
              for (i = 1; i <= m/2; i++)
	      {
	        temp = pef[i] + c[m]*pef[m-i];
                pef[m-i] += c[m]*pef[i];
	        pef[i] = temp;
              }
            }
	    /*
	     * Flip signs to be compatible with ESPS conventions
	    */

	    for(i = 1; i <= order; i++) {
		c[i] = -c[i];
		pef[i] = -pef[i];
	    }

	    return(0);
          }
