
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by: Ken Nelson
 *
 * Brief description: ascii editing of fea files
 *
 */

static char *sccs_id = "@(#)fea_edit.c	3.12	6/9/93	ESI/ERL";

/* fea_edit - edit an ESPS FEA file
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 * 	
 *  This program is an ESPS feature file editor.   It puts the feature
 *  file into a temp file in ascii and calls the editor named by the
 *  environment variable "EDITOR" or vi if that is NULL.     Usage is
 *   
 *  fea_edit [-x debug-level] [-n] [-g] file [output-file]
 *
 *  -x is debug option
 *  -n causes the original header not be saved as a embedded header
 *  -g causes the generic headers to be present for editing also
 *  file is the file to edit
 *  output-file is an optional output file.   If no output file is given
 *  then the changed file overwrites the input file.   If an output file
 *  is given, then the input file is not changed.
 *
 *  Author: Alan Parker, ESI, Washington, DC
 *	    -g option by Ajaipal S. Virdy
 *
 */

/* ESPS includes */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>

/* Unix includes */
#include <ctype.h>

#define VERSION "3.12"
#define DATE "6/9/93"

#define PROG "fea_edit"
#define BUFSIZE 1024
#define DEFAULT_ED "vi"

/*
 * E S P S
 *  R O U T I N E S
 *   R E F E R E N C E D
 */

int	add_fea_fld ();
float	**f_mat_alloc ();
char	*get_cmd_line ();
char	*get_fea_ptr ();
char	*get_genhd ();
long    get_fea_siz ();
int	lin_search2 ();
char	*savestring ();
short   get_fea_type ();
short	fea_encode ();
int	set_fea_deriv ();
char    *e_temp_name();

/* L O C A L  R O U T I N E S */

void	clear_fea_rec ();
char	**fill_codes1 ();
void	print_rec ();
short	type_str ();
char    *local_fgets();

/* G L O B A L  V A R I A B L E S */

char	buf[BUFSIZE];
char	*editor;
int	debug_level = 0;


