/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Written by:  John Shore 
 *  				
 * Module:	vqdesign.c
 *
 * This function designs a full-search vector quantization codebook
*/
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
#ifndef lint
static char *sccs_id = "@(#)vqdesign.c	1.15	11/14/96 ESI";
#endif
#define BIGDIST 1E20	/*large distortion value*/
#define LARGERAND 2147483646.0	/*-1 plus maximum value returned by random()*/
#define SPLIT_DITH .01	/*codeword fraction for random dither in split*/

#define Fprintf (void)fprintf
#define Fflush  (void)fflush
#define FLUSH_HST if (histrm != NULL) (void) fflush(histrm)
#define ERR_EXIT(text){(void)fprintf(stderr,"%s - exiting\n",text);exit(1);}
#define HIST(n) if ((histrm != NULL) && (hdetail >= n)) (void)fprintf
#define HISTF(n,text) {if ((histrm != NULL) && (hdetail >= n)) { \
    (void)fprintf(histrm, text); \
    (void)fflush(histrm); }}
/*
 *The following defines facilitate the handling of errors reported
 *by the four functions that are passed to vqdesign.
 */
#define CHECK_GC_ERR(err) { \
    if (err != 0) { \
	if (histrm != NULL) (void) fprintf(histrm, \
	    "vqdesign: error %d return from get_chunk\n", err);\
        return VQ_GC_ERR;  } } 

#define CHECK_VC_ERR(err) { \
    if (err != 0) { \
	if (histrm != NULL) (void) fprintf(histrm, \
	    "vqdesign: error %d return from vec_to_cdwd\n", err);\
        return VQ_VECWD_ERR;  } }

#define CHECK_DIST_ERR(err) { \
    if (err != 0) { \
	if (histrm != NULL) (void) fprintf(histrm, \
	    "vqdesign: error %d return from distort\n", err);\
        return VQ_DIST_ERR;  } }

#define CHECK_SPLIT_ERR(err) { \
    if (err != 0) { \
	if (histrm != NULL) (void) fprintf(histrm, \
	    "vqdesign: error %d return from split\n", err);\
        return VQ_SPLIT_ERR; } }
/*
 * system functions and variables
 */
#if !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *calloc();
char *malloc();
void free();
#endif
long time();
char *ctime();
double log();
/*
 * external SPS functions
 */
float **f_mat_alloc();

/*
 *other global delcarations
 */
static int nullvec2cdwd();	/*routine to use if vec_to_cdwd == NULL*/
static long nullgetchunk();	/*routine to use if get_chunk == NULL*/
static int nullcheckpoint();	/*routine to use if checkpoint == NULL*/
static void vec_add_f();	/*updates a vector sum*/
static void vec_copy_f();	/*copies a vector*/
static long findmax_f(), findmax_l(); /*find index of maximum array value*/
static void wrt_cbk_hist();	/*utility for writing history info*/
static void wrt_cbkhead();	/*utility for writing codebook header info*/
static void wrt_codewords();	/*utility for writing just codewords*/
static void wrt_clstdata();	/*utility for writing cluster sizes and distortions*/
static void wrt_entropy();	/*utility for writing codebook entropies */
static long encode();		/*encodes training sequence with a codebook*/
static void split_fill();	/*fills empty cell by splitting given cell*/
static long make_root();	/*routine to make the root codeword*/
static void cbk_split();	/*enlarges codebook by splitting codewords*/
static int gen_split();	/*generic split routine for splitting codewords*/
static void encode_init();	/*function to initialize codebooks prior to encode*/
long vqencode();   	/*function for encoding a single feature vector*/
static void wrt_fea();		/*function to write feature vectors*/
/*
 *Here's the main function VQDESIGN definition
 */
int
vqdesign(histrm, hdetail, data, len, dim, cbk, enc, init, split_crit, 
	get_chunk, vec_to_cdwd, distort, split, checkpoint, max_iter)
FILE		*histrm;	/*stream for history and debugging info*/
int		hdetail;	/*controls level of detail of histrm output*/
float		**data;	    	/*matrix of training data*/
long		len;	    /*number of rows in data*/
long		dim;	    /*number of columns in data; dimension of feature
			   vector*/
struct vqcbk	*cbk;	    /*the initial and then final codebook*/
long		*enc;	    /*length len vector of codeword indices - enc[i]
			    is the index of the closest codeword to data[i]*/
