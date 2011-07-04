/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description: Parks-McClellan FIR design
 *
 */

static char *sccs_id = "@(#)pkmc_filt.c	1.7 7/30/97 ERL";

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <esps/esps.h>
#include <esps/constants.h>
#include <esps/fea.h>
#include <esps/feafilt.h>
#include <esps/feasd.h>
#include <esps/strlist.h>

#define SYNTAX USAGE ("pkmc_filt [-P param_file][-x debug_level] filt_file");
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
					 argv[0], text); exit(1);}

/* local definitions*/
#define MAX_ITER 25
#define MAX_NBANDS 10
#define SMALL 0.00000001
#define OPPSIGN(A, B) ((A > 0 && B < 0) || (A < 0 && B > 0))

struct diff{
  double err;    /* value */
  int idx;       /* position on the grid */
};

struct edges{
  int idx1;
  int idx2;
};

struct spec{
  double edge[2];
  double des;
  double wt;
};

int debug_level = 0;

#define MULTIBAND 0
#define DIFFERENTIATOR 1
#define HILBERT 2
static char *jtype_code[] = {"MULTIBAND", "DIFFERENTIATOR", "HILBERT"};

#define CASE1 1
#define CASE2 2
#define CASE3 3
#define CASE4 4


/* system functions */
double pow(), fabs();
void perror();


/*ESPS functions*/
int getopt();
char *get_cmd_line();

/* local function definitions*/
void find_del_interpolate();
void find_extrema();
int check_extrema();
void write_outsd();
void get_coef();
void remez_exchange();
void setup();
void find_impulse();


char *Date="7/30/97";
char *Version="1.7";
char *Program="@(#)pkmc_filt.c	1.7";

