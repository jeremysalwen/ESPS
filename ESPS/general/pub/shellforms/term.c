/* Last update: 01/13/88  11:16 AM  (Edition: 9) */
#include	<stdio.h>
#include	<ctype.h>
#include	<strings.h>
#include	"form.h"

#define	YES	(1)
#define	NO	(0)
#define	EOS	'\0'
#define	when		break; case
#define	otherwise	break;default

static	char	*Erase = "\010 \010";	/* erase string */

struct	tcap	{
	char		*tc_id;		/* key for capability	*/
	char		*tc_str;	/* ptr to Tc_str area	*/
	unsigned char	tc_delay;	/* # of msec to delay	*/
	unsigned char	tc_len;		/* length of tc_str	*/
	};

static	char	Termcap [1024];
static	char	Tstr [1024];		/* buffer for real escape sequence */
static	char	*Tsp = Tstr;		/* pointer to be used by tgetstr */
static	int	Tcap_count = 0;		/* # of entries extracted */

/*-------- You may want to modify the following (also term.h) ----------*/
#include	"term.h"

static	struct	tcap Tcap [] = {
		{ "bc",	0, NULL, 0 },	/* cursor backspace		*/
		{ "cm",	0, NULL, 0 },	/* cursor motion		*/
		{ "cl", 0, NULL, 0 },	/* clear entire screen		*/
		{ "cd", 0, NULL, 0 },	/* clear to end of display	*/
		{ "ce", 0, NULL, 0 },	/* clear to end of line		*/
		{ "ho", 0, NULL, 0 },	/* home cursor			*/
		{ "ks", 0, NULL, 0 },	/* start keypad xmit mode	*/
		{ "ke", 0, NULL, 0 },	/* end keypad xmit mode		*/
		{ "ku", 0, NULL, 0 },	/* (input) cursor upper		*/
		{ "kd", 0, NULL, 0 },	/* (input) cursor down		*/
		{ "kl", 0, NULL, 0 },	/* (input) cursor left		*/
		{ "kr", 0, NULL, 0 },	/* (input) cursor right		*/
		{ "md", 0, NULL, 0 },	/* mode dim (or highlite)	*/
		{ "me", 0, NULL, 0 },	/* mode end (return to normal)	*/
		{ "rc", 0, NULL, 0 },	/* restore cursor		*/
		{ "sc", 0, NULL, 0 },	/* save cursor			*/
		{ "so", 0, NULL, 0 },	/* start reverse video mode	*/
		{ "se", 0, NULL, 0 },	/* end				*/
		{ "us", 0, NULL, 0 },	/* start underline mode 	*/
		{ "ue", 0, NULL, 0 },	/* end				*/
		{ NULL, 0, NULL, 0 }
	};

char	*getenv ();
char	*tgetstr ();
char	*tgoto ();

/*-------------------------------------------------------------05/10/86-+
|									|
|	      tcap_init : initialize termcap data structure		|
|									|
+----------------------------------------------------------------------*/
tcap_init ()
	{
	struct	tcap	*p;
	char		*tp;
	unsigned int	delay;
	int		status;
	char		*termtype = getenv ("TERM");

	if ((status = tgetent (Termcap, termtype)) != 1) {
		if (status == 0) {
			fprintf (stderr, "No entry for %s in termcap\r\n",
				 termtype);
			}
		else	fprintf (stderr, "Can not open termcap file\r\n");
		exit (1);
		}

	for (p= &Tcap[0]; p->tc_id != EOS; p++) {
		tp = tgetstr (p-> tc_id, &Tsp);
		if (tp == NULL) {	 /* no such capability */
			if (p == &Tcap[BC]) tp = "\010";
			else tp = "";
			}
		delay = 0;
		while (isdigit (*tp)) {
			delay = delay*10 + (*tp++) - '0';
			}
		p->tc_str = tp;
		p->tc_delay = delay;
		p->tc_len = strlen (tp);
		Tcap_count++;
		}
	}

