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
 * Brief description: synthesize from excitation signal or from parametric
 *                    source
 *
 */

static char *sccs_id = "@(#)lp_syn.c	1.13 1/22/97 ERL";

char *ProgName = "lp_syn";
char *Version = "1.13";
char *Date = "1/22/97";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/lpsyn.h>
#ifdef DEC_ALPHA
#include <float.h>
#endif

#define SYNTAX USAGE("lp_syn [-P param_file][-{pr} range][-s range][-i int_const][-t synth_rate][-f f0_ratio][-u up_ratio][-d damp][-z][-x debug_level] lpfile exfile outfile")
#define ERROR_EXIT(text){(void) fprintf(stderr,"%s: %s - exiting\n", \
					av[0], text); exit(1);}
int debug_level = 0;
double  rc_gain(), lattice();

main(ac,av)
     int ac;
     char **av;
{
  extern char *optarg;
  extern int optind;

  char *pfname, *efname, *ofname, *range=NULL;
  FILE *pfile, *efile, *ofile;
  struct header *phd, *ehd, *ohd;
  struct fea_data *pfea_rec, *efea_rec;
  struct feasd *osd_rec, *esd_rec;
  long s_rec, e_rec;
  float *pdata;
  double *pdata_d;
  double *rmsdata, *probdata, *f0data;
  double *dpdata = NULL, *f0, *prob, *rms;
  short *edata, *odata, rcdatatype;
  char *param_file = NULL, *sym;
  double stime, pfreq, efreq, pstart_time, estart_time, src_sf;
  int coef_size;

  short *ep, *epi, *op;
  register int i, j, k;
  double cint=.95;
  int c, nout, fn, is_lp=TRUE, sp_type, norm_gain, excite_signal,
     pdx=0, edx=0, actsize;
  short usr = 1;
  double start, end, lpc[MAX_ORD], lpm[MAX_ORD], ref[MAX_ORD], scr[MAX_ORD],
         damp=.1, f0scale = 1.0, rate = 1.0;
  LP_frame  *lp0=NULL, *lpp=NULL;
  Pblock *pb;

  int iflag=0, rflag=0, sflag=0, fflag=0, tflag =0, uflag=0, dflag=0, 
     parcorflag=0;
  /* parcorflag=1 - use conventional PARCOR sign convention.
     parcorflag=0 - use ESPS refcof sign convention.
     synthesis routines used here assueme conventional PARCOR sign convention*/
  char *get_cmd_line(), *cmd_line;
  void get_range();

  while((c = getopt(ac,av,"P:f:i:d:u:t:x:p:r:s:z")) != EOF) {
    switch(c) {
    case 'P':
      param_file = optarg;
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
    case 'i':
      cint = atof(optarg);
      iflag++;
      break;
    case 't':
      rate = atof(optarg);
      tflag++;
      break;
    case 'f':
      f0scale = atof(optarg);
      fflag++;
      break;
    case 'u':
      usr = atoi(optarg);
      uflag++;
      break;
    case 'd':
      damp = atof(optarg);
      dflag++;
      break;
    case 'z':
      parcorflag = 1;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX;
      exit(1);
    }
  }

  if((ac -optind) != 3) {
    SYNTAX;
    exit(1);
  }

  (void) read_params(param_file, SC_NOCOMMON, (char *) NULL);

  if(!iflag && symtype("int_const") != ST_UNDEF)
    cint = getsym_d("int_const");
  if(!tflag && symtype("synth_rate") != ST_UNDEF)
    rate = getsym_d("synth_rate");
  if(!fflag && symtype("f0_ratio") != ST_UNDEF)
    f0scale = getsym_d("f0_ratio");
  if(!uflag && symtype("up_ratio") != ST_UNDEF)
    usr = getsym_i("up_ratio");
  if(!dflag && symtype("damp") != ST_UNDEF)
    damp = getsym_d("damp");

  /* 6/4/93: Assumed F0, rms, prob_voice data are DOUBLE as it has always been
     in formant f0 file, but spec_param data may be FLOAT (as produced
     by refcof), or DOUBLE (as the way it should have been) */
  /* read spectral parameter file */
  pfname = eopen(av[0], av[optind], "r", NONE, NONE, &phd, &pfile);
  get_range( &s_rec, &e_rec, range, rflag, sflag, phd);
  pstart_time = get_genhd_val("start_time", phd, 0.0);
  pfreq = get_genhd_val("record_freq", phd, 200.0);
  src_sf = get_genhd_val("src_sf", phd, 2.0 * get_genhd_val("bandwidth", phd, 5000.0));
  sp_type = *(short *) get_genhd("spec_rep", phd);
  is_lp = (sp_type == RC)? FALSE : TRUE;
  pfea_rec = allo_fea_rec( phd );
  rcdatatype = get_fea_type("spec_param", phd);
  if( rcdatatype == FLOAT) 
    pdata = (float *) get_fea_ptr(pfea_rec, "spec_param", phd);
  else
    pdata_d = (double *) get_fea_ptr(pfea_rec, "spec_param", phd);
  coef_size = get_fea_siz("spec_param", phd, NULL, NULL);

  /* read excitation file */
  efname = eopen(av[0], av[optind+1], "r", NONE, NONE, &ehd, &efile);
  estart_time = get_genhd_val("start_time", ehd, 0.0);
  efreq = get_genhd_val("record_freq", ehd, 200.0);
  if (ehd->hd.fea->fea_type == FEA_SD){      /* direct excitation signal */
    int buflen;
    int	nperf = 0.5 + efreq/pfreq;
    buflen = 1000*nperf;
    esd_rec = allo_feasd_recs(ehd, SHORT, buflen, NULL, NO);
    edata = (short *) esd_rec->data;
    norm_gain = FALSE;
    excite_signal = TRUE;
    if(debug_level) fprintf(stderr, 
	 "%s: Using %s as the excitation signal\n",av[0], efname);
  }
  else{
    efea_rec = allo_fea_rec(ehd);        /* parameteric excitation */
    probdata = (double *) get_fea_ptr(efea_rec, "prob_voice", ehd);
    f0data = (double *) get_fea_ptr(efea_rec, "F0", ehd);
    rmsdata = (double *) get_fea_ptr(efea_rec, "rms", ehd);
    norm_gain = TRUE;
    excite_signal = FALSE;
  }
  pdx = s_rec - 1;
  if( (stime = pstart_time + (s_rec-1)/pfreq ) < estart_time){
    pdx += (int) (0.5 + (estart_time - stime) * pfreq);
    stime = estart_time;
  }
  else
    edx = (int) (0.5 + (stime - estart_time) * efreq);
  if ((phd->common.ndrec != -1 && pdx + 1 >= phd->common.ndrec)
      || (ehd->common.ndrec != -1 && edx + 1 >= ehd->common.ndrec)) 
    ERROR_EXIT("Specified input range is out of bound");
  fea_skiprec(pfile, pdx, phd);
  fea_skiprec(efile, edx, ehd);

  /* setup output FEA_SD file */
  ofname = eopen(av[0], av[optind+2], "w", NONE, NONE, &ohd, &ofile);
  cmd_line = get_cmd_line(ac, av);
  ohd = new_header(FT_FEA);
  (void) strcpy (ohd->common.prog, ProgName);
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  ohd->common.tag = NO;
  add_source_file(ohd, pfname, phd);
  add_source_file(ohd, efname, ehd);
  add_comment(ohd, cmd_line);

  if((init_feasd_hd(ohd, SHORT, 1, &stime, 0, src_sf * usr)) !=0){
    fprintf(stderr, "lp_syn: Couldn't allocate FEA_SD header - exiting.\n");
	  exit(1);
  }
  if( !excite_signal){
    (void) add_genhd_d("int_const", &cint, 1, ohd);
    (void) add_genhd_d("synth_rate", &rate, 1, ohd);
    (void) add_genhd_d("f0_ratio", &f0scale, 1, ohd);
    (void) add_genhd_s("up_ratio", &usr, 1, ohd);
    (void) add_genhd_d("damp", &damp, 1, ohd);
  }
  write_header(ohd, ofile);

  if(excite_signal) {
    int buflen;
    int	nperf = 0.5 + efreq/pfreq;
    int total_samps=0, total_pframes=0;
    buflen = 1000*nperf;
    fn = 0;
    total_pframes = e_rec - s_rec + 1;
    while(actsize = get_feasd_recs(esd_rec,(long)0L, buflen, ehd, efile)){
      total_samps += actsize;
      for(j=0, ep=edata; j<actsize; j += nperf, ep += nperf) {
	fn++;
	if( get_fea_rec( pfea_rec, phd, pfile) == EOF || fn > total_pframes)
	  break;
	epi = (short *) malloc(sizeof(short) * nperf);
	spsassert(epi, "malloc failed");
	for(i=0; i<nperf; i++) epi[i] = ep[i];
	dpdata = (double *) malloc(sizeof(double) * coef_size);
	spsassert(dpdata, "malloc failed");

	if( rcdatatype == FLOAT)
	for(i=0;i<coef_size;i++) 
	  dpdata[i] = (parcorflag) ? pdata[i]: -pdata[i];
      else
	for(i=0;i<coef_size;i++) 
	  dpdata[i] = (parcorflag) ?pdata_d[i]: -pdata_d[i] ;

	if(!(lpp = link_to_new_lp(dpdata,coef_size,0.0,0.0,0.0,epi,nperf,0.0,0,
				  EX_SHORT_RESID,sp_type,lpp))) {
	  fprintf(stderr,"lp_syn: problems at fn:%d\n",fn);
	  exit(-1);
	}
	if(!lp0) {
	  lp0 = lpp;
	}
      }
    }
    nout = total_samps;
  } else {      /* prepare for parametric source synthesis */
    double ft = 1.0/(rate*pfreq), ftime, fratio = efreq/pfreq;
    double period = 0.01, rms = 0.0, prob = 0.0;
    long en = -1, pn = -1, fn=0, total_pframes;
    int size;
    ftime = stime;
    total_pframes = e_rec - s_rec + 1;
    while( get_fea_rec(pfea_rec, phd, pfile) != EOF && total_pframes-- ){
      pn++;
      if( get_fea_rec(efea_rec, ehd, efile) == EOF) break;
      else en++;
      dpdata = (double *) malloc(sizeof(double) * coef_size);
      spsassert(dpdata, "malloc failed");

      if( rcdatatype == FLOAT)
	for(i=0;i<coef_size;i++) 
	  dpdata[i] = (parcorflag) ? pdata[i]: -pdata[i];
      else
	for(i=0;i<coef_size;i++) 
	  dpdata[i] = (parcorflag) ?pdata_d[i]: -pdata_d[i] ;
      if (*f0data > 0.0)
	period = 1.0/(f0scale * (*f0data));
      else
	period = 0.01;  /* punt ehsn nominal F0 is zero */

      rms = *rmsdata;
      prob = *probdata;
      if(!(lpp = link_to_new_lp(dpdata,coef_size, period,
				0.0, rms, NULL,0, ftime,
				(double)(prob > 0.5) ? VOICED : 
				UNVOICED, EX_IMPULSE,sp_type,lpp))) {
	fprintf(stderr,"lp_syn: problems at fn:%d\n",fn);
	exit(-1);
	};
      if(!lp0) {
	lp0 = lpp;
      }
      ftime += ft;
      fn++;
    }
    pb = new_pblock(lp0,lpp, src_sf, lpp->time - lp0->time,
		    lp0->time, fn, NULL,NULL);
    nout = ((en + 1)/ efreq ) * usr * src_sf / rate;
  }

  osd_rec = allo_feasd_recs( ohd, SHORT, nout, NULL, NO);
  odata = (short *) osd_rec->data;

  if(is_lp) /* WRONG!  Should convert to RC then use rcsyn() */
    lpsyn(lp0,1,odata); /* broken!! (seems to work with external excitation) */
  else {
    if(excite_signal)
      rcsyn0(lp0,1,odata);
    else
      rcsyn(pb, 1, odata, damp, cint, usr );
  }
 
  put_feasd_recs( osd_rec, 0L, nout, ohd, ofile);

  exit(0);
}

