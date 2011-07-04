/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore (atoarrayf, atoarray2), 
 *              Rodney Johnson (atoarray)
 * Checked by:  Alan Parker (atoarrayf)
 * Revised by:
 *
 * Brief descriptions: 
 *
 *     atoarrayf, converts an ASCII file to a float array		
 *     atoarrays, converts an ASCII file to a string array	
 *     atoarray,  which converts an ASCII file to an array of any type.	
 *
 * The file contains numerical values separated by white space or new  
 * lines.							       
 */

static char *sccs_id = "@(#)atoarray.c	1.9	2/20/96	ERL/ESI";


/*
 * include files
 */

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <math.h>

/*
 * defines
 */

#define DATA_CHUNK 25000	/* number of ASCII points handled at a time */
#define ABS(a) (((a) >= 0) ? (a) : -(a))

/*
 * system functions and variables
 */

#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char	    *realloc();
char	    *calloc();
double	    sqrt();
void        addstr();
#endif
/*
 * ESPS functions
 */
  
int 	    typesiz();
char        *savestring();

/*
 * local functions
 */

static int  numeric();

/*
 * main functions
 */

float *
atoarrayf(strm, nvalues, max)
    FILE    *strm;	/*ASCII file stream*/
    long    *nvalues;	/*pointer to number of values found*/
    float   *max;	/*pointer to return maximum absolute value found*/
{
    float *data;	    /*pointer to data*/
    unsigned dsize = 0;		/* count of currently allocated floats */
    unsigned cnt = 0;		/* count within current chunk */
    int nchar;			/* number of characters matched by fscanf */
    spsassert(strm != NULL, "atoarrayf: NULL file pointer");
    spsassert(nvalues != NULL, "atoarrayf: NULL pointer for nvalues");
    spsassert(max != NULL, "atoarrayf: NULL pointer for max value");
    *max = 0.0;
    *nvalues = 0;	
    /*
     * allocate space for first DATA_CHUNK of floats
     */
    data = (float*) calloc ((unsigned) DATA_CHUNK, sizeof(float));
    dsize = DATA_CHUNK;
    spsassert(data != NULL, "memory allocation failure");
    while ((nchar = fscanf(strm, "%f", &data[*nvalues])) != EOF) {
	spsassert(nchar != 0, "atoarrayf: input not in float format");
	if (ABS(data[*nvalues]) > *max) *max = ABS(data[*nvalues]);
	(*nvalues)++;
	cnt++;
	/*
	 * get more space if we've used up the chunk
	 */
	 if (cnt == DATA_CHUNK) {
	    dsize += DATA_CHUNK;
	    data = (float*) realloc
	        ((char *)data, (unsigned)dsize*sizeof(float));
	    spsassert(data != NULL, "atoarrayf: memory allocation failure");
	    cnt = 0;
	 }
    }
    /*
     * don't return more memory than needed
     */
    data = (float*) realloc((char *)data, (unsigned) *nvalues * sizeof(float));
    spsassert(data != NULL, "atoarrayf: realloc failed to reduce Size");
    return data;
}


#define MAXLINE 250
char **
atoarrays(strm, nvalues, max)
    FILE    *strm;	/*ASCII file stream*/
    long    *nvalues;	/*pointer to number of values found*/
    int      *max;	/*pointer to return maximum string length found*/
{
    char **data = NULL;	    /*pointer to data*/
    char *string_item = (char *) malloc(MAXLINE);
    int length = 0;		/* length of latest string */

    spsassert(strm != NULL, "atoarrays: NULL file pointer");
    spsassert(nvalues != NULL, "atoarrays: NULL pointer for nvalues");

    if (max != NULL) 
      *max = 0;

    *nvalues = 0;	

    while (fgets(string_item, MAXLINE, strm) != NULL) {

	length = strlen(string_item);

	if (length > 0) {

	    /* remove trailing line feed*/

	    if (string_item[length-1] == '\n')
	      string_item[length-1] = '\0';

	    length = strlen(string_item);

	    if (length > 0) {

		addstr(savestring(string_item), &data);

		if ((max != NULL) && (length > *max))
		  *max = length; 

		(*nvalues)++;
	      }
	  }
      }
    return data;
}


#define CHUNK_SIZE  (DATA_CHUNK * sizeof(float))

