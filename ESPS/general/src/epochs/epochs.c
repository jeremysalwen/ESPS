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
 * Brief description: Estimate the F0 and voicing state, using 
 *     dynamic programming to
 *     find the major points of periodic excitation in a voiced speech
 *     waveform.
 *
 *
 */

static char *sccs_id = "@(#)epochs.c	1.7	1/22/97	ERL";


/* The algorithm is as follows:

   (1) A variety of signals, including the original PCM data may be used.
   For optimal selection of the exact point of excitation the signal may
   be prepared as follows:
   
    (a) Highpass the signal with a symmetric, non-causal FIR to remove
           DC, breath noise, etc.
       (b) Filter the result with an autoregressive filter of order
           int(2 + Fs/1000) after preemphasizing with 1st order filter (.97).
       (c) If necessary, apply phase correction to restore peakiness of
           "glottal flow derivative" (e.g. for TIMIT).

   (2)  Find all local maxima in the signal.  The number
   of these may be reduced while maintaining accuracy by constraining
   their absolute values to be greater than some running threshold (like
   the local rms). 

   (3) Associate with each peak: its time location, its polarity, its
   absolute value and first derivative.  

   (4) For each peak of a given polarity: 
       (a) Compute a local cost based on the relative local amplitude of
           the peak and the degree to which it fits a glottal flow
	   derivative model. (E.g. amplitude/(local rms) or
	   amplitude/(highest amp. in region); add in (s(t+.2ms) - s(t)).
       (b) For all preceeding peaks of the same polarity in an interval
            near the estimated F0 interval:
	      (i) Compute the transition cost based on the closeness of
	          the interpeak interval (IPI) to the estimated period
		  of the previous best candidate (E.g. abs(log(IPI*F0))
		  and on the similarity of the peak amplitudes
		  (E.g. abs(log(val1/val2))).
	      (ii) Save a backpointer to the previous peak which has the
	           lowest sum of accumulated cost and transition cost.
	      (iii) Assign the cost of the current peak selection to be
	            the sum of its local cost, transition cost and best
		    connection cost.  Save the interval to the best previous
		    peak for subsequent interations.
(5) Starting at the end of the signal (or at the end of each voiced
    interval), examine the peaks in a voiced interval corresponding to the
    F0 range of interest and select the lowest cost peak in that interval
    as the starting point for backtracking to select the most likely set
    of peaks.
*/
#include <math.h>
#include <Objects.h>
#include <labels.h>
#include <esps/esps.h>
#include <esps/fea.h>

#if defined(LINUX_OR_MAC)
#include <strings.h>
/* for strncpy, strcat, strrchr */
#endif
#define AMP 10000
#define min(x,y) ((x > y)? y : x)
#define max(x,y) ((x > y)? x : y)
#define PTIME(p) (BUF_START_TIME(sd) + ((double)(p)->sample)/sd->freq)
#define ITIME(i) (BUF_START_TIME(sd) + ((double)(i))/sd->freq)
int MARKER_COLOR;
get_wobject(){};
/*pf_default(){};*/
kill_wobject(){};
/*pf_open(){};*/
Signal *get_any_signal();

/* DP fudge factors: */
float CLIP = 0.5,  /* clipping level for local RMS*/
  PEAK = 1.0,	/* weight given to peak quality */
  TSIM = .4,	/* cost of period dissimilarities */
  PSIM = .05,	/* cost for peak quality dissimilarity */
  PQUAL = .35,	/* relative contribution of shape to peak quality */
  FDOUB = .7,		/* cost of each frequency doubling/halving */
  AWARD = .4,		/* award for selecting a peak */
  VOFFCOST = 0.2,		/* cost for V-UV transition */
  VONCOST = 0.2,		/* cost for UV-V transition */
  VONOFF = 0.3,		/* cost for rms rise/fall appropriateness */
  UVCOST = 0.7,		/* cost of unvoiced classification */
  VUCOST,
  JITTER = .1;		/* reasonable inter-period variation */