int		init;	    /*whether or not to cluster an initial codebook*/
int		split_crit;	    /*criterion for which codeword to split*/
long		(*get_chunk)();	    /*function to get more training data*/
int		(*vec_to_cdwd)();   /*function to convert feature vector to codeword*/
double		(*distort)();	/*function to compute distortion*/
int		(*split)();	/*function to split a codeword*/
int		(*checkpoint)();    /*function to call for checkpoints*/
int		max_iter;	/* max no. of iterations for one clustering*/
{
/*
 * declarations
 */
    register long i;	/*loop variable*/
    int gc_err = 0;		/*error return value from get_chunk*/
    int vc_err = 0;		/*error return value from vec_to_cdwd*/
    int split_err = 0;	/*error return value from split*/
    int dist_err = 0;	/*error status from distort*/
    short clustered;	/*flag to indicate clustering already done*/
    float **centroids;	/*each row will be a centroid feature vector*/
    long cbk_dim;   	/*dimension of codewords*/
    double avg_dist;		/*average distortion of current codebook*/
    double prev_avg_dist;	/*average distortion of codebook from previous
			   iteration*/
    int clust_iter = 0;	/*number of iterations for one codebook clustering*/
    long sind;	    /*index of codeword to be split*/
    int empty_filled = 0;    /*flag used to indicate that empty cells were
			       filled*/
    int first;		/*flag to indicate first time through inner loop*/
    long tloc;		    /*for local time*/
/*
 * initialization
 */
    HISTF(1, "vqdesign: entering function vqdesign\n");
    spsassert(cbk != NULL, "vqdesign: NULL codebook passed");
    *cbk->cbk_struct = FULL_SEARCH;
    cbk_dim = *cbk->dimen;
    *cbk->num_iter = 0;
    *cbk->num_empty = 0;
    *cbk->final_dist = BIGDIST;  /*large distortion value to start*/
    /*
     *allocate centroid matrix same size as cbk->codebook;
     *This will hold the centroids of feature vector clusters.
     */
    centroids = f_mat_alloc((unsigned) *cbk->design_size, (unsigned) cbk_dim);
    if (centroids == NULL) {
	HISTF(0, "vqdesign: not enough memory available\n");
	Fprintf(stderr, "vqdesign: not enough memory available\n");
		exit(1);
    }
    /*
     *insert suitable functions if vec_to_cdwd, split, or get_chunk are NULL
     */
    if (get_chunk == NULL) {
	HISTF(1, "vqdesign: no get_chunk supplied\n");
	get_chunk = nullgetchunk;
    }
    else HISTF(1, "vqdesign: external get_chunk supplied\n");
    if (vec_to_cdwd == NULL) {
	HISTF(1, "vqdesign: generic vec_to_cdwd being used\n");
	vec_to_cdwd = nullvec2cdwd;
	if (dim != cbk_dim) {
	    HISTF(0, "vqdesign: fatal error; (cbk_dim != dim) when vec_to_cdwd == NULL\n");
	    return VQ_INPUT_ERR;
	}
    }
    else HISTF(1, "vqdesign: external vec_to_cdwd being used\n");
    if (split == NULL) {
	HISTF(1, "vqdesign: generic split routine being used\n");
	split = gen_split;
    }
    else HISTF(1, "vqdesign: external split routine being used\n");
    if (distort == NULL) {
	/*
	 *distort is left as NULL in this case as the vqencode routine
	 *uses mean square error when passed NULL for distort.
	 */
        if (dim != cbk_dim) ERR_EXIT
	  ("feature vectors and codewords must have same dimension for mse distort");
	HISTF(1, "vqdesign: mean-square-error distortion being used\n");
    }
    else HISTF(1, "vqdesign: external distortion function being used\n");
    if (checkpoint == NULL) checkpoint = nullcheckpoint;
    else HISTF(1, "vqdesign: external checkpoint function being used\n");
/*
 *create root node if there's no initial codebook
 */
    if (*cbk->current_size == 0) {
	if (0 == make_root(histrm, hdetail, data, len, dim, cbk, enc, 
		get_chunk, &gc_err, vec_to_cdwd, &vc_err)) {
	   HISTF(0, "vqdesign: no training sequence from get_chunk\n");
	   return VQ_INPUT_ERR;
	}
	CHECK_GC_ERR(gc_err);	
	CHECK_VC_ERR(vc_err);
	init = INIT_NOCLUSTER; /*the root codebook doesn't need clustering*/
	wrt_cbk_hist(histrm, hdetail, 
	    "Root codebook created; distortion not computed.", cbk);
    }
    else 
	wrt_cbk_hist(histrm, hdetail, "Initial codebook supplied.", cbk);
    clustered = (init == INIT_NOCLUSTER) ? 1 : 0;
/*
 *at this point we have an initial codebook of size *cbk->current_size
 */
/*
 *Here is the main (outer) loop, which generates codebooks of 
 *increasing size.
 */
    while ((*cbk->current_size < *cbk->design_size) || clustered == 0) {
	if (clustered) {
	    checkpoint(cbk, CHK_SPLIT);
	    HISTF(1, "vqdesign: Splitting codebook.\n");
	    cbk_split(cbk, split_crit, split, &split_err);
	    CHECK_SPLIT_ERR(split_err);
	    spsassert((*cbk->current_size <= *cbk->design_size), 
		"vqdesign: codebook current_size corrupted");
	    if ((hdetail > 2) && (histrm != NULL)) {
		Fprintf(histrm, "vqdesign: Codebook after split:\n");
		Fprintf(histrm, "\tCurrent codebook size = %d\n", *cbk->current_size);
		wrt_codewords(histrm, cbk);
	    }
	}
    /*
     *cluster the enlarged codebook; this is the inner loop, which refines
     *codebooks of the current size.
     */
	HISTF(1, "vqdesign: Clustering codebook.\n");
	if (!empty_filled) 
	    clust_iter = 0;
	prev_avg_dist = BIGDIST;
	avg_dist = BIGDIST / 2;
	first = 1;
	while (((prev_avg_dist - avg_dist) / prev_avg_dist) > *cbk->conv_ratio){
	    spsassert(avg_dist >= 0.0, "vqdesign: negative average distortion!");
	    prev_avg_dist = avg_dist;
	/*
	 *If this is not the first time through the loop, convert the 
	 *the cluster centroids from the previous pass to codewords.  We
	 *do this here so that the stats match the codebook when the loop
	 *exits.  
	 */
	    if (!first) {
		for (i = 0; i < *cbk->current_size; i++) {
		    /*don't convert centroids from empty clusters*/
		    if (cbk->clustersize[i] != 0) {
			vec_to_cdwd(centroids[i], dim, cbk->codebook[i], cbk_dim, &vc_err);
			CHECK_VC_ERR(vc_err);
		    }
		}
		if ((hdetail > 3) && (histrm != NULL)) {
		    Fprintf(histrm, "vqdesign: new codebook after moving centroids:\n");
		    wrt_codewords(histrm, cbk);
		}
	    }
	    first = 0;  
	/*
	 *encode training sequence with current codebook
	 */
	    if (0 == encode(histrm, hdetail, data, len, dim, cbk, enc, 
		centroids, get_chunk, &gc_err, distort, &dist_err)) {
		HISTF(0, "vqdesign: no training sequence from get_chunk\n");
		return VQ_INPUT_ERR;
	    }
	    CHECK_DIST_ERR(dist_err);   
	    CHECK_GC_ERR(gc_err);
	    (*cbk->num_iter)++;
	    clust_iter++;
	    avg_dist = *cbk->final_dist;
	    checkpoint(cbk, CHK_ENCODE);
	    if ((hdetail > 1) && (histrm != NULL)) {
		Fprintf(histrm, 
		 "vqdesign: average distortion after encode = %g\n", avg_dist);
		Fflush(histrm);
	    }
	    if (hdetail > 3) {
		wrt_entropy(histrm, cbk);
    		wrt_clstdata(histrm, cbk);
	    }
	    if (avg_dist > prev_avg_dist) {
		Fprintf(stderr, 
		   "vqdesign: WARNING - average distortion increased; %g -> %g\n", prev_avg_dist, avg_dist);
		Fprintf(histrm, 
		   "vqdesign: WARNING - average distortion increased; %g -> %g\n", prev_avg_dist, avg_dist);
		Fflush(histrm);
	    }
	/*
	 *It's possible, though unlikely, that the codebook is perfect, i.e.,
	 *that the distortion is zero.  If so, now's the time to get out.
	 */
	    if (avg_dist == 0.0) {
		HIST(0)(histrm, "vqdesign: Zero-distortion codebook found.\n");
		HIST(0)(histrm, "\tcodebook size = %d\n", *cbk->current_size);
		wrt_cbk_hist(histrm, hdetail, "Final codebook:", cbk);
		FLUSH_HST;
		return VQ_OK;
	    }
	/*
	 *Here we make sure that cluster iteration limit hasn't been
         *exceeded. 
	 */
	    if (clust_iter > max_iter){
		HIST(0) (histrm,
		    "vqdesign: FAILED - too many iterations at one level (max_iter)\n");
  		HISTF(0, "vqdesign: FAILED - codebook did not converge\n");
		HIST(0)(histrm, 
		    "\tfinal average distortion = %g\n", *cbk->final_dist);
		HIST(0) (histrm, 
		    "\tcurrent codebook size = %d\n", *cbk->current_size);
	        HIST(0)(histrm,
		    "\ttotal clustering iterations = %d\n", *cbk->num_iter);
	        HIST(0)(histrm,
		    "\tmaximum permitted iterations at one level = %d\n", max_iter);
		HIST(0)(histrm, 
		    "\ttotal number empty cells found = %d\n", *cbk->num_empty);
		FLUSH_HST;
    		return VQ_NOCONVG;
	    }
	}
    /*
     *end of inner loop - now we have a reclustered codebook
     */
        clustered = 1;
	tloc = time(0);
	if (hdetail) Fprintf(histrm, "vqdesign: Codebook clustered, %s", ctime(&tloc));
        wrt_cbk_hist(histrm, hdetail, "New codebook:", cbk);
    /*
     *Now check for empty cells and deal with them.
     */
	empty_filled = 0;
	for (i = 0; i < *cbk->current_size; i++) {
	    if (cbk->clustersize[i] == 0) { /*empty cell*/
		(*cbk->num_empty)++;
		empty_filled = 1;
		HIST(2)(histrm, 
		    "vqdesign: found empty cluster %d, discarding it\n", i);
		switch (split_crit) {
		    case SPLIT_POP:
		        sind = findmax_l(cbk->clustersize, *cbk->current_size);
			break;
		    case SPLIT_DIST:
			sind = findmax_f(cbk->clusterdist, *cbk->current_size);
		    	break;
		}
		HIST(2)(histrm, 
		    "vqdesign: splitting cluster %d (old index)\n", sind);
		FLUSH_HST;
		split_fill(cbk, i, sind, split, &split_err);
		CHECK_SPLIT_ERR(split_err);
		/*It's possible that there were two empty clusters in a
		 *row; if so, the second one now has index i equal to
		 *the one that was just split; hence we need to decrement
		 *i in case this happened (no harm if it didn't happen)
		 */
		i--;
		if (hdetail >= 3) 
		    wrt_cbk_hist(histrm, hdetail, 
			"Codebook after filling empty cell:", cbk);
	    }
	}
	if (empty_filled) {
	    HISTF(1, "vqdesign: finished filling empty cells\n");
	    wrt_cbk_hist(histrm, hdetail, "Codebook with filled cells:", cbk);
	    /*
	     *Filling an empty cell changes the codebook.  But the
	     *overall size (*cbk->current_size) was not changed
	     *by this operation, so we need to recluster before enlarging
	     *the codebook - i.e., execute inner loop again.  
	     */
	    clustered = 0;
	}
    }
    /*
     *This is the end of the outer loop, which generates codebooks of
     *of increasing size.  If we complete this loop successfully,  a 
     *codebook was designed properly. 
     */
    HIST(1)(histrm,"vqdesign: codebook converged.\n");
    wrt_cbk_hist(histrm, hdetail, "Final codebook:", cbk);
    FLUSH_HST;
    free((char *) centroids);
    return VQ_OK;
}
static
long
make_root(histrm, hdetail, data, d_len, d_dim, cdbk, d_enc, get_chunk, 
    gch_err,  vec_to_cdwd, vc_err)
