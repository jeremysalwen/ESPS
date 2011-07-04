/*---------------------------------------------------------------------------+
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
  Joseph T. Buck, Entropic Processing, Inc.
 					
  frange_switch parses text of the form
  a:b, a:, :b, a, or :		
  and return values for a and b. If an endpoint is unspecified, the value is
  left unmodified, so you can say	
  	start = def_st; end = def_end; frange_switch (text, &start, &end); 
  v1.2 (Bernard Fraenkel): add a:+i corresponding to start=a, end=a+i	  
*/

#ifndef lint
	static char *sccsid = "@(#)frngswitch.c	1.2	7/29/87 ESI";
#endif

#define NULL 0
void
frange_switch (text, startp, endp)
char *text;
double *startp, *endp;
{
    char   *p, *index ();
    double atof ();
    if (text == NULL || *text == '\0')
	return;
    if (*text == ':') {
	if (*(text + 1))
	    *endp = atof (text + 1);
	return;
    }
    *startp = atof (text);
    if ((p = index (text, ':'))) {
	if (*(p + 1)) {
	    if (*(p + 1) == '+' && *(p + 2))
		*endp = *startp + atof (p + 2);
	    else
		*endp = atof (p + 1);
	}
    }
    else
	*endp = *startp;
    return;
}
