/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * sampled data file support routines
 */

static char *sccs_id = "@(#)sdsupport.c	1.25	12/18/96	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/sd.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif

short	get_fea_type();
long	get_fea_siz();
long	get_feasd_recs();
int	put_feasd_recs();
char	*type_convert();


/*
 * get_sd_type returns the type of the data in a sampled data file this
 * routine assumes that SD files consist of a one element record of uniform
 * type 
 */

int
get_sd_type(hd)
    struct header   *hd;
{
    spsassert(hd != NULL, "get_sd_type: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD,
	      "get_sd_type: file not SD or FEA_SD");

    switch (hd->common.type)
    {
    case FT_SD:
	if (hd->common.ndouble == 1)
		return (DOUBLE);
	if (hd->common.nfloat == 1)
		return (FLOAT);
	if (hd->common.nlong == 1)
		return (LONG);
	if (hd->common.nshort == 1)
		return (SHORT);
	if (hd->common.nchar == 1)
		return (CHAR);
	break;
    case FT_FEA:
	return get_fea_type("samples", hd);
	break;
    }
    return (NONE);		/* unknown */
}

/*
 * set_sd_type sets the type codes in a sampled data file. it makes the same
 * assumptions as get_sd_type 
 */

void
set_sd_type(hd, type)
struct header  *hd;
int             type;

{
	spsassert(hd != NULL, "set_sd_type: hd is NULL");
	spsassert(hd->common.type == FT_SD, "set_sd_type: file not SD");

	hd->common.ndouble = 0;
	hd->common.nfloat = 0;
	hd->common.nlong = 0;
	hd->common.nshort = 0;
	hd->common.nchar = 0;
	switch (type) {
	case DOUBLE:
		hd->common.ndouble = 1;
		return;
	case FLOAT:
		hd->common.nfloat = 1;
		return;
	case LONG:
		hd->common.nlong = 1;
		return;
	case SHORT:
		hd->common.nshort = 1;
		return;
	case CHAR:
	case BYTE:
		hd->common.nchar = 1;
		return;
	}
	Fprintf(stderr, "set_sd_type: called with unknown type %d, type\n");
	exit(1);
}

/*
 * return the sizeof the sampled data file record.  This assumes that the
 * record consists of one datum of one of the below types. 
 */

int
get_sd_recsize(hd)
struct header  *hd;
{
	spsassert(hd != NULL, "get_sd_recsize: hd is NULL");
	spsassert(hd->common.type == FT_SD, "get_sd_recsize: file not SD");

	if (hd->common.ndouble == 1)
		return (sizeof(double));
	if (hd->common.nfloat == 1)
		return (sizeof(float));
	if (hd->common.nlong == 1)
		return (sizeof(long));
	if (hd->common.nshort == 1)
		return (sizeof(short));
	if (hd->common.nchar == 1)
		return (sizeof(char));
	Fprintf(stderr, "get_sd_recsize: called with unknown type %d, type\n");
	exit(1);
	return (0);
}

/*
 * read records from a sampled data file.  The header is checked for the type
 * of the data in the file.  The data is returned in floating format.  There
 * is one of these for returning the data as floats, shorts, and doubles.  
 * Others are easily added. 
 */

