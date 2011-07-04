/* Last update: 01/13/88  10:59 AM  (Edition: 11) */
#include	<stdio.h>
#include	<strings.h>
#include	"basic.h"
#include	"form.h"
/*
    Two flavors to display message:

	form_msg	display message then  position cursor at given
			position, this allows form filling on terminal
			without cursor save restore capability.

	formmsg		display  message and return   to  the caller's
			position  using terminal's save/restore cursor
			ability. This is designed to be used  by field
			checking routine where  user don't know  where
			the field is.
*/

#define MAXSMSG 5
#define MSGSIZE 132

int	Form_msg = 0;			/* form message on screen flag */

static	char	Savemsg [MAXSMSG][MSGSIZE];
static	int	Sdx = 0;		/* number of next entry to use */
static	int	Smcount = 0;		/* number of messages saved */
/*----------------------------------------------------------------------+
|									|
|	    form_msg : display message on the message line		|
|									|
+----------------------------------------------------------------------*/
form_msg (s, line, col)
char		*s;		/* message to display, if NULL, clear message */
unsigned char	line;		/* line to go at end */
unsigned char	col;		/* column to go at end */
	{
	ENTER (form_msg);
	poscur ((unsigned char)24, (unsigned char)1, (char *)NULL);
	screen (SCR_REVERSE);
	screen (SCR_EEOL);
	if (s) {
		Form_msg = 1;
		put_string (s, 0);
		save_msg (s);
		}
	else	Form_msg = 0;
	poscur (line, col, (char *)NULL);
	EXIT;
	}
 
/*----------------------------------------------------------------------+
|									|
|	  formmsg : form_msg () use terminal SC, RC feature		|
|									|
+----------------------------------------------------------------------*/
formmsg (s)
char	*s;		/* message to display, if NULL, clear message */
	{
	ENTER (formmsg);
	screen (SCR_SAVE);
	poscur ((unsigned char)24, (unsigned char)1, (char *)NULL);
	screen (SCR_REVERSE);
	screen (SCR_EEOL);
	if (s) {
		Form_msg = 1;
		put_string (s, 0);
		save_msg (s);
		}
	else	Form_msg = 0;
	screen (SCR_RESTORE);
	EXIT;
	}
 
/*----------------------------------------------------------------------+
|									|
|	    save_msg : save a message on the message buffer		|
|									|
+----------------------------------------------------------------------*/
save_msg (s)
char	*s;
	{
	ENTER (save_msg);
	strncpy (Savemsg[Sdx], s, MSGSIZE);
	if (++Sdx >= MAXSMSG) Sdx = 0;
	if (Smcount < MAXSMSG) Smcount++;
	EXIT;
	}
 
/*----------------------------------------------------------------------+
|									|
|	    prev_msg : display previously displayed message		|
|									|
+----------------------------------------------------------------------*/
prev_msg ()
	{
	ENTER (prev_msg);
	if (!Smcount) return (0);
	/* This routine actually pop the message stored */
	if (--Sdx < 0) Sdx = MAXSMSG-1;
	screen (SCR_SAVE);
	poscur ((unsigned char)24, (unsigned char)1, (char *)NULL);
	screen (SCR_REVERSE);
	screen (SCR_EEOL);
	put_string (Savemsg[Sdx], 0);
	Form_msg = 1;
	screen (SCR_RESTORE);
	RETURN (1);
	}
