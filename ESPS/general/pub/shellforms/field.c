/* Last update: 01/26/88  11:26 PM  (Edition: 25) */
#include	<stdio.h>
#include	<signal.h>
#include	<strings.h>
#include	<ctype.h>
#include	"term.h"
#include	"form.h"
#include	"field.h"
#include	"basic.h"

extern	int		Fno;
extern	struct	field	Field[];
extern	char		Screen[SCRLINE][SCRCOL+1];
extern	char		Enter_mode[];
extern	char		Exit_mode[];
extern	int		Form_msg;
extern	char		Exitc;

/*-------------------------------------------------------------05/09/86-+
|									|
|	      get_field : get a copy of field data to buffer		|
|									|
+----------------------------------------------------------------------*/
get_field (fno, buf)
int	fno;				/* field number */
char	*buf;				/* buffer to be filled */
	{
	struct	field	*f;
	char		*p;
	int		size;

	ENTER(get_field);
	if (fno < 0 || fno >= Fno) EXIT;
	f = &Field[fno];
	p = &Screen [f-> f_line -1][f-> f_off];
	size = fldlen (p, f-> f_len);
	strncpy (buf, p, size);
	buf[size] = EOS;		/* mark the end */
	EXIT;
	}
/*----------------------------------------------------------------------+
|									|
|  fldlen : return the number of data in a field (skip trailing blanks) |
|									|
+----------------------------------------------------------------------*/
fldlen (buf, size)
char		*buf;		/* pointer to field buffer */
unsigned	size;		/* size of the field */
	{
	int		blank;

	ENTER(fldlen);
	for (blank=0; buf[size-1-blank] == ' '; blank++) {
		if (blank >= size) break;
		}
	RETURN (size - blank);		/* return length of field */
	}

/*----------------------------------------------------------------------+
|									|
|	  upd_field : move data into a field and display it		|
|									|
+----------------------------------------------------------------------*/
upd_field (idx, type, data)
unsigned	idx;		/* field number (0 to Fno-1) */
int		type;		/* type of output */
int		data;		/* data, type may vary */
	{
	struct	field	*fp;
	char		*fmt;
	char		*scp;		/* pointer to screen buffer */
	char		buf[81];
	int		data2;		/* cents */

	ENTER(upd_field);
	if (idx >= Fno) {
		fprintf (stderr,
			 "Field number: %d out of range, max %d\r\n",
			 idx, Fno);
		EXIT;
		}
 
	fp = &Field[idx];
	switch (type) {
		case UF_MONEY:	fmt = "%5d.%02d";
				data2 = data % 100;
				data = data / 100;	break;
		case UF_NUMBER: fmt = "%d";		break;
		case UF_STRING:
		default:	fmt = "%s";		break;
		}
 
	sprintf (buf, fmt, data, data2);
	scp = &Screen[fp-> f_line - 1][fp-> f_off];
	bcopy (scp, buf, (unsigned)fp-> f_len);
	poscur (fp-> f_line, fp-> f_col, (char *)NULL);
	put_string (scp, fp-> f_len);
	poscur (fp-> f_line, fp-> f_col, (char *)NULL);
	EXIT;
	}

/*-------------------------------------------------------------07/14/87-+
|									|
|	closest : find the closest field above/below current line	|
|									|
+----------------------------------------------------------------------*/
closest (fno, direction)
unsigned	fno;		/* current field number */
int		direction;	/* compare direction: 1=below, -1=above */
	{
	struct	field	*fp = &Field[fno];
	int		col;		/* target column */
	int		line;		/* current line */
	int		diff;		/* current difference */
	int		mindiff = 100;	/* minimal difference */
	int		mini = 0;	/* minimal field index */
	register int	i;

	ENTER(closest);
	col = fp->f_col;
	line = fp->f_line;

	for (i=fno+direction; i != fno; i += direction) {
		if (i < 0) i = Fno-1;
		else if (i >= Fno) i = 0;
		fp = &Field[i];
		if (fp->f_line != line) break;
		}
	mini = i;
	for (line = fp->f_line; ; i += direction) {
		if (i < 0) i = Fno-1;
		else if (i >= Fno) i = 0;
		fp = &Field[i];
		if (fp->f_line != line) break;
		if ((diff = abs ((int)fp->f_col - (int)col)) == 0) RETURN (i);
		if (diff < mindiff) {
			mindiff = diff;
			mini = i;
			}
		}
	RETURN (mini);
	}

