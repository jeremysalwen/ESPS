/*  dpfund.c   */
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
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)dpfund.c	1.6	12/13/93	ATT/ESI/ERL";


/* a fundamental frequency estimation algorithm using the normalized cross
	correlation function and dynamic programming	*/
/**/
/* For each frame, up to MAXCANDS crosscorrelation peaks will be considered
	as F0 intervals.  Each will be scored according to its within-
	frame properties (relative amplitude, relative location), and
	according to its connectivity with each of the candidates in the
	previous frame.  Also, a probability of voicing measure will be
	computed based on max. ampl. of the correlation function and
	frame-to-frame continuity properties. */
/* At each frame, each candidate has associated with it the following
	items:
	its peak value
	its peak value modified by its within-frame properties
	its location
	the candidate # in the previous frame yielding the min. err.
		(this is the optimum path pointer!)
	its cumulative error: (local error + connectivity error +
		cumulative error of its best-previous-frame-match).
Candidates will be ordered according to increasing F0 interval, for
	convenience. */

#include <Objects.h>
#include "tracks.h"

#define MAXPEAKS	40 /* max. # of peaks to examine for cands. */
#define	MAXCANDS	10 /* max. # peaks to allow as cands. at each fr. */

int xpr=0, ypr=0;
extern int debug;
double is_voiced();
double find_rmsmax();
char *localloc();

/*********************************************************************/
	/* Here are the tweak factors for the F0 estimator. */
/*********************************************************************/
double CO_CAND =	.3; /* all peaks within CO_CAND of the max are
				considered possible candidates. */
double PEAK_WT = 1.0; /* weight given to peak "quality". */

double LAG_WT = 0.3; /* linear attenuation coef. for lag weighting of
				crosscorrelation coefficients. */
/* This default reduces the amplitude of the longest-lag correlation
	coefficient by .3. */

double FREQ_WT = 2.0; /* The cost per octave per second of frequency change */

double F_THRESH = 20.0; /* Allowable rate of frequency change before
		applying ANY frequency change penalty (oct/sec) */

/*********************************************************************/
/*********************************************************************/

Signal *dpfund(sp, wdur, fmin, fmax, frame_int, cros, 
	       dplat, maxrms_dur, maxrms_val)
/* All frequencies are in Hz.; times are in seconds; delays in samples. */
Signal	*sp; /* a speech waveform signal */
CROSS	***cros; /* structure array containing (or to receive)
			 parameters derived from the crosscorrelation comp. */
