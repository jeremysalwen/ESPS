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
 * Brief description: FIR design by Kaiser windowing
 *
 */

static char *sccs_id = "@(#)win_filt.c	1.4	7/9/93	ERL";

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <esps/esps.h>
#include <esps/constants.h>
#include <esps/window.h>
#include <esps/fea.h>
#include <esps/feafilt.h>

#define SYNTAX USAGE ("win_filt [-P param_file][-x debug_level] filt_file");
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
					 argv[0], text); exit(1);}

#define MAX_NBANDS 10

double pow();
int debug_level = 0;

main( argc, argv)
     int argc;
     char **argv;
{
  extern char *optarg;
  extern optind;
  char *Date="06/05/93";
  char *Version="1.0";
  FILE *fpout;
  struct header *oh;
  struct feafilt *frec;
  char *param_file = NULL;
  char *filt_file = NULL;
  filtdesparams *fdp;
  char *str, stri[3];
  char *stu = "band_edge", *stv="_des", *stb="band";
  int c, i, j, eflag = -99;
  float *des, *edges, trans_band, rej_db;
  int nbands;
  float *hd_edges, *hd_des;
  long Ntaps=0;
  double sf, sf2, del_omega, omegac, sinc0, G, *filter, beta, y, alpha, *sinc,
     delay = 0;

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
      else TRYOPEN("iir_filt", filt_file, "w", fpout);
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
	
  if( symtype("samp_freq") != ST_UNDEF) sf = getsym_d("samp_freq");
  else ERROR_EXIT("samp_freq not found");	
  sf2 = sf/2;
  if( symtype("nbands") != ST_UNDEF) nbands = getsym_i("nbands");
  else ERROR_EXIT("nbands not found");	
  if( symtype("trans_band") != ST_UNDEF) trans_band = getsym_d("trans_band");
  else ERROR_EXIT("trans_band not found");
  if( trans_band >= sf2 ) ERROR_EXIT("Invalid transition band, too large");
  if( symtype("rej_db") != ST_UNDEF) rej_db = getsym_d("rej_db");
  else ERROR_EXIT("rej_db not found");	
  if( symtype("evenflag") != ST_UNDEF) eflag = getsym_i("evenflag");

  edges = (float *) malloc( sizeof(float) * (nbands+1));
  des = (float *) malloc( sizeof(float) * (nbands+1));
  str = (char *) malloc(sizeof(char)*20);
  for( i =1, j=0 ;i<=nbands+1; i++, j++){
    str = (char *) strcpy( str, stu);
    sprintf(stri, "%d", i);
    str = (char *) strcat(str, stri);
    /* find edges */
    if( symtype(str) != ST_UNDEF){
      if( (edges[j] = getsym_d(str)) > sf2) 
	ERROR_EXIT("band edges greater than Nyquist rate");
    }
    else{
      fprintf(stderr,"%s: missing %d-th band edge\n", argv[0], i);
      exit(1);
    }
  }
  for(j=1; j<=nbands; j++)
    if( edges[j] < edges[j-1] )
      ERROR_EXIT("band edges must be specified in an increasing order");
  if( edges[0] !=0 || edges[nbands] != sf2)
    ERROR_EXIT("first and last bandedges must start at 0 and end at Nyquist rate");
  for(i=1,j=0 ; i<=nbands; i++, j++){
    str = (char *) strcpy( str, stb);
    sprintf(stri, "%d", i);
    str = (char *) strcat(str, stri);
    str = (char *) strcat(str, stv);
    /* find desired value */
    if( symtype(str) != ST_UNDEF)
      des[j] = getsym_d(str);
    else{
      fprintf(stderr,"%s: missing desired value for band %d\n", argv[0], i);
      exit(1);
    }
  }
  des[nbands] = 0;

  /* get ideal sinc response */

  /* determine length and beta */
  del_omega = 2 * PI * trans_band / sf;
  if ( rej_db > 50 )
    beta = 0.1102 * ( rej_db - 8.7);
  else if ( rej_db < 21 )
    beta = 0;
  else
    beta = 0.5842 * pow((rej_db - 21), 0.4) + 0.07886 * (rej_db - 21);
  y = ceil((rej_db - 8)/(2.285 * del_omega));
  Ntaps = ( (int) y  % 2  == 0 ) ? y+1 : y;   /* make sure length is odd */
  if(eflag == 0) 
    Ntaps = ( (int) y  % 2  == 0 ) ? y : y+1;  /* even length desired */
  if(eflag == 1)
    Ntaps = ( (int) y  % 2  == 0 ) ? y+1 : y;  /* odd length desired */
  alpha = (Ntaps - 1 ) / 2;                   

  sinc = (double *) malloc(sizeof(double )*Ntaps);
  spsassert(sinc, "can't malloc sinc");
  for(i=0; i<Ntaps; i++) sinc[i]=0;
  filter = (double *) malloc(sizeof(double )*Ntaps);
  spsassert(filter, "can't malloc filter");
  for(i=0; i< Ntaps; i++){
    for(j=0; j< nbands; j++){
      G=des[j] - des[j+1];
      omegac = 2 * PI * edges[j+1]/sf;
      sinc0 = omegac / PI;
      if( i-alpha == 0) sinc[i] += G * sinc0;
      else sinc[i] += G * sin( (i-alpha) * omegac) / (PI *(i-alpha));
    }
  }
  windowd( Ntaps, sinc, filter, WT_KAISER, &beta);

  /* set up output generic header items */
  hd_edges = (float *) malloc(sizeof(float) * nbands+1);
  hd_des = (float *) malloc(sizeof(float) *nbands);
  for(i=0; i<=nbands; i++){ 
    hd_edges[i] = edges[i] / sf;
  }
  for(i=0; i<nbands; i++){
    hd_des[i] = des[i];
  }

  fdp = (filtdesparams *) calloc( 1, sizeof(filtdesparams));
  fdp->define_pz = NO;
  fdp->method = WINDOW_METH;
/*  fdp->func_spec = BAND;*/
  fdp->nbands = nbands;
  fdp->bandedges = hd_edges;
  fdp->gains = hd_des;
  delay = (double) (Ntaps-1) / 2.0;

  oh = new_header(FT_FEA);
  (void) strcpy (oh->common.prog, argv[0]);
  (void) strcpy (oh->common.vers, Version);
  (void) strcpy (oh->common.progdate, Date);
  oh->common.tag = NO;
  add_comment(oh, get_cmd_line(argc, argv));
  init_feafilt_hd( oh, (long)(Ntaps), 0L, fdp);
  (void)add_genhd_f("trans_band", &trans_band, 1, oh);
  (void)add_genhd_f("rej_db", &rej_db, 1, oh);
  (void)add_genhd_d("samp_freq", &sf, 1, oh);
  (void)add_genhd_d("delay_samples", &delay, 1, oh);

  (void) write_header ( oh, fpout );
  frec = allo_feafilt_rec(oh);
  *frec->num_size = Ntaps;
  for(i=0;i<Ntaps;i++) frec->re_num_coeff[i] = filter[i];
  put_feafilt_rec( frec, oh, fpout);
}