char *parfile = NULL;
typedef struct peak {
  int sample;
  float value;
  float rms;
  struct pcand *cands;
  struct peak *next, *prev;
} Peak;

Peak *neg=NULL, *pos=NULL;
float srate, fratio, range = 0.7, ln2;
short *ppul, *npul;
int imin, imax, debug_level=0, peaks, tcands;
Signal *sd, *so;

static char *voiced[] = {"D","H","V","M","F",NULL};

new_peak(pp, loc, val, rms)
     register Peak **pp;
     register int loc;
     register float val;
     register short rms;
{
  register Peak *pk;

  if(!(pk = (Peak*)malloc(sizeof(Peak)))) {
    printf("Can't allocate a Peak.\n");
    exit(-1);
  }
  pk->sample = loc;
  pk->value = val;
  pk->rms = rms;
  pk->prev = *pp;
  pk->next = NULL;
  pk->cands = NULL;
  *pp =pk;
  if(pk->prev)
    pk->prev->next = pk;
  return;
}

typedef struct pcand {
  Peak *this;			/* parent peak at head of this list */
  struct pcand *best;		/* best previous candidate resulting from DP */
  int inter;			/* interval (in samples) to "best" */
  int type;			/* voiced=1 or unvoiced=0 */
  float cost;			/* cost of choosing "best" */
  struct pcand *next;		/* next candidate for this peak */
} Pcand;

Pcand neg_p = {NULL,NULL,0,0,(float)0.0,NULL},
  pos_p = {NULL,NULL,0,0,(float)0.0,NULL};

/*------------------------------------------------------*/
Pcand *link_new_cand(pp,pc,linter,tcost,type)
     register Peak *pp;
     register Pcand *pc;
     register int linter, type;
     register float tcost;
{
  Pcand *p;
  
  if((p = (Pcand*)malloc(sizeof(Pcand)))) {
    if(pp) {
      p->next = pp->cands;
      pp->cands = p;
    } else
      p->next = NULL;
    p->inter = linter;
    p->best = pc;
    p->cost = tcost + pc->cost;
    p->type = type;
    if(debug_level == 256)
      printf("%f %f %d %d\n",p->cost,tcost,p->inter,pc->this);
    p->this = pp;
    return(p);
  }
  return(NULL);
}

/*------------------------------------------------------*/
Pcand *get_best_track(pp)
     Peak *pp;
{
  register Pcand *pc;

  if(!(pp && (pc = pp->cands))) return(NULL);
  else {
    float cmin, cc, tc;
    Pcand *pmin;

    if(!pc->this) return(NULL);
    cmin = pc->cost;
    pmin = pc;
    while(pc = pc->next) {
      if(((cc =  pc->cost) < cmin)) {
	cmin = cc;
	pmin = pc;
      }
    }
    return(pmin);
  }
}

/*------------------------------------------------------*/
/* Return the lowest cost candidate from all peaks within the maximum plausible
   pitch period. */
Pcand *get_best_peak(pp)
     Peak *pp;
{
  register Pcand *pc, *pmin;
  register int low;

  if(pp) {
    low = pp->sample - imin;
    pmin = get_best_track(pp);
    while((pp=pp->prev) && (pp->sample >= low)) {
      if((pc = get_best_track(pp)) && pc->this) {
	if(pc->cost < pmin->cost)
	  pmin = pc;
      } else
	return(pmin);
    }
    return(pmin);
  }
  return(NULL);
}

/*      ----------------------------------------------------------      */
char *new_ext(oldn,newex)
char *oldn, *newex;
{
  int	j;
  char *dot, *localloc();
#if !defined(LINUX_OR_MAC)
  char *strcat(), *strrchr(), *strncpy();
#endif
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
  dot=strcat(newn,newex);
  return(dot);
/*  return(strcat(newn,newex)); */
}

/*------------------------------------------------------*/

#define SYNTAX USAGE("epochs [-P param_file ] [-p | -n] [-b in_labelfile] [-f in_f0file] [-o out_labelfile] [-x debug_level] in_file out_file")


#define NONE 0
#define POSITIVE 1
#define NEGATIVE 2