void
main( argc, argv)
     int argc;
     char **argv;
{
  extern char *optarg;
  extern optind;
  FILE *fpout;
  struct header *oh;
  struct feafilt *frec;
  char *param_file = NULL;
  char *filt_file = NULL;
  filtdesparams *fdp;
  char *str, stri[3], *str_edge1, *str_edge2, *str_des, *str_wt;
  char *stu = "band";
  char *stl1 = "edge1";
  char *stl2 = "edge2";
  char *std = "des";
  char *stw = "wt";
  char *filt_type = NULL;
  int c, i, j, err=0;
  double *des, *cx, *wt;
  int Ngrid, nbands;
  float *hd_edges, *hd_des, *hd_wt;
  struct edges *bands;
  struct spec *firspecs;
  int Ncoef, Ntaps, jtype, fcase;
  double *A, fs, *coef, *h, *f, del, delay = 0;
  float *ripple_db;

/*
 * Check the command line options.
 */
  while ((c = getopt (argc, argv, "P:x:")) != EOF)
    {
      switch (c)
        {
        case 'P':
          param_file = optarg;
          break;
        case 'x':
          debug_level = atoi(optarg);
          break;
        default:
          SYNTAX
          }
    }

/*
 * Get the output filter filename
 */
  if (optind < argc)
    {
      filt_file = argv [optind];
      if (strcmp (filt_file, "-") == 0)
        {
          filt_file = "<stdout>";
          fpout = stdout;
        }
      else TRYOPEN("pkmc_filt", filt_file, "w", fpout);
      if (debug_level)
        Fprintf (stderr,"%s: Output file is %s\n", argv[0],filt_file);
    }
  else
    {
      Fprintf (stderr,"%s: No output file specified.\n", argv[0]);
      SYNTAX
        exit (1);
    }

/* 
 *Read parameter file and get design parameters 
 */
  
  if (debug_level)
    Fprintf (stderr,"%s: Reading parameter file %s\n", argv[0],param_file);
  
  if (read_params (param_file, SC_NOCOMMON, (char *)NULL) != 0)
    ERROR_EXIT("read_params could not read the params file.");

  /* filt_type: MULTIBAND, DIFFERENTIATOR, HILBERT */
  if (symtype("filt_type") != ST_UNDEF)
    filt_type = getsym_s ("filt_type");
  else ERROR_EXIT("Filt_type not specified in params file.");
  if((jtype = lin_search(jtype_code, filt_type)) == -1 ){
    Fprintf(stderr, "%s: Invalid filt_type (%s) specified - exiting.\n",
	    argv[0], filt_type);
    exit(1);
  }

  /* filter length parameter: even or odd number */
  if (symtype("filt_length") != ST_UNDEF)
    Ntaps = getsym_i ("filt_length");
  else ERROR_EXIT("Filt_length not specified in params file.");

  /* Ngrid parameter */
  if (symtype("ngrid") != ST_UNDEF)
    Ngrid = getsym_i ("ngrid");
  else Ngrid = 2 * Ntaps;

  /* sampling frequency parameter*/
  if( symtype("samp_freq") != ST_UNDEF)
    fs = getsym_d("samp_freq");
  else ERROR_EXIT("Samp_freq not specified in params file.");
  
  /* nbands parameter */
  if( symtype("nbands") != ST_UNDEF)
    nbands = getsym_i("nbands");
  else ERROR_EXIT("Nbands not specified in params file.");
 
  firspecs = (struct spec *) malloc( sizeof(struct spec) * nbands);
  /* band edges parameters, must be < fs/2 */
  str = (char *) malloc(sizeof(char)*20);
  str_edge1 = (char *) malloc(sizeof(char)*20);
  str_edge2 = (char *) malloc(sizeof(char)*20);
  str_des = (char *) malloc(sizeof(char)*20);
  str_wt = (char *) malloc(sizeof(char)*20);
  for( i = 1 ;i<=nbands; i++){
    str = (char *) strcpy( str, stu);
    sprintf( stri, "%d", i);
    str = (char *) strcat(str, stri);
    str = (char *) strcat(str, "_");
    /* find edges */
    str_edge1 = (char *) strcpy (str_edge1, str);
    str_edge1 = (char *) strcat(str_edge1, stl1);
    str_edge2 = (char *) strcpy (str_edge2, str);
    str_edge2 = (char *) strcat(str_edge2, stl2);
    if( symtype(str_edge1) != ST_UNDEF)
      firspecs[i-1].edge[0] = getsym_d(str_edge1);
    else err = 1;
    if( symtype(str_edge2) != ST_UNDEF)
      firspecs[i-1].edge[1] = getsym_d(str_edge2);
    else err = 1;
    if(err){
      Fprintf(stderr,"%s: missing band edges for band %d\n", argv[0], i);
      exit(1);
    }
    if( firspecs[i-1].edge[1] > fs/2)
      ERROR_EXIT("bandedge can not be greater than the Nyquist rate");
    /* find desired value */
    str_des = (char *) strcpy (str_des, str);
    str_des = (char *) strcat (str_des, std);
    if( symtype(str_des) != ST_UNDEF)
      firspecs[i-1].des = getsym_d(str_des);
    else{
      Fprintf(stderr,"%s: missing desired value for band %d\n", argv[0], i);
      exit(1);
    }
    str_wt = (char *) strcpy (str_wt, str);
    str_wt = (char *) strcat (str_wt, stw);
    if( symtype(str_wt) != ST_UNDEF)
      firspecs[i-1].wt = getsym_d(str_wt);
    else{
      Fprintf(stderr,"%s: missing weight value for band %d\n", argv[0], i);
      exit(1);
    }
  }

  /* Check that upper limit of last band == (sampling rate)/2.
     DB added to avoid numeric-error/core-dump that can occur.
   */
  if( firspecs[nbands-1].edge[1] != fs/2.) {
      Fprintf(stderr, "Upper bandedge of last band (%.2f) != sampling frequency / 2 (%.2f).\n\tPlease correct parameters and try again.\n",
		 firspecs[nbands-1].edge[1], fs/2);
      exit(1);
    }

  /* set up output generic header items */
  hd_edges = (float *) malloc(sizeof(float) * 2* nbands);
  hd_des = (float *) malloc(sizeof(float) *nbands);
  hd_wt = (float *) malloc(sizeof(float) *nbands);
  for(i=0, j=0; i<nbands;i++, j+=2){ 
    hd_edges[j] = firspecs[i].edge[0];
    hd_edges[j+1] = firspecs[i].edge[1];
    hd_des[i] = firspecs[i].des;
    hd_wt[i] = firspecs[i].wt;
  }

  bands = (struct edges *) malloc( sizeof(struct edges) * nbands);

  des = (double *) malloc(sizeof(double) * Ngrid);
  wt = (double *) malloc(sizeof(double) * Ngrid);
  cx = (double *) malloc(sizeof(double) * Ngrid);
  A = (double *) malloc (sizeof(double)* Ngrid);
  coef = (double *) malloc (sizeof(double)* Ntaps);
  h = (double *) malloc( sizeof(double) * Ntaps);
  f = (double *) malloc(sizeof(double) *Ngrid);
  ripple_db = ( float *) malloc(sizeof(float) * nbands);

  setup( f, cx, des, wt, &Ngrid, nbands, bands, Ntaps, &Ncoef, firspecs, jtype, &fcase, fs );
  remez_exchange( Ncoef, Ngrid, cx, des, wt, nbands, bands, A, coef, &del);

  for(i=0;i<nbands;i++) ripple_db[i] = 20*log10( firspecs[i].des + del/wt[i] );
  if(debug_level > 0)
    for(i=0;i<nbands;i++) 
      Fprintf(stderr,"%s: ripple in band %d is %f dB\n", argv[0],  
	      i, ripple_db[i]);
  
  if(debug_level == 15521) {
    write_outsd(A,Ngrid,"pkmc_A");
    write_outsd(des,Ngrid,"pkmc_des");
  }

  find_impulse(coef, Ncoef, h, fcase, Ntaps );

  fdp = (filtdesparams *) calloc( 1, sizeof(filtdesparams));
  fdp->define_pz = NO;
  fdp->method = PARKS_MC;
  fdp->nbands = nbands;
  fdp->bandedges = hd_edges;
  fdp->wts = hd_wt;
  fdp->gains = hd_des;
  fdp->g_size = Ngrid;
  delay = (double) (Ntaps-1) / 2;

  oh = new_header(FT_FEA);
  (void) strcpy (oh->common.prog, argv[0]);
  (void) strcpy (oh->common.vers, Version);
  (void) strcpy (oh->common.progdate, Date);
  oh->common.tag = NO;
  add_comment(oh, get_cmd_line(argc, argv));
  init_feafilt_hd( oh, (long)(Ntaps), 0L, fdp);
  (void)add_genhd_f("band_edges", hd_edges, 2*nbands, oh);
  (void)add_genhd_f("desired_value", hd_des, nbands, oh);
  (void)add_genhd_f("desired_weight", hd_wt, nbands, oh);
  (void)add_genhd_f("ripple_dB", ripple_db, nbands, oh);
  (void)add_genhd_d("samp_freq", &fs, 1, oh);
  (void)add_genhd_d("delay_samples", &delay, 1, oh);

  (void) write_header ( oh, fpout );
  frec = allo_feafilt_rec(oh);
  *frec->num_size = Ntaps;
  for(i=0;i<Ntaps;i++) frec->re_num_coeff[i] = h[i];
  put_feafilt_rec( frec, oh, fpout);


  /* clean up*/
  free(firspecs);
  free(str);
  free(str_edge1);
  free(str_wt);
  free(str_edge2);
  free(str_des);
  free(hd_edges);
  free(hd_des);
  free(hd_wt);
  free(bands);
  free(des);
  free(wt);
  free(cx);
  free(A);
  free(coef);
  free(h);
  free(f);
  free(ripple_db);
  free(fdp);
  
  exit(0);
}

