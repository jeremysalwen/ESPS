/* 
 * This material contains proprietary software of Entropic Speech, Inc.   
 * Any reproduction, distribution, or publication without the the prior	   
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice			
 *
 *             "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 *				
 * Written by:  Joseph T. Buck
 * Modified by: David Burton 			
 * Module:	lloyd.c
 *	
 * Generates a scalar quantizer using Lloyd's algorithm
 *
 */
#ifndef lint
	static char *sccs_id = "@(#)lloyd.c	1.14 2/20/96 ESI";
#endif
#define delta 0.00001
#define HUGE 1.0e37
#define MAXEMPTY 10
#include <stdio.h>
#include <esps/esps.h>
#include <assert.h>
#define Fprintf (void)fprintf
#ifndef DEC_ALPHA
char   *malloc ();
#endif
void qsort();

/*
 * This function is used by qsort to sort the data at the end.
 */
static int
compar (x, y)
register double *x, *y;
{
    return (*x > *y) ? 1 : -1;
}

/* This function obtains values for empty cells by splitting the
 * cells with the largest populations.
 */
static void
fixempty (cbk, pop, empty, nempty, ncbk)
long     pop[];
int empty[];
double  cbk[];
{
    int     i, iemp, imost;
    for (iemp = 0; iemp < nempty; iemp++) {
	int     most = 0;
	for (i = 0; i < ncbk; i++) {
	    if (pop[i] > most) {
		most = pop[i];
		imost = i;
	    }
	}
	cbk[empty[iemp]] = cbk[imost] + delta;
	pop[imost] = 0;		/* so we wont use it again */
    }
}


/*
 * This is an implementaion of the Lloyd algorithm for generation
 * of a locally optimal scalar quantizer.  The only thing below
 * that may need explanation is the convergence threshold.  The
 * routine splits codewords to double their number, then replaces
 * each codeword by the centroid of all codewords that map into it
 * (are nearest to it).  This goes on until (old - new)/old, where
 * old is the old mean square error, and new is the new mean square
 * error, is less than or equal to convergence.  A convergence
 * threshold of zero will work, but may be more expensive than
 * required.
 */
