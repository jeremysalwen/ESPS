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
 * Brief description: read variable from Matlab .mat file and write to FEA file
 *
 */

static char *sccs_id = "@(#)mat2fea.c	1.2	10/20/93	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define Fprintf (void)fprintf
#define SYNTAX {\
Fprintf(stderr, "%s: import MATLAB files into ESPS FEA files.\n", PROG); \
Fprintf(stderr, "Usage:  %s [-c \"comment text\"] [-x debug_level] [-m] -f variable infile.mat outfile.fea \n", PROG); \
exit(1); }

typedef struct {
     long type;   /* type */
     long mrows;  /* row dimension */
     long ncols;  /* column dimension */
     long imagf;  /* flag indicating imag part */
     long namlen; /* name length (including NULL) */
} Fmatrix;


int debug_level=0;
char *PROG="mat2fea";
char *VERSION="1.2";
char *DATE="10/20/93";

char *get_cmd_line();
char *marg_index();
#ifndef DEC_ALPHA
char *strcpy();
char *calloc();
#endif

main( argc, argv)
int argc;
char **argv;
{
    extern char *optarg;
    extern int optind;
    char opt;

    struct header *outhd;
    struct fea_data *outrec;
    char *comment=NULL;

    FILE *fpin, *fpout; 
    char *fnameout;

    Fmatrix x, xread;

    char *fname=NULL;
    char *targ_fname;
    long dim[2];
    short rank;
    long size;
    short type;

    char *cptr;
    double *dd, **mdd;
    double_cplx *cdd, **cmdd;

    double *rmptr, *imptr;

    int arrsize;
    int cnt;
    int fflag=0;
    int mflag=0;
    int field_found=0;
    int maxfnamelen=0;

    int m, n;

    while ( (opt = getopt(argc, argv, "x:c:f:m")) != EOF ) 
	switch(opt) {
	    case 'x': {
		debug_level=atoi(optarg);
		break;
	    }
	    case 'f': {
		fflag++;
		targ_fname = optarg;
		break;
	    }
	    case 'c': {
		comment = optarg;
		break;
	    }
	    case 'm': {
		mflag=1;
		break;
	    }
	}
    if (!fflag) {
	Fprintf(stderr, "%s: must specify field name.\n", PROG);
	SYNTAX;
    }

    if ( (optind+2) != argc) {
	Fprintf(stderr, "%s: must specify an input and an output file.", PROG);
	SYNTAX;
    }
    
    if ( (fpin=fopen(argv[optind++], "r")) == NULL ) {
	Fprintf(stderr, "%s: can't open input file %s.\n", argv[optind-1], PROG);
	exit(1);
    }

    field_found = 0;
    while ( fread((char *)&x, sizeof(Fmatrix), (int)1, fpin ) == 1 && !field_found ) {

	if (x.mrows<=0 || x.ncols <=0) {
	    Fprintf(stderr, "trouble reading input file - bad header structure.\n");
	    exit(1);
	}
	if (x.namlen > maxfnamelen) {
	    maxfnamelen = x.namlen;
	    if (fname != NULL)
		free(fname);
	    fname = (char *) calloc( (unsigned)  maxfnamelen, sizeof(char));
	    spsassert( fname!= NULL, "can't allocate memory for field name.");
	}

	if ( fread((char *) fname, sizeof(char), (int)x.namlen, fpin) != x.namlen) 
	    break;

	if (debug_level) {
	    if (x.imagf)
		Fprintf(stderr, "%s: found field %s %dx%d IMAG\n", PROG, fname, x.mrows, x.ncols);
	    else
		Fprintf(stderr, "%s: found field %s %dx%d REAL\n", PROG, fname, x.mrows, x.ncols);
	}    

	if ( strcmp( fname, targ_fname ) == 0 ) {
	    xread = x;
	    spsassert( !(xread.type & 1), "matlab text fields not supported.");
	    field_found=1;
	    arrsize = xread.mrows * xread.ncols;
	    rmptr = (double *) calloc( (unsigned) arrsize, sizeof(double));
	    spsassert( rmptr!=NULL, "can't allocate output matrix memory.");
	    cnt = fread((char *)rmptr, sizeof(double), (int) arrsize, fpin);
	    spsassert(cnt == arrsize, "short read from input file.");
	    if (xread.imagf) {
		imptr = (double *) calloc( (unsigned) arrsize, sizeof(double));
		spsassert( imptr!=NULL, "can't allocate output matrix memory.");
		cnt = fread((char *)imptr, sizeof(double), (int) arrsize, fpin);
		spsassert(cnt == arrsize, "short read from input file.");
	    }
	}
    }
    (void)fclose(fpin);
    if (!field_found) {
	Fprintf(stderr, "%s: couldn't field field %s.\n", PROG, targ_fname);
	exit(1);
    }

    fnameout = eopen( PROG, argv[optind++], "w", FT_FEA, NONE, &outhd, &fpout);
    if (debug_level)
	Fprintf(stderr, "%s: writing to FEA file %s\n", PROG, fnameout);

    outhd = new_header(FT_FEA);
    (void) strcpy(outhd->common.prog, PROG);
    (void) strcpy(outhd->common.vers, VERSION);
    (void) strcpy(outhd->common.progdate, DATE);

    add_comment( outhd, get_cmd_line(argc, argv));
    if (comment != NULL )
	add_comment( outhd, comment);

    if (mflag) {
	dim[0] = xread.mrows;
	dim[1] = xread.ncols;
	size = dim[0] * dim[1];
	rank = 2;
    } else {
	if (xread.ncols==1)
	    rank = 0;
	else
	    rank = 1;
	size = xread.ncols;
    }
    if (xread.imagf)
	type = DOUBLE_CPLX;
    else
	type = DOUBLE;

    if ( add_fea_fld( fname, size, rank, dim, type, (char **)NULL, outhd) != 0 ) {
	Fprintf(stderr, "%s: unable to add output field.\n");
	exit(1);
    }
    
    write_header( outhd, fpout);
    outrec = allo_fea_rec(outhd);

    if (mflag) {

	if ( xread.imagf ) {
	    cptr = (char *) get_fea_ptr( outrec, fname, outhd);
	    cmdd = (double_cplx **) marg_index( (char *) cptr, (short)2, dim, DOUBLE_CPLX, 0); 
	    spsassert( cmdd!=NULL, "can't allocate output record memory.");
	}  else {
	    cptr = (char *) get_fea_ptr( outrec, fname, outhd);
	    mdd = (double **) marg_index( (char *) cptr, (short)2, dim, DOUBLE, 0); 
	    spsassert( mdd!=NULL, "can't allocate output record memory.");
	}

	for (m=0; m<xread.mrows; m++)
	    for (n=0; n<xread.ncols; n++) {
		cnt = n * xread.mrows + m;
		if ( xread.imagf ) {
		    cmdd[m][n].real = rmptr[cnt];
		    cmdd[m][n].imag = imptr[cnt];
		} else 
		    mdd[m][n] = rmptr[cnt];
	    } 

	put_fea_rec( outrec, outhd, fpout);

    } else {

	if ( xread.imagf ) {
	    cdd = (double_cplx *) get_fea_ptr( outrec, fname, outhd);
	    spsassert( cdd!=NULL, "can't allocate output record memory.");
	} else {
	    dd = (double *) get_fea_ptr( outrec, fname, outhd);
	    spsassert( dd!=NULL, "can't allocate output record memory.");
	} 
	
	for (m=0; m<xread.mrows; m++) {
	    for (n=0; n<xread.ncols; n++) {
		cnt = n * xread.mrows + m;
		if (xread.imagf) {
		    cdd[n].real = rmptr[cnt];
		    cdd[n].imag = imptr[cnt];
		} else
		    dd[n] = rmptr[cnt];
	    }		
	    put_fea_rec( outrec, outhd, fpout);
	}
    }
    
    (void)fclose(fpout);
}