void
find_impulse(coef, Ncoef, h, fcase, Ntaps)
  double *coef, *h;
  int Ncoef, fcase, Ntaps;
{
  int i;
  switch(fcase){
  case CASE1:
    if( Ncoef != (Ntaps - 1)/2 + 1) 
      Fprintf(stderr,"ERROR: number of coefficients mismatch.\n");
    for(i=0; i< Ncoef-1; i++) h[i] = 0.5 * coef[Ncoef- 1 - i];
    h[Ncoef-1] = coef[0];
    for(i=Ncoef; i< Ntaps; i++) h[i] = h[Ntaps -i -1];
    break;
  case CASE2:
    if( Ncoef != Ntaps / 2)
      Fprintf(stderr,"ERROR: number of coefficients mismatch.\n");
    if( Ncoef > 1){
      h[0] = 0.25 * coef[Ncoef - 1];
      for(i=1; i< Ncoef-1; i++) h[i] = 0.25* (coef[Ncoef-1-i] + coef[Ncoef-i]);
      h[Ncoef - 1] = 0.5*(coef[0] + 0.5 * coef[1]);
    }
    else
      h[0] = 0.5*coef[0];
    for(i=Ncoef; i< Ntaps; i++) h[i] = h[Ntaps -i -1];
    break;
  case CASE3:
    if( Ncoef != (Ntaps-1) / 2)
      Fprintf(stderr,"ERROR: number of coefficients mismatch.\n");
    if( Ncoef >= 3){
      h[0] = .25 * coef[Ncoef -1];
      h[1] =  0.25 * coef[Ncoef -1 ];
      for(i=2; i< Ncoef-1; i++) h[i]= 0.25*(coef[Ncoef-i-1] - coef[Ncoef-i+1]);
      h[Ncoef-1] = 0.5 *(coef[0] -0.5*coef[2]);
    }
    else if(Ncoef==2){
      h[0] = 0.25 * coef[1];
      h[1] = 0.5 * coef[0];
    }
    else if(Ncoef==1)
      h[0] = 0.5 * coef[0];
    
    h[Ncoef] = 0;
    for(i=Ncoef+1; i< Ntaps; i++) h[i] = -h[Ntaps -i-1];
    break;
  case CASE4:
    if( Ncoef != Ntaps/ 2)
      Fprintf(stderr,"ERROR: number of coefficients mismatch.\n");
    if( Ncoef > 1){
      h[0] = 0.25 * coef[Ncoef-1];
      for(i=1; i< Ncoef-1; i++) h[i]= 0.25*(coef[Ncoef-i-1]-coef[Ncoef-i]);
      h[Ncoef-1] = 0.5 * (coef[0] - 0.5 * coef[1]);
    }
    else
      h[0] = 0.5 * coef[0];

    for(i=Ncoef; i<Ntaps; i++) h[i] = - h[Ntaps-i-1];
    break;
  }
}

