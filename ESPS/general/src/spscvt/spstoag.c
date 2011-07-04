/* spstoa - Converts an sps data file to an ASCII file
 *	    suitable for mailing or movement across different machine types.
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Author: Original program by Joe Buck,
 *	   Modified for ESPS by Alan Parker.
 *	   Added support for ESPS FEA files by Ajaipal S. Virdy.
 */

#ifdef SCCS
static char *sccs_id = "@(#)spstoag.c	3.12 4/28/98 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>
#include <esps/filt.h>
#include "tagdef.h"

/* Local function called */

char   *cvt_tt ();
void print_rec ();
void dump_gens ();

/*
 * G L O B A L
 *  V A R I A B L E S
 */

char   *ProgName = "spstoa";
char   *in_file = "<stdin>";
int     Argc;
char  **Argv;
char   *get_cmd_line ();

main (argc, argv)
char  **argv;
{
    FILE * istrm = stdin;
    FILE * ostrm = stdout;
    struct header  *h;

    char   *eopen ();
    struct fea_data *fea_rec;
    struct fea_data *allo_fea_rec ();

    Argc = argc;
    Argv = argv;
    if (argc != 3) {
	Fprintf (stderr,
		"Usage: %s infile outfile\n", ProgName);
	exit (1);
    }

    in_file = eopen (ProgName, argv[1], "r", NONE, NONE, &h, &istrm);
    switch (h -> common.type) {
	case FT_FEA: 
	    if (is_file_complex(h)) {
		Fprintf(stderr,
		 "%s: Cannot deal with a file with complex fields, yet.\n",
		 ProgName);
	 	exit(1);
	    }
	case FT_SD: 
	case FT_FILT:
	    break;
	default: 
	    Fprintf (stderr,
		    "%s: Input file %s must either be an ESPS SD, FILT  or an ESPS FEA file.\n",
		    ProgName, in_file);
	    exit (1);
	    break;
    }

    (void) eopen
	(ProgName, argv[2], "w", NONE, NONE, (struct header **) NULL, &ostrm);

    dumphead (ostrm, h, 1);

    (void) fflush (ostrm);

    switch (h -> common.type) {
	case FT_SD: 
	case FT_FILT: 
	    while (dump_sdrec (istrm, ostrm, h));
	    break;
	case FT_FEA: 
	    fea_rec = allo_fea_rec (h);
	    while (get_fea_rec (fea_rec, h, istrm) != EOF) {
		Fprintf (ostrm, "\n");
		print_rec (fea_rec, h, ostrm);
	    }
	    break;
	default: 
	    Fprintf (stderr,
		    "%s: Unknown file type: %d\n", ProgName, h -> common.type);
	    exit (1);
	    break;
    }
    return 0;
}

