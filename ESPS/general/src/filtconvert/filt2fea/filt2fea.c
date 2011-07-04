/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description: transform ESPS FILT file to FEA_FILT file
 *
 */
#ifndef lint
static char *sccs_id = "@(#)filt2fea.c	1.7	3/30/92	ERL";
#endif

#include <stdio.h>
#define LONG_MAX 2147483647

#include <esps/esps.h>
#include <esps/filt.h>
#include <esps/fea.h>
#include <esps/feafilt.h>
#include <esps/spsassert.h>

#define Fprintf (void)fprintf
#define SYNTAX {\
Fprintf(stderr, "%s: transform ESPS FILT file to a ESPS FEA_FILT file\n", PROG);\
Fprintf(stderr, "usage: %s [options] filt_file fea_filt_file\n", PROG);\
Fprintf(stderr, " -P parameter file (default params)\n");\
Fprintf(stderr, " -x debug level (default 0)\n");\
Fprintf(stderr, " -r first_record:last_record (default 1:last_in_file)\n");\
exit(1);}

char *PROG       = "filt2fea";
char *DATE       = "3/30/92";
char *VERSION    = "1.7";

int  debug_level = 0;

char *get_cmd_line();
int  atoi();

main(argc, argv)
    int     argc;
    char    **argv;
{
  extern                  optind;
  extern       char       *optarg;

  char                    *pfile = NULL;        /* param file */
  char                    *range = NULL;	    /* range argument */
  long                    start=0;                  /* first input record */
  long                    last=0;                   /* last input record */
  long                    recno;                    /* input record counter */
  char                    c;
  int                     k;
  short                   xflag=0;                  /* option x flag */
  short                   rflag=0;                  /* option r flag */
  char                    *iname;                   /* input file name */
  char                    *oname;                   /* output file name */
  struct       header     *ih, *oh;		    /* header ptrs */
  FILE                    *fpin;                    /* input file stream ptr */ 
  FILE                    *fpout;       	    /* output file steam ptr */
  struct       filt_data  *filt_rec;		    /* FILT data structure */
  struct       feafilt    *fea_filt_rec;            /* FEA_FILT data structure */
  filtdesparams           fdp;                      /* FEA_FILT header info */

  while ( (c = getopt( argc, argv, "x:P:")) != EOF )
    switch(c) {
	case 'x': {
	    debug_level = atoi(optarg); 
	    xflag++;
	    break;
	  }
	case 'P': {
	    pfile = optarg;
	    break;
	  }
	case 'r': {
	    range = optarg;
	    rflag++;
	    break;
	  }
	default: {
	    SYNTAX;
	  }
      }

  if ( argc < optind + 2 ) {
      Fprintf(stderr, "%an input and an output filename must be given.\n");
      SYNTAX;
    }
  if ( argc > optind + 2) {
      Fprintf(stderr, "too many options.\n");
      SYNTAX;
    }

  iname = eopen( PROG, argv[optind++], "r", FT_FILT, NONE, &ih, &fpin);
  spsassert( fpin != NULL, "can't open input file.");

  oname = eopen( PROG, argv[optind++], "w", NONE, NONE, (struct header **)NULL, &fpout);
  spsassert( fpout != NULL, "can't open output file.");


  (void) read_params(pfile, SC_NOCOMMON, (char *)NULL);

  /* process parameters */

  if ( !xflag && symtype("debug_level") != ST_UNDEF)
    debug_level = getsym_i("debug_level");

  if ( range != NULL ) 
    lrange_switch( range, &start, &last, 1);

  if ( !start && symtype("start") != ST_UNDEF ) 
    start = getsym_i("start");
  if ( !start )
    start = 1;

  if ( !last  && symtype("nan") != ST_UNDEF ) 
    last = start + getsym_i("nan");
  if ( !last ) 
    last = ( fpin == stdin ) ? LONG_MAX : ih->common.ndrec;

  if (debug_level)  {
    Fprintf(stderr, "%s: input file %s, output file %s\n", PROG, iname, oname);
    Fprintf(stderr, "%s: reading record(s) %d to %d from %s\n", PROG, start, last, iname);
  }

  
  /* read input header and allocate record */
  filt_rec = allo_filt_rec(ih);
  spsassert( filt_rec != NULL, "can't allocate input filter record\n");


  /* allocate output header and record based on input record */
  if (debug_level > 1 ) 
    Fprintf(stderr, "creating fea_filt header based on input filt header.\n");

  fdp.filter_complex  = NO;
  fdp.define_pz       = NO;
  fdp.type            = ih->hd.filt->type;
  fdp.method          = ih->hd.filt->method;
  fdp.func_spec       = ih->hd.filt->func_spec;
  fdp.g_size          = ih->hd.filt->g_size;
  fdp.nbits           = ih->hd.filt->nbits;
  fdp.nbands          = ih->hd.filt->nbands;
  fdp.npoints         = ih->hd.filt->npoints;
  if (fdp.nbands || fdp.npoints)
    fdp.gains = ih->hd.filt->gains;
  if (fdp.nbands)
    fdp.bandedges = ih->hd.filt->bandedges;
  if (fdp.nbands || fdp.npoints)
    fdp.wts = ih->hd.filt->wts;
  
  oh = new_header(FT_FEA);
  add_source_file( oh, iname, ih);
  add_comment( oh, get_cmd_line(argc, argv));
  (void) strcpy(oh->common.prog, PROG);
  (void) strcpy(oh->common.vers, VERSION);
  (void) strcpy(oh->common.progdate, DATE);

  init_feafilt_hd( oh, (long)ih->hd.filt->max_num, (long)ih->hd.filt->max_den, &fdp);

  copy_genhd( oh, ih, (char *)NULL);

  write_header( oh, fpout);

  fea_filt_rec = allo_feafilt_rec(oh);  
  spsassert( fea_filt_rec != NULL, "can't allocate output record.");

  if (debug_level > 1)
    Fprintf(stderr,"writing output records.\n");

  /* skip start-1 input records */
  for (recno=1; recno<start; recno++)
    spsassert( get_filt_rec( filt_rec, ih, fpin) != EOF, "premature end of file.");
    

  for (recno=start; recno<=last; recno++) {
      if ( get_filt_rec( filt_rec, ih, fpin) == EOF )
	break;
      if ( debug_level > 1 )
	Fprintf(stderr, "record = %d\n", recno);
      if ( debug_level > 2 )
	print_filt_rec( filt_rec, ih, stderr);
      
      /* copy filt fields to fea_filt fields */
      *fea_filt_rec->num_size   = filt_rec->filt_func.nsiz;
      *fea_filt_rec->denom_size = filt_rec->filt_func.dsiz;

      for (k=0; k<filt_rec->filt_func.nsiz; k++) 
	fea_filt_rec->re_num_coeff[k] = filt_rec->filt_func.zeros[k]; 

      for (k=0; k<filt_rec->filt_func.dsiz; k++)
	fea_filt_rec->re_denom_coeff[k] = filt_rec->filt_func.poles[k];

      /* write fea_filt record */
      put_feafilt_rec( fea_filt_rec, oh, fpout);
    }
  if ( fpin != stdin && recno < last ) 
    Fprintf(stderr, "%s: Premature EOF - %d records expected but only %d read\n",
	    PROG, last - start + 1, recno - start + 1);
  if (debug_level) 
    Fprintf(stderr, "%s: read %d records from %s\n", PROG, recno-start, iname);

  fclose(fpin);
  fclose(fpout);
}

