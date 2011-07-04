/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description: writes FEA file data to matlab .mat file
 *
 */

static char *sccs_id = "@(#)fea2mat.c	1.4	1/22/97	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define Fprintf (void)fprintf
#define SYNTAX {\
Fprintf(stderr, "%s: export ESPS FEA files into MATLAB files.\n", PROG); \
Fprintf(stderr, "Usage:  %s [-x debug_level] [-r range] -f field_name infile outfile \n", PROG); \
exit(1); }

typedef struct {
     long type;   /* type */
     long mrows;  /* row dimension */
     long ncols;  /* column dimension */
     long imagf;  /* flag indicating imag part */
     long namlen; /* name length (including NULL) */
} Fmatrix;

int matlab_machine_id = 1000;

int debug_level=0;
char *PROG="fea2mat";

void range_switch();
char *type_convert();
char *marg_index();
#ifndef DEC_ALPHA
char *calloc();
#endif

main( argc, argv)
int argc;
char *argv[];
{
    extern char *optarg;
    extern int optind;
    char opt;

    struct header *inhd;
    struct fea_data *inrec;
    int eof;

    FILE *fpin, *fpout; 
    char *fnamein;

    Fmatrix x;

    char *fname=NULL;

    double *rptr;
    double *iptr;
    int cntr;

    short field_type;
    long field_len;
    short rank;
    long *dim;
    
    int recno;
    int start=0, end=0, nan;
    long arrsize;

    int n, m;
    int fflag=0;

    while ( (opt = getopt(argc, argv, "f:x:r:")) != EOF ) 
	switch(opt) {
	    case 'x': {
		debug_level=atoi(optarg);
		break;
	    }
	    case 'f': {
		fflag++;
		fname = optarg;
		spsassert( fflag == 1, "only one field at a time.");
		if ( debug_level )
		    Fprintf(stderr, "%s: field %s specified.\n", PROG, optarg);
		break;
	    }
	    case 'r': {
		range_switch( optarg, &start, &end, (int)0);
	    }
	}
    if ( fflag != 1 ) {
	Fprintf(stderr, "must specify a field specified - exiting.\n"); 
	SYNTAX;
    }

    if ( (optind+2) != argc){
	Fprintf(stderr, "%s: must specify an input file and an output file.\n", PROG);
	SYNTAX;
    }
    
    fnamein = eopen( PROG, argv[optind++], "r", FT_FEA, NONE, &inhd, &fpin);
    spsassert( fpin != stdin, "standard input not allowed.");

    inrec = allo_fea_rec(inhd);
    if ( start == 0 )
	start = 1;
    if ( end == 0 )
	end = inhd->common.ndrec;
    spsassert(end != -1,
	      "must specify an explicit end of range with this file format.");
    nan = end - start + 1;
    
    if (debug_level)
	Fprintf(stderr, "%s: processing %s from record %d to %d.\n", PROG, fnamein, start, end);

    fpout = fopen(argv[optind++], "w");
    spsassert(fpout != NULL, "can't open output file.");
    
    if ( (field_type = get_fea_type( fname, inhd)) == UNDEF ) {
	Fprintf(stderr, "%s: field %s undefined in %s - exiting.\n", PROG, fname, fnamein);
	exit(1);
    } 
    
    field_len = get_fea_siz(fname, inhd, &rank, &dim);
    spsassert(rank==0 || rank==1 || rank==2, "improper rank specified.");
    if ( rank == 0 || rank == 1 )  {
	arrsize = nan*field_len;
	x.mrows = nan;
	x.ncols = field_len;
    }
    if ( rank == 2 ) {
	spsassert( nan==1, "if rank=2, can only process a single record.");
	arrsize = dim[0] * dim[1];
	x.mrows = dim[0];
	x.ncols = dim[1];
    }
    
    rptr = (double *) calloc( (unsigned) arrsize, sizeof(double));
    spsassert( rptr != NULL, "can't allocate memory for output array.");
    
    if ( is_type_complex(field_type) ) {
	x.imagf=1;
	iptr = (double *) calloc( (unsigned) arrsize, sizeof(double));
	spsassert( iptr != NULL, "can't allocate memory for output array.");
	if (debug_level) 
	    Fprintf(stderr, " %s:\t%d x %d\tIMAG\n",
		    fname, x.mrows, x.ncols, field_len);
    } else {
	x.imagf=0;
	if (debug_level) 
	    Fprintf(stderr, " %s:\t%d x %d\tREAL\n",
		    fname, x.mrows, x.ncols, field_len);
    }

    for (recno=1; recno<start; recno++)
    {
	eof = get_fea_rec(inrec, inhd, fpin) == EOF;
	spsassert(!eof, "premature end of file encountered in input.");
    }

    for (recno=start; recno<=end; recno++) {
	eof = get_fea_rec(inrec, inhd, fpin) == EOF;
	spsassert(!eof, "premature end of file encountered in input.");

	if (debug_level>1)
	    Fprintf(stderr,"Read Record %d:\n", recno);

	if ( !is_type_complex(field_type) ) {
	    double *ptr=NULL;
	    double **mptr;

	    ptr = (double *) calloc( (unsigned) field_len, sizeof(double));
	    (void) type_convert((long)field_len,
				(char *) get_fea_ptr(inrec, fname, inhd),
				(int) field_type, (char *) ptr, DOUBLE,
				(void(*)()) NULL);
	    if ( rank != 2 ) {
		for (n=0; n<x.ncols; n++) {
		    cntr = n * x.mrows + (recno-start);
		    rptr[cntr] = ptr[n];
		}
	    } else {
		mptr = (double **) marg_index((char *) ptr,
					      (short)2, dim, DOUBLE, 0); 
		for (m=0; m<x.mrows; m++)
		    for (n=0; n<x.ncols; n++) { 
			cntr = n * x.mrows + m;
			rptr[cntr] = mptr[m][n];
		    }
	    }
	} else {
	    double_cplx *ptr=NULL;
	    double_cplx **mptr;

	    ptr = (double_cplx *) calloc( (unsigned) field_len, sizeof(double_cplx));
	    (void)type_convert( (long)field_len, (char *) get_fea_ptr(inrec, fname, inhd), (int)field_type, 
			 (char *) ptr, DOUBLE_CPLX, (void(*)())NULL);
	    if ( rank != 2 ) {
		for (n=0; n<x.ncols; n++) {
		    cntr = n * x.mrows + (recno-start) ;
		    rptr[cntr] = ptr[n].real;
		    iptr[cntr] = ptr[n].imag;
		}
	    } else {
		mptr = (double_cplx **) marg_index( (char *) ptr, (short)2, dim, DOUBLE_CPLX, 0); 
		for (m=0; m<x.mrows; m++)
		    for (n=0; n<x.ncols; n++) { 
			cntr = n * x.mrows + m;
			rptr[cntr] = mptr[m][n].real;
			iptr[cntr] = mptr[m][n].imag;
		    }
	    }
	}
    }
    (void)fclose(fpin);
    if (debug_level>1)
	for (m=0; m<x.mrows; m++) {
	    for (n=0; n<x.ncols; n++) { 
		cntr = n * x.mrows + m;	
		if ( is_type_complex( field_type ) )
		    Fprintf(stderr, " % e + % e", rptr[cntr], iptr[cntr]);
		else
		    Fprintf(stderr, " % e", rptr[cntr]);
	    }
	    Fprintf(stderr, "\n");
	}

    x.type = matlab_machine_id;
    x.namlen = strlen(fname) + 1;
    (void)fwrite( (char *)&x, sizeof(Fmatrix), (int)1, fpout);
    (void)fwrite( (char *)fname, sizeof(char), (int)x.namlen, fpout);
    (void)fwrite( (char *)rptr, sizeof(double), (int)arrsize, fpout);
    if (x.imagf)
	(void)fwrite( (char *)iptr, sizeof(double), (int)arrsize, fpout);
    (void)fclose(fpout);
}


