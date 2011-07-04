/* formant.c */
/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1994  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  David Talkin
 * Revised by: John Shore
 *
 */

static char *sccs_id = "@(#)formant.c	1.23	11/5/96	ATT/ESI/ERL";

#define VERSION "1.19"
#define DATE "9/20/94"

#include <Objects.h>
#ifndef LINUX_OR_MAC
#include <sgtty.h>
#endif
#ifndef APOLLO_68K
#include <math.h>
#else
#include "math.h"
#endif
#include "tracks.h"
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/sd.h>
#include <string.h>

#define SYNTAX \
USAGE("formant [-p preemphasis] [-r n:[[+]m]] [-n num_formants] [-o lpc_order]\n \
 [-O output_path] [-i frame_interval] [-w window_duration] [-W window_type]\n \
 [-t lpc_type] [-S] [-f ds_freq] [-B max_buff_bytes] [-M maxrms_value]\n \
 [-N nominal_f1_freq] [-R maxrms_duration] [-x debug_level] [-F] infile.sd")

char *get_cmd_line();
void e_make_sd_hdr();
void e_make_fea_hdr();

int debug_level = 0, cmap_size;
int debug = 0;
int w_verbose = 0;

long max_buff_bytes = 2000000;  /* limit on size of signal data buffers */

int read_poles();

Signal *dpfund(), *lpc_poles(), *downsample(), *highpass(), *get_signal(),
       *dpform(), *new_signal(), *get_signal_from_path();

extern int fea_sd_special; /*in header.c*/
/* char *strcat(), *strrchr(), *strncpy() */

char *localloc( int n );

char *new_ext(oldn,newex)
char *oldn, *newex;
{
  int	j;
  char *dot;
  static char newn[256];
  static int len = 0;

  dot = strrchr(oldn,'.');
  if(dot != NULL){
    *dot = 0;
    j = strlen(oldn) + 1;
    *dot = '.';

  }
  else j = strlen(oldn);
  if((len = j + strlen(newex)+ 1) > 256) {
    printf("Name too long in new_ext()\n");
    return(NULL);
  }
  strncpy(newn,oldn,j);
  newn[j] = 0;
  return(strdup(strcat(newn,newex)));
}

