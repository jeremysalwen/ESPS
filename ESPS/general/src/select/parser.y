%{
/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"

  Parser for select.  
  Written by: Alan Parker, ESI, Washington, DC

*/

char *parser_sccs = "%W%	%G% ESI";

#include "ptype.h"
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#define NOSPSINCLUDE		/* causes esps.h not to include other files */
#include <esps/esps.h>
#include "message.h"
#include "interp.h"
#include <esps/unix.h>


void code(), operand();
void initcode();
static jmp_buf env;
static int parse_status;
int msg_prt;
int query_error;
%}
%start line
%token  <ival> BAD CLEAR CLOSE HELP LOG QUIT VERSION WRITE FROM SHOW 
%token  <ival> FIELDS LAST TO SIZE SELECT WITH WITHOUT BUFFER HEADER FILES
%token  <ival> UNDO EVAL
%token  <sval> STRING FIELD FUNCTION_1 
%token  <dval> VALUE
%type   <ival> line command
%type   <ival> frmlst towith towo frmlstterm towithterm towoterm
%type   <ival> fieldname
%type   <ival> expr
%token  <ival> INDEX 
%left   <ival> '|' 
%left   <ival> '&'
%left <ival> '<' '>' GE LE EQL NE
%left <ival> '+' '-'
%left <ival> '*' '/'
%left	<ival> UNARYMINUS NOT
%right <ival> '^'
%%

 line:	  command		{$$ = parse_status = 1;}
	| command ';' line	{$$ = parse_status = $1 && $3;}
	;


command:  CLEAR			{$$ = clear_buffer();}
	| CLOSE			{$$ = close_logfile(2);}
	| HEADER STRING		{$$ = psps_in($2);}
	| HEADER 		{$$ = psps_in("-D");}
	| HELP			{$$ = help();}
	| LOG STRING		{$$ = open_logfile($2);}
	| LOG			{$$ = show_log_status();}
	| QUIT			{$$ = terminate();}
	| VERSION		{$$ = show_version();}
	| WRITE			{$$ = write_buffer();}
	| FROM 			{$$ = close_infile();}
	  frmlst
	| FROM 			{$$ = close_infile();}
	| SHOW BUFFER		{$$ = show_buffer();}
	| SHOW FIELDS		{$$ = show_fields();}
	| SHOW FROM STRING	{$$ = psps_in($3);}
	| SHOW FROM		{$$ = psps_in(" ");}
	| SHOW LOG		{$$ = show_log_status();}
	| SHOW LAST		{$$ = show_last();}
	| SHOW TO STRING	{$$ = show_to($3);}
	| SHOW TO		{$$ = show_to(" ");}
	| SHOW SIZE		{$$ = show_size();}
	| SHOW FILES		{$$ = show_files();}
	| SIZE			{$$ = show_size();}
	| SELECT 		{initcode();}
	  expr 			{code(STOP); $$ = select_rec();} 
	| SHOW SELECT 		{initcode();}
	  expr 			{code(STOP); $$ = show_select();}
	| EVAL 			{initcode();}
	  expr 			{code(STOP); $$ = eval();} 
	| SHOW			{yyerror("SHOW what?\n"); $$ = 0;}
	| SELECT		{yyerror("SELECT what?\n"); $$ = 0;}
	| TO STRING		{$$ = close_outfile();}
	  WITH towith 		{$$ = open_outfile($2);}
	| TO STRING 		{$$ = close_outfile();}
 	  WITHOUT towo 		{$$ = open_outfile($2);}
	| TO STRING		{$$ = close_outfile(); (void)open_outfile($2);}
	| TO			{yyerror("TO what?\n"); $$ = 0;}
	| UNDO			{$$ = undo();}
	| BAD			{$$ = 0;}
	;

frmlst:   frmlstterm		
	| frmlstterm ',' frmlst 
	;
frmlstterm:	STRING 		{$$ = in_files($1);}
	;

towith:	  towithterm		
	| towithterm ',' towith
	;
towithterm:	STRING		{$$ = towith($1);}
	;


towo:	  towoterm	
	| towoterm ',' towo
	;
towoterm:	STRING		{$$ = towithout($1);}
	;

expr: 		VALUE			{ $$ = 1; code(Const); 
					  operand(DOUBLE,(char *)NULL,$1); }
	|	fieldname		{ $$ = 1; }
	|	STRING			{ $$ = 1; code(Const);
					  operand(CHAR,$1,(double)0);}
	|	FUNCTION_1		{ $$ = 1; code(func1);
					  operand(CHAR,$1,(double)0);}
	|	FIELD '(' FIELD ')'	{ $$ = 1; code(func2);
					  operand(CHAR,$1,(double)0);
					  operand(CHAR,$3,(double)0); }
	|	FIELD '(' expr ')'	{ $$ = $3; code(bltin);
					  operand(CHAR,$1,(double)0); }
	|	'(' expr ')'		{ $$ = $2; }
	|	expr '+' expr		{ code(add); }
	|	expr '-' expr		{ code(sub); }
	|	expr '*' expr		{ code(mul); }
	|	expr '/' expr		{ code(divide); }
	|	expr '^' expr		{ code(power); }
	|	'-' expr %prec UNARYMINUS
					{ code(negate); }
	|	expr '<' expr  		{ code(lt); }
	|	expr '>' expr  		{ code(gt); }
	|	expr EQL expr  		{ code(eq); }
	|	expr NE expr   		{ code(ne); }
	|	expr GE expr   		{ code(ge); }
	|	expr LE expr   		{ code(le); }
	| 	expr '&' expr		{ code(and); }
	|	expr '|' expr		{ code(or); }
	|	NOT expr		{ $$ = $2; code(not);}
	;

fieldname:	FIELD  INDEX 	{ code(getfld); operand(CHAR,$1,(double)0);
			  	  operand(DOUBLE,(char *)NULL,(double)$2);
				}
	 |	FIELD	     	{ code(getfld); operand(CHAR,$1,(double)0);
				  operand(DOUBLE,(char *)NULL,(double)-1);
				}
	 ;

%%



/* do_command is called from outside; yyparse is not called directly. */
do_command() {
    int (*oldsig)();   /* not used here, but I'll keep it */

    init_lex();
    msg_prt = parse_status = 0;
    if (setjmp (env) != 0) {
	lex_reset ();
	return 0;
    }
    (void) yyparse();
    return parse_status;
}

static
yyerror(msg) char *msg; {
    extern char yytext[];
    char *p = yytext;
    if (*p == '\n' || *p == ';') p = "expected more text";
    if (msg_prt == 0) errmsg2("%s: %s\n",msg, p);
    msg_prt = 1;
    query_error++;
    yyclearin; 
    longjmp (env, 1);
}


