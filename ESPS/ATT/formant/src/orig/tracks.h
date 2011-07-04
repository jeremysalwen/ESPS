/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)tracks.h	1.3 9/21/94 ATT/ERL/ESI
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

 /* this is an older version of the waves tracks.h needed for this version 
    of formant
 */



#define DEB_PAUSE	8	/* debug levels */
#define DEB_LPC_PARS	4
#define DEB_PARAMS	2
#define DEB_ENTRY	1
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define PI 3.1415927
#define MAXFORMANTS 7

/* structure definitions for the pitch tracker */
#define CROSS  struct cross_rec
struct cross_rec { /* for storing the crosscorrelation information */
	double	rms;	/* rms energy in the reference window */
	double	k1;	/* 1st-order autoregressive flattening constant. */
	double	maxval;	/* max in the crosscorr. fun. q15 */
	short	maxloc; /* lag # at which max occured	*/
	short	nlags;	/* the number of correlation lags computed */
	short	firstlag; /* the first non-zero lag computed */
	short	*correl; /* the normalized corsscor. fun. q15 */
};

#define DPREC struct dp_rec
struct dp_rec { /* for storing the DP information */
	short	ncands;	/* # of candidate pitch intervals in the frame */
	short	*locs; /* locations of the candidates */
	short	*pvals; /* peak values of the candidates */
	double	*mpvals; /* modified peak values of the candidates */
	short	*prept; /* pointers to best previous cands. */
	double	*dpvals; /* cumulative error for each candidate */
};
/* end of structure definitions for the pitch tracker */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Structure definitions for the formant tracker.. */

typedef struct form_latt { /* structure of a DP lattice node for formant tracking */
	short ncand; /* # of candidate mappings for this frame */
	short **cand;      /* pole-to-formant map-candidate array */
	short *prept;	 /* backpointer array for each frame */
	double *cumerr; 	 /* cum. errors associated with each cand. */
} FORM;

typedef struct pole_array {   /* structure to hold raw LPC analysis data */
	double rms;    /* rms for current LPC analysis frame */
	double rms2;    /* rms for current F0 analysis frame */
	double f0;     /* fundamental frequency estimate for this frame */
	double pv;		/* probability that frame is voiced */
	double change; /* spec. distance between current and prev. frames */
	short npoles; /* # of complex poles from roots of LPC polynomial */
	double *freq;  /* array of complex pole frequencies (Hz) */
	double *band;  /* array of complex pole bandwidths (Hz) */
} POLE;
/* End of structure definitions for the formant tracker. */
