/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  David Talkin, Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *          performs a pitch-synchronous LP analysis
 *
 */

static char *sccs_id = "@(#)ps_ana.c	1.14	08 Aug 1997	ERL";

char *ProgName = "ps_ana";
char *Version = "1.14";
char *Date = "08 Aug 1997";

#include <math.h>
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/window.h>
#include <esps/ana_methods.h>
#include <esps/lpsyn.h>
#include <Objects.h> /* include object.h/labels.h only for Vlist declaration */
#include <labels.h>

#define F_TYPE float
/* #define LOWEST_F0 50.0 */
#define LOWEST_F0 30.0

#define SYNTAX USAGE("ps_ana [-P param_file][-f out_f0file][-{pr} range][-s range][-o order][-S frame_step][-i frame_step][-c spec_rep][-m spec_method][-w window][-e preemphasis][-z][-x debug_level] in_data in_pulse outfile")
#define ERROR_EXIT(text){(void) fprintf(stderr,"%s: %s - exiting\n", \
                                        ProgName, text); exit(1);}

int debug_level=0;
FILE *fdeb=NULL;
static long n_rec();

/********************************************************************/
LP_frame *do_analysis(data, start_time, end_time, freq,par,time)
     float *data;
     double start_time, end_time, freq;
     Aparams *par;
     double time;
{
  int i, j, k, size, order, ordd;
  float *p;
  double *ret, coeffs[50], rms, rmse, wsize, rc[50], scr[50], index, wcent;
  short *sp;
  int wtype;

  if(par && data) {
    if(!(order = par->order))	/* SIDE EFFECT ON par! */
      par->order = order = 2 + 0.5 + freq/1000.0;
    switch(par->type) {
    case VOICED:
      /* The idea here is to make the analysis window ps_wsize pitch periods
	 long and centered 1/4 of a period after the main excitation point
	which is assumed to be at (time - par->period). */
      if((par->method == AM_COV))
	index = time - par->period + 2.0/freq; /* just let it start at the epoch */
      else
	index = time - par->period*(par->phase + par->ps_wsize/2.0);
      /* index - start of region, time - end of current epoch */
      if(index < start_time) {
	index = start_time;
	wsize = time - index;
      } else
	wsize = par->ps_wsize * par->period;
      if(index+wsize > end_time)
	wsize = end_time - index;
      size = 0.5 + wsize*freq;
      if(size < (1 + order*2))
	return(NULL);
      break;
    case UNVOICED:
      if((wsize = par->per_wsize)+(index=time) > end_time)
	index = end_time - wsize;
      size = 0.5 + wsize*freq;
      break;
    default:
      printf("Unknown type (%d) in do_analysis()\n",par->type);
      return(NULL);
    }

    /* now 'data' is float */
    p = data + (int)(0.5 + (index-start_time)*freq);
  /* Note that the window routines assume n+1 data points are in the buffer. */
    sp = (short *) malloc(sizeof(short)*(size+1));
    for(i=0; i <= size; i++) sp[i] = p[i];

    /* should really used clean compute_rc() and windows.h definitions */
    /* do pre-emphasis before compute_rc() <-> lpc() */
    wtype = W_HANN;
    if(par->wtype == WT_RECT) wtype = W_RECT;
    if(par->wtype == WT_HAMMING) wtype = W_HAMM;
    if(par->wtype == WT_COS4) wtype = W_COS4;
    switch(par->returned) {
      /* in covariance method, calling covar2() won't work */
    case AFC:
      switch(par->method){
      case AM_AUTOC:
	lpc(order,par->stab,size,sp,coeffs,NULL,NULL,&rmse, &rms,
	    par->preemp, wtype);  /*3 for window type */
	ret = coeffs+1;
	break;
      case AM_COV:
	ordd = order;
	w_covar(sp, &ordd, size, 0, coeffs, &rmse, &rms, par->preemp, wtype);
	rms = sqrt(rms/(size-order));
	ret = coeffs+1;
	break;
      default:
	printf("Unknown analysis method (%d) requested in do_analysis()\n",
	       par->returned);
	return(NULL);
	break;
      }
      break;
    case RC:
      switch(par->method){
      case AM_AUTOC:
	lpc(order,par->stab,size,sp,NULL,NULL,coeffs,&rmse, &rms,
	    par->preemp,wtype);
	ret = coeffs;
	break;
      case AM_COV:
	ordd = order;
	w_covar(sp, &ordd, size, 0, coeffs, &rmse, &rms, par->preemp, wtype);
	dlpcref(coeffs,scr,rc,&ordd);
	for(i=ordd;i<order;i++) rc[i] = 0.0;
	rms = sqrt(rms/(size-order));
	ret = rc;
	break;
      default:
	printf("Unknown analysis method (%d) requested in do_analysis()\n",
	       par->returned);
	return(NULL);
	break;
      }
      break;
    default:
      printf("Unknown analysis type (%d) requested in do_analysis()\n",
	     par->returned);
      return(NULL);
      break;
    }
    wcent = index+((double)size)/(2.0 * freq);
/*    if(!fdeb) {
      fdeb = fopen("wcenters","w");
      fprintf(fdeb,"#\n");
    }
    fprintf(fdeb,"%20.10f 121\n",wcent);*/
    return(copy_to_new_lp(ret,order,par->period,wsize,rms,NULL,0,
			  index+((double)size)/(2.0 * freq),par->type,
			  EX_NONE, par->returned, NULL));
  } else
    printf("Bad data (%d) or parameter block (%d) in do_analysis()\n",data,par);
  return(NULL);
}

