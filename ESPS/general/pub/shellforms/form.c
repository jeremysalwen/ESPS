/* Last update: 02/08/88  00:18 AM  (Edition: 56) */
#include	<stdio.h>
#include	<ctype.h>
#include	<signal.h>
#include	"form.h"
#include	"field.h"
#include	"term.h"
#include	"basic.h"

extern  int    erase_screen;
extern  int    num_lines;
char	Screen [SCRLINE][SCRCOL+1];
char	Form_name[20] = "";		/* current form name */
char	Exitc;				/* field exit char */
 
char	Enter_mode [20];		/* display before input field */
char	Exit_mode [20];			/* display after input field */

struct	field	Field[MAXFIELD];	/* field nodes */
int		Fno;			/* next field to use (or count) */
unsigned char	Fhead[SCRLINE];		/* 1st field index on each line */
unsigned char	Flcount[SCRLINE];	/* number of fields on this line */
unsigned char	Lastdata = 24;		/* last line with data on form */

char	Varfile [MAXFNAME];		/* output variable file name */

extern	int		Form_msg;	/* defined in msg.c */
extern	unsigned char	Shell;		/* output shell script type */

int	sig_interrupt();
int	sig_stop();

/*----------------------------------------------------------------------+
|									|
|		show_form : display a given form file			|
|									|
+----------------------------------------------------------------------*/
show_form (fname)
char	*fname;		/* name of form file, NULL = stdin, 1 = redisplay */
	{
	int		i, j;
	int		idx;		/* index to current field */
	unsigned	size;
	unsigned	off;
	unsigned	len;		/* string length */
	int		pend;		/* previous end position */
	char		*lp;		/* line pointer (for speed) */
	char		*p;
	extern	char	*Prgname;

	ENTER(show_form);
	if (fname != (char *)1 &&		/* redisplay */
	    (fname == NULL || strcmp (fname, Form_name) != 0)) {
		if (!load_form (fname)) RETURN (0);
		}

	if (erase_screen)
	  screen (SCR_ERASE);
	for (i=0; i<num_lines; i++) {
		p = lp = Screen[i];
		if (Flcount[i] != 0) {
			pend = 0;
			for (j=0, idx=Fhead[i]; j<Flcount[i]; j++, idx++) {
				size = Field[idx].f_len;
				off = Field[idx].f_off;
				put_string (p, off-pend);
				put_string (Enter_mode, 0); 
				put_string (lp + off, size);
				put_string (Exit_mode, 0);
				p = lp + off + size;
				pend = off + size;
				}
			}
		if ((len = strlen (p)) == 0) continue;
		put_string (p, len);
		put_char ('\r');
		}
	RETURN (1);
	}

/*----------------------------------------------------------------------+
|									|
|	   edit_form : edit a form on all the input fields		|
|									|
+----------------------------------------------------------------------*/
edit_form (fname, action)
char	*fname;		/* name of the form to edit */
int	(*action) ();	/* action routine to check at end of each field */
	{
	unsigned	fdx = 0;	/* current cursor in field index */
	struct	field	*fp;		/* pointer to current edit field */
	int		n;		/* action routine return status */
	int		efopt;		/* option to edit field */

	ENTER(edit_form);
	signal (SIGINT, sig_interrupt);
	signal (SIGTSTP, sig_stop);

	term_init ();
	cbreakio (1);
	if (!show_form (fname)) goto end;
	if (Fno == 0) goto end;
 
	form_msg ((char *)NULL, 1, 1);
	screen (SCR_KEYXMIT);
	while (1) {
		fp = &Field[fdx];
		efopt = EF_BOF;
		do	{
			if (fp->f_attr & FA_SELECTION) sel_field (fdx);
			else edit_field (fdx, efopt);
			if (!action) break;
			n = (*action) (fdx, &Screen[fp->f_line-1][fp->f_off],
					fp->f_len, Exitc);
			switch (n) {
				when EF_ERR:	efopt = EF_BOF;
				when EF_FILL:	efopt = EF_FILL;
				}
			} while (n != EF_OK);
 
		if ((Exitc & 0x80) == 0x80) {
			switch (Exitc & 0x7f) {
				when KU:	Exitc = CTRL('P');
				when KD:	Exitc = CTRL('N');
				}
			}
		switch (Exitc) {
			when CTRL('L'):	show_form ((char *)1);
			when CTRL('X'): goto wrout;
			when CTRL('P'):	fdx = closest (fdx, -1);
			when CTRL('N'):	fdx = closest (fdx, 1);
			when CTRL('M'):	fdx = closest (fdx, 1);
			when CTRL('J'):	fdx = closest (fdx, 1);
			when CTRL('T'): fdx = (fdx == 0) ? Fno-1 : (fdx-1);
			when CTRL('I'): if (++fdx == Fno) fdx = 0;
			otherwise:	show_summary ();
					show_form ((char *)1);
			}
		}
wrout:	write_var ();
	poscur (Lastdata, (unsigned char)1, "\n"); screen (SCR_EEOL);
end:
	screen (SCR_NOKEYXMIT);
	cbreakio (0);
	term_close ();
	EXIT;
	}

/*-------------------------------------------------------------05/08/86-+
|									|
|	       write_var : write field variables to a file		|
|									|
+----------------------------------------------------------------------*/
write_var ()
	{
	register int	i;
	FILE		*fd;		/* output file pointer */
	char		*p;
	char		*op;
	char		c;
	char		buf[80+1];
	char		obuf[80+1];
	struct	field	*f;

	ENTER(write_var);
	if (!strlen (Varfile)) EXIT;

	if ((fd = fopen (Varfile, "w")) == NULL) {
		fprintf (stderr, "write_var: can not open file %s for write\r\n",
			 Varfile);
		exit (1);
		}

	for (i=0; i<Fno; i++) {
		f = &Field[i];
		if (f-> f_var != NULL) {
			get_field (i, buf);
			p = buf;  op = obuf;
			while (c = *p++) {
				switch (c) {
				    when '\'':	if (Shell== PERL) *op++ = '\\';
						else	{
							*op++ ='\'';*op++ ='"';
							*op++ ='\'';*op++ ='"';
							}
				    when '!':	if (Shell == CSH) *op++ = '\\';
				    }
				*op++ = c;
				}
			*op = EOS;
			fprintf (fd, "%s%s='%s'%s\n",
				 (Shell==CSH)?"set ":((Shell==PERL)?"$":""),
				 f-> f_var, obuf,
				 (Shell == PERL) ? ";" : "");
			}
		}
	fclose (fd);
	EXIT;
	}

/*-------------------------------------------------------------01/12/88-+
|									|
|		 sig_interrupt : Interrupt signal handler		|
|									|
+----------------------------------------------------------------------*/
sig_interrupt ()
	{
	ENTER (sig_interrupt);
	form_msg ("Aborted by user\n", 24+1, 1);
	screen (SCR_NORMAL);
	cbreakio (0);
	exit (1);
	}

/*-------------------------------------------------------------01/12/88-+
|									|
|		      sig_stop : Stop signal handler			|
|									|
+----------------------------------------------------------------------*/
sig_stop ()
	{
	ENTER (sig_stop);
	form_msg ("stopped by user\n", 24+1, 1);
	screen (SCR_NORMAL);
	cbreakio (0);
	kill (getpid(), SIGSTOP);
	put_string ("Press CTRL L to refresh display: ", 33);
	cbreakio (1);
	}