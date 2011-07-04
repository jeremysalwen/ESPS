/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by: David Burton 9/12/96
 *
 * Brief description: filtering utilities for block_filter2()
 *
 */

static char *sccs_id = "@(#)block_futil.c	1.8	9/12/96	ERL";

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/feafilt.h>

extern int debug_level;
char *arr_alloc();

static int  cplxcompare();
static void pair();

/*  
 * initialize the fdata structure, does all allocation. initialize
 *  and compute data values for the appropriate filter architecture 
 */

struct fdata *
init_fdata( type, filtrec, fh, cplx_sig, cplx_fil)
     int type;
     struct feafilt *filtrec;
     struct header *fh;
     int cplx_sig, cplx_fil;
{
  struct fdata *frec;

  frec = (struct fdata *) malloc (sizeof(struct fdata));
  spsassert(frec,"init_fdata: frec malloc failed");

  switch( type ){
  case IIR_FILTERING:

/* For numerical stability, IIR filter is implemented by cascading
   2nd structures, instead of direct implementation.
   This is for numerical stability.  See Jackson (1986).  Poles are 
   ordered from the most peaked (closet to unit circle) to the least
   peaked.  This is done by qsort(3).  Each pole is paired up with
   a zero closet to it.  Each 2nd order IIR is a direct form II
   implementation. 
   Now the function does not support complex filter.  To support,
   need complex a0, a1, b0, b1 and different qsort */
    
    {
      struct iirfilt *iirdata;
      double_cplx *poles, *zeros;
      long p_siz, z_siz;
      short *sdp, *sdz;          /* single or double pole/zero flag array, 0
				  means double, 1 means single pole/zero */
      int i;
      int no_pz_flag = 0;
      
      if(debug_level>1) Fprintf(stderr,"initializing IIR filter\n\n");

      frec->filtertype = IIR_FILTERING;

      iirdata = (struct iirfilt *) malloc(sizeof(struct iirfilt));
      spsassert(iirdata,"init_fdata: iirdata malloc failed");

      if( *filtrec->num_size >= 1)
	iirdata->gain_factor = (double) filtrec->re_num_coeff[0];
      else{
	Fprintf(stderr,"init_fdata: re_num_coeff[0] does not exist for the overall gain of pole/zero system -- exiting\n");
	exit(1);
      }
      if(debug_level > 1)
	Fprintf(stderr, "init_fdata: gain_iir is re_num_coeff[0]: %e\n",
		filtrec->re_num_coeff[0]);

      iirdata->arma = filtrec;
      if( !filtrec->poles ) no_pz_flag = 1;

      if( no_pz_flag ){                   /* direct implementation */

	  /* Pure autoregressive filters try to allocate 0 bytes for saving
	     numerator state information. The memory is never used, so it
	     is not strictly an error. Some OSs complain when you try
	     to do this, however, so we check and avoid this allocation.*/

	if( cplx_sig ){
	    if (*filtrec->num_size != 1) {
	      iirdata->xstatec = (double_cplx *) 
		 calloc( *filtrec->num_size - 1, sizeof(double_cplx));
	      spsassert(iirdata->xstatec, "init_fdata: xstatec calloc fails");
	    }
	    iirdata->ystatec = (double_cplx *) 
	      calloc( *filtrec->denom_size - 1, sizeof(double_cplx));
	    spsassert(iirdata->ystatec, "init_fdata: ystatec calloc fails");
	}
	else{
	    if (*filtrec->num_size != 1) {
	      iirdata->xstate = (double *) 
		calloc( *filtrec->num_size - 1, sizeof(double));
	      spsassert(iirdata->xstate, "init_fdata: xstate calloc fails");
	    }
	    iirdata->ystate = (double *) calloc( *filtrec->denom_size -1,
					      sizeof(double));
	    spsassert(iirdata->ystate, "init_fdata: ystate calloc fails");
	}
      }
      else{                               /* numerical stable implemenation */
	poles = iirdata->arma->poles;
	zeros = iirdata->arma->zeros;
	p_siz = *iirdata->arma->pole_dim;
	z_siz = *iirdata->arma->zero_dim;

	sdz = (short *) calloc( (unsigned)z_siz, sizeof(short));
	spsassert(sdz, "init_fdata: sdz calloc failed");
	sdp = (short *) calloc( (unsigned)p_siz, sizeof(short));
	spsassert(sdz, "init_fdata: sdp calloc failed");
	/* order and pair poles and zeros */
	/* sorting poles from the most peaked to the least peaked */
	(void ) qsort( poles, p_siz, sizeof(double_cplx), cplxcompare);
	/* sorting zeros so that same real zeros are clustered */
	(void ) qsort( zeros, z_siz, sizeof(double_cplx), cplxcompare);
	(void ) pair( zeros, poles, &z_siz, &p_siz, sdz, sdp );
	iirdata->stages = MAX(p_siz, z_siz);

	if (debug_level>1){
	  Fprintf(stderr,"(0/1) for double/single root\n");
	  for(i = 0; i < iirdata->stages; i++){
	    Fprintf(stderr,"pair %d:\n",i);
	    if( i<p_siz)
	      Fprintf(stderr, "pole:[%f, %f] (%d), ",
		      poles[i].real, poles[i].imag, sdp[i]);
	    if( i<z_siz)
	      Fprintf(stderr, "zero:[%f, %f] (%d)\n", 
		      zeros[i].real, zeros[i].imag, sdz[i]);
	  }
	}	
	/* allocate IIR data structure */
	if( cplx_sig ){                    /* complex signal */
	  iirdata->w0c = (double_cplx *) calloc (iirdata->stages, 
						 sizeof(double_cplx));
	  spsassert(iirdata->w0c, "init_fdata, w0c calloc fails");
	  iirdata->w1c = (double_cplx *) calloc (iirdata->stages, 
						 sizeof(double_cplx));
	  spsassert(iirdata->w1c, "init_fdata, w1c calloc fails");
	  iirdata->w2c = (double_cplx *) calloc (iirdata->stages, 
						 sizeof(double_cplx));
	  spsassert(iirdata->w2c, "init_fdata, w2c calloc fails");
	  iirdata->sc = (double_cplx *) calloc (1+iirdata->stages, 
						sizeof(double_cplx));
	  spsassert(iirdata->sc, "init_fdata, sc calloc fails");
	}
	else{                                /* real signal */
	  iirdata->w0 = (double *) calloc(iirdata->stages, sizeof(double));
	  spsassert(iirdata->w0, "init_fdata, w0 calloc fails");
	  iirdata->w1 = (double *) calloc(iirdata->stages, sizeof(double));
	  spsassert(iirdata->w1, "init_fdata, w1 calloc fails");
	  iirdata->w2 = (double *) calloc(iirdata->stages, sizeof(double));
	  spsassert(iirdata->w2, "init_fdata, w2 calloc fails");
	  iirdata->s = (double *) calloc (1+iirdata->stages, sizeof(double));
	  spsassert(iirdata->s, "init_fdata, s calloc fails");
	}
	iirdata->a0 = (double *) calloc(iirdata->stages, sizeof(double));
	spsassert(iirdata->a0, "init_fdata: a0 calloc fails");
	iirdata->a1 = (double *) calloc(iirdata->stages, sizeof(double));
	spsassert(iirdata->a1, "init_fdata: a1 calloc fails");
	iirdata->b0 = (double *) calloc(iirdata->stages, sizeof(double));
	spsassert(iirdata->b0, "init_fdata: b0 calloc fails");
	iirdata->b1 = (double *) calloc(iirdata->stages, sizeof(double));
	spsassert(iirdata->b1, "init_fdata: b1 calloc fails");

	/* compute 2nd order IIR coefficients for each stage */
	/* for now, only real coefficients are supported */
	for( i=0; i<iirdata->stages; i++){
	  if( i < z_siz ){
	    if(sdz[i] == 0){          /* a double zero, maybe cplx or real */
	      iirdata->b0[i] = -2 * zeros[i].real;
	      iirdata->b1[i] = (zeros[i].real * zeros[i].real +
				zeros[i].imag * zeros[i].imag);
	    }
	    else iirdata->b0[i] = - zeros[i].real;  /* single zero, is real */
	  }
	  if( i < p_siz ){
	    if(sdp[i] == 0){          /* a double pole */
	      iirdata->a0[i] = 2 * poles[i].real;
	      iirdata->a1[i] = - (poles[i].real *poles[i].real +
				  poles[i].imag *poles[i].imag);
	    }
	    else iirdata->a0[i] = poles[i].real;  /* single pole */
	  }
	  if(debug_level > 1){
	    Fprintf(stderr, "stage %d a0 %f a1 %f\n", i, iirdata->a0[i],
		    iirdata->a1[i]);
	    Fprintf(stderr, "stage %d b0 %f b1 %f\n", i, iirdata->b0[i],
		    iirdata->b1[i]);
	  }
	}
      }
      frec->arch = (void *) iirdata;
      break;
    }
  case FIR_FILTERING:
    {
      struct firfilt *firdata;
      int state_siz;
      
      if(debug_level>1) Fprintf(stderr,"initializing FIR filter\n\n");

      frec->filtertype = FIR_FILTERING;

      firdata = (struct firfilt *) malloc(sizeof(struct firfilt));
      spsassert(firdata,"init_fdata: firdata malloc failed");

      firdata->arma = filtrec;

      state_siz = (*filtrec->num_size) - 1;
      if (state_siz != 0) {
	  firdata->state = (double *) calloc(state_siz, sizeof(double));
	  spsassert(firdata->state,"init_fdata: state calloc failed");
	  firdata->statec = (double_cplx *) 
	    calloc(state_siz, sizeof(double_cplx));
	  spsassert(firdata->statec,"init_fdata: statec calloc failed");
      }
      frec->arch = (void *) firdata;
      break;
    }
  default:
    {
      Fprintf(stderr,"filutil: unknown filter type\n");
      exit(1);
      break;
    }
  }
  return(frec);
}