/*    case SP_FORM:
      printf("Formant analysis not implemented yet in do_analysis()\n");
      return(NULL); */



/********************************************************************/
Pblock *analyze(idata, nsamp, start_time, freq, times, par)
     float *idata;
     long nsamp;
     double start_time, freq;
     Vlist *times;
     Aparams *par;
{
  int i, j, n_lpframes, nframes;
  double tstart, tend, end_time, size, wsize, pseg, pend,
     index, maxsize, pperiod;
  LP_frame *lp_first, *lp_last, *new_frame;

  if(!(idata && times && par)) return(NULL);
  end_time = start_time + nsamp / freq;
  tstart = start_time;
  lp_first = lp_last = NULL;
  n_lpframes = 0;
  pperiod = -100.0;
  maxsize = 1.0/LOWEST_F0;
  while(times) {
    if(debug_level)
      printf("*T=%f	",times->end);
    if((tend = times->end) > end_time)
      tend = end_time;
    if((size = tend-tstart) > maxsize) { /* do periodic analysis */
      if(debug_level)
	printf("PERIODIC (tstart=%f tend=%f)",tstart,tend);
      par->type = UNVOICED;
      par->period = 0;
      if((index = tstart - (par->per_wsize/2.0)) < start_time)
	index = start_time;
      pend = tstart+size;                 /* end of unvoiced region */
      nframes = 1 + (pend - index - par->per_wsize/2.0)/par->per_step;
      for(i=0; i < nframes; i++, index += par->per_step) {
	new_frame = do_analysis(idata,start_time, end_time, freq, par,index);
	new_frame->prev = lp_last;
	if(!lp_first) {
	  lp_first = lp_last = new_frame;
	} else {
	  lp_last->next = new_frame;
	  lp_last = new_frame;
	}
	n_lpframes++;
      }
      tstart = new_frame->time + new_frame->wsize/2.0;
      pperiod = tend;
      if(debug_level)
	printf("(new tstart=%f  pperiod=%f)\n",tstart,pperiod);
    } else {			/* do pitch-synchronous analysis */
      if(debug_level)
	printf("P-S (tstart=%f tend=%f pperiod=%f)",tstart,tend,pperiod);
      if(pperiod > 0.0) {
	par->type = VOICED;
	par->period = tend-pperiod;
	if(new_frame = do_analysis(idata,start_time, end_time, freq, par, tend)) {
	  new_frame->prev = lp_last;
	  if(!lp_first) {
	    lp_first = lp_last = new_frame;
	  } else {
	    lp_last->next = new_frame;
	    lp_last = new_frame;
	  }
	  n_lpframes++;
	  if((tstart = new_frame->time + new_frame->wsize/2.0) < tend)
	    tstart = tend;
	} else
	  printf("\n>>do_analysis() failure at tend=%f; pperiod=%f; tstart=%f\n", tend, pperiod, tstart);
      } else
	tstart = tend;
      pperiod = tend;
      if(debug_level)
	printf("(new tstart=%f  pperiod=%f)\n",tstart,pperiod);
    }
    times = times->next;
  }				/* finished segment list. */
  return(new_pblock(lp_first,lp_last, freq,lp_last->time - lp_first->time,
		    lp_first->time,n_lpframes,NULL,NULL));
}

