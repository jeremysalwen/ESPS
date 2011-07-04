/*
 * strchr - find first occurrence of a character in a string
 */
/*
 * This routine was donated to the public domain by Henry Spencer,
 * with the specific notice that it could be used by anyone, on any
 * machine for any purpose.
 *
*/

#ifndef lint
	static char *sccs_id = "@(#)strchr.c	1.5 3/2/98 PD";
#endif

#define	NULL	0

#ifndef LINUX_OR_MAC
char *				/* found char, or NULL if none */
strchr(s, charwanted)
char *s;
register char charwanted;
{
	register char *scan;

	/*
	 * The odd placement of the two tests is so NUL is findable.
	 */
	for (scan = s; *scan != charwanted;)	/* ++ moved down for opt. */
		if (*scan++ == '\0')
			return(NULL);
	return(scan);
}
#endif