FILE		*histrm;	    /*stream for history*/
int		hdetail;	    /*level of history detail*/
float		**data;		    /*training sequence*/
long	d_len;		    /*rows in data*/
long	d_dim;		    /*cols in data*/
struct vqcbk	*cdbk;		    /*the codebook -- put root in here*/
long	*d_enc;		    /*codword indices corresponding to *data*/
long	(*get_chunk)();	    /*routine to get chunk from training
				       seq.*/
int		*gch_err;	    /*used to pass errors from get_chunk*/
int		(*vec_to_cdwd)();   /*routine to convert fea to codeword*/
int		*vc_err;	    /*used to pass errors from vec_to_cdwd*/
/*
 *This function makes a root codebook by averaging the entire training
 *sequence and then converting the centroid to a codeword.  The function
 *returns the length of the training sequence.  
 */
{
    long nsofar = 0;	/*length of training sequence*/
    long new;	/*number of new training vectors from get_chunk*/
    float *centr;	/*to hold centroid of training sequence*/
    register long i, j;
    /*
     *initialization
     */
    for (i = 0; i < *cdbk->design_size; i++) { 
	cdbk->clustersize[i] = 0;
	cdbk->clusterdist[i] = 0.0;
        for (j = 0; j < *cdbk->dimen; j++) cdbk->codebook[i][j] = 0.0;
    }    
    centr = (float *) calloc((unsigned) *cdbk->dimen, sizeof (float));
    if (centr == NULL) {
	Fprintf(histrm, "make_root: not enough memory available for root\n");
	Fprintf(stderr, "make_root: not enough memory available for root\n");
	exit(1);
    }
    nsofar = 0;
    for (i = 0; i < d_len; i++) d_enc[i] = 0;
    /*
     *compute cumulative element-by-element sum of feature vectors
     */
    new = get_chunk(data, d_len, d_dim, 0, gch_err);
    HIST(5)(histrm, "make_root: get_chunk returns %d feature vectors\n", new);
    while (new != 0) {
	for (i = 0; i < new; i++) vec_add_f(centr, data[i], d_dim);
	nsofar += new;
	new = get_chunk(data, d_len, d_dim, nsofar, gch_err);
        HIST(5)(histrm, "make_root: get_chunk returns %d feature vectors\n", new);
    }
    /*
     *complete the averaging and convert to a codeword
     */
    if (nsofar != 0) for (i = 0; i < d_dim; i++) centr[i] = (centr[i])/ nsofar;
    vec_to_cdwd(centr, d_dim, cdbk->codebook[0], *cdbk->dimen, vc_err);
    /*
     *complete codebook initialization
     */
    *cdbk->num_iter = 1;
    *cdbk->current_size = 1;
    *cdbk->train_length = nsofar;
    free((char *)centr);
    return nsofar;
}

