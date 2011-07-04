/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Program:	atoh.c
 *
 * Checked by:  Alan Parker
 *
 */


#include <ctype.h>
#include <stdio.h>
#include <esps/esps.h>

#ifndef lint
	static char *sccsid = "@(#)atoh.c	1.4	7/30/87 ESI";
#endif

int
atoh(s)
char *s;
{
    int hex = 0;
    spsassert(s != NULL, "atoh: s is NULL");
    while (*s) {
	if (isupper(*s)) (void)tolower(*s);
	if (isdigit(*s)) hex = 16*hex - '0' + *s;
	else if (*s>='a' && *s<='f') hex = 16*hex - 'a' + 10 + *s;
	else return hex;
	s++;
    }
    return hex;
}
