/*---------------------------------------------------------------------------+
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987,1990 Entropic Speech, Inc.; All rights reserved"
 				

 	
 Joseph T. Buck, Entropic Processing, Inc.	
									     |
 lrange_switch parses text of the form	
 a-b, a:b, a-, a:, -b, :b, a, b, -, or :
 and return values for a and b. If us ("unsigned" flag) is true, '-' is
 considered a field separator and only unsigned values are returned, though
 : is also a separator. If us is false, only the colon is a field separator,
 and signed values may be used. If an endpoint is unspecified, the value is
 left unmodified, so you can say					  
 	start = def_st; end = def_end; range_switch (text, &start, &end, us);
 The difference between lrangeswitch and rangeswitch is that the outputs are
 long.									   
									  
 v1.1: created from rngswitch.c 1.2					 
*/

#ifndef lint
	static char *sccsid = "@(#)lrngswitch.c	1.4	3/23/90 ESI";
#endif
#define NULL 0

long atol();

void
lrange_switch (text, startp, endp, us)
char *text;
long *startp, *endp; 
int us;
{
    char   *p, *index ();
    if (text == NULL || *text == '\0')
	return;
/* Check for form beginning with separator (:x, -x, :, -) */
    if ((us && *text == '-') || *text == ':') {
	if (*(text + 1))
	    *endp = atol (text + 1);
	return;
    }
/* first argument given. Set startp and try for second argument. */
    *startp = atol (text);
    if (us && (p = index (text, '-'))) {
	if (*(p + 1))
	    *endp = atol (p + 1);
    }
    else if ((p = index (text, ':'))) {
	if (*(p + 1)) {
	    if (*(p + 1) == '+' && *(p + 2))
		*endp = *startp + atol (p + 2);
	    else
		*endp = atol (p + 1);
	}
    }
/* no second argument or separator. Set end = start. */
    else
	*endp = *startp;
    return;
}