static void
cbk_split(cbk, split_type, splitfunc, error)
struct vqcbk *cbk;
int split_type;
int (*splitfunc)();
int *error;
/*
 *This is a generic codebook split function. It splits each codeword
 *by adding or subracting a random amount from each codeword elemant.  
 *The amount is uniformly distributed over a fixed percentage of 
 *the particular codeword element.
 */
{
    int i;
    static first = 1;		    /*flag for first time through*/
    static float **newcodebook;	    /*storage for new codewords*/
    long n_tosplit;	    /*number of codewords to split*/
    long n_index;	    /*index for new codewords*/
    long s_ind;		    /*codeword index*/    
    static long *split_ind;	/*array to indicate which codewords to split*/
    if (first) {
	first = 0;
	newcodebook = f_mat_alloc((unsigned) *cbk->design_size, 
	    (unsigned) *cbk->dimen);
	split_ind = (long *) malloc((unsigned) *cbk->design_size * sizeof (long));
	if ((newcodebook == NULL) || (split_ind == NULL)) 
	    ERR_EXIT("cbk_split: couldn't allocate enough memory"); 
    }
    /*initialize array of split indicators (1 ==> split this one)
     */
    for (i = 0; i < *cbk->design_size; i++) split_ind[i] = 0;
    /*
     *figure out how many codewords to split
     */
    if ((2 * *cbk->current_size) > *cbk->design_size)
	n_tosplit = *cbk->design_size - *cbk->current_size;
    else 
	n_tosplit = *cbk->current_size;
    /*
     *select which codewords to split
     */
    for (i = 0; i < n_tosplit; i++) {
	switch (split_type) {
	    case SPLIT_POP:
		s_ind = findmax_l(cbk->clustersize, *cbk->current_size);
		cbk->clustersize[s_ind] = 0; /*to prevent further selection*/
		break;
	    case SPLIT_DIST:
		s_ind = findmax_f(cbk->clusterdist, *cbk->current_size);
		cbk->clusterdist[s_ind] = -1.0; /*to prevent further selection*/
		break;
	}
	split_ind[s_ind] = 1;
    }
    /*
     *now copy or split all of the codwords 
     */
    n_index = 0;
    for (i = 0; i < *cbk->current_size; i++) {
	if (split_ind[i] == 1) {
	    vec_copy_f(newcodebook[n_index], cbk->codebook[i], *cbk->dimen);
	    n_index++;
	    splitfunc(newcodebook[n_index], newcodebook[n_index-1], *cbk->dimen);
	}
	else 
	    vec_copy_f(newcodebook[n_index], cbk->codebook[i], *cbk->dimen);
	n_index++;
    }
    /*
     *done with split; copy back to codebook and update current size
     */
    *cbk->current_size += n_tosplit;
    for (i = 0; i < *cbk->current_size; i++) 
	vec_copy_f(cbk->codebook[i], newcodebook[i], *cbk->dimen);
    *error = 0;
}