int iirfunc( nsamp, inptr, outptr, inptrc, outptrc, arpt)
     long nsamp;
     double *inptr, *outptr;
     double_cplx *inptrc, *outptrc;
     struct iirfilt *arpt;
{
  int datacode = 0;           /* 0: r/d,r/c.  1:c/d,r/c.  10:r/d,c/c.
				11: c/d,c/c */
  if( inptr == NULL ) datacode = 1;
  
  if( !arpt->arma->poles ){            /* direct filter implementaion */
    if( datacode ==  0 ){                  /* real signal data */
      register double *num, *den;
      register double sumx, sumy;
      register long i,j;
      long nstate, k;
      
      nstate = *arpt->arma->num_size-1;
      num = arpt->arma->re_num_coeff;
      den = arpt->arma->re_denom_coeff;
      if( den[0] == 0){
	Fprintf(stderr,"ERROR in iirfunc: denominator[0] = 0\n");
	return(ERR);
      }
      for ( i=0; i<nsamp; i++){
	sumx = 0;
	sumy = 0;
	for( j=0; j<*arpt->arma->num_size; j++){	    
	  if( (k=i-j) < 0 ) sumx += num[j] * arpt->xstate[-k-1];
	  else sumx += num[j] * inptr[k];
	}
	for( j=1; j<*arpt->arma->denom_size; j++)
	  sumy += den[j] * arpt->ystate[j-1];
	outptr[i] = (sumx - sumy)/den[0];
	
	/* prepare initial states for the next time */
	for( j= *arpt->arma->denom_size-2; j>=1;  j--)
	  arpt->ystate[j] = arpt->ystate[j-1];
	arpt->ystate[0] = outptr[i];
      }
      for( j=0, k=nsamp-1; j< nstate; j++)
	arpt->xstate[j] = inptr[k-j];
      
    }
    else{                                /*direct filter implementation */
      register double *num, *den;        /* complex signal data */
      register double_cplx sumxc, sumyc, samplec;
      register long i,j;
      long nstate,  k;
      
      nstate = *arpt->arma->num_size-1;
      num = arpt->arma->re_num_coeff;
      den = arpt->arma->re_denom_coeff;
      if( den[0] == 0){
	Fprintf(stderr,"ERROR in iirfunc: denominator[0] = 0\n");
	return(ERR);
      }
      for ( i=0; i<nsamp; i++){
	sumxc.real = 0;
	sumxc.imag = 0;
	sumyc.real = 0;
	sumyc.imag = 0;
	for( j=0; j<*arpt->arma->num_size; j++){
	  if( (k=i-j) < 0 ) samplec = arpt->xstatec[-k-1];
	  else samplec = inptrc[k];
	  sumxc.real += samplec.real * num[j];
	  sumxc.imag += samplec.imag * num[j]; 
	}
	for( j=1; j<*arpt->arma->denom_size; j++){
	  sumyc.real += arpt->ystatec[j-1].real * den[j];
	  sumyc.imag += arpt->ystatec[j-1].imag * den[j];
	}
	outptrc[i].real = (sumxc.real- sumyc.real) / den[0];
	outptrc[i].imag = (sumxc.imag- sumyc.imag) / den[0];
	
	/* prepare initial states for the next time */
	for( j= *arpt->arma->denom_size-2; j>=1;  j--)
	  arpt->ystatec[j] = arpt->ystatec[j-1];
	arpt->ystatec[0] = outptrc[i];
      }
      for( j=0,k=nsamp-1; j<*arpt->arma->num_size - 1; j++)
	arpt->xstatec[j] = inptrc[k-j];
    }
  }
  else{                            /* numerically stable implementaion*/
    register double *a0, *a1, *b0, *b1, *w0, *w1, *w2, *s;
    register double_cplx *w0c, *w1c, *w2c, *sc;
    double scale;
    int stages;

    stages = arpt->stages;
    scale = arpt->gain_factor;
    a0 = arpt->a0;
    a1 = arpt->a1;
    b0 = arpt->b0;
    b1 = arpt->b1;
    if( datacode == 0 ){
      w0 = arpt->w0;
      w1 = arpt->w1;
      w2 = arpt->w2;
      s = arpt->s;
    }
    else{
      w0c = arpt->w0c;
      w1c = arpt->w1c;
      w2c = arpt->w2c;
      sc = arpt->sc;
    }
    
    if( datacode == 0 ){                /* real input data */
      register long i,j,k;
      
      for( i=0; i<nsamp; i++){
	s[0] = inptr[i];
	for( k=0; k<stages; k++){
	  w0[k] = a0[k] * w1[k] + a1[k] * w2[k] + s[k];
	  s[k+1] = w0[k] + b0[k] * w1[k] + b1[k] * w2[k];
	}
	for (j=0; j<stages; j++) w2[j] = w1[j];
	for (j=0; j<stages; j++) w1[j] = w0[j];
	outptr[i] = scale * s[stages];
      }
    }
    else{                               /* compolex input data */
      register long i,j,k;
      
      for( i=0; i<nsamp; i++){
	sc[0] = inptrc[i];
	for( k=0; k<stages; k++){
	  w0c[k].real = w1c[k].real * a0[k]+ w2c[k].real * a1[k] + sc[k].real;
	  w0c[k].imag = w1c[k].imag * a0[k]+ w2c[k].imag * a1[k]+ sc[k].imag;
	  sc[k+1].real = w0c[k].real + w1c[k].real * b0[k] +w2c[k].real*b1[k];
	  sc[k+1].imag = w0c[k].imag + w1c[k].imag * b0[k] +w2c[k].imag*b1[k];
	}
	for (j=0; j<stages; j++) w2c[j] = w1c[j];
	for (j=0; j<stages; j++) w1c[j] = w0c[j];
	outptrc[i].real = sc[stages].real * scale;
	outptrc[i].imag = sc[stages].imag * scale;
      }
    }
  }
  return(0);
}  



