/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)feafilt.h	1.7 2/20/96 ERL
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:  Derek Lin for sdata and fdata
 *
 * Brief description:include file for feature file subtype FILT 
 *
 */

#ifndef feafilt_H
#define feafilt_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/*
 * structure definition for FEAFILT records 
 */

struct feafilt
{
  long              *num_size;
  long              *denom_size;
  long              *zero_dim;
  long              *pole_dim;
  double            *re_num_coeff;
  double            *im_num_coeff;
  double            *re_denom_coeff;
  double            *im_denom_coeff;
  double_cplx       *zeros;
  double_cplx       *poles;
  struct  fea_data  *fea_rec;	/* pointer to corresponding FEA record */
};

/*
 * The items in the filter structure have the following shapes (dimension
 * information refers to generic header items):
 *
 *	Item		Shape	Dimensions
 *
 *      num_size        scalar
 *      denom_size      scalar
 *      zero_dim        scalar
 *      pole_dim        scalar
 *      re_num_coeff    vector  max_num
 *      im_num_coeff    vector  max_num
 *      re_denom_coeff  vector  max_denom
 *      im_denom_coeff  vector  max_denom
 *      zeros           vector  max_num 
 *      poles           vector  max_denom 
 *
 * The pointer fea_rec does not correspond to actual data that is
 * part of the feastat record.  
 */

/* 
 * structure used by allo_feafilt_rec()
 */

typedef struct _filtdesparams{
    short       filter_complex;
    short       define_pz;
    short       type;
    short       method;
    short       func_spec;
    long        g_size;
    long        nbits;
    float       *gains;
    long        nbands;
    float       *bandedges;
    long        npoints;
    float       *points;
    float       *wts;
  } filtdesparams;

/*
 * declarations for support functions
 */

int
init_feafilt_hd ARGS((struct header *hd, long int max_num,
		      long int max_denom, filtdesparams *fdp));

struct feafilt *
allo_feafilt_rec ARGS((struct header *hd));

int
get_feafilt_rec ARGS((struct feafilt *feafilt_rec,
		      struct header *hd, FILE *file));

void
put_feafilt_rec ARGS((struct feafilt *feafilt_rec,
		      struct header *hd, FILE *file));

struct zfunc
feafilt_to_zfunc ARGS((struct feafilt *feafiltrec));


/*
 *  The follwoing are definitions used for block_filter2();
 */


#define FIR_FILTERING 1
#define IIR_FILTERING 2
#define ERR 1

/*
 *  Signal data structure
 */

struct sdata{
  struct header *hd;
  struct feasd *rec;
  void *data, **ptrs;     /* data/ptrs point to single/multiple channel data*/
  short data_type;

  int no_channel;
  int *channel;
  double sample_rate;
  char *name;
};

/*
 *  filter data structure
 */

struct fdata{
  int filtertype;
  void *arch;              /* an architecture pointer, points to
			      iirfilt, firfilt or future filter
			      implemenation */
};


/* see Discrete Time Signal Processing, Oppenheim & Schafer (1989), page 304.
   stages: number of cascaded 2nd order IIR.  Each is direct form II
   implementation.

   a0, a1, b0, b1: arrays of size "stages".  a0[i], a1[i], b0[i] and b1[i]
      completely describes the ith stage 2nd order IIR, ie.

      1 + b0 * z^-1 + b1 * z^-2
      -------------------------
      1 - a0 * z^-1 - a1 * z^-2

   w0, w1, w2, s: arrays of size "stages" (except s, s array has size
      stages+1).  w0[i], w1[i], w2[i] completely describes 
      the internal states of 2nd order IIR, s[i+1] describes the output
      of ith IIR.  s[i] is the input of ith IIR. ie.
   
   gain_factor: overall gain at the end of the cascade.

     s[i]           w0[i]      s[i+1]          *gain_factor
    -->-- +------>-----+----->-----+------> ......-----------> output
          |            |           |
          ^            v           ^
	  |            |           |
          |   a0[i]  w1[i]  b0[i]  |
	  +----<-------.----->-----+
          |            |           |
          ^            v           ^
          |            |           |
          |   a1[i]  w2[i]  b1[i]  |
	  -----<-------.----->------

    w0c, w1c, w2c, sc: complex counterpart of w0, w1, w2, s, applicable
        only when input signal is complex.

    xstate, ystate: applicable only when IIR filter must be implemented
        directly from its transfer function (ie when pole/zeros fields 
	in *arma is not available, but num/denomenator coeffiecients). 

	y[n] =  sum   ( b  * x[n-k]) -  sum  ( a  * y[n-k])
	       k=0..M    k             k=1..N   k

	xstate[0] = x[n-1]...xstate[M-1] = x[n-M];   size M
	ystate[0] = y[n-1]...ystate[N-1] = y[n-N];   size N

    xstatec, ystatec: complex counterpart of xstate, ystate, applicable
        only when input signal is complex.
    
    cplx_flag: YES means complex coefficients, otherwise NO	

*/
struct iirfilt{
  struct feafilt *arma;
  int cplx_flag;
  int stages;
  double *a0, *a1, *b0, *b1;
  double *w0, *w1, *w2, *s;
  double_cplx *w0c, *w1c, *w2c, *sc;
  double gain_factor;
  
  double *xstate, *ystate;
  double_cplx *xstatec, *ystatec;
};


/* A direct implementation for FIR.  
   state: array for previous s state, where s is the number or numerator
          coefficients minus 1.  state[0] is the most recent state.
   statec: complex counterpart of state, applicable only when input
          signal is complex
   cplx_flag: YES means complex coefficients, otherwise NO
*/

struct firfilt{
  struct feafilt *arma;
  int cplx_flag;
  double *state;
  double_cplx *statec;
};

int
block_filter2 ARGS((long int nsamp, struct sdata *in,
		    struct sdata *out, struct fdata **filtrec));

struct fdata *
init_fdata ARGS((int type, struct feafilt *filtrec,
		 struct header *fh, int cplx_sig, int cplx_fil));


#ifdef __cplusplus
}
#endif

#endif /* feafilt_H */
