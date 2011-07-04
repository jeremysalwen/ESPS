/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker, Entropic Speech, Inc.
 * Checked by:
 * Revised by:
 *
 * FEA file support routines
 */

static char *sccs_id = "@(#)feasupport.c	1.33	5/1/98	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <ctype.h>
#include <esps/fea.h>
#if !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <math.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

char   *savestring ();
int     lin_search2 (), idx_ok ();
char   *get_sphere_hdr();

static char *myrealloc();  /* my version of realloc */
static void fatal();
static int legal_name();
static void decode_data();

/* add a new field to a feature file
*/

int
add_fea_fld (name, size, rank, dimen, type, coded, hd)
char   *name;
long    size,
       *dimen;
short   rank;
short   type;
char  **coded;
struct header  *hd;

{

    struct fea_header  *fea;
    long    prod;
    int     i, k;

/* check the arguments 
*/
    spsassert(name != NULL, "add_fea_fld: name is NULL");
    spsassert(size > 0, "add_fea_fld: size is zero or less");
    spsassert(rank >= 0, "add_fea_fld: rank is negative");
    if (rank == 0)
	spsassert(size == 1, "add_fea_fld: size given for scalar is not 1");
    if (rank >= 2)
    {
	spsassert(dimen != NULL, "add_fea_fld: rank >= 2, but dimen is NULL");
	prod = 1;
	for (i = 0; i < rank; i++)
	    prod *= dimen[i];
	spsassert(size == prod,
		  "add_fea_fld: dimensions inconsistent with size");
    }
    spsassert(hd != NULL, "add_fea_fld: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "add_fea_fld: file not FEA");
    if (!legal_name(name)) {
	Fprintf(stderr,"add_fea_fld: %s not a valid field name.\n",name);
	Fprintf(stderr,"add_fea_fld: exiting program.\n");
	exit (1);
    }
	
    fea = hd -> hd.fea;
    if (fea->names && lin_search2 (fea -> names, name) != -1) {
	Fprintf (stderr, "add_fea_fld: name %s already defined\n", name);
	return (-1);
    }
/* increment the counter and reallocate the arrays
*/
    fea -> field_count++;
    fea -> names = (char **) myrealloc ((char *) fea -> names,
	    sizeof (char *) * (fea -> field_count + 1));
    spsassert (fea -> names, "add_fea_fld: realloc failed");

    fea -> names[fea -> field_count] = NULL;
    fea -> sizes = (long *) myrealloc ((char *) fea -> sizes,
	    fea -> field_count * sizeof (long));
    spsassert (fea -> sizes, "add_fea_fld: realloc failed");

    fea -> starts = (long *) myrealloc ((char *) fea -> starts,
	    fea -> field_count * sizeof (long));
    spsassert (fea -> starts, "add_fea_fld: realloc failed");

    fea -> ranks = (short *) myrealloc ((char *) fea -> ranks,
	    fea -> field_count * sizeof (short));
    spsassert (fea -> ranks, "add_fea_fld: realloc failed");

    fea -> dimens = (long **) myrealloc ((char *) fea -> dimens,
	    fea -> field_count * sizeof (long *));
    spsassert (fea -> dimens, "add_fea_fld: realloc failed");

    fea -> types = (short *) myrealloc ((char *) fea -> types,
	    fea -> field_count * sizeof (short));
    spsassert (fea -> types, "add_fea_fld: realloc failed");

    fea -> enums = (char ***) myrealloc ((char *) fea -> enums,
	    sizeof (char **) * fea -> field_count);
    spsassert (fea -> enums, "add_fea_fld: realloc failed");

    fea -> srcfields = (char ***) myrealloc ((char *) fea -> srcfields,
	    sizeof (char **) * fea -> field_count);
    fea -> srcfields[fea -> field_count-1] = NULL;
    spsassert (fea -> srcfields, "add_fea_fld: realloc failed");

    fea -> derived = (short *) myrealloc ((char *) fea -> derived,
	    fea -> field_count * sizeof (short));
    fea -> derived[fea -> field_count-1] = NULL;
    spsassert (fea -> derived, "add_fea_fld: realloc failed");
/* save all the new values
*/
    k = fea -> field_count - 1;
    fea -> names[k] = savestring (name);
    spsassert (fea -> names[k], "add_fea_fld: savestring failed");
    fea -> sizes[k] = size;
    fea -> ranks[k] = rank;
    fea -> types[k] = type;
    fea -> derived[k] = 0;

    if (type == CODED)
	fea -> enums[k] = coded;

    if (rank == 0)
    {
	fea->dimens[k] = NULL;
    }
    else if (rank == 1)
    {
	fea->dimens[k] = (long *) malloc(sizeof(long));
	fea->dimens[k][0] = size;
    }
    else
	fea->dimens[k] = dimen;

/* fill in the starts array.  This information is redundant, but
   worth computing here
*/
    switch (fea -> types[k]) {
	case DOUBLE: 
	    hd -> common.ndouble += fea -> sizes[k];
	    fea -> starts[k] = fea -> ndouble;
	    fea -> ndouble += fea -> sizes[k];
	    break;
	case FLOAT: 
	    hd -> common.nfloat += fea -> sizes[k];
	    fea -> starts[k] = fea -> nfloat;
	    fea -> nfloat += fea -> sizes[k];
	    break;
	case LONG: 
	    hd -> common.nlong += fea -> sizes[k];
	    fea -> starts[k] = fea -> nlong;
	    fea -> nlong += fea -> sizes[k];
	    break;
	case SHORT: 
	case CODED: 
	    hd -> common.nshort += fea -> sizes[k];
	    fea -> starts[k] = fea -> nshort;
	    fea -> nshort += fea -> sizes[k];
	    break;
	case CHAR: 
	case BYTE:
	    hd -> common.nchar += fea -> sizes[k];
	    fea -> starts[k] = fea -> nbyte;
	    fea -> nbyte += fea -> sizes[k];
	    break;
	case DOUBLE_CPLX:
	    hd -> common.ndouble += fea->sizes[k]*2;	
	    fea -> starts[k] = fea -> ndcplx;	
	    fea -> ndcplx += fea -> sizes[k];
	    break;
	case FLOAT_CPLX:
	    hd -> common.nfloat += fea->sizes[k]*2;	
	    fea -> starts[k] = fea -> nfcplx;	
	    fea -> nfcplx += fea -> sizes[k];
	    break;
	case LONG_CPLX:
	    hd -> common.nlong += fea->sizes[k]*2;	
	    fea -> starts[k] = fea -> nlcplx;	
	    fea -> nlcplx += fea -> sizes[k];
	    break;
	case SHORT_CPLX:
	    hd -> common.nshort += fea->sizes[k]*2;	
	    fea -> starts[k] = fea -> nscplx;	
	    fea -> nscplx += fea -> sizes[k];
	    break;
	case BYTE_CPLX:
	    hd -> common.nchar += fea->sizes[k]*2;	
	    fea -> starts[k] = fea -> nbcplx;	
	    fea -> nbcplx += fea -> sizes[k];
	    break;
	default: 
	    Fprintf (stderr, "add_fea_fld: bad type found for %s\n", 
		fea -> names[k]);
	    exit (1);
    }
    return 0;
}

/* allocate a feature file record;  this must be done after all
   add_fea_fld calls and before a write header  for a new file, and
   after a read header for an existing file.  In other words, the
   header must be complete.
*/

struct fea_data *
allo_fea_rec (hd)
struct header  *hd;
{