void
setup( w, cx, des, wt, Ngrid, nbands, bands, Ntaps, Ncoef, 
	   firspecs, jtype, fcase, nfs )
     double *w, *cx, *des, *wt;
     int *Ngrid, nbands, *Ncoef, Ntaps;
     struct edges *bands;
     struct spec *firspecs;
     int jtype, *fcase;
     double nfs;        /* Nyquist rate, not sampling rate */
{
  double pi2, dfreq, minf, maxf, f, desired, weight;
  int k,m,i,j,d;
  

  pi2 = 2 * PI;
  dfreq = 0.5 / (*Ngrid-1);

  if( jtype == MULTIBAND ){
    if( 2*(Ntaps/2) != Ntaps) { 
      *fcase = CASE1;
      minf = 0;
      maxf = 0.5;
      *Ncoef = (Ntaps -1)/2 + 1;        /* THIS IS L ONLY!!!*/
    }
    else { 
      *fcase = CASE2;
      minf = 0;
      maxf = 0.5-dfreq;
      *Ncoef = Ntaps / 2; 
    }
  }
  else{
    if( 2*(Ntaps/2) != Ntaps) { 
      minf = dfreq;
      maxf = 0.5-dfreq;
      *Ncoef = (Ntaps -1)/2; 
      *fcase = CASE3;
    }
    else { 
      minf = dfreq;
      maxf = 0.5;
      *Ncoef = Ntaps / 2; 
      *fcase = CASE4;
    }
  }

/* use JTYPE determine des, wt: JTYPE=HILBERT, keep const, JTYPE=DIFFERENTIATOR, desired=1/f*/


  /* convert frequency point to 0 to 0.5 */
  k = 0;
  m = 0;
  for(i=0; i<nbands; i++){
    firspecs[i].edge[0] = MAX(minf, firspecs[i].edge[0]/ nfs);
    firspecs[i].edge[1] = MIN(maxf, firspecs[i].edge[1]/ nfs);
    d = (firspecs[i].edge[1] - firspecs[i].edge[0]) / dfreq;
    bands[i].idx1 = m;
    bands[i].idx2 = bands[i].idx1 + d;
    m = bands[i].idx2 + 1;
    for( j=0; j <= d; j++){
      f = MAX(minf, firspecs[i].edge[0]) + dfreq * j;
      w[k] = f*nfs;
      if( jtype != DIFFERENTIATOR ){
	desired = firspecs[i].des;
	weight = firspecs[i].wt;
      }
      else{
	desired = firspecs[i].des * f;
	if( f!=0) weight = firspecs[i].wt / f;
	else weight = firspecs[i].wt;
      }
      cx[k] = cos( pi2 * f);
      if(*fcase == CASE1){
	des[k] = desired;
	wt[k] = weight;
      }
      if(*fcase == CASE2){
	des[k] = desired / cos(PI*f);
	wt[k] = weight * cos(PI*f);
      }
      if(*fcase == CASE3){
	des[k] = desired / sin(pi2*f);
	wt[k] = weight * sin(pi2*f);
      }
      if(*fcase == CASE4){
	des[k] = desired/ sin(PI*f);
	wt[k] = weight * sin(PI*f);
      }
      k++;
    }
  }
  *Ngrid = k;
}
  



