/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1990 Entropic Speech, Inc.  All rights reserved."   |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: plot3d							|
|  Module: p3ddata.c							|
|									|
|  Data arrays.								|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)p3ddata.c	1.10 09 Apr 1997 ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include "plot3d.h"

extern int		debug_level;

extern char		*arr_alloc();
extern void		free_fea_rec();

extern void		do_axes();
extern void		set_xname(), set_yname();

#define DATA_CHUNK 100

char			*read_data();
static float		**read_and_count();
static float		**reallo_data();
void			make_x_rnum(),	make_x_rtime(),	make_x_tag(),
			make_x_ttime(),	make_x_other();
void			make_y_inum(),	make_y_freq(),	make_y_other();
void			xyz_vals();

static long		startitem;	/* starting item number */
static long		nitems;		/* number of items to plot */
static long		startrec;	/* starting record number */
static long		nrecs;		/* number of records to plot */
static float		**data = NULL;	/* data array */
static double		*x = NULL;	/* x coords of records */
static double		*y = NULL;	/* y coords of items */
static long		*tags;
static struct header	*hdr;		/* data file header */
extern int              data_loaded;
extern int	        enable_waves_mode,	
		        send_to_waves;


char *
read_data(file, hd, fld, sitem, eitem, srec, erec)
    FILE	    *file;
    struct header   *hd;
    char	    *fld;
    long	    sitem, eitem;
    long	    *srec, *erec;
{
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    long	    i, j;
    double	    datamin, datamax;
    char	    *errmsg;


    if (data) {
	arr_free((char *)data, 2, FLOAT, 0);
	data = (float **)NULL;
    }
    if (tags)
	free((char *) tags);
    hdr = hd;
    startitem = sitem;
    nitems = eitem - sitem + 1;
    nrecs = *erec - *srec + 1;
    data = read_and_count(file, hd, fld, sitem, nitems, srec, &nrecs, &tags);
    if (!data)
	return "Can't read data.";
    startrec = *srec;
    *erec = nrecs + *srec - 1;
    if (nrecs < 1)
	return "No records in range.";

    if (debug_level)
        Fprintf(stderr, "%ld %s\n%s: %ld\n",
                nrecs, "from read_and_count",
                "initial record", *srec);

    x = (double *) arr_alloc(1, &nrecs, DOUBLE, 0);
    if (!x)
	return "Can't allocate memory.";
    make_x_rnum();

    y = (double *) arr_alloc(1, &nitems, DOUBLE, 0);
    if (!y)
	return "Can't allocate memory.";
    make_y_inum();

    datamin = FLT_MAX;
    datamax = -FLT_MAX;
    for (i = 0; i < nrecs; i++)
        for (j = 0; j < nitems; j++)
        {
            if (data[i][j] < datamin) datamin = data[i][j];
            if (data[i][j] > datamax) datamax = data[i][j];
        }
    set_z_axis_lims(datamin, datamax);

    if (debug_level)
        Fprintf(stderr, "Extreme data values: %g, %g.\n", datamin, datamax);

    if(send_to_waves) {
	disable_waves_cursors();
        enable_waves_mode = 1;
    }

    if(enable_waves_mode)
	enable_waves_cursors();

    return NULL;
}


static void
free_feaspec_rec(rec)
    struct feaspec  *rec;
{
    free_fea_rec(rec->fea_rec);
/*!*//* Finish. */
    free((char *) rec);
}


int
freq_ok_for_y()
{
    if ( !data_loaded )
	return 0;
    else
	return hdr->hd.fea->fea_type == FEA_SPEC
	    && *get_genhd_s("freq_format", hdr) != SPFMT_ARB_VAR;
}


void
make_y_inum()
{
    long    j;
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    do_axes();
    for (j = 0; j < nitems; j++)
	y[j] = startitem + j;
    set_y_axis_lims(y[0], y[nitems-1]);
    set_yname("item");
    do_axes();
}