/*      ----------------------------------------------------------      */
main(argc,argv)
int	argc;
char	**argv;
{
  POLE **pole;
  int nframes, nform, nsamp, nsamp_ds, i,j, lpc_ord, in, lpc_type, f0only,
  w_type, save_sig_files, c;
  double  frame_int, wdur, maxrms_dur, maxrms_val, f0typ,
  **freq, **band, ds_freq, nom_f1 = -10.0,
  preemp;
  double cor_wdur, fmin, fmax, *f0, **dpp, **dpps;
  short *data, *scr, *vuv;
  CROSS	**cp;
  DPREC	**dp;
  char *polename, *f0name, *sp, station[200], path[200];
  Signal *polesig, *f0sig, *formantsig, *spsig, *dssig, *hpsig, *stasig;
  Signal *hpsrc, *polesrc, *f0src;
  extern int optind;
  extern char *optarg;
  char *command;		/* function command line */
  char *range;
  struct header *ehd_sp, *ehd_ds, *ehd_hp, *ehd_fb, *ehd_f0;
  /* ESPS headers */

  /* make sure FEA_SD are read as shorts, since some routines insist on
     them; note that this can be a problem if the external file does not
     contain shorts since it's likely that we'll lose accuracy
     */
	
  fea_sd_special = 1;

  command = get_cmd_line(argc, argv);
  lpc_ord = 12;
  lpc_type = 0;			/* use bsa's stabilized covariance if != 0 */
  w_type = 2;			/* window type: 0=rectangular; 1=Hamming; 2=cos**4 */
  ds_freq = 10000.0;
  wdur = .049;			/* for LPC analysis */
  cor_wdur = .01;		/* for crosscorrelation F0 estimator */
  maxrms_dur = 0.0;		/* window for max rms - 0 means all of past */
  maxrms_val = 0.0;		/* value for max rms - 0 means normalize */
  frame_int = .01;
  preemp = .7;
  nform = 4;
  fmin = 60.0;			/* limits for F0 estimator */
  fmax = 500.0;
  f0only = FALSE;
  save_sig_files = FALSE;
  *station = 0;			/* "stationarity" signal */
  *path = 0;
  range = NULL;
  if(argc < 2)
    SYNTAX;

  while((c = getopt(argc,argv,"Ff:x:B:o:O:W:t:p:w:i:r:Ss:n:R:M:N:y:z:")) != EOF) {
    switch(c) {
    case 'F':
      f0only = TRUE;
      break;
    case 'f':
      ds_freq = atof(optarg);
      break;
    case 'x':
      debug_level = debug = atoi(optarg);
      break;
    case 'N':
      nom_f1 = atof(optarg);
      break;
    case 'o':
      lpc_ord = atoi(optarg);
      break;
    case 'O':
      strcpy(path,optarg);
      break;
    case 'W':
      w_type = atoi(optarg);
      break;
    case 't':
      lpc_type =  atoi(optarg);
      break;
    case 'p':
      preemp = atof(optarg);
      break;
    case 'w':
      wdur = atof(optarg);
      break;
    case 'i':
      frame_int = atof(optarg);
      break;
    case 'y':
      fmax = atof(optarg);
      break;
    case 'z':
      fmin = atof(optarg);
      break;
    case 'r':
      range = optarg;
      break;
    case 'S':
      save_sig_files = TRUE;
      break;
    case 's':
      strcpy(station,optarg);
      break;
    case 'n':
      nform = atoi(optarg);
      break;
    case 'B':
      max_buff_bytes = atol(optarg);
      break;
    case 'R':
      maxrms_dur = atof(optarg);
      break;
    case 'M':
      maxrms_val = atof(optarg);
      break;
    default:
      SYNTAX;
      break;
    }
  }

  /* Now process all of the input files
     (automatically creating output file names) */
  for(in=optind; in < argc; in++ ) {
    sp = argv[in];
    formantsig = polesig = f0sig = spsig = dssig = hpsig = NULL;

    /*
     * Check for errors in specifying parameters
     */
    if ((maxrms_val != 0.0) && (maxrms_dur != 0.0)) {
      printf("Can't use both -R and -M\n");
      exit (1);
    }

    if ((maxrms_dur > 0.0) && (maxrms_dur < cor_wdur)) {
      maxrms_dur = frame_int;
      printf("WARNING: maxrms_duration less than cross-correlation window (%g)\n", cor_wdur);
      printf("\treset maxrms_duration to %g.\n", cor_wdur);
    }

    if(nform > (lpc_ord-4)/2){
      printf("Number of formants must be <= (lpc order - 4)/2 - exiting.\n");
      exit (1);
    }

    if(nform > MAXFORMANTS){
      printf("A maximum of %d formants are supported at this time - exiting.\n",
	     MAXFORMANTS);
      exit(1);
    }

    if(range) {
      if((spsig = get_signal(sp,0.0,0.0,NULL))) {
	long start_p=0, end_p=0;
	double start, duration;
	lrange_switch (range, &start_p, &end_p, 1);
	if (end_p > spsig->file_size -1)
	    end_p = spsig->file_size -1;
	if(!end_p)
	  end_p = spsig->file_size -1;
	if(start_p > 0) start_p--; /* base 1 to base 0 conversion */
	if(start_p >= end_p) {
	  fprintf(stderr,"Unrealistic computation range limits\n");
	  exit(-1);
	}
	start = spsig->start_time + (((double)start_p)/spsig->freq);
	duration = ((double)(end_p - start_p + 1))/spsig->freq;
	spsig->utils->read_data(spsig,start,duration);
      }
    } else
      spsig = get_signal(sp,0.0,-1.0,NULL);

    if(spsig) {
      if(ds_freq != spsig->freq) {
	dssig = NULL;
	if(!((dssig = get_signal_from_path(path,new_ext(spsig->name,"ds"),0.0,-1.0,NULL)) &&
	     (dssig->freq == ds_freq) )) {
	  if(dssig) free_signal(dssig);
	  if((dssig = downsample(spsig,ds_freq))) {
	    if(spsig != dssig) {
	      /* insert proper ESPS header for downsampled signal*/
	      e_make_sd_hdr(dssig, spsig, command);
	      add_source_file(dssig->header->esps_hdr, sp,
			      spsig->header->esps_hdr);
	      add_comment(dssig->header->esps_hdr,
			  "This is the down sampled signal.\n");
	      dssig->header->magic = ESPS_MAGIC;
	      put_signal_to_path(path,dssig);
	    }
	  } else {
	    printf("Problems downsampling the signal %s\n",spsig->name);
	    exit(-1);
	  }
	}
      }

      hpsrc = (dssig ? dssig : spsig);

      if(f0only || (preemp < 1.0)) { /* be sure DC and rumble are gone! */
	if(! (hpsig = get_signal_from_path(path,new_ext(hpsrc->name,"hp"),0.0,-1.0,NULL))) {
	  if((hpsig = highpass(hpsrc))) {
	    /* insert proper ESPS header for high pass signal */
	    e_make_sd_hdr(hpsig, hpsrc, command);
	    add_source_file(hpsig->header->esps_hdr, hpsrc->name,
			    hpsrc->header->esps_hdr);
	    add_comment(hpsig->header->esps_hdr,
			"This is the high pass signal.\n");
	    hpsig->header->magic = ESPS_MAGIC;
	    put_signal_to_path(path,hpsig);
	  } else {
	    printf("Couldn't highpass the signal\n");
	    exit(-1);
	  }
	}
      }
    } else {
      printf("Can't access sampled speech signal: %s\n",sp);
      exit(-1);
    }

    polesrc = (hpsig ? hpsig : spsig);

    if(! f0only) {		/* compute LPC complex poles as needed  */
      if(!(polesig=lpc_poles(polesrc,wdur,frame_int,lpc_ord,preemp,lpc_type,
			     w_type))) {
	printf("Problems in lpc_poles()\n");
	exit(-1);
      }
      /* force writing of pole file as ASCII file*/
      polesig->header->magic = SIGNAL_MAGIC;
      /*	polesig->type |= SIG_ASCII;*/
      put_signal_to_path(path,polesig);
    }

    /*compute F0*/

    if(!(f0sig = dpfund(polesrc,cor_wdur, fmin, fmax, frame_int,
			&cp, &dp, maxrms_dur, maxrms_val))) {
      printf("Problems in dpit()\n");
      exit(-1);
    }
    fflush(stdout);
    free_dp_cp(dp,cp,f0sig->buff_size);
    /* insert proper header for F0 FEA file*/
    e_make_fea_hdr(f0sig, 5, command);
    f0sig->header->e_scrsd = 0;
    ehd_f0 = f0sig->header->esps_hdr;
    add_source_file(ehd_f0, polesrc->name, polesrc->header->esps_hdr);

    *add_genhd_d("preemphasis", (double *) NULL, 1, ehd_f0) = preemp;
    *add_genhd_d("window_duration", (double *) NULL, 1, ehd_f0) = wdur;
    *add_genhd_d("frame_duration", (double *) NULL, 1, ehd_f0) = frame_int;
    *add_genhd_s("lpc_order", (short *) NULL, 1, ehd_f0) = lpc_ord;
    *add_genhd_s("lpc_type", (short *) NULL, 1, ehd_f0) = lpc_type;
    *add_genhd_s("window_type", (short *) NULL, 1, ehd_f0) = w_type;
    (void) add_fea_fld("F0", 1L,
		       0, (double *) NULL, DOUBLE, (char **) NULL, ehd_f0);

    (void) add_fea_fld("prob_voice", 1L,
		       0, (double *) NULL, DOUBLE, (char **) NULL, ehd_f0);

    (void) add_fea_fld("rms", 1L,
		       0, (double *) NULL, DOUBLE, (char **) NULL, ehd_f0);

    (void) add_fea_fld("ac_peak", 1L,
		       0, (double *) NULL, DOUBLE, (char **) NULL, ehd_f0);

    (void) add_fea_fld("k1", 1L,
		       0, (double *) NULL, DOUBLE, (char **) NULL, ehd_f0);
    f0sig->header->magic = ESPS_MAGIC;
    put_signal_to_path(path,f0sig);
    f0sig->header->magic = SIGNAL_MAGIC;
    f0sig->type = P_DOUBLES | SIG_F0;
    f0sig->name = new_ext(f0sig->name,"f0.sig");
    if(save_sig_files)
      put_signal_to_path(path,f0sig);

    if(!f0only) {
      if(*station) {		/* slip a stationarity function into the pole signal */
	if((stasig = get_signal(station,0.0,-1.0,NULL)) &&
	   (stasig->freq == polesig->freq)) {
	  int iop, ios, nfr;
	  double dt;
	  POLE **pp;

	  ios = iop = 0;
	  if((dt = (stasig->start_time - polesig->start_time)) > 0.0)
	    iop = 0.5 + dt * polesig->freq;
	  else
	    ios = 0.5 - dt * polesig->freq;
	  if((nfr = polesig->buff_size - iop) > (stasig->buff_size - ios))
	    nfr = stasig->buff_size - ios;
	  for(pp = (POLE**)polesig->data,
	      dpps = (double**)stasig->data, i=0; i < iop; i++)
	    pp[i]->rms = *dpps[0]; /* pad beginning */
	  for(j=ios; j < nfr+ios; j++, i++)
	    pp[i]->rms = dpps[0][j];
	  for( ;i < polesig->buff_size; i++)
	    pp[i]->rms = dpps[0][j-1]; /* pad end */
	  free_signal(stasig);
	} else
	  printf("Problems getting the stasig\n");
      }
	
      /* LPC poles are now available for the formant estimator. */
      if((formantsig = dpform(polesig, nform, nom_f1))) {
	dpp = (double**)formantsig->data;
	/* insert ESPS FEA header for FB file */
	e_make_fea_hdr(formantsig, 2*nform, command);
	formantsig->header->e_scrsd = 0;
	ehd_fb = formantsig->header->esps_hdr;
	/*f0sig name seems to have changed here - don't know why*/
	add_source_file(ehd_fb, new_ext(polesig->name, "f0"), ehd_f0);
	add_comment(ehd_fb,
		    "ASCII pole file is also a source\n");

	(void) add_fea_fld("fm", nform,
			   1, (double *) NULL, DOUBLE, (char **) NULL, ehd_fb);
	(void) add_fea_fld("bw", nform,
			   1, (double *) NULL, DOUBLE, (char **) NULL, ehd_fb);

	formantsig->header->magic = ESPS_MAGIC;
	put_signal_to_path(path,formantsig);
	formantsig->header->magic = SIGNAL_MAGIC;
	formantsig->type = P_DOUBLES | SIG_FORMANTS;
	formantsig->name = new_ext(formantsig->name,"fb.sig");
	put_signal_to_path(path,formantsig);

      } else {
	printf("Problems in formant tracker\n");
	exit(-1);
      }
      if(formantsig)free_signal(formantsig);
      if(polesig) free_signal(polesig);
    }
    if(f0sig)
	free_signal(f0sig);
    if(spsig && (spsig != f0sig))
	free_signal(spsig);
    if(hpsig && (hpsig != spsig) && (hpsig != f0sig))
	free_signal(hpsig);
    if(dssig && (dssig && hpsig) && (dssig != spsig) && (dssig != f0sig))
	free_signal(dssig);
    *station = 0;
  }
}