int
get_sd_recf(fbuf, nsmpls, hd, stream)
    float		*fbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, k;
    int			type, edr_flag, machine;
    char    	    	*data;
    static struct feasd	rec = {FLOAT, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "get_sd_recf: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "get_sd_recf: file not SD or single_channel FEA_SD");
    spsassert(fbuf != NULL, "get_sd_recf: fbuf is NULL");
    spsassert(nsmpls >= 0, "get_sd_recf: nsmpls < 0");
    spsassert(stream != NULL, "get_sd_recf: stream is NULL");

    if (nsmpls == 0) return 0;

    switch (hd->common.type)
    {
    case FT_SD:
	type = get_sd_type(hd);
	edr_flag = hd->common.edr;
	machine = hd->common.machine_code;
	switch (type)
	{
	case CHAR:
	case BYTE:
	case SHORT:
	case LONG:
	case DOUBLE:
	    data = calloc((unsigned) nsmpls, (unsigned) typesiz(type));
	    spsassert(data, "calloc failed!");
	    n = miio_get(type, data, nsmpls, edr_flag, machine, stream);
	    (void) type_convert((long) nsmpls, data, type,
				(char *) fbuf, FLOAT, (void (*)()) NULL);
	    free(data);
	    break;
	case FLOAT:		/* No conversion is needed. */
	    for (k = 0; k < nsmpls; k++)
		fbuf[k] = 0;
	    n = miio_get_float(fbuf, nsmpls, edr_flag, machine, stream);
	    break;
	default:
	    Fprintf(stderr, "get_sd_recf: unknown type code '%d'\n", type);
	    exit(1);
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) fbuf;
	n = get_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream);
	break;
    }

    return (n);
}


int
get_sd_recs(sbuf, nsmpls, hd, stream)
    short		*sbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, k;
    int			type, edr_flag, machine;
    char    	    	*data;
    static struct feasd	rec = {SHORT, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "get_sd_recs: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "get_sd_recs: file not SD or single_channel FEA_SD");
    spsassert(sbuf != NULL, "get_sd_recs: sbuf is NULL");
    spsassert(nsmpls >= 0, "get_sd_recs: nsmpls < 0");
    spsassert(stream != NULL, "get_sd_recs: stream is NULL");

    if (nsmpls == 0) return 0;

    switch (hd->common.type)
    {
    case FT_SD:
	type = get_sd_type(hd);
	edr_flag = hd->common.edr;
	machine = hd->common.machine_code;
	switch (type) {
	case CHAR:
	case BYTE:
	case LONG:
	case DOUBLE:
	case FLOAT:
	    data = calloc((unsigned) nsmpls, (unsigned) typesiz(type));
	    spsassert(data, "calloc failed!");
	    n = miio_get(type, data, nsmpls, edr_flag, machine, stream);
	    (void) type_convert((long) nsmpls, data, type,
				(char *) sbuf, SHORT, (void (*)()) NULL);
	    free(data);
	    break;
	case SHORT:		/* No conversion is needed. */
	    for (k = 0; k < nsmpls; k++)
		sbuf[k] = 0;
	    n = miio_get_short(sbuf, nsmpls, edr_flag, machine, stream);
	    break;
	default:
	    Fprintf(stderr, "get_sd_recf: unknown type code '%d'\n", type);
	    exit(1);
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) sbuf;
	n = get_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream);
	break;
    }

    return (n);
}

int
get_sd_recd(dbuf, nsmpls, hd, stream)
    double		*dbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, k, edr_flag, machine;
    int			type;
    char    	    	*data;
    static struct feasd	rec = {DOUBLE, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "get_sd_recd: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "get_sd_recd: file not SD or single_channel FEA_SD");
    spsassert(dbuf != NULL, "get_sd_recd: dbuf is NULL");
    spsassert(nsmpls >= 0, "get_sd_recd: nsmpls < 0");
    spsassert(stream != NULL, "get_sd_recd: stream is NULL");

    if (nsmpls == 0) return 0;

    switch (hd->common.type)
    {
    case FT_SD:
	type = get_sd_type(hd);
	edr_flag = hd->common.edr;
	machine = hd->common.machine_code;
	switch (type) {
	case CHAR:
	case BYTE:
	case SHORT:
	case LONG:
	case FLOAT:
	    data = calloc((unsigned) nsmpls, (unsigned) typesiz(type));
	    spsassert(data, "calloc failed!");
	    n = miio_get(type, data, nsmpls, edr_flag, machine, stream);
	    (void) type_convert((long) nsmpls, data, type,
				(char *) dbuf, DOUBLE, (void (*)()) NULL);
	    free(data);
	    break;
	case DOUBLE:		/* No conversion is required. */
	    for (k = 0; k < nsmpls; k++)
		dbuf[k] = 0;
	    n = miio_get_double(dbuf, nsmpls, edr_flag, machine, stream);
	    break;
	default:
	    Fprintf(stderr, "get_sd_recd: unknown type code '%d'\n", type);
	    exit(1);
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) dbuf;
	n = get_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream);
	break;
    }

    return (n);
}

