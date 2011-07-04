/*
 * geth - get short integer from file.  Used by tpen.
 */
#ifndef lint
    static char *sccs_id = "@(#)geth.c	3.1	10/20/87	ESI";
#endif

#include <stdio.h>
geth (stream)
FILE *stream;
	{
	union {
	short s;
	struct {char a, b;} c;
	} u;
	u.c.a = getc (stream);
	u.c.b = getc (stream);
	return ((int)(u.s));
	}