/*      ----------------------------------------------------------      */
free_dp_cp(dp,cp,n)
     CROSS **cp;
     DPREC **dp;
     int n;
{
  int i;
  for(i=0; i < n; i++) {
    free(cp[i]->correl);
    free(cp[i]);
    free(dp[i]->locs);
    free(dp[i]->prept);
    free(dp[i]->mpvals);
    free(dp[i]->pvals);
    free(dp[i]->dpvals);
    free(dp[i]);
  }
  free(cp);
  free(dp);
}


/*      ----------------------------------------------------------      */
Signal
*highpass(s)
     Signal *s;
{

  short *datain, *dataout;
  static short *lcf;
  static int len = 0;
  short **dpp;
  double scale, fn;
  register int i;
  int it;
  Signal *so;
  char *cp,temp[200];
  Header *h, *dup_header();

#define LCSIZ 101
  /* This assumes the sampling frequency is 10kHz and that the FIR
     is a Hanning function of (LCSIZ/10)ms duration. */

  if(s) {
    cp = new_ext(s->name,"hp");
    if((dpp = (short**)malloc(sizeof(short*))) &&
       (dpp[0] = dataout = (short*)malloc(sizeof(short) * s->buff_size)) &&
       (so=new_signal(cp,SIG_UNKNOWN,dup_header(s->header),dpp,s->buff_size,s->freq,s->dim))) {
       if(!len) {		/* need to create a Hanning FIR? */
	 lcf = (short*)localloc(sizeof(short) * LCSIZ);
	 len = 1 + (LCSIZ/2);
	 fn = PI * 2.0 / (LCSIZ - 1);
	 scale = 32767.0/(.5 * LCSIZ);
	 for(i=0; i < len; i++)
	   lcf[i] = scale * (.5 + (.4 * cos(fn * ((double)i))));
       }
       datain = ((short**)s->data)[0];
       do_fir(datain,s->buff_size,dataout,len,lcf,1); /* in downsample.c */
       so->start_time = s->start_time + (((double)s->start_samp)/s->freq);
       it = s->version +1;
       head_printf(so->header,"version",&it);
       sprintf(temp,"highpass: filter Hanning nsamples %d signal %s",
	       LCSIZ,s->name);
       head_printf(so->header,"operation",temp);
       head_printf(so->header,"time",get_date());
       return(so);
     } else
       printf("Can't make a new Signal in highpass()\n");
  } else
    printf("Null signal passed to highpass()\n");
  return(NULL);
}

