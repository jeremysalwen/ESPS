/* 
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"

  Alan Parker, ESI
 
  sccs: @(#)ptype.h	3.1 9/8/87 ESI
*/
 				

 	

typedef union {
	int ival;
	double dval;
	char *sval;
	} YYSTYPE;

