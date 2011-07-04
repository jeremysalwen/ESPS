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
 * Written by:  David Talkin/Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *     Apply a time-varying linear fir filter to a signal. Convert formant
 *     and bandwidth or PARCOR representations to LPC's as necessary.
 *     Optionally normalize residual amplitudes by RC gain or inverse filter
 *     DC gain.
 *
 */

static char *sccs_id = "@(#)get_resid.c	1.10	11/7/96	ERL";

char *ProgName = "get_resid";
char *Version = "1.10";
char *Date = "11/7/96";

/*Get_resid accepts a 16-bit PCM SIGnal file and a SIGnal file of filter
parameters as input and produces an inverse filtered 16-bit PCM file
as output.  The input filter parameters may be predictor or PARCOR
coefficients (as produced by, for example, alpc or psan,
respectively), or formant frequencies and bandwidths (as produced by
the "formant" program).

If -a 1, the signal amplitude will be normalized by the DC gain.
If -a 2, the signal amplitude will be normalized by the RC gain.
Unless -s is specified, the LP coefficients and gain will be linearly
interpolated between frame centers.  If -s is specified the
coefficients and gains will switch abruptly at frame boundaries.
Integrator takes on values from 0 to 1.0 and specifies the coefficient
of a first-order integrator to be applied at the output of the inverse
filter [0.0].

If the filter parameters are "formants" the -b option permits
specification of parameters for synthesizing "formant" bandwidths from
formant frequencies.  If BW is the argument to the -b option
       B[i] = int(BW) + F[i]*(BW - int(BW))
where F[i] are the "formant" frequencies and B[i] the synthetic
bandwidths.  If -b is not specified, the actual bandwidths in the
formant file will be used.  -n permits specification of the number of
formants to use from the formant file.  If -n is not specified, all
"formants" will be used.  If -n is specified, the first N formants
will be used.
*/

#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/anafea.h>
#include <esps/feasd.h>

#define SYNTAX USAGE("get_resid [-P param_file][-{pr} range][-s range][-a normal][-i int_const][-b band_width][-n nformants][-y][-x debug_level] in_signal in_coef out_file")

#define MAXORDER 60

int debug_level = 0;
double rc_gain(), lp2rc_gain();
float int_c = 0.0, band = 0.0;
char *arr_alloc();
static void get_range();
static long	n_feasd_rec();
static long	n_fea_rec();


