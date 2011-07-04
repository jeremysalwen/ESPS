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
 * Brief description: creates new fea file feidl and fills it with data
 *
 */

static char *sccs_id = "@(#)addfea.c	1.20	7/1/93	ESI/ERL";

int     debug_level = 0;


#ifndef lint
#define DATE "7/1/93"
#define VERS "1.20"
#else
#define DATE "none"
#define VERS "none"
#endif

#define SYNTAX USAGE("addfea [-P param] [-f field_name] [-t field_type] [-s field_size] \n\t[-T subtype_code] [-x debug_level] [-c comment] [-C comment_file] ascfile feafile [feafile.out]")
#define COMMENT_LINE_SIZE 512

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>

extern int  optind;
extern char *optarg;
char    sbuf[256];
FILE * fopen ();
char   *get_cmd_line ();
void copy_fea_rec ();
char  *savestring();
char *add_lf();

#define error(s,f) error2(s," ",f)
void
error2 (s, t, flag)
char   *s,
       *t;
int     flag;
{
    (void) fprintf (stderr, "addfea: %s %s\n", s, t);
    if (flag)
	SYNTAX;
    exit (1);
}

void
get_comment (file, hd)
char   *file;
struct header  *hd;
{
    FILE * strm;
    char   *buffer = malloc ((unsigned) COMMENT_LINE_SIZE);

    spsassert (buffer, "malloc failed!");
    if (strcmp ("<stdin>", file) == 0) {
	fprintf (stderr, "addfea: Please enter comment, ended with blank line:\n");
	strm = stdin;
    }
    else
    if ((strm = fopen (file, "r")) == NULL)
	error2 ("cannot open comment file:", file, 0);

    while (fgets (buffer, COMMENT_LINE_SIZE, strm) != NULL) {
	if (strm == stdin && strlen (buffer) == 1)
	    return;
	add_comment (hd, buffer);
    }
    free (buffer);
}

char *
add_lf(line)
char *line;			/* line to add a line feed to */
{
  char *newline = malloc((unsigned) strlen(line) + 2);
  return(savestring(strcat(strcpy(newline, line),"\n")));
}