#ifdef OBSOLETE_VERSION
/*      ----------------------------------------------------------      */
/* Find the roots of the LPC denominator polynomial and convert the z-plane
	zeros to equivalent resonant frequencies and bandwidths.	*/
/* The complex poles are then ordered by frequency.  */
formant(lpc_order,s_freq,lpca,n_form,freq,band,init)
int	lpc_order, /* order of the LP model */
	*n_form,   /* number of COMPLEX roots of the LPC polynomial */
	init; 	   /* preset to true if no root candidates are available */
double	s_freq,    /* the sampling frequency of the speech waveform data */
	*lpca, 	   /* linear predictor coefficients */
	*freq,     /* returned array of candidate formant frequencies */
	*band;     /* returned array of candidate formant bandwidths */
{
  double  x, flo, pi2t, theta;
  static double  rr[31], ri[31];
  int	i,ii,iscomp1,iscomp2,fc,swit;

  if(debug & 4) {
    printf("formant: lpc_order:%d",lpc_order);
    for(i=0;i<=lpc_order;i++) printf("%9.5f",lpca[i]);
    printf("\n");
  }

  if(init){ /* set up starting points for the root search near unit circle */
    x = PI/(lpc_order + 1);
    for(i=0;i<=lpc_order;i++){
      flo = lpc_order - i;
      rr[i] = 2.0 * cos((flo + 0.5) * x);
      ri[i] = 2.0 * sin((flo + 0.5) * x);
    }
  }
  if(! lbpoly(lpca,lpc_order,rr,ri)){ /* find the roots of the LPC polynomial */
    *n_form = 0;		/* was there a problem in the root finder? */
    return(FALSE);
  }

  pi2t = PI * 2.0 /s_freq;

  /* convert the z-plane locations to frequencies and bandwidths */
  for(fc=0, ii=0; ii < lpc_order; ii++){
    if((rr[ii] != 0.0)||(ri[ii] != 0.0)){
      theta = atan2(ri[ii],rr[ii]);
      freq[fc] = fabs(theta / pi2t);
      if((band[fc] = 0.5 * s_freq *
	  log(((rr[ii] * rr[ii]) + (ri[ii] * ri[ii])))/PI) < 0.0)
	band[fc] = -band[fc];
      fc++;			/* Count the number of real and complex poles. */

      if((rr[ii] == rr[ii+1])&&(ri[ii] == -ri[ii+1]) /* complex pole? */
	 && (ri[ii] != 0.0)) ii++; /* if so, don't duplicate */
    }
  }


  /* Now order the complex poles by frequency.  Always place the (uninteresting)
     real poles at the end of the arrays. 	*/
  theta = s_freq/2.0;		/* temporarily hold the folding frequency. */
  for(i=0; i < fc -1; i++){	/* order the poles by frequency (bubble) */
    for(ii=0; ii < fc -1 -i; ii++){
      /* Force the real poles to the end of the list. */
      iscomp1 = (freq[ii] > 1.0) && (freq[ii] < theta);
      iscomp2 = (freq[ii+1] > 1.0) && (freq[ii+1] < theta);
      swit = (freq[ii] > freq[ii+1]) && iscomp2 ;
      if(swit || (iscomp2 && ! iscomp1)){
	flo = band[ii+1];
	band[ii+1] = band[ii];
	band[ii] = flo;
	flo = freq[ii+1];
	freq[ii+1] = freq[ii];
	freq[ii] = flo;
      }
    }
  }
  /* Now count the complex poles as formant candidates. */
  for(i=0, theta = theta - 1.0, ii=0 ; i < fc; i++)
    if( (freq[i] > 1.0) && (freq[i] < theta) ) ii++;
  *n_form = ii;
  if(debug & 4) {
    int j;
    printf("#poles:%4d  ",ii);
    for(j=0;j<ii;j++)
      printf("%7.0f",freq[j]);
    printf("\n             ");
    for(j=0;j<ii;j++)
      printf("%7.0f",band[j]);
    printf("\n");
  }
  return(TRUE);
}
#endif

