/*	
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

|  Joseph T. Buck, Entropic Processing, Inc.				     |
|  									     |
|  This function allocates memory for a string, copies it into the	     |
|  allocated memory, and returns a pointer to the copy.			     |
----------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)savstring.c	1.4	1/16/92 ESI";
#endif
#define NULL 0
char *
savestring(s)
char *s;
{
    char *p, *malloc(), *strcpy();
/* If arg is null, or we can't get memory, return NULL */
    if ((s == NULL) || (p = malloc ((unsigned)(strlen(s)+1))) == NULL)
	return NULL;
/* Copy the argument into the dynamic memory, and return a pointer to it */
    return strcpy (p,s);
}
