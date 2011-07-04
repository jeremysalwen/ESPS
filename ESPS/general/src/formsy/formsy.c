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
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:a crude, cascade, n-fromant, pitch-excited synthesizer
 *
 */

static char *sccs_id = "@(#)formsy.c	1.3	5/19/98	ERL";

/*
formsy [-P param_file][-b bandwidths][-r frame_rate_scale][-a formant_scale][-v voic_prob][-l min_bandwidth][-n n_formants][-s synthesis_rate][-h n_high_pole_formants][-f f0_scale][-g ][-e excite_file ] f0file fbfile outfile

Where "f0" is a file containing F0, VUV, and amplitude data, and "fb" contains
the formant frequencies and bandwidths.  The program "formant" produces
files of the required format.  These may be used as templates for
hand-generated or modified parameter files.  The "syn" file contains the
synthesis output.  The options are:
 -bn.m
   Instead of using the bandwidths from the "fb" file, bandwidths are
	synthesized from formant frequencies by the formula
		bandwidth = n + ( .m * frequency) [null].
 -vn
   The probability of voicing in the .f0 file is ignored and the value
	of n is used throughout the synthesis instead (0 <= n <= 1) [null].
 -ln
   The bandwidths from the "fb" file are constrained to be >= n.  The
	-b and -l options are mutually exclusive [null].
 -nn
   Only the first n formants in the "fb" file are used [null].
 -sn
   The synthesis frequency is set to nHz [10kHz].
 -hn
   n constant "higher-pole" formants are synthesized  [0].
 -an
   n is the factor by which formant frequencies are to be scaled [1.0]
 -fn
   The fundamental frequency from the "f0" file is multiplied by n [1.0].
 -rn
   The frame rate of both the "f0" and "fb" files are multiplied by n [1.0].
 -gn
   n is a boolean (1 or 0) which determines whether amplitude correction
	is applied after synthesis.  This tends to improve the quality
	in the voiced regions and degrade quality in unvoiced regions [0].
 -ef
   f is the name of a signal file to use for excitation [null].  If f is
	specified, the .f0 file must NOT be specified and the v, s, f, r and
	g options are not available; the synthesis frequency will be that
	of the excitation file.

Formsy is a simple n-formant synthesizer designed to help assess the
validity of formant and F0 estimates obtained from the program
"formant."  "Formsy" is not a serious attempt at quality synthesis and
should be used accordingly.  In particular, the voiced and unvoiced
excitation functions can be substantially improved and the amplitude
balance between voiced and unvoiced segments is not quite correct.
*/

#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <Objects.h>

#define NUPDATE 1
#define NFORM 10
#define SFREQ		16000.0

#define SYNTAX USAGE("formsy [-P param_file][-b bandwidths][-r frame_rate_scale][-a formant_scale][-v voice_prob][-l min_bandwidth][-n n_formants][-s synthesis_rate][-h n_high_pole_formants][-f f0_scale][-g][-e excite_file ][-x debug_level] f0file fbfile outfile")

int debug_level = 0, w_verbose = 0, max_buff_bytes = 2000000;

char	*ProgName = "formsy";
char	*Version = "1.3";
char	*Date = "5/19/98";