main(ac,av)
     int ac;
     char **av;
{
  extern int optind;
  extern char *optarg;
  char *param_file = NULL;


  register double *lpp1, *lpp2, sum1, sum2;
  register int ordd, i, j, istep;
  register short *q;
  double frint = .010, wsize = .02, preemp = .97, ftemp, **dpp, ind,
       lp1[MAXORDER], lp2[MAXORDER], alp, r0, *eng,  ewind= .02, stime,
       *wr, *wf, out, *wrp, *wfp, nfact1, nfact2, pst, ist, ist2, psf, isf,
       step, *op1, *op2, *dp1, *dp2, boff, bfac, rc[MAXORDER],
       **pdata, *spf, *spb;
  int size,  k, l, in, dimo, osamp, lnorm=1, *ds, *pi, *pi2, nfr, dim,
       nwind, nbk, sloff, curp, is_lpc=FALSE,
       is_rc=FALSE, c, sdim;
  short anorm = 0, nform=0, interp=TRUE;
  long arrdim[2];
  short *p, *sp, **spp;
  char *cp, *pfname, *ifname, *ofname;
  struct header *phd, *ihd, *ohd;
  FILE *pfile, *ifile, *ofile;
  struct anafea *ana_rec;
  struct feasd *sd_recin, *sd_recout;
  struct fea_data *fea_rec;
  char *get_cmd_line(), *cmd_line, *range=NULL;
  int rflag=0, sflag=0, iflag=0, aflag=0, bflag=0, nflag=0, yflag=0;
  long s_rec, e_rec, total_samps, total_precs;
  long inrec, pnrec;

  band = 0.0;
  while ((c = getopt(ac,av,"P:p:r:s:a:i:b:n:x:y")) != EOF) {
    switch(c) {
    case 'P':
      param_file = optarg;
      break;
    case 'p':
    case 'r':
      if( range ){
	fprintf(stderr,"Error: -r should not be used with -s\n");
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
    case 'a':
      anorm = atoi(optarg);
      aflag++;
      break;
    case 'i':
      int_c = atof(optarg);
      iflag++;
      break;
    case 'b':
      band = atof(optarg);
      bflag++;
      break;
    case 'n':
      nform = atoi(optarg);
      nflag++;
      break;
    case 'y':
      interp = FALSE;
      yflag++;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX;
      exit(1);
    }
  }

  if((ac - (in = optind)) != 3){
    SYNTAX;
    exit(1);
  }
  (void) read_params(param_file, SC_NOCOMMON, (char *)NULL);

  if(!iflag && symtype("int_const") != ST_UNDEF){
    int_c = getsym_d("int_const");
    iflag++;
  }
  if(!aflag && symtype("normal") != ST_UNDEF){
    anorm = getsym_i("normal");
    aflag++;
  }
  if(!bflag && symtype("band_width") != ST_UNDEF){
    band = getsym_d("band_width");
    bflag++;
  }
  if(!nflag && symtype("nformants") != ST_UNDEF){
    nform = getsym_i("nformants");
    nflag++;
  }
  if(!yflag && symtype("interp") != ST_UNDEF){
    interp = getsym_i("interp");
    yflag++;
  }

  if (strcmp(av[optind], "-") == 0 && strcmp(av[optind+1], "-") == 0) {
    fprintf(stderr, "%s: input files can't both be standard input.\n",
	    ProgName);
    exit(1);
  }
  ifname = eopen(ProgName, av[optind], "r", FT_FEA, FEA_SD, &ihd, &ifile);
  pfname = eopen(ProgName, av[optind+1], "r", FT_FEA, NONE, &phd, &pfile);
  ofname = eopen(ProgName, av[optind+2], "w", NONE, NONE, &ohd, &ofile);

  get_range( &s_rec, &e_rec, range, rflag, sflag, ihd );

  if (s_rec < 1) {
    fprintf(stderr, "%s: starting point (%ld) less than 1.\n",
	    ProgName, s_rec);
    exit(1);
  }

  pst = (double) get_genhd_val("start_time", phd, 0.0);
  psf = (double) *get_genhd_d("record_freq", phd);
  ist = (double) get_genhd_val("start_time", ihd, 0.0);
  isf = (double) *get_genhd_d("record_freq", ihd);

  ist2 = ist + (s_rec-1)/isf;

  stime = (pst > ist2) ? pst : ist2;
  s_rec = 1 + (long) (0.5 + (stime - ist) * isf);

  if(debug_level)
    fprintf(stderr,"Input sample range: %d - %d\n", s_rec, e_rec);
  sloff = 0.5 + (stime - pst) * psf;

  inrec = n_feasd_rec(&ifile, &ihd);
  if (e_rec > inrec)
      e_rec = inrec;
  total_samps = e_rec - s_rec + 1;

  pnrec = n_fea_rec(&pfile, &phd);
  total_precs = (long) (0.5 + (total_samps / isf) * psf);
  if (total_precs > pnrec - sloff)
    total_precs = pnrec - sloff;

  sd_recin = allo_feasd_recs(ihd, SHORT, total_samps, NULL, NO);

  (void) fea_skiprec( ifile, s_rec - 1, ihd);
  (void) fea_skiprec( pfile, sloff, phd);

  if( phd->hd.fea->fea_type == FEA_ANA ){           /* RC/AFC file */
    if( RC == (short) *get_genhd_s("spec_rep", phd ))
      is_rc = TRUE;
    if( AFC == (short) *get_genhd_s("spec_rep", phd ))
      is_lpc = TRUE;
    sdim = get_fea_siz("spec_param", phd, NULL, NULL);
    /* for RC file only, no formant data allocation *******/
    arrdim[0] = sdim;
    arrdim[1] = total_precs;
    pdata = (double **) arr_alloc( 2, arrdim, DOUBLE, 0);
    ana_rec = allo_anafea_rec(phd);
    for( i=0; i<total_precs; i++){
      get_anafea_rec( ana_rec, phd, pfile);
      for(j=0; j<sdim; j++)
	pdata[j][i] = - ana_rec->spec_param[j];

    }
    ordd = sdim;
  } else if( phd->hd.fea->fea_type == NONE ){              /* formant file */
    if( (i = get_fea_type("fm", phd) == UNDEF) ||
       ((i = get_fea_type("bw", phd) == UNDEF) )){
      fprintf(stderr,"trouble getting formant file %s\n", av[optind+1]);
      exit(1);
    }
    dim = get_fea_siz("fm", phd, NULL, NULL);
    sdim = 2*dim;
    arrdim[0] = sdim;
    arrdim[1] = total_precs;
    pdata = (double **) arr_alloc(2, arrdim, DOUBLE, 0);
    fea_rec = allo_fea_rec(phd);
    spf = (double *) get_fea_ptr(fea_rec, "fm", phd);
    spb = (double *) get_fea_ptr(fea_rec, "bw", phd);
    for( i=0; i<total_precs; i++){
      get_fea_rec(fea_rec, phd, pfile);
      for(j=0; j< dim; j++){
	pdata[j][i] = spf[j];
	pdata[j+dim][i] = spb[j];
      }
    }
    if(! nform) nform = dim;
    ordd = (2 * nform);
  }

  step = isf/psf;
  size = step * 2.0;
  if (size < 1) size = 1;
  istep = step;		/* assume analysis window == 2*step for now */

  /* If filter input is formants: generate synthetic bandwidths? */
  if(!(is_rc || is_lpc) && (band > 0.0)) {
    i = band;
    boff = i;
    bfac = band - boff;
    for(j=0, dpp = (double**)(pdata);j < sdim/2; j++) {
      for(k = j+ sdim/2, i = 0; i < total_precs ; i++)
	dpp[k][i] = boff + bfac*dpp[j][i];
    }
  }
  nfr = total_precs;
  osamp = nfr * step;

  /* initialize output header */
  cmd_line = get_cmd_line(ac,av);
  ohd = new_header(FT_FEA);
  ohd->common.tag = NO;
  add_source_file(ohd, ifname, ihd);
  add_source_file(ohd, pfname, phd);
  add_comment(ohd, cmd_line);
  set_pvd(ohd);
  if((init_feasd_hd(ohd, SHORT,1, &stime, 0, isf))!=0){
    fprintf(stderr,"can't allocate output file header -- exiting\n");
    exit(1);
  }
  (void) add_genhd_d("record_freq", &isf, 1, ohd);
  (void) add_genhd_d("start_time", &stime, 1, ohd);
  if(iflag) (void) add_genhd_f("int_const", &int_c, 1, ohd);
  if(aflag) (void) add_genhd_s("normal", &anorm, 1, ohd);
  if(bflag) (void) add_genhd_f("band_width", &band, 1, ohd);
  if(nflag) (void) add_genhd_s("nformants", &nform, 1, ohd);
  if(yflag) (void) add_genhd_s("interp", &interp, 1, ohd);
  write_header( ohd, ofile);

  if(osamp && (spp = (short**)malloc(sizeof(short*))) &&
     (wr = (double*)malloc(sizeof(double)*size)) &&
     (wf = (double*)malloc(sizeof(double)*size)) &&
     (sd_recout = allo_feasd_recs(ohd, SHORT, osamp,
			      (char *) NULL,NO) )) {
    if(interp) {
      for(i=0, j=step-1; i < step; i++) { /* make interp.*/
	wr[i]  = ((double)i)/j;
	wf[i] = 1.0 - wr[i];
      }
    } else {
      for(i=0, j=step-1, k=step/2; i < step; i++) { /* make switch*/
	if(i < k)
	  wr[i]  = 0.0;
	else
	  wr[i]  = 1.0;
	wf[i] = 1.0 - wr[i];
      }
    }
    /* input/out initialization */
    for(i=0, out = 0.0, sp = (short *) sd_recout->data; i < ordd; i++)
      *sp++ = 0.0;
    dpp = (double**)pdata;
    get_feasd_recs( sd_recin, 0L, total_samps, ihd, ifile);
    p= (short *) sd_recin->data;
    if(is_lpc)		/* load up the first set of coeffs. */
      for(j=1; j <= ordd; j++){ lp1[j] = dpp[j-1][0]; lp1[0] = 1;}
    else
      if(is_rc) {
	for(j=0; j < ordd; j++) rc[j] = dpp[j][0];
	k_to_a(rc,lp1+1,ordd);
	*lp1 = 1.0;
      } else
	make_lpc(lp1,0,dpp,nform,dim,isf);
    if(anorm==1)
      for(nfact1 = 0.0, j=0; j <=ordd; j++) nfact1 += lp1[j];
    else
      if(anorm == 2) {
	nfact1 = 1.0/lp2rc_gain(ordd,lp1);
      } else
	nfact1 = 1.0;

    if(debug_level > 2){
      fprintf(stderr,"%s: first set of LPC:\n", av[0]);
      for(i=0; i<=ordd; i++) fprintf(stderr,"   LPC[%d] = %f\n",i,lp1[i]);
    }

    /* ########### Main filter loop ... ########### */
    for( k=1, curp=1; k < nfr; k++, curp = !curp) {
      if(curp) {
	if(is_lpc)
	  for(j=1; j <= ordd; j++){ lp2[j] = dpp[j-1][k]; lp2[0] = 1;}
	else
	  if(is_rc) {
	    for(j=0; j < ordd; j++) rc[j] = dpp[j][k];
	    k_to_a(rc,lp2+1,ordd);
	    *lp2 = 1.0;
	  } else
	    make_lpc(lp2,k,dpp,nform,dim,isf);
	if(anorm==1)
	  for(nfact2 = 0.0, j=0; j <=ordd; j++) nfact2 += lp2[j];
	else
	  if(anorm == 2) {
	    nfact2 = 1.0/lp2rc_gain(ordd,lp2);
	  } else
	    nfact2 = 1.0;
	for(i=0, wrp=wr, wfp=wf;	i < istep; i++){
	  for(j=0, q=p++, sum1=0.0, sum2=0.0,lpp2 = lp2+ordd,
	      lpp1 = lp1+ordd; j < ordd; j++) {
	    sum1 += *lpp1-- * *q; /* old coeffs. */
	    sum2 += *lpp2-- * *q++;	/* new */
	  }
	  sum1 += *q;
	  sum2 += *q;
	  out = (sum1 * *wfp++ /nfact1) + (sum2 * *wrp++ /nfact2) +
	    int_c*out;
	  *sp++ = (out > 0.0)? out + 0.5 : out - 0.5;
	}
      } else {
	if(is_lpc)
	  for(j=1; j <= ordd; j++){ lp1[j] = dpp[j-1][k]; lp1[0] = 1;}
	else
	  if(is_rc) {
	    for(j=0; j < ordd; j++) rc[j] = dpp[j][k];
	    k_to_a(rc,lp1+1,ordd);
	    *lp1 = 1.0;
	  } else
	    make_lpc(lp1,k,dpp,nform,dim,isf);
	if(anorm==1)
	  for(nfact1 = 0.0, j=0; j <=ordd; j++) nfact1 += lp1[j];
	else
	  if(anorm == 2) {
	    nfact1 = 1.0/lp2rc_gain(ordd,lp1);
	  } else
	    nfact1 = 1.0;
	for(i=0, wrp=wr, wfp=wf; i < istep; i++){
	  for(j=0, q=p++, sum1=0.0, sum2=0.0,lpp2 = lp2+ordd,
	      lpp1 = lp1+ordd; j < ordd; j++) {
	    sum1 += *lpp1-- * *q; /* new coeffs. */
	    sum2 += *lpp2-- * *q++;	/* old */
	  }
	  sum1 += *q;
	  sum2 += *q;
	  out = (sum1 * *wrp++ /nfact1) + (sum2 * *wfp++ /nfact2) +
	    int_c*out;
	  *sp++ = (out > 0.0)? out + 0.5 : out - 0.5;
	}
      }
    }

    put_feasd_recs(sd_recout, 0L, osamp, ohd, ofile);

    exit(0);
  } else
    exit(1);
}


make_lpc(lpc,index,dpp,ordd,dim,samprt)
     double samprt, *lpc, **dpp;
     register int index, ordd, dim;
{
  double f1[16],bw1[16];
  struct comp {double c_r,c_i;} sos[MAXORDER], sos1[MAXORDER];
  int nsos,np,nleft,i,dimlpc,dim2,dimr;

  nsos = ordd;
  np = 2*ordd;
  for(i=0; i < ordd; i++) {
    f1[i] = dpp[i][index];
    bw1[i] = dpp[i+dim][index];
  }
  dfbwsos(f1,bw1,&nsos,sos,&samprt);
  dsoslpc(sos,lpc,&nsos);

}


double
rc_gain(rc,n)
     register double *rc;
     register int n;
{
  register double sum, dif, *dp=rc;
  int i=n;
  for(sum = 1.0; n--;) {
    dif = *rc++;
    sum *= (1.0 - dif*dif);
  }
  if(sum < 0.0){
    for(n=0;n<i;n++) printf("%f ",dp[n]);
    printf("\n");
    return(0.0);
  }
  return(sqrt(sum));
}


dlpcref(a,s,c,n) double *a,*c,*s; int *n;{
/*	convert lpc to ref
	a - lpc
	c - ref
	s - scratch
	n - no of cofs
					*/
double ta1,rc;
register double *pa1,*pa2,*pa3,*pa4,*pc;

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

double
lp2rc_gain(ordd,lp)
     int ordd;
     double *lp;
{
  double scr[MAXORDER], rc[MAXORDER];

  dlpcref(lp, scr, rc, &ordd);

  return(1.0/rc_gain(rc,ordd));
}


static void
get_range(srec, erec, rng, pflag, Sflag, hd)
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
n_feasd_rec(file, hd)
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


/*
 * Get number of records in a file.
 * Replace input stream with temporary file if input is a pipe
 * or record length is variable.
 */

static long
n_fea_rec(file, hd)
    FILE	    **file;
    struct header   **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe, or record length
				     * is variable. */
    {
	FILE		*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	struct fea_data	*tmprec; /* record for writing and reading temp file */
	long		ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);
	tmprec = allo_fea_rec(tmphdr);

	for (ndrec = 0; get_fea_rec(tmprec, *hd, *file) != EOF; ndrec++)
	    put_fea_rec(tmprec, tmphdr, tmpstrm);

	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}
