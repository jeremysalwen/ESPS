/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  copy_fea_rec.c							|
|  									|
|  Rodney Johnson							|
|  									|
|  Copy data from one feature record to another having the same fields	|
|  but formatted according to a different header.			|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)copy_fea_rec.c	1.7 3/29/90 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define COPY(type) { \
    type    *ip = (type *) ifea_ptr, \
	    *op = (type *) ofea_ptr; \
    for (j = 0; j < size; j++) op[j] = ip[j]; }


long	get_fea_siz();

void
copy_fea_rec(irec, ihd, orec, ohd, fields, trans)
    struct fea_data *irec;
    struct header   *ihd;
    struct fea_data *orec;
    struct header   *ohd;
    char    **fields;
    short   **trans;
{
    int     i;
    long    j;
    char    *ifea_ptr, *ofea_ptr;
    short   type;
    long    size;
    short   rk;
    long    *dim;

/* check input arguments
*/
    spsassert(irec != NULL, "copy_fea_rec: irec is NULL");
    spsassert(ihd != NULL, "copy_fea_rec: ihd is NULL");
    spsassert(ihd->common.type == FT_FEA, "copy_fea_rec: ihd not FEA file");
    spsassert(orec != NULL, "copy_fea_rec: orec is NULL");
    spsassert(ohd != NULL, "copy_fea_rec: ohd is NULL");
    spsassert(ohd->common.type == FT_FEA, "copy_fea_rec: ohd not FEA file");

    if (fields == NULL) fields = ihd->hd.fea->names;

    for (i = 0; fields[i] != NULL; i++) 
    {
	if((ifea_ptr = get_fea_ptr(irec, fields[i], ihd)) == NULL) continue;
	if((ofea_ptr = get_fea_ptr(orec, fields[i], ohd)) == NULL) continue;
	size = get_fea_siz(fields[i], ihd, &rk, &dim);
	type = get_fea_type(fields[i], ihd);

	switch(type)
	{
	case CHAR:
	case BYTE:
	    COPY(char)
	    break;
	case SHORT:
	    COPY(short)
	    break;
	case CODED:
	    if (trans == NULL)
		COPY(short)
	    else
	    {
		short	*ip = (short *) ifea_ptr,
			*op = (short *) ofea_ptr;
		for (j = 0; j < size; j++) op[j] = trans[i][ip[j]];
	    }
	    break;
	case LONG:
	    COPY(long)
	    break;
	case FLOAT:
	    COPY(float)
	    break;
	case DOUBLE:
	    COPY(double)
	    break;
	case DOUBLE_CPLX:
	    COPY(double_cplx)
	    break;
	case FLOAT_CPLX:
	    COPY(float_cplx)
	    break;
	case LONG_CPLX:
	    COPY(long_cplx)
	    break;
	case SHORT_CPLX:
	    COPY(short_cplx)
	    break;
	case BYTE_CPLX:
	    COPY(byte_cplx)
	    break;
	default:
	    Fprintf(stderr,
		"copy_fea_rec: unrecognized type code %d.\n", type);
	    exit(1);
	    break;
	}
    }

    if(ihd->common.tag == YES) 
	orec->tag = irec->tag;
	
}
