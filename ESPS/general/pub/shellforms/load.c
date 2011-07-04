/* Last update: 01/26/88  10:53 PM  (Edition: 63) */
#include	<stdio.h>
#include	<strings.h>
#include	"form.h"
#include	"field.h"
#include	"basic.h"

#define	TOKENSIZE	256

char	Varname [MAXVNAME];		/* variable name string pool */
char	*Varwp = Varname;		/* next write position */

extern  int		num_lines;
extern	char		Screen[SCRLINE][SCRCOL+1];
extern	char		Form_name[];
extern	unsigned char	Fhead[];	/* 1st field index on each line */
extern	unsigned char	Flcount[];	/* number of fields on this line */
extern	unsigned char	Lastdata;
extern	struct	field	Field[];	/* field nodes */
extern	int		Fno;		/* next field to use (or count) */

/*----------------------------------------------------------------------+
|									|
|		load_form : load a form from disk file			|
|									|
+----------------------------------------------------------------------*/
load_form (fname)
char	*fname;
	{
	FILE			*fp;
	register unsigned char	i;

	ENTER (load_form);
	if (fname == NULL) fp = stdin;
	else if ((fp = fopen (fname, "r")) == 0) {
		fprintf (stderr, "Can not open form file %s\r\n", fname);
		RETURN (0);
		}
	for (i=0; i<num_lines; i++) Screen[i][0] = '\0';
	init_field ();
	for (i=0; i<num_lines; i++) {
		if (fgets (Screen[i], SCRCOL+1, fp) == 0) break;
		if (Screen[i][0] == CTRL('L')) {
			Screen[i][0] = EOS;
			field_attr (fp);
			break;
			}
		make_field (i+1, Screen[i]);
		}
	init_sno ();		/* fill data for field with selections */
	Lastdata = i;
	if (fname != NULL) {
		strcpy (Form_name, fname);
		}
	RETURN (1);
	}

/*----------------------------------------------------------------------+
|									|
|    bcopy : copy len byte from src to dest (extra fill with blanks)	|
|									|
+----------------------------------------------------------------------*/
bcopy (dest, src, len)
char		*dest;
char		*src;
unsigned	len;
	{
	char		c = ' ';

	ENTER (bcopy);
	while (len-- != 0) {
		if (c == EOS) *dest++ = ' ';	/* fill blanks to end */
		else if ((c = *src++) != EOS)
			*dest++ = c;
		else	*dest++ = ' ';
		}
	EXIT;
	}

/*-------------------------------------------------------------05/08/86-+
|									|
|	   add_var : add a variable name to field structure		|
|									|
+----------------------------------------------------------------------*/
add_var (fno, name)
int	fno;
char	*name;
	{
	int		len = strlen (name) + 1;

	ENTER (add_var);
	if (Varwp + len >= &Varname[MAXVNAME]) {
		fprintf (stderr, "add_var: no more space for variable name\r\n");
		exit (1);
		}
	Field[fno].f_var = Varwp;
	strcpy (Varwp, name);
	Varwp += len;
	EXIT;
	}

/*-------------------------------------------------------------05/08/86-+
|									|
|	  field_attr : read in field attribute of the form		|
|									|
+----------------------------------------------------------------------*/
field_attr (fd)
FILE	*fd;
	{
	int		field = -1;
	char		tokbuf [TOKENSIZE];	/* token value */
	char		token;			/* token specifier */
	int		i, n;
	unsigned	len, size;
	struct	field	*f;
	char		*p;

	ENTER (field_attr);
	while (token = get_token (fd, tokbuf)) {
		switch (token) {
			when 'v': if (++field >= Fno) RETURN (0);
				  add_var (field, tokbuf);
			when 's': n = (field < 0) ? 0 : field;
				  f = &Field[n];
				  size = strlen(tokbuf);
				  build_selection (f, tokbuf, size);
			when 'h': n = (field < 0) ? 0 : field;
				  f = &Field[n];
				  build_help_msg (f, tokbuf);
			when 'd': n = (field < 0) ? 0 : field;
				  f = &Field[n];
				  size = strlen(tokbuf);
				  len = (size > f-> f_len) ? f-> f_len : size;
				  for (p=tokbuf, i=0; i<len; i++) {
					  Screen[f-> f_line - 1][f-> f_off + i] = *p++;
					  }
			when 'a': n = (field < 0) ? 0 : field;
				  f = &Field[n];
				  set_field_attr (f, tokbuf);
			otherwise:fprintf (stderr, "unknown token %c (field %d)\r\n",
					   token, field);	
			}
		}
	RETURN (1);
	}

/*-------------------------------------------------------------05/08/86-+
|									|
|     get_token : return next token in field description section	|
|									|
+----------------------------------------------------------------------*/
get_token (fd, tokbuf)
FILE	*fd;
char	*tokbuf;
	{
	static	char	buf[TOKENSIZE];
	static	char	*p;
	static	int	len = 0;
	static	char	dchar;
	register char	*tp;		/* temp pointer */
	char		token;

	ENTER (get_token);
	if (len == 0) {
		do	{
			if (!fgets (buf, sizeof (buf), fd)) RETURN ((char)0);
			len = strlen (buf) - 1;		/* no NL */
			} while (len == 0);
		dchar = buf[0];		/* delimiter char */
		p = buf + 1;		/* 1st char */
		}

	for (tp=p; tp < &buf[len]; ) {
		if (*tp++ == dchar) break;
		}

	token = *p++;			/* return token char */
	p++;				/* skip =, actually ignore it */
	while (p < tp-1)
		*tokbuf++ = *p++;
	*tokbuf = EOS;

	if ((p = tp) == &buf[len]) len = 0;
	RETURN (token);
	}
 