/* function pointer called by qsort() - (3-UNIX).  Complex numbers
   are orderd from the largest magnitude to the smallest */

static int
cplxcompare(i,j)
  double_cplx *i, *j;
{
  double k;
  k=(((*i).real) * ((*i).real) + ((*i).imag) * ((*i).imag)) -
    (((*j).real) * ((*j).real) + ((*j).imag) * ((*j).imag));

  if(k<0) 
    return(1);
  else if (k>0)
    return(-1);
  else
   return(0);

}


/* This is an efficient algorithm.  Picture a matrix with its element
   (i,j) of value x, where x is the distance from ith pole to jth zero.
   Start from row k=1, search the minimun at column j=j', then pole
   1 is paired up zero j'.  Replace j'th column with 1th column. 
   for next pole k=2, search starts at 2th element of k=2 row... 
   found colummn j'', replace j''th col with 2th column.  Iterate.... */
/* for real roots, they are treats as single */
/* The above is actually carried out by indexing */
/* assumes equal poles and zeros are clusterd together--done by qsort(3) */

static void 
pair( z, p, zn, pn, sdz, sdp )
double_cplx *z, *p;
long *zn, *pn;
short *sdz, *sdp;
{
  int i, j, l, idx_min, tmpzn, tmppn;
  long dim[2];
  double **dm, d;                /* dm the matrix */
  double_cplx *tmpa;
  int tmpi;
  int *idxz, *idxp;

  tmpzn = *zn;
  tmppn = *pn;
  /* indexing array for poles and zeros */
  idxz = (int *) malloc((*zn)*sizeof(int));
  spsassert(idxz,"pair: idxz malloc failed");
  idxp = (int *) malloc((*pn)*sizeof(int));
  spsassert(idxp,"pair: idxp malloc failed");

  for(i=0; i<*zn; i++) sdz[i] = 0; /*  */
  for(i=0; i<*pn; i++) sdp[i] = 0;

  /* get the indexing array */
  for(i=0,j= -1; i<*zn; i++){
    j++;
    if( z[i].imag == 0) /* real */
      if( i+1 < *zn){             /* compare equal roots */
	if( z[i].real==z[i+1].real && z[i].imag==z[i].imag ){
	  idxz[j] = i;            /* this is a double real root */
	  i++;
	}
	else{
	  idxz[j] = i;            /* this is a single real root */
	  sdz[j] = 1;
	}
      }
      else{                       /* single real root, at the end */
	idxz[j] = i;
	sdz[j] = 1;
      }
    else                /* complex */
      idxz[j] = i;
  }
  *zn = j+1;

  for(i=0,j= -1; i<*pn; i++){
    j++;
    if( p[i].imag == 0) /* real */
      if( i+1 < *pn){             /* compare equal roots */
	if( p[i].real==p[i+1].real && p[i].imag==p[i].imag ){
	  idxp[j] = i;            /* this is a double real root */
	  i++;
	}
	else{
	  idxp[j] = i;            /* this is a single real root */
	  sdp[j] = 1;
	}
      }
      else{                       /* single real root, at the end */
	idxp[j] = i;
	sdp[j] = 1;
      }
    else                /* complex */
      idxp[j] = i;
  }
  *pn = j+1;

  dim[0] = *pn;
  dim[1] = *zn;
  dm = (double **) arr_alloc(2, dim, DOUBLE, 0);  
  spsassert(dm,"pair: dm arr_alloc failed");

  /* compute distance table */
  for( i=0; i<*pn; i++){
    for( j=0; j<*zn; j++){
      dm[i][j] = (p[idxp[i]].real - z[idxz[j]].real) * 
	(p[idxp[i]].real - z[idxz[j]].real) + /*  */
	(p[idxp[i]].imag - z[idxz[j]].imag) * 
	  (p[idxp[i]].imag - z[idxz[j]].imag) ;
    }
  }

  idx_min = 0;
  for( i=0; i < *pn; i++ ){     /* start with each pole */
   d = dm[i][i];
   for( j=i+1; j<*zn; j++)                 /* search min zero index */
      if( dm[i][j] < d){
      	idx_min = j;
	d = dm[i][j];
      }
   if( idx_min != i){
     tmpi = idxz[i];
     idxz[i] = idxz[idx_min];
     idxz[idx_min] = tmpi;
     tmpi = sdz[i];
     sdz[i] = sdz[idx_min];
     sdz[idx_min] = tmpi;
     for( l=0; l<*pn; l++)  dm[l][idx_min] = dm[l][i];  /* swap columns */
   }
  }

  tmpa = (double_cplx *) malloc( tmpzn * sizeof(double_cplx));
  spsassert(tmpa,"pair: tmpa malloc failed");
  for(i=0;i<tmpzn;i++) {
    tmpa[i].real = z[i].real;
    tmpa[i].imag = z[i].imag;
  }
  for(i=0;i<*zn;i++){
    z[i].real = tmpa[idxz[i]].real;
    z[i].imag = tmpa[idxz[i]].imag;
  }
  
  tmpa = (double_cplx *) realloc((char *)tmpa, tmppn*sizeof(double_cplx));
  spsassert(tmpa,"pair: tmpa realloc failed");
  for(i=0;i<tmppn;i++) {
    tmpa[i].real = p[i].real;
    tmpa[i].imag = p[i].imag;
  }
  for(i=0;i<*pn;i++){
    p[i].real = tmpa[idxp[i]].real;
    p[i].imag = tmpa[idxp[i]].imag;
  }
  free(dm);
  free(tmpa);
  free(idxz);
  free(idxp);
}


