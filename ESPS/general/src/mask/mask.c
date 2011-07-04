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
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description: Mask sampled data signal using a mask signal
 *    in a mask signal file
 *
 */

static char *sccs_id = "@(#)mask.c	1.9 1/23/97 ERL";

char *ProgName = "mask";
char *Version="1.9";
char *Date="1/23/97";

/*mask is useful for fading out or gating off selected segments of an
utterance.  In particular, it can be used in conjunction with the .f0
files produced by the fzero program to mask off the unvoiced segments
of an utterance or to substitute unvoiced regions from one version of
an utterance into another version.
*/

#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>

#define SYNTAX USAGE("mask [-f mask_field] [-s subfile] [-x debug_level] maskfile infile outfile")
#define ERROR_EXIT(text){(void) fprintf(stderr,"%s: %s - exiting\n", \
					av[0], text); exit(1);}
int debug_level=0; 

static long n_feasd_rec();
static long n_fea_rec();


main(ac, av)
     int ac;
     char **av;
{
  extern char *optarg;
  extern int optind;
  char *ifname, *ofname, *gfname, *sfname = NULL;  /* in, out, mask, sub */
  FILE *ifile, *ofile, *gfile, *sfile;
  struct header *ihd, *ohd, *ghd, *shd;
  long  inrec, gnrec, snrec;
  struct feasd *isd_rec, *osd_rec, *ssd_rec;
  struct fea_data *gfea_rec;
  float *idata, *gdata, *fgdata, *sdata;
  double *dgdata;
  short *odata, datatype;
  char c, *fldname="prob_voice";
  double stime, endtime, istart_time, gstart_time, sstart_time, ifreq,
     gfreq, sfreq;
  register int i, j, k, perf, tot, m, n;
  int l, ioi, iog, ios;
  double amp = 0, da;

  while((c = getopt(ac, av, "f:s:x:")) != EOF){
    switch(c){
    case 'f':
      fldname = optarg;
      break;
    case 's':
      sfname = optarg;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      SYNTAX;
    }
  }
	
  if( ac - optind != 3){
    SYNTAX;
  }

  /* initialze mask signal */
  gfname = eopen(av[0], av[optind], "r", NONE, NONE, &ghd, &gfile);
  l = 1;
  if(genhd_type("start_time", &l, ghd) == HD_UNDEF)
    ERROR_EXIT("Can't find start_time header item in mask signal")
  else gstart_time = *get_genhd_d("start_time", ghd);
  if(genhd_type("record_freq", &l, ghd) == HD_UNDEF)
    ERROR_EXIT("Can't find record_freq header item in mask signal")
  else gfreq = *get_genhd_d("record_freq", ghd);
  gfea_rec = allo_fea_rec(ghd);
  datatype = get_fea_type(fldname, ghd);
  if( datatype == DOUBLE ){
    if(!( dgdata = (double *) get_fea_ptr(gfea_rec, fldname, ghd))){
      fprintf(stderr,"%s: Can't find the field name %s - exiting\n", av[0], fldname);
      exit(1);
    }
  }
  else if(datatype == FLOAT){
    if(!(fgdata = (float *) get_fea_ptr(gfea_rec, fldname, ghd))){
      fprintf(stderr,"%s: Can't find the field name %s - exiting\n", av[0], fldname);
      exit(1);
    }
  }
  else{
    fprintf(stderr,"%s: Can't find the field name %s of FLOAT or DOUBLE type - exiting\n", av[0], fldname);
    exit(1);
  }
  gnrec = n_fea_rec(&gfile, &ghd);
  gdata = (float *) malloc( sizeof(float) * gnrec);
  spsassert(gdata,"malloc failed for gdata");
  for(i=0; i< gnrec; i++){
    get_fea_rec(gfea_rec, ghd, gfile);
    gdata[i] = (datatype == DOUBLE)? *dgdata : *fgdata;
    if((gdata[i] < 0 || gdata[i] > 1) && sfname )
      fprintf(stderr,"WARNING: The mask signal is not bounded by [0,1]");
  }

  /* init input signal */
  ifname = eopen(av[0], av[optind+1], "r", FT_FEA, FEA_SD, &ihd, &ifile);
  istart_time = *get_genhd_d("start_time", ihd);
  ifreq = *get_genhd_d("record_freq", ihd);
  if( gfreq > ifreq){
    ERROR_EXIT("record_freq in mask signal is larger than that in input signal")
  }
  inrec = n_feasd_rec(&ifile, &ihd);
  isd_rec = allo_feasd_recs( ihd, FLOAT, inrec, NULL, NO);
  idata = (float *) isd_rec->data;
  get_feasd_recs( isd_rec, (long) 0L, inrec, ihd, ifile);

  ioi = iog  = 0;		/* frame offsets */
  perf = 0.5 + ifreq / gfreq;
  if((stime = istart_time) < gstart_time) stime = gstart_time;
  ioi = 0.5 + (stime - istart_time) * ifreq;
  iog = 0.5 + (stime - gstart_time) * gfreq;
  if( (endtime = gstart_time + gnrec/gfreq) >
    istart_time + inrec/ifreq)
    endtime = istart_time + inrec/ifreq;
  tot = 0.5 + (endtime - stime) * ifreq;

  if(debug_level){
    fprintf(stderr,"%s: start_time %f, end_time %f\n", 
	    ifname, istart_time, istart_time + inrec/ifreq);
    fprintf(stderr,"%s: start_time %f, end_time %f\n", 
	    gfname, gstart_time, gstart_time + gnrec/gfreq);
    fprintf(stderr,"There'll be %d output samples\n", tot);
  }

  /* init sub signal */
  if(sfname){
    sfname = eopen(av[0], sfname, "r", FT_FEA, FEA_SD, &shd, &sfile);
    sstart_time = *get_genhd_d("start_time", shd);
    sfreq = *get_genhd_d("record_freq", shd);
    snrec = n_feasd_rec(&sfile, &shd);
    ssd_rec = allo_feasd_recs( shd, FLOAT, snrec, NULL, NO);
    sdata = (float *) ssd_rec->data;
    get_feasd_recs( ssd_rec, (long) 0L, snrec, shd, sfile);
    if(sfreq != ifreq)
      ERROR_EXIT("Original and substitute sample frequencies differ")
    if(stime < sstart_time)
      ERROR_EXIT("start_time in substitute is too late")
    ios = 0.5 + (stime - sstart_time)*sfreq;
    if( snrec - ios < tot ) 
      ERROR_EXIT("Not enough samples in the substitution file")
  }
  if(debug_level && sfname){
    fprintf(stderr,"%s: start_time %f, end_time %f\n", 
	    sfname, sstart_time, sstart_time + snrec/sfreq);
    fprintf(stderr,"%s: there are %d samples\n", sfname, snrec);
  }
    

  /* init output signal */
  ofname = eopen(av[0], av[optind+2], "w", NONE, NONE, &ohd, &ofile);
  ohd =  new_header(FT_FEA);
  init_feasd_hd(ohd, SHORT, 1, &stime, NO, ifreq );
  osd_rec = allo_feasd_recs( ohd, SHORT, tot, NULL, NO);
  odata = (short *) osd_rec->data;
  add_comment(ohd,get_cmd_line(ac,av));
  (void) add_genhd_d("record_freq", &ifreq, 1, ohd);
  (void) add_genhd_d("start_time", &stime, 1, ohd);
  (void) strcpy (ohd->common.prog, ProgName);
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  add_source_file (ohd, av[optind], ghd);
  add_source_file (ohd, av[optind+1], ihd);
  if(sfname) add_source_file (ohd, sfname, shd);
  (void) write_header ( ohd, ofile );

  k = 0;
  if( sfname ) {		/* substitution will be performed */
    m=ioi;
    n=ios;
    for (i = 1+iog; i< gnrec; i++) {
      amp = gdata[i-1];
      da = (gdata[i] - gdata[i-1]) / perf;
      for(j=0; (j++ < perf) && (k < tot); amp += da, m++, n++, k++)
	odata[k] = 0.5 + (amp * idata[m]) + ((1-amp) * sdata[n]);
    }      
  }
  else{
    m = ioi;
    for (i = iog+1; i < gnrec; i++) {
      amp = gdata[i-1];
      da = (gdata[i] - gdata[i-1]) / perf;
      for(j=0; (j++ < perf) && (k < tot); amp += da, m++, k++)
	odata[k] = 0.5 + (amp * idata[m]);
    }
  }
  put_feasd_recs( osd_rec, 0L, tot, ohd, ofile);

  exit(0);
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
