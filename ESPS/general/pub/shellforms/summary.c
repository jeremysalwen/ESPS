/* Last update: 01/14/88  00:30 AM  (Edition: 14) */
#include	<stdio.h>
#include	"basic.h"
#include	"form.h"

static char	*Summary[] = {
"    RETURN - terminate        CTRL P or up arrow - vertical up one field",
"    TAB    - next field       CTRL N or down arrow - vertical down one field",
"    CTRL T - prev field       Intr char - abort",
"    CTRL L - refresh screen   Stop char - stop process",
"",
"--- Editable Field ---                  --- Selection Field ---",
"  Movement:",
"    CTRL A - beginning of field         SPACE  - show next selection",
"    CTRL E - end of field               CTRL H - show prev selection",
"    CTRL F - forward one char           x      - find next selection starts",
"             or --> key                          with character 'x'",
"    CTRL B - backword one char          ?      - get help message (-m flag)",
"             or <-- key or CTRL H",
"  Editing:",
"    CTRL U - delete to start of field",
"    CTRL K - delete to end of field",
"    CTRL W - delete prev word",
NULL
};

static char	*Newline = "\r\n\r\n";
static char	*Header = "Shell Form Version: ";
extern	char	*Version;
extern	char	*Copyright;
extern	char	*Bugs;
extern	char	get_char();
/*-------------------------------------------------------------01/13/88-+
|									|
|		 show_summary : display on-line summary 		|
|									|
+----------------------------------------------------------------------*/
show_summary ()
	{
	char	**pp = Summary;
	char	*p;

	ENTER (show_summary);
	screen (SCR_ERASE);
	put_string (Header, 0);    put_string (Version, 0);
	put_string (Newline, 2);
	put_string (Copyright, 0); put_string (Newline, 2);
	put_string (Bugs, 0);      put_string (Newline, 4);
	while ((p = *pp++) != NULL) {
		put_string (p, 0);
		put_string (Newline, 2);
		}
	put_string ("\r\n     Press SPACE to continue: ", 0);
	while (get_char() != ' ');
	EXIT;
	}
