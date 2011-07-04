/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
  sccs: @(#)interp.h	3.3 7/16/91 ESI
*/
 	

typedef struct Stack {
	double dval;
	char *sval;
	short type;
} Stack;

typedef struct Inst {
	double dval;
	char *sval;
	int (*inst)();
	short type;
} Inst;

#define STOP (int (*)()) 0
#define NOOP (int (*)()) 1

extern Inst prog[];
extern int eq(), and(), or(), lt(), gt(), le(), ge(), ne(), not(), getfld(); 
extern int Const(), add(), sub(), mul(), divide(), power(), negate();
extern int func1(), bltin(), func2();

#define code2(a,b) code(a); code(b);
#define code3(a,b,c) code(a); code(b); code(c);
#define operand2(a,b) operand(a); operand(b);