void
remez_exchange( Ncoef, Ngrid, cx, des, wt, Nbands, bands, A, coef, final_db)
int Ncoef;   /* L+1 in 7.110 */
int Ngrid;   /* number of points on frequency */     
double *cx;  /* the cos'ed frequecy grid point on appx. bands */
double *des, *wt;   /* desired, weights ordinate on cx abscissa */
int Nbands;
struct edges *bands;
double *A, *coef, *final_db;
{
  int do_again = 1, iter = MAX_ITER;
  int Nexm;
  int CNexm;
  struct diff *exm, *Cexm;
  int dgrid, i, j, k, m,  bdN;
  double del, olddel=0;
  double *d, *C;

  Nexm = Ncoef+1;
  exm = (struct diff *) malloc (sizeof(struct diff)* Nexm);
  Cexm = (struct diff *) malloc (sizeof(struct diff)* Ngrid);

  d = (double *) malloc( sizeof(double) * Nexm );
  C = (double *) malloc( sizeof(double) * Nexm-1 );

  /* initial guess, place extremas uniformly on the bands */
  for(i=0, k=0; i<Nbands-1; i++){
    if(0==(bdN = (int) Nexm *  (bands[i].idx2 - bands[i].idx1+1)/Ngrid)){
      Fprintf(stderr,"ERROR: transition band too narrow - exiting\n");
      exit(1);
    }
    dgrid = (int) (bands[i].idx2 - bands[i].idx1 ) / bdN;
    for( m = 0, j=bands[i].idx1; j<= bands[i].idx2 && m< bdN; j += dgrid, m++ )
      {exm[k].idx = j; k++;}
  }
  bdN = Nexm - k;
  dgrid = (int) (bands[Nbands-1].idx2 - bands[Nbands-1].idx1 ) / bdN;
  for( m=0, j=bands[Nbands-1].idx1; j<= bands[Nbands-1].idx2 && m<bdN; m++,j += dgrid ) {exm[k].idx = j; k++;}

  if(debug_level>=10){
    Fprintf(stderr,"initial guess on extremas\n");
    for(i=0; i<Nexm; i++) Fprintf(stderr,"extrema on %d\n", exm[i].idx);
  }
  while( do_again && iter-- > 0 ){
    
    if(debug_level>=10) Fprintf(stderr,"Iteration %d\n", MAX_ITER - iter);

    (void) find_del_interpolate(exm, Nexm, des, wt, cx, Ngrid, A, &del, d, C);

    if(debug_level >=10)
      Fprintf(stderr,"   delta = %g\n", del);

    if(fabs(del) < olddel){
      Fprintf(stderr,"ERROR: remez_exchange: delta value non-increasing, numerical problem - exiting\n");
      exit(1);
    }
    olddel = fabs(del);

    (void) find_extrema(A, des, wt, Ngrid, del, Cexm, &CNexm);

    if(debug_level >=10){
      Fprintf(stderr,"   found %d extremas\n", CNexm);
      for(i=0; i<CNexm; i++) Fprintf(stderr,"   found extrema on %d val %g\n",
				     Cexm[i].idx, Cexm[i].err);
    }
    do_again = check_extrema( CNexm, Cexm, &Nexm, exm, del);

    if(debug_level >=10){
      Fprintf(stderr,"   found %d extremas\n", CNexm);
      Fprintf(stderr,"   decide %d extremas\n", Nexm);
      for(i=0; i<Nexm; i++) Fprintf(stderr,"   use extrema on %d val %g\n",
				    exm[i].idx, exm[i].err);
      Fprintf(stderr,"\n");
    }
  }
  if( iter == 0 ) {
    Fprintf(stderr,"remez_exchange: Unable to find a solution after %d iterations.-- exiting\n", MAX_ITER);
    exit(1);
  }
  get_coef(cx, Ncoef, coef, exm, d,C);
  *final_db = fabs(del);

  /* clean up */
  free(exm);
  free(Cexm);
  free(d);
  free(C);
}