lpsyn(lpf,init,op) 
     register short *op;
     register int init;
     LP_frame *lpf;
{
  register int i, j, k, ordd;
  register double sum;
  register short *ep;
  static double lpm[MAX_ORD], *mp1, *mp2, *cp;

  if(init)
    for(i=MAX_ORD, mp1=lpm; i--; ) *mp1++ = 0.0;
  while(lpf) {
    ordd = lpf->order;
    for(k=lpf->n, ep=lpf->ex; k--;) {
      for(sum=0.0,i=ordd, cp=lpf->coef + ordd-1,mp1=lpm+ordd,mp2=mp1-1;
	  i--; )  sum += *cp-- * (*mp1-- = *mp2--);
      *mp1 = -(sum += *ep++);
      *op++ = (sum > 0.0)? sum + .5 : sum - .5;
    }
    lpf = lpf->next;
  }
}


rcsyn0(rcf,init,op)
     register short *op;
     register int init;
     LP_frame *rcf;
{
  register int i, j, k, ordd;
  register double sum, gain;
  register short *ep;
  static double lpm[MAX_ORD], *mp1, *mp2, *cp;

  if(init)
    for(i=MAX_ORD, mp1=lpm; i--; ) *mp1++ = 0.0;
  while(rcf) {
    ordd = rcf->order;
    gain = rc_gain(rcf->coef,ordd);
    for(k=rcf->n, ep=rcf->ex; k--;) {
      sum = lattice(rcf->coef, ordd, lpm, gain * *ep++, 1);
      *op++ = (sum > 0.0)? sum + .5 : sum - .5;
    }
    rcf = rcf->next;
  }
}

