/*	
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

   put_fdata - Joseph T. Buck
    This routine writes data to the output file, converting it from floating
    form to the desired type. The format of the data in the output file	   
    is determined by type, which may be 'b', 'w', 'l', 'f', or 'd'.	  
    BUFSIZ is defined in <stdio.h>; using the same size buffer as is used
    by the standard I/O library improves efficiency.			

*/
#ifndef lint
	static char *sccsid = "@(#)putfdata.c	1.9 11/4/89 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>


put_fdata (stream, type, fbuf, nsmpls)
FILE *stream;
char type;
float *fbuf;
int nsmpls;
{
    int n = BUFSIZ / sizetype (type), k;
   spsassert(fbuf, "put_fdata: fbuf NULL");
/* Write the next block of n samples. If fewer than n remain to be written,
   write the smaller number */
    while (nsmpls > 0) {
	if (nsmpls < n) n = nsmpls;
	switch (type) {
	    case 'b': {
		char data[BUFSIZ], *p = data;
		for (k = 0; k < n; k++) {
		    if (*fbuf < CHAR_MIN) fbuf++, *p++ = CHAR_MIN;
		    else if (*fbuf > CHAR_MAX) fbuf++, *p++ = CHAR_MAX;
		    else if (*fbuf > 0) *p++ = (int)(.5 + *fbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *fbuf++);
#else
		    else *p++ = -(int)(.5 - *fbuf++);
#endif
		}
		if (!fwrite (data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'w': {
		short data[BUFSIZ/sizeof (short)], *p = data;
		for (k = 0; k < n; k++) {
		    if (*fbuf < SHRT_MIN) fbuf++, *p++ = SHRT_MIN;
		    else if (*fbuf > SHRT_MAX) fbuf++, *p++ = SHRT_MAX;
		    else if (*fbuf > 0) *p++ = (int)(.5 + *fbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *fbuf++);
#else
		    else *p++ = -(int)(.5 - *fbuf++);
#endif
		}
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'l': {
		long data[BUFSIZ/sizeof (long)], *p = data;
		for (k = 0; k < n; k++) {
		    if (*fbuf < LONG_MIN) fbuf++, *p++ = LONG_MIN;
		    else if (*fbuf > LONG_MAX) fbuf++, *p++ = LONG_MAX;
		    else if (*fbuf > 0) *p++ = (int)(.5 + *fbuf++);
#ifdef MACII
		    else *p++ = 0-(int)(.5 - *fbuf++);
#else
		    else *p++ = -(int)(.5 - *fbuf++);
#endif
		}
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
	    case 'd': {
		double data[BUFSIZ/sizeof (double)], *p = data;
		for (k = 0; k < n; k++) *p++ = *fbuf++;
		if (!fwrite ((char *)data, sizeof *data, n, stream)) goto bomb;
		break;
	    }
/* For 'f', no conversion is needed */
	    case 'f': {
		if (!fwrite ((char *)fbuf, sizeof *fbuf, n, stream)) goto bomb;
		fbuf += n;
		break;
	    }
	    default:
		(void)fprintf (stderr, 
			"put_fdata: can't write type '%c'\n", type);
		exit (1);
	}
	nsmpls -= n;
    }
    return;
bomb:
    perror ("put_fdata: write error");
    exit (2);
}
