/* fea_sup: support modules for spstoa(1-ESPS) and atosps(1-ESPS).
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Author: Ajaipal S. Virdy, Entropic Speech, Inc.
 *
 * Modules:  dump_fea_head, cvt_feahdr, cvt_fearec
 *
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <assert.h>
#include <ctype.h>
#include <esps/unix.h>

#ifdef SCCS
    static char *sccs_id = "@(#)fea_sup.c	3.11 6/9/93 ESI";
#endif

#define VERSION "3.11"
#define DATE "6/9/93"
#define PROG "fea_sup.c"
#define BUFSIZE 1024


/*
 * E S P S
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

short	fea_encode ();
float	**f_mat_alloc ();
long    get_fea_siz ();
short   get_fea_type ();
char	*get_genhd ();
int	lin_search2 ();
char	*savestring ();

/*
 * L O C A L
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

void	clear_fea_rec ();
char	**fill_codes1();
short   type_str ();

/*
 * G L O B A L
 *  V A R I A B L E S
 */

char    buf[BUFSIZE];
int	debug_level = 0;


dump_fea_head (tstrm, fh)
register FILE	*tstrm;
register struct header	*fh;
{
    extern  errno;
    extern  char *get_cmd_line ();

    struct fea_header	*fea;

    int	    i,
	    j,
	    k;

    int	    ngen = 0;
    char    **gen_names;

    long   size;
    short  type;

    int     getopt ();
    short   type_str ();
    void    print_rec ();


    fea = fh -> hd.fea;

    Fprintf (tstrm,
    "%x %x %x\n",
    fh->common.tag, fea->fea_type, fea->segment_labeled);

    for (i = 0; i < fea -> field_count; i++) {
	Fprintf (tstrm, "%s %s ", fea -> names[i],
		type_codes[fea -> types[i]]);
	Fprintf (tstrm, "%ld %d ", fea -> sizes[i], fea -> ranks[i]);
	if (fea -> ranks[i] > 1)
	    for (j = 0; j < fea -> ranks[i]; j++)
		Fprintf (tstrm, "%ld ", fea -> dimens[i][j]);
	Fprintf (tstrm, "\n");
	if (fea -> types[i] == CODED) {
	    char  **s;
	    s = fea -> enums[i];
	    while (*s != NULL) {
		Fprintf (tstrm, " %s\n", *s++);
	    }
	}
	if (fea -> derived[i]) {
	    char  **s;
	    s = fea -> srcfields[i];
	    while (*s != NULL) {
		Fprintf (tstrm, "\t%s\n", *s++);
	    }
	}
    }

 gen_names = genhd_list (&ngen, fh);
 if (ngen > 0) {
    for (i = 0; i < ngen; i++) {
	char *cptr; double *dptr; float *fptr; long *lptr; short *sptr;
	type = genhd_type (gen_names[i], (int *) &size, fh);
	Fprintf (tstrm, "\n");
	Fprintf (tstrm, "@%s %s %ld ", gen_names[i], type_codes[type], size);
	k = 0;
	cptr = get_genhd(gen_names[i], fh);
	if (type == CODED) {
	    char    **s;
	    s = genhd_codes(gen_names[i], fh);
	    Fprintf (tstrm, "[ ");
	    while (*s != NULL)
		Fprintf (tstrm, "%s ", *s++);
	    Fprintf (tstrm, "] ");
	}
	if ((type == CHAR) && (size > 1)) {
	    Fprintf (tstrm, "%s ", cptr);
	}
	else
	    for (j = 0; j < size; j++) {
		k++;
		if (k > 7) {
		    Fprintf (tstrm, "\n");
		    k = 0;
		}
		switch(type) {
		    case DOUBLE:
		    dptr = (double *)get_genhd(gen_names[i],fh);
		    Fprintf (tstrm, "%lg ",dptr[j]);
		    break;
		case FLOAT:
		    fptr = (float *)get_genhd(gen_names[i],fh);
		    Fprintf (tstrm, "%g ",fptr[j]);
		    break;
		case SHORT:
		    sptr = (short *)get_genhd(gen_names[i],fh);
		    Fprintf (tstrm, "%d ",sptr[j]);
		    break;
		case LONG:
		    lptr = (long *)get_genhd(gen_names[i],fh);
		    Fprintf (tstrm, "%ld ",lptr[j]);
		    break;
		case CHAR:
		    cptr = get_genhd(gen_names[i],fh);
		    Fprintf (tstrm, "0x%x",cptr[j]);
		    break;
		case CODED:  {
		    char **codes = genhd_codes(gen_names[i],fh);
		    sptr = (short *)get_genhd(gen_names[i],fh);
		    if (idx_ok(sptr[j],codes))
			Fprintf (tstrm, "%s ",codes[sptr[j]]);
		    else
			Fprintf (tstrm, "invalid code %d ",sptr[j]);
		    break;
		}
		}
	    }
    }
    Fprintf (tstrm, "\n\n");
 } else
    Fprintf (tstrm, "\n");

}

