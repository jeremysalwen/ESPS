/*
 * This material contains proprietary software of Entropic Speech, Inc.   Any
 * reproduction, distribution, or publication without the the prior	  
 * written permission of Entropic Speech, Inc. is strictly prohibited. Any
 * public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice			
 *
 * Copyright 1989 Entropic Speech, Inc. All Rights Reserved.
 *
 *
 * This module contains:
 *
 * is_file_complex(h) - returns YES (1) if file contains complex fields,
 * is_field_complex(h,name) - returns YES if field is complex type,
 * is_complex(type) - returns YES if type is a complex type.
 *
 * Written by: Alan Parker, ESI Washington, DC.
 *
*/

#ifndef lint
	static char *sccs_id = "@(#)is_complex.c	1.1 10/18/89 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>


int
is_file_complex(h)
struct header *h;
{
	spsassert(h, "h is NULL");
	spsassert(h->common.type == FT_FEA, "h is not a feature file");

	if (h->hd.fea->ndcplx || h->hd.fea->nfcplx ||
	        h->hd.fea->nlcplx || h->hd.fea->nscplx ||
	        h->hd.fea->nbcplx)	
	  return YES;
	else
	  return NO;
}

short get_fea_type();

int
is_field_complex(h,name)
struct header *h;
char *name;
{
	spsassert(h, "h is NULL");
	spsassert(h->common.type == FT_FEA, "h is not a feature file");
	spsassert(name, "name  is NULL");

	switch(get_fea_type(name,h)) {
		case DOUBLE_CPLX:
		case FLOAT_CPLX:
		case LONG_CPLX:
		case SHORT_CPLX:
		case BYTE_CPLX:
			return YES;
			break;
		default:
			return NO;
			break;
	}
/*NOTREACHED*/
}

int
is_type_complex(type)
int type;
{
	switch (type) {
		case BYTE_CPLX:
		case SHORT_CPLX:
		case LONG_CPLX:
		case FLOAT_CPLX:
		case DOUBLE_CPLX:
			return YES;
			break;
		default:
			return NO;
			break;
	}
/*NOTREACHED*/
}