char *
atoarray(strm, type, nvalues, max)
    FILE    *strm;
    int	    type;
    long    *nvalues;
    double  *max;
{
    char	*data;		/* pointer to data*/
    unsigned	dsize;		/* count of currently allocated elements */
    unsigned	d_chunk;	/* number of elements to allocate at a time */
    unsigned	tsize;		/* size of one data element */
    long	cnt;		/* count of items read */
    int	    	nitem;		/* number of items matched by fscanf */
    double  	absval;		/* absolute value of data element */
    double  	reval, imval;	/* real & imaginary parts of data element */
    double  	maxabs;		/* max absolute value */

    spsassert(strm, "atoarray: NULL file pointer");
    spsassert(nvalues, "atoarray: NULL pointer for nvalues");
    spsassert(numeric(type), "atoarray: non-numeric type code");

    tsize = typesiz(type);
    cnt = 0;
    if (max) maxabs = 0.0;

    /*
     * allocate space for first data chunk
     */

    d_chunk = CHUNK_SIZE / tsize;
    data = calloc(d_chunk, tsize);
    spsassert(data, "atoarray: can't allocate space for data");
    dsize = d_chunk;
    
    for ( ; ; )		/* Will break out of loop on EOF or bad data. */
    {
	switch(type)
	{
	case DOUBLE: {
	    double	*arr = (double *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%lf", &arr[cnt])) == 1);
		 cnt++)
	    {
		if (max)
		{
		    absval = arr[cnt];
		    if (absval < 0) absval = -absval;
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case FLOAT: {
	    float	*arr = (float *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%f", &arr[cnt])) == 1);
		 cnt++)
	    {
		if (max)
		{
		    absval = arr[cnt];
		    if (absval < 0) absval = -absval;
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case LONG: {
	    long	*arr = (long *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%ld", &arr[cnt])) == 1);
		 cnt++)
	    {
		if (max)
		{
		    absval = arr[cnt];
		    if (absval < 0) absval = -absval;
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case SHORT: {
	    short	*arr = (short *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%hd", &arr[cnt])) == 1);
		 cnt++)
	    {
		if (max)
		{
		    absval = arr[cnt];
		    if (absval < 0) absval = -absval;
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case BYTE: {
	    char	*arr = (char *) data;
	    short	item;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%hd", &item)) == 1);
		 cnt++)
	    {
		arr[cnt] = item;
		if (max)
		{
		    absval = arr[cnt];
		    if (absval < 0) absval = -absval;
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case DOUBLE_CPLX: {
	    double_cplx	*arr = (double_cplx *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%lf%lf",
				     &arr[cnt].real, &arr[cnt].imag)) == 2);
		 cnt++)
	    {
		if (max)
		{
		    reval = arr[cnt].real;
		    if (reval < 0) reval = -reval;
		    imval = arr[cnt].imag;
		    if (imval < 0) imval = -imval;

		    if (reval > imval)
		    {
			imval /= reval;
			absval = reval * sqrt(1.0 + imval*imval);
		    }
		    else if (imval > reval)
		    {
			reval /= imval;
			absval = imval * sqrt(1.0 + reval*reval);
		    }
		    else absval = reval * sqrt(2.0);

		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case FLOAT_CPLX: {
	    float_cplx	*arr = (float_cplx *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%f%f",
				     &arr[cnt].real, &arr[cnt].imag)) == 2);
		 cnt++)
	    {
		if (max)
		{
		    reval = arr[cnt].real;
		    if (reval < 0) reval = -reval;
		    imval = arr[cnt].imag;
		    if (imval < 0) imval = -imval;

		    if (reval > imval)
		    {
			imval /= reval;
			absval = reval * sqrt(1.0 + imval*imval);
		    }
		    else if (imval > reval)
		    {
			reval /= imval;
			absval = imval * sqrt(1.0 + reval*reval);
		    }
		    else absval = reval * sqrt(2.0);
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case LONG_CPLX: {
	    long_cplx	*arr = (long_cplx *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%ld%ld",
				     &arr[cnt].real, &arr[cnt].imag)) == 2);
		 cnt++)
	    {
		if (max)
		{
		    reval = arr[cnt].real;
		    imval = arr[cnt].imag;
		    absval = sqrt(reval*reval + imval*imval);
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case SHORT_CPLX: {
	    short_cplx	*arr = (short_cplx *) data;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%hd%hd",
				     &arr[cnt].real, &arr[cnt].imag)) == 2);
		 cnt++)
	    {
		if (max)
		{
		    reval = arr[cnt].real;
		    imval = arr[cnt].imag;
		    absval = sqrt(reval*reval + imval*imval);
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	case BYTE_CPLX: {
	    byte_cplx	*arr = (byte_cplx *) data;
	    short	re_item, im_item;

	    for ( ;
		 (cnt < dsize
		  && (nitem = fscanf(strm, "%hd%hd",
				     &re_item, &im_item)) == 2);
		 cnt++)
	    {
		arr[cnt].real = re_item;
		arr[cnt].imag = im_item;
		if (max)
		{
		    reval = arr[cnt].real;
		    imval = arr[cnt].imag;
		    absval = sqrt(reval*reval + imval*imval);
		    if (absval > maxabs) maxabs = absval;
		}
	    }
	    break; }
	}
	if (cnt != dsize) break; /* Exit loop on EOF or bad data. */
	else
	{
	    dsize += d_chunk;
	    data = realloc(data, dsize*tsize);
	    spsassert(data, "atoarray: can't reallocate space for data");
	}
    }
    switch (nitem)
    {
    case 0:
	Fprintf(stderr, "%s: badly formatted input item.\n", "atoarray");
	exit(1);
	break;
    case 1:
	Fprintf(stderr,
		"%s:  badly formatted input item or missing re/im part.\n",
		"atoarray");
	exit(1);
	break;
    case EOF:
	data = realloc(data, dsize*tsize);
	spsassert(data, "atoarray: realloc failed to reduce size");
	if (max) *max = maxabs;
	*nvalues = cnt;
	return data;
	break;
    default:
	Fprintf(stderr, "%s: error reading data. fscanf returned %d\n",
	       "atoarray", nitem);
	exit(1);
	break;
    }

    /*NOTREACHED*/
}



/*
 * Check for numeric data type.
 */

static int
numeric(type)
    int	    type;
{
    switch (type)
    {
    case BYTE:
    case SHORT:
    case LONG:
    case FLOAT:
    case DOUBLE:
    case BYTE_CPLX:
    case SHORT_CPLX:
    case LONG_CPLX:
    case FLOAT_CPLX:
    case DOUBLE_CPLX:
	return YES;
	break;
    default:
	return NO;
	break;
    }

    /*NOTREACHED*/
}

