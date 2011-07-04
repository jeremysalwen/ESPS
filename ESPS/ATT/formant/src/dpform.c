/*	dpform.c       */
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

static char *sccs_id = "@(#)dpform.c	1.6	9/21/94	ATT/ESI/ERL";


/* a formant tracker based on LPC polynomial roots and dynamic programming */
				/***/
/* At each frame, the LPC poles are ordered by increasing frequency.  All
   "reasonable" mappings of the poles to F1, F2, ... are performed.
   The cost of "connecting" each of these mappings with each of the mappings
   in the previous frame is computed.  The lowest cost connection is then
   chosen as the optimum one.  At each frame, each mapping has associated
   with it a cost based on the formant bandwidths and frequencies.  This
   "local" cost is finally added to the cost of the best "connection."  At
   end of utterance (or after a reasonable delay like .5sec) the best
   mappings for the entire utterance may be found by retracing back through
   best candidate mappings, starting at end of utterance (or current frame).
*/

#include <Objects.h>
#include "tracks.h"

/* Here are the major fudge factors for tweaking the formant tracker. */
#define MAXCAN	300  /* maximum number of candidate mappings allowed */
static double MISSING = 1, /* equivalent delta-Hz cost for missing formant */
	NOBAND = 1000, /* equivalent bandwidth cost of a missing formant */
	DF_FACT =  20.0, /* cost for proportional frequency changes */
	/* with good "stationarity" function:*/
/*        DF_FACT =  80.0, /*  cost for proportional frequency changes */
	DFN_FACT = 0.3, /* cost for proportional dev. from nominal freqs. */
	BAND_FACT = .002, /* cost per Hz of bandwidth in the poles */
/*	F_BIAS	  = 0.0004,   bias toward selecting low-freq. poles */
	F_BIAS	  = 0.000, /*  bias toward selecting low-freq. poles */
	F_MERGE = 2000.0; /* cost of mapping f1 and f2 to same frequency */
static double	*fre,
		fnom[]  = {  500, 1500, 2500, 3500, 4500, 5500, 6500},/*  "nominal" freqs.*/
		fmins[] = {   50,  400, 1000, 2000, 2000, 3000, 3000}, /* frequency bounds */
		fmaxs[] = { 1500, 3500, 4500, 5000, 6000, 6000, 8000}; /* for 1st 5 formants */

static int	maxp,	/* number of poles to consider */
		maxf,	/* number of formants to find */
		ncan,  domerge = TRUE;

static short **pc;

extern int debug;

static int candy(), canbe();

set_nominal_freqs(f1)
     double f1;
{
  int i;
  double f, fp, fm;
  for(i=0; i < MAXFORMANTS; i++) {
    fnom[i] = ((i * 2) + 1) * f1;
    fmins[i] = fnom[i] - ((i+1) * f1) + 50.0;
    fmaxs[i] = fnom[i] + (i * f1) + 1000.0;
  }
}