cvt_feahdr (tstrm, oh)
register FILE	*tstrm;
register struct header	*oh;
{
    int	    flag = 1;

    char    *ProgName = "cvt_feahdr";

    int	    c;

    char    **code_ptr,
	    **cc_ptr,
	    **enums = NULL,
	    **srcfields = NULL;

    int	    type_tmp,
	    tag_tmp,
	    segment_tmp;

    double *dptr;
    float  *fptr;
    long   *lptr, *dimens = NULL, size;
    short  *sptr, rank, type;
    char   *bptr, *cptr, *dimen_chr, *name, *type_chr, *size_chr, *rank_chr;


	if (fgets (buf, BUFSIZE, tstrm) == NULL) {
	    Fprintf (stderr,
	    "%s: premature EOF while trying to convert fea hdr.\n",
	    ProgName);
	}

	(void) sscanf (buf, "%x %x %x", &tag_tmp, &type_tmp, &segment_tmp);

	oh -> common.tag = tag_tmp;
	oh -> hd.fea -> fea_type = type_tmp;
	oh -> hd.fea -> segment_labeled = segment_tmp;

	if (fgets (buf, BUFSIZE, tstrm) == NULL) {
	    Fprintf (stderr,
	    "%s: premature EOF while trying to convert fea hdr.\n",
	    ProgName);
	}

	while (buf[0] != '\n') {
	    if ((name = strtok (buf, " \t")) == NULL)
		goto trouble;
	    if ((type_chr = strtok ((char *) NULL, " \t")) == NULL)
		goto trouble;
	    if ((size_chr = strtok ((char *) NULL, " \t")) == NULL)
		goto trouble;
	    if ((rank_chr = strtok ((char *) NULL, " \t")) == NULL)
		goto trouble;
	    size = atol (size_chr);
	    rank = atoi (rank_chr);
	    dimens = NULL;
	    enums = NULL;
	    name = savestring (name);

	    if (rank > 1) {
		int     r = rank;
		long   *dp;
		dp = dimens = (long *) calloc ((unsigned)rank, sizeof (long));
		while (r--) {
		    if ((dimen_chr = strtok ((char *) NULL, " \t")) == NULL)
			break;
		    *dp++ = atol (dimen_chr);
		}
	    }

	    type = type_str (type_chr);
	    if (type == CODED) {
		int     k = 0;
		/* name = savestring (name); */	    /* already saved */
		enums = (char **) calloc (1, sizeof (char *));
		while (1) {
		    if (fgets (buf, BUFSIZE, tstrm) == NULL)
			goto trouble;
		    if (*buf != ' ')
			break;
		    enums[k++] = savestring (strtok (buf + 1, "\n"));
		    enums = (char **) realloc ((char *) enums,
		     (unsigned) ((k + 1) * sizeof (char *)));
		    enums[k] = NULL;
		}
	    }

	    if (add_fea_fld(name, size, rank, dimens, type, enums, oh) != 0) {
		Fprintf (stderr,
		"fea_sup: Trouble creating field for %s.\n", name);
		goto trouble;
	    }

	    if (type != CODED)
		if (fgets (buf, BUFSIZE, tstrm) == NULL)
		    goto trouble;

	    if (buf[0] == '\t') {	/* it's a derived field */
		int     k = 0;
		srcfields = (char **) calloc (1, sizeof (char *));
		while (1) {
		    if (buf[0] != '\t')
			break;
		    srcfields[k++] = savestring (strtok (buf + 1, "\n"));
		    srcfields = (char **) realloc ((char *) srcfields,
		     (unsigned) ((k + 1) * sizeof (char *)));
		    srcfields[k] = NULL;
		    if (fgets (buf, BUFSIZE, tstrm) == NULL)
			goto trouble;
		}
		if (set_fea_deriv(name, srcfields, oh) != 0) {
		    Fprintf (stderr,
		    "fea_sup: Trouble setting derived field for %s.\n",
		    name);
		    goto trouble;
		}
	    }
	
	} /* end while (buf[0] != '\n') */

	/*
	 * A single carriage return has just been read.
	 * Next fgets() either gets generic header stuff (i.e. buf[0]
	 * is '@') or data (buf[0] = '#') depending upon the gflag.
	 */

	/* Now process the generic header stuff (if any):
	 *
	 * Get next character in stream to determine if any generic
	 * header stuff exists.  If the character is '@' then generic
	 * header items exist, otherwise they don't.
	 *
	 * Remember to put that character back onto the stream!
	 */

     c = getc(tstrm);

     if (c == '@') {	/* generic items exist */
	(void) ungetc((char) c, tstrm);
	if (fgets (buf, BUFSIZE, tstrm) == NULL)
	    goto trouble;
	while (buf[0] != '\n') {
	    if (flag == 0 || *buf == '\n') {
		if (fgets (buf, BUFSIZE, tstrm) == NULL)
		    break;
	    }
	    else if (*buf == '@') {
		if (debug_level > 0)
		    Fprintf (stderr, "%s: buf = %s\n", ProgName, buf);
		if ((name = strtok (buf, " \t")) == NULL)
		    goto trouble;
		name = savestring (name);
		name++;
		if ((type_chr = strtok ((char *) NULL, " \t")) == NULL)
		    goto trouble;
		if ((size_chr = strtok ((char *) NULL, " \t")) == NULL)
		    goto trouble;

		type = type_str (type_chr);
		size = atol (size_chr);

		if (debug_level > 2)
		    Fprintf (stderr,
		    "%s: name = %s, type = %s, size = %ld\n",
		    ProgName, name, type_codes[type], size);

		if (type == CODED) {
		    int	    gen_fields = 0;
		    cc_ptr = (char **) calloc (1, sizeof (char *));

		    code_ptr = fill_codes1 (cc_ptr, size, name, oh,
			    tstrm, '@', &gen_fields);

		    if (debug_level > 5)
		       Fprintf (stderr, "gen_fields = %d\n", gen_fields);

		    (void) add_genhd_e (name, (short *) NULL, (int) size, code_ptr, oh);

		} else
		    (void) add_genhd (name, type, (int) size,
				      (char *) NULL, (char **) NULL, oh);

		switch (type) {
		    case SHORT:
			sptr = (short *) get_genhd (name, oh);
			flag = fill_shorts (sptr, size, tstrm, '@');
			break;
		    case LONG:
			lptr = (long *) get_genhd (name, oh);
			flag = fill_longs (lptr, size, tstrm, '@');
			break;
		    case FLOAT:
			fptr = (float *) get_genhd (name, oh);
			flag = fill_floats (fptr, size, tstrm, '@');
			break;
		    case DOUBLE:
			dptr = (double *) get_genhd (name, oh);
			flag = fill_doubles (dptr, size, tstrm, '@');
			break;
		    case CHAR:
			cptr = get_genhd (name, oh);
			flag = fill_chars (cptr, size, tstrm, '@');
			break;
		    case CODED:
			sptr = (short *) get_genhd (name, oh);
			flag = fill_gencoded (sptr, size, name,
			oh, tstrm, '@', code_ptr);
			break;
		}
	    }
	    else if (*buf == '#')
		    break;

	}  /* end while (buf[0] != '\n') */
     } else
	(void) ungetc((char) c, tstrm);

	/* write_header (oh, out); */

	return;

trouble:
    Fprintf (stderr, "cvt_feahdr: Trouble reading ascii file back.\n");
    exit (1);
}


