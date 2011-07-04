/*---------------------------------------------------------------------------+
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
  get_ddata - Joseph T. Buck		
   This routine reads in data from the input file and converts it
   to double precision form. The format of the data in the input file is
   determined by type, which may be 'b', 'w', 'l', 'f', or 'd'.	 
   BUFSIZ is defined in <stdio.h>; using the same size
*/

#ifndef lint
	static char *sccsid = "@(#)getddata.c	1.2	7/29/87 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

get_ddata (stream, type, dbuf, nsmpls)
FILE *stream;
char type;
double *dbuf;
int nsmpls;
{
    int n = BUFSIZ / sizetype (type), k;
#ifdef UE
   spsassert(stream != NULL, "get_ddata: stream is NULL");
   spasssert(dbuf != NULL, "get_ddata: dbuf is NULL");
#endif
/* Read the next block of n samples. If fewer than n remain to be read, then
   read the smaller number */
    while (nsmpls > 0) {
	if (nsmpls < n) n = nsmpls;
	switch (type) {
	    case 'b': {
		char data[BUFSIZ], *p = data;
		if (!fread (data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *dbuf++ = *p++;
		break;
	    }
	    case 'w': {
		short data[BUFSIZ/sizeof (short)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *dbuf++ = *p++;
		break;
	    }
	    case 'l': {
		long data[BUFSIZ/sizeof (long)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *dbuf++ = *p++;
		break;
	    }
	    case 'f': {
		float data[BUFSIZ/sizeof (float)], *p = data;
		if (!fread ((char *)data, sizeof *data, n, stream)) goto bomb;
		for (k = 0; k < n; k++) *dbuf++ = *p++;
		break;
	    }
/* For 'd', no conversion is needed */
	    case 'd': {
		if (!fread ((char *)dbuf, sizeof *dbuf, n, stream)) goto bomb;
		dbuf += n;
		break;
	    }
	    default:
		fprintf (stderr, "get_ddata: can't read type '%c'\n", type);
		exit (1);
	}
	nsmpls -= n;
    }
    return;
bomb:
    perror ("get_ddata: read error");
    exit (2);
}