static long
encode(histrm, hdetail, data, d_len, d_dim, cdbk, d_enc, centr, 
	get_chunk, gch_err, distort, distr_err)
FILE		*histrm;	    /*stream for history*/
int		hdetail;		    /*history detail level*/
float		**data;		    /*feature vector training sequence*/
long	d_len;		    /*rows in data*/
long	d_dim;		    /*cols in data*/
struct vqcbk	*cdbk;		    /*the current codebook*/
long	*d_enc;		    /*codword indices corresponding to *data*/
float		**centr;	    /*matrix for cluster centroids*/
long	(*get_chunk)();	    /*routine to get more training data*/
int		*gch_err;	    /*used for errors from get_chunk*/
double		(*distort)();	    /*routine to compute distortion*/
int		*distr_err;	    /*used for errors from distort*/
{
/*This routine encodes an entire training sequence with respect to 
 *a given codebook.  It fills in clustersize, clusterdist, final_dist,
 *and train_length fields in the passed codebook.  It also fills in centr
 *with the cluster centroids produced by the encoding.  
 */
    register long i, j;	/*loop counters*/
    long new_nfea;	/*number of feature vectors from get_chunk*/
    long tot_nfea = 0;		/*total number of feature vectors processed*/
    long min_index;	/*index of min. distortion codeword*/    
    double mindist;	/*temp distortion values*/
    /*
     *Initialization
     */
    spsassert(get_chunk != NULL, "encode: null pointer for get_chunk");
    encode_init(cdbk, centr, d_dim);
    new_nfea = get_chunk(data, d_len, d_dim, 0, gch_err);
    HIST(5)(histrm, "encode: get_chunk returns %d feature vectors\n", new_nfea);
    if(*gch_err != 0) return new_nfea;
    /*
     *This is the outer loop; it steps through every chunk of feature vectors
     */
    while (new_nfea != 0) {
	/*
	 *This inner loop steps through individual feature vectors
	 */
        for (i = 0; i < new_nfea; i++) {
	    /*
	     *find the closest codeword in the codebook
	     */
	    min_index = vqencode(data[i], d_dim, cdbk->codebook, 
	      *cdbk->current_size,  *cdbk->dimen, &mindist, distort, distr_err);
	    d_enc[i] = min_index;
	    if (*distr_err != 0) return (tot_nfea + i);
	    if (hdetail > 5) {
		wrt_fea(histrm, data[i], d_dim, (long) (tot_nfea + i + 1));
		Fprintf(histrm, 
		    "\tindex = %d, distortion = %g\n", min_index, mindist);
	    }
	    /*
	     *Given the closest codeword (which identifies the cluster for 
	     *feature vector i), update the cluster centroids and average
	     *cluster distortions.
	     */
	    vec_add_f(centr[min_index], data[i], d_dim);
	    cdbk->clustersize[min_index]++;
	    cdbk->clusterdist[min_index] += mindist;
	}
	/*
	 *At this point we have to get the next chunk of feature vectors
	 */
        tot_nfea += new_nfea;
        new_nfea = get_chunk(data, d_len, d_dim, tot_nfea, gch_err);
        HIST(5)(histrm, "encode: get_chunk returns %d feature vectors\n", new_nfea);
	if(*gch_err != 0) return tot_nfea;
    }
    /*
     *Having processed all feature vectors, we now complete the
     *averaging for centroids and distortions.
     */
    *cdbk->train_length = tot_nfea;
    for (i = 0; i < *cdbk->current_size; i++) {
	*cdbk->final_dist += cdbk->clusterdist[i];
	if (cdbk->clustersize[i] != 0) { /*remember to bypass empty clusters*/
	    for (j = 0; j < d_dim; j++) 
		centr[i][j] = (centr[i][j])/ cdbk->clustersize[i];
	    cdbk->clusterdist[i] /= cdbk->clustersize[i];
	}
    }
    /*
     *Finally, compute the overall average distortion.
     */
    *cdbk->final_dist /= *cdbk->train_length;
    return tot_nfea;
}   