cvt_fearec (tstrm, out, oh)
register FILE	*tstrm;
register FILE	*out;
register struct header *oh;
{
    int	    flag = 1,
	    rec_count = 0;

    double *dptr;
    float  *fptr;
    long   *lptr, size;
    short  *sptr, type;
    char   *bptr, *cptr, *name;

    struct  fea_data	*fea_rec;
    struct  fea_data	*allo_fea_rec();


    	fea_rec = allo_fea_rec (oh);

	/* flush a single carraige return */

	if (fgets (buf, BUFSIZE, tstrm) == NULL) {
	    Fprintf (stderr, "cvt_fearec: No data to process back in?\n");
	    exit (0);
	}

	/* read first record line */
	if (fgets (buf, BUFSIZE, tstrm) == NULL) {
	    Fprintf (stderr, "cvt_fearec: No data to process back in?\n");
	    exit (0);
	}

	while (1) {
	    if (flag == 0 || *buf == '\n') {
		put_fea_rec (fea_rec, oh, out);
		clear_fea_rec (fea_rec, oh);
		rec_count++;
		if (fgets (buf, BUFSIZE, tstrm) == NULL)
		   break;
	    }
	    else if (*buf == '#') {
		if (debug_level > 0)
		    Fprintf (stderr, "fea_sup: %s", buf);
		if ((name = strtok (buf, " \t")) == NULL)
		    goto trouble;
		if (strcmp (name + 1, "Tag") == 0) {
		    if ((cptr = strtok ((char *) NULL, " \t\n")) == NULL) {
			Fprintf (stderr, "fea_sup: Missing Tag value.\n");
			goto trouble;
		    }
		    fea_rec -> tag = atol (cptr);
		    if (fgets (buf, BUFSIZE, tstrm) == NULL)
			goto trouble;
		}
		else {
		    name = savestring (name);
		    name++;
		    size = get_fea_siz (name, oh, (short *)NULL, (long **)NULL);
		    if (size == 0) {
			Fprintf (stderr,
			 "fea_sup: name: %s not a valid field.\n", name);
			goto trouble;
		    }
		    type = get_fea_type (name, oh);
		    switch (type) {
			case SHORT:
			    sptr = (short *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_shorts (sptr, size, tstrm, '#');
			    break;
			case LONG:
			    lptr = (long *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_longs (lptr, size, tstrm, '#');
			    break;
			case FLOAT:
			    fptr = (float *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_floats (fptr, size, tstrm, '#');
			    break;
			case DOUBLE:
			    dptr = (double *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_doubles (dptr, size, tstrm, '#');
			    break;
			case CHAR:
			    cptr = get_fea_ptr (fea_rec, name, oh);
			    flag = fill_chars (cptr, size, tstrm, '#');
			    break;
			case BYTE:
			    bptr = get_fea_ptr (fea_rec, name, oh);
			    flag = fill_bytes (bptr, size, tstrm, '#');
			    break;
			case CODED:
			    sptr = (short *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_coded (sptr, size, name, oh, tstrm, '#');
			    break;
		    }
		}
	    }
	}

/*
 *	(void) rewind (out);
 *	oh -> common.ndrec = rec_count;
 *	write_header (oh, out);
 */
	(void) fclose (out);
	(void) fclose (tstrm);

    exit (0);

trouble:
    Fprintf (stderr, "cvt_fearec: Trouble reading ascii file back.\n");
    exit (1);

}

short
type_str (s)
char   *s;
{
    if (strcmp (s, "SHORT") == 0)
	return SHORT;
    if (strcmp (s, "LONG") == 0)
	return LONG;
    if (strcmp (s, "FLOAT") == 0)
	return FLOAT;
    if (strcmp (s, "DOUBLE") == 0)
	return DOUBLE;
    if (strcmp (s, "CHAR") == 0)
	return CHAR;
    if (strcmp (s, "BYTE") == 0)
	return BYTE;
    if (strcmp (s, "CODED") == 0)
	return CODED;
    Fprintf (stderr, "fea_sup: type_str: bad type code: %s \n", s);
    return 0;
}

void
print_rec (rec, hd, file)
struct fea_data *rec;
struct header  *hd;
FILE * file;
{
    int     i, count = 0;
    long    psize,  *l_ptr;
    short   *s_ptr;
    double *d_ptr;
    float  *f_ptr;
    char   *b_ptr, *c_ptr;
    struct fea_header  *fea = hd -> hd.fea;
    if (hd -> common.type != FT_FEA) {
	Fprintf (stderr, "fea_sup: print_fea_rec: called with non fea hd\n");
	exit (1);
    }
    if (hd -> common.tag)
	Fprintf (file, "#Tag %ld\n", rec -> tag);
    for (i = 0; i < fea -> field_count; i++) {
	Fprintf (file, "#%s\t", fea -> names[i]);
	if (fea -> types[i] == DOUBLE)
	    d_ptr = (double *) get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == FLOAT)
	    f_ptr = (float *) get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == LONG)
	    l_ptr = (long *) get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == SHORT || fea -> types[i] == CODED)
	    s_ptr = (short *) get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == CHAR)
	    c_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == BYTE)
	    b_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	count = 0;
	psize = fea -> sizes[i];
	while (psize--) {
	    count++;
	    if (fea -> types[i] == DOUBLE)
		Fprintf (file, "%lg ", *d_ptr++);
	    if (fea -> types[i] == FLOAT)
		Fprintf (file, "%g ", *f_ptr++);
	    if (fea -> types[i] == LONG)
		Fprintf (file, "%ld ", *l_ptr++);
	    if (fea -> types[i] == SHORT)
		Fprintf (file, "%d ", *s_ptr++);
	    if (fea -> types[i] == BYTE)
		Fprintf (file, "%d ", *b_ptr++);
	    if (fea -> types[i] == CHAR)
		if (isprint (*c_ptr))
		    Fprintf (file, "%c", *c_ptr++);
	    if (fea -> types[i] == CODED) {
		if (idx_ok (*s_ptr, fea -> enums[i]))
		    Fprintf (file, "%s ", fea -> enums[i][*s_ptr++]);
		else
		    Fprintf (file, "bad-code:%d ", *s_ptr++);
		count++;
	    }
	    if ((fea -> types[i] != CHAR && count > 7)
		    || (fea -> types[i] == CHAR && count > 50)) {
		count = 0;
		Fprintf (file, "\n\t");
	    }
	}
	Fprintf (file, "\n");
    }
}