double  drand48();
void formsy();

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);
}
int
main(ac,av)
     int ac;
     char **av;
{
  int i, j, nsamples, fd, nform, in, nframes, fix_gain, nforce, high_pole,
  f0skip, fbskip, seskip, c;
  double limit, f0scale, rate, start, end;
  double fs = SFREQ, fscale;
  short *sig, **spp, *ep;	/* output buffer for the synthesized signal */
  double **dpp, *ff[NFORM], *fb[NFORM], *f0, *pv, *rms, frint, bws, vforce;
  char *cp;
  extern int optind;
  extern char *optarg;
  Signal *fbs, *f0s, *so, *se, *get_any_signal();
  char temp[200], *excit;
  char *param_file = NULL, *str, *bdstr = "0.0";
  int bflag=0, eflag=0, lflag=0, fflag=0, aflag=0, rflag=0, sflag=0,
      vflag=0, gflag=0, hflag=0, nflag=0;
  
  if(ac < 2) 
    SYNTAX;

  excit = NULL;
  f0s = NULL;
  fix_gain = 0;
  bws = 0.0;
  limit = 0.0;
  f0scale = 1.0;
  vforce = -1.0;
  nforce = -1;
  rate = 1.0;
  high_pole = 0;
  fscale = 1.0;

  while((c = getopt(ac,av,"P:b:e:l:f:a:r:s:v:h:n:x:g")) != EOF) {
    switch(c) {
    case 'P':
      param_file = optarg;
      break;
    case 'b':
      bws = atof(optarg);
      bflag++;
      break;
    case 'e':
      excit = optarg;
      eflag++;
      break;
    case 'l':
      limit = atof(optarg);
      lflag++;
      break;
    case 'f':
      f0scale = atof(optarg);
      fflag++;
      break;
    case 'a':
      fscale = atof(optarg);
      aflag++;
      break;
    case 'r':
      rate= atof(optarg);
      rflag++;
      break;
    case 's':
      fs = atof(optarg);
      sflag++;
      break;
    case 'v':
      vforce = atof(optarg);
      vflag++;
      break;
    case 'g':
      fix_gain = 1;
      gflag++;
      break;
    case 'h':
      high_pole = atoi(optarg);
      hflag++;
      break;
    case 'n':
      nforce = atoi(optarg);
      nflag++;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX
    }
  }

  (void) read_params(param_file, SC_NOCOMMON, (char *) NULL);
  if(!bflag && symtype("bandwidth") != ST_UNDEF){
    bdstr = getsym_s("bandwidth");
    bws = atof(str);
  }
  if(!eflag && symtype("excite_file") != ST_UNDEF)
    excit = getsym_s("excite_file");
  if(!lflag && symtype("min_bandwidth") != ST_UNDEF)
    limit = getsym_d("min_bandwidth");
  if(!fflag && symtype("f0_scale") != ST_UNDEF)
     f0scale = getsym_d("f0_scale");
  if(!aflag && symtype("formant_scale") != ST_UNDEF)
    fscale = getsym_d("formant_scale");
  if(!rflag && symtype("frame_rate_scale") != ST_UNDEF)
    rate = getsym_d("frame_rate_scale");
  if(!sflag && symtype("synthesis_rate") != ST_UNDEF)
    fs = getsym_d("synthesis_rate");
  if(!vflag && symtype("voice_prob") != ST_UNDEF)
    vforce = getsym_d("voice_prob");
  if(!gflag && symtype("amp_correct") != ST_UNDEF){
    str = getsym_s("amp_correct");
    if( !strcmp(str, "YES") || !strcmp(str, "yes") || !strcmp(str, "Yes"))
      fix_gain = 1;
  }
  if(!hflag && symtype("n_high_pole_formants") != ST_UNDEF)
    high_pole = getsym_i("n_high_pole_formants");
  if(!nflag && symtype("n_formants") != ST_UNDEF)
    nforce = getsym_i("n_formants");

  for(in=optind; in < ac; in++) {
    cp = av[ac-1];
    if((fd = open(cp,O_TRUNC|O_RDWR|O_CREAT,0664)) >= 0) {
      if(debug_level & 1)
	printf("File is open for output\n");
      if(excit) {
	i = 0;
	if(!(se = get_any_signal(excit,0.0,-1.0,NULL))) {
	  printf("Can't get excitation file %s\n",excit);
	  exit(-1);
	}
      } else {
	i = 1;
	if(!(f0s = get_any_signal(av[in],0.0,-1.0,NULL))) {
	  printf("Can't get F0 file (%s)\n",av[in]);
	  exit(-1);
	}
      }
      if((fbs = get_any_signal(av[in+i],0.0,-1.0,NULL))) {
	if(debug_level & 1)
	  printf("Got input signals %s and %s \n",av[in],av[in+i]);
	/* Determine the time region common to both input signals.
	   Arrange to synthesize based only on this common segment. */
	start = BUF_START_TIME(fbs);
	end = BUF_END_TIME(fbs);
	if(f0s) {
	  if(BUF_START_TIME(f0s) > start) {
	    start = BUF_START_TIME(f0s);
	    if(BUF_END_TIME(fbs) <= start) {
	      printf("F0 and Fn files do not overlap in time\n");
	      exit(-1);
	    }
	  } else
	    if(start >= BUF_END_TIME(f0s)) {
	      printf("F0 and Fn files do not overlap in time\n");
	      exit(-1);
	    }
	  if(end > BUF_END_TIME(f0s))
	    end = BUF_END_TIME(f0s);
	}
	if(excit) {
	  if(BUF_END_TIME(se) < end) end = BUF_END_TIME(se);
	  if(BUF_START_TIME(se) > start) start = BUF_START_TIME(se);
	}
	fbskip = (start - BUF_START_TIME(fbs)) * fbs->freq;
	if(f0s)
	  f0skip = (start - BUF_START_TIME(f0s)) * f0s->freq;
	if(excit) {
	  seskip = (start - BUF_START_TIME(se)) * se->freq;
	  fs = se->freq;
	  rate = 1.0;
	  ep = *((short**)se->data) + seskip;
	} else
	  ep = NULL;
	nframes = (end - start) * fbs->freq;
	nsamples = nframes * ((int)(0.5 + fs/fbs->freq));
	if((sig = (short*)malloc(sizeof(short)*nsamples))) {
	  if(debug_level & 1)
	    printf("Allocated memory (%d)\n",nsamples);
	  nform = fbs->dim/2;
	  for(i=0; i < nform; i++) {
	    ff[i] = ((double**)fbs->data)[i] + fbskip;
	    fb[i] = ((double**)fbs->data)[i+nform] + fbskip;
	  }
	  if(f0s) {
	    dpp = (double**)f0s->data;
	    f0 = dpp[0] + f0skip;
	    pv = dpp[1] + f0skip;
	    rms = dpp[2] + f0skip;
	  }
	  if(debug_level & 1)
	    printf("Finished pointersetup\n");
	  frint = 1.0/(rate * fbs->freq);
	  if((nforce > 0) && (nforce < nform)) nform = nforce;
	  if(debug_level & 1)
	    printf("Entering formsy()\n");
	  formsy(nframes,frint,nform,ff,fb,f0,pv,rms,sig,fs,bws,
		 limit,fscale,f0scale,vforce, high_pole,ep);
	  if(f0s && fix_gain) {
	    double frate, wsize, *dp;
	    int nrms;
	    short *rmsd, *sig2, *get_rms(), *adjust_gain();
	      
	    frate = 1.0/frint;
	    wsize = frint;
	    dp = ((double**)f0s->data)[2];
	    rmsd = get_rms(sig,nsamples,fs,&frate,&wsize,&nrms);
	    sig2 = adjust_gain(sig,nsamples,fs,rmsd,nrms,frate,dp);
	    free(sig);
	    free(rmsd);
	    sig = sig2;
	  }
	  spp = (short**)malloc(sizeof(short*));
	  *spp = sig;

	  if((so=new_signal(cp,fd,NULL,spp, nsamples,fs,1))) {
	    Header *w_new_header();

	    so->header = w_new_header(1);
	    if(fbs->header->magic == ESPS_MAGIC) {
	      struct header *sd_oh;
	      char   *get_cmd_line (), *cmd_line;
	      
	      /* create output ESPS header*/
	      cmd_line = get_cmd_line (ac, av);
	      sd_oh = new_header (FT_FEA);
	      add_source_file (sd_oh, fbs->name, fbs->header->esps_hdr);
	      if(excit)
		add_source_file (sd_oh, se->name, se->header->esps_hdr);
	      else
		add_source_file (sd_oh, f0s->name, f0s->header->esps_hdr);
	      add_comment (sd_oh, cmd_line);
	      sd_oh -> common.tag = NO;
	      set_pvd(sd_oh);
	      if((init_feasd_hd(sd_oh, SHORT, 1, &start, 0, fs)) !=0){
		fprintf(stderr, "formsy: Couldn't allocate FEA_SD header - exiting.\n");
		exit(1);
	      }
	      (void)add_genhd_d("record_freq", &fs, 1, sd_oh);
	      
	      if(strcmp(bdstr,"0.0"))
		(void)add_genhd_c("bandwidth", bdstr, strlen(bdstr), sd_oh);
	      if(limit > 1.0)
		(void)add_genhd_d("min_bandwidth", &limit, 1, sd_oh);
	      if(f0scale != 1)
		(void)add_genhd_d("f0_scale", &f0scale, 1, sd_oh);
	      if(fscale != 1)
		(void)add_genhd_d("formant_scale", &fscale, 1, sd_oh);
	      if(rate != 1)
		(void)add_genhd_d("frame_rate_scale", &rate, 1, sd_oh);
	      if(vforce >= 0.0)
		(void)add_genhd_d("voice_prob", &vforce, 1, sd_oh);
	      if(fix_gain)
		(void)add_genhd_c("amp_correct", "YES", 3, sd_oh);
	      else
		(void)add_genhd_c("amp_correct", "NO", 3, sd_oh);
	      if(high_pole > 0)
		(void)add_genhd_s("n_high_pole_formants",&high_pole, 1, sd_oh);
	      if(nforce >=0 )
		(void)add_genhd_s("n_formants", &nforce, 1, sd_oh);
	      so->header->e_scrsd = TRUE;
	      so->header->esps_hdr = sd_oh;
	      so->header->magic = ESPS_MAGIC;
	      so->header->strm = fdopen(fd, "w");
	    }
	    j = fbs->version + 1;
	    head_printf(so->header,"version",&j);
	    sprintf(temp,"formsy: b:%7.3f l:%4.0f a:%4.2f f:%3.1f r:%3.1f s:%5.0f v:%4.2f g:%1d n:%1d h:%d",bws,limit,fscale,f0scale,rate,fs,vforce,fix_gain,nform,high_pole);
	    head_printf(so->header,"operator",temp);
	    so->type = P_SHORTS;
	    so->start_time = start;
	    get_maxmin(so);
	    head_printf(so->header,"maximum",so->smax);
	    head_printf(so->header,"minimum",so->smin);
	    head_printf(so->header, "type_code", &(so->type));
	    head_ident(so->header,"PCM_signal");
	    head_printf(so->header,"samples",&nsamples);
	    head_printf(so->header,"frequency",&(so->freq));
	    so->band = so->freq/2.0;
	    head_printf(so->header,"bandwidth",&(so->band));
	    head_printf(so->header,"start_time",&(so->start_time));
	    head_printf(so->header,"dimensions",&(so->dim));
	    head_printf(so->header,"date",get_date());
	    if(! put_signal(so))
	      printf("Problems with put_signal in formsy()\n");
	    free_signal(so);
	  } else
	    printf("Problems with new_signal() in formsy()\n");
	} else
	  printf("Can't allocate an output buffer\n");
	free_signal(fbs);
	free_signal(f0s);
      } else
	printf("Can't get the required signals (%s %s)\n",av[in],av[in+1]);
    } else
      printf("Can't open %s for output\n",cp);
    in += 2;
  }
return(0);
}

