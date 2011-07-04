/*
 * puth - output short integer on stream.  Used by tpen.
 */
#ifndef lint
    static char *sccs_id = "@(#)puth.c	3.1	10/20/87	ESI";
#endif

#include <stdio.h>
puth (i,stream)
    FILE *stream;
    int  i;
{
    union
    {
	short s;
	struct {char a, b;} c;
    } u;

    u.s = i;
    putc (u.c.a,stream);
    putc (u.c.b,stream);
}