Signal *dpform(ps, nform, nom_f1)
     Signal *ps;
     int nform;
     double nom_f1;
{
  double pferr, conerr, minerr, dffact, ftemp, berr, ferr, bfact, ffact,
         rmsmax, fbias, **fr, **ba, rmsdffact, merger, merge_cost,
         FBIAS, get_stat_max();
  register int	i, j, k, l, ic, ip, mincan;
  short	**pcan;
  char *localloc();
  FORM	**fl;
  POLE	**pole; /* raw LPC pole data structure array */
  Signal *fbs, *new_signal();
  char *cp, temp[600], *new_ext();
  int dmaxc,dminc,dcountc,dcountf;
  
  if(ps) {
    if(nom_f1 > 0.0)
      set_nominal_freqs(nom_f1);
    pole = (POLE**)ps->data;
    rmsmax = get_stat_max(pole, ps->buff_size);
    FBIAS = F_BIAS /(.01 * ps->freq);
    /* Setup working values of the cost weights. */
    dffact = (DF_FACT * .01) * ps->freq; /* keep dffact scaled to frame rate */
    bfact = BAND_FACT /(.01 * ps->freq);
    ffact = DFN_FACT /(.01 * ps->freq);
    merge_cost = F_MERGE;
    if(merge_cost > 1000.0) domerge = FALSE;

    /* Allocate space for the formant and bandwidth arrays to be passed back. */
    if(debug & DEB_ENTRY){
      printf("Allocating formant and bandwidth arrays in dpform()\n");
    }
    fr = (double**)localloc(sizeof(double*) * nform * 2);
    ba = fr + nform;
    for(i=0;i < nform*2; i++){
      fr[i] = (double*)localloc(sizeof(double) * ps->buff_size);
    }
    cp = new_ext(ps->name,"fb");
    if((fbs=new_signal(cp,SIG_UNKNOWN,dup_header(ps->header),fr,ps->buff_size,		       ps->freq, nform * 2))) {
      /* Allocate space for the raw candidate array. */
      if(debug & DEB_ENTRY){
	printf("Allocating raw candidate array in dpform()\n");
      }
      pcan = (short**)localloc(sizeof(short*) * MAXCAN);
      for(i=0;i<MAXCAN;i++) pcan[i] = (short*)localloc(sizeof(short) * nform);

      /* Allocate space for the dp lattice */
      if(debug & DEB_ENTRY){
	printf("Allocating DP lattice structure in dpform()\n");
      }
      fl = (FORM**)localloc(sizeof(FORM*) * ps->buff_size);
      for(i=0;i < ps->buff_size; i++)
	fl[i] = (FORM*)localloc(sizeof(FORM));

      /*******************************************************************/
      /* main formant tracking loop */
      /*******************************************************************/
      if(debug & DEB_ENTRY){
	printf("Entering main computation loop in dpform()\n");
      }
      for(i=0; i < ps->buff_size; i++){	/* for all analysis frames... */

	ncan = 0;		/* initialize candidate mapping count to 0 */

	/* moderate the cost of frequency jumps by the relative amplitude */
	rmsdffact = pole[i]->rms;
	rmsdffact = rmsdffact/rmsmax;
	rmsdffact = rmsdffact * dffact;

	/* Get all likely mappings of the poles onto formants for this frame. */
	if(pole[i]->npoles){	/* if there ARE pole frequencies available... */
	  get_fcand(pole[i]->npoles,pole[i]->freq,pole[i]->band,nform,pcan);

	  /* Allocate space for this frame's candidates in the dp lattice. */
	  fl[i]->prept =  (short*)localloc(sizeof(short) * ncan);
	  fl[i]->cumerr = (double*)localloc(sizeof(double) * ncan);
	  fl[i]->cand =   (short**)localloc(sizeof(short*) * ncan);
	  for(j=0;j<ncan;j++){	/* allocate cand. slots and install candidates */
	    fl[i]->cand[j] = (short*)localloc(sizeof(short) * nform);
	    for(k=0; k<nform; k++)
	      fl[i]->cand[j][k] = pcan[j][k];
	  }
	}
	fl[i]->ncand = ncan;
	/* compute the distance between the current and previous mappings */
	for(j=0;j<ncan;j++){	/* for each CURRENT mapping... */
	  if( i ){		/* past the first frame? */
	    minerr = 0;
	    if(fl[i-1]->ncand) minerr = 2.0e30;
	    mincan = -1;
	    for(k=0; k < fl[i-1]->ncand; k++){ /* for each PREVIOUS map... */
	      for(pferr=0.0, l=0; l<nform; l++){
		ic = fl[i]->cand[j][l];
		ip = fl[i-1]->cand[k][l];
		if((ic >= 0)	&& (ip >= 0)){
		  ftemp = 2.0 * fabs(pole[i]->freq[ic] - pole[i-1]->freq[ip])/
		           (pole[i]->freq[ic] + pole[i-1]->freq[ip]);
    /*		  ftemp = pole[i]->freq[ic] - pole[i-1]->freq[ip];
		  if(ftemp >= 0.0)
		    ftemp = ftemp/pole[i-1]->freq[ip];
		  else
		    ftemp = ftemp/pole[i]->freq[ic]; */
		  /* cost prop. to SQUARE of deviation to discourage large jumps */
		  pferr += ftemp * ftemp;
		}
		else pferr += MISSING;
	      }
	      /* scale delta-frequency cost and add in prev. cum. cost */
	      conerr = (rmsdffact * pferr) + fl[i-1]->cumerr[k]; 
	      if(conerr < minerr){
		minerr = conerr;
		mincan = k;
	      }
	    }			/* end for each PREVIOUS mapping... */
	  }	else {		/* (i.e. if this is the first frame... ) */
	    minerr = 0;
	  }

	  fl[i]->prept[j] = mincan; /* point to best previous mapping */
	  /* (Note that mincan=-1 if there were no candidates in prev. fr.) */
	  /* Compute the local costs for this current mapping. */
	  for(k=0, berr=0, ferr=0, fbias=0; k<nform; k++){
	    ic = fl[i]->cand[j][k];
	    if(ic >= 0){
	      if( !k ){		/* F1 candidate? */
		ftemp = pole[i]->freq[ic];
		merger = (domerge &&
			  (ftemp == pole[i]->freq[fl[i]->cand[j][1]]))?
			  merge_cost: 0.0;
	      }
	      berr += pole[i]->band[ic];
	      ferr += (fabs(pole[i]->freq[ic]-fnom[k])/fnom[k]);
	      fbias += pole[i]->freq[ic];
	    } else {		/* if there was no freq. for this formant */
	      fbias += fnom[k];
	      berr += NOBAND;
	      ferr += MISSING;
	    }
	  }

	  /* Compute the total cost of this mapping and best previous. */
	  fl[i]->cumerr[j] = (FBIAS * fbias) + (bfact * berr) + merger +
	                     (ffact * ferr) + minerr;
	}			/* end for each CURRENT mapping... */

	if(debug & DEB_LPC_PARS){
	  printf("\nFrame %4d  # candidates:%3d stat:%f prms:%f",i,ncan,rmsdffact,pole[i]->rms);
	  for (j=0; j<ncan; j++){
	    printf("\n	");
	    for(k=0; k<nform; k++)
	      if(pcan[j][k] >= 0)
		printf("%6.0f ",pole[i]->freq[fl[i]->cand[j][k]]);
	      else
		printf("  NA   ");
	    printf("  cum:%7.2f pp:%d",fl[i]->cumerr[j], fl[i]->prept[j]);
	  }
	}
      }				/* end for all analysis frames... */	
      /**************************************************************************/

      /* Pick the candidate in the final frame with the lowest cost. */
      /* Starting with that min.-cost cand., work back thru the lattice. */
      if(debug & DEB_ENTRY){
	printf("Entering backtrack loop in dpform()\n");
      }
      dmaxc = 0;
      dminc = 100;
      dcountc = dcountf = 0;
      for(mincan = -1, i=ps->buff_size - 1; i>=0; i--){
	if(debug & DEB_LPC_PARS){
	  printf("\nFrame:%4d mincan:%2d ncand:%2d ",i,mincan,fl[i]->ncand);
	}
	if(mincan < 0)		/* need to find best starting candidate? */
	  if(fl[i]->ncand){	/* have candidates at this frame? */
	    minerr = fl[i]->cumerr[0];
	    mincan = 0;
	    for(j=1; j<fl[i]->ncand; j++)
	      if( fl[i]->cumerr[j] < minerr ){
		minerr = fl[i]->cumerr[j];
		mincan = j;
	      }
	  }
	if(mincan >= 0){	/* if there is a "best" candidate at this frame */
	  if((j = fl[i]->ncand) > dmaxc) dmaxc = j;
	  else
	    if( j < dminc) dminc = j;
	  dcountc += j;
	  dcountf++;
	  for(j=0; j<nform; j++){
	    k = fl[i]->cand[mincan][j];
	    if(k >= 0){
	      fr[j][i] = pole[i]->freq[k];
	      if(debug & DEB_LPC_PARS){
		printf("%6.0f",fr[j][i]);
	      }
	      ba[j][i] = pole[i]->band[k];
	    } else {		/* IF FORMANT IS MISSING... */
	      if(i < ps->buff_size - 1){
		fr[j][i] = fr[j][i+1]; /* replicate backwards */
		ba[j][i] = ba[j][i+1];
	      } else {
		fr[j][i] = fnom[j]; /* or insert neutral values */
		ba[j][i] = NOBAND;
	      }
	      if(debug & DEB_LPC_PARS){
		printf("%6.0f",fr[j][i]);
	      }
	    }
	  }
	  mincan = fl[i]->prept[mincan];
	} else {		/* if no candidates, fake with "nominal" frequencies. */
	  for(j=0; j < nform; j++){
	    fr[j][i] = fnom[j];
	    ba[j][i] = NOBAND;
	    if(debug & DEB_LPC_PARS){
	      printf("%6.0f",fr[j][i]);
	    }
	  } 
	}			/* note that mincan will remain =-1 if no candidates */
      }				/* end unpacking formant tracks from the dp lattice */
      /* Deallocate all the DP lattice work space. */
      if(debug & DEB_ENTRY){
	printf("%s complete; max. cands:%d  min. cands.:%d average cands.:%f\n",
	     fbs->name,dmaxc,dminc,((double)dcountc)/dcountf);
	printf("Entering memory deallocation in dpform()\n");
      }
      for(i=ps->buff_size - 1; i>=0; i--){
	if(fl[i]->ncand){
	  if(fl[i]->cand) {
	    for(j=0; j<fl[i]->ncand; j++) free(fl[i]->cand[j]);
	    free(fl[i]->cand);
	    free(fl[i]->cumerr);
	    free(fl[i]->prept);
	  }
	}
      }
      for(i=0; i<ps->buff_size; i++)	free(fl[i]);
      free(fl);
      fl = 0;

      /* Deallocate space for the raw candidate aray. */
      for(i=0;i<MAXCAN;i++) free(pcan[i]);
      free(pcan);

      fbs->version += 1;
      head_printf(fbs->header,"version",&(fbs->version));
      sprintf(temp,"dpform: MISSING %f NOBAND %f DF_FACT %f DFN_FACT %f BAND_FACT %f F_BIAS %f F_MERGE %f signal %s",
	      MISSING,NOBAND,DF_FACT,DFN_FACT,BAND_FACT,F_BIAS,F_MERGE, ps->name);
      head_printf(fbs->header,"operation",temp);
      fbs->type = P_DOUBLES | SIG_FORMANTS;
      head_printf(fbs->header,"type_code",&(fbs->type));
      head_printf(fbs->header,"dimensions",&(fbs->dim));
      cp = temp;
      for(i=0; i< nform; i++) {
	sprintf(cp,"F%d ",i+1);
	cp = temp + strlen(temp);
      }
      for(i=0; i< nform; i++) {
	sprintf(cp,"B%d ",i+1);
	cp = temp + strlen(temp);
      }
      head_ident(fbs->header,temp);
      head_printf(fbs->header,"time",get_date());
      return(fbs);
    } else
      printf("Can't create a new Signal in dpform()\n");
  } else
    printf("Bad data pointers passed into dpform()\n");
  return(NULL);
}