void
get_coef(cx,  Ncoef, coef, exm, d, C)
     double  *coef, *cx, *d, *C;
     int Ncoef;
     struct diff *exm;
{
  double sum = 0;
  int i, k, j, sign = 0, M;
  double x;
  double num, den;
  double *Ai, pi2, D;

  pi2 = 2* PI;
  Ai = (double *) malloc(sizeof(double) * Ncoef);

  M = 2 * Ncoef - 1;
  for(i = 0; i< Ncoef; i++){
    num =0;
    den = 0;
    x = cos(pi2 * i / M);
    for( k=0; k<Ncoef; k++){
      if( fabs(cx[exm[k].idx] - x) <= SMALL){
	num = C[k];
	den = 1.0;
	sign = (sign > 0) ? -1: 1;
	break;
      }
      else{
	D = d[k] / ( x - cx[exm[k].idx]);
	num += D * C[k];
	den += D;
	sign = (sign > 0) ? -1: 1;
      }
    }
    Ai[i] = num/den;    /* frequency sampling */
  }
  if(debug_level == 15521) write_outsd(Ai, Ncoef, "pkmc_Ai");

/* freq response in Ai is
   A(w) = sum a[n]*cos[wn], n->[0, N-1]    N terms       --> Ncoef
   A(w) also = sum b(n) * exp(jwn) , n->[0, M-1] M terms;
       M = 2N-1
       b(0) = a(0)
       b(n) = a(n)/2 for n->(0,N)
       b(n) = b(M-n) for n->[N, M)

   if Q(k) = Q(2*PI k/M), the dft of b(0).... b(M-1)  Q(k) = Q(M-k).
   inv dft gives
       b(n) = 1/M * sum Q(k)*exp(j2PI kn/M) ,   k->[0, M-1]
            = (Q(0)/M) + (2/M) * sum Q(k) *cos(2 PI kn/M), k->[1, N-1]

   -->
   a(0) = 1/M (P(0) + 2 sum Q(k)), k->[1, N-1]
   a(n) = (2/M) [Q(0) + 2*sum Q(k) *cos(2 PI kn/M)], k->[1, N-1] for
       all n ->(0, N)
*/


/*  Ncoef ---> terms used in sum a(n) * cos(wn) */
  sum = 0;
  for(i=1; i< Ncoef; i++) sum += Ai[i];
  coef[0] = (Ai[0] + 2 * sum ) / M;

