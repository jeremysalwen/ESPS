/* ftFilter.h */
/* In-Line FIR filter class */

#include "fft.h"

// The input size is set to FD_FBUF_SIZE and the output size is twice
// that.  Does NOT have to be a power of two.
#define FD_FBUF_SIZE    8192

// This sets the limit on the maximum value of the denominator in the
// ratio that approximates the sample rate ratios when rate conversion
// is done.  FdFilter is really not appropriate if the ration needs to be
// greater than about 5, because time-domain approaches are then
// faster.  However, perverse or adventuresome souls might want to try
// increasing this if they have extra computer cydcles to waste.
#define FD_MAX_DECIMATE 5

class FdFilter {
 public:
  // Constructor when doing arbitrary FIR filtering using the response in file 'specShape'
  FdFilter(float inFreq, char *specShape);
  // Constructor when doing arbitrary FIR filtering using the response in array 'specArray'
  FdFilter(float inFreq, float *specArray, int nMagnitudes);
  // Constructor when doing highpass or lowpass filtering or when doing rate conversion
  FdFilter(float inFreq, float cFreq, int doHp, float fDur, int convertF);

  ~FdFilter();

  // Read and filter/convert all input from inStream and write to
  // outStream until inStream is exhausted.  Return 1 for success 0
  // for failure.
  int filterStream(FILE *fIn, FILE *fOut);

  // Process nIn samples from 'input' array into 'output' array.  Set
  // 'first' non-zero if this is the first call for a new signal.  Set
  // 'last' non-zero if the end of the signal is in 'input'.  The
  // number of samples transferred to 'output' is the return value.
  // 'maxOut' is the size of caller's output array.
  int filterArray(short *input, int nIn, int first, int last, short *output, int maxOut);

  // Return number of samples left after a call to filterArray() in the
  // case where 'maxOut' is smaller than the number of output samples
  // produced.
  int getArrayOutputLeftover();

  // Use after a call to filterArray() to get number of 'input'
  // samples actually used (in cases where the caller's output array
  // size has limited the number used, or when the caller's input size
  // is larger than getMaxInputSize()).
  int getArraySamplesUsed();

  // Return the largest batch of input samples that can be processed
  // with a single call to filterBuffer()
  int getMaxInputSize();

  // When sample-rate conversion is being done, return the actual
  // output frequency, which may differ from that requested.
  float getActualOutputFreq();

 private:
  // The main initialization function.  Called by all constructors.
  void FdFilterInitialize(float inFreq, float cFreq, int doHp, float fDur,
		     int convertF, char *specShape, float *specArray, int nMagnitudes);

  // Use the half filter in fc to create the full filter in fCoeff
  void mirror_filter(float *fc, int invert);

  // Window approach to creating a high- or low-pass FIR kernel
  void lc_lin_fir(float fc, int *nf, float *coef);

  // Find the closest integer ratio to the float 'a'
  void ratprx(float a, int *k, int *l, int qlim);

  // Complex inner product of length 'n'; result goes to (r3,i3)
  void cmul(int n, float *r1, float *i1, float *r2, float *i2, float *r3, float *i3);

  // Assuming nInput samples are in inBuffer, process the signal into outBuffer.
  // Returns number transferred to outBuffer in 'nOutput'.
  void filterBuffer(int nInput, int *nOutput);


  // DATA
  short	outBuffer[FD_FBUF_SIZE*2], inBuffer[FD_FBUF_SIZE]; // I/O buffers
  float *fCoeff; // becomes the filter coefficient array
  int nCoeff; // The number of filter coefficients
  int nCoeffBy2; // Precompute for convenience/speed
  int left_over, firstout, to_skip; // bookkeeping for the OLA filtering operations
  int arrayLeftover, arrayIndex, arraySamplesUsed; // Bookkeeping for buffered processing.
  int state; // State of processing WRT input and completion
  float trueOutputFreq; // For sample rate conversion, this can differ from requested freq.
  short *leftovers;
  float *outputDelayed;
  float *x, *y, *xf, *yf; // FFT processing arrays
  int nFFT; // Number of points in the FFTs
  int nFFTBy2; // Precompute for convenience/speed
  int insert, decimate; // for sample rate conversion, these are the interpolation/decimations
  int maxInput;  // maximum allowed input with each call to filterBuffer
  int filterMode; // arbitrary, lowpass, highpass for -1, 0, 1
  FFT *ft;
};