void
lloyd (data, n, cbk, ncbk_desired, convergence, hstrm, final_dist, cbk_dist, pop)
register double *data;		/* training sequence for quantizer */
long n;				/* number of elements in data */
register double *cbk;		/* output array for codebook */
unsigned ncbk_desired;		/* number of entries desired for codebook */
double convergence;		/* fractional convergence threshold */
FILE *hstrm;			/* stream for history info, if != NULL */
double *final_dist;		/* average distortion for final codebook */
double *cbk_dist;		/* average distortion associated with cbk[k] */
register long *pop;		/* number of data elements with nearest neighbor cbk[k] */
{
    register int i;		/* indexes data and map */
    register int k;		/* indexes cbk, cbk_dist, and pop */
    int ncbk;			/* current # of codewords */
    int new_ncbk=0;		/* new # of codewords after split */
    int iter;			/* counts iterations */
    int nempty;			/* counts empty cells */
    int empty[MAXEMPTY];	/* array of empty cell locations */
    int smallpop;		/* smallest cluster population */
    double sum;			/* used to compute mean */
    double omsqerr;		/* previous mean square error */
    double msqerr=HUGE;		/* current mean square error */
    double berr;		/* best error: min error for data[i] */
    double err;			/* squared error for current i,k pair */
    double tmp;			/* temporay storage for array sorting*/
    extern short debug_level;	/* debug flag */    
/*
 * map[i] will be the index of the codeword that data[i] is nearest to.
 */
    int    *map = (int *) malloc ((unsigned)(n * sizeof *map));
/*
 * copy[k] is temporary storage for final codebook
*/
    double *copy = (double *) malloc (ncbk_desired * sizeof *copy);

    assert(map != NULL);
    assert(copy != NULL);

/* check input function data */
   if(debug_level > 2)
   {
     (void)fprintf(stderr, 
           "LLOYD: number of data items is %d, number of codewords is %d\n",
             n, ncbk_desired);
     (void)fprintf(stderr, "LLOYD: convergence threshold is %f\n", convergence);
     (void)fprintf(stderr, 
             "LLOYD: data[0] is %f, data[%d] is %f\n", data[0], n-1, data[n-1]);
     (void)fflush(stderr);
   }
/* initialize several arrays to zero*/
    for(k=0; k<ncbk_desired; k++) {
       copy[k] = 0.0;
    }

    /* first compute the mean */
    for (sum = i = 0; i < n; i++)
	sum += data[i];
    /* The first codebook is the mean */
    cbk[0] = sum / n;
    if (hstrm) {
	Fprintf (hstrm, "Lloyd algorithm for generation of a scalar quantizer\n");
	Fprintf (hstrm, "%d samples, centroid = %g, %d codewords required\n",
			n, cbk[0], ncbk_desired);
	Fprintf (hstrm, "Convergence threshold: %g\n", convergence);
    }
    ncbk = 1;
    /* Now get larger codebooks */
    while (ncbk < ncbk_desired) {
       if(msqerr <= 0.){
	Fprintf(stderr, 
	    "Mean Squared error = 0 before codebook has %d codewords\n", 
	    ncbk_desired);
	    Fprintf(stderr, "Redesign with codebook size = %d\n", new_ncbk);
	    exit(1);
       }
	/* split */
	new_ncbk = ncbk * 2;
	if (hstrm)
	    Fprintf (hstrm, "splitting: now have %d codewords\n", new_ncbk);
	if (new_ncbk > ncbk_desired)
	    new_ncbk = ncbk_desired;
	for (k = 0; k < new_ncbk - ncbk; k++)
	    cbk[ncbk + k] = cbk[k] + delta;
	ncbk = new_ncbk;
	omsqerr = HUGE;
	msqerr = HUGE / 100;
	iter = 0;
	while ((omsqerr - msqerr) / omsqerr > convergence) {
            iter += 1;
	    /* map the dataing data into clusters */
	    omsqerr = msqerr;
	    for (msqerr = i = 0; i < n; i++) {
		berr = HUGE;
		for (k = 0; k < ncbk; k++) {
		    err = (data[i] - cbk[k]);
		    err *= err;
		    if (err < berr) {
			map[i] = k;
			berr = err;
		    }
		}
		msqerr += berr;
	    }
	    msqerr /= (double) n;
	    if (hstrm)
		Fprintf (hstrm, "iter %d:\tmean square error = %g\n", iter, msqerr);
	    /* move the centroids */
	    nempty = 0;
	    for (k = 0; k < ncbk; k++)
		cbk[k] = pop[k] = 0;
	    for (i = 0; i < n; i++) {
		cbk[map[i]] += data[i];
		pop[map[i]] += 1;
	    }
	    smallpop = pop[0];
	    for (k = 0; k < ncbk; k++) {
		if (pop[k] < smallpop) smallpop = pop[k];
		if (pop[k])
		    cbk[k] /= (double) pop[k];
		else {
		    if (nempty >= MAXEMPTY) {
			Fprintf (stderr, "TOO MANY EMPTY CELLS!\n");
			exit (1);
		    }
		    empty[nempty++] = k;
		}
	    }
	    if (nempty > 0) {
		if (hstrm)
		    Fprintf (hstrm, "\t%d empty cells\n", nempty);
		fixempty (cbk, pop, empty, nempty, ncbk);
	    }
	    else if (hstrm)
		Fprintf (hstrm, "\tSmallest cluster has %d members\n", smallpop);
	    if(msqerr <= 0.)
		break;
	}
    }
    
    *final_dist = msqerr;
    if(debug_level>5)
    {
      (void)fprintf(stderr,"pop was:\n");
      for(k=0;k<ncbk_desired;k++)
          (void)fprintf(stderr, " %d ",pop[k]);
      (void)fprintf(stderr,"\n");
    }
/* save a copy of the unsorted codebook*/
    for (k=0; k<ncbk_desired; k++){
    copy[k] = cbk[k];
    }
    if(debug_level>5)
    {
      (void)fprintf(stderr, "cbk was:\n");
      for(k=0;k<ncbk_desired;k++)
          (void)fprintf(stderr," %g ", cbk[k]);
      (void)fprintf(stderr, "\n");
    }

/* compute final individual codeword distortions */
    for(i=0; i<n; i++) 
        cbk_dist[map[i]]+=((cbk[map[i]]-data[i])*(cbk[map[i]]-data[i]));

/* compute average codeword distortion*/
    for(k=0; k<ncbk_desired; k++)
        cbk_dist[k] /= (double)pop[k];
    if(debug_level>5)
    {
      (void)fprintf(stderr, "cbk_dist was:\n");
      for(k=0;k<ncbk_desired;k++)
          (void)fprintf(stderr," %g ", cbk_dist[k]);
      (void)fprintf(stderr, "\n");
    }

/* sort the codebook to be in increasing order */
    qsort ((char *)cbk, (unsigned)ncbk, sizeof *cbk, compar);

/* realign final_distortions and codeword populations with sorted codebook */
   for(k=0;k<ncbk_desired;k++) {
      for(i=0;i<ncbk_desired;i++){
         if(copy[i]==cbk[k] && i!=k){
            tmp=copy[i];
	    copy[i]=copy[k];
	    copy[k]=tmp;
            tmp=cbk_dist[i];
	    cbk_dist[i]=cbk_dist[k];
	    cbk_dist[k]=tmp;
            ncbk = pop[i]; /*ncbk is used for temporary storage*/
            pop[i]=pop[k];
            pop[k] = ncbk; 
    	    }
       }
  }
   if(debug_level>5)
   	{
   	      (void)fprintf(stderr, "pop is:\n");
   	      for(k=0;k<ncbk_desired;k++)
       		(void)fprintf(stderr," %d ", pop[k]);
   	      (void)fprintf(stderr, "\n");
   	}
    if(debug_level>5)
	{
              (void)fprintf(stderr, "cbk is:\n");
      	      for(k=0;k<ncbk_desired;k++)
      		  (void)fprintf(stderr," %g ", cbk[k]);
              (void)fprintf(stderr, "\n");
    	}
    if(debug_level>5)
	{
              (void)fprintf(stderr, "cbk_dist is:\n");
     	      for(k=0;k<ncbk_desired;k++)
        	(void)fprintf(stderr," %g ", cbk_dist[k]);
    	      (void)fprintf(stderr, "\n");
        }

}