/*
 * writes records into a sampled data file.  The header is checked for the
 * type of the data in the file.  The data is taken in floating format. 
 */

void
put_sd_recf(fbuf, nsmpls, hd, stream)
    float		*fbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, type;
    int			edr_flag;
    char    	    	*data;
    static struct feasd	rec = {FLOAT, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "put_sd_recf: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "put_sd_recf: file not SD or single_channel FEA_SD");
    spsassert(fbuf != NULL, "put_sd_recf: fbuf is NULL");
    spsassert(nsmpls > 0, "put_sd_recf: nsmpls <= 0");
    spsassert(stream != NULL, "put_sd_recf: stream is NULL");

    switch (hd->common.type)
    {
    case FT_SD:
	if ((type = get_sd_type(hd)) == NONE) {
		Fprintf(stderr, "put_sd_recf: type not set yet.\n");
		exit(1);
	}
	if (nsmpls == 0)
		return;
	edr_flag = hd->common.edr;
	switch (type) {
	case CHAR:
	case BYTE:
	case SHORT:
	case LONG:
	case DOUBLE:
	    data = calloc((unsigned) nsmpls, (unsigned) typesiz(type));
	    spsassert(data, "calloc failed!");
	    (void) type_convert((long) nsmpls, (char *) fbuf, FLOAT,
				data, type, (void (*)()) NULL);
	    n = miio_put(type, data, nsmpls, edr_flag, stream);
	    if (n != nsmpls) goto bomb;
	    free(data);
	    break;
	case FLOAT:		/* No conversion is needed. */
	    n = miio_put_float(fbuf, nsmpls, edr_flag, stream);
	    if (n != nsmpls) goto bomb;
	    break;
	default:
	    Fprintf(stderr, "put_sd_recf: unknown type code '%d'\n", type);
	    exit(1);
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) fbuf;
	if (put_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream))
	    goto bomb;
	break;
    }

	return;
bomb:
	perror("put_sd_recf: write error");
	exit(1);
}

/*
 * writes records into a sampled data file.  The header is checked for the
 * type of the data in the file.  The data is taken in double format. 
 */

void
put_sd_recd(dbuf, nsmpls, hd, stream)
    double		*dbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, type;
    int			edr_flag;
    char    	    	*data;
    static struct feasd	rec = {DOUBLE, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "put_sd_recd: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "put_sd_recd: file not SD or single_channel FEA_SD");
    spsassert(dbuf != NULL, "put_sd_recd: dbuf is NULL");
    spsassert(nsmpls > 0, "put_sd_recd: nsmpls <= 0");
    spsassert(stream != NULL, "put_sd_recd: stream is NULL");

    switch (hd->common.type)
    {
    case FT_SD:
	if ((type = get_sd_type(hd)) == NONE) {
		Fprintf(stderr, "put_sd_recd: type not set yet.\n");
		exit(1);
	}
	if (nsmpls == 0)
		return;
	edr_flag = hd->common.edr;
	switch (type) {
	case CHAR:
	case BYTE:
	case SHORT:
	case LONG:
	case FLOAT:
	    data = calloc((unsigned) nsmpls, (unsigned) typesiz(type));
	    spsassert(data, "calloc failed!");
	    (void) type_convert((long) nsmpls, (char *) dbuf, DOUBLE,
				data, type, (void (*)()) NULL);
	    n = miio_put(type, data, nsmpls, edr_flag, stream);
	    if (n != nsmpls) goto bomb;
	    free(data);
	    break;
	case DOUBLE:		/* No conversion is needed. */
	    n = miio_put_double(dbuf, nsmpls, edr_flag, stream);
	    if (n != nsmpls) goto bomb;
	    break;
	default:
	    Fprintf(stderr, "put_sd_recd: unknown type code '%d'\n", type);
	    exit(1);
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) dbuf;
	if (put_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream))
	    goto bomb;
	break;
    }

	return;