/*************************************************************************/
double integerize(time, freq)
     register double time, freq;
{
  register int i;

  i = .5 + (freq * time);
  return(((double)i)/freq);
}


char *localloc(n)
     int n;
{
  char *p;

#if !defined(IBM_RS6000) && !defined(HP400) && !defined(HP700) && !defined(DS3100) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *malloc();
#endif

  if((p = malloc(n))) return p;

  printf("\nCan't allocate %d more bytes of memory in localloc()\n",n);
  exit(-1);
}

notify(s,x,y)
     char *s;
     int x,y;
{
  printf("%s\n",s);
}

/*	Round the argument to the nearest integer.			*/
Round(flnum)
register double	flnum;
{
	return((flnum >= 0.0) ? (int)(flnum + 0.5) : (int)(flnum - 0.5));
}

void
e_make_sd_hdr(newsig, oldsig, command_line)
Signal *newsig, *oldsig;
char *command_line;
{
  struct header *newhd, *oldhd;
  newhd = new_header(FT_FEA);
 (void) init_feasd_hd(newhd, SHORT, (int) 1,
		      &newsig->start_time, (int) 0, (double) newsig->freq);
  add_comment(newhd, command_line);
  strcpy (newhd->common.prog, "formant");
  strcpy (newhd->common.vers, VERSION);
  strcpy (newhd->common.progdate, DATE);
  if((newsig->type & VECTOR_SIGNALS) == P_SHORTS)
    newsig->header->e_scrsd = 1;
  *add_genhd_d("src_sf", (double *) NULL, 1, newhd) = oldsig->freq;
  newsig->header->esps_hdr = newhd;
}