void
make_y_freq()
{
    struct feaspec  *rec;
    float	    *frqs;
    long	    j;		    
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    if (!freq_ok_for_y()) return;
    do_axes();
    rec = allo_feaspec_rec(hdr, FLOAT);
    frqs = rec->frqs;
    for (j = 0; j < nitems; j++)
	y[j] = frqs[startitem + j];
    free_feaspec_rec(rec);
    set_y_axis_lims(y[0], y[nitems-1]);
    set_yname("frequency");
    do_axes();
}


void
make_y_other()
{
    do_axes();
/*!*/
    do_axes();
}


int
tag_ok_for_x()
{
    return hdr->common.tag && tags;
}


int
time_ok_for_x()
{
    int	    from_rec=0, from_tag=0;

    if ( data_loaded ) {

	from_rec = get_genhd_val("record_freq", hdr, 0.0) != 0.0;
	from_tag = hdr->common.tag
	    && (get_genhd_val("src_sf", hdr, 0.0) != 0.0
		|| (hdr->hd.fea->fea_type == FEA_SPEC
		    && get_genhd_val("sf", hdr, 0.0) != 0.0));

    }

    return (from_rec && from_tag) ? XTIME_FROM_BOTH
	   : from_rec ? XTIME_FROM_REC
	   : from_tag ? XTIME_FROM_TAG
	   : NONE;
}


void
make_x_rnum()
{
    long    i;
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    do_axes();
    for (i = 0; i < nrecs; i++)
	x[i] = startrec + i;
    set_x_axis_lims(x[0], x[nrecs-1]);
    set_xname("record");
    do_axes();
}


void
make_x_rtime()
{
    long    i;
    double  rf=1.0, st=0.0;
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    do_axes();
    if ( data_loaded ) {
	rf = get_genhd_val("record_freq", hdr, 1.0);
	st = get_genhd_val("start_time", hdr, 0.0);
    }
    for (i = 0; i < nrecs; i++)
	x[i] = st + (startrec + i - 1)/rf;
    set_x_axis_lims(x[0], x[nrecs-1]);
    set_xname("time");
    do_axes();
}


void
make_x_tag()
{
    long    i;
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    do_axes();
    for (i = 0; i < nrecs; i++)
	x[i] = tags[i];
    set_x_axis_lims(x[0], x[nrecs-1]);
    set_xname("tag");
    do_axes();
}


void
make_x_ttime()
{
    long    i;
    double  sf=1.0;
    extern void	    set_x_axis_lims(), set_y_axis_lims(), set_z_axis_lims();

    do_axes();
    if ( data_loaded ) {
	sf = get_genhd_val("src_sf", hdr, 0.0);
	if (sf == 0.0)
	    sf = get_genhd_val("sf", hdr, 1.0);
	for (i = 0; i < nrecs; i++)
	    x[i] = (tags[i] - 1)/sf;
    }
    set_x_axis_lims(x[0], x[nrecs-1]);
    set_xname("time");
    do_axes();
}


void
make_x_other()
{
    do_axes();
/*!*/
    set_xname("other");
    do_axes();
}


