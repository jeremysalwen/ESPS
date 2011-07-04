/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)spectrogram.h	1.2 9/26/95 ATT/ERL/ESI */
/* spectrogram.h 
*/

/* Here is a structure used to specify parameters for the spectrogram
   computation and plotting routines. */
typedef struct spectrogram {
  int nfft,			/* basic fft size used (power of 2) */
      window_type,		/* 0==rect., 1==Hamm, 2==cos**4, 3==Hann, */
      xdith,			/* dim. of dither filt. in x dir. */
      ydith,			/* dim. of dither filt. in y dir. */
      yinterp;			/* number of sams. to interp. in y dir. */
  Signal *sig;			/* Signal upon which to base computation */
  char *signame;		/* name of that signal */
  char *outname;		/* name for spectrogram to be created. */
  double sigfreq;		/* frequency of that signal */
  double q_step,		/* quantization step size for dithering */
      q_thresh,			/* quantization threshold for dithering */
      agc_ratio,		/* part of total rms used to normalize */
      start_time,		/* start of analysis in Signal */
      end_time,			/* end of analysis in Signal */
      window_size,		/* number of ms in input array */
      window_step,		/* ms to advance at each time step */
      preemp,			/* 1st-order preemphasis filter coeff. */
      var_ratio,		/* part of variance used to norm. ampl. */
      *dimp;			/* dither filter impulse response */
#ifdef FOR_SUNVIEW
  Pixrect	*bitmap;        /* optional dithered spectrogram for mono */
#else
#ifdef FOR_XVIEW
  Server_image	bitmap;
#else
  caddr_t	bitmap;
#endif
#endif
} Spectrogram;

#define SCOPE_VIEW	1
#define SCOPE_BUFFER	2

/* Definitions specific to DSP32 shared memory spectrograms */

#define INBUF_CHARS_32C 128000
/* INBUF_CHARS_32C is the total number of bytes available in the shared memory
   buffer. */

#define INBUF_SHORTS_32C 63996
/*INBUF_SHORTS_32C is the number of shorts available in shared memory minus the
  2 (int) locations used for buffer size and frame count. */

#define INBUF_INTS_32C 31998
/*INBUF_INTS_32C is the number of ints available in shared memory minus the
  2 (int) locations used for buffer size and frame count. */

#define INBUF_CHARS_32 2048
/* INBUF_CHARS_32 is 1/2 the total number of bytes available in the shared 
   memory buffer. */

#define INBUF_SHORTS_32 2046
/*INBUF_SHORTS_32 is the number of shorts available in shared memory minus the
  2 locations used for buffer size and frame count. */