main (argc, argv)
int     argc;
char  **argv;
{
    char   *out_file = NULL;
    char   *in_file = NULL;
    char   *asc_file = NULL;
    FILE * asc_strm = NULL;
    FILE * in_strm = NULL;
    FILE * out_strm = NULL;
    char   *subtype = NULL;
    int    subtype_code = 0;
    char   *comment = NULL;
    char   *comment_file = NULL;
    struct header  *in_hd,
                   *out_hd;
    struct fea_data *out_rec,
                   *in_rec;
    int     c,
            new_file = 0,	/* 1 when output file is new */
            f_flag = 0,
            t_flag = 0,
            s_flag = 0,
            update = 0,		/* 1 when updating input file */
	    add_tag=0;
    short   type;
    char   *ptr;
    double  dbuf;
    int     size;
    long   *l_ptr;
    short  *s_ptr;
    float  *f_ptr;
    double *d_ptr;
    char   *c_ptr;
    double_cplx *dc_ptr;
    float_cplx *fc_ptr;
    long_cplx *lc_ptr;
    short_cplx *sc_ptr;
    byte_cplx *bc_ptr;

/* default parameters */

    char   *param_file = NULL;/* default parameter file name */
    char   *field_name = NULL;	/* NO default field_name */
    char   *field_type = "FLOAT";/* default field type */
    long    field_size = 1;	/* default field_size */

    while ((c = getopt (argc, argv, "P:f:t:s:c:C:T:x:")) != EOF) {
	switch (c) {
	    case 'P': 
		param_file = optarg;
		break;
	    case 'f': 
		f_flag = 1;
		field_name = optarg;
		break;
	    case 't': 
		t_flag = 1;
		field_type = optarg;
		break;
	    case 's': 
		s_flag = 1;
		field_size = atoi (optarg);
		break;
	    case 'c': 
		comment = add_lf(optarg);
		break;
	    case 'C': 
		comment_file = optarg;
		break;
	    case 'T':
		subtype = optarg;
		break;
	    case 'x': 
		debug_level = 1;
		break;
	}
    }
    if ((argc - optind) < 2)
	error ("at least two files must be given", 1);

    if (field_name && strcmp(field_name,"Tag") == 0) {
	add_tag = 1;
	t_flag=1;
	field_type = "LONG";
    }

    if (comment != NULL && comment_file != NULL)
	error ("cannot give both -C and -c options", 1);

    if (!(f_flag && t_flag && s_flag && (comment || comment_file))) {
/* need to read parameter file */
	(void) read_params (param_file, SC_NOCOMMON, NULL);
	if (!f_flag && (symtype ("field_name") == ST_STRING))
	    field_name = getsym_s ("field_name");
	if (!t_flag && (symtype ("field_type") == ST_STRING))
	    field_type = getsym_s ("field_type");
	if (!s_flag && (symtype ("field_size") == ST_INT))
	    field_size = getsym_i ("field_size");

	if (!subtype && (symtype ("FEA_subtype") == ST_STRING))
	    subtype = getsym_s ("FEA_subtype");

	if (!(comment || comment_file) && symtype ("comment") ==
		ST_STRING)
	    comment = getsym_s ("comment");
    }



    if ((argc - optind) == 3)
	out_file = argv[optind + 2];

    asc_file = argv[optind];
    in_file = argv[optind + 1];

    if (strcmp (asc_file, in_file) == 0)
	error ("ascfile and feafile cannot be the same", 1);


    if (field_name == NULL)
	error("no \"field_name\" specifed - exiting", 0);

    if (field_type == NULL)
	error ("no \"field_type\" specified - exiting.", 0);

    if (debug_level) {
	fprintf (stderr, "comment: %s\n", comment);
	fprintf (stderr, "comment_file %s\n", comment_file);
	fprintf (stderr, "asc_file: %s\n", asc_file);
	fprintf (stderr, "in_file: %s\n", in_file);
	fprintf (stderr, "out_file: %s\n", out_file);
	fprintf (stderr, "param_file: %s\n", param_file);
	fprintf (stderr, "field_name: %s\n", field_name);
	fprintf (stderr, "field_type: %s\n", field_type);
	fprintf (stderr, "field_size: %d\n", field_size);
    }

    type = lin_search (type_codes, field_type);
    if (type == -1)
	error2 ("unknown type code:", field_type, 0);
    if (type == CODED)
	error ("type CODED not supported yet", 0);

/* 
  I've already checked that asc_file and in_file aren't both the same
*/
    if (strcmp (asc_file, "-") == 0)
	asc_strm = stdin;
    else
    if (!(asc_strm = fopen (asc_file, "r")))
	error2 ("cannot open ASCII data file:", asc_file, 0);

/* 
  If a third filename is given, then take it to be an output file to
  open for writing.  In this case, the second filename must be an
  existing feature file.
  
*/

    if (out_file) {
	new_file = 1;
	out_file = eopen ("addfea", out_file, "w", FT_FEA, NONE,
		&out_hd, &out_strm);

	in_file = eopen ("addfea", in_file, "r", FT_FEA, NONE,
		&in_hd, &in_strm);
	in_rec = allo_fea_rec (in_hd);
    }
    else {
/*
  An output file is not given, so the second name might be either an
  existing file for update, or a non-existing file for output
*/

	if ((in_strm = fopen (in_file, "r")) == NULL) {
	    /* file does not exist */
	    if(add_tag)
		error("cannot create a new file with just a tag", 0);
	    new_file = 1;
	    in_file = eopen ("addfea", in_file, "w", FT_FEA, NONE,
		    &out_hd, &out_strm);
	}
	else {
	    /* file does exist, to be updated */
	    fclose (in_strm);	/* prefer to use eopen */
	    in_file = eopen ("addfea", in_file, "r", FT_FEA, NONE,
		    &in_hd, &in_strm);
	    in_rec = allo_fea_rec (in_hd);
	    update = 1;
	    out_strm = tmpfile ();
	}
    }


    if (new_file & !out_file) {
	out_hd = new_header (FT_FEA);
    }
    else {
	out_hd = copy_header (in_hd);
	add_source_file (out_hd, in_file, in_hd);
    }
    strcpy (out_hd -> common.prog, "addfea");
    strcpy (out_hd -> common.progdate, DATE);
    strcpy (out_hd -> common.vers, VERS);

    if (add_tag) {
	out_hd->common.tag = 1;
    }

    add_comment (out_hd, get_cmd_line (argc, argv));
    sprintf (sbuf, "Field %s added by addfea\n", field_name);
    add_comment (out_hd, sbuf);

    if (comment_file)
	get_comment (comment_file, out_hd);
    else if (comment) 
	add_comment (out_hd, comment);
    else if (strcmp ("-", asc_file) && strcmp ("<stdin>", in_file))
	get_comment ("<stdin>", out_hd);
    else
	error ("you must give a comment on the command line or in the parameter file", 0);

    if (field_size < 1)
	error ("the field size cannot be less than 1", 0);

    if (!add_tag && 
	(add_fea_fld (field_name, field_size, (short) 1, (long *) NULL, type,
		(char **) NULL, out_hd) == -1))
	error2 ("field already in output file:", field_name, 0);

    /*set subtype code if called for */

    if (subtype) {
       if ((subtype_code = lin_search(fea_file_type, subtype)) == -1)
	  Fprintf(stderr,"addfeahd: unknown FEA file subtype %s ignored.\n",
		subtype);
       else
	  out_hd->hd.fea->fea_type = subtype_code;
     }

	

    write_header (out_hd, out_strm);

    out_rec = allo_fea_rec (out_hd);
    size = field_size;
    if (!add_tag) {
    	ptr = get_fea_ptr (out_rec, field_name, out_hd);
    	spsassert (ptr != NULL, "get_fea_ptr returned null");

    	size = field_size;
    	d_ptr = (double *) ptr;
    	f_ptr = (float *) ptr;
    	l_ptr = (long *) ptr;
    	s_ptr = (short *) ptr;
    	c_ptr = ptr;
    	dc_ptr = (double_cplx *) ptr;
    	fc_ptr = (float_cplx *) ptr;
    	lc_ptr = (long_cplx *) ptr;
    	sc_ptr = (short_cplx *) ptr;
    	bc_ptr = (byte_cplx *) ptr;
     }
     else
	l_ptr = &(out_rec->tag);

    while (1) {
      int retval = 0;
	if (type != CHAR) {
	    if ((retval = fscanf (asc_strm, "%lf", &dbuf)) == EOF)
		break;
	    if(retval == 0){
	      Fprintf(stderr, 
"addfea: Data type in file \"%s\" and specified value (%s) differ - exiting.\n",
		      asc_file, field_type);
	      exit(1);
	    }
        }
	else if (type == CHAR && field_size != 1) {
	    if (fgets (ptr, size+1, asc_strm) == NULL)
	    	break;
	}
	else if (type == CHAR && field_size == 1) {
	    if (fscanf (asc_strm, "%c", ptr) == EOF)
		break;
	}
	switch (type) {
	    case DOUBLE: 
		*d_ptr++ = dbuf;
		break;
	    case FLOAT: 
		*f_ptr++ = dbuf;
		break;
	    case LONG: 
		*l_ptr++ = dbuf;
		break;
	    case SHORT: 
		*s_ptr++ = dbuf;
		break;
	    case BYTE:
		*c_ptr++ = dbuf;
		break;
	    case CHAR: 
		size = 1;
		break;
	    case DOUBLE_CPLX:
		dc_ptr->real = dbuf;
	    	if (fscanf (asc_strm, "%lf", &dbuf) == EOF)
			dc_ptr->imag = 0;
		else
			dc_ptr->imag = dbuf;
		dc_ptr++;
		break;
	    case FLOAT_CPLX:
		fc_ptr->real = dbuf;
	    	if (fscanf (asc_strm, "%lf", &dbuf) == EOF)
			fc_ptr->imag = 0;
		else
			fc_ptr->imag = dbuf;
		fc_ptr++;
		break;
	    case LONG_CPLX:
		lc_ptr->real = dbuf;
	    	if (fscanf (asc_strm, "%lf", &dbuf) == EOF)
			lc_ptr->imag = 0;
		else
			lc_ptr->imag = dbuf;
		lc_ptr++;
		break;
	    case SHORT_CPLX:
		sc_ptr->real = dbuf;
	    	if (fscanf (asc_strm, "%lf", &dbuf) == EOF)
			sc_ptr->imag = 0;
		else
			sc_ptr->imag = dbuf;
		sc_ptr++;
		break;
	    case BYTE_CPLX:
		bc_ptr->real = dbuf;
	    	if (fscanf (asc_strm, "%lf", &dbuf) == EOF)
			bc_ptr->imag = 0;
		else
			bc_ptr->imag = dbuf;
		bc_ptr++;
		break;
	    default: 
		error ("internal error, should not be here #1", 0);
	      }

	if (size-- == 1) {
	    if (new_file && !out_file)
		put_fea_rec (out_rec, out_hd, out_strm);
	    else
	    if (get_fea_rec (in_rec, in_hd, in_strm) != EOF) {
		copy_fea_rec (in_rec, in_hd, out_rec, out_hd,
			(char **) NULL, (short **) NULL);
		if(add_tag)
			out_rec->tag = dbuf;
		put_fea_rec (out_rec, out_hd, out_strm);
	      }
	    else {

		break;
	    }
	    size = field_size;
	    if(!add_tag) {
	    	d_ptr = (double *) ptr;
	    	f_ptr = (float *) ptr;
	    	l_ptr = (long *) ptr;
	    	s_ptr = (short *) ptr;
	    	c_ptr = ptr;
    	    	dc_ptr = (double_cplx *) ptr;
    	    	fc_ptr = (float_cplx *) ptr;
    	    	lc_ptr = (long_cplx *) ptr;
    	    	sc_ptr = (short_cplx *) ptr;
    	    	bc_ptr = (byte_cplx *) ptr;
	    }
	    else 
		l_ptr =  &(out_rec->tag);
	}
    }

    if (size != field_size) {
	while (size--)
	    switch (type) {
		case DOUBLE: 
		    *d_ptr++ = 0;
		    break;
		case FLOAT: 
		    *f_ptr++ = 0;
		    break;
		case LONG: 
		    *l_ptr++ = 0;
		    break;
		case SHORT: 
		    *s_ptr++ = 0;
		    break;
		case BYTE:
		    *c_ptr++ = 0;
		    break;
		case DOUBLE_CPLX:
		    dc_ptr->real = 0;
		    dc_ptr->imag = 0;
		    dc_ptr++;
		    break;
		case FLOAT_CPLX:
		    fc_ptr->real = 0;
		    fc_ptr->imag = 0;
		    fc_ptr++;
		    break;
		case LONG_CPLX:
		    lc_ptr->real = 0;
		    lc_ptr->imag = 0;
		    lc_ptr++;
		    break;
		case SHORT_CPLX:
		    sc_ptr->real = 0;
		    sc_ptr->imag = 0;
		    sc_ptr++;
		    break;
		case BYTE_CPLX:
		    bc_ptr->real = 0;
		    bc_ptr->imag = 0;
		    bc_ptr++;
		    break;
	    }
	if (new_file && !out_file)
	    put_fea_rec (out_rec, out_hd, out_strm);
	else
	if (get_fea_rec (in_rec, in_hd, in_strm) != EOF) {
	    copy_fea_rec (in_rec, in_hd, out_rec, out_hd,
		    (char **) NULL, (short **) NULL);
	    put_fea_rec (out_rec, out_hd, out_strm);
	}
	    size = field_size;
	    if(!add_tag) {
	    	d_ptr = (double *) ptr;
	    	f_ptr = (float *) ptr;
	    	l_ptr = (long *) ptr;
	    	s_ptr = (short *) ptr;
	    	c_ptr = ptr;
    	    	dc_ptr = (double_cplx *) ptr;
    	    	fc_ptr = (float_cplx *) ptr;
    	    	lc_ptr = (long_cplx *) ptr;
    	    	sc_ptr = (short_cplx *) ptr;
    	    	bc_ptr = (byte_cplx *) ptr;
	    }
	    else 
		l_ptr =  &(out_rec->tag);
    }


    if (out_file || update) {	/* get rest of input records */
	long    extra_count = 0;
	if (type == CHAR)
	    size = 1;
	while (size--)
	    switch (type) {
		case DOUBLE: 
		    *d_ptr++ = 0;
		    break;
		case FLOAT: 
		    *f_ptr++ = 0;
		    break;
		case LONG: 
		    *l_ptr++ = 0;
		    break;
		case SHORT: 
		    *s_ptr++ = 0;
		    break;
		case CHAR: 
		    if (field_size == 1)
			*ptr = ' ';
		    else
		    	strcpy (ptr, " ");
		    break;
		case BYTE:
		    *c_ptr++ = 0;
		    break;
		case DOUBLE_CPLX:
		    dc_ptr->real = 0;
		    dc_ptr->imag = 0;
		    dc_ptr++;
		    break;
		case FLOAT_CPLX:
		    fc_ptr->real = 0;
		    fc_ptr->imag = 0;
		    fc_ptr++;
		    break;
		case LONG_CPLX:
		    lc_ptr->real = 0;
		    lc_ptr->imag = 0;
		    lc_ptr++;
		    break;
		case SHORT_CPLX:
		    sc_ptr->real = 0;
		    sc_ptr->imag = 0;
		    sc_ptr++;
		    break;
		case BYTE_CPLX:
		    bc_ptr->real = 0;
		    bc_ptr->imag = 0;
		    bc_ptr++;
		    break;
	    }
	while (get_fea_rec (in_rec, in_hd, in_strm) != EOF) {
	    copy_fea_rec (in_rec, in_hd, out_rec, out_hd,
		    (char **) NULL, (short **) NULL);
	    put_fea_rec (out_rec, out_hd, out_strm);
	    extra_count++;
	}
	if (extra_count)
	    fprintf (stderr, "addfea: Warning, %ld extra records in %s were copied \naddfea:         to output file after ASCII input exhausted.\n",
		    extra_count, in_file);
    }

/* if updating the original file, we rewind the temp file and then
   copy the data over
*/
    if (update) {

	FILE * t_strm;

	rewind (out_strm);	/* rewind the temp file */
	fclose (in_strm);
	t_strm = out_strm;
	if ((out_strm = fopen (in_file, "w")) == NULL)
	    error2 ("cannot open input file for writing:", in_file, 0);
	else
	    while ((c = getc (t_strm)) != EOF)
		putc (c, out_strm);
    }

    exit(0);

}
