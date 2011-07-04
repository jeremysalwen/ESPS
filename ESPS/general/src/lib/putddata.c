/*	
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

   put_ddata - Joseph T. Buck						    
    This routine writes data to the output file, converting it from double 
    precision form to the desired type. The format of the data in the output
    file is determined by type, which may be 'b', 'w', 'l', 'f', or 'd'.   
    BUFSIZ is defined in <stdio.h>; using the same size buffer as is used 
    by the standard I/O library improves efficiency.			 
    put_ddata does not check for overflow in the conversion!		
*/

#ifndef lint
	static char *sccsid = "@(#)putddata.c	1.6 11/4/89 ESI";
#endif
#include <stdio.h>
#include <esps/esps.h>


put_ddata (stream, type, dbuf, nsmpls)
FILE *stream;
char type;
double *dbuf;
int nsmpls;
{
    int n = BUFSIZ / sizetype (type), k;
/* Write the next block of n samples. If fewer than n remain to be written,
   write the smaller number */
    while (nsmpls > 0) {
	if (nsmpls < n) n = nsmpls;
	switch (type) {
	    case 'b': {
		char data[BUFSIZ], *p = data;
		for (k = 0; k < n; k++) {
		    if (*dbuf < CHAR_MIN) dbuf++, *p++ = CHAR_MIN;
		    else if (*dbuf > CHAR_MAX) dbuf++, *p++ = CHAR_MAX;
		    else if (*dbuf > 0) *p++ = (int)(.5 + *dbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *dbuf++);
#else
		    else *p++ = -(int)(.5 - *dbuf++);
#endif
		}
		if (!fwrite (data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'w': {
		short data[BUFSIZ/sizeof (short)], *p = data;
		for (k = 0; k < n; k++) {
		    if (*dbuf < SHRT_MIN) dbuf++, *p++ = SHRT_MIN;
		    else if (*dbuf > SHRT_MAX) dbuf++, *p++ = SHRT_MAX;
		    else if (*dbuf > 0) *p++ = (int)(.5 + *dbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *dbuf++);
#else
		    else *p++ = -(int)(.5 - *dbuf++);
#endif
		}
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'l': {
		long data[BUFSIZ/sizeof (long)], *p = data;
		for (k = 0; k < n; k++) {
		    if (*dbuf < LONG_MIN) dbuf++, *p++ = LONG_MIN;
		    else if (*dbuf > LONG_MAX) dbuf++, *p++ = LONG_MAX;
		    else if (*dbuf > 0) *p++ = (int)(.5 + *dbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *dbuf++);
#else
		    else *p++ = -(int)(.5 - *dbuf++);
#endif
		}
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'f': {
		float data[BUFSIZ/sizeof (float)], *p = data;
		for (k = 0; k < n; k++) *p++ = *dbuf++;
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
/* For 'd', no conversion is needed */
	    case 'd': {
		if (!fwrite ((char *)dbuf, sizeof *dbuf, n, stream)) goto bomb;
		dbuf += n;
		break;
	    }
	    default:
		(void)fprintf (stderr, 
		  "put_ddata: can't write type '%c'\n", type);
		exit (1);
	}
	nsmpls -= n;
    }
    return;
bomb:
    perror ("put_ddata: write error");
    exit (2);
}