double rate = 1.0, f0=1.0, damp=0.05, intc = -1.0, syfreq = 0.0;;

/********************************************************************/
main(ac,av)
     int ac;
     char **av;
{
  extern char *optarg;
  extern int optind;
  void get_range();
  char *pfname, *ifname, *ofname, *ffname=NULL; /*pulse, in, out, F0 */
  FILE *pfile, *ifile, *ofile, *ffile;
  struct header *phd, *ihd, *ohd, *fhd;
  long	pnrec, inrec;
  struct feasd *isd_rec, *psd_rec;
  struct feaana *ana_rec;
  struct fea_data *ofea_rec, *ffea_rec;
  long s_rec, e_rec, s_prec, e_prec, total_samps;
  float *idata;
  double *pdata, *rmsdata, *probdata, *f0data;
  char *param_file = NULL, c, *sym;
  char *range = NULL;

  double istart_time, pstart_time, ntime, ifreq,
     end_time, ofreq, band;
  register int i, j, k;
  int stepsams = -1;
  double step=.005, time;
  int noframes;

  Vlist *vl=NULL, *v, *ptime_from_file();
  Aparams *par;
  Pblock *pb;
  LP_frame *fp, *fbp;

  int oflag=0, cflag=0, mflag=0, wflag=0, pflag=0, uflag=0, vflag=0, sflag =0,
      bflag=0, aflag=0, fflag=0, iflag = 0, rflag=0;
  int parcorflag = 0;
  /* By default ps_ana produces conventional PARCOR coefficients,
     parcorflag = 1 puts coeffs in refcof form, opposite of PARCOR.*/

  par = (Aparams *) malloc(sizeof(Aparams));
  par->type = 0;
  par->order = 0;
  par->returned = RC;
  par->method = AM_AUTOC;
  par->wtype = WT_HANNING;
  par->preemp = 0.95;
  par->per_step = 0.002;
  par->per_wsize = 0.01;
  par->ps_wsize = 1.0;       /* should be 0.5??? */
  par->stab = 70.0;
  par->phase = 0.75;

  while((c = getopt(ac,av,"P:x:f:o:c:m:w:p:r:s:S:i:e:z")) != EOF) {
    switch(c) {
    case 'P':
      param_file = optarg;
      break;
    case 'f':
      ffname = optarg;
      break;
    case 'p':
    case 'r':
      if( range ){
	fprintf(stderr,"Error: -r -p should not be used with -s\n");
	exit(0);
      }
      range = optarg;
      rflag++;
      break;
    case 's':
      if( range ){
	fprintf(stderr,"Error: -s should not be used with -r\n");
	exit(0);
      }
      range = optarg;
      sflag++;
      break;
    case 'o':
      par->order = atoi(optarg);
      oflag++;
      break;
    case 'c':
      par->returned = lin_search(spec_reps, optarg);
      spsassert(par->returned > -1, "spec_type not recognized");
      cflag++;
      break;
    case 'm':
      par->method = lin_search(ana_methods, optarg);
      spsassert(par->method > -1, "analysis method not recognized");
      mflag++;
      break;
    case 'w':
      par->wtype = win_type_from_name(optarg);
      spsassert(par->wtype != WT_NONE, "window type not recognized");
      wflag++;
      break;
    case 'e':
      par->preemp = atof(optarg);
      pflag++;
      break;
    case 'S':
      if( iflag){
	fprintf(stderr,"Error: -S should not be used with -i\n");
	exit(0);
      }
      stepsams = atoi(optarg);
      break;
    case 'i':
      if( stepsams > 0 ) {
	fprintf(stderr,"Error: -i should not be used with -S");
	exit(0);
      }
      step = atof(optarg);
      iflag++;
      break;
    case 'z':
      parcorflag = 1;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    }
  }
  if((ac-optind) !=3){
    SYNTAX;
    exit(1);
  }

  (void) read_params(param_file, SC_NOCOMMON, (char *)NULL);

  if(stepsams < 0 && !iflag && symtype("frame_step") != ST_UNDEF)
    step = getsym_d("frame_step");

  if(!oflag && symtype("order") != ST_UNDEF)
    par->order = getsym_i("order");
  if(!cflag && symtype("spec_rep") != ST_UNDEF){
    sym = getsym_s("spec_rep");
    par->returned = lin_search(spec_reps, sym);
    spsassert(par->returned > -1, "spec_type not recognized");
  }
  if(!mflag && symtype("spec_method") != ST_UNDEF){
    sym = getsym_s("spec_method");
    par->method = lin_search(ana_methods, sym);
    spsassert(par->method > -1, "analysis method not recognized");
  }
  if(!wflag && symtype("window") != ST_UNDEF){
    sym = getsym_s("window");
    par->wtype = win_type_from_name(sym);
    spsassert(par->wtype != WT_NONE, "window type not recognized");
  }
  if(!pflag && symtype("preemphasis") != ST_UNDEF)
    par->preemp = getsym_d("preemphasis");
  if( symtype("per_step") != ST_UNDEF)
    par->per_step = getsym_d("per_step");
  if( symtype("per_wsize") != ST_UNDEF)
    par->per_wsize = getsym_d("per_wsize");
  if( symtype("ps_wsize") != ST_UNDEF)
    par->ps_wsize = getsym_d("ps_wsize");
  if( symtype("stability") != ST_UNDEF)
    par->stab = getsym_d("stability");
  if( symtype("phase") != ST_UNDEF)
    par->phase = getsym_d("phase");

  /* read input file */
  ifname = eopen(av[0], av[optind], "r", FT_FEA, FEA_SD, &ihd, &ifile);
  get_range( &s_rec, &e_rec, range, rflag, sflag, ihd );
  if(debug_level)
    fprintf(stderr,"Input sample range: %d - %d\n", s_rec, e_rec);
  total_samps = e_rec - s_rec + 1;

  inrec = n_rec(&ifile, &ihd);
  if ( s_rec + total_samps -1 > inrec)
    total_samps = inrec - s_rec + 1;
  istart_time = *get_genhd_d("start_time", ihd);
  ifreq = *get_genhd_d("record_freq", ihd);
  band = ifreq / 2;
  isd_rec = allo_feasd_recs( ihd, FLOAT, total_samps, NULL, NO);
  idata = (float *) isd_rec->data;
  (void) fea_skiprec ( ifile , s_rec - 1,  ihd);
  get_feasd_recs( isd_rec, (long) 0L, total_samps, ihd, ifile);
  ntime = istart_time + (s_rec-1)/ifreq;

  if( stepsams > 0) step = stepsams / ifreq;

  /* read pulse file, should allow reading label file in case of
   hand-labelled pulses */
  pfname = av[optind+1];
  pfname = eopen("ps_ana", pfname, "r", FT_FEA, FEA_SD, &phd, &pfile);
  pstart_time = *get_genhd_d("start_time", phd);
  if( ifreq != *get_genhd_d("record_freq", phd)){
    fprintf(stderr,"Error: input speech and pulse files have different frequencies\n");
    exit(0);
  }
  s_prec = 1;
  if( pstart_time <= ntime ){
    s_prec = (int)(ifreq * ( ntime - pstart_time )) + 1;
    pstart_time = ntime;
  }
  e_prec = s_prec + total_samps - 1;
  pnrec = n_rec(&pfile, &phd);
  if( e_prec > pnrec) e_prec = pnrec;

  psd_rec = allo_feasd_recs( phd, FLOAT, e_prec-s_prec+1, NULL, NO);
  (void) fea_skiprec( pfile, s_prec-1, phd);
  get_feasd_recs( psd_rec, (long) 0L, e_prec-s_prec+1, phd, pfile);

  vl = ptime_from_file( pstart_time, ifreq, e_prec-s_prec+1, psd_rec );

  if(!vl) {
    vl = (Vlist*)malloc(sizeof(Vlist));
    vl->start = ntime;
    vl->end = ntime + total_samps / ifreq;
    vl->next = NULL;
  }

  if(debug_level){
    for(v=vl; v; v=v->next)
      fprintf(stderr,"start:%f end%f\n",v->start,v->end);
  }

  if(!(pb = analyze(idata, total_samps, ntime, ifreq, vl, par)))
    ERROR_EXIT("Problem in analyze()")

  /* prepare out_file */
  ofname = eopen(av[0], av[optind+2], "w", NONE, NONE, &ohd, &ofile);
  ohd = new_header(FT_FEA);
  (void) strcpy (ohd->common.prog, ProgName);
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  ohd->common.tag = NO;
  add_source_file(ohd,ifname,ihd);
  add_source_file(ohd,pfname,phd);
  add_comment(ohd,get_cmd_line(ac,av));

  add_fea_fld("spec_param", par->order, 1, NULL, DOUBLE, NULL, ohd);
  ofea_rec = allo_fea_rec(ohd);

  pdata = (double *) get_fea_ptr(ofea_rec,"spec_param", ohd);

  noframes = 1 + pb->dur/step;
  ofreq = 1.0/step;
  add_genhd_d("record_freq", &ofreq, 1, ohd);
  add_genhd_d("start_time", &pb->start_time, 1, ohd);
  add_genhd_d("bandwidth", &band, 1 , ohd);
  add_genhd_e("spec_rep", &par->returned, 1, spec_reps, ohd);
  add_genhd_e("spec_method", &par->method, 1, ana_methods, ohd);
  add_genhd_e("window", &par->wtype, 1, window_types, ohd);
  add_genhd_d("preemphasis", &par->preemp, 1, ohd);
  add_genhd_d("per_step", &par->per_step, 1, ohd);
  add_genhd_d("per_wsize", &par->per_wsize, 1, ohd);
  add_genhd_d("ps_wsize", &par->ps_wsize, 1, ohd);
  add_genhd_d("stability", &par->stab, 1, ohd);
  add_genhd_d("phase", &par->phase, 1, ohd);

  write_header(ohd, ofile);

  /* prepare out_f0file */
  if(ffname){
    ffname = eopen(av[0], ffname, "w", NONE, NONE, &fhd, &ffile);
    fhd = new_header(FT_FEA);
    (void) strcpy (fhd->common.prog, ProgName);
    (void) strcpy (fhd->common.vers, Version);
    (void) strcpy (fhd->common.progdate, Date);
    fhd->common.tag = NO;
    add_source_file(fhd,ifname,ihd);
    add_source_file(fhd,pfname,phd);
    add_comment(fhd,get_cmd_line(ac,av));

    add_fea_fld("F0", 1, 0, NULL, DOUBLE, NULL, fhd);
    add_fea_fld("prob_voice", 1,0, NULL, DOUBLE, NULL, fhd);
    add_fea_fld("rms", 1, 0, NULL, DOUBLE, NULL, fhd);
    ffea_rec = allo_fea_rec(fhd);

    f0data = (double *) get_fea_ptr(ffea_rec,"F0", fhd);
    probdata = (double *) get_fea_ptr(ffea_rec,"prob_voice", fhd);
    rmsdata = (double *) get_fea_ptr(ffea_rec,"rms", fhd);

    add_genhd_d("record_freq", &ofreq, 1, fhd);
    add_genhd_d("start_time", &pb->start_time, 1, fhd);
    add_genhd_e("spec_rep", &par->returned, 1, spec_reps, fhd);
    add_genhd_e("spec_method", &par->method, 1, ana_methods, fhd);
    add_genhd_e("window", &par->wtype, 1, window_types, fhd);
    add_genhd_d("preemphasis", &par->preemp, 1, fhd);
    add_genhd_d("per_step", &par->per_step, 1, fhd);
    add_genhd_d("per_wsize", &par->per_wsize, 1, fhd);
    add_genhd_d("ps_wsize", &par->ps_wsize, 1, fhd);
    add_genhd_d("stability", &par->stab, 1, fhd);
    add_genhd_d("phase", &par->phase, 1, fhd);

    write_header(fhd, ffile);
  }

  if(debug_level) {
    fprintf(stderr,"#\n");
    for(fp = pb->head; fp; fp=fp->next)
      fprintf(stderr,"%f\n",fp->time);
  }
  for(i=0, time = pb->start_time, fp= pb->head; i < noframes;
      i++, time += step) {
    while(fp->next && (fp->next->time < time)) fp = fp->next;
    if(fp->next) {
      if(fabs(time - fp->time) < fabs(time - fp->next->time))
	fbp = fp;
	else
	  fbp = fp->next;
    } else
      fbp = fp;

    for(j=0; j<par->order; j++)
      pdata[j] = (parcorflag) ? fbp->coef[j] : -fbp->coef[j];

    if(ffname){
      *rmsdata = fbp->rms;
      if( fbp->fr_type == VOICED) *probdata = 1.0;
      else *probdata = 0.0;
      if(fbp->period > 0.0) *f0data = 1.0 / fbp->period;
      else *f0data = 0;
      put_fea_rec( ffea_rec, fhd, ffile);
    }

    put_fea_rec( ofea_rec, ohd, ofile);
  }
}

