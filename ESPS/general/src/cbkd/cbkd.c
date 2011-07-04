/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1991-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This program is used to compute a distortion table from a 
 * FEA_VQ codebook record 
 */

static char *sccs_id = "@(#)cbkd.c	1.3	1/21/97	ESI/ERL";

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
#include <esps/spsassert.h>
#include <esps/limits.h>

#define Fprintf (void)fprintf
#define Fflush (void)fflush
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
		ProgName, text); exit(1);}
#define SYNTAX USAGE ("cbkd [-P param] [-x debug_level] [-r range] [-t table_name] cbkin.fea_vq cbkout.fea_vq")

int getopt ();
#ifndef IBM_RS6000
int atoi(), fclose();
#endif
char *get_fea_ptr();

extern  optind;
extern	char *optarg;
/*
char *eopen();
char getsym_i();
char *getsym_s();
int symtype();
long *get_genhd_l();
struct header *new_header();
long *add_genhd_l();
struct fea_data *allo_fearec();
*/

char *PROG = "cbkd";
char *VERSION = "1.3";
char *DATE = "1/21/97";
int debug_level = 0; 

void lrange_switch();

main (argc, argv)
int argc;
char **argv;
{
    int             c;
    int             ptr1, ptr2;

    FILE            *incbk_fp;
    char            *incbk_fname;
    struct header   *incbk_hd;
    struct vqcbk    *incbk_rec;
    int             incbk_recno=0;
    int             cw_dim=0;
    long            design_size=0;
    long            incbk_size;

    FILE            *outcbk_fp;
    char            *outcbk_fname;
    struct header   *outcbk_hd;
    struct vqcbk    *outcbk_rec;
    struct fea_data *outcbk_fea_rec;
    char            *pfile = "params";
    char            *table_field_name = "distance_table";
    long            dimen[2];
    double          *distances=NULL;
    double          dist;

    int             tflag=0;
    int             rflag=0;
    char            *rrange=NULL;
    long            start=1;
    long            nan=LONG_MAX;
    long            last=LONG_MAX;
    int             xflag=0;

    double dist_l2();

    while ( (c=getopt(argc, argv, "P:x:t:r:")) != EOF ) 
	switch (c) {
	    case 'x':
	     debug_level = atoi(optarg);
	     xflag++;
	     break;
	    case 'P':
	     pfile = optarg;
	     break;
	    case 't':
	     table_field_name = optarg;
	     tflag++;
	     break;
	    case 'r':
	     rrange = optarg;
	     rflag++;
	     break;
	 }

    if (rflag) {
	lrange_switch( rrange, &start, &last, (int) 1);
	spsassert( start>0 && last >= start, "Invalid range options.");
    }

    if (debug_level>1)
	Fprintf(stderr, "%s: reading parameters.\n", PROG);
    (void) read_params(pfile, SC_NOCOMMON, (char *)NULL);
    if ( !xflag && symtype("debug_level") != ST_UNDEF )
	debug_level = getsym_i("debug_level");
    if ( !tflag && symtype("table_field") != ST_UNDEF)
	table_field_name = getsym_s("table_field");
    if ( !rflag ) {
	if ( symtype("start") != ST_UNDEF ) 
	    start = getsym_i("start");
	if ( symtype("nan") != ST_UNDEF ) {
	    nan = getsym_i("nan");
	    last = start + nan - 1;
	}
	spsassert( nan > 0 && start > 0, "Invalid record range in parameter file.");
    }

    if (debug_level>1)
	Fprintf(stderr, "%s: reading input header.\n", PROG);
    if (optind < argc) 
	incbk_fname = eopen(PROG, argv[optind++], "r", FT_FEA, FEA_VQ, &incbk_hd, &incbk_fp);
    else {
	Fprintf(stderr, "%s: No codebook specified - exiting.\n", PROG);
	SYNTAX;
    }

    if (debug_level>1)
	Fprintf(stderr, "%s: opening output file.\n", PROG);
    if (optind < argc)
	outcbk_fname = eopen(PROG, argv[optind++], "w", FT_FEA, FEA_VQ, &outcbk_hd, &outcbk_fp);
    else {
	Fprintf(stderr, "%s: No output file specified - exiting.\n", PROG);
	SYNTAX;
    }

    cw_dim = *get_genhd_l("codeword_dimen", incbk_hd);    
    spsassert(cw_dim, "Codeword dimension 0.");
    design_size = *get_genhd_l("design_size", incbk_hd);    
    spsassert(design_size, "Codebook design size 0.");
    if (incbk_hd->common.ndrec != -1 && last == LONG_MAX)
	last = incbk_hd->common.ndrec;

    if (debug_level) {
	Fprintf(stderr, "%s: FEA_VQ input file %s, FEA output file %s.\n", 
		PROG, incbk_fname, outcbk_fname);
	Fprintf(stderr, "%s: table field name %s; codebook design size %d.\n", 
		PROG, table_field_name, design_size);
	if ( last < LONG_MAX )
	    Fprintf(stderr, "%s: Processing records %d to %d\n", PROG, start, last);
	else
	    Fprintf(stderr, "%s: Processing records %d to end of file\n", PROG, start);
    }

    if (debug_level > 1)
	Fprintf(stderr, "%s: allocating cbk header.\n", PROG);
    spsassert ((incbk_rec = allo_vqfea_rec(incbk_hd)) != NULL, "Can't allocate VQ record");

    if (debug_level >1)
	Fprintf(stderr, "%s: making output file header.\n", PROG);
    outcbk_hd = copy_header(incbk_hd);
    spsassert(outcbk_hd != NULL, "Can't allocate output header.");
    add_source_file(outcbk_hd, incbk_fname, incbk_hd);
    outcbk_hd->common.tag = NO;
    (void) strcpy(outcbk_hd->common.prog, PROG);
    (void) strcpy(outcbk_hd->common.vers, VERSION);
    (void) strcpy(outcbk_hd->common.progdate, DATE);
    outcbk_hd->variable.refer = incbk_fname;
    add_comment(outcbk_hd, get_cmd_line(argc, argv));
    *add_genhd_l("design_size", (long *)NULL, 1, outcbk_hd) = design_size;
    dimen[0] = dimen[1] = design_size;
    spsassert( add_fea_fld( table_field_name, (long)design_size*design_size, (short)2, (long *)dimen,
			   DOUBLE, (char **)NULL, outcbk_hd) == 0,
	      "can't add field for distance table.");
    if (debug_level >1)
	Fprintf(stderr, "%s: writing header.\n", PROG);
    write_header( outcbk_hd, outcbk_fp);
    if (debug_level >1)
	Fprintf(stderr, "%s: allocating record.\n", PROG);
    outcbk_rec = allo_vqfea_rec(outcbk_hd);
    spsassert(outcbk_rec != NULL, "Can't allocate output record.");
    distances = (double *) get_fea_ptr( outcbk_rec->fea_rec, table_field_name, outcbk_hd); 
    spsassert( distances != NULL, "can't get distance field from output record.");

    if (debug_level > 1)
	Fprintf(stderr, "%s: reading codebooks.\n", PROG);

    /* skip records if necessary */
    if (start > 1)
	for (ptr1=1; ptr1<start; ptr1++)
	    spsassert( get_vqfea_rec(incbk_rec, incbk_hd, incbk_fp) != EOF, 
		      "Not enough records in input.");

    incbk_recno = start;
    while ( incbk_recno <= last ) {
	/* get codebook */
	if ( get_vqfea_rec(incbk_rec, incbk_hd, incbk_fp) == EOF ) {
	    if ( last != LONG_MAX && incbk_recno < last )
		Fprintf(stderr, "%s: premature end of file.\n");
	    break;
	}
	
	incbk_size = *incbk_rec->current_size;
	if (debug_level>1) 
	    Fprintf(stderr, "%s: %d word codebook read from record %d of %s.\n",
		    PROG, incbk_size, incbk_recno, incbk_fname);

	/* copy input record to output record */

	copy_fea_rec(incbk_rec->fea_rec, incbk_hd,
		     outcbk_rec->fea_rec, outcbk_hd,
		     (char **) NULL, (short **) NULL);
	

	if (debug_level>1)
	    Fprintf(stderr, "%s: Computing distances for record %d.\n", PROG, incbk_recno);
	for (ptr1=0; ptr1<incbk_size; ptr1++)
	    for (ptr2=0; ptr2<=ptr1; ptr2++) {
		if (ptr1 != ptr2)
		    dist = dist_l2( incbk_rec->codebook[ptr1], incbk_rec->codebook[ptr2], cw_dim);
		else 
		    dist = 0.0;
		distances[ ptr1 * incbk_size + ptr2] = 
		    distances[ ptr2 * incbk_size + ptr1 ] = dist;
		if (debug_level>2)
		    Fprintf(stderr, "D(cw[%d], cw[%d]) = D(cw[%d], cw[%d]) = %e\n", 
			    ptr1, ptr2, ptr2, ptr1, dist);
	}
	if (debug_level>1)
	    Fprintf(stderr, "%s: Writing distances for record %d.\n",
		    PROG, incbk_recno);

	(void) put_vqfea_rec( outcbk_rec, outcbk_hd, outcbk_fp);

	incbk_recno++;
    }   
    if (debug_level>1)
	Fprintf(stderr, "%s: Done.\n", PROG);
    (void) fclose(incbk_fp);
    (void) fclose(outcbk_fp);
}

double
dist_l2( v1, v2, dim)
float *v1, *v2;
int dim;
{
    double res=0.0, ires;
    int ptr;

    double sqrt();

    for (ptr=0; ptr<dim; ptr++) {
	ires = v1[ptr] - v2[ptr];
	res += ires * ires;
    }
    res = sqrt(res);

    return(res);
}