main(ac,av)
     int ac;
     char **av;
{
  extern int optind;
  extern char *optarg;

  register int i, j, k, npeak=1, ppeak=1;
  register short *p, *q, *r, s, t, pm2, pm1, *pmm, *ppm, thresh;
  register Pcand *pk1, *pk2;
  register double ssq;
  float wsize=.02, amax, maxrms, f0min=50, f0max=800, val, ppval, npval;
  double outf=100.0;
  short *rms, *scrp;
  char mess[200], *sym;
  int c, off, npoints, msec, outd=3;
  FILE *fp=NULL, *fopen(), *fop=NULL;
  Vlist *vl=NULL, *v, *read_label(), *read_vuv_signal();
  int kflag=0,tflag=0,qflag=0,sflag=0,dflag=0,aflag=0,vflag=0,uflag=0,
     rflag=0,cflag=0,jflag=0,lflag=0,uvflag=0,nflag=0;
  static char *polar_code[] = {"NONE","+","-", NULL};
  struct header *srcohd;

  short polar_type = 0;

  
  while((c = getopt(ac,av,"nupb:f:o:P:x:")) != EOF) {
    switch(c) {
    case 'P':
      parfile = optarg;
      break;
      break;
    case 'o':
      if((fop = fopen(optarg,"w"))) {
	fprintf(fop,"#\n");
      } 
      else
	printf("Can't open epoc location file %s\n",optarg);
      break;
    case 'u':
    case 'p':
      if(nflag){
	fprintf(stderr,"ERROR:%s: can't use -u and -n together - exiting\n",av[0]);
	exit(1);
      }
      ppeak = 1;
      npeak = 0;
      uflag++;
      break;
    case 'n':
      if(uflag){
	fprintf(stderr,"ERROR:%s: can't use -u and -n together - exiting\n",av[0]);
	exit(1);
      }
      ppeak = 0;
      npeak = 1;
      nflag++;
      break;
    case 'f':
      if(!vl) {
	if(!(vl = read_vuv_signal(optarg))) {
	  printf("Can't access prob_voice F0 file %s\n",optarg);
	  exit(-1);
	}
      } else {
	printf("Label and V/UV signal files are mutually exclusive.\n");
	exit(-1);
      }
      break;
    case 'b':
      if(!vl) {
	if(!(vl=read_label(optarg, voiced))) {
	  printf("Can't access V/UV label file %s\n",optarg);
	  exit(-1);
	}
	if(debug_level == 2){
	  for(v=vl; v; v=v->next)
	    printf("start:%f end%f\n",v->start,v->end);
	}
      }else {
	printf("Label and V/UV signal files are mutually exclusive.\n");
	exit(-1);
      }
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX;
      exit(1);
    }
  }

  if((ac- optind) != 2){
    SYNTAX;
    exit(1);
  }

  (void) read_params(parfile, SC_NOCOMMON, (char *)NULL);

  if(!lflag && symtype("clip_level") !=ST_UNDEF){
    CLIP = getsym_d("clip_level");
    lflag++;
  }
  if(!(uflag || nflag) && symtype("polarity") !=ST_UNDEF){
    sym = getsym_s("polarity");
    polar_type = lin_search(polar_code, sym);
    if(polar_type==POSITIVE){ 
      ppeak=1; 
      npeak=0;
    } else if (polar_type == NEGATIVE){ 
      ppeak=0; 
      npeak=1;
    } else if (polar_type != NONE){
      fprintf(stderr,"ERROR: Unknown polarity in %s file.\n", parfile);
      fprintf(stderr,"   polarity is either %s, %s or %s\n", 
	      polar_code[0], polar_code[1], polar_code[2]);
      exit(1);
    }
    uflag++;
    nflag++;
  }
  if(!kflag && symtype("peak_quality_wt") !=ST_UNDEF){
    PEAK = getsym_d("peak_quality_wt");
    kflag++;
  }
  if(!tflag && symtype("period_dissim_cost") !=ST_UNDEF){
    TSIM= getsym_d("period_dissim_cost");
    tflag++;
  }
  if(!qflag && symtype("peak_qual_dissim_cost") !=ST_UNDEF){
    PSIM = getsym_d("peak_qual_dissim_cost");
    qflag++;
  }
  if(!sflag && symtype("shape_to_peak") !=ST_UNDEF) {
    PQUAL = getsym_d("shape_to_peak");
    sflag++;
  }
  if(!dflag && symtype("freq_dh_cost") !=ST_UNDEF) {
    FDOUB = getsym_d("freq_dh_cost");
    dflag++;
  }
  if(!aflag && symtype("peak_award") !=ST_UNDEF) {
    AWARD= getsym_d("peak_award");
    aflag++;
  }
  if(!vflag && symtype("v_uv_cost") !=ST_UNDEF) {
    VOFFCOST= getsym_d("v_uv_cost");
    vflag++;
  }
  if(!uvflag && symtype("uv_v_cost") !=ST_UNDEF) {
    VONCOST= getsym_d("uv_v_cost");
    uvflag++;
  }
  if(!rflag && symtype("rms_onoff_cost") !=ST_UNDEF) {
    VONOFF = getsym_d("rms_onoff_cost");
    rflag++;
  }
  if(!cflag && symtype("uv_cost") !=ST_UNDEF) {
    UVCOST = getsym_d("uv_cost");
    cflag++;
  }
  if(!jflag && symtype("jitter") !=ST_UNDEF) {
    JITTER = getsym_d("jitter");
    jflag;
  }

  if(debug_level)
    printf("%s\nPEAK:%f TSIM:%f PSIM:%f PQUAL:%f FDOUB:%f AWARD:%f\n VONCOST:%f VOFFCOST:%f VONOFF:%f UVCOST:%f\n",
	   av[optind], PEAK,TSIM,PSIM,PQUAL,FDOUB,AWARD,VONCOST,VOFFCOST,
	   VONOFF,UVCOST);
  if((sd = get_signal(av[optind],0.0,-1.0,NULL))) {
    srate = sd->freq;
    imin = srate/f0min;
    imax = srate/f0max;
    tcands = 0;
    peaks = 0;
    VUCOST = PEAK*UVCOST - AWARD + VOFFCOST;
    JITTER = log(1.0 + JITTER);
    if((rms = (short*)malloc(sizeof(short)*sd->buff_size)) &&
       (scrp = (short*)malloc(sizeof(short)*sd->buff_size)) &&
       (ppul = (short*)malloc(sizeof(short)*sd->buff_size)) &&
       (npul = (short*)malloc(sizeof(short)*sd->buff_size))) {
      
      /* Zero out the new epoch pulse array. */
      for(i=sd->buff_size, p=ppul, q = npul; i-- > 0; ) *p++ = *q++ =0;

      /* Compute a running estimate of the rms using a rectangular window
	 of duration wsize seconds. */
      for(k=wsize*sd->freq*.5, i=0, ssq=0.0, q = p = *(short**)(sd->data);
	  i<k; i++, p++)
	ssq += *p * *p;
      for(k *= 2, r=rms, maxrms=0.0; i < k; i++, p++) {
	*r++ = (amax = sqrt(ssq/i));
	if(amax > maxrms) maxrms = amax;
	ssq += *p * *p;
      }
      for(j = sd->buff_size - i; j-- > 0;q++, p++) {
	ssq -= *q * *q;
	*r++ = (amax = ((ssq > 0.0)? sqrt(ssq/i) : 0.0));
	if(amax > maxrms) maxrms = amax;
	ssq += *p * *p;
      }
      for(ssq = (ssq > 0.0)? sqrt(ssq/i) : 0.0, k /= 2; k-- > 0;) 
	*r++ = ssq;

      if(!vl) {		/* in case no V/UV label file is present */
	vl = (Vlist*)malloc(sizeof(Vlist));
	vl->start = BUF_START_TIME(sd);
	vl->end = BUF_END_TIME(sd);
	vl->next = NULL;
      }
      /* vl is a linked list of voiced intervals (non-overlapping).
	For each voiced interval, find the optimum set of pitch peaks. */
      for(v = vl; v && (v->end <= BUF_START_TIME(sd)); )
	v = v->next;
      for( neg_p.cost = 0.0, pos_p.cost = 0.0 ; v; v = v->next) {
	if(BUF_END_TIME(sd) > v->start) {
	  if(v->start < BUF_START_TIME(sd))
	    v->start = BUF_START_TIME(sd);
	  off = (v->start - BUF_START_TIME(sd)) * sd->freq;
	  if(v->end > BUF_END_TIME(sd))
	    npoints = 1 + ((BUF_END_TIME(sd) - v->start) * sd->freq);
	  else
	    npoints = 1 + ((v->end - v->start) * sd->freq);
	  if(!(msec = 0.5 + .0002 * sd->freq))	/* ~number of samples in .2ms */
	    msec = 1;
	  maxrms /= 4.0;
	  /* Find all peaks with absolute value greater than the local rms*clip. */
	  for(p= *(short**)(sd->data) + off, pm2 = *p++,
		ppm = p + msec, pm1 = *p++, j = off + npoints - 2,
	      i = off + 1, q=rms + off;	i < j ; i++, ppm++) {
	      scrp[i] = 0;
	      s = *p++;
	      t = *q / 3;
	      thresh = CLIP * *q++;
	      if(!t) t = 1;
	      if(ppeak && (pm1 > thresh)) {	/* large pos. peak possible? */
		if((s < pm1) && (pm1 >= pm2)){ /* it's a positive peak.*/
		  val = (1.0 - PQUAL) * pm1 + PQUAL * (pm1 - *ppm);
		  if(val > 0.0) {
		    val = ((float)t)/val;
		    scrp[i] = 100.0/val;
		    if(debug_level == 2)
		      printf("+ %f %f \n", ITIME(i),val);
		    new_peak(&pos,i,val,t);
		    do_dp(pos,&pos_p);
		  }
		}
	      } else {		/* maybe it's a large neg. peak... */
		if(npeak && (-pm1 > thresh) && (s > pm1) && (pm1 <= pm2)) {
		  val = -((1.0 - PQUAL) * pm1 + PQUAL * (pm1 - *ppm));
		  if(val > 0.0) {
		    val = ((float)t)/val;
		    scrp[i] = -100.0/val;
		    if(debug_level == 2)
		      printf("- %f %f\n",ITIME(i),val);
		    new_peak(&neg,i,val,t);
		    do_dp(neg, &neg_p);
		  }
		}
	      }
	      pm2 = pm1;
	      pm1 = s;
	    }
      
	    /* Now backtrack to find optimum voicing track. */
	    pk1 = get_best_peak(pos);
	    if(pk1) {
	      pos_p.cost = pk1->cost;
	      if(debug_level == 64)
		printf("+%7.4f pos_cum:%f\n",ITIME(i),pos_p.cost);
	    }
	    pk2 = get_best_peak(neg);
	    if(pk2) {
	      neg_p.cost = pk2->cost;
	      if(debug_level == 64)
		printf("-%7.4f neg_cum:%f\n", ITIME(i),neg_p.cost);
	    }
	    while(pk1 && pk1->this) {
	      if(pk1->type) ppul[pk1->this->sample] = AMP;
	      if(debug_level == 1)
		printf("+t:%f v:%d\n",ITIME(pk1->this->sample),
		       ppul[pk1->this->sample]);
	      pk1 = pk1->best;
	    }
	    while(pk2 && pk2->this) {
	      if(pk2->type) npul[pk2->this->sample] = -AMP;
	      if(debug_level == 1)
		printf("-t:%f v:%d\n",ITIME(pk2->this->sample),
		       npul[pk2->this->sample]);
	      pk2 = pk2->best;
	    }
	    clobber_history();
	  }
	}			/* done with all "voiced" segments */

      /* save the header */
      srcohd = copy_header(sd->header->esps_hdr);

      rename_signal(sd,av[optind+1]);
      if((ppeak && !npeak) || (ppeak && npeak &&
	 ((ppval = pos_p.cost) <
	  (npval = neg_p.cost)))) {
	((short**)(sd->data))[0] = ppul;
      } else {
	((short**)(sd->data))[0] = npul;
      }
      if(fop) {
	for(i=0,p = *(short**)(sd->data); i < sd->buff_size; i++)
	  if(*p++ ) 
	    fprintf(fop,"%20.7f\n",ITIME(i));
	fprintf(fop,"%20.8f\n",BUF_END_TIME(sd));
	fclose(fop);
      }
      get_maxmin(sd);
    sprintf(mess,"epochs: PEAK %f TSIM %f PSIM %f PQUAL %f FDOUB %f AWARD %f VONCOST %f VOFFCOST %f VONOFF %f UVCOST %f signal %s\n",
 PEAK,TSIM,PSIM,PQUAL,FDOUB,AWARD,VONCOST,VOFFCOST,VONOFF,UVCOST,av[optind]);
      head_printf(sd->header,"operation",mess);
      head_ident(sd->header,"epochs");
      head_printf(sd->header,"maximum",sd->smax);
      head_printf(sd->header,"minimum",sd->smin);
      head_printf(sd->header,"samples",&(sd->buff_size));
      sd->end_time = BUF_END_TIME(sd);
      head_printf(sd->header,"end_time",&(sd->end_time));
      /* ESPS file */
      {
	struct header *sd_oh;               /* for ESPS write out */
	char   *get_cmd_line (), *cmd_line;
	/* create output ESPS header*/
	cmd_line = get_cmd_line (ac, av);
	sd_oh = new_header (FT_FEA);
	add_source_file (sd_oh, av[optind], srcohd);
	add_comment (sd_oh, cmd_line);
	sd_oh -> common.tag = NO;
	set_pvd(sd_oh);
	if((init_feasd_hd(sd_oh, SHORT, 1, &sd->start_time, 0, sd->freq)) !=0){
	  fprintf(stderr, "epochs: Couldn't allocate FEA_SD header - exiting.\n");
	  exit(1);
	}

	if( uflag||nflag) 
	  (void)add_genhd_e("polarity", &polar_type, 1, polar_code, sd_oh);
	if(kflag) (void)add_genhd_f("peak_quality_wt", &PEAK, 1, sd_oh);
	if(tflag) (void)add_genhd_f("period_dissim_cost", &TSIM, 1, sd_oh);
	if(qflag) (void)add_genhd_f("peak_qual_dissim_cost", &PSIM, 1, sd_oh);
	if(sflag) (void)add_genhd_f("shape_to_peak", &PQUAL, 1, sd_oh);
	if(dflag) (void)add_genhd_f("freq_dh_cost", &FDOUB, 1, sd_oh);
	if(aflag) (void)add_genhd_f("peak_award", &AWARD, 1, sd_oh);
	if(vflag) (void)add_genhd_f("v_uv_cost", &VOFFCOST, 1, sd_oh);
	if(uvflag) (void)add_genhd_f("uv_v_cost", &VONCOST, 1, sd_oh);
	if(rflag) (void)add_genhd_f("rms_onoff_cost", &VONOFF, 1, sd_oh);
	if(cflag) (void)add_genhd_f("uv_cost", &UVCOST, 1, sd_oh);
	if(jflag) (void)add_genhd_f("jitter", &JITTER, 1, sd_oh);
	if(lflag) (void)add_genhd_f("clip_level", &CLIP, 1, sd_oh);

	sd->header->e_scrsd = TRUE;
	sd->header->esps_hdr = sd_oh;
	sd->header->magic = ESPS_MAGIC;
	sd->header->strm = fdopen(sd->file, "w");
      }

      put_signal(sd);
      if(debug_level == 128)
	printf("total peaks:%d, total cands:%d, cands/peaks:%f\n",peaks, tcands,
	       ((double)tcands)/peaks);
      if(debug_level == 512) {
	rename_signal(sd,new_ext(sd->name,"peaks"));
	*((short**)(sd->data)) = scrp;
	put_signal(sd);
      }
    } else
      printf("Can't allocate rms memory.\n");
  } else
    printf("Can't get signal (%s)\n",av[optind]);
}