/*----------------------------------------------------------------------+
|									|
|		screen : common screen operation routine		|
|									|
+----------------------------------------------------------------------*/
screen (code)
int	code;		/* operation code */
	{
	int		n = BAD;	/* init to not valid entry */
	struct	tcap	*te;

	if (Tcap_count == 0) tcap_init ();
	switch (code) {
		when SCR_DEL:		put_string (Erase, 0);
		when SCR_BACKSPACE:	n = BC;
		when SCR_ERASE:		n = CL;
		when SCR_HOME:		n = HO;
		when SCR_EEOL:		n = CE;
		when SCR_SAVE:		n = SC;
		when SCR_KEYXMIT:	n = KS;
		when SCR_NOKEYXMIT:	n = KE;
		when SCR_RESTORE:	n = RC;
		when SCR_REVERSE:	n = SO;
		when SCR_NORMAL:	n = SE;
		otherwise:	;	/* ignore it */
		}
	if (n != BAD) {
		te = &Tcap[n];
		delay (te-> tc_delay);
		put_string (te-> tc_str, (unsigned)te-> tc_len);
		}
	}
 
/*----------------------------------------------------------------------+
|									|
|	   poscur : position cursor on line, column on screen		|
|									|
+----------------------------------------------------------------------*/
poscur (line, column, s)
unsigned char	line;
unsigned char	column;
char		*s;		/* option string to output at line, column */
	{
	char	*p;

	if (Tcap_count == 0) tcap_init ();
	p = tgoto (Tcap[CM].tc_str, column-1, line-1);
	delay (Tcap[CM].tc_delay);
	put_string (p, 0);
	if (s != NULL) put_string (s, 0);
	}

/*-------------------------------------------------------------05/10/86-+
|									|
|	  delay : delay output for n msec (actually write NULL)		|
|									|
+----------------------------------------------------------------------*/
static
delay (n)
unsigned char	n;			/* # of msec to delay */
	{
	static	char	c = EOS;
	register int	i;

	if (n == 0) return;

	for (i=0; i<n; i++) {
		write (fileno (stdout), &c, 1);
		}
	}

/*-------------------------------------------------------------05/11/86-+
|									|
|		  Routines to get mode-specific strings			|
|									|
+----------------------------------------------------------------------*/
get_undline (sbuf, ebuf)
char	*sbuf;				/* enter mode buffer */
char	*ebuf;				/* exit mode buffer */
	{
	if (Tcap_count == 0) tcap_init ();
	strcpy (sbuf, Tcap[US].tc_str);
	strcpy (ebuf, Tcap[UE].tc_str);
	}

get_hilite (sbuf, ebuf)
char	*sbuf;				/* enter mode buffer */
char	*ebuf;				/* exit mode buffer */
	{
	if (Tcap_count == 0) tcap_init ();
	strcpy (sbuf, Tcap[MD].tc_str);
	strcpy (ebuf, Tcap[ME].tc_str);
	}

get_rvideo (sbuf, ebuf)
char	*sbuf;				/* enter mode buffer */
char	*ebuf;				/* exit mode buffer */
	{
	if (Tcap_count == 0) tcap_init ();
	strcpy (sbuf, Tcap[SO].tc_str);
	strcpy (ebuf, Tcap[SE].tc_str);
	}

/*-------------------------------------------------------------07/05/87-+
|									|
|	    getkey : get user entered key (handle cursor key)		|
|									|
+----------------------------------------------------------------------*/
getkey ()
	{
	char		buf[20];	/* temporary hold the input stream */
	char		c;
	register int	i = 0;
	register int	ci = 0;		/* current matched index */
	int		idx;
	char		match;		/* flag, YES/NO */
	char		kmatch [4];	/* record how may char matched in
					   each key (KU, KD, KL, KR) */
	extern char	get_char();

	for (i=0; i<4; i++) kmatch[i] = 0;
loop:
	c = get_char () & 0x7f;			/* make it ASCII */
	for (match=NO, i=0; i<4; i++) {
		if (kmatch[i] < ci) continue;
		switch (i) {
			when 0: idx = KU;
			when 1: idx = KD;
			when 2: idx = KL;
			when 3: idx = KR;
			}
		if (c == Tcap[idx].tc_str[ci]) {
			kmatch[i]++;
			if (Tcap[idx].tc_len == ci+1) {
				ci = 0;
				return (0x80 | idx);
				}
			match = YES;
			}
		}
	buf[ci++] = c;		/* save input char in temp buffer */
	if (match == YES) goto loop;

	pushback (&buf[1], ci-1);
	return (buf[0]);
	}