DPREC	***dplat; /* the DP lattice used during f0 est. returned for debug */
double	wdur, /* ref. wind. dur. for crosscorr. comps. */
        maxrms_dur, /*window over past in which to pick max rms; 0 means 
                      all of past (as in older versions)*/
        maxrms_val, /*fixed value for maxrms; i.e., no window used*/
	fmin, /* minimum F0 to consider */
	fmax, /* maximum F0 to consider */
	frame_int;	/* interval between frames for F0 estimates */
{
  double freqwt, lagwt, ftemp, duration, *f, err, ferr, errmin, mincand,
         locerr, fmaxval, engref, engpre, k1, kfac, efac, prewt,
         fthresh, ttemp, engthr, sum, *f0p, *rmsp, *acpkp, *kp, *vuvp,
         rms, **datap;
  double rmsmax = 0.0;
  double *rmsmaxa = NULL;
  int maxrms_frames = 0; /*number of previous frames for max rms*/
  register short *p, *q, *r, *s;
  register int  i, j, k, m;
  int	l, nframes, smaxval;
  int	step, size, nlags, start, stop, ncomp, maxloc, maxpre,
        loc1, loc2;
  int	ncand, ncandp, minloc;
  short   peaks[MAXPEAKS], locs[MAXPEAKS], *data;
  CROSS	**cp;
  DPREC	**dp;
  Signal *f0s, *new_signal();
  char *cpp, temp[600], *new_ext();

  if ((maxrms_val != 0.0) && (maxrms_dur != 0.0)) {
    printf("dpfund: can't specify both rms window and rms value\n");
    return(NULL);
  }

  /*set up maxrms value if constant value supplied */

  if (maxrms_val != 0.0)	
    rmsmax = maxrms_val;

  if( (! sp) || (! sp->buff_size)){
    printf("No sp!\n");
    return(NULL);		/* check data pointer */
  }

  /*compute number of frames over which maxrms energy is kept*/

  maxrms_frames = ((maxrms_dur == 0)
		   ? 0 : Round(1 + ((maxrms_dur - wdur)/frame_int)));

  data = ((short**)sp->data)[0];
  step = 	Round(frame_int * sp->freq);
  size = Round(wdur * sp->freq);
  frame_int = ((double)step)/sp->freq;
  wdur = ((double)size)/sp->freq;
  start = Round(sp->freq / fmax);
  stop = Round(sp->freq / fmin);
  nlags = stop - start + 1;
  ncomp = size + stop + 1;	/* # of samples required by xcorr comp. per fr. */


  /*************************************************************/
  /*	SET UP THE FUDGE FACTORS	 */
  /*************************************************************/

  /* Lag-dependent weighting factor to emphasize early peaks. */
  /* lagwt = ((10000.0/sp->freq) * LAG_WT)/(nlags+start); */
  lagwt = (10000.0 * LAG_WT * fmin)/(sp->freq * (sp->freq + fmin));

  /* Penalty for a time skip in F0 per frame */
  freqwt = (100.0 * FREQ_WT) /(sp->freq * (frame_int));

  /* Threshold to exceed before applying ANY frequency-shift penalty */
  fthresh = F_THRESH * (frame_int);

  nframes = 1 + ((sp->buff_size - ncomp) / step);	/* # of whole analysis frames */
  cpp = new_ext(sp->name,"f0");
  datap = (double**)localloc(5 * sizeof(double*));
  if(datap &&
     (f0s = new_signal(cpp,SIG_UNKNOWN,dup_header(sp->header),datap,nframes,		       1.0/frame_int, 5))) {
       f0s->type = P_DOUBLES | SIG_F0;
  
     /* Allocate space for structure pointer, structures, and xcorr arrays. */
     if(debug&DEB_ENTRY) notify("Allocating CROSS",0,0);
     *cros = cp = (CROSS**)localloc(sizeof(CROSS*) * nframes);
     for(i=0;i<nframes;i++){
       cp[i] = (CROSS*)localloc(sizeof(CROSS));
       cp[i]->correl = (short*)localloc(sizeof(short)*(10 +nlags));
       /** extra room allocated to check segmentation fault problem !?!?!?!? **/
     }

     /* Allocate arrays to return F0 and probability of voicing. */
     /** extra room allocated to check segmentation fault problem !?!?!?!? **/
     if(debug&DEB_ENTRY) notify("Allocating f0, vuv, acpk, rms and k",0,0);
       f0p = (double*)localloc(sizeof(double) * (10 + nframes));
       vuvp = (double*)localloc(sizeof(double)*(10+nframes));
       rmsp = (double*)localloc(sizeof(double) * (10 + nframes));
       acpkp = (double*)localloc(sizeof(double) * (10 + nframes));
       kp = (double*)localloc(sizeof(double) * (10 + nframes));

     /* Allocate space for the DP storage lattice. */
     /** extra room allocated to check segmentation fault problem !?!?!?!? **/
     if(debug&DEB_ENTRY){
       notify("Allocating dplat",0,0);
       printf(" nframes%d\n",nframes);
     }
     *dplat = dp = (DPREC**)localloc(sizeof(DPREC*) * (10 + nframes));
     for(i=0;i<nframes;i++){
       dp[i] = (DPREC*)localloc(sizeof(DPREC));
       dp[i]->ncands = 0;
       /** extra room allocated to check segmentation fault problem !?!?!?!? **/
       dp[i]->locs = (short*)localloc(sizeof(short) * (2 + MAXCANDS));
       dp[i]->pvals = (short*)localloc(sizeof(short) * (2 + MAXCANDS));
       dp[i]->mpvals = (double*)localloc(sizeof(double) * (2 + MAXCANDS));
       dp[i]->prept = (short*)localloc(sizeof(short) * (2 + MAXCANDS));
       dp[i]->dpvals = (double*)localloc(sizeof(double) * (2 + MAXCANDS));
       /** extra room allocated to check segmentation fault problem !?!?!?!? **/
     }

     /* Initialize rms peak finder. */

       if (maxrms_frames != 0) /* must keep track of maxrms throughout*/
            rmsmaxa = (double*)localloc(nframes * sizeof(double));

     /* initialize the errors to zero */
     for(f= dp[0]->dpvals, i=0; i<MAXCANDS; i++) *f++ = 0;

     /***********************************************************************/
     /* MAIN FUNDAMENTAL FREQUENCY ESTIMATION LOOP */
     /***********************************************************************/
     if(debug&DEB_ENTRY){
       printf("Entering main loop; ");
       printf(" nframes:%d  size:%d  start:%d  nlags:%d\n",
	      nframes,size,start,nlags);
     }


     for(i= 0; i < nframes; i++){

       cross(data + (i * step), size, start, nlags, &engref, &k1, &maxloc,
	     &fmaxval, cp[i]->correl);
       
       cp[i]->maxloc = maxloc;
       cp[i]->maxval = fmaxval;
       cp[i]->k1 = k1;
       cp[i]->rms = sqrt(engref/size);
       cp[i]->nlags = nlags;
       cp[i]->firstlag = start;
       k1 = cp[i]->k1;
       fmaxval = cp[i]->maxval;
       rms = cp[i]->rms;

       if (maxrms_val == 0) {

	 /*set maxrms value(s) if window was specified*/

	 if (maxrms_frames == 0) {
	   if (rms > rmsmax) rmsmax = rms;
	 }
	 else {
	   rmsmaxa[i] = find_rmsmax(cp, i, maxrms_frames);
	 }
       }

       get_cand(cp[i],peaks,locs,&ncand); /* return high peaks in xcorr */

       if(ncand) {		/* were there any f0 peak candidates? */
	 if(ncand > MAXCANDS){	/* need to prune candidates? */
	   for(j=0; j < ncand-1; j++){
	     for(k=0, m=ncand-1-j; k < m; k++)
	       if(peaks[k] < peaks[k+1]){ /* sort by decreasing peak value */
		 smaxval = peaks[k];
		 maxloc = locs[k];
		 locs[k] = locs[k+1];
		 peaks[k] = peaks[k+1];
		 peaks[k+1] = smaxval;
		 locs[k+1] = maxloc;
	       }
	   }
	   ncand = MAXCANDS;
	 }			

	 /*    Move the peak value and location arrays into the dp structure */
	 for(j=0; j < ncand; j++){
	   dp[i]->locs[j] = locs[j];
	   dp[i]->pvals[j] = peaks[j];
	 }
	 dp[i]->ncands = ncand;

	 /* Apply a lag-dependent weighting to the peaks to encourage the selection
	    of the first major peak.  Translate the modified peak values into
	    costs (high peak ==> low cost). */
	 for(j=0; j < ncand; j++){
	   ftemp = 1.0 - ((double)locs[j] * lagwt);
	   dp[i]->mpvals[j] = 1.0 - ((double)peaks[j] * ftemp)/32767.0;
	 }

	 /*    PERFORM THE DISTANCE MEASURES AND ACCUMULATE THE ERRORS. */
	 ncandp = ( i )? dp[i-1]->ncands : 0 ;
	 for(k=0; k<ncand; k++){ /* for each of the current candidates... */
	   minloc = 0;
	   errmin = 2.0e30;
	   loc2 = dp[i]->locs[k];
	   for(j=0; j<ncandp; j++){ /* for each PREVIOUS candidate... */

	     /*    Get error due to absolute time difference of peaks. */
	     loc1 = dp[i-1]->locs[j];
	     if(loc1) {		/* did previous frame have valid candidates? */
	       if(loc1 > loc2){	/* compute fractional frequency change */
		 ttemp = loc1 - loc2;
		 ftemp = ttemp/loc1;
	       } else {
		 ttemp = loc2 - loc1;
		 ftemp = ttemp/loc2;
	       }
	     } else {		/* don't penalize for F0 hops */
	       ftemp = 0;
	     }
	     /* Is this frequency change unusually large? */
	     ferr = (ftemp > fthresh)? ftemp * freqwt : 0 ;

	     /*    Add in cumulative error associated with previous peak. */
	     err = ferr + dp[i-1]->dpvals[j];
	     if(err < errmin){	/* find min. error */
	       errmin = err;
	       minloc = j;
	     }
	   } /* Now have found the best path from this cand. to prev. frame */

	   if( i ){
	     dp[i]->dpvals[k] = errmin + (PEAK_WT * dp[i]->mpvals[k]);
	     dp[i]->prept[k] = minloc;
	   } else {		/* this is the first frame */
	     dp[i]->dpvals[k] = dp[i]->mpvals[k];
	     dp[i]->prept[k] = 0;
	   }
	 }			/*    end of DP for this frame */

       } else {			/* if(! ncand) */
	 dp[i]->ncands = 1;
	 dp[i]->mpvals[0] = 0.0;
	 dp[i]->pvals[0] = 32767;
	 dp[i]->locs[0] = 0;	/* flag that this is a dummy frame */
	 errmin = 2.0e30;
	 minloc = 0;
	 if( i ){
	   for(j=0;j<dp[i-1]->ncands;j++){ /* find best previous candidate */
	     if(dp[i-1]->dpvals[j] < errmin){
	       errmin = dp[i-1]->dpvals[j];
	       minloc  = j;
	     }
	   }
	 } else {		/* if this is the first frame */
	   errmin = 0;
	   minloc = 0;
	 }
	 dp[i]->prept[0] = minloc;
	 dp[i]->dpvals[0] = errmin;
       }			/* end if there were no candidates for this frame */

       if( debug & DEB_LPC_PARS ){
	 printf("\n%d engref:%10.0f engpre:%10.0f max:%7.5f kfac:%7.5f loc:%4d",
		i,engref,engpre,fmaxval,k1,maxloc);
       }

     }
     /***************************************************************/
     /* done with DP for all frames */
     /***************************************************************/

     /* Now backtrack through the DP lattice to retrieve the optimum F0 track. */
     /* First find the best candidate in the final frame. */
     j = nframes-1;
     errmin = dp[j]->dpvals[0];
     minloc = 0;
     for(i=1; i < dp[j]->ncands; i++){
       if((ftemp = dp[j]->dpvals[i]) < errmin){
	 errmin = ftemp;
	 minloc = i;
       }
     }

     /* Use the best final candidate as the starting point and work backwards. */
     for(i= nframes-1; i >= 0; i--){ /* for all frames, starting with last... */
       if(i==(nframes-1)) {
	 engref = (.5 * cp[i-1]->rms)+ (.5 * cp[i]->rms);
       } else {
	 if(i == 0) {
	   engref =  (.5 * cp[i]->rms)+ (.5 * cp[i+1]->rms);
	 } else {
	   engref = (.25 * cp[i-1]->rms)+ (.5 * cp[i]->rms)+ (.25 * cp[i+1]->rms);
	 }
       }
       rmsp[i] = cp[i]->rms;
       kp[i] = cp[i]->k1;
       acpkp[i] = ((double)dp[i]->pvals[minloc])/32767.0; /* use ac at f0 lag */
       if ((maxrms_frames == 0) || (maxrms_val != 0))
	 vuvp[i] = is_voiced(cp[i]->rms,engref/rmsmax,cp[i]->k1,acpkp[i]);
       else
	 vuvp[i] = is_voiced(cp[i]->rms,engref/rmsmaxa[i],cp[i]->k1,acpkp[i]);

       loc1 = dp[i]->locs[minloc];
       ftemp = loc1;
       if(loc1 > 0){		/* Was f0 actually estimated for this frame? */
 /*****************************************************************************
  * Parabolic interpolation of the crosscorrelation function for more
  * accurate F0 estimation  -- KS 3-Jun-88 (due to DT circa 1973)
  *****************************************************************************/
	   if (loc1 > start && loc1 < stop)
	     { register double correlmax,  /* 3 values of the cross-correlation function  */
	                       cprevious,
		               cnext,
		               denominator;
	       register int itemp;

	       itemp = loc1 - start;
	       correlmax = cp[i]->correl[itemp];
	       cprevious = cp[i]->correl[itemp + 1];
	       cnext   =   cp[i]->correl[itemp - 1];
	       denominator = (2.0 * ( cprevious + cnext - (2.0 * correlmax) ));
	       /*
		* Only interpolate if correlmax is indeed a local turning point
		*/
	       if (fabs(denominator) > 0.000001) 
		 ftemp = ftemp + 2.0 
		         - ( ( (5.0 * cprevious) + (3.0 * cnext) - (8.0 * correlmax) )
			     / denominator );
	     }
        /* End of interpolation ***********************************************************/
	 f0p[i] = sp->freq/ftemp;
	 minloc = dp[i]->prept[minloc];
       } else {			/* just fake some arbitrary F0. */
	 f0p[i] = 0;
       }
       if( debug & DEB_LPC_PARS )
	 printf(" i:%4d%8.1f%8.1f\n",i,f0p[i],vuvp[i]);
     }

     /* Free up the DP lattice.
	for(i=0;i<nframes;i++){
	free(dp[i]->locs);
	free(dp[i]->prept);
	free(dp[i]->mpvals);
	free(dp[i]->pvals);
	free(dp[i]->dpvals);
	free(dp[i]);
	}
	free(dp); */
       datap[0] = f0p;
       datap[1] = vuvp;
       datap[2] = rmsp;
       datap[3] = acpkp;
       datap[4] = kp;
       f0s->band = fmax;
       minloc = sp->version + 1;
       head_printf(f0s->header,"version",&minloc);
       sprintf(temp,"dpfund: CO_CAND %f PEAK_WT %f LAG_WT %f FREQ_WT %f F_THRESH %f window %f fmax %f fmin %f signal %s",
	       CO_CAND,PEAK_WT,LAG_WT,FREQ_WT,F_THRESH, wdur, fmax, fmin, sp->name);
       head_printf(f0s->header,"operation",temp);
       head_printf(f0s->header,"type_code",&(f0s->type));
       head_printf(f0s->header,"frequency",&(f0s->freq));
       head_printf(f0s->header,"samples",&(f0s->buff_size));
       head_printf(f0s->header,"dimensions",&(f0s->dim));
       head_ident(f0s->header,"F0 P(voice) rms ac_peak k1");
       head_printf(f0s->header,"time", get_date());
       return(f0s);
     } else
       printf("Couldn't make a new signal or allocate data array pointer\n");
  return(NULL);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Get likely candidates for F0 peaks. */
/* Note that if the number of peaks found exceeds MAXPEAKS this
	routine uncerimoniously quits looking for peaks.  The better way to
	handle this would be to only return the MAXPEAKS highest peaks.
	This will be corrected later if necessary. */
get_cand(cross,peak,loc,ncand)
     CROSS *cross;
     short	*peak, *loc;
     int  *ncand;
{
  double x;
  register short i, lastl, *p, *q, *r, *s, *t, clip;
  short start, ncan, maxl;

  clip = Round(CO_CAND * cross->maxval * 32767.0);
  maxl = cross->maxloc;
  lastl = cross->nlags - 1;
  start = cross->firstlag;

  ncan=0;
  p= cross->correl;
  q= p+1;
  r= q+1;
  for(s = peak, t = loc, i=1; i<lastl; i++, p++, q++, r++){
    if(*q > clip)		/* is this a high peak? */
      /* NOTE: this finds SHOLDERS as well as peaks (is this a good idea?) */
      if(((*q > *p)&&(*q >= *r))||((*q >= *p)&&(*q > *r))){
	*s++ = *q;		/* record the peak value */
	*t++ = i + start;	/* and its location */
	ncan++;			/* count number of peaks found */
	if(ncan >= (MAXPEAKS-1)){ /* leave room for maxloc */
	  *s++ = cross->maxval;
	  *t++ = cross->maxloc;
	  ncan++;
	  i = lastl;		/* force a break and return */
	}
      }
  }
  *ncand = ncan;
}
	

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
      /*  A multivariate Gaussian voiced/unvoiced classifier. */
/* trained on hand labeled data from 25 male and 25 female talkers */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double is_voiced(eng, releng, k1, corrp)
     double eng, releng, k1, corrp;
{
/* Inverse covariance matrix for the voiced class. */
static	double	invvo[] =
{
 8.67465e-6,  -2.696427e-5,  -5.67748e-6,  -2.534274e-6,  -2.696427e-5,
 1.060583e-4,  3.08306e-5,  -1.774188e-5,  -5.67748e-6,  3.08306e-5,
 9.58462e-5,  -2.967243e-5,  -2.534274e-6,  -1.774188e-5,  -2.967244e-5,
 9.57112e-5
};
/* Inverse covariance matrix for the unvoiced class. */
static	double	invuv[] =
{
 0.001983508,  -0.00646111,  -3.63317e-6,  1.170623e-5,  -0.00646111,
 0.02168207,  5.134557e-5,  -7.491706e-5,  -3.633396e-6,  5.13463e-5,
 9.6784e-6,  -9.93972e-6,  1.170624e-5,  -7.49171e-5,  -9.93972e-6,
 5.78222e-5
};
/* Mean vector for the voiced class. */
static	double	vmean[] =
{
 1953.138,  611.861,  899.261,  914.513
};

/* Mean vector for the unvoiced class. */
static	double	uvmean[] =
{
 64.1201,  19.81763,  536.3225,  454.782
};

static double	 uvdet =  35.37686,	/* these are the LOGS of the
					 * determinants */
		 vodet =  41.43197;

  double dv, du, fe[6], uvt[4], vt[4], xu[4], xv[4], *fpv, *fpu, gv, guv, pv,
        coefv, coefuv, tpisq, tmp;
  int	k, l, nfeats;

  tpisq = (2.0 * 3.1415927)*(2.0 * 3.1415927)*2.0;
  coefv = 1.0/(tpisq * exp(vodet/2));
  coefuv = 1.0/(tpisq * exp(uvdet/2));
  nfeats = 4;
  
  fe[1] = releng * 1000.0;
  fe[0] = fe[1] * 3.2;		/* disable the absolute energy measure */
  fe[2] = k1 * 1000.0;
  fe[3] = corrp * 1000.0;

  for(k=0; k < nfeats; k++) {
    vt[k] = fe[k] - vmean[k];
    uvt[k] = fe[k] - uvmean[k];
  }

  for(k=0, fpv = invvo, fpu = invuv; k < nfeats; k++) {
    for(l=0, xv[k]=0.0, xu[k]=0.0 ; l < nfeats; l++){
      xv[k] += vt[l] * (*fpv++);
      xu[k] += uvt[l] * (*fpu++);
    }
  }
  for(dv = 0.0, du = 0.0, k=0; k < nfeats; k++) {
    dv += xv[k] * vt[k];
    du += xu[k] * uvt[k];
  }
  gv = exp( -.5 * dv ) * coefv;
  guv = exp( -.5 * du ) * coefuv;
  if((tmp = (gv + guv)) > 0.0) pv = gv/tmp;
  else pv = 0.0;
  return(pv);
  /* This once returned a boolean:
     dv += vodet;
     du += uvdet;
     if(dv < du) return(TRUE);
     else return(FALSE); */
    
}

double
find_rmsmax(cross, frame, window)
CROSS **cross;			/* cross correlation structure */
int frame;			/* frame number */
int window;			/* number of (previous) ccor frames over
				 which to find max rms (0 means entire
				 history as in old version)*/
{
    static int frame_max = 0;
    static double rms_max = 0;
    int i, low;

    double rms = cross[frame]->rms;

    if (frame == 0) { /*first time through set rmsmax at first frame*/
      frame_max = 0;
      rms_max = rms;
      return(rms_max);
    }

    if (window == 0) { /*if no window, just update the max*/

      if (rms > rms_max) rms_max = rms;
    }
    else { /* must process window */

      low = frame - window + 1;
      if (low < 0) low = 0;

      if (frame_max >= low) { /*previous max still in window*/

	if (rms > rms_max) {
	  rms_max = rms;
	  frame_max = frame;
	}
      }
      else {  /*must search window for current max*/
	rms_max = 0;
	frame_max = low;
	for (i = low; i <= frame; i++) {
	  rms = cross[i]->rms;
	  if (rms >= rms_max) {
	    rms_max = rms;
	    frame_max = i;
	  }
	}      
      }
    }
    return(rms_max);
  }