/*------------------------------------------------------*/
clobber_history()
{
  Peak *p;
  Pcand *pc, *pcn;

  while(neg) {
    pc = neg->cands;
    while(pc) {
      pcn = pc->next;
      free(pc);
      pc = pcn;
    }
    p = neg->prev;
    free(neg);
    neg = p;
  }
  while(pos) {
    pc = pos->cands;
    while(pc) {
      pcn = pc->next;
      free(pc);
      pc = pcn;
    }
    p = pos->prev;
    free(pos);
    pos = p;
  }
}

/*------------------------------------------------------*/
Vlist *
read_vuv_signal(name)
     char *name;
{
  FILE *ifile;
  struct header *ihd;
  char *ifname;
  double start_time, sf;
  struct fea_data *fea_rec;


  Vlist *vl=NULL, *vl0=NULL, *vlt;
  register double time, tinc, tp;
  register double *pv;
  int vo;

  ifname = eopen("epochs", name, "r", NONE, NONE, &ihd, &ifile);
  sf = (double) *get_genhd_d("record_freq", ihd);
  start_time = (double) get_genhd_val("start_time",ihd, 0.0);
  fea_rec = allo_fea_rec(ihd);
  if(DOUBLE == get_fea_type("prob_voice", ihd))
    pv = (double *) get_fea_ptr(fea_rec,"prob_voice", ihd);
  else{
    fprintf(stderr,"epochs: prob_voice field must be type DOUBLE - exiting\n");
    return(NULL);
  }
  if(!pv){
    fprintf(stderr,"epochs: Couldn't get prob_voice field in f0 file - exiting\n");
    return(NULL);
  }

  get_fea_rec(fea_rec, ihd, ifile);
  for (time=(tp = start_time)+(tinc = 1.0/sf), vo=(*pv > 0.5);
       get_fea_rec(fea_rec, ihd, ifile) != EOF;
       time += tinc)
  {
    if(vo != (*pv > .5)) {
      if(vo) {
	vlt = (Vlist*)malloc(sizeof(Vlist));
	spsassert(vlt,"1st vlt malloc failed in read_vuv_signal\n");
	vlt->next = NULL;
	vlt->start = tp;
	vlt->end = time;
	if(vl)vl->next = vlt;
	if(!vl0)vl0 = vlt;
	vl = vlt;
      }
      tp = time;
      vo = !vo;
    }
  }
  if(vo && (time > tp))  {
    vlt = (Vlist*)malloc(sizeof(Vlist));
    spsassert(vlt,"2nd vlt malloc failed in read_vuv_signal\n");
    vlt->next = NULL;
    vlt->start = tp;
    vlt->end = time;
    if(vl)vl->next = vlt;
    if(!vl0)vl0 = vlt;
    vl = vlt;
  }
  return(vl0);
}