    struct fea_data *p = (struct fea_data  *) calloc (1, sizeof *p);
    struct fea_header *fea;

    spsassert (p, "allo_fea_rec: calloc failed");
    spsassert (hd,"allo_fae_rec: hd is NULL");
    spsassert (hd->common.type == FT_FEA, "allo_fea_rec: file not FEA");
    fea = hd->hd.fea;


/* allocate storage
*/
    if (fea->ndouble) {
    p->d_data = (double *)calloc((unsigned)fea->ndouble, sizeof (double));
    spsassert(p->d_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nfloat) {
    p->f_data = (float *)calloc((unsigned) fea->nfloat, sizeof (float));
    spsassert(p->f_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nlong) {
    p->l_data = (long *)calloc((unsigned) fea->nlong, sizeof (long));
    spsassert(p->l_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nshort) {
    p->s_data = (short *)calloc((unsigned) fea->nshort, sizeof (short));
    spsassert(p->s_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nbyte) {
    p->b_data = calloc((unsigned) fea->nbyte, sizeof (char));
    spsassert(p->b_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->ndcplx) {
    p->dc_data = (double_cplx *)calloc((unsigned)fea->ndcplx, 
	sizeof (double_cplx));
    spsassert(p->dc_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nfcplx) {
    p->fc_data = (float_cplx *)calloc((unsigned)fea->nfcplx, 
	sizeof (float_cplx));
    spsassert(p->fc_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nlcplx) {
    p->lc_data = (long_cplx *)calloc((unsigned)fea->nlcplx, 
	sizeof (long_cplx));
    spsassert(p->lc_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nscplx) {
    p->sc_data = (short_cplx *)calloc((unsigned)fea->nscplx, 
	sizeof (short_cplx));
    spsassert(p->sc_data, "allo_fea_rec: calloc failed!");
    }
    if (fea->nbcplx) {
    p->bc_data = (byte_cplx *)calloc((unsigned)fea->nbcplx, 
	sizeof (byte_cplx));
    spsassert(p->bc_data, "allo_fea_rec: calloc failed!");
    }
/* return
*/
    return p;
}

/* del_fea_fld deletes a feature file field (before a write header
   is done ofcourse)
*/

int
del_fea_fld(name, hd)
char 	*name;
struct header *hd;
{
     struct fea_header *fea = hd -> hd.fea;
     long  size;
     short type, type_i;
     int   i, k;
	
     spsassert(name, "del_fea_fld: name is NULL");
     spsassert(hd, "del_fea_fld: hd is NULL");

     if ((k = lin_search2(fea->names, name)) == -1)
          return -1;

     type = fea->types[k];
     size = fea->sizes[k];

     /* adjust sizes of data storage areas */

     switch (type) {
	case DOUBLE: 
	    hd->common.ndouble -= size;
	    fea->ndouble -= size;
	    break;
	case FLOAT: 
	    hd->common.nfloat -= size;
	    fea->nfloat -= size;
	    break;
	case LONG: 
	    hd->common.nlong -= size;
	    fea->nlong -= size;
	    break;
	case SHORT: 
	case CODED: 
	    /* normalize type for comparison with type_i in loop below */
	    type = SHORT;
	    hd->common.nshort -= size;
	    fea->nshort -= size;
	    break;
	case CHAR: 
	case BYTE:
	    /* normalize type for comparison with type_i in loop below */
	    type = BYTE;
	    hd->common.nchar -= size;
	    fea->nbyte -= size;
	    break;
	case DOUBLE_CPLX:
	    hd->common.ndouble -= size*2;	
	    fea->ndcplx -= size;
	    break;
	case FLOAT_CPLX:
	    hd->common.nfloat -= size*2;	
	    fea->nfcplx -= size;
	    break;
	case LONG_CPLX:
	    hd->common.nlong -= size*2;	
	    fea->nlcplx -= size;
	    break;
	case SHORT_CPLX:
	    hd->common.nshort -= size*2;	
	    fea->nscplx -= size;
	    break;
	case BYTE_CPLX:
	    hd->common.nchar -= size*2;	
	    fea->nbcplx -= size;
	    break;
	default: 
	    Fprintf(stderr, "del_fea_fld: bad type found for %s\n", name);
	    return -1;
     }

     for(i=k+1; i < fea->field_count; i++) {
	fea->names[i-1] = fea->names[i];
	fea->sizes[i-1] = fea->sizes[i];

	/* Cf. reassignment of type in switch above.
	 * This defines type_i so that (type_i == type) just when
	 * fea->types[i] and fea->types[k] share storage areas.
	 */

	switch (type_i = fea->types[i])
	{
	case CODED:
	    type_i = SHORT;
	    break;
	case CHAR:
	    type_i = BYTE;
	    break;
	default:
	    break;
	}
	fea->starts[i-1] =
	    (type_i == type) ? fea->starts[i] - size : fea->starts[i];

	fea->ranks[i-1] = fea->ranks[i];
	fea->dimens[i-1] = fea->dimens[i];
	fea->types[i-1] = fea->types[i];
	fea->enums[i-1] = fea->enums[i];
	fea->srcfields[i-1] = fea->srcfields[i];
	fea->derived[i-1] = fea->derived[i];
     }
     
     fea->field_count--;

     fea->names[fea->field_count] = NULL;

     return 0;
}

     


char   *
get_fea_ptr (rec, name, hd)
struct fea_data *rec;
char   *name;
struct header  *hd;
{


    struct fea_header  *fea = hd -> hd.fea;
    char   *ptr;
    int     k;

    spsassert (rec, "get_fea_ptr: rec is NULL");
    spsassert (hd, "get_fea_ptr: hd is NULL");
    spsassert (hd->common.type == FT_FEA, "get_fea_ptr: file not FEA");


    if (name == NULL || fea->names == NULL)
	return NULL;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return NULL;

    switch (fea -> types[k]) {
	case DOUBLE: 
	    ptr = (char *) & rec -> d_data[fea -> starts[k]];
	    break;
	case FLOAT: 
	    ptr = (char *) & rec -> f_data[fea -> starts[k]];
	    break;
	case LONG: 
	    ptr = (char *) & rec -> l_data[fea -> starts[k]];
	    break;
	case SHORT: 
	case CODED: 
	    ptr = (char *) & rec -> s_data[fea -> starts[k]];
	    break;
	case CHAR: 
	case BYTE:
	    ptr = &rec -> b_data[fea -> starts[k]];
	    break;
	case DOUBLE_CPLX:
	    ptr = (char *) & rec -> dc_data[fea -> starts[k]];
	    break;
	case FLOAT_CPLX:
	    ptr = (char *) & rec -> fc_data[fea -> starts[k]];
	    break;
	case LONG_CPLX:
	    ptr = (char *) & rec -> lc_data[fea -> starts[k]];
	    break;
	case SHORT_CPLX:
	    ptr = (char *) & rec -> sc_data[fea -> starts[k]];
	    break;
	case BYTE_CPLX:
	    ptr = (char *) & rec -> bc_data[fea -> starts[k]];
	    break;
	default:
	    spsassert(0,"failure in get_fea_ptr");
    }

    return ptr;
}



/* put_fea_rec write a feature file record to a file
*/

void
put_fea_rec (rec, hd, file)
struct fea_data *rec;
struct header  *hd;
FILE *file;
{
    struct fea_header *fea = hd -> hd.fea;

    spsassert(hd != NULL, "put_fea_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "put_fea_rec: file not FEA");
    spsassert(rec != NULL, "put_fea_rec: rec is NULL");
    spsassert(file != NULL, "put_fea_rec: file is NULL");

    if (hd -> common.tag)
	if (!miio_put_long(&rec->tag, 1, hd->common.edr, file))
  	    fatal("put_fea_rec: i/o error");

#ifndef NOPAD
    if (fea -> field_order) {
	char *ptr;
	int i;
	for (i=0; i < fea->field_count; i++) {
		switch (fea->types[i]) {
		 case DOUBLE:
			ptr = (char *)&rec->d_data[fea->starts[i]];
			if(!miio_put_double(ptr,(int)fea->sizes[i],
				hd->common.edr,file)) 
				fatal("put_fea_rec: i/o error");
			break;
		 case FLOAT:
			ptr = (char *)&rec->f_data[fea->starts[i]];
			if(!miio_put_float(ptr,(int)fea->sizes[i],
				hd->common.edr,file)) 
				fatal("put_fea_rec: i/o error");
			break;
		 case LONG:
			ptr = (char *)&rec->l_data[fea->starts[i]];
			if(!miio_put_long(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case CODED:
		 case SHORT:
			ptr = (char *)&rec->s_data[fea->starts[i]];
			if(!miio_put_short(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case CHAR:
		 case BYTE:
			ptr = &rec->b_data[fea->starts[i]];
			if(!miio_put_char(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case DOUBLE_CPLX:
			ptr = (char *)&rec->dc_data[fea->starts[i]];
			if(!miio_put_dcplx(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case FLOAT_CPLX:
			ptr = (char *)&rec->fc_data[fea->starts[i]];
			if(!miio_put_fcplx(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case LONG_CPLX:
			ptr = (char *)&rec->lc_data[fea->starts[i]];
			if(!miio_put_lcplx(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case SHORT_CPLX:
			ptr = (char *)&rec->sc_data[fea->starts[i]];
			if(!miio_put_scplx(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		 case BYTE_CPLX:
			ptr = (char *)&rec->bc_data[fea->starts[i]];
			if(!miio_put_bcplx(ptr,(int)fea->sizes[i],
				hd->common.edr,file))
				fatal("put_fea_rec: i/o error");
			break;
		  default:
			spsassert(0,"failure in put_fea_rec");

		}
	}
    }
    else 
#endif
    {

    	if (hd -> hd.fea->ndouble)
		if (!miio_put_double(rec -> d_data, (int)hd->hd.fea->ndouble,
		    hd->common.edr,file))
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->ndcplx)
		if (!miio_put_dcplx (rec -> dc_data, (int)hd->hd.fea->ndcplx, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nfloat)
		if (!miio_put_float (rec -> f_data, (int)hd->hd.fea->nfloat, 
		    hd->common.edr, file))
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nfcplx)
		if (!miio_put_fcplx (rec -> fc_data, (int)hd->hd.fea->nfcplx, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nlong)
		if (!miio_put_long (rec -> l_data, (int)hd->hd.fea->nlong, 
		    hd->common.edr, file))
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nlcplx)
		if (!miio_put_lcplx (rec -> lc_data, (int)hd->hd.fea->nlcplx, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nshort)
		if (!miio_put_short (rec -> s_data, (int)hd->hd.fea->nshort, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nscplx)
		if (!miio_put_scplx (rec -> sc_data, (int)hd->hd.fea->nscplx, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nbyte)
		if (!miio_put_char (rec -> b_data, (int)hd->hd.fea->nbyte, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    	if (hd -> hd.fea->nbcplx)
		if (!miio_put_bcplx (rec -> bc_data, (int)hd->hd.fea->nbcplx, 
		    hd->common.edr, file)) 
			fatal("put_fea_rec: i/o error");
    }
}


/* get_fea_rec reads a feature file record
*/

int
get_fea_rec (rec, hd, file)
struct fea_data *rec;
struct header  *hd;
FILE *file;

{
    struct fea_header *fea = hd -> hd.fea;
    char 	    *sphere_hdr_ptr;
    esignal_hd 	    *esignal_hdr_ptr;
    pc_wav_hd	    *pc_wav_hdr_ptr;
    int		     encoding;

    spsassert(hd != NULL, "get_fea_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "get_fea_rec: file not FEA");
    spsassert(rec != NULL, "get_fea_rec: rec is NULL");
    spsassert(file != NULL, "get_fea_rec: file is NULL");

    if ((esignal_hdr_ptr = get_esignal_hdr(hd)) != NULL)
	return esignal_get_rec(rec, esignal_hdr_ptr, hd, file);

    if ((pc_wav_hdr_ptr = get_pc_wav_hdr(hd)) != NULL)
	return pc_wav_get_rec(rec, pc_wav_hdr_ptr, hd, file);

    if (hd -> common.tag)
	if (!miio_get_long(&rec->tag, 1, hd->common.edr, hd->common.machine_code,file))
	    return EOF;

    sphere_hdr_ptr = get_sphere_hdr(hd);
    encoding = (int)get_genhd_val("encoding", hd, (double)0.0);

#ifndef NOPAD
    if (fea -> field_order) {
	char *ptr;
	int i;
	for (i=0; i < fea->field_count; i++) {
		switch (fea->types[i]) {
		 case DOUBLE:
			ptr = (char *)&rec->d_data[fea->starts[i]];
			if(!miio_get_double((double *)ptr,(int)fea->sizes[i],
			  hd->common.edr, hd->common.machine_code,file))
				return EOF;
			break;
		 case FLOAT:
			ptr = (char *)&rec->f_data[fea->starts[i]];
			if(!miio_get_float((float *)ptr,(int)fea->sizes[i],
			  hd->common.edr, hd->common.machine_code,file))
				return EOF;
			break;
		 case LONG:
			ptr = (char *)&rec->l_data[fea->starts[i]];
			if(!miio_get_long((long *)ptr,(int)fea->sizes[i],
			  hd->common.edr, hd->common.machine_code,file))
				return EOF;
			break;
		 case CODED:
		 case SHORT:
			ptr = (char *)&rec->s_data[fea->starts[i]];
			if(!miio_get_short((short *)ptr,(int)fea->sizes[i],
			  hd->common.edr, hd->common.machine_code,file))
				return EOF;
			break;
		 case CHAR:
		 case BYTE:
			ptr = (char *)&rec->b_data[fea->starts[i]];
			if(!miio_get_char(ptr,(int)fea->sizes[i],
			  hd->common.edr, hd->common.machine_code,file))
				return EOF;
			break;
		 case DOUBLE_CPLX:
			ptr = (char *)&rec->dc_data[fea->starts[i]];
			if(!miio_get_dcplx(ptr,(int)fea->sizes[i],
			  hd->common.edr,hd->common.machine_code,file))
				return EOF;
			break;
		 case FLOAT_CPLX:
			ptr = (char *)&rec->fc_data[fea->starts[i]];
			if(!miio_get_fcplx(ptr,(int)fea->sizes[i],
			  hd->common.edr,hd->common.machine_code,file))
				return EOF;
			break;
		 case LONG_CPLX:
			ptr = (char *)&rec->lc_data[fea->starts[i]];
			if(!miio_get_lcplx(ptr,(int)fea->sizes[i],
			  hd->common.edr,hd->common.machine_code,file))
				return EOF;
			break;
		 case SHORT_CPLX:
			ptr = (char *)&rec->sc_data[fea->starts[i]];
			if(!miio_get_scplx(ptr,(int)fea->sizes[i],
			  hd->common.edr,hd->common.machine_code,file))
				return EOF;
			break;
		 case BYTE_CPLX:
			ptr = (char *)&rec->bc_data[fea->starts[i]];
			if(!miio_get_bcplx(ptr,(int)fea->sizes[i],
			  hd->common.edr,hd->common.machine_code,file))
				return EOF;
			break;
		  default:
			spsassert(0,"failure in put_fea_rec");
		}
	}
    }
    else 
#endif
        {
    	if (hd -> hd.fea->ndouble)
		if (!miio_get_double(rec->d_data, (int)hd->hd.fea->ndouble, 
		    hd->common.edr, hd->common.machine_code,file))
	    	return EOF;
    	if (hd -> hd.fea->ndcplx)
		if (!miio_get_dcplx (rec -> dc_data, (int)hd->hd.fea->ndcplx, 
		    hd->common.edr, hd->common.machine_code,file)) 
	    	return EOF;
    	if (hd -> hd.fea->nfloat)
		if (!miio_get_float(rec->f_data, (int)hd->hd.fea->nfloat, 
		    hd->common.edr, hd->common.machine_code,file))
	    	return EOF;
    	if (hd -> hd.fea->nfcplx)
		if (!miio_get_fcplx (rec -> fc_data, (int)hd->hd.fea->nfcplx, 
		    hd->common.edr, hd->common.machine_code,file)) 
	    	return EOF;
    	if (hd -> hd.fea->nlong)
		if (!miio_get_long(rec->l_data, (int)hd->hd.fea->nlong, 
		    hd->common.edr, hd->common.machine_code,file))
	    	return EOF;
    	if (hd -> hd.fea->nlcplx)
		if (!miio_get_lcplx (rec -> lc_data, (int)hd->hd.fea->nlcplx, 
		    hd->common.edr, hd->common.machine_code,file)) 
	    	return EOF;
    	if (hd -> hd.fea->nshort) {
		if (sphere_hdr_ptr) {
		    if (!sp_mc_read_data(rec->s_data, 1L, sphere_hdr_ptr))
			return EOF;
		} else if (encoding){
		    if (!miio_get_char((char *)(rec->s_data), 
                        (int)hd->hd.fea->nshort, 
		        hd->common.edr, hd->common.machine_code,file))
	    	        	return EOF;
			decode_data(rec->s_data, hd->hd.fea->nshort, encoding);
                } else {
		    if (!miio_get_short(rec->s_data, (int)hd->hd.fea->nshort, 
		        hd->common.edr, hd->common.machine_code,file))
	    		return EOF;
		}
        }
    	if (hd -> hd.fea->nscplx)
		if (!miio_get_scplx (rec -> sc_data, (int)hd->hd.fea->nscplx, 
		    hd->common.edr, hd->common.machine_code,file)) 
	    	return EOF;
    	if (hd -> hd.fea->nbyte)
		if (!miio_get_char(rec->b_data, (int)hd->hd.fea->nbyte, 
		    hd->common.edr, hd->common.machine_code,file))
	    	return EOF;
    	if (hd -> hd.fea->nbcplx)
		if (!miio_get_bcplx (rec -> bc_data, (int)hd->hd.fea->nbcplx, 
		    hd->common.edr, hd->common.machine_code,file)) 
	    	return EOF;
   }
    return (1);
}

void
print_fea_recf (rec, hd, file, fields)
struct fea_data *rec;
struct header  *hd;
FILE *file;
char   *fields[];
{
    int     i,
            count = 0,
            i_addr = 0;
    long    psize,
           *l_ptr;
    short  *s_ptr;
    double *d_ptr;
    float  *f_ptr;
    char   *c_ptr;
    double_cplx	*dc_ptr;
    float_cplx	*fc_ptr;
    long_cplx	*lc_ptr;
    short_cplx	*sc_ptr;
    byte_cplx	*bc_ptr;

    struct fea_header  *fea = hd -> hd.fea;

    spsassert(rec != NULL, "print_fea_rec: rec is NULL");
    spsassert(hd != NULL, "print_fea_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "print_fea_rec: file not FEA");
    spsassert(file != NULL, "print_fea_rec: file is NULL");

    fprintf (file, "\n");
    if (hd -> common.tag && ((fields == NULL) ||
	(fields != NULL && (lin_search2 (fields, "tag") != -1))))
	fprintf (file, "Tag: %ld\n", rec -> tag);
    for (i = 0; i < fea -> field_count; i++) {
	if ((fields == NULL) ||
	  (fields != NULL && (lin_search2 (fields, fea -> names[i]) != -1))) {
	    fprintf (file, "%s:  ", fea -> names[i]);
	    if (fea -> types[i] == DOUBLE)
		d_ptr = (double *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == FLOAT)
		f_ptr = (float *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == LONG)
		l_ptr = (long *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == SHORT || fea -> types[i] == CODED)
		s_ptr = (short *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == CHAR || fea -> types[i] == BYTE)
		c_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == DOUBLE_CPLX)
		dc_ptr = (double_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == FLOAT_CPLX)
		fc_ptr = (float_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == LONG_CPLX)
		lc_ptr = (long_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == SHORT_CPLX)
		sc_ptr = (short_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == BYTE_CPLX)
		bc_ptr = (byte_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    count = 0;
	    psize = fea -> sizes[i];
	    if (psize == 1) {
		if (fea -> types[i] == DOUBLE)
		    fprintf (file, "%.8lg ", *d_ptr++);
		if (fea -> types[i] == FLOAT)
		    fprintf (file, "%.8g ", *f_ptr++);
		if (fea -> types[i] == LONG)
		    fprintf (file, "%ld ", *l_ptr++);
		if (fea -> types[i] == SHORT)
		    fprintf (file, "%d ", *s_ptr++);
		if (fea -> types[i] == BYTE)
		    fprintf (file, "%d ", (int)*c_ptr++);
		if (fea -> types[i] == CHAR)
		    fprintf (file, "%c ", *c_ptr);
		if (fea -> types[i] == DOUBLE_CPLX) {
		    fprintf (file, "[%lg, %lg] ",dc_ptr->real,dc_ptr->imag);
		    dc_ptr++;}
		if (fea -> types[i] == FLOAT_CPLX) {
		    fprintf (file, "[%g, %g] ",fc_ptr->real,fc_ptr->imag);
		    fc_ptr++;}
		if (fea -> types[i] == LONG_CPLX) {
		    fprintf (file, "[%ld, %ld] ",lc_ptr->real,lc_ptr->imag);
		    lc_ptr++;}
		if (fea -> types[i] == SHORT_CPLX) {
		    fprintf (file, "[%d, %d] ",sc_ptr->real,sc_ptr->imag);
		    sc_ptr++;}
		if (fea -> types[i] == BYTE_CPLX) {
		    fprintf (file, "[%d, %d] ",bc_ptr->real,bc_ptr->imag);
		    bc_ptr++;}
		if (fea -> types[i] == CODED) {
		    if (idx_ok (*s_ptr, fea -> enums[i]))
			fprintf (file, "%s ", fea -> enums[i][*s_ptr++]);
		    else
			fprintf (file, "bad code: %d ", *s_ptr++);
		}
	    }
	    else {
		fprintf (file, "\n  0: ");
		i_addr = 0;
		while (psize--) {
		    if (fea -> types[i] == DOUBLE)
			fprintf (file, "%13.8lg ", *d_ptr++);
		    if (fea -> types[i] == FLOAT)
			fprintf (file, "%13.8g ", *f_ptr++);
		    if (fea -> types[i] == LONG)
			fprintf (file, "%ld ", *l_ptr++);
		    if (fea -> types[i] == SHORT)
			fprintf (file, "%d ", *s_ptr++);
		    if (fea -> types[i] == BYTE)
			fprintf (file, "%d ", (int)*c_ptr++);
		    if (fea -> types[i] == CHAR) {
			fprintf (file, "%s ", c_ptr);
			psize = 0;
		    }
		    if (fea -> types[i] == DOUBLE_CPLX) {
			fprintf (file, "[%lg, %lg] ", 
			 dc_ptr->real,dc_ptr->imag);
			dc_ptr++;}
		    if (fea -> types[i] == FLOAT_CPLX) {
			fprintf (file, "[%g, %g] ", 
			 fc_ptr->real,fc_ptr->imag);
			fc_ptr++;}
		    if (fea -> types[i] == LONG_CPLX) {
			fprintf (file, "[%ld, %ld] ", 
			 lc_ptr->real,lc_ptr->imag);
			lc_ptr++;}
		    if (fea -> types[i] == SHORT_CPLX) {
			fprintf (file, "[%d, %d] ", 
			 sc_ptr->real,sc_ptr->imag);
			sc_ptr++;}
		    if (fea -> types[i] == BYTE_CPLX) {
			fprintf (file, "[%d, %d] ", 
			 bc_ptr->real,bc_ptr->imag);
			bc_ptr++;}
		    if (fea -> types[i] == CODED) {
			if (idx_ok (*s_ptr, fea -> enums[i]))
			    fprintf (file, "%s ", fea -> enums[i][*s_ptr++]);
			else
			    fprintf (file, "bad code: %d ", *s_ptr++);
			count++;
		    }
		    i_addr++;
		    if (fea -> types[i] == SHORT || fea -> types[i] == BYTE)
			count += 2;
		    else if (fea -> types[i] == CHAR)
			count++;
		    else
			count += 5;
		    if (psize && count > 20) {
			fprintf (file, "\n%3d: ", i_addr);
			count = 0;
		    }
		}
	    }
	    fprintf (file, "\n");
	}
    }
    fprintf (file, "\n");
}


void
print_fea_rec (rec, hd, file)
struct fea_data *rec;
struct header  *hd;
FILE * file;
{
    print_fea_recf (rec, hd, file, NULL);
}

/* fea_decode gets a string corresponding to a code for a coded field
*/

char   *
        fea_decode (code, name, hd)
short   code;
char   *name;
struct header  *hd;
{
    int     k;

    spsassert(hd != NULL, "fea_decode: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "fea_decode: file not FEA");

    if (name == NULL || code < 0 || hd->hd.fea->names == NULL)
	return NULL;
    if ((k = lin_search2 (hd -> hd.fea -> names, name)) == -1)
	return NULL;
    if (hd -> hd.fea -> types[k] != CODED)
	return NULL;
    if (idx_ok (code, hd -> hd.fea -> enums[k]))
	return hd -> hd.fea -> enums[k][code];
    else
	return NULL;
}


/* fea_encode gets the code corresponding to a string for a CODED field
*/

short
fea_encode (str, name, hd)
char   *str;
char   *name;
struct header  *hd;
{
    int     k;

    spsassert(hd != NULL, "fea_encode: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "fea_encode: file not FEA");

    if (name == NULL || str == NULL || hd->hd.fea->names == NULL)
	return - 1;
    if ((k = lin_search2 (hd -> hd.fea -> names, name)) == -1)
	return -2;
    if (hd -> hd.fea -> types[k] != CODED)
	return -2;
    return lin_search2 (hd -> hd.fea -> enums[k], str);
}

/* fea_encode_ci: case insensitive fea_encode */

short
fea_encode_ci (str, name, hd)
char   *str;
char   *name;
struct header  *hd;
{
    int     k;

    spsassert(hd != NULL, "fea_encode_ci: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "fea_encode_ci: file not FEA");

    if (name == NULL || str == NULL || hd->hd.fea->names == NULL)
	return - 1;
    if ((k = lin_search (hd -> hd.fea -> names, name)) == -1)
	return -2;
    if (hd -> hd.fea -> types[k] != CODED)
	return -2;
    return lin_search (hd -> hd.fea -> enums[k], str);
}

/* set_seg_lab flags a feature file as being segment-labeled and
   creates the required field 
*/

void
set_seg_lab (hd, files)
struct header  *hd;
char  **files;
{

    spsassert(hd != NULL, "set_seg_lab: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "set_seg_lab: file not FEA");

    if (hd -> common.tag)
	Fprintf (stderr,
		"set_seg_lab: warning: common.tag is also set\n");

    hd -> hd.fea -> segment_labeled = YES;

    if (add_fea_fld ("source_file", (long) 1, 0, NULL, CODED, files, hd)) 
	fatal("set_seg_lab: source_file already defined.");

    if (add_fea_fld ("segment_start", (long) 1, 0, NULL, LONG, NULL, hd))
	fatal("set_seg_lab: segment_start already defined.");

    if (add_fea_fld ("segment_length", (long) 1, 0, NULL, LONG, NULL, hd)) 
	fatal("set_seg_lab: segment_length already defined.");
}


long
        get_fea_siz (name, hd, rank, dimen)
char   *name;
struct header  *hd;
short  *rank;
long  **dimen;
{


    struct fea_header  *fea = hd -> hd.fea;
    int     k;

    spsassert(hd != NULL, "get_fea_siz: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "get_fea_siz: file not FEA");

    if (name == NULL || fea->names == NULL)
	return 0;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return 0;

    if (rank)
	*rank = fea -> ranks[k];
    if (dimen)
	*dimen = fea -> dimens[k];
    return fea -> sizes[k];
}

short
        get_fea_type (name, hd)
char   *name;
struct header  *hd;
{

    struct fea_header  *fea;
    int     k;

    spsassert(hd != NULL, "get_fea_type: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "get_fea_type: file not FEA");
    fea = hd -> hd.fea;

    if (name == NULL || fea->names == NULL)
	return UNDEF;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return UNDEF;
    return fea -> types[k];
}

int
        copy_fea_fld (hd1, hd2, name)
struct header  *hd1,
               *hd2;
char   *name;
{
    struct fea_header  *fea;
    int     k;
    assert (hd1);
    assert (hd2);

    spsassert(hd1 != NULL, "copy_fea_fld: hd1 is NULL");
    spsassert(hd1->common.type == FT_FEA, "copy_fea_fld: file1 not FEA");
    spsassert(hd2 != NULL, "copy_fea_fld: hd2 is NULL");
    spsassert(hd2->common.type == FT_FEA, "copy_fea_fld: file2 not FEA");
    fea = hd2 -> hd.fea;

    if (name == NULL || fea->names == NULL)
	return -1;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return -1;
    return(add_fea_fld(name, fea -> sizes[k], fea -> ranks[k], fea -> dimens[k],
		fea -> types[k], fea -> enums[k], hd1));
}


void
copy_fea_fields (dest, src)
struct header  *dest,
               *src;
{
    struct fea_header  *fea;
    int     k, size;
    
    spsassert(dest != NULL, "copy_fea_fields: dest is NULL");
    spsassert(dest->common.type == FT_FEA, "copy_fea_fields: dest not FEA");
    spsassert(src != NULL, "copy_fea_fields: src is NULL");
    spsassert(src->common.type == FT_FEA, "copy_fea_fields: src not FEA");
    fea = src->hd.fea;
    if (fea->segment_labeled)
	dest->hd.fea->segment_labeled = YES;
    size = fea->field_count;
    for (k = 0; k < size; k++) {
	spsassert(
	    add_fea_fld(fea->names[k], fea->sizes[k], fea->ranks[k], 
		fea->dimens[k],	fea->types[k], fea->enums[k], dest) == 0,
	    "copy_fea_fields: couldn't add field to dest header");
    }
}

int
set_fea_deriv (name, srcfield, hd)
char   *name;
char  **srcfield;
struct header  *hd;
{
    struct fea_header  *fea;
    int     k;

    spsassert(hd, "set_fea_deriv: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "set_fea_deriv: file not FEA");
    spsassert(srcfield, "set_fea_deriv: srcfield is NULL");
    fea = hd -> hd.fea;

    if (name == NULL || fea->names == NULL)
	return - 1;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return - 1;
    fea -> derived[k] = 1;
    fea -> srcfields[k] = srcfield;
    return 0;
}

char  **
get_fea_deriv (name, hd)
char   *name;
struct header  *hd;
{
    int     k;
    struct fea_header  *fea;

    spsassert (hd != NULL, "get_fea_deriv: hd is NULL");
    spsassert (hd->common.type == FT_FEA, "get_fea_deriv: file not FEA");
    fea = hd -> hd.fea;

    if (name == NULL || fea->names == NULL)
	return NULL;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return NULL;
    if (fea -> derived[k] == 0)
	return NULL;
    return fea -> srcfields[k];
}

char  **
get_fea_codes (name, hd)
char   *name;
struct header  *hd;
{
    int     k;
    struct fea_header  *fea;

    spsassert (hd != NULL, "get_fea_codes: hd is NULL");
    spsassert (hd->common.type == FT_FEA, "get_fea_codes: file not FEA");
    fea = hd -> hd.fea;

    if (name == NULL || fea->names == NULL)
	return NULL;
    if ((k = lin_search2 (fea -> names, name)) == -1)
	return NULL;
    if (fea -> types[k] != CODED)
	return NULL;
    return fea -> enums[k];
}


/* myrealloc is like realloc, except that if ptr is NULL, it acts like
   a malloc
*/

static char *
myrealloc(ptr, size)
char *ptr;
unsigned size;
{
	spsassert(size > 0, "myrealloc: size <= 0");
	if (ptr == NULL)
		return calloc((unsigned)1,size);
	return realloc(ptr, size);
}


/* this function returns 1 if the name is a legal field name */
static int
legal_name(name)
char *name;
{
	int i;
	if(*name != '_' && !isalpha(*name)) return 0;
	for(i=1; i<strlen(name); i++)
		if (!isalpha(name[i]) && !isdigit(name[i]) && 
		     name[i] != '_') return 0;
	return 1;
}

static void
fatal(s)
char *s;
{
	Fprintf(stderr,"%s\n",s);
	exit(1);
}

/* get_fea_elem_count returns the number of elements per field.  For
   non complex types, this is the same as the size of the field. For
   complex types it is the size times two.   Others might be added that
   are different
*/

long
get_fea_elem_count(name,hd)
char *name;
struct header *hd;
{
	int k;
	long size;
	spsassert(name,"get_fea_elem_count: name is null");
	spsassert(hd,"get_fea_elem_count: hd is null");

	if(strcmp("tag",name) == 0) return 1;

    	if ((k = lin_search2 (hd->hd.fea-> names, name)) == -1)
		return 0;

	size = hd->hd.fea->sizes[k];
	if (is_type_complex(hd->hd.fea->types[k])) 
		return 2*size;
	else
		return size;
}



long
get_fea_element(name,hd)
char *name;
struct header *hd;
{
	int k;
	long element;
	spsassert(name,"get_fea_element: name is null");
	spsassert(hd,"get_fea_element: hd is null");

	if(strcmp("tag",name) == 0) return 0;

    	if ((k = lin_search2 (hd->hd.fea-> names, name)) == -1)
		return -1;
	
#ifndef NOPAD
	if (hd->hd.fea->field_order == NO) 
#endif
        {

	   if(is_field_complex(hd,name))
           	element = 1 + hd->hd.fea->starts[k]*2; 
	   else
           	element = 1 + hd->hd.fea->starts[k]; 

           switch (hd->hd.fea->types[k])
           {			/* All cases fall through. */

	    case BYTE_CPLX:
		element += hd->hd.fea->nbyte;
	    case BYTE:
            case CHAR:
		element += hd->hd.fea->nscplx*2;
	    case SHORT_CPLX:
	   	element += hd->hd.fea->nshort;
   	    case SHORT:
	    case CODED:
		element += hd->hd.fea->nlcplx*2;
	    case LONG_CPLX:
		element += hd->hd.fea->nlong;
	    case LONG:
		element += hd->hd.fea->nfcplx*2;
	    case FLOAT_CPLX:
		element += hd->hd.fea->nfloat;
	    case FLOAT:
		element += hd->hd.fea->ndcplx*2;
	    case DOUBLE_CPLX:
		element += hd->hd.fea->ndouble;
	    case DOUBLE:
		{}
	    }
	}
#ifndef NOPAD
	else {
           int i;
 	   element = 1;
	   for (i=0; i < hd->hd.fea->field_count; i++) {
		if (strcmp(name, hd->hd.fea->names[i]) == 0) 
			return element;
		if (is_field_complex(hd,hd->hd.fea->names[i]))
			element += hd->hd.fea->sizes[i]*2;
		else
			element += hd->hd.fea->sizes[i];
	   }
	}
#endif
	
	return element;
}

/*
 * Function to free FEA record storage.
 * Used by library functions such as get_feasd_recs to avoid
 * accumulation of inaccessible temporary records.
 * USE WITH CAUTION--invalidates any pointers into the data arrays of
 * the FEA record.
 */
void
free_fea_rec(rec)
    struct fea_data *rec;
{
    if (rec)
    {
        if (rec->d_data) free((char *) rec->d_data);
        if (rec->f_data) free((char *) rec->f_data);
        if (rec->l_data) free((char *) rec->l_data);
        if (rec->s_data) free((char *) rec->s_data);
        if (rec->b_data) free((char *) rec->b_data);
	if (rec->dc_data) free((char *) rec->dc_data);
	if (rec->fc_data) free((char *) rec->fc_data);
	if (rec->lc_data) free((char *) rec->lc_data);
	if (rec->sc_data) free((char *) rec->sc_data);
	if (rec->bc_data) free((char *) rec->bc_data);
        free(rec);
    }
}

long *
fea_index_to_elem(elem_array, size, name, hd)
long *elem_array;
long *size;
char *name;
struct header *hd;
{
	long *new_array;
	long i,j;

	spsassert(elem_array,"fea_index_to_elem: elem_array is NULL");
	spsassert(name,"fea_index_to_elem: name is NULL");
	spsassert(hd,"fea_index_to_elem: hd is NULL");

	if(!is_field_complex(hd,name))
		return elem_array;

	new_array = malloc_l(*size*2);
	j=0;
	for (i=0; i< *size; i++) {
		new_array[j] = elem_array[i]*2;
		new_array[j+1] = new_array[j] + 1;
		j++;
		j++;
	}
	*size = *size*2;
	free(elem_array);
	return new_array;
}

/*
 * these routines are used to implement a varient of mu_law and bit reverse
 * for Ft. Meade
 */
/***********************************************************************/

static int
rev(i, bits)
   int             i;
   int             bits;
{
   int             rev_i = i;
   switch (bits) {
   case 8:
      rev_i = ((rev_i & 0xf0) >> 4) | ((rev_i & 0x0f) << 4);
   case 4:
      rev_i = ((rev_i & 0xcc) >> 2) | ((rev_i & 0x33) << 2);
   case 2:
      rev_i = ((rev_i & 0xaa) >> 1) | ((rev_i & 0x55) << 1);
      break;
   default:
      spsassert(0, "error in call to rev (bits)");
   }
   return rev_i;
}

static void
bit_reverse(buffer, num_read)
   char           *buffer;
   int             num_read;
{
   int             i;
   for (i = 0; i < num_read; i++) {
      buffer[i] = rev(buffer[i], 8);
   }
}

static void
mu2_to_linear(in, out, n)
   char           *in;
   short          *out;
   long            n;
{
   int             i;
   double          M, N, s, sign, two_pow_N, result;

   for (i = 0; i < n; i++) {
      M = (double) (rev((in[i] & 0xf0) >> 4, 4) ^ 0xf);
      N = (double) (rev((in[i] & 0x0e), 4) ^ 0x7);
      s = (double) ((in[i] & 0x01) ^ 0x1);

      sign = pow((double) -1.0, s);
      two_pow_N = pow((double) 2.0, N);
      result = sign * (((double) 16.5 + M) * two_pow_N - (double) 16.5);
      out[i] = result;
   }
}


static void
decode_data(data, len, encoding)
char *data;
long len;
int encoding;
{
	short *temp_data;
	temp_data = (short *)malloc(sizeof(short)*len);
	spsassert(temp_data,"malloc failure in decoding mulaw data!");
	
	switch (encoding) {
	 case 1:		/* normal esps mu-law encoded */
	    mu_to_linear(data, temp_data, len);
	    break;
	 case 2:		/* bit reverse esps mu-law encoded */
	    bit_reverse(data, len);
	    mu_to_linear(data, temp_data, len);
	    break;
	    /*
	     * the following two methods were provides by Terri Kamm at DoD.
	     * The reason that the no bit reverse case calls the reverse
	     * routine is that the reverse is already in the algorithm
	     */
	 case 3:		/* Terri Kamm mu_law, bit reverse */
	    mu2_to_linear(data, temp_data, len);
	    break;
	 case 4:		/* Terri Kamm Sun mu_law, no bit reverse */
	    bit_reverse(data, len);
	    mu2_to_linear(data, temp_data, len);
	    break;
	    /*
	     * these are the two A-law versions that we have
	     */
	 case 5:		/* standard alaw */
	    (void) a_to_linear(data, temp_data, len);
	    break;
	 case 6:		/* CCITT */
	    (void) a_to_linear_2(data, temp_data, len);
	    break;
	 }
        (void)memcpy(data, temp_data, (int)len*sizeof(short));
        free(temp_data);
        return;
}