void
e_make_fea_hdr(newsig, size, command_line)
Signal *newsig;
int    size;
char *command_line;
{
  int j;
  struct header *newhd;

  newsig->type = P_MIXED;
  newsig->types = malloc_i(size);
  for (j=0; j<size; j++)
    newsig->types[j] = P_DOUBLES;
  newhd = new_header(FT_FEA);
  add_comment(newhd, command_line);
  strcpy (newhd->common.prog, "formant");
  strcpy (newhd->common.vers, VERSION);
  strcpy (newhd->common.progdate, DATE);
  *add_genhd_d("start_time", (double *) NULL, 1, newhd) = newsig->start_time;
  *add_genhd_d("record_freq", (double *) NULL, 1, newhd) = newsig->freq;
  newsig->header->esps_hdr = newhd;
}

char *new_sig_name(path,old)
     char *path, *old;
{
  static char newname[1024];
  char *basename();

  if(path && *path && old && *old) {
    strcpy(newname, path);
    if(path[strlen(path)-1] != '/')
      strcat(newname,"/");
    strcat(newname,basename(old));
    return(newname);
  }
  return(old);
}

put_signal_to_path(path,sig)
     char *path;
     Signal *sig;
{
  int itsok = FALSE;

  if(sig) {
    char *nhold = sig->name;

    sig->name = new_sig_name(path,nhold);
    itsok = put_signal(sig);
    sig->name = nhold;
  }
  return(itsok);
}
	
Signal *get_signal_from_path(path,name,start,size,proc)
     char *path, *name;
     double start, size;
     void *proc;
{

  return(get_signal(new_sig_name(path,name),start,size,proc));
}
