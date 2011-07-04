/* Last update: 01/13/88  11:26 AM  (Edition: 5) */
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/file.h>

#ifndef		_SGTTYB_
#include	<sgtty.h>
#endif

int		Term_input;
int		Term_output;
extern	int	Debug;
/*-------------------------------------------------------------05/07/86-+
|									|
|	    put_string : write out a string to Term_output		|
|									|
+----------------------------------------------------------------------*/
put_string (s, len)
char		*s;
unsigned	len;
	{
	if (len == 0) len = strlen (s);
	if (Debug) {
		register int	i;
		register char	*p = s;

		for (i=0; i<len; i++) put_char (*p++);
		}
	else write (Term_output, s, (int)len);
	}

/*-------------------------------------------------------------05/07/86-+
|									|
|	    put_char : write out one char to Term_output		|
|									|
+----------------------------------------------------------------------*/
put_char (c)
char	c;
	{
	if (Debug) {
		if (isprint(c)) write (Term_output, &c, 1);
		else	{
			char	buf [2];
			buf[0] = (c < ' ') ? '^' : '~';
			buf[1] = (c | 0x40) & 0x5f;
			write (Term_output, buf, 2);
			}
		}
	else write (Term_output, &c, 1);
	}

static	char	Pushbuf [80];
static	int	Pushi = 0;
/*-------------------------------------------------------------05/07/86-+
|									|
|		get_char : get a char from Term_input			|
|									|
+----------------------------------------------------------------------*/
char
get_char ()
	{
	char		c;
	register int	i;

	if (Pushi > 0) {		/* push buffer not empty */
		c = Pushbuf[0];
		Pushi--;
		for (i=0; i<Pushi; i++) {
			Pushbuf[i] = Pushbuf[i+1];
			}
		return (c);
		}
	read (Term_input, &c, 1);
	c &= 0x7f;			/* make it ASCII */
	return (c);
	}

pushback (buf, len)
char	*buf;
int	len;
	{
	register char	*p = &Pushbuf [Pushi];

	Pushi += len;
	while (len-- > 0) {
		*p++ = *buf++;
		}
	}

static	int		Old_flags;
static	struct	sgttyb	Mode_tty;
/*-------------------------------------------------------------05/07/86-+
|									|
|		 cbreakio : enter/exit cbreak I/O mode			|
|									|
+----------------------------------------------------------------------*/
cbreakio (n)
int	n;				/* 1 -- enter, 0 -- exit */
	{
	static	int	cbreak_io = 0;
	int		ostate;

	ostate = cbreak_io;
	if (n) {
		if (!cbreak_io) {
			open_tty ();
			cbreak_io = 1;
			}
		}
	else	{
		if (cbreak_io) {
			close_tty ();
			cbreak_io = 0;
			}
		}
	return (ostate);
	}

/*-------------------------------------------------------------01/12/88-+
|									|
|		term_init : open terminal for read/write		|
|									|
+----------------------------------------------------------------------*/
term_init ()
	{
	extern	char	*Prgname;

	if ((Term_input = open ("/dev/tty", O_RDONLY)) < 0) {
		perror ("term_init: open(/dev/tty,r)");
		exit (1);
		}
	if ((Term_output = open ("/dev/tty", O_WRONLY)) < 0) {
		perror ("term_init: open(/dev/tty,r)");
		exit (1);
		}

	if (!isatty(Term_input) || !isatty(Term_output)) {
		fprintf (stderr, "You have to run %s interactively\n",
			 Prgname);
		exit (1);
		}
	}

/*-------------------------------------------------------------01/12/88-+
|									|
|		 term_close : close terminal descriptors		|
|									|
+----------------------------------------------------------------------*/
term_close ()
	{
	close (Term_input);
	close (Term_output);
	}
/*------------------------------------------------------------07/10/84--+
|									|
|	 open_tty : open terminal in CBREAK mode without ECHO		|
|									|
+----------------------------------------------------------------------*/
static	open_tty ()
	{
	gtty (Term_input, &Mode_tty);
	Old_flags = Mode_tty.sg_flags;		/* save old setting */
#ifdef hpux
	Mode_tty.sg_flags |= RAW;
#else
	Mode_tty.sg_flags |= CBREAK;
#endif
	Mode_tty.sg_flags &= ~(ECHO | CRMOD);
	stty (Term_input, &Mode_tty);
	}

/*------------------------------------------------------------07/10/84--+
|									|
|	close_tty : close terminal and restore original setting		|
|									|
+----------------------------------------------------------------------*/
static	close_tty ()
	{
	Mode_tty.sg_flags = Old_flags;
	stty (Term_input, &Mode_tty);		/* restore original setting */
	}
