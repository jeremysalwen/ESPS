/* Last update: 01/27/88  00:04 AM  (Edition: 45) */
#include	<stdio.h>
#include	<sys/ioctl.h>
#include	<ctype.h>
#include	<strings.h>
#include	"term.h"
#include	"form.h"
#include	"field.h"
#include	"basic.h"

int	Help_display = NO;		/* if YES, display help */

extern	struct	field	Field[];
extern	char		Enter_mode[];
extern	char		Exit_mode[];
extern	int		Form_msg;
extern	char		Exitc;
extern	char		*malloc();
extern	char		*calloc();
/*
    Selection buffer looks like:

	<delchr> string1 <delchr> string2 <delchr> ... stringn <delchr>

    The first character in the buffer will be used as the delimiter to
    separate selection strings.  The last delimiter character is optional,
    i.e., EOS will also terminate the last selection string.
*/

/*-------------------------------------------------------------08/01/87-+
|									|
|     build_selection : take selection definition and create a list	|
|									|
+----------------------------------------------------------------------*/
build_selection (fldp, buf, len)
struct	field	*fldp;			/* ptr to field structure */
char		*buf;			/* selection strings */
unsigned	len;			/* length of string */
	{
	register char	*p = (char *)malloc(len+1);
	register char	**tp;
	register int	i;
	char		delimiter = buf[0];	/* selection delimiter */
	char		c;
	unsigned	cnt = 0;		/* # of selections */
	char		*tmp[50];		/* temp array */

	ENTER(build_selection);
	if (len == 0) len = strlen (buf);
	while (len-- != 0) {
		if ((c = *buf++) == delimiter) {
			*p++ = EOS;
			if (cnt >= 50) {
				/* silent ignored extra for now */
				}
			else tmp[cnt++] = p;
			}
		else	*p++ = c;
		}
	fldp->f_sel = tp = (char **) malloc (sizeof (char *) * (cnt + 1));
	for (i=0; i<cnt; i++) *tp++ = tmp[i];
	*tp = NULL;
	fldp->f_attr |= FA_SELECTION;
	RETURN (0);
	}

/*-------------------------------------------------------------08/01/87-+
|									|
|	sel_num : return the number of selections in a given field	|
|									|
+----------------------------------------------------------------------*/
sel_num (fldp)
struct	field	*fldp;
	{
	register int	i = 0;
	register char	**pp = fldp-> f_sel;

	ENTER(sel_num);
	while (*pp++ != NULL) i++;
	RETURN (i);
	}

/*-------------------------------------------------------------08/01/87-+
|									|
|	sel_field : allow user to select a selection from the list	|
|									|
+----------------------------------------------------------------------*/
sel_field (idx)
unsigned	idx;			/* index to Field array */
	{
	struct	field	*fldp = &Field[idx];
	int		n = fldp-> f_sno;	/* selection number */
	int		selno;			/* # of possible selections */
	int		i = -1;
	char		c;

	ENTER(sel_field);
	selno = sel_num (fldp);
	poscur (fldp-> f_line, fldp-> f_col, Enter_mode);
	upd_field (idx, UF_STRING, fldp-> f_sel[n]);
	while (1) {
		int	might_tab;
		if (Help_display) display_help (fldp, n);
		c = getkey ();
		might_tab = 0;
		if (Form_msg) {
			form_msg ((char *)NULL, fldp->f_line, fldp->f_col);
			put_string (Enter_mode, 0);
			}
		if ((c & 0x80) == 0x80) {
			switch (c & 0x7f) {
				case KL:	goto prev_sel;
				case KR:	goto next_sel;
				}
			}
		switch (c) {
			case ' ':
			case CTRL('F'):
next_sel:			if (++n >= selno) n = 0;  break;
			case CTRL('H'):
prev_sel:			if (--n < 0) n = selno-1; break;
			case '?':
				if (!Help_display) {
					display_help (fldp, n);
					break;
					}
			default:if (c < LOW_GCHAR || c > HIGH_GCHAR) {
					Exitc = c;
					goto eoe;
					}
				i = cmp1st (c, fldp->f_sel, i+1);
				if (i < 0) continue;
				might_tab = 1;
				n = i;
			}
		upd_field (idx, UF_STRING, fldp-> f_sel[n]);
		if (might_tab && (fldp->f_attr & FA_AUTOTAB) != 0) {
			Exitc = TAB;
			break;
			}
		}
eoe:	put_string (Exit_mode, 0);
	fldp-> f_sno = n;		/* save selection number */
	EXIT;
	}	

/*-------------------------------------------------------------08/03/87-+
|									|
|	   cmp1st : compare 1st character in a keyword list		|
|									|
+----------------------------------------------------------------------*/
cmp1st (c, list, start)
char	c;				/* char to compare */
char	**list;				/* keyword list */
int	start;				/* start entry index */
	{
	register int	i, j;
	register int	n;

	ENTER(cmp1st);
	if (list == NULL || list[0] == NULL) RETURN (-1);
	for (n=0; list[n] != NULL; n++);
	for (i=start, j=0; j<n; i++, j++) {
		if (i >= n) i = 0;
		if ((c | 0x20) == (list[i][0] | 0x20)) RETURN (i);
		}
	RETURN (-1);			/* no match */
	}

/*-------------------------------------------------------------08/04/87-+
|									|
|		build_help_msg : build help message list		|
|									|
+----------------------------------------------------------------------*/
build_help_msg (fldp, buf)
struct	field	*fldp;			/* field where the msg belongs */
char		*buf;			/* message stores here */
	{
	unsigned	len = strlen (buf);
	unsigned	n;
	register int	i;
	register char	*p;

	ENTER(build_help_msg);
	n = sel_num (fldp);
	if (fldp-> f_help == NULL) {		/* first time call */
		fldp-> f_help = (char **) calloc(n+1, sizeof(char *));
		}
	for (i=0; i<n; i++) {
		if (fldp-> f_help[i] == NULL) break;
		}
	if (i < n) {
		fldp-> f_help[i] = p = (char *) malloc (len + 1);
		strcpy (p, buf);
		}
	EXIT;
	}

/*-------------------------------------------------------------08/04/87-+
|									|
|	   display_help : display help message on message line		|
|									|
+----------------------------------------------------------------------*/
display_help (fldp, i)
struct	field	*fldp;			/* field pointer */
int		i;			/* index to help message */
	{
	char		*p;
	int		numchar;	/* # of pending input chars */

	ENTER(display_help);
	if (fldp-> f_help == NULL) EXIT;
	p = fldp-> f_help[i];
	if (p != NULL) {
		ioctl (0, FIONREAD, &numchar);
		if (numchar == 0) form_msg (p, fldp->f_line, fldp->f_col);
		}
	EXIT;
	}