void
formsy(nframes,frint,nform,ff,fb,f0,pv,rms,sig,fs,bws,limit,fscale,f0scale,vforce, hp, ep)
     double **ff, **fb, *f0, *pv, *rms, frint, fs, bws, limit, f0scale,
            fscale, vforce;
     int nframes, nform, hp;
     short *sig, *ep;
{
  int i, j, k, m, nsamples, bwc, nsy;
  double f[NFORM];
  double b[NFORM];
  double df[NFORM], db[NFORM], fhp, bhp, f1nom;
  double  ampl = 2000.;
  double ts, bwf, dpvo, pvo, intrp;
  double pi = 3.1415927;
  double as[10], bs[10], cs[10], ra, ag, bg; /* filter coefficients */
  double z0, zm1[10], zm2[10], input, curamp=0.0; /* filter memories */
  double tf0 = .003;	/* decay constant for exponential pitch pulse */
  double pit, pit2, damp, pvm1;
  double pdecay, pitoff, dtmp, df0, fz;
  short pitcnt, npit, update;
  
  ts = 1.0/fs;
  pitcnt = 10000;		/* force a new pitch period */
  npit = 0;
  if(bws > 1.0) {
    bwc = bws;
    bwf = (bws - bwc);
  }
  
  for( i=0; i < nform+hp; i++){ /* zero the filter memories */
    zm1[i] = 0.;
    zm2[i] = 0.;
  }
  pdecay = exp(-ts/tf0);	/* decay constant for pitch pulses */
  for(i=0; i < nform; i++) {
    f[i] = fscale * ff[i][0];
    if(bws > 1.0) {
      b[i] = ((double)bwc) + (bwf * f[i]);
    } else {
      b[i] = fb[i][0];
      if(limit > 1.0) {
	if(b[i] < limit) b[i] = limit;
      }
    }
  }
  pit = ts * pi;
  pit2 = 2.0 * pit;

  if(hp) {			/* enable hp "higher pole" formants */
    for(m=0, i=0, f1nom=0.0; m < nframes; m++)
      if(ep || (pv[m] > .8)) {
	for(j=0;j<nform;j++) {
	  f1nom += fscale * ff[j][m]/((j*2) + 1);
	  i++;
	}
      }
    if(i > 100) f1nom /= i;
    else f1nom = 500;
    for(i=nform; i < nform+hp; i++) {
      fhp = (i*2*f1nom) + f1nom;
      if((fhp > (fs/2.0)) || (fhp > 4800)) {
	hp = i - nform;
	break;
      }
      bhp = 60 + .05*fhp;
      ag = pit2*fhp;
      bg = pit*bhp;
      ra = exp(-bg);
      bs[i] = 2.0 * ra * cos(ag);
      cs[i] = - ra * ra;
      as[i] = 1.0 - bs[i] - cs[i];
    }
  }
  nsy = nform + hp;
      
  if(!ep) {
    ampl = rms[0];
    if((fz = f0[0]) < 60.0) fz = 60.0;
    if(vforce >= 0.0) pvo = vforce;
    else
      pvo = pv[0];
  }
  for(m=0; m < nframes; m++) { /* Synthesize all frames. */
    if(m == (nframes-1)) j = m;
    else j = m+1;
    /* this junk maintains a constant F2 derivative magnitude by adjusting rate
   if(!ep && (pv[m] > .5)) ffact = fscale * fabs(ff[1][j] - ff[1][m])/100.0;
   else ffact = 1.0; 
   nsamples = 0.5 + (frint * fs * ffact);
   if(nsamples < 10) nsamples = 10;
   */
    nsamples = 0.5 + (frint * fs);
    if(debug_level && !m) printf("frames:%d samples/frame:%d\n",nframes,nsamples);
    intrp  = ((double)NUPDATE)/nsamples;
    for(i=0; i < nform; i++) {
      if( ff[i][j] > 10.0) {
	if(bws > 1.0) {
	db[i] = intrp*(bwf*((fscale * ff[i][j]) - f[i]));
      } else {
	dtmp = fb[i][j];
	if((limit > 1.0) && (dtmp < limit)) dtmp = limit;
	db[i] = intrp*(dtmp - b[i]);
      }
      df[i] = intrp*((fscale * ff[i][j]) - f[i]);
      } else {
	db[i] = 0.0;
	df[i] = 0.0;
      }
    }
    
    if(!ep){
      if(vforce < 0.0)
	dpvo = intrp*(pv[j] - pvo);
      else
	dpvo = 0.0;
      damp = intrp*(rms[j]-ampl);
      if((dtmp = f0[j]) < 60.0) dtmp = 60.0;
      df0 = intrp*(dtmp - fz);
      for(k=0, update = 0; k < nsamples; k++) {  /* synthesize a frame's worth */
      curamp *= pdecay;
      if(pitcnt++ >= npit){	/* time for a new pitch pulse? */
      curamp *= pdecay;
	npit = (0.5 + (1.0/(f0scale*fz*ts))); /* # samples per pitch period */
	for(i=0, dtmp=0., ag=ampl; i < npit; i++, ag *= pdecay)  dtmp += ag;
	pitoff = dtmp/npit;
	pitcnt = 1;
	curamp = ampl;
      }
      if(update <= 0){	/* compute new filter coefficients */
	update = NUPDATE;
	for (i=0; i<nform; i++){
	  ag = pit2*f[i];
	  bg = pit*b[i];
	  ra = exp(-bg);
	  bs[i] = 2.0 * ra * cos(ag);
	  cs[i] = - ra * ra;
	  as[i] = 1.0 - bs[i] - cs[i];
	  f[i] += df[i];
	  b[i] += db[i];
	}
	pvo += dpvo;
	pvm1 = 1.0 - pvo;
	ampl += damp;
	fz += df0;
      }
      update--;
      input = (pvo * (curamp - pitoff)) + (pvm1 * ampl * (drand48() - drand48()));
      for (j=0; j < nsy; j++){
	z0 = (as[j]*input) + (bs[j]*zm1[j]) + (cs[j]*zm2[j]);
	  zm2[j] = zm1[j];
	  zm1[j] = z0;
	  input = z0;
      }
	if(input >= 0.0) *sig++ = (short)(0.5 + input);
	else             *sig++ = (short)(input - 0.5);
    }
  } else {
    for(k=0, update = 0; k < nsamples; k++) {  /* synthesize a frame's worth */
      if(update <= 0){	/* compute new filter coefficients */
	update = NUPDATE;
	for (i=0; i<nform; i++){
	  ag = pit2*f[i];
	  bg = pit*b[i];
	  ra = exp(-bg);
	  bs[i] = 2.0 * ra * cos(ag);
	  cs[i] = - ra * ra;
	  as[i] = 1.0 - bs[i] - cs[i];
	  f[i] += df[i];
	  b[i] += db[i];
	}
      }
      update--;
      input = *ep++;
      for (j=0; j < nsy; j++){
	z0 = (as[j]*input) + (bs[j]*zm1[j]) + (cs[j]*zm2[j]);
	  zm2[j] = zm1[j];
	  zm1[j] = z0;
	  input = z0;
      }
	if(input >= 0.0) *sig++ = (short)(0.5 + input);
	else             *sig++ = (short)(input - 0.5);
    }
  }
  }
}
  