main (argc, argv)
int     argc;
char  **argv;
{
    int	    c,
	    flag = 1,	    /* flag for reading new line into buf */
	    rec_count = 0;

    extern  optind, errno;
    extern  char *optarg;

    char    *ProgName = "fea_edit";

    char    *tempfile;
    FILE    *in,
	    *tstrm,
	    *out;

    struct header   *ih,
		    *oh;

    struct fea_data	*fea_rec;
    struct fea_header	*fea;

    int     i,		    /* auxilary array indices */
	    j,
	    k;

    int	    nflag = 0,	    /* don't save original header if flag is set */
	    update = 0,	    /* update file if set, else create file */
    	    gflag = 0;	    /* allow generic header editing if flag set */

    int	    type_tmp,
	    tag_tmp,
	    segment_tmp,
	    ngen;

    char    **gen_names,
	    **genhd,
	    *outfile,
	    **code_ptr,
	    **cc_ptr,
	    **enums = NULL,
	    **srcfields = NULL;

    double *dptr;
    float  *fptr;
    long   *lptr, *dimens = NULL;
    short  *sptr, rank, type;
    char   *bptr, *cptr, *dimen_chr, *name, *type_chr, *size_chr, *rank_chr;
    int    size;
    long   rec_num=1;

    int	    getopt ();

    if (argc == 1) {
	Fprintf (stderr,
	"Usage: fea_edit [-x debug_level] [-g] [-n] file [outfile]\n");
	exit (0);
    }

    while ((c = getopt (argc, argv, "x:ng")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'n': 
		nflag++;
		break;
	    case 'g':
		gflag++;
		break;
	}
    }

    switch (argc - optind) {
	case 1:  update = YES;
		 break;
	case 2:  update = NO;
		 break;
	default: Fprintf (stderr, 
		 "Usage: fea_edit [-x debug_level] [-g] [-n] file [outfile]\n");
		 exit (1);
		 break;
    }

    outfile = argv[argc - 1];

    if (debug_level > 0)
	if (!update)
	    Fprintf (stderr,
	    "%s: input file is %s, output file is %s.\n",
	    ProgName, argv[optind], outfile);
	else
	    Fprintf (stderr,
	    "%s: input file %s will be updated.\n",
	    ProgName, argv[optind]);

    TRYOPEN ("fea_edit", argv[optind], "r", in);
    if (!(ih = read_header (in)))
	NOTSPS ("fea_edit", argv[optind]);
    if (ih -> common.type != FT_FEA) {
	Fprintf (stderr, "fea_edit: %s not a FEA file.\n", argv[optind]);
	exit (1);
    }
/* for now this program doesn't do FEA files with any complex fields;
   this will have to change
*/
    if (is_file_complex(ih)) {
	Fprintf(stderr,
	 "fea_edit: this program cannot handle a file with complex fields, yet.\n");
	exit(1);
    }
	

    tempfile = e_temp_name("editXXXXXX");

    if (debug_level)
      Fprintf(stderr, "fea_edit: temp file is %s.\n", tempfile);

    if ((tstrm = fopen (tempfile, "w")) == NULL)
	CANTOPEN ("fea_edit", tempfile);

    fea_rec = allo_fea_rec (ih);
    fea = ih -> hd.fea;

    /*
     * Display Feature Header information.
     */

    Fprintf (tstrm, 
     "%x %x %x << do not edit >>\n", ih -> common.tag, fea -> fea_type,
     fea -> segment_labeled);
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
    }  /* end for (i = 0; i < fea->field_count; i++) */

    /*
     * Display Generic Header information if gflag is set.
     */

  if (gflag) {
    gen_names = genhd_list (&ngen, ih);
    for (i = 0; i < ngen; i++) {
	char *cptr; double *dptr; float *fptr; long *lptr; short *sptr;
	type = genhd_type (gen_names[i], &size, ih);
	Fprintf (tstrm, "\n");
	Fprintf (tstrm, "@%s %s %d ", gen_names[i], type_codes[type], size);
	k = 0;
	cptr = get_genhd(gen_names[i], ih);
	if (type == CODED) {
	    char    **s;
	    s = genhd_codes(gen_names[i], ih);
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
		    dptr = (double *)get_genhd(gen_names[i],ih);
		    Fprintf (tstrm, "%lg ",dptr[j]);
		    break;
		case FLOAT: 
		    fptr = (float *)get_genhd(gen_names[i],ih);
		    Fprintf (tstrm, "%g ",fptr[j]);
		    break;
		case SHORT: 
		    sptr = (short *)get_genhd(gen_names[i],ih);
		    Fprintf (tstrm, "%d ",sptr[j]);
		    break;
		case LONG: 
		    lptr = (long *)get_genhd(gen_names[i],ih);
		    Fprintf (tstrm, "%ld ",lptr[j]);
		    break;
		case CHAR: 
		    cptr = get_genhd(gen_names[i],ih);
		    Fprintf (tstrm, "0x%x",cptr[j]);
		    break;
		case CODED:  {
		    char **codes = genhd_codes(gen_names[i],ih);
		    sptr = (short *)get_genhd(gen_names[i],ih);
		    if (idx_ok(sptr[j],codes))
			Fprintf (tstrm, "%s ",codes[sptr[j]]);
		    else
			Fprintf (tstrm, "invalid code %d ",sptr[j]);
		    break;
		}
		}
	    }
    }

    if (ngen > 0)
	Fprintf (tstrm, "\n");
  }

    /*
     * Display record information.
     */

    while (get_fea_rec (fea_rec, ih, in) != EOF) {
	Fprintf (tstrm, "\n");
	Fprintf (tstrm, "* Record %ld\n",rec_num++);
	print_rec (fea_rec, ih, tstrm);

    }

    (void ) fclose (in);
    (void ) fclose (tstrm);

    /*
     * Enable editing of file.
     */