int
firfunc( nsamp, inptr, outptr, inptrc, outptrc, arpt )
     long nsamp;
     double *inptr, *outptr;
     double_cplx *inptrc, *outptrc;
     struct firfilt *arpt;
{
  int nstate, num_siz;
  double *num;
  int datacode = 0;

  num_siz = *arpt->arma->num_size;
  nstate = num_siz -1;
  num = arpt->arma->re_num_coeff;

  if( inptr == NULL) datacode = 1;
  if( datacode == 0){                  /* real data */
    register double *numpt;
    register double sum;
    register long i, j, k;
    
    numpt = num;
    for( i = 0; i<nsamp; i++ ){
      sum = 0;
      for( j=0; j<num_siz; j++ ){
	if( (k=i-j) < 0 ) sum += num[j] * arpt->state[-k-1];
	else sum += num[j] * inptr[k];
      }
      outptr[i] = sum;
    }
    for( j=0, k=nsamp-1; j< nstate; j++ )
      arpt->state[j] = inptr[k-j];
  }
  else{                                  /* complex data */
    register long i,j,k;
    register double *numpt;
    register double_cplx sumc;
    register double_cplx samplec;
    numpt = num;
    for( i=0; i<nsamp; i++){
      sumc.real = 0;
      sumc.imag = 0;
      for( j=0; j< num_siz ; j++){
	if( (k=i-j)< 0)  samplec = arpt->statec[-k-1];
	else  samplec = inptrc[k];
	sumc.real += samplec.real * num[j];
	sumc.imag += samplec.imag * num[j]; 
      }
      outptrc[i] = sumc;
    }
    for( j=0, k=nsamp-1; j< nstate; j++ )
      arpt->statec[j] = inptrc[k-j];
  }
  return(0);
}
 
