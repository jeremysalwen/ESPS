/*	
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

| statsiz, fstatsiz: return size of a file in bytes			     |
| statsiz takes a filename; fstatsiz takes a Unix file descriptor (int)	     |
+---------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)statsiz.c	1.3	11/19/87 ESI";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

long statsiz(name)
char *name; {
    struct stat buf;
    if (stat (name, &buf) >= 0)
	return (long) buf.st_size;
    else
	return - 1;
}

long fstatsiz (fd)
int fd; {
    struct stat buf;
    if (fstat (fd, &buf) >= 0)
	return (long) buf.st_size;
    else
	return - 1;
}