char *meth_make_object()
{
  return("null");
}

char *get_receiver()
{
  return("null");
}

char *get_methods()
{
  return("null");
}



#define round(x) ((x) + 0.5)
/*----------------------------------------------------------------------- */
short	*get_rms(data,nsamples,fs0,fsrms,wdur,nret)	/* return pointer to rms
						 * signal */
short	*data;			/* pointer to original data */
int	nsamples,		/* # of original data samples */
	*nret;			/* # of samples in returned rms signal */
double	fs0,			/* original sampling frequency */
	*fsrms,			/* sampling frequency for rms computation */
	*wdur;			/* dur. of rms measurement window */
{
    short	*rmsd, *pt;
    register short	*p, *q;
    register int	nwind, i, j;
    double	sum;
    int	nrms, nstep;
    
/* Constrain the window size to be an integral number of samples. */
    nwind = round(fs0 * (*wdur));
    *wdur = ((double)nwind)/fs0;
/* Similarly treat the rms sampling interval (frequency). */
    nstep = round(fs0/(*fsrms));
    *fsrms = fs0/nstep;
/* Setup the number of samples of RMS to compute (use no partial windows). */
    *nret = nrms = 1 + (nsamples - nwind)/nstep;
    
/* Allocate space for the RMS "signal." */
    rmsd = (short*)malloc(sizeof(short) * nrms);

    for(i=0,pt=data;i<nrms;i++,pt += nstep) {	/* for all frames... */
	for(p=pt,sum=0.0,q=pt+nwind ;p < q; ) {	/* compute sum-of-squares */
	    j = *p++;
	    sum += (j * j);
	}
	rmsd[i] = 2.0 * sqrt(sum/nwind);	/* and root mean square */
    }

    return(rmsd);
}