/*----------------------------------------------------------------------+
|									|
|	make_field : extract input fields to make field structure	|
|									|
+----------------------------------------------------------------------*/
make_field (lno, line)
unsigned	lno;		/* line number of this line */
char		*line;
	{
	char		*p;
	unsigned	col = 1;	/* current column position */
	unsigned	in_field = 0;	/* number of input field char */
	unsigned	cno;

	ENTER (make_field);
	for (p=line; *p != EOS; p++) {
		switch (*p) {
			case '~':	 /* input field */
				*p = ' ';
				if (in_field++ == 0) cno = col;
				col++;
				break;
			case '\t':
				col = ((col-1)/8 + 1) * 8 + 1;
				goto lab;
			default:
				col++;
lab:				if (in_field)
					add_field (lno, cno, p-line-in_field,
						   in_field);
				in_field = 0;
			}
		}
	EXIT;
	}

/*----------------------------------------------------------------------+
|									|
|	init_field : initialize all field control related data		|
|									|
+----------------------------------------------------------------------*/
init_field ()
	{
	int		i;

	ENTER (init_field);
	Fno = 0;
	for (i=0; i<num_lines; i++) {
		Fhead[i] = 0;
		Flcount[i] = 0;
		}
	EXIT;
	}
 
/*----------------------------------------------------------------------+
|									|
|	    add_field : add a given field to field structure		|
|									|
+----------------------------------------------------------------------*/
add_field (line, column, offset, len)
unsigned	line;		/* line of the field */
unsigned	column;		/* column of the field */
unsigned	offset;		/* char offset from beginning of the line */
unsigned	len;		/* field size */
	{
	struct	field	*fp;

	ENTER (add_field);
	if (Fno >= MAXFIELD) {
		fprintf (stderr, "Too many fields in a screen max=%d\r\n",
			 MAXFIELD);
		exit (1);
		}
	fp = &Field[Fno];
	fp-> f_line = line;
	fp-> f_col = column;
	fp-> f_off = offset;
	fp-> f_len = len;
	fp-> f_attr = 0;		/* 'a' -- attribute */
	fp-> f_sel = NULL;		/* 's' -- selection */
	fp-> f_help = NULL;		/* 'h' -- help message */
	fp-> f_sno = 0;
	fp-> f_var = NULL;
 
	if (Flcount[line-1]++ == 0) {
		Fhead[line-1] = Fno;
		}
	Fno++;
	EXIT;
	}

#define	STRNEQ(x,y,n)	((*(x) == *(y)) && strncmp (x,y,n) == 0)
#define	BADINIT	"Field %d, Init value: [%.*s] no in selection list, ignored\n"
/*-------------------------------------------------------------01/11/88-+
|									|
|       init_sno : initialize selection number for preloaded fields	|
|									|
+----------------------------------------------------------------------*/
init_sno ()
	{
	register int	idx, j;
	int		field_count;
	int		init_size;	/* initial default data size */
	struct	field	*f;
	char		*p;

	ENTER (init_sno);
	for (idx=0; idx<Fno; idx++) {

	    f = &Field[idx];
	    p = &Screen[f-> f_line - 1][f-> f_off];
	    init_size = fldlen (p, f-> f_len);

	    if (f-> f_sel != NULL) {
		if (init_size) {
			field_count = sel_num (f);
			/* check if default data is in the selection list */
			for (j=0; j<field_count; j++) {
				register char	*selp;
				selp = f-> f_sel[j];
				if (STRNEQ (p, selp, strlen(selp))) {
					f-> f_sno = j;
					break;
					}
				}
			if (j >= field_count) {
				fprintf (stderr, BADINIT, idx, init_size, p);
				j = 0;  goto mv_1st;
				}
			}
		else	{
			/* move the first selection into the field */
			j = 0;
mv_1st:			strncpy (p, f-> f_sel[j], strlen(f-> f_sel[j]));
			}
		}
	    }
	EXIT;
	}

/*-------------------------------------------------------------01/26/88-+
|									|
|	    set_field_attr : set attributes for a given field		|
|									|
+----------------------------------------------------------------------*/
set_field_attr (fieldp, buf)
struct	field	*fieldp;
char		*buf;
	{
	char	c;

	ENTER (set_field_attr);
	while ((c = *buf++) != EOS) {
		switch (c) {
		    when 'd':	fieldp-> f_attr |= FA_NUMERIC;
		    when 'a':	fieldp-> f_attr |= FA_AUTOTAB;
		    when 'b':	fieldp-> f_attr |= FA_BLOCK;
		    otherwise:	fprintf (stderr,
					 "unknown field attribute: %c\r\n", c);
				exit (1);
		    }
		}
	EXIT;
	}
