/*	
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				


| Joseph T. Buck, Entropic Processing, Inc.				     |
|									     |
| range_switch parses text of the form					     |
| a-b, a:b, a-, a:, -b, :b, a, b, -, or :				     |
| and return values for a and b. If us ("unsigned" flag) is true, '-' is     |
| considered a field separator and only unsigned values are returned, though |
| : is also a separator. If us is false, only the colon is a field separator,|
| and signed values may be used. If an endpoint is unspecified, the value is |
| left unmodified, so you can say					     |
| 	start = def_st; end = def_end; range_switch (text, &start, &end, us);|
|									     |
| v1.2: support the format a:+i, meaning a:a+i				     |
+---------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)rngswitch.c	1.3	11/19/87 ESI";
#endif
#define NULL 0

void
range_switch (text, startp, endp, us)
char *text;
int *startp, *endp, us;
{
    char   *p, *index ();
    if (text == NULL || *text == '\0')
	return;
/* Check for form beginning with separator (:x, -x, :, -) */
    if ((us && *text == '-') || *text == ':') {
	if (*(text + 1))
	    *endp = atoi (text + 1);
	return;
    }
/* first argument given. Set startp and try for second argument. */
    *startp = atoi (text);
    if (us && (p = index (text, '-'))) {
	if (*(p + 1))
	    *endp = atoi (p + 1);
    }
    else if ((p = index (text, ':'))) {
	if (*(p + 1)) {
	    if (*(p + 1) == '+' && *(p + 2))
		*endp = *startp + atoi (p + 2);
	    else
		*endp = atoi (p + 1);
	}
    }
/* no second argument or separator. Set end = start. */
    else
	*endp = *startp;
    return;
}