/*----------------------------------------------------------------------- */
short	*adjust_gain(data,nsamples,fs0,rmsd,nrms,fsrms,want)
/* return pointer to gain-adjusted signal */
short *data, *rmsd;
int	nsamples, nrms;
double	fs0, fsrms, *want;
{
    register short	*p, *q, *r;
    register int	i, j, nstep;
    register double	inc, gain;
    int	nframes;
    
    gain = 1.0;			/* initialize "old" gain value */
    nstep = round(fs0/fsrms);
    nframes = nsamples/nstep;

/* allocate space for the "output" signal. */
    r = (short*)malloc(sizeof(short) * nsamples);

    for(i=0,p=data,q=r;i<nframes;i++) {
        j = rmsd[i];
	if(!j) j=1;		/* Avoid divide by zero (infinite gain). */
	inc = ((want[i]/j) - gain)/nstep;	/* Set up interpolation step. */
	for(j=0;j<nstep;j++) {
	    *q++ = round(gain * (*p++));	/* Apply gain term. */
	    gain += inc;	/* Interpolate gain term. */
	}
    }
/* Now taper the gain to unity while processing the remaining endpoints of
   the signal.*/
    j = nsamples - (nframes*nstep);
    inc = (1.0 - gain)/j;
    for(i=0; i < j; i++, gain += inc)
		*q++ = round(gain * *p++);
    
    return(r);
    
}