int
fill_shorts (sptr, size, strm, symbol)
short  *sptr;
long    size;
FILE * strm;
char	symbol;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_shorts: Too many values, extra ignored\n");
	    else
		*sptr++ = atoi (ptr);
	}
    }
}


int
fill_bytes (bptr, size, strm, symbol)
char  *bptr;
long    size;
FILE * strm;
char	symbol;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_bytes: Too many values, extra ignored\n");
	    else
		*bptr++ = atoi (ptr);
	}
    }
}

int
fill_longs (lptr, size, strm, symbol)
long   *lptr;
long    size;
FILE * strm;
char	symbol;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_longs: Too many values, extra ignored\n");
	    else
		*lptr++ = atol (ptr);
	}
    }
}

int
fill_floats (fptr, size, strm, symbol)
float  *fptr;
long    size;
FILE * strm;
char	symbol;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_floats: Too many values, extra ignored\n");
	    else
		*fptr++ = atof (ptr);
	}
    }
}

int
fill_doubles (dptr, size, strm, symbol)
double *dptr;
long    size;
FILE * strm;
char	symbol;
{
    static long count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_doubles: Too many values, extra ignored\n");
	    else
		*dptr++ = atof (ptr);
	}
    }
}

int
fill_chars (cptr, size, strm, symbol)
char   *cptr;
long    size;
FILE * strm;
char	symbol;
{
    static long count;
    char   *ptr, *s;
    int     len;
    char tokenstring[4];

    sprintf(tokenstring,"\n%c",symbol);

    count = 0;
    s = NULL;
    *cptr = '\0';
    while (1) {
	if ((ptr = strtok (s, tokenstring)) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (((count += strlen (ptr)) > size) &&
		strcmp(&ptr[strlen(ptr)-1], " ")!=0 ) {
		Fprintf (stderr,
			"fea_sup: fill_chars: Too many values, extra ignored\n");
	      }
	    len = size;
	    (void) strncat (cptr, ptr, len);
	}
    }
}


