/*
  * This material contains proprietary software of Entropic Speech, Inc.
  * Any reproduction, distribution, or publication without the prior
  * written permission of Entropic Speech, Inc. is strictly prohibited.
  * Any public distribution of copies of this work authorized in writing by
  * Entropic Speech, Inc. must bear the notice
  *
  *   "Copyright (c) 1991 Entropic Speech, Inc. All rights reserved."
  *
  * Program: dtw.c
  * Written by: Bill Byrne, January 19, 1991
  * 
  * This program computes the dynamic time warping distance between
  * two sequences of floats.
  *
  */
#ifndef lint
     static char *sccs_id = "@(#)dtw_l2.c	1.2 10/19/93 ERL";
#endif

#include <stdio.h>
#include <esps/limits.h>
#include <esps/spsassert.h>

#define PROG "dtw_l2"
#define Fprintf (void)fprintf
#define Sprintf (void)sprintf

double dtw_l2(test, ref, N, M, dim, delta, bsofar, mapping, dist_fn)
/*  Find dtw distance between reference and test sequence.
    
    test         pointer to array of data test[N][dim]
    ref          pointer to array of data ref[M][dim]
    delta        allows for initial and final points of test to be compared
                 [0,delta] points of reference
    bsofar       best so far: in dtw comparisons of multiple words, search
                 can be abandoned when interim distance exceeds this value;
		 ignored if bsofar == NULL
    mapping      best alignment of ref to test found by backtracking
                 after dtw comparison finished; ignored if mapping == NULL:
		 backing not performed
    dist_fn      function used to compute distance between vectors test[i] and
                 ref[i]; if dist_fn == NULL, the euclidean distance is used

    Routine finds mapping w from [0,N-1] to [0,M-1] to minimize 
    dist_fn(test[0], ref[w(0)]) + ... + dist_fn(test[N-1], ref[w(N-1)] 
    (see dtw (3-ESPS)).

    returns double value of dtw distance 
    if debug_level is set, writes all sorts of messages
*/
long N, M, dim, delta, *mapping;
float ** test, ** ref;
double *bsofar, (*dist_fn)();
{
    extern int debug_level;
    
    struct global_boundary {
	long lower_lim;
	long upper_lim;
    };
    struct interim_distance {
	double distance; 
	long best_prev_m;
	short n_constant;
    };
    struct global_boundary * global;
    struct interim_distance ** dist;
    
    int n, m, mptr, mprevptr, BackTrack=0, l1, l2;
    int best_fault=0, best_test=0;
    double best_cost_at_n=0.0;
    long best_prev_m_at_n;
    long best_m;

    double l2_dist();
#ifndef DEC_ALPHA
    char *calloc();
#endif
    void exit();


    if (debug_level>1) {
	Fprintf(stderr, "%s: test len: %d, ref len: %d, dim: %d delta: %d\n",
		PROG, N, M, dim, delta);
    }
    spsassert( test != NULL && ref != NULL && N && M,
	      "routine called with null or zero parameters.");

    if (NULL != mapping)
	BackTrack = 1;
    if (NULL != bsofar)
	best_test = 1;

    if (debug_level>1) {
	if (BackTrack)
	  Fprintf(stderr, "%s: Backtracking set.\n", PROG);
	else
	  Fprintf(stderr, "%s: Backtracking not set.\n", PROG);
	if (best_test)
	  Fprintf(stderr, "%s: best distance so far: %e\n", PROG, *bsofar);
	else
	  Fprintf(stderr, "%s: bsofar not set.\n", PROG);
	if (dist_fn == NULL)
	    Fprintf(stderr, "%s: using euclidean distance.\n", PROG);
    }

    if (dist_fn == NULL)
	dist_fn = l2_dist;

    /* allocate memory and compute global boundaries corresponding to slopes 1/2 and 2 */
    if (debug_level>2)
	Fprintf(stderr, "%s: allocating memory and computing boundaries\n", PROG);
    global = (struct global_boundary *) calloc( (unsigned) N, sizeof(struct global_boundary));
    spsassert( global != NULL, "Can't allocate global boundaries.");
    dist = (struct interim_distance **) calloc( (unsigned) N, sizeof(struct interim_distance *));
    spsassert( dist != NULL, "Can't allocate distance.");

    for (n=0; n<N; n++) {
	/* compute global constraints for mapping w:
	   global[n].lower_lim <= w(n) <= global[n].upper_lim */
	l1 = n/2;
	l2 = (int) (M-1 - 2*(N-1-n)) - delta;
	global[n].lower_lim = ( l1 > l2 )? l1 : l2;

	l1 = 2 * n + delta;
	l2 = (int) (M-1 - 0.5*(float)(N-1-n)); 
	global[n].upper_lim = ( l1 < l2 ) ? l1 : l2;

	/* for all points in the range [ global[n].lower_lim, global[n].upper_lim] , 
	   assign an instance of the interim_distance, which is 
	   used to keep track of the dynamic programming info.  In order to save memory,
	   this is not allocated as a square array */
	dist[n] = (struct interim_distance *) 
	    calloc( (unsigned)(global[n].upper_lim - global[n].lower_lim + 1), 
		   sizeof(struct interim_distance));
	spsassert ( dist[n] != NULL, "can't allocate memory of inner distance array.");
    }

    /* perform DP match */
    if (debug_level>1)
	Fprintf(stderr, "%s: beginning dtw distance comparison.\n", PROG);
    for (m=global[0].lower_lim; m<=global[0].upper_lim; m++) {
	if (debug_level>4)
	    Fprintf(stderr, "%s: Comparing ref[%d] to test[0]\n", PROG, m);
	dist[0][m].distance = l2_dist( test[0], ref[m], dim ); 
    }
    for (n=1; n<N; n++) {
	if (debug_level>3)
	  Fprintf(stderr, "%s: record %d of %d\n", PROG, n, N);
	best_cost_at_n = DBL_MAX;
	for (m=global[n].lower_lim; m<=global[n].upper_lim; m++) {
	    
	    /* 
	      The interim cost of a path ending at (n,m), i.e. test[n] aligned
	      with ref[m] is computed in this loop.  Denote the distance accumulated 
	      along the best path which ends at (n,m) as D(n,m).  It is computed as
	      D(n,m) = dist_fn( test[n], ref[m]) + 
	                min( D(n-1,m), D(n-1,m-1), D(n-1,m-2) 
	      where the term D(n-1,m) is included in the minimization only if
	      the best path to (n-1,m) did not pass through (n-2,m).
	      These calculations are performed and info about which term achieved the
	      minimum is included for backtracking.
	      */

	    /* mptr is used to access info in arrays global and dist */
	    mptr = m-global[n].lower_lim;
	    dist[n][mptr].distance = DBL_MAX;
	    
	    /* test D(n-1, m-1) */
	    if ( (m-1 >= global[n-1].lower_lim) && (m-1 <= global[n-1].upper_lim) ) {
		if (debug_level>4)
		    Fprintf(stderr, "%s: Comparing ref[%d] to test[%d].\n", PROG, m-1, n-1);
		mprevptr = m - 1 - global[n-1].lower_lim;

		if ( dist[n][mptr].distance > dist[n-1][mprevptr].distance) {
		    dist[n][mptr].distance =  dist[n-1][mprevptr].distance;
		    dist[n][mptr].best_prev_m = m - 1;
		  }
	      }

	    /* test D( n-1, m-2 ) */
	    if ( (m-2 >= global[n-1].lower_lim) && (m-2 <= global[n-1].upper_lim) ) {
		if (debug_level>4)
		    Fprintf(stderr, "%s: Comparing ref[%d] to test[%d].\n", PROG, m-2, n-1);
		mprevptr = m - 2 - global[n-1].lower_lim;
		if ( dist[n][mptr].distance > dist[n-1][mprevptr].distance ) {
		    dist[n][mptr].distance =  dist[n-1][mprevptr].distance;
		    dist[n][mptr].best_prev_m = m - 2;
		  }
	      }

	    /* test D( n-1, m ) */
	    if ( (m >= global[n-1].lower_lim) && (m <= global[n-1].upper_lim) ) {
		if (debug_level>4)
		  Fprintf(stderr, "%s: Comparing ref[%d] to test[%d].\n", PROG, m, n-1);
		mprevptr = m - global[n-1].lower_lim;
		if ( !dist[n-1][mprevptr].n_constant ) {
		    if ( dist[n][mptr].distance > dist[n-1][mprevptr].distance) {
			dist[n][mptr].distance =  dist[n-1][mprevptr].distance;
			dist[n][mptr].n_constant = 1;
			dist[n][mptr].best_prev_m = m;
		      }
		  }
	      }

	    /* it is possible that the global limits permit points (n,m) which
	       do not have valid predecessors. It happens due to roundoff */
	    if ( DBL_MAX != dist[n][mptr].distance ) 
	      dist[n][mptr].distance += (*dist_fn)( test[n], ref[m], dim);

	    if ( best_test && dist[n][mptr].distance < best_cost_at_n) 
	      best_cost_at_n = dist[n][mptr].distance;
	  }

	if ( best_test == 1 && best_cost_at_n > *bsofar) {
	    best_fault=1;
	    if (debug_level>1)
	      Fprintf(stderr, "%s: Best previous value exceeded at record %d\n",
		      PROG, n);
	    best_cost_at_n = *bsofar;
	    break;
	  }
      }

    if (BackTrack && best_fault == 1 && debug_level)
      Fprintf(stderr, "%s: Interim distance exceeds best previous distance - no backtracking\n",
	      PROG);


    if ( !best_fault ) {
	best_cost_at_n = DBL_MAX;
	for (mptr=0; mptr<=delta; mptr++)
	  if (dist[N-1][mptr].distance < best_cost_at_n) {
	      best_cost_at_n = dist[N-1][mptr].distance;
	      best_m = mptr + global[N-1].lower_lim;
	    }

	if (BackTrack) {
	    /* load best alignment path in mapping */
	    if (debug_level>2)
	      Fprintf(stderr, "%s: Backtracking...", PROG);
	    for (n=N-1; n>=0; n--) {
		mapping[n] = best_m;
		mptr = best_m - global[n].lower_lim;
		best_m = dist[n][mptr].best_prev_m;
	      }
	    if (debug_level>1)
	      Fprintf(stderr, "%s: done\n", PROG);
	  }
      }

    /* return data */
    if (debug_level>1) {
	if (best_fault)
	  Fprintf(stderr, "%s: returning best_so_far value due to previous distance default\n", PROG);
	else
	  Fprintf(stderr, "%s: Distance: %e\n", PROG, best_cost_at_n);
      }

    /* free all boundary pointers and distance arrays */
    for (n=0; n<N; n++) 
      free(dist[n]);
    free(dist);
    free(global);
	
    return ( (double) best_cost_at_n);

  }
 /* end dpmatch */

double l2_dist (v1, v2, dim)
float *v1, *v2;
long dim;
{
  double sqrt();

  int j;
  double result, dummy;
  for (j=0, result=0.0; j<dim; j++) {
    dummy = v1[j] - v2[j];
    result += dummy*dummy;
  }
  result = sqrt( result );

  return(result);
}