static float **
read_and_count(file, hd, name, startitem, nitems, startrec, nrecs, tags)
    FILE	    *file;
    struct header   *hd;
    char	    *name;
    long	    startitem, nitems, *startrec, *nrecs;
    long	    **tags;
{
    struct fea_data *rec;
    char	    *field;
    int		    fea_type;
    long	    dim[2];
    float	    **data;
    long	    data_size;
    long    	    i, j;

    if (debug_level > 1)
	Fprintf(stderr, "%s\n%s: %ld, %ld\n",
		"Beginning of read_and_count",
		"initial record and number of records", *startrec, *nrecs);

    fea_type = get_fea_type(name, hd);
    rec = allo_fea_rec(hd);
    field = get_fea_ptr(rec, name, hd);

    dim[0] = DATA_CHUNK;
    dim[1] = nitems;
    data_size = DATA_CHUNK;
    data = (float **) arr_alloc(2, dim, FLOAT, 0);
    if (!data)
    {
	Fprintf(stderr, "Can't allocate data array.\n");
	return NULL;
    }

    if (hd->common.tag)
    {
	*tags = (long *) arr_alloc(1, &data_size, LONG, 0);
	if (!*tags)
	{
	    Fprintf(stderr, "Can't allocate tags array.\n");
	    return NULL;
	}
    }
    else
	*tags = NULL;

    fea_skiprec(file, *startrec - 1, hd);

    for (i = 0;
	 i < *nrecs && get_fea_rec(rec, hd, file) != EOF;
	 i++)
    {
	if (i == data_size)
	{
	    data = reallo_data(data, &data_size, nitems, tags);
	    if (!data)
	    {
		Fprintf(stderr, "Can't reallocate data array.\n");
		return NULL;
	    }
	    if (hd->common.tag && !*tags)
	    {
		Fprintf(stderr, "Can't reallocate tags array.\n");
		return NULL;
	    }
	}
	
#define	CASE(type) \
	    for (j = 0; j < nitems; j++) \
		data[i][j] = ((type *) field)[startitem + j]; \
	    if (*tags) \
		(*tags)[i] = rec->tag; \
	    break
	switch (fea_type)
	{
	case BYTE:	CASE(char);
	case SHORT:	CASE(short);
	case LONG:	CASE(long);
	case FLOAT:	CASE(float);
	case DOUBLE:	CASE(double);
	default:
            Fprintf(stderr, "Not a real numeric field.\n");
            return NULL;
	}	
#undef	CASE
    }

    *nrecs = i;

    free_fea_rec(rec);
    return data;
}


static float **
reallo_data(data, size, nitems, tags)
    float   **data;
    long    *size;
    long    nitems;
    long    **tags;
{
    float   **new_chunk;
    long    i, old_size;
    long    dim[2];

    dim[0] = DATA_CHUNK;
    dim[1] = nitems;
    old_size = *size;
    *size += DATA_CHUNK;

    if (*tags)
	*tags = (long *) realloc((char *) *tags,
				 (unsigned) *size * sizeof(long));

    data = (float **) realloc((char *) data,
			      (unsigned) *size * sizeof(float *));
    if (!data)
	return NULL;

    new_chunk = (float **) arr_alloc(2, dim, FLOAT, 0);
    if (!new_chunk)
	return NULL;

    for (i = 0; i < dim[0]; i++)
	data[old_size + i] = new_chunk[i];

    free((char *) new_chunk);

    return data;
}

void
draw_plot()
{
    extern void	plot();

    plot(data, x, y, (int) nrecs, (int) nitems);
}

/*!*//*?*/
void
draw_axes()
{
    extern void	axes();

    axes();
}

void
draw_testplot()
{
    extern void	testplot();

    testplot(data, (int) nrecs, (int) nitems);
}


void
draw_pointplot()
{
    extern void	pointplot();

    pointplot(data, x, y, (int) nrecs, (int) nitems);
}


int
interp_srch(v, a, m, n)
    double  v, *a;
    long    m, n;
{
    if (v - a[m] <= 0) return m;
    if (v - a[n] >= 0) return n;
    while (n - m > 1)
    {
	int	i;
	double	d;

	i = m + (int)(0.5 + (v - a[m])/(a[n] - a[m])*(n - m));
	if (i <= m)
	    i = m + 1;
	else if (i >= n)
	    i = n - 1;

	d = v - a[i];
	if (d < 0)
	    n = i;
	else if (d > 0)
	    m = i;
	else
	    return i;
    }

    return (v - a[m] < a[n] - v) ? m : n;
}


void
xyz_vals(xx, yy, xval, yval, zval)
    double  xx, yy, *xval, *yval, *zval;
{
    int	    i, j;

    i = interp_srch(xx, x, 0L, nrecs - 1);
    j = interp_srch(yy, y, 0L, nitems - 1);
    *xval = x[i];
    *yval = y[j];
    *zval = data[i][j];
}