/*----------------------------------------------------------------------+
|									|
|		edit_field : edit a given field in a form		|
|									|
+----------------------------------------------------------------------*/
edit_field (idx, efopt)
unsigned	idx;			/* index to Field array */
int		efopt;			/* option */
	{
	struct	field	*fp;
	char		c;
	char		*p;
	char		*s;		/* beginning of field buffer */
	unsigned	n;
	register int	i;

	ENTER (edit_field);
	fp = &Field[idx];
	poscur (fp-> f_line, fp-> f_col, Enter_mode);
	p = s = &Screen [fp-> f_line - 1][fp-> f_off];
	switch (efopt) {
		case EF_FILL:
		case EF_EOF:	n = fldlen (s, fp-> f_len);
				if (n != 0) {
					if (efopt == EF_FILL)
						put_string (p, n);
					p += n;
					}
				break;
		default:	;	/* start at beginning of field */
		}
	while (1) {
		c = getkey ();
		if (Form_msg) {
			form_msg ((char *)NULL, fp-> f_line, fp-> f_col + (p-s));
			put_string (Enter_mode, 0);
			}
		if ((c & 0x80) == 0x80) {
			switch (c & 0x7f) {
				case KL:	goto cur_left;
				case KR:	goto cur_right;
				}
			}
		switch (c) {
			case '\177':		/* delete char */
				if (p > s) { *(--p) = ' '; screen (SCR_DEL); }
				continue;
			case CTRL('W'):		/* delete prev word */
				while (p > s) {
					if (*(p-1) != ' ') break;
					p--; screen (SCR_DEL);
					}
				while (p > s) {
					if (*(p-1) == ' ') break;
					*(--p) = ' ';
					screen (SCR_DEL);
					}
				continue;
			case CTRL('K'):
				n = fldlen (s, fp-> f_len);
				for (i=0; i<n; i++) put_char (*(p+i) = ' ');
				for (i=0; i<n; i++) screen (SCR_BACKSPACE);
				continue;
			case CTRL('H'):
			case CTRL('B'):
cur_left:			if (p > s) { p--; screen (SCR_BACKSPACE); }
				continue;
			case CTRL('F'):
cur_right:			if (p < s + fp-> f_len) {
					put_char (*p++);
					}
				continue;
			case CTRL('G'):
			case CTRL('U'):
				while (p > s) {
					*(--p) = ' ';  screen (SCR_DEL);
					}
				continue;
			case CTRL('E'):		/* goto end of field */
				n = fldlen (s, fp-> f_len);
				p = s + n;
				poscur (fp-> f_line, fp-> f_col + n, (char *)NULL);
				continue;
			case CTRL('A'):		/* emacs style */
				p = s;
				poscur (fp-> f_line, fp-> f_col, (char *)NULL);
				continue;
			case CTRL('O'):
				prev_msg ();
				continue;
			default:
				if ((fp-> f_attr & FA_NUMERIC) != 0 &&
				     isprint (c) && !isdigit (c)) {
					put_char (CTRL('G'));
					form_msg ("Numeric field, 0-9 only",
						  fp-> f_line,
						  fp-> f_col + (p-s));
					continue;
					}
				if (c < LOW_GCHAR || c > HIGH_GCHAR) {
					Exitc = c;	/* save exit char */
					goto eoe;
					}
			}
		if (p - s >= fp-> f_len) { put_char (CTRL('G')); continue; }
		*p++ = c;
		put_char (c);
		if ((fp-> f_attr & FA_AUTOTAB) != 0 && p - s == fp-> f_len) {
			Exitc = TAB;		/* tab to next field */
			break;
			}
		}
	/* *p = EOS will put a marker at the end of buffer */
eoe:	put_string (Exit_mode, 0);
	RETURN (p-s);
	}