static void
encode_init(cdbk, centr, fea_dim)
struct vqcbk *cdbk;
float **centr;
long fea_dim;
/*
 *This routine initializes various elements of a vqdbk prior to 
 *an encode operation.  In particular, the final distortion, cluster 
 *distortions, cluster sizes, and cluster centroids are all zeroed.  
 */
{
    int i, j;
    for (i = 0; i < *cdbk->current_size; i++) { 
	cdbk->clustersize[i] = 0;
	cdbk->clusterdist[i] = 0.0;
        for (j = 0; j < fea_dim; j++) centr[i][j] = 0.0;
     }
    *cdbk->final_dist = 0.0;
}

static int
gen_split(cdwd_dest, cdwd_src, cdwd_size)
float		*cdwd_dest;	/*codeword resulting from split*/
float		*cdwd_src;	/*codeword to be split*/
long	cdwd_size;	/*dimension of codewords*/
/*
 *This "generic split" routine is used to split a codeword whenever NULL
 *is passed as the split parameter to vqdesign.  It splits each codeword
 *by adding or subracting a random amount from each codeword element.  
 *The amount is uniformly distributed over a fixed percentage of 
 *the particular codeword element.  This split algorithm is used 
 *to compute cdwd_dest from cdwd_src and also to modify cdwd_src.  
 */
{
    static float rnd_fact = 2 * SPLIT_DITH / LARGERAND;
    long i;
    for (i = 0; i < cdwd_size; i++) {
        cdwd_dest[i]  = 
#ifdef OS5
	    cdwd_src[i] * (1 + (SPLIT_DITH - rnd_fact * rand()));
#else
	    cdwd_src[i] * (1 + (SPLIT_DITH - rnd_fact * random()));
#endif
        cdwd_src[i]  = 
#ifdef OS5
	    cdwd_src[i] * (1 +(SPLIT_DITH - rnd_fact * rand()));
#else
	    cdwd_src[i] * (1 +(SPLIT_DITH - rnd_fact * random()));
#endif
    }
}