bomb:
	perror("put_sd_recd: write error");
	exit(1);
}



/*
 * writes records into a sampled data file.  The header is checked for the
 * type of the data in the file.  The data is taken in short format. 
 */

void
put_sd_recs(sbuf, nsmpls, hd, stream)
    short		*sbuf;
    int			nsmpls;
    struct header	*hd;
    FILE		*stream;
{
    int			n, type;
    int			edr_flag;
    char    	    	data[BUFSIZ];
    static struct feasd	rec = {SHORT, 0, 1L, NULL, NULL};

    spsassert(hd != NULL, "put_sd_recs: hd is NULL");
    spsassert(hd->common.type == FT_SD
	      || (hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD
		  && 1 == get_fea_siz("samples", hd,
				      (short *) NULL, (long **) NULL)),
	      "put_sd_recs: file not SD or single_channel FEA_SD");
    spsassert(sbuf != NULL, "put_sd_recs: sbuf is NULL");
    spsassert(nsmpls > 0, "put_sd_recs: nsmpls <= 0");
    spsassert(stream != NULL, "put_sd_recs: stream is NULL");

    switch (hd->common.type)
    {
    case FT_SD:
	if ((type = get_sd_type(hd)) == NONE) {
		Fprintf(stderr, "put_sd_recs: type not set yet.\n");
		exit(1);
	}
	edr_flag = hd->common.edr;
	n = BUFSIZ / typesiz(type);
	/*
	 * Write the next block of n samples. If fewer than n remain to be
	 * written, write the smaller number 
	 */
	while (nsmpls > 0)
	{
	    if (nsmpls < n)
		n = nsmpls;
	    switch (type) {
	    case CHAR:
	    case BYTE:
	    case LONG:
	    case DOUBLE:
	    case FLOAT:
		(void) type_convert((long) n, (char *) sbuf, SHORT,
				    data, type, (void (*)()) NULL);
		if (miio_put(type, data, n, edr_flag, stream) != n)
		    goto bomb;
		break;
	    case SHORT:
		if (miio_put_short(sbuf, n, edr_flag, stream) != n)
		    goto bomb;
		break;
	    default:
		Fprintf(stderr, 
			"put_sd_recs: called with unknown type %d\n" ,type);
		exit(1);
	    }
	    sbuf += n;
	    nsmpls -= n;
	}
	break;
    case FT_FEA:
	rec.num_records = nsmpls;
	rec.data = (char *) sbuf;
	if (put_feasd_recs(&rec, 0L, (long) nsmpls, hd, stream))
	    goto bomb;
	break;
    }

    return;
bomb:
	perror("put_sd_recs: write error");
	exit(1);
}


int
get_sd_orecd(dbuf, framelen, nmove, first, hd, file)
double         *dbuf;
int             framelen;
int             nmove;
int             first;
struct header  *hd;
FILE           *file;
{
	int             i;
	int             num_read;

	spsassert(hd != NULL, "get_sd_orecd: hd is NULL");
	spsassert(hd->common.type == FT_SD
		 || (hd->common.type == FT_FEA
		     && hd->hd.fea->fea_type == FEA_SD
		     && 1 == get_fea_siz("samples", hd,
					 (short *) NULL, (long **) NULL)),
		 "get_sd_orecd: file not SD or single_channel FEA_SD");
	spsassert(dbuf != NULL, "get_sd_orecd: dbuf is NULL");
	spsassert(framelen >= 0, "get_sd_orecd: framelen < 0");
	spsassert(nmove >= 0, "get_sd_orecd: nmove < 0");
	spsassert(file != NULL, "get_sd_orecd: file is NULL");

	/*
	 * first case is with first == 0, just read framelen number of points 
	 */
	if (first == 1)
		return (get_sd_recd(dbuf, framelen, hd, file));

	if (nmove == 0)
		return (framelen);

	if (nmove >= framelen) {
		fea_skiprec(file, (long) (nmove - framelen), hd);
		return (get_sd_recd(dbuf, framelen, hd, file));
	}
	for (i = 0; i <= framelen - nmove; i++)
		dbuf[i] = dbuf[i + nmove];
	num_read = get_sd_recd(dbuf + (framelen - nmove), nmove, hd, file);
	return (num_read + framelen - nmove);

}