dfwdflt(sigin,sigout,nsig,filcof,ncof)
     double *sigin,*sigout,*filcof; int *nsig,*ncof;
{
  static double *psin,*psinl;
  static double *psou1,*psou2,*pcof,*pcofl;
  pcofl = filcof + *ncof + 1;
  psinl = sigin + *nsig;
  psou1 = sigout;
  for(psin=sigin;psin<psinl;psin++,psou1++)
    {
      psou2 = psou1 - 1;
      *psou1 = *psin;
      for(pcof=filcof+1;pcof<pcofl;pcof++,psou2--)
	*psou1 += *psou2 * *pcof;
    }
}

dlpcref(a,s,c,n)
     double *a,*c,*s; int *n;
{
  /*	convert lpc to ref
	a - lpc
	c - ref
	s - scratch
	n - no of cofs
	*/
  register double *pa1,*pa2,*pa3,*pa4,*pc;
  double ta1,rc;

  pa2 = s + *n + 1;
  pa3 = a;
  for(pa1=s;pa1<pa2;pa1++,pa3++) *pa1 = *pa3;
  pc = c + *n -1; 
  for(pa1=s+*n;pa1>s;pa1--,pc--)
    {
      *pc = *pa1;
      rc = 1. - *pc * *pc;
      pa2 = s + (pa1-s)/2;
      pa4 = pa1-1;
      for(pa3=s+1;pa3<=pa2;pa3++,pa4--)
	{
	  ta1 = (*pa3 - *pc * *pa4)/rc;
	  *pa4 = (*pa4 - *pc * *pa3)/rc;
	  *pa3 = ta1;
	}
    }
}

quadsol(a,b,c,r1r,r1i,r2r,r2i) /* find x, where a*x**2 + b*x + c = 0 	*/
     double	a, b, c;
     double *r1r, *r2r, *r1i, *r2i; /* return real and imag. parts of roots */
{
  double  sqrt(), numi;
  double  den, y;

  if(a == 0.0){
    if(b == 0){
      printf("Bad coefficients to _quad().\n");
      return(FALSE);
    }
    *r1r = -c/b;
    *r1i = *r2r = *r2i = 0;
    return(TRUE);
  }
  den = 2.0 * a;
  numi = b*b -(4.0 * a * c);
  if(numi >= 0.0){
    *r1i = *r2i = 0.0;
    y = sqrt(numi);
    *r1r = (-b + y)/den;
    *r2r = (-b - y)/den;
    return(TRUE);
  }
  else {
    *r1i = sqrt( -numi )/den;
    *r2i = -*r1i;
    *r2r = *r1r = -b/den;
    return(TRUE);
  }
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