static void
split_fill(cdbk, e_ind, s_ind, split, error)
struct vqcbk *cdbk;	/*main codebook*/
long e_ind;	/*index of empty codeword*/
long s_ind;	/*index of codeword that is to be split*/
int *error;		/*used to pass back error status from split*/
int (*split)();		/*the split function*/
/*
 *This function splits a given codeword and replaces that codeword
 *and a given, zero-cluster codeword with the results of the split.
 *The codebook is shuffled around so that the newly-filled cell
 *is adjacent to the codeword that is split.  
 */
{
    long i;
    /*
     *First remove the empty codeword from the codebook and push the 
     *ones below it up.  (This leaves a missing codeword at the end.)
     */
    for (i = e_ind + 1; i < *cdbk->current_size; i++) {
	    vec_copy_f(cdbk->codebook[i-1], cdbk->codebook[i], *cdbk->dimen);    
	    cdbk->clustersize[i-1] = cdbk->clustersize[i];
	    cdbk->clusterdist[i-1] = cdbk->clusterdist[i];
    }
    /*If the codeword to be split was moved up, decrement its index.
     */
    if (e_ind < s_ind) s_ind--;
    /*
     *Next, insert an empty codeword right after the one that will be split
     */
    for (i = *cdbk->current_size - 2; i > s_ind; i--) {
	    vec_copy_f(cdbk->codebook[i+1], cdbk->codebook[i], *cdbk->dimen);    
	    cdbk->clustersize[i+1] = cdbk->clustersize[i];
	    cdbk->clusterdist[i+1] = cdbk->clusterdist[i];
    }
    /*Now split the selected codeword
     */
    split(cdbk->codebook[s_ind + 1], cdbk->codebook[s_ind], *cdbk->dimen);
    /*
     *make sure that the codeword that was split is not selected
     *for splitting again in case there are additional empty cells
     */
    cdbk->clustersize[s_ind] = cdbk->clustersize[s_ind + 1] = 1; 
		    /*bit of a hack; can't say zero 
			     cause this cluster could be split again*/
    cdbk->clusterdist[s_ind] = cdbk->clusterdist[s_ind + 1] = 0.0;
    *error = 0;
}

static void
vec_copy_f(dst, src, n)
float  *dst, *src;	/*pointers to destination and source vectors*/
long n;		/*size of vectors*/
/*
 *This routine copies src to dst.
 */
{
    while (n-- != 0)
	*dst++ = *src++;
}

static long
findmax_f(vec, n)
float *vec;		/*vector of interest*/ 
long n;		/*size of vec*/
/*
 *This routine returns the index of the largest element in a given
 *vector of floating point values.
 */
{
    long maxind = 0;
    float maxval = 0.0;
    register long i;
    for (i = 0; i < n; i++)
	if (vec[i] >= maxval) {
	    maxval = vec[i];
	    maxind = i;
	}
    return maxind;
}

static long
findmax_l(vec, n)
long *vec;		/*vector of interest*/
long n;		/*size of vector*/
/*
 *This routine returns the index of the largest element in a given
 *vector of floating point values.
 */
{
    long maxind = 0;
    long maxval = 0;
    register long i;
    for (i = 0; i < n; i++)
	if (vec[i] > maxval) {
	    maxval = vec[i];
	    maxind = i;
	}
    return maxind;
}

static void
vec_add_f(vec1, vec2, size)
float *vec1, *vec2;
long size;
/*This function does a vector addition setting vec1 = vec1 + vec2.*/
{
    while (size-- != 0)
	*vec1++ += *vec2++;
}