dumphead (o, h, first)
register    FILE * o;
register struct header *h;
int     first;
{
    register int    i;

    /* Headers start with { and end with }, and may be nested */
    /* Reference Headers start with {{ and end with }, and may be nested 
    */

    if (h -> common.type != FT_SD && h -> common.type != FT_FEA
        && h -> common.type != FT_FILT) {
	Fprintf (stderr, "spstoa: unsupported header type dropped.\n");
	return;
    }

    Fprintf (o, "{\n");
/*
 * common part of header
 */

    Fprintf (o, "%02x %x\n", TAG_type, h -> common.type);
    Fprintf (o, "%02x %x\n", TAG_check, h -> common.check);
    if (h -> common.date[0])
	Fprintf (o, "%02x %.26s\n", TAG_date, h -> common.date);
    if (h -> common.hdvers[0])
	Fprintf (o, "%02x %.8s\n", TAG_hdvers, h -> common.hdvers);
    if (h -> common.prog[0])
	Fprintf (o, "%02x %.16s\n", TAG_prog, h -> common.prog);
    if (h -> common.vers[0])
	Fprintf (o, "%02x %.8s\n", TAG_vers, h -> common.vers);
    if (h -> common.progdate[0])
	Fprintf (o, "%02x %.26s\n", TAG_progdate, h -> common.progdate);
    if (h -> common.user[0])
	Fprintf (o, "%02x %s\n", TAG_user, h -> common.user);
    if (h -> common.ndrec)
	Fprintf (o, "%02x %x\n", TAG_ndrec, h -> common.ndrec);
    if (h -> common.tag)
	Fprintf (o, "%02x %x\n", TAG_tag, h -> common.tag);
    if (h -> common.ndouble)
	Fprintf (o, "%02x %x\n", TAG_ndouble, h -> common.ndouble);
    if (h -> common.nfloat)
	Fprintf (o, "%02x %x\n", TAG_nfloat, h -> common.nfloat);
    if (h -> common.nlong)
	Fprintf (o, "%02x %x\n", TAG_nlong, h -> common.nlong);
    if (h -> common.nshort)
	Fprintf (o, "%02x %x\n", TAG_nshort, h -> common.nshort);
    if (h -> common.nchar)
	Fprintf (o, "%02x %x\n", TAG_nchar, h -> common.nchar);

    for (i = 0; i < NSPARES && h -> common.spares[i]; i++)
	Fprintf (o, "%02x %x\n%x\n", TAG_spare, i, h -> common.spares[i]);

/*
 * variable part of header
 */

    for (i = 0; i < MAX_SOURCES && h -> variable.source[i]; i++)
	Fprintf (o, "%02x %s\n", TAG_source, h -> variable.source[i]);
    if (h -> variable.typtxt)
	Fprintf (o, "%02x %s\n", TAG_typtxt, cvt_tt (h -> variable.typtxt));
    if (first == 1) {
	(void) add_comment (h, get_cmd_line (Argc, Argv));
    }
    if (h -> variable.comment)
	Fprintf (o, "%02x %s\n", TAG_comment, cvt_tt (h -> variable.comment));
    if (h -> variable.refer)
	Fprintf (o, "%02x %s\n", TAG_refer, h -> variable.refer);

/* 
 * type specific part of header
 */

    switch (h -> common.type) {
	case FT_SD: 
	    Fprintf (o, "%02x %x\n", TAG_equip, h -> hd.sd -> equip);
	    Fprintf (o, "%02x %g\n", TAG_max_value, h -> hd.sd -> max_value);
	    Fprintf (o, "%02x %.7g\n", TAG_sf, h -> hd.sd -> sf);
	    Fprintf (o, "%02x %.7g\n", TAG_src_sf, h -> hd.sd -> src_sf);
	    Fprintf (o, "%02x %x\n", TAG_synt_method, h -> hd.sd -> synt_method);
	    Fprintf (o, "%02x %.7g\n", TAG_scale, h -> hd.sd -> scale);
	    Fprintf (o, "%02x %.7g\n", TAG_dcrem, h -> hd.sd -> dcrem);
	    Fprintf (o, "%02x %x\n", TAG_q_method, h -> hd.sd -> q_method);
	    Fprintf (o, "%02x %x\n", TAG_v_excit_method, h -> hd.sd -> v_excit_method);
	    Fprintf (o, "%02x %x\n", TAG_uv_excit_method,
		    h -> hd.sd -> uv_excit_method);
	    if (h -> hd.sd -> spare1)
		Fprintf (o, "%02x %x\n", TAG_spare1, h -> hd.sd -> spare1);
	    Fprintf (o, "%02x %x\n", TAG_nchan, h -> hd.sd -> nchan);
	    Fprintf (o, "%02x %x\n", TAG_synt_interp, h -> hd.sd -> synt_interp);
	    Fprintf (o, "%02x %x\n", TAG_synt_pwr, h -> hd.sd -> synt_pwr);
	    Fprintf (o, "%02x %x\n", TAG_synt_rc, h -> hd.sd -> synt_rc);
	    Fprintf (o, "%02x %x\n", TAG_synt_order, h -> hd.sd -> synt_order);
	    for (i = 0; i < SD_SPARES && h -> hd.sd -> spares[i]; i++)
		Fprintf (o, "%02x %x\n%x\n", TAG_spare, i, h -> hd.sd -> spares[i]);
	    if (h -> hd.sd -> prefilter)
		dump_z (o, TAG_prefilter, h -> hd.sd -> prefilter);
	    if (h -> hd.sd -> de_emp)
		dump_z (o, TAG_de_emp, h -> hd.sd -> de_emp);
	    dump_gens(h,o);
	    break;

	case FT_FILT: 
	    Fprintf (o, "%02x %x\n", TAG_max_num, h -> hd.filt -> max_num);
	    Fprintf (o, "%02x %x\n", TAG_max_den, h -> hd.filt -> max_den);
	    Fprintf (o, "%02x %x\n", TAG_func_spec, h -> hd.filt -> func_spec);
	    Fprintf (o, "%02x %x\n", TAG_nbands, h -> hd.filt -> nbands);
	    Fprintf (o, "%02x %x\n", TAG_npoints, h -> hd.filt -> npoints);
	    Fprintf (o, "%02x %x\n", TAG_g_size, h -> hd.filt -> g_size);
	    Fprintf (o, "%02x %x\n", TAG_nbits, h -> hd.filt -> nbits);
	    Fprintf (o, "%02x %x\n", TAG_filttype, h -> hd.filt -> type);
	    Fprintf (o, "%02x %x\n", TAG_filtmethod, h -> hd.filt -> method);
	    Fprintf (o, "%02x %x\n", TAG_filtmethod, h -> hd.filt -> method);
	    for (i = 0; i < FILT_SPARES && h -> hd.filt -> spares[i]; i++)
		Fprintf (o, "%02x %x\n%x\n", TAG_spare, i, h -> hd.filt -> spares[i]);
	    if (h -> hd.filt -> func_spec == BAND) {
		Fprintf (o, "%02x ", TAG_bandedges);
		for (i = 0; i < h -> hd.filt -> nbands * 2; i++)
		    Fprintf (o, "%g ", h -> hd.filt -> bandedges[i]);
		Fprintf(o,"\n");
	    }
	    if (h -> hd.filt -> func_spec == POINT) {
		Fprintf (o, "%02x ", TAG_points);
		for (i = 0; i < h -> hd.filt -> npoints; i++)
		    Fprintf (o, "%g ", h -> hd.filt -> points[i]);
		Fprintf(o,"\n");
	    }
	    if (h -> hd.filt -> func_spec == BAND) {
		Fprintf (o, "%02x ", TAG_gains);
		for (i = 0; i < h -> hd.filt -> nbands; i++)
		    Fprintf (o, "%g ", h -> hd.filt -> gains[i]);
		Fprintf(o,"\n");
	    }
	    if (h -> hd.filt -> func_spec == BAND) {
		Fprintf (o, "%02x ", TAG_wts);
		for (i = 0; i < h -> hd.filt -> nbands; i++)
		    Fprintf (o, "%g ", h -> hd.filt -> wts[i]);
		Fprintf(o,"\n");
	    }
	    if (h -> hd.filt -> func_spec == POINT) {
		Fprintf (o, "%02x ", TAG_wts);
		for (i = 0; i < h -> hd.filt -> npoints; i++)
		    Fprintf (o, "%g ", h -> hd.filt -> wts[i]);
		Fprintf(o,"\n");
	    }
	    dump_gens(h,o);
	    break;

	case FT_FEA: 
	    Fprintf (o, "%02x\n", TAG_Fea);
	    dump_fea_head (o, h);
	    if (h -> variable.refhd != NULL) {
		Fprintf (o, "{");
		dumphead (o, h -> variable.refhd, 0);
	    }
	    break;
    }				/* end switch */

    for (i = 0; i < MAX_SOURCES && h -> variable.srchead[i]; i++)
	dumphead (o, h -> variable.srchead[i], 0);

    Fprintf (o, "}\n");
}