int
fill_coded (sptr, size, name, hd, strm, symbol)
short  *sptr;
long    size;
char   *name;
struct header  *hd;
FILE * strm;
char	symbol;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_coded: Too many values, extra ignored\n");
	    else
		*sptr++ = fea_encode (ptr, name, hd);
	}
    }
}


int
fill_gencoded (sptr, size, name, hd, strm, symbol, codes)
short  *sptr;
long    size;
char   *name;
struct header  *hd;
FILE * strm;
char	symbol;
char	**codes;
{
    long    count;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_gencoded: Too many values, extra ignored\n");
	    else {
		/* *sptr++ = fea_encode (ptr, name, hd); */
		*sptr++ = lin_search2(codes, ptr);
		if (debug_level > 5)
		    Fprintf (stderr, "lin_search2(codes, %s) = %d\n",
		    ptr, lin_search2(codes, ptr));
		}
	}
    }
}


int
fill_codes (code_ptr, size, name, hd, strm, symbol)
char	**code_ptr;
long    size;
char	*name;
struct header  *hd;
FILE	*strm;
char	symbol;
{
    long    count;
    long    index = 0;
    char   *ptr, *s;

    count = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_sup: fill_codes: Too many values, extra ignored\n");
	    else
		code_ptr[index++] = savestring (ptr);
	}
    }
}


