/* Screen controls */
 
#define SCR_ERASE	0	/* clear screen			*/
#define SCR_HOME	1	/* home the cursor		*/
#define SCR_EEOL	2	/* erase to end of line		*/
#define SCR_DEL		3	/* BS SPACE BS			*/
#define SCR_SAVE	4	/* save cursor position		*/
#define SCR_RESTORE	5	/* restore cursor position	*/
#define SCR_REVERSE	6	/* reverse video mode		*/
#define SCR_NORMAL	7	/* normal video mode		*/
#define SCR_BACKSPACE	8	/* cursor left			*/
#define SCR_KEYXMIT	9	/* enter keypad xmit mode	*/
#define SCR_NOKEYXMIT	10	/* exit keypad xmit mode	*/
 
#define EF_OK	1		/* field is ok */
#define EF_ERR	2		/* error on field */
#define EF_BOF	3		/* start by placing cursor at beg of field */
#define EF_EOF	4		/* start by placing cursor at end of field */
#define EF_FILL 5		/* fill rest of field */
 
/* update field options */
 
#define UF_MONEY	1	/* money format: dollar.cents */
#define UF_NUMBER	2	/* number */
#define UF_STRING	3	/* string to display */
#define UF_TIME		4	/* current time - HH:MM */
#define UF_DATE		5	/* today's date - MM/DD/YY */

#define SCRLINE		24	/* max number of lines on screen */
#define SCRCOL		80	/* number of columns on screen */
#define MAXVNAME	500	/* # of chars in variable name pool */
#define MAXFNAME	80	/* # of chars allowed in filename */
 
#define	BOURNE		1	/* bourne shell output */
#define	CSH		2	/* C shell output */
#define	PERL		3	/* perl script output */
