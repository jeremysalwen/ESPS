/*---------------------------------------------------------------------------+
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 	
  get_fdata - Joseph T. Buck
   This routine reads in data from the input file and converts it
   to floating form. The format of the data in the input file is determined
   by type, which may be 'b', 'w', 'l', 'f', or 'd'.
   BUFSIZ is defined in <stdio.h>; using the same size
   buffer as is used by the standard I/O library improves efficiency.
*/
#ifndef lint
	static char *sccsid = "@(#)getfdata.c	1.3	7/30/87 ESI";
#endif
#include <stdio.h>
get_fdata (stream, type, fbuf, nsmpls)
FILE *stream;
char type;
float *fbuf;
int nsmpls;
{
    int n = BUFSIZ / sizetype (type), k;
/* Read the next block of n samples. If fewer than n remain to be read, then
   read the smaller number */
#ifdef UE
   spsassert(stream != NULL, "get_fdata: stream is NULL");
   spasssert(fbuf != NULL, "get_fdata: fbuf is NULL");
#endif
    while (nsmpls > 0) {
	if (nsmpls < n) n = nsmpls;
	switch (type) {
	    case 'b': {
		char data[BUFSIZ], *p = data;
		if (!fread (data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *fbuf++ = *p++;
		break;
	    }
	    case 'w': {
		short data[BUFSIZ/sizeof (short)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *fbuf++ = *p++;
		break;
	    }
	    case 'l': {
		long data[BUFSIZ/sizeof (long)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *fbuf++ = *p++;
		break;
	    }
	    case 'd': {
		double data[BUFSIZ/sizeof (double)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *fbuf++ = *p++;
		break;
	    }
/* For 'f', no conversion is needed */
	    case 'f': {
		if (!fread ((char *)fbuf, sizeof *fbuf, n, stream)) goto bomb;
		fbuf += n;
		break;
	    }
	    default:
		fprintf (stderr, "get_fdata: can't read type '%c'\n", type);
		exit (1);
	}
	nsmpls -= n;
    }
    return;
bomb:
    perror ("get_fdata: read error");
    exit (2);
}