dump_sdrec (in, out, h)
FILE * in, *out;
struct header  *h;
{
    register int    i;
    long    pos;

    int edr = h->common.edr;
    int machine = h->common.machine_code;

    if (h -> common.tag) {
	if (!miio_get_long(&pos, 1, edr, machine, in))
	    return 0;
	Fprintf (out, "%lx \n", pos);
    }
    if (h -> common.ndouble) {
	double *dbuf = malloc_d ((unsigned)h -> common.ndouble);
	spsassert (dbuf, "malloc_d failed");
	if (!miio_get_double(dbuf, (int)h->common.ndouble, edr, machine, in))
	    return 0;
	for (i = 0; i < h -> common.ndouble; i++)
	    Fprintf (out, "%.16g \n", dbuf[i]);
    }
    if (h -> common.nfloat) {
	float  *fbuf = malloc_f ((unsigned)h -> common.nfloat);
	spsassert (fbuf, "malloc_f failed");
	if (!miio_get_float(fbuf, (int)h->common.nfloat, edr, machine, in))
	    return 0;
	for (i = 0; i < h -> common.nfloat; i++)
	    Fprintf (out, "%.8g \n", fbuf[i]);
    }
    if (h -> common.nlong) {
	long   *lbuf = malloc_l ((unsigned)h -> common.nlong);
	spsassert (lbuf, "malloc_l failed");
	if (!miio_get_long(lbuf, (int)h->common.nlong, edr, machine, in))
	    return 0;
	for (i = 0; i < h -> common.nlong; i++)
	    Fprintf (out, "%ld \n", lbuf[i]);
    }
    if (h -> common.nshort) {
	short  *sbuf = malloc_s ((unsigned)h -> common.nshort);
	spsassert (sbuf, "malloc_s failed");
	if (!miio_get_short(sbuf, (int)h->common.nshort, edr, machine, in))
	    return 0;
	for (i = 0; i < h -> common.nshort; i++)
	    Fprintf (out, "%d \n", sbuf[i]);
    }
    if (h -> common.nchar) {
	char   *cbuf = malloc ((unsigned)h -> common.nchar);
	spsassert (cbuf, "malloc failed");
	if (!miio_get_char(cbuf, (int)h->common.nchar, edr, machine, in))
	    return 0;
	for (i = 0; i < h -> common.nchar; i++)
	    Fprintf (out, "%x \n", cbuf[i]);
    }
    return 1;
}