  for(i=1; i< Ncoef; i++){
    sum = 0;
    for(j = 1; j< Ncoef; j++)  sum += Ai[j] * cos(pi2* i * j / M);
    coef[i] = 2 * ( Ai[0] + 2*sum) / M;
  }

  /*clean up*/
  free(Ai);
      
}


void 
find_del_interpolate(exm, Nexm, des, wt, cx, Ngrid, A, delta, d, C)
     double *des, *wt, *A, *cx;
     struct diff *exm;
     int Ngrid, Nexm;
     double *delta;
     double *d, *C;
{

  int i, k, j, st, order, sign;
  double num=0, den=0, b=1, D;

  order = Nexm - 1;
  st = Nexm / 10 + 1;
  sign = 1;
  for( k=0; k< Nexm; k++){
    b = 1.0;
    for( j =0; j< st; j++){
      for(i=j; i<Nexm; i+=st){
	if( i!=k ) b =  2 * b * (cx[exm[k].idx] - cx[exm[i].idx]);
      }
    }
    if( fabs(b) == 0){
      Fprintf(stderr,"ERROR: lagrange coef b[%d] not distinct\n", k);
      exit(1);
    }
    b = 1/b;
    d[k] = b * (cx[exm[k].idx] - cx[exm[Nexm-1].idx]);
    num += b * des[exm[k].idx];
    den += (sign > 0) ? (b / wt[exm[k].idx]): (-b / wt[exm[k].idx]);
    sign = (sign > 0) ? -1: 1;
  }
  *delta = num / den;

  for( sign = 1, i = 0; i<order; i++){
    if(sign > 0)
      C[i] = des[exm[i].idx] - (*delta / wt[exm[i].idx]);
    else
      C[i] = des[exm[i].idx] + (*delta / wt[exm[i].idx]);
    sign = (sign>0) ? -1:1;
  }

  for( j=0; j<Ngrid; j++){
    num = 0;
    den = 0;
    for( k=0; k<order; k++){
      if( exm[k].idx == j){
	num = C[k];
	den = 1.0;
	break;
      }
      else{
	D = d[k] / (cx[j] - cx[exm[k].idx]);
	num += D * C[k];
	den += D;
      }
    }
    A[j] = num / den;
  }
  if(debug_level >= 1000) 
    for(i=0; i<Ngrid; i++) 
      Fprintf(stderr,"err[%d] %g A[%d] %g\n", i, wt[i]*(des[i]-A[i]), i, A[i]);
}

void 
find_extrema(A, des, wt, Ngrid, del, exm, Nexm)
     double *A, *des, *wt, del;
     int Ngrid,  *Nexm;
     struct diff *exm;
{
  int i, j, k;
  double *err, *ferr;
  del = fabs(del);
  err = (double *) malloc(sizeof(double ) *Ngrid);
  ferr = (double *) malloc(sizeof(double ) *Ngrid);
  for( i=0; i<Ngrid; i++){       /* each i <-> x[i], the freq */
    err[i] = wt[i] * (des[i] - A[i]);
    ferr[i] = fabs(err[i]);
  }
  