static int
nullvec2cdwd(fea_vector, vec_dim, codeword, cdwd_dim, error)
float *fea_vector;
long vec_dim;
float *codeword;
long cdwd_dim;
int *error;
/*this funtion is a "straight-through" conversion from feature 
 *vectors to codewords; That is,  it just copies a feature vector 
 *to a codeword.  It is used when the vqdesign parameter 
 *vec_to_cdwd == NULL; vec_dim == cdwd_dim holds;
 */
{
    *error = 0;
    vec_copy_f(codeword, fea_vector, vec_dim);
}

static long
nullgetchunk(data, len, dim, num_prev, error)
float **data;
long len;
long dim;
long num_prev;
int *error;
/*This function is used for get_chunk when get_chunk == NULL. 
 *In this case, all of the data has already been passed, and the
 *only thing needed is to return a 0 for the number of new 
 *feature vectors.
 */
{
    *error = 0;
    if (num_prev == 0) 
	return len;
    else 
	return 0;
}

static int
nullcheckpoint(cdbk, chk_type)
struct vqcbk *cdbk;
int chk_type;
/*
 *This function is used for checkpoint when checkpoint == NULL.
 *It does absolutely nothing.
 */
{
}

static void
wrt_cbk_hist(strm, detail, msg, cbk)
FILE *strm;
int detail;
char *msg;
struct vqcbk *cbk;
/*
 *This function is used to write a message and assorted codebook 
 *information to a file; it's used for history file outputs.
 */
{
    if (strm == NULL) return;
    if (detail > 0) {
	Fprintf(strm, "vqdesign: %s\n", msg);
	wrt_cbkhead(strm, cbk);
    }
    if (detail > 2) wrt_clstdata(strm, cbk);
    if (detail > 2) wrt_codewords(strm, cbk);
    Fflush(strm);
}

static void
wrt_cbkhead(strm, cdbk)
FILE *strm;
struct vqcbk *cdbk;
{
    Fprintf(strm, "\tCurrent codebook size = %d\n", *cdbk->current_size);
    Fprintf(strm, "\tSize of training sequence = %d\n",*cdbk->train_length);
    Fprintf(strm, "\tTotal number of cluster iterations = %d\n", *cdbk->num_iter);
    Fprintf(strm, "\tCurrent average distortion = %g\n", *cdbk->final_dist);
    Fprintf(strm, "\tNumber of times empty cell found = %d\n", *cdbk->num_empty);
    wrt_entropy(strm, cdbk);
     }

static void 
wrt_clstdata(strm, cdbk)
FILE *strm;
struct vqcbk *cdbk;
{
    long i;
    Fprintf(strm, "\tCurrent cluster sizes and distortions:\n");
    Fprintf(strm, "\t\tIndex\tSize\tDistortion\n");
    for (i = 0; i < *cdbk->current_size; i++) Fprintf(strm, 
        "\t\t%d\t%d\t%g\n", i, cdbk->clustersize[i], cdbk->clusterdist[i]);
}

static void
wrt_codewords(strm, cdbk)
FILE *strm;
struct vqcbk *cdbk;
{
    long i, j;
    Fprintf(strm, "\tCurrent codewords:\n");
    for (i = 0; i < *cdbk->current_size; i++) {
        for (j = 0; j < *cdbk->dimen; j++) Fprintf(strm, "\t%f ", cdbk->codebook[i][j]);
        Fprintf(strm, "\n");
    }
}

static void
wrt_fea(strm, fea_vec, size, number)
FILE *strm;		/*file stream to use*/
float *fea_vec;		/*feature vector to print*/
long size;	/*dimension of feature vector*/
long number;		/*number of fea_vec in training sequence*/
/*
 *utility function for printing a feature vector*/
{
    long i;
    Fprintf(strm, "\tencoded input vector %d:\n", number);
    for (i = 0; i < size; i++) Fprintf(strm, "\t%f ", fea_vec[i]);
    Fprintf(strm, "\n");
}

static void
wrt_entropy(strm, cdbk)
FILE *strm;
struct vqcbk *cdbk;
{
/* compute and print the codebook entropy and the 
 * the maximum noise entropy (assuming MSE distortion)
 */
    float cbk_ent = 0.0;	/* entropy of current codebook */
    double log2 = log(2.0);	/* needed for base 2 logs */
    double prob;		/* probability of cluster */
    int i;
    if (*cdbk->train_length <= 0) return;
    for (i = 0; i < *cdbk->current_size; i++) {
	if (cdbk->clustersize[i] > 0.0) {
	    prob = (double)cdbk->clustersize[i] / (double) *cdbk->train_length;
	    cbk_ent +=  - prob * log(prob) / log2;
	}
    }
    Fprintf(strm, "\tEntropy of current codebook = %f\n", cbk_ent);
}