Vlist *ptime_from_file( stime, pfreq, nrec, psd_rec )
     double stime, pfreq;
     long nrec;
     struct feasd *psd_rec;
{
  Vlist *vl = NULL, *vlt, *vl0 = NULL;
  double start;
  float *pdata;
  int i;

  start = stime;
  pdata = (float *) psd_rec->data;

  for(i = 0; i < nrec; i++){
    if( pdata[i] ){
      vlt = (Vlist *) malloc(sizeof(Vlist));
      vlt->start = start;
      vlt->end = stime + ((double)i) / pfreq;
      vlt->next = NULL;
      if( vl ) vl->next = vlt;
      vl = vlt;
      if(!vl0) vl0 = vlt;
      start = vlt->end;
    }
  }
  if( !pdata[nrec - 1] ){
    vlt = (Vlist *) malloc(sizeof(Vlist));
    vlt->start = start;
    vlt->end = stime + ((double)nrec) / pfreq;
    vlt->next = NULL;
    if( vl ) vl->next = vlt;
    vl = vlt;
    if(!vl0) vl0 = vlt;
    start = vlt->end;
  }
  return(vl0);
}


void get_range(srec, erec, rng, pflag, Sflag, hd)
/*
 * This function facilitates ESPS range processing.  It sets srec and erec
 * to their parameter/common values unless a range option has been used, in
 * which case it uses the range specification to set srec and erec.  If
 * there is no range option and if start and nan do not appear in the
 * parameter/common file, then srec and erec are set to 1 and LONG_MAX.
 * Get_range assumes that read_params has been called; If a command-line
 * range option (e.g., -r range) has been used, get_range should be called
 * with positive pflag and with rng equal to the range specification.
 */