/* Given a set of pole frequencies and allowable formant frequencies
   for nform formants, calculate all possible mappings of pole frequencies
   to formants, including, possibly, mappings with missing formants. */
get_fcand(npole,freq,band,nform,pcan)
     int	npole, nform;
     short **pcan;
     double	*freq, *band; /* poles ordered by increasing FREQUENCY */
{	

  ncan = 0;
  pc = pcan;
  fre = freq;
  maxp = npole;
  maxf = nform;
  candy(ncan, 0, 0);
  ncan++;	/* (converts ncan as an index to ncan as a candidate count) */
}

/* This does the real work of mapping frequencies to formants. */
static int candy(cand,pnumb,fnumb)
     int	cand, /* candidate number being considered */
       pnumb, /* pole number under consideration */
       fnumb;	/* formant number under consideration */
{
  int i,j;

  if(fnumb < maxf) pc[cand][fnumb] = -1;
  if((pnumb < maxp)&&(fnumb < maxf)){
    /*   printf("\ncan:%3d  pnumb:%3d  fnumb:%3d",cand,pnumb,fnumb); */
    if(canbe(pnumb,fnumb)){
      pc[cand][fnumb] = pnumb;
      if(domerge&&(fnumb==0)&&(canbe(pnumb,fnumb+1))){ /* allow for f1,f2 merger */
	ncan++;
	pc[ncan][0] = pc[cand][0];
	candy(ncan,pnumb,fnumb+1); /* same pole, next formant */
      }
      candy(cand,pnumb+1,fnumb+1); /* next formant; next pole */
      if(((pnumb+1) < maxp) && canbe(pnumb+1,fnumb)){
	/* try other frequencies for this formant */
	ncan++;			/* add one to the candidate index/tally */
	/*		printf("\n%4d  %4d  %4d",ncan,pnumb+1,fnumb); */
	for(i=0; i<fnumb; i++)	/* clone the lower formants */
	  pc[ncan][i] = pc[cand][i];
	candy(ncan,pnumb+1,fnumb);
      }
    } else {
      candy(cand,pnumb+1,fnumb);
    }
  }
  /* If all pole frequencies have been examined without finding one which
     will map onto the current formant, go on to the next formant leaving the
     current formant null. */
  if((pnumb >= maxp) && (fnumb < maxf-1) && (pc[cand][fnumb] < 0)){
    if(fnumb){
      j=fnumb-1;
      while((j>0) && pc[cand][j] < 0) j--;
      i = ((j=pc[cand][j]) >= 0)? j : 0;
    } else i = 0;
    candy(cand,i,fnumb+1);
  }
}


static int canbe(pnumb, fnumb) /* can this pole be this freq.? */
int	pnumb, fnumb;
{
return((fre[pnumb] >= fmins[fnumb])&&(fre[pnumb] <= fmaxs[fnumb]));
}


/*      ----------------------------------------------------------      */
/* find the maximum in the "stationarity" function (stored in rms) */
double get_stat_max(pole, nframes)
     register POLE **pole;
     register int nframes;
{
  register int i;
  register double amax, t;

  for(i=1, amax = (*pole++)->rms; i++ < nframes; )
    if((t = (*pole++)->rms) > amax) amax = t;

  return(amax);
}