/*------------------------------------------------------*/
do_dp(pp,pcinit)
     register Peak *pp;
     register Pcand *pcinit;
{
  register Peak *p, *pstart, *phold, *bestp;
  register Pcand *pc, *pmin, *uvpmin, *bestuv;
  register int  low, high, cands=0;
  register float cmin, tcmin, uvcmin, uvtcmin, tc, cc, linter, ttemp,
           ftemp, ft1, tcost, vcost, vutcost, uvtcost,  ucost, maxcost;
  
  low = pp->sample - imin;
  high = pp->sample - imax;
  if(debug_level == 1)
    printf("%f ",PTIME(p));
  p = pp->prev;
  while(p && (p->sample > high)) p = p->prev;
  if(!(pstart = p)) {
    link_new_cand(pp,pcinit,0,0.0,1);
    link_new_cand(pp,pcinit,0,0.0,0);
    if(debug_level == 1)
      printf("(no pre at T=%f)\n",PTIME(pp));
    return;
  }
  peaks++;
  ln2 = log((double)2.0);
  vcost = PEAK * pp->value - AWARD;
  ucost = PEAK * (UVCOST - pp->value) - AWARD;
  pmin = NULL;
  uvpmin = NULL;
  bestuv = NULL;
  bestp = NULL;
  maxcost =  uvcmin = 1.0e30;
  phold = p;
  while(p && (p->sample >= low)) { /* find highest peak as UV cand. */
    if(p->value < maxcost){
      maxcost = p->value;
      bestp = p;
    }
    p = p->prev;
  }
  /* There are always pleanty of low-quality peaks;  question is:
     Is the BEST previous peak still better classified as UNVOICED? */
  if(bestp && (pc = bestp->cands)) {
    while(pc) {
      if(! pc->type) {		/* get UV-V transition cost */
	bestuv = pc;
	uvtcost =  vcost + VONCOST + VONOFF*log(bestp->rms/pp->rms);
	break;
      }
      pc = pc->next;
    }
  } /* (Now have the unvoiced hypothesis of the HIGHEST peak.) */
  p = phold;
  while(p && (p->sample >= low)) { /* for each possible candidate */
    linter = pp->sample - p->sample; /* local time interval */
    tcost =  vcost + (PSIM * fabs(log(pp->value/p->value)));
    vutcost =  VUCOST + VONOFF*log(pp->rms/p->rms);
    if((pc = p->cands)) {
      cmin = 1.0e30;
      while(pc) {		/* for each of its candidates */
	if(pc->type) {		/* is it a voiced hypothesis? */
	  if(pc->inter && (pc->best->type)) { /* prev. per. available? */
	    ttemp = fabs(log(linter/pc->inter));
	    ftemp = (ttemp > (ft1 = FDOUB + fabs(ttemp - ln2)))?
	      ft1 : ttemp;
	  } else ftemp = JITTER;
	  if((cc = ((tc = (tcost + (TSIM * ftemp))) + pc->cost)) <
	     cmin) {
	    cmin = cc;
	    tcmin = tc;
	    pmin = pc;
	  }
	  /* Now compute previous voiced to current unvoiced cost. */
	  if((cc = vutcost + pc->cost) < uvcmin) {
	    uvcmin = cc;
	    uvtcmin = vutcost;
	    uvpmin = pc;
	  }
	} else {    /* Check for unvoiced-to-voiced transition as best. */
	  if(pc == bestuv) { /* is it the LEAST likely unvoiced candidate? */
	    if((cc = uvtcost + pc->cost) < cmin) {
	      cmin = cc;
	      tcmin = uvtcost;
	      pmin = pc;
	    }
	  }
	}
	pc = pc->next;
      }
    } else {
      printf("Peak with no candidates in dp_dp()!\n");
    }
    if(!link_new_cand(pp,pmin,(int)linter,tcmin,1)) { /* voiced cands. */
      printf("Problems with link_new_cand()\n");
      exit(-1);
    }
    cands++;
    if(debug_level == 1)
      printf("%f:%f;%fL%d ", PTIME(p),tc,pmin->cost,(int)linter);
    p = p->prev;
  }	/* finished all previous peaks in F0 range */

  /* get the cost of the unvoiced-unvoiced transition */
  if(bestuv && ((cc = ucost + bestuv->cost) < uvcmin)) {
    uvcmin = cc;
    uvtcmin = ucost;
    uvpmin = bestuv;
  }

  if(uvpmin) { /* record the best connection for the unvoiced hypothesis  */
    if(!link_new_cand(pp,uvpmin,(int)linter,uvtcmin,0)) { /* unvoiced cand. */
      printf("Problems with link_new_cand()\n");
      exit(-1);
    }
    cands++;
  }
  if(!pmin)		/* find a bogus best to maintain continuity */
    if((pc = get_best_track(pstart))) {
      link_new_cand(pp,pc,pc->inter,0.0,1); /* equal cost V and UV cands. */
      link_new_cand(pp,pc,pc->inter,0.0,0);
      cands += 2;
    } else
      printf("No prev. candidates and no track at T=%f)\n",PTIME(pp));
  if(debug_level == 128)
    printf("%f  %d\n",PTIME(pp), cands);
  tcands += cands;
  return;

}
