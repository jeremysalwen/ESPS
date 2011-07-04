/*
 * strrchr - find last occurrence of a character in a string
 */
/*
 * This routine was donated to the public domain by Henry Spencer,
 * with the specific notice that it could be used by anyone, on any
 * machine for any purpose.
 *
*/

#ifndef lint
	char *sccs_id = "@(#)strrchr.c	1.2 2/11/88 PD";
#endif

#define	NULL	0

char *				/* found char, or NULL if none */
strrchr(s, charwanted)
char *s;
char charwanted;
{
	char *scan;
	char *place;

	place = NULL;
	for (scan = s; *scan != '\0'; scan++)
		if (*scan == charwanted)
			place = scan;
	if (charwanted == '\0')
		return(scan);
	return(place);
}
