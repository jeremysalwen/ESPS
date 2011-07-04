/* 
 * This material contains proprietary software of Entropic Speech, Inc.   
 * Any reproduction, distribution, or publication without the the prior	   
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice			
 *
 *             "Copyright 1986 Entropic Speech, Inc."
 *				
 * Written by:  Alan Parker
 * 			
 * Module:	filtsupport.c
 *	
 * FILT file support routines
 * Alan Parker, Entropic Speech, Inc.
*/


#ifdef SCCS
static char	*sccsid = "@(#)filtsupport.c	1.8 6/5/89 ESI";
#endif

#include <stdio.h>
#include <ctype.h>

#include <esps/esps.h>
#include <esps/filt.h>



/* UE check for FILT record routines
 */

static void
filt_ue_check(func, hd)
char	*func;
struct header *hd;
{
	if (hd->common.type != FT_FILT) {
		(void) fprintf(stderr, "%s: called with non FILT header\n", func);
		exit(1);
	}
	if (hd->hd.filt->max_num < 0 || hd->hd.filt->max_den < 0) {
		(void) fprintf(stderr, "%s: max_num or max_den < 0\n", func);
		exit(1);
	}
}


/* allocate memory for filt_data for use with the file described by
 * header hd
*/

struct filt_data *
allo_filt_rec(hd)
struct header *hd;
{
	struct filt_data *p;
	(void) filt_ue_check("allo_filt_rec", hd);
	p = (struct filt_data *)calloc(1, sizeof * p);
	if (hd->hd.filt->max_num != 0)
		p->filt_func.zeros = 
		    (float *)calloc(hd->hd.filt->max_num, sizeof (float));
	if (hd->hd.filt->max_den != 0)
		p->filt_func.poles = 
		    (float *)calloc(hd->hd.filt->max_den, sizeof (float));
	return(p);
}

/*
 * This is the NEW get_filt_rec(3-SPS) which reads the data in proper order.
 */

int
get_filt_rec(rec, hd, file)
struct filt_data *rec;
struct header	 *hd;
FILE		 *file;
{
	int edr, machine_code;

	(void) filt_ue_check("get_filt_rec", hd);
	spsassert(file != NULL, "get_filt_rec: file is NULL");
        spsassert(rec != NULL, "get_filt_rec: rec is NULL");

	edr = hd->common.edr;
	machine_code = hd->common.machine_code;

	if (!miio_get_long(&rec->tag, 1, edr, machine_code, file))
		return(EOF);

	if (hd->hd.filt->max_num > 0)
		if (!miio_get_float(rec->filt_func.zeros, 
		    hd->hd.filt->max_num, edr, machine_code, file))
			return(EOF);

	if (hd->hd.filt->max_den > 0)
		if (!miio_get_float(rec->filt_func.poles, 
		    hd->hd.filt->max_den, edr, machine_code, file))
			return(EOF);

	if (!miio_get_short(&rec->filt_func.nsiz, 1, edr, machine_code,file) || 
	    !miio_get_short(&rec->filt_func.dsiz, 1, edr, machine_code, file)) 
		return(EOF);

	if (!miio_get_short(rec->spares, FDSPARES, edr, machine_code, file)) 
		return(EOF);

	return(1);
}

/*
 * This is the NEW put_filt_rec(3-SPS) which writes the data in proper order.
 */

void
put_filt_rec(rec, hd, file)
struct filt_data *rec;
struct header *hd;
FILE *file;

{
	int edr;
	(void) filt_ue_check("put_filt_rec", hd);
	spsassert(file != NULL, "put_filt_rec: file is NULL");
        spsassert(rec != NULL, "put_filt_rec: rec is NULL");

	edr = hd->common.edr;

	if (!miio_put_long(&rec->tag, 1, edr, file)) {
		(void)fprintf(stderr, "put_filt_rec: write error\n");
		exit(1);
	}

	if (hd->hd.filt->max_num > 0) {
		if (!miio_put_float(rec->filt_func.zeros, hd->hd.filt->max_num,
		   edr, file)) {
			(void)fprintf(stderr, "put_filt_rec: write error\n");
			exit(1);
		}
	}

	if (hd->hd.filt->max_den > 0) {
		if (!miio_put_float(rec->filt_func.poles, hd->hd.filt->max_den,
		   edr, file)) {
			(void)fprintf(stderr, "put_filt_rec: write error\n");
			exit(1);
		}
	}

	if (!miio_put_short(&rec->filt_func.nsiz, 1, edr, file) || 
	    !miio_put_short(&rec->filt_func.dsiz, 1, edr, file)) {
		(void)fprintf(stderr, "put_filt_rec: write error\n");
		exit(1);
	}

	if (!miio_put_short(rec->spares, FDSPARES, edr, file)) {
		(void)fprintf(stderr, "put_filt_rec: write error\n");
		exit(1);
	}
}

/* print an filt record, useful for debugging and used by psds */
void
print_filt_rec(rec, ih, fp)
struct filt_data *rec;
struct header *ih;
FILE *fp;
{
	int	i, j
;
	char	tmpbuf[100];

	(void) filt_ue_check("print_filt_rec", ih);
        spsassert(rec != NULL, "print_filt_rec: rec is NULL");
        spsassert(fp != NULL, "print_filt_rec: fp is NULL");

	fprintf(fp, "tag = %ld\n", rec->tag);

	for (i = 0; i < FDSPARES; i++)
		if (rec->spares[i] != 0)
			(void) fprintf(fp, "spares[%d] = 0x%x\n", rec->spares[i]);

	(void) fprintf(fp, "filt_func: nsiz = %d, dsiz = %d\n{\n",
	    rec->filt_func.nsiz, rec->filt_func.dsiz);

	for (i = 0; i < rec->filt_func.nsiz; i++) {
		(void) sprintf (tmpbuf, "%8d:  %s%.4g\n",
		i, (rec->filt_func.zeros[i] >= 0) ? " " : "",
		rec->filt_func.zeros[i]);
		j = 0;
		while ( !isdigit (tmpbuf[j]) )
		   ++j;
		tmpbuf[j-1] = 'a';
		(void) fprintf (fp, "%s", tmpbuf);
	}

	(void) fprintf(fp, "} / {\n");

	if (rec->filt_func.dsiz == 0)
		(void) fprintf(fp, "      b0:   1.0\n");

	for (i = 0; i < rec->filt_func.dsiz; i++) {
		(void) sprintf (tmpbuf, "%8d:  %s%.4g\n",
		i, (rec->filt_func.poles[i] >= 0) ? " " : "",
		rec->filt_func.poles[i]);
		j = 0;
		while ( !isdigit (tmpbuf[j]) )
		   ++j;
		tmpbuf[j-1] = 'b';
		(void) fprintf (fp, "%s", tmpbuf);
	}

	(void) fprintf(fp, "}\n");
}


