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
 * Written by:  David Burton
 * Checked by:
 * Revised by:
 *
 * Compute the signal-to-noise ratio distortion between a reference signal
 * and the error between the refernce signal and another signal. Typically
 * the other signal is a processed version of the original signal.
 *
 */

#ifndef lint
static char *sccs_id = "@(#)get_snr.c	1.1 9/4/98	ERL";
#endif

/*
 * System Includes
 */
#include <math.h>
#include <stdio.h>


float 
get_snr(ref, proc, size)
  float   *ref;      /* array containing reference speech data */
  float   *proc;     /* array containing processed speech data */
  int     size;      /* size of data arrays */
{
  float  result = 0;
  double numSum = 0, denSum = 0, diff = 0;
  int    cnt;

  /* 
   * Check inputs
   */
  if(ref == NULL) {
      (void)fprintf(stderr, "get_snr: Input reference data is NULL\n");
      exit(1);
   }
  if(proc == NULL) {
      (void)fprintf(stderr, "get_snr: Input processed data is NULL\n");
      exit(1);
  }
  if(size <=0) {
      (void) fprintf(stderr, "get_snr: Specified data array size <= 0\n");
      exit(1);
  }

  
  /*
   * Compute SNR
   */
  for(cnt = 0; cnt < size; cnt++) {
      numSum = numSum + ref[cnt] * ref[cnt];
      diff = ref[cnt] - proc[cnt];
      denSum = denSum + diff * diff;
  }

  if(numSum == 0.0 && denSum == 0.0) {
      result = 0.0;

  } else if (numSum == 0.0) {
      result = -385.0;

  } else if (denSum == 0.0) {
      result = 385.0;

  } else {
      result = 10*log10(numSum/denSum);
  }
		      
  return result;
}



/* Simple skeleton test program */
/* Remove this begin and ending comment symbol to test
main()
{

float in[2], out[2];
int i, size = 2;
float answer;

for(i=0; i< size; i++) {
    in[i] = i;
    out[i]= i+1;
}

answer = get_snr(in, out, size);

printf("answer = %f\n", answer);
}

*/