int
get_sd_orecf(fbuf, framelen, nmove, first, hd, file)
float          *fbuf;
int             framelen;
int             nmove;
int             first;
struct header  *hd;
FILE           *file;
{
	int             i;
	int             num_read;

	spsassert(hd != NULL, "get_sd_orecf: hd is NULL");
	spsassert(hd->common.type == FT_SD
		 || (hd->common.type == FT_FEA
		     && hd->hd.fea->fea_type == FEA_SD
		     && 1 == get_fea_siz("samples", hd,
					 (short *) NULL, (long **) NULL)),
		 "get_sd_orecf: file not SD or single_channel FEA_SD");
	spsassert(fbuf != NULL, "get_sd_orecf: fbuf is NULL");
	spsassert(framelen >= 0, "get_sd_orecf: framelen < 0");
	spsassert(nmove >= 0, "get_sd_orecf: nmove < 0");
	spsassert(file != NULL, "get_sd_orecf: file is NULL");

	/*
	 * first case is with first == 0, just read framelen number of points 
	 */
	if (first == 1)
		return (get_sd_recf(fbuf, framelen, hd, file));

	if (nmove == 0)
		return (framelen);

	if (nmove >= framelen) {
		fea_skiprec(file, (long) (nmove - framelen), hd);
		return (get_sd_recf(fbuf, framelen, hd, file));
	}
	for (i = 0; i <= framelen - nmove; i++)
		fbuf[i] = fbuf[i + nmove];
	num_read = get_sd_recf(fbuf + (framelen - nmove), nmove, hd, file);
	return (num_read + framelen - nmove);

}

int
get_sd_orecs(sbuf, framelen, nmove, first, hd, file)
short          *sbuf;
int             framelen;
int             nmove;
int             first;
struct header  *hd;
FILE           *file;
{
	int             i;
	int             num_read;

	spsassert(hd != NULL, "get_sd_orecs: hd is NULL");
	spsassert(hd->common.type == FT_SD
		 || (hd->common.type == FT_FEA
		     && hd->hd.fea->fea_type == FEA_SD
		     && 1 == get_fea_siz("samples", hd,
					 (short *) NULL, (long **) NULL)),
		 "get_sd_orecs: file not SD or single_channel FEA_SD");
	spsassert(sbuf != NULL, "get_sd_orecs: sbuf is NULL");
	spsassert(framelen >= 0, "get_sd_orecs: framelen < 0");
	spsassert(nmove >= 0, "get_sd_orecs: nmove < 0");
	spsassert(file != NULL, "get_sd_orecs: file is NULL");

	/*
	 * first case is with first == 0, just read framelen number of points 
	 */
	if (first == 1)
		return (get_sd_recs(sbuf, framelen, hd, file));

	if (nmove == 0)
		return (framelen);

	if (nmove >= framelen) {
		fea_skiprec(file, (long) (nmove - framelen), hd);
		return (get_sd_recs(sbuf, framelen, hd, file));
	}
	for (i = 0; i <= framelen - nmove; i++)
		sbuf[i] = sbuf[i + nmove];
	num_read = get_sd_recs(sbuf + (framelen - nmove), nmove, hd, file);
	return (num_read + framelen - nmove);

}