char **
fill_codes1 (code_ptr, size, name, hd, strm, symbol, gen_fields)
char	**code_ptr;
long    size;
char	*name;
struct header  *hd;
FILE	*strm;
char	symbol;
int	*gen_fields;
{
    char   *ptr, *s;

    *gen_fields = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (fgets (buf, BUFSIZE, strm) == NULL)
		return (code_ptr);
	    if (*buf == symbol || *buf == '\n')
		return (code_ptr);
	    s = buf;
	}
	else {
	    s = NULL;

	    if (debug_level > 5)
	        Fprintf (stderr, "ptr = %s\n", ptr);

	    if (index (ptr, ']') != NULL)  /* ']' exists in string */
		return (code_ptr);
	    if (index (ptr, '[') == NULL)  {
		/* '[' does not exist in string */
		code_ptr[*gen_fields] = savestring (ptr);
		*gen_fields += 1;
		code_ptr = (char **) realloc ((char *) code_ptr,
			    (unsigned) ((*gen_fields + 1) * sizeof (char *)));
		code_ptr[*gen_fields] = NULL;
		if (debug_level > 5) {
		    Fprintf (stderr,
		    "fill_codes1: code_ptr[*gen_fields = %d] = %s\n",
		    *gen_fields - 1, code_ptr[*gen_fields - 1]);
		    (void) fflush (stderr);
		}
	    }

	}
    }

}


void
clear_fea_rec (rec, hd)
struct fea_data *rec;
struct header  *hd;
{
    double *dptr;
    float  *fptr;
    short  *sptr;
    long   *lptr;
    char   *cptr;
    int     k;

    k = hd -> common.ndouble;
    dptr = rec -> d_data;
    while (k--)
	*dptr++ = 0;
    k = hd -> common.nfloat;
    fptr = rec -> f_data;
    while (k--)
	*fptr++ = 0;
    k = hd -> common.nlong;
    lptr = rec -> l_data;
    while (k--)
	*lptr++ = 0;
    k = hd -> common.nshort;
    sptr = rec -> s_data;
    while (k--)
	*sptr++ = 0;
    k = hd -> common.nchar;
    cptr = rec -> b_data;
    while (k--)
	*cptr++ = 0;
}
