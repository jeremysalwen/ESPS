/* fft.h */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.1415927
#endif

#define TRUE 1
#define FALSE 0

class FFT {
 public:
  // Constructor: Prepare for radix-2 FFT's of size (1<<pow2)
  FFT(int pow2);

  ~FFT();

  // Forward fft.  Real time-domain components in x, imaginary in y
  void fft(float *x, float *y);

  // Inverse fft.  Real frequency-domain components in x, imaginary in y
  void ifft(float *x, float *y);

  // Compute the dB-scaled log-magnitude spectrum from the real
  // spectal amplitude values in 'x', and imaginary values in 'y'.
  // Return the magnitude spectrum in z.  Compute 'n' components.
  int flog_mag(float *x, float *y, float *z, int n);

 private:
  // Create the trig tables appropriate for transforms of size (1<<pow2).
  int makefttable(int pow2);
  float *fsine, *fcosine; // The trig tables
  int fft_ftablesize;	/* size of trig tables (= (max fft size)/2) */
  int power2, kbase, fftSize; // Misc. values pre-computed for convenience
};

