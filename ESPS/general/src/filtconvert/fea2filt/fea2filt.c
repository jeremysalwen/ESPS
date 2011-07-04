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
 * Brief description: transform ESPS FEA_FILT file to FILT file
 *
 */
#ifndef lint
static char *sccs_id = "@(#)fea2filt.c	1.5	8/9/91	ERL";
#endif

#include <stdio.h>
#define LONG_MAX 2147483647

#include <esps/esps.h>
#include <esps/filt.h>
#include <esps/fea.h>
#include <esps/feafilt.h>
#include <esps/spsassert.h>

#define Fprintf (void)fprintf
#define GHIL(name) (get_genhd_l(name, ih)==NULL) ? 0 : *get_genhd_l(name, ih)
#define GHIS(name) (get_genhd_s(name, ih)==NULL) ? 0 : *get_genhd_s(name, ih)
#define GHIF(name) (get_genhd_f(name, ih)==NULL) ? 0 : get_genhd_f(name, ih)
#define SYNTAX {\
Fprintf(stderr, "%s: transform ESPS FEA_FILT file to an ESPS FILT file\n", PROG);\
Fprintf(stderr, "usage: %s [options] fea_filt_file filt_file\n", PROG);\
Fprintf(stderr, " -P parameter file (default params)\n");\
Fprintf(stderr, " -x debug level (default 0)\n");\
Fprintf(stderr, " -r first_record:last_record (default 1:last_in_file)\n");\
exit(1);}

char *PROG       = "fea2filt";
char *DATE       = "8/9/91";
char *VERSION    = "1.5";

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

  while ( (c = getopt( argc, argv, "x:P:r:")) != EOF )
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

  iname = eopen( PROG, argv[optind++], "r", FT_FEA, FEA_FILT, &ih, &fpin);
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
  fea_filt_rec = allo_feafilt_rec(ih);
  spsassert( fea_filt_rec != NULL, "can't allocate input filter record\n");

  /* allocate output header and record based on input record */
  if (debug_level > 1 ) 
    Fprintf(stderr, "creating filt header based on input fea_filt header.\n");

  if ( GHIS("filter_complex") == YES ) 
    Fprintf(stderr, "%s: WARNING - discarding imaginary parts of filter coefficients!\n", PROG);

  oh = new_header(FT_FILT);
  oh->hd.filt->max_den   = GHIL("max_denom");
  oh->hd.filt->max_num   = GHIL("max_num");
  oh->hd.filt->type      = GHIS("type");
  oh->hd.filt->method    = GHIS("method");
  oh->hd.filt->func_spec = GHIS("func_spec");
  oh->hd.filt->g_size    = GHIL("g_size");
  oh->hd.filt->nbits     = GHIL("nbits");
  oh->hd.filt->nbands    = GHIL("nbands");
  oh->hd.filt->npoints   = GHIL("npoints");
  if (oh->hd.filt->nbands || oh->hd.filt->npoints)
    oh->hd.filt->gains     = GHIF("gains");
  if (oh->hd.filt->nbands)
    oh->hd.filt->bandedges = GHIF("bandedges");
  if (oh->hd.filt->nbands || oh->hd.filt->npoints)
    oh->hd.filt->wts       = GHIF("wts");

  add_source_file( oh, iname, ih);
  add_comment( oh, get_cmd_line(argc, argv));
  copy_genhd( oh, ih, (char *) NULL);
  (void) strcpy(oh->common.prog, PROG);
  (void) strcpy(oh->common.vers, VERSION);
  (void) strcpy(oh->common.progdate, DATE);

  write_header( oh, fpout);

  filt_rec = allo_filt_rec(oh);  
  spsassert( filt_rec != NULL, "can't allocate output record.");

  if (debug_level > 1)
    Fprintf(stderr,"writing output records.\n");

  /* skip start-1 input records */
  for (recno=1; recno<start; recno++)
    spsassert( get_feafilt_rec( fea_filt_rec, ih, fpin) != EOF, "premature end of file.");
    
  for (recno=start; recno<=last; recno++) {
      if ( get_feafilt_rec( fea_filt_rec, ih, fpin) == EOF )
	break;
      if ( debug_level > 1 )
	Fprintf(stderr, "record = %d\n", recno);
      
      /* copy filt fields to fea_filt fields */
      filt_rec->filt_func.nsiz = *fea_filt_rec->num_size;
      filt_rec->filt_func.dsiz = *fea_filt_rec->denom_size;

      for (k=0; k<filt_rec->filt_func.nsiz; k++) 
	filt_rec->filt_func.zeros[k] = fea_filt_rec->re_num_coeff[k]; 

      for (k=0; k<filt_rec->filt_func.dsiz; k++)
	filt_rec->filt_func.poles[k] = fea_filt_rec->re_denom_coeff[k];

      if ( debug_level > 2 )
	print_filt_rec( filt_rec, oh, stderr);

      /* write fea_filt record */
      put_filt_rec( filt_rec, oh, fpout);
    }
  if ( fpin != stdin && recno < last ) 
    Fprintf(stderr, "%s: Premature EOF - %d records expected but only %d read.\n",
	    PROG, last - start + 1, recno - start + 1);
  if (debug_level) 
    Fprintf(stderr, "%s: read %d records from %s\n", PROG, recno-start, iname);

  fclose(fpin);
  fclose(fpout);
}

