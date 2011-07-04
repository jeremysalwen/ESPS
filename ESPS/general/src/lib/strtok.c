/*
 * Get next token from string s (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from delim.  Writes NULs into s to end tokens.  delim need not
 * remain constant from call to call.
 */

/*
 * This routine was donated to the public domain by Henry Spencer,
 * with the specific notice that it could be used by anyone, on any
 * machine for any purpose.
 *
*/
#ifndef OS5

#ifndef lint
	static char *sccs_id = "@(#)strtok.c	1.1 12/1/87 PD";
#endif

#define	NULL	0
#define CONST 

static char *scanpoint = NULL;

char *				/* NULL if no token left */
strtok(s, delim)
char *s;
register CONST char *delim;
{
	register char *scan;
	char *tok;
	register CONST char *dscan;

	if (s == NULL && scanpoint == NULL)
		return(NULL);
	if (s != NULL)
		scan = s;
	else
		scan = scanpoint;

	/*
	 * Scan leading delimiters.
	 */
	for (; *scan != '\0'; scan++) {
		for (dscan = delim; *dscan != '\0'; dscan++)
			if (*scan == *dscan)
				break;
		if (*dscan == '\0')
			break;
	}
	if (*scan == '\0') {
		scanpoint = NULL;
		return(NULL);
	}

	tok = scan;

	/*
	 * Scan token.
	 */
	for (; *scan != '\0'; scan++) {
		for (dscan = delim; *dscan != '\0';)	/* ++ moved down. */
			if (*scan == *dscan++) {
				scanpoint = scan+1;
				*scan = '\0';
				return(tok);
			}
	}

	/*
	 * Reached end of string.
	 */
	scanpoint = NULL;
	return(tok);
}
#endif
