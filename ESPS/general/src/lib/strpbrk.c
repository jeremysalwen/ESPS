/*
 * strpbrk - find first occurrence of any char from breakat in s
 */
/*
 * This routine was donated to the public domain by Henry Spencer,
 * with the specific notice that it could be used by anyone, on any
 * machine for any purpose.
 *
*/

#ifndef lint
	static char *sccs_id = "@(#)strpbrk.c	1.2 12/10/87 PD";
#endif

#define	NULL	0

char *				/* found char, or NULL if none */
strpbrk(s, breakat)
char *s;
char *breakat;
{
	register char *sscan;
	register char *bscan;

	for (sscan = s; *sscan != '\0'; sscan++) {
		for (bscan = breakat; *bscan != '\0';)	/* ++ moved down. */
			if (*sscan == *bscan++)
				return(sscan);
	}
	return(NULL);
}