again:
    if ((editor = getenv ("EDITOR")) == NULL)
	editor = DEFAULT_ED;
    (void) sprintf (buf, "%s %s", editor, tempfile);
    if (system (buf) != NULL) {
	Fprintf(stderr,"fea_edit: Cannot run editor. Buffer: %s\n",buf);
	(void) unlink(tempfile);
    }

    Fprintf(stderr,"fea_edit: Make changes? [return for yes, n for abort] ");

    if (strcmp(gets(buf),"n") != 0) {
	if (update) {
	    Fprintf (stderr,"fea_edit: Making changes to input file, ");
	    (void) sprintf (buf, "%s.bak", argv[optind]);
	    if (rename (argv[optind], buf) != 0) {
	        perror ("fea_edit");
	        (void) unlink(tempfile);
	        exit (1);
	    }
	    Fprintf (stderr,"original %s saved in %s\n", argv[optind], buf);
	}
	else
	    Fprintf(stderr,"fea_edit: Creating output file %s\n", outfile);
		
	TRYOPEN ("fea_edit", outfile, "w+", out);
	TRYOPEN ("fea_edit", tempfile, "r", tstrm);

	/*
	 * Read edited file back into new file (or updated file).
	 */

	oh = new_header (FT_FEA);
	if (!nflag) {
	    add_source_file (oh, argv[optind], ih);
	    oh->variable.refhd = ih->variable.refhd;
	}

	(void) add_comment (oh, get_cmd_line (argc, argv));
	(void) strcpy(oh->common.prog, PROG);
	(void) strcpy(oh->common.vers, VERSION);
	(void) strcpy(oh->common.progdate, DATE);
	oh->variable.refer = ih->variable.refer;
	
    if (!gflag) {
	genhd = genhd_list(&ngen,ih);
	while (ngen--) {
	    int size;
	    short type;
	    type = genhd_type (genhd[ngen], &size, ih);
	    (void) add_genhd (genhd[ngen],type, size, get_genhd(genhd[ngen],
		ih), genhd_codes(genhd[ngen], ih), oh);
	}
    }

	if (fgets (buf, BUFSIZE, tstrm) == NULL)
	    goto trouble;
	(void) sscanf (buf, "%x %x %x", &tag_tmp, &type_tmp, &segment_tmp);
	oh -> common.tag = tag_tmp;
	oh -> hd.fea -> fea_type = type_tmp;
	oh -> hd.fea -> segment_labeled = segment_tmp;

	if (fgets (buf, BUFSIZE, tstrm) == NULL)
	    goto trouble;

	while (buf[0] != '\n') {
	    if ((name = strtok (buf, " \t")) == NULL)
		goto trouble;
	    if ((type_chr = strtok (0, " \t")) == NULL)
		goto trouble;
	    if ((size_chr = strtok (0, " \t")) == NULL)
		goto trouble;
	    if ((rank_chr = strtok (0, " \t")) == NULL)
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
		    if ((dimen_chr = strtok (0, " \t")) == NULL)
			break;
		    *dp++ = atol (dimen_chr);
		}
	    }

	    type = type_str (type_chr);
	    if (type == CODED) {
		int     k = 0;
		/* name = savestring (name); */  /* already saved */
		enums = (char **) calloc (1, sizeof (char *));
		while (1) {
		    if (fgets (buf, BUFSIZE, tstrm) == NULL)
			goto trouble;
		    if (*buf != ' ')
			break;
		    enums[k++] = savestring (strtok (buf + 1, "\n"));
		    enums = (char **) realloc ((char *) enums, 
		     (k + 1) * sizeof (char *));
		    enums[k] = NULL;
		}
	    }
	    if(add_fea_fld(name,(long)size,rank,dimens,type,enums,oh) != 0) {
		Fprintf (stderr, 
	         "fea_edit: Trouble creating field for %s.\n", name);
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
		    (k + 1) * sizeof (char *));
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

	}  /* end while (buf[0] != '\n') */

	/*
	 * A single carriage return has just been read.
	 * Next fgets() either gets generic header stuff (i.e. buf[0]
	 * is '@') or data (buf[0] = '#') depending upon the gflag.
	 */

	/* Now process the generic header stuff (if any) */

    if (gflag) {
	if (fgets (buf, BUFSIZE, tstrm) == NULL)    /* buf[0] is '@' */
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
		if ((type_chr = strtok (0, " \t")) == NULL)
		    goto trouble;
		if ((size_chr = strtok (0, " \t")) == NULL)
		    goto trouble;

		type = type_str (type_chr);
		size = atol (size_chr);

		if (debug_level > 2)
		    Fprintf (stderr,
		    "%s: name = %s, type = %s, size = %d\n",
		    ProgName, name, type_codes[type], size);

		if (type == CODED) {
		    int	    gen_fields = 0;

		    cc_ptr = (char **) calloc (1, sizeof (char *));
		    code_ptr = fill_codes1 (cc_ptr, (long)size, name, oh,
			    tstrm, '@', &gen_fields);

		    if (debug_level > 5)
		       Fprintf (stderr, "gen_fields = %d\n", gen_fields);

		    (void) add_genhd_e (name, NULL, size, code_ptr, oh);

		} else
		    (void) add_genhd (name, type, size,
				      NULL, (char **) NULL, oh);

		switch (type) {
		    case SHORT:
			sptr = (short *) get_genhd (name, oh);
			flag = fill_shorts (sptr, (long)size, tstrm, '@');
			break;
		    case LONG:
			lptr = (long *) get_genhd (name, oh);
			flag = fill_longs (lptr, (long)size, tstrm, '@');
			break;
		    case FLOAT:
			fptr = (float *) get_genhd (name, oh);
			flag = fill_floats (fptr, (long)size, tstrm, '@');
			break;
		    case DOUBLE:
			dptr = (double *) get_genhd (name, oh);
			flag = fill_doubles (dptr, (long)size, tstrm, '@');
			break;
		    case CHAR: 
			cptr = get_genhd (name, oh);
			flag = fill_chars (cptr, (long)size, tstrm, '@');
			break;
		    case CODED: 
			sptr = (short *) get_genhd (name, oh);
			flag = fill_gencoded (sptr, (long)size, name,
			oh, tstrm, '@', code_ptr);
			break;
		}
	    }
	    else if (*buf == '#')
		    break;

	}  /* end while (buf[0] != '\n') */
    }

	flag = 1;   /* reset flag */

	write_header (oh, out);

	fea_rec = allo_fea_rec (oh);

	/* A single carriage return following the generic header (if any)
	 * has just been read.  Next fgets() gets data (i.e. buf[0] = '#').
	 */

	if (local_fgets (buf, BUFSIZE, tstrm) == NULL) {
	    Fprintf (stderr, "fea_edit: No data to process back in?\n");
	    exit (0);
	}

	while (1) {
	    if (flag == 0 || *buf == '\n') {
		put_fea_rec (fea_rec, oh, out);
		clear_fea_rec (fea_rec, oh);
		rec_count++;
		if (local_fgets (buf, BUFSIZE, tstrm) == NULL)
		    break;
	    }
	    else if (*buf == '#') {
		if (debug_level > 0)
		    Fprintf (stderr, "fea_edit: %s", buf);
		if ((name = strtok (buf, " \t")) == NULL)
		    goto trouble;
		if (strcmp (name + 1, "Tag") == 0) {
		    if ((cptr = strtok (0, " \t\n")) == NULL) {
			Fprintf (stderr, "fea_edit: Missing Tag value.\n");
			goto trouble;
		    }
		    fea_rec -> tag = atol (cptr);
		    if (local_fgets (buf, BUFSIZE, tstrm) == NULL)
			goto trouble;
		}
		else {
		    name = savestring (name);
		    name++;
		    size = get_fea_siz (name, oh, NULL, NULL);
		    if (size == 0) {
			Fprintf (stderr, 
			 "fea_edit: name: %s not a valid field.\n", name);
			goto trouble;
		    }
		    type = get_fea_type (name, oh);
		    switch (type) {
			case SHORT: 
			    sptr = (short *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_shorts (sptr, (long)size, tstrm, '#');
			    break;
			case LONG: 
			    lptr = (long *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_longs (lptr, (long)size, tstrm, '#');
			    break;
			case FLOAT: 
			    fptr = (float *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_floats (fptr, (long)size, tstrm, '#');
			    break;
			case DOUBLE: 
			    dptr = (double *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_doubles (dptr, (long)size, tstrm, '#');
			    break;
			case CHAR: 
			    cptr = get_fea_ptr (fea_rec, name, oh);
			    flag = fill_chars (cptr, (long)size, tstrm, '#');
			    break;
			case BYTE: 
			    bptr = get_fea_ptr (fea_rec, name, oh);
			    flag = fill_bytes (bptr, (long)size, tstrm, '#');
			    break;
			case CODED: 
			    sptr = (short *) get_fea_ptr (fea_rec, name, oh);
			    flag = fill_coded (sptr, (long)size, name, oh, tstrm, '#');
			    break;
		    }
		}
	    }
	}

	rewind (out);
	oh -> common.ndrec = rec_count;
	write_header (oh, out);
	(void) fclose (out);
	(void) fclose (tstrm);
    }
    else {			/* other part of the if abort */
	Fprintf (stderr,"fea_edit: Aborting without making changes.\n");
    }
    (void) unlink (tempfile);
    exit (0);

trouble: 
    Fprintf (stderr, "fea_edit: Trouble reading temp file back.\n");
    Fprintf (stderr, "fea_edit: Temp file saved in %s\n", tempfile);
    Fprintf (stderr, "fea_edit: Want to edit text again? [yn] ");
    if (strcmp(gets(buf),"n") != 0)
	goto again;
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
    if (strcmp (s, "CODED") == 0)
	return CODED;
    if (strcmp (s, "BYTE") == 0)
	return BYTE;
    Fprintf (stderr, "fea_edit: bad type code: %s \n", s);
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
    char   *c_ptr, *b_ptr;
    struct fea_header  *fea = hd -> hd.fea;
    if (hd -> common.type != FT_FEA) {
	Fprintf (stderr, "fea_edit: print_fea_rec: called with non fea hd\n");
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
	if (fea -> types[i] == BYTE)
	    b_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == SHORT || fea -> types[i] == CODED)
	    s_ptr = (short *) get_fea_ptr (rec, fea -> names[i], hd);
	if (fea -> types[i] == CHAR)
	    c_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	count = 0;
	psize = fea -> sizes[i];
	while (psize--) {
	    count++;
	    if (fea -> types[i] == DOUBLE)
		Fprintf (file, "%.8lg ", *d_ptr++);
	    if (fea -> types[i] == FLOAT)
		Fprintf (file, "%.8g ", *f_ptr++);
	    if (fea -> types[i] == LONG)
		Fprintf (file, "%ld ", *l_ptr++);
	    if (fea -> types[i] == SHORT)
		Fprintf (file, "%d ", *s_ptr++);
	    if (fea -> types[i] == BYTE)
		Fprintf (file, "%d ", (int)*b_ptr++);
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
		if (psize)  /* more data remains to be printed */
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_shorts: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_bytes: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_longs: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_floats: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_doubles: Too many values, extra ignored\n");
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
    char tokenstring[3];


    sprintf(tokenstring,"\n%c",symbol);

    count = 0;
    s = NULL;
    *cptr = '\0';
    while (1) {
	if ((ptr = strtok (s, tokenstring)) == NULL) {
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if ( ((count += strlen (ptr) + 1) > size) &&
		strcmp(&ptr[strlen(ptr)-1]," ")!=0)
	      Fprintf (stderr,
		       "fea_edit: fill_chars: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_coded: Too many values, extra ignored\n");
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
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
		return 0;
	    if (*buf == symbol || *buf == '\n')
		return 1;
	    s = buf;
	}
	else {
	    s = NULL;
	    if (count++ > size)
		Fprintf (stderr,
			"fea_edit: fill_gencoded: Too many values, extra ignored\n");
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


/*
 * This funtion is not used in this program anymore:
 *
 * int
 * fill_codes (code_ptr, size, name, hd, strm, symbol)
 * char	**code_ptr;
 * long    size;
 * char	*name;
 * struct header  *hd;
 * FILE	*strm;
 * char	symbol;
 * {
 *     long    count;
 *     long    index_val = 0;
 *     char   *ptr, *s;
 * 
 *     count = 0;
 *     s = NULL;
 *     while (1) {
 * 	if ((ptr = strtok (s, " \t\n")) == NULL) {
 * 	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
 * 		return 0;
 * 	    if (*buf == symbol || *buf == '\n')
 * 		return 1;
 * 	    s = buf;
 * 	}
 * 	else {
 * 	    s = NULL;
 * 	    if (count++ > size)
 * 		Fprintf (stderr,
 * 		"fea_edit: fill_codes: Too many values, extra ignored\n");
 * 	    else
 * 		code_ptr[index_val++] = savestring (ptr);
 * 	}
 *     }
 * }
 * 
 */


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
    long    index_val = 0;
    char   *ptr, *s;

    *gen_fields = 0;
    s = NULL;
    while (1) {
	if ((ptr = strtok (s, " \t\n")) == NULL) {
	    if (local_fgets (buf, BUFSIZE, strm) == NULL)
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
				     (*gen_fields + 1) * sizeof (char *));
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

char *
local_fgets(buf, n, stream)
char *buf;
int n;
FILE *stream;
{
    if(fgets(buf,n,stream) == NULL) return NULL;
    while(*buf == '*')
        if(fgets(buf,n,stream) == NULL) return NULL;
    return buf;
}

