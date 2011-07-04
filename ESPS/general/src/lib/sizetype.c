/*	
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

| sizetype - Joseph T. Buck						     |
| 									     |
| sizetype (type), where type is 'b', 'w', 'l', 'f', 'd', or 'm', returns the|
| size of the type. sizetype ('m') is 1 (as is sizetype (other))	     |
+---------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)sizetype.c	1.3	11/19/87 ESI";
#endif

sizetype (type) char type; {
    switch (type) {
	case 'b': return sizeof (char);
	case 'w': return sizeof (short);
	case 'l': return sizeof (long);
	case 'f': return sizeof (float);
	case 'd': return sizeof (double);
	default: return 1;
    }
}
	