dump_z (o, tag, z)
FILE * o;
int     tag;
struct zfunc   *z;
{
    register int    i;
    Fprintf (o, "%02x %d %d ", tag, z -> nsiz, z -> dsiz);
    for (i = 0; i < z -> nsiz; i++)
	Fprintf (o, "%.7g ", z -> zeros[i]);
    for (i = 0; i < z -> dsiz; i++)
	Fprintf (o, "%.7g ", z -> poles[i]);
    (void) fputc ('\n', o);
}

char   *
        cvt_tt (s)
char   *s;
{
    register char  *p = s;
    while (*p) {
	if (*p == '\n')
	    *p = '\r';
	p++;
    }
    return s;
}


void
dump_gens (h,o) 
struct header *h;
FILE *o;
{

    int     ngen;
    int     j,
            i;
    char  **gen_names;
    double *dptr;
    float  *fptr;
    short  *sptr;
    long   *lptr;
    gen_names = genhd_list (&ngen, h);
    if (ngen > 0) {
	for (i = 0; i < ngen; i++) {
	    int     size;
	    short   type = genhd_type (gen_names[i], &size, h);
	    Fprintf (o, "%02x %s %d %d\n", TAG_gen, gen_names[i],
		    type, size);
	    if (type == CODED) {
		char  **s;
		s = genhd_codes (gen_names[i], h);
		while (*s != NULL)
		    Fprintf (o, "%02x %s\n", TAG_coded, *s++);
	    }
	    Fprintf (o, "%02x ", TAG_gen_value);
	    if ((type == CHAR))
		Fprintf (o, "%s\n", get_genhd (gen_names[i], h));
	    else
		for (j = 0; j < size; j++) {
		    switch (type) {
			case DOUBLE: 
			    dptr = (double *) get_genhd (gen_names[i], h);
			    Fprintf (o, "%lg ", dptr[j]);
			    break;
			case FLOAT: 
			    fptr = (float *) get_genhd (gen_names[i], h);
			    Fprintf (o, "%g ", fptr[j]);
			    break;
			case SHORT: 
			case CODED: 
			    sptr = (short *) get_genhd (gen_names[i], h);
			    Fprintf (o, "%d ", sptr[j]);
			    break;
			case LONG: 
			    lptr = (long *) get_genhd (gen_names[i], h);
			    Fprintf (o, "%ld ", lptr[j]);
			    break;
		    }
		}
	    Fprintf (o, "\n");
	}

    }
}