  /* first endpoint */
  j = 0;
  if( ferr[0] >= del-SMALL )
    if( (err[0] > 0 && err[1] > 0 && err[0] > err[1]) ||
       ( err[0] < 0 && err[1] < 0 && err[0] < err[1]) ||
       OPPSIGN(err[0], err[1]) ){
      exm[j].idx = 0;
      exm[j].err = err[0];
      j++;
    }

  for( k=1; k< Ngrid-1; k++){
    if( ((err[k] > err[k+1] && err[k] > err[k-1]) ||    /* local max */
	 (err[k] < err[k-1] && err[k] < err[k+1]) ) &&  /* local min */
       ferr[k] >= del -SMALL ){                         /* |E| >= delta */
      if( j == 0 ){
	exm[j].idx = k;
	exm[j].err = err[k];
	j++;
      }
      else if( OPPSIGN(exm[j-1].err, err[k])){         /* possible alter.*/
	exm[j].idx = k;
	exm[j].err = err[k];
	j++;
      }
    }
  }

  /* last endpoint */
  k = Ngrid - 1;
  if(   ferr[k] >= del - SMALL &&
        OPPSIGN(exm[j-1].err , err[k])){
      exm[j].idx = k;
      exm[j].err = err[ k];
      j++;
    }
  *Nexm = j;

  /*clean up */
  free(err);
  free(ferr);
}


int 
check_extrema(Nexm, exm, NexmN, exmN, del)
     double del;
     int *NexmN, Nexm;
     struct diff *exm, *exmN;        /* exm: new found */
                                     /* exmN: largest extremas of new found 
					was the old extremas */
{
  int st, en, i, j, redo=0;
  del = fabs(del);
  if( Nexm > *NexmN ){                 /* retain the largest */
    i = Nexm;    
    st = 0;
    en = Nexm - 1;
    while( i-- > *NexmN ){                 /* rid extra end points */
      if( fabs(exm[st].err) >= fabs(exm[en].err) )
	en--;
      if( fabs(exm[st].err) < fabs(exm[en].err) )
	st++;
    }
    for( i = st, j=0; i<= en; i++, j++){
      exmN[j].idx = exm[i].idx;
      exmN[j].err = exm[i].err;
    }
    redo = 1;
  }
  if ( *NexmN == Nexm ){
    redo = 0;
    for(i=0; i < Nexm; i++){
      if( exmN[i].idx != exm[i].idx || fabs(exm[i].err) >= del + SMALL){
	for(j = 0; j<Nexm; j++){
	  exmN[j].idx = exm[j].idx;
	  exmN[j].err = exm[j].err;
	}
	redo = 1;
	break;
      }
    }
  }
  if( *NexmN > Nexm ){
    Fprintf(stderr,
	    "ERROR: check_extrema: only found %d extremas, should be %d\n\n - numerical problem - exiting\n", Nexm, *NexmN);
    exit(1);
  }
  return( redo);
}

void
write_outsd(pt, nsamp, fname )
     double *pt;
     int nsamp;
     char *fname;
{
  struct header *ohd;
  FILE *osdfile;
  char *oname = NULL;
  struct feasd *osd_rec;
  double *data;
  double start_time = 0;
  int i;

  oname = eopen("remez", fname, "w", NONE, NONE, &ohd, &osdfile);
  ohd = new_header(FT_FEA);
  (void) strcpy (ohd->common.prog, "remez");
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  ohd->common.tag = NO;
  init_feasd_hd(ohd, DOUBLE, 1 , &start_time  ,NO, 1.0);
  (void) write_header ( ohd, osdfile );
  osd_rec = allo_feasd_recs(ohd, DOUBLE, nsamp, (char*) NULL, NO);
  data = (double *) osd_rec->data;
  for(i=0;i<nsamp;i++) data[i] = pt[i];
  put_feasd_recs( osd_rec, 0L, nsamp, ohd, osdfile );
}

