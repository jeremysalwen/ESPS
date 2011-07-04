/* Last update: 01/13/88  11:17 AM  (Edition: 5) */
#include	<stdio.h>
#include	<strings.h>
#include	"form.h"
#include	"basic.h"

unsigned char	Rvideo;			/* reverse video */
unsigned char	Undline;		/* under line */
unsigned char	Hilite;			/* high light */
unsigned char	Shell = CSH;		/* output script type */

extern	char	Varfile[];
extern	char	Enter_mode[];
extern	char	Exit_mode[];
 
/*-------------------------------------------------------------05/08/86-+
|									|
|	       set_options : set form editor options			|
|									|
+----------------------------------------------------------------------*/
set_options (hilite, rvideo, undline, fname)
int	hilite;				/* highlight attribute */
int	rvideo;				/* reverse video attribute */
int	undline;			/* underline attribute */
char	*fname;				/* output variable file name */
	{
	char		sbuf [80];	/* set mode */
	char		ebuf [80];	/* exit mode */

	ENTER(set_options);

	sbuf[0] = ebuf[0] = EOS;
	if (hilite) {
		get_hilite (sbuf, ebuf);
		strcat (Enter_mode, sbuf);
		strcat (Exit_mode, ebuf);
		}
	if (undline) {
		get_undline (sbuf, ebuf);
		strcat (Enter_mode, sbuf);
		strcat (Exit_mode, ebuf);
		}
	if (rvideo) {
		get_rvideo (sbuf, ebuf);
		strcat (Enter_mode, sbuf);
		strcat (Exit_mode, ebuf);
		}
	if (fname) strncpy (Varfile, fname, MAXFNAME);
	EXIT;
	}