long *srec;                     /* starting record */
long *erec;                     /* end record */
char *rng;                      /* range string from range option */
int pflag;                      /* flag for whether -r or -p used */
int Sflag;                      /* flag for whether -S used */
struct header *hd;              /* input file header */
{
    long common_nan;

    *srec = 1;
    *erec = LONG_MAX;
    if (pflag)
        lrange_switch (rng, srec, erec, 1);
    else if (Sflag)
        trange_switch (rng, hd, srec, erec);
    else {
        if(symtype("start") != ST_UNDEF) {
            *srec = getsym_i("start");
        }
        if(symtype("nan") != ST_UNDEF) {
            common_nan = getsym_i("nan");
            if (common_nan != 0)
                *erec = *srec + common_nan - 1;
        }
    }
}


/*
 * Get number of samples in a sampled-data file.
 * Replace input stream with temporary file if input is a pipe or
 * record size is not fixed.
 */

#define BUFSIZE 1000

static long
n_rec(file, hd)
    FILE **file;
    struct header **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe or has
				     * variable record length. */
    {
	FILE	*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	static double
		buf[BUFSIZE];
	int	num_read;
	long	ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);

	do
	{
	    num_read = get_sd_recd(buf, BUFSIZE, *hd, *file);
	    if (num_read != 0) put_sd_recd(buf, num_read, tmphdr, tmpstrm);
	    ndrec += num_read;
	} while (num_read == BUFSIZE);
	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}
