/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*/

 	

#ifndef lint
	static char *sccs_id = "@(#)getarspect.c	1.3 2/20/96 EPI";
#endif

#include <stdio.h>
#define dftsze 1024

void
get_arspect (c, order, res_power, samp_freq, y)
int     order, samp_freq;
float   c[], res_power, y[];
/*
To compute the lpc spectra, given lpc filter coefficients and residual power

Input:
	c 		--- "order" lpc filter coefficients
	res_power	--- residual power
	samp_freq	--- Sampling Frequency
Output:
	y		--- Power spectrum (512 values in the range 0 - samp_freq).
*/
{
    double  t, log10 (), sqrt ();
    int     i, logN = 10;
    float temp_real[dftsze], temp_imag[dftsze];

    for (c[0] = -1.0, i = 0; i < dftsze; i++)
	temp_real[i] = (i <= order) ? -c[i] : 0.0;

    for (i = 0; i < dftsze; i++)
	temp_imag[i] = 0.0;

    get_fft (temp_real, temp_imag, logN);

    for (i = 0; i <= dftsze / 2; i++)
    {
	t = temp_real[i] * temp_real[i]
	    + temp_imag[i] * temp_imag[i];
	t = (2.0 * res_power) / (t * samp_freq);
	if (t <.0000001)
	    t = 0.0000001;
	y[i] = 10.0 * log10 ((double) t);
    }
}
