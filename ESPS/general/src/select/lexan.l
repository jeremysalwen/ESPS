%{
/*
| This material contains proprietary software of Entropic Speech, Inc.
| Any reproduction, distribution, or publication without the the prior	 
| written permission of Entropic Speech, Inc. is strictly prohibited.
| Any public distribution of copies of this work authorized in writing by 
| Entropic Speech, Inc. must bear the notice			 
|									
|              "Copyright 1986 Entropic Speech, Inc."	
|  							
| Written by:	J. T. Buck, modified for select by Alan Parker
|						
| Module:	lexan.l			
|  				
|  Lexical analyzer for select. Since keywords that may be abbreviated are
|  used, start states are used to decide what type of token to expect at
|  a given point.						
|  							
|  A token value may be an integer or a string (for filenames). 
|  						
|  We redefine the input macro to get characters using the Getc function in
|  the "io.c" module. This function takes care of prompting and log files. 
*/

char *lexan_sccs = "@(#)lexan.l	3.4	2/28/96 ESI";
#define LEXDEBUG
#include "y.tab.h"
#include "ptype.h"
#include "message.h"
#define esps_H /* do not include esps.h */
#include <esps/unix.h>

double atof();
/*
 Redefine Lex's input macro to use our Getc function and get rid of yylineno
*/
#undef input
#define input()((yytchar=yysptr>yysbuf?U(*--yysptr):Getc(yyin))==EOF?0:yytchar)
#include <ctype.h>
#include <stdio.h>
extern YYSTYPE yylval;
extern msg_prt, eof_flag;
char *savestring(), *quotestring();
int com_file_depth = 0;
struct key {
    char *text;
    int code;
};

/* keyword tables. prim gives keywords at beginning of line. 
 */

static struct key prim[] = {
    "clear", CLEAR,
    "close", CLOSE,
    "help", HELP,
    "log", LOG,
    "quit", QUIT,
    "version", VERSION,
    "write", WRITE,
    "from", FROM,
    "show", SHOW,
    "size", SIZE,
    "select", SELECT,
    "header", HEADER,
    "to", TO,
    "undo",UNDO,
    "eval",EVAL,
    0, 0
};

static struct key kshow[] = {
    "buffer", BUFFER,
    "fields", FIELDS,
    "from", FROM,
    "log", LOG,
    "last", LAST, 
    "to", TO,
    "size", SIZE,
    "select", SELECT,
    "files", FILES,
    0, 0
};


/* Start states:
At the beginning of a line, we're in state BOL. We expect a keyword
from the primary keyword table. Other states:

S_KEY: 		want second keyword for a SHOW command.
FILEARG:	want a file argument 
		Or we want a string argument (DEFINE)
EAT:		this state eats input till end of line, returns token for EOL
		It's meant to be used after an error has been seen
ROL:		rest of line returned as a string
EXPR:		expression for SELECT
FINDEX:		get index after a fieldname
*/
%}
%Start  S_KEY  EXPR FILEARG BOL EAT ROL FINDEX
L	[A-Za-z]
A	[A-Za-z0-9_]
D	[0-9]
S	[-+]?
E	[eE]{S}{D}+
KEY	{L}{A}*
NB	[^ \t\n]
NQ	[^\"\n]
NA	[^\'\n]
EQ	\\\"
EA	\\\'
%%
<ROL>.*$		{ yylval.sval = savestring(yytext);
			  BEGIN FILEARG; return STRING;
			}
<BOL>"@"[^ \t\n;]+	{ (void) command_file (yytext + 1);}
";"			{ BEGIN BOL; return ';'; /* statement separator */ }
^"!".*\n		{ /* shell escape : no token returned */
			  yytext[yyleng-1] = '\0'; /* zap the newline */
			  (void) system(yytext+1);
			}
<BOL>{KEY}		{	/* look up the keyword, select next state.
				   Always return the keyword as the token. */
				int v = key_lookup(prim,"");
				switch (v) {
				 case SHOW:
					BEGIN S_KEY;
				    	break;
				 case SELECT:
					BEGIN EXPR;
					break;
				 case EVAL:
					BEGIN EXPR;
					break;
				 case BAD:
					BEGIN EAT;
					break;
				 default:
					BEGIN FILEARG;
					break;
				}
				return v;
			}
<S_KEY>{KEY}		{ int v = key_lookup(kshow,"show ");
			  if (v==BAD) BEGIN EAT;
			  else if (v==SELECT) BEGIN EXPR;
			  else BEGIN FILEARG;
			  return v;
			}
<EAT>.*\n		{ /* EAT state eats rest of line after bad keyword */
			  BEGIN BOL;
			  return 0;
			}
<FILEARG>[^ \t\n;,]+	{ /* Filename: any sequence of nonblanks, but ; */
			  yylval.sval = savestring(yytext);
			  return STRING;
			}
<FILEARG>","		{ return (yylval.ival = ',');}
<FILEARG>WITH[ \t]*	{ return yylval.ival = WITH;}
<FILEARG>with[ \t]*	{ return yylval.ival = WITH;}
<FILEARG>WITHOUT[ \t]*	{ return yylval.ival = WITHOUT;}
<FILEARG>without[ \t]*	{ return yylval.ival = WITHOUT;}
<EXPR>{D}+\.? 	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
<EXPR>{D}*\.{D}+ 	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
<EXPR>{D}+\.?{E} 	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
<EXPR>{D}*\.{D}+{E}	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
<EXPR>[_]*{L}+{A}*	{ yylval.sval = savestring(yytext);
			  return FIELD;
			}
<EXPR>\[{D}+\]		{ yytext[strlen(yytext)-1] = '\0';
			  yylval.ival= atoi(&yytext[1]);
			  return INDEX;
			}
<EXPR>\'({NA}|{EA})*\'	{ /* to put a quote in a string, precede with \ */
			  yytext[yyleng-1] = '\0';
			  yylval.sval = quotestring (yytext + 1);
			  return STRING;
			}
<EXPR>\'({NA}|{EA})*$	{ /* catch mismatched strings: no multiline strings */
			  errmsg ("Missing \'.\n");
			  BEGIN EAT;
			  return BAD;
			}
<EXPR>\"({NQ}|{EQ})*\"	{ /* to put a quote in a string, precede with \ */
			  yytext[yyleng-1] = '\0';
			  yylval.sval = quotestring (yytext + 1);
			  return STRING;
			}
<EXPR>\"({NQ}|{EQ})*$	{ /* catch mismatched strings: no multiline strings */
			  errmsg ("Missing \".\n");
			  BEGIN EAT;
			  return BAD;
			}  
<EXPR>\${L}+{A}*	{ yylval.sval = savestring(&yytext[1]);
			  return FUNCTION_1;
			}
"=="			{ return (yylval.ival = EQL); }
"!="			{ return (yylval.ival = NE); }
">="			{ return (yylval.ival = GE); }
"<="			{ return (yylval.ival = LE); }
"&&"			{ return (yylval.ival = '&'); }
"||"			{ return (yylval.ival = '|'); }
"="			{ return (yylval.ival = EQL); }
"+"			{ return (yylval.ival = '+'); }
"-"			{ return (yylval.ival = '-'); }
"*"			{ return (yylval.ival = '*'); }
"/"			{ return (yylval.ival = '/'); }
"^"			{ return (yylval.ival = '^'); }
<BOL>\n			;
[ \t]			; /* always ignore blanks, tabs */
\n			{ /* newline ends command */
				BEGIN BOL; return 0;}
<EXPR>.			{ return (yylval.ival = yytext[0]);}
.			{ errmsg1("Unexpected character '%c'\n",yytext[0]);
			  BEGIN EAT;
			  return BAD;
			}
<BOL>#.*\n		; /* if whole line is commented out, eat newline */
#.*$			; /* comment, ignore */
%%
static int key_lookup(table,msg)
struct key *table;
char *msg;
{
    char *s = yytext;
    while (*s) {
	if (isupper(*s)) *s = tolower(*s);
	s++;
    }
    while (table->text) {
	yylval.sval = table->text;
	if (strncmp(yytext,table->text,yyleng) == 0)
	    return table->code;
	else table++;
    }
    errmsg2("Unknown command: \"%s%s\"\n",msg,yytext);
    msg_prt++;
    return BAD;
}


/* put lexical analyzer in proper state */
init_lex()
{
    BEGIN BOL;
}

#define MAX_COM_DEPTH 3
static FILE *fd_stack[MAX_COM_DEPTH+1];

command_file (name)
char *name;
{
    FILE *cmd_fd;
    if (com_file_depth >= MAX_COM_DEPTH) {
	errmsg ("Maximum nesting depth for command files exceeded\n");
	return 0;
    }
/* delete any leading spaces */
    while (isspace(*name)) name++;
    if ((cmd_fd = fopen (name, "r")) == NULL)
	errmsg1 ("Can't open command file %s\n", name);
    else {
	fd_stack[com_file_depth++] = yyin;
	yyin = cmd_fd;
	return 1;
    }
    return 0;
}

/* The lexical analyzer calls yywrap at EOF. If input is a terminal, make
   the user type QUIT. */
static
yywrap() {
    static int nquit = 0;

    if (com_file_depth) {
	(void) fclose (yyin);
	yyin = fd_stack[--com_file_depth];
	fd_stack[com_file_depth] = NULL;
	return 0;
    }
    else if (isatty(fileno(yyin))) {
	errmsg(nquit > 2 ? "OK, if you insist...\n" : "Type QUIT to exit\n");
	if (nquit > 2) exit(1);
	nquit++;
	clearerr(yyin);
	return 0;
    }
    eof_flag = 1;
    return 1;
}

/* Close any open command files */
abort_command_file () {
    int n = com_file_depth;
    while (com_file_depth) (void) yywrap ();
    eof_flag = 0;
    return n;
}

/* This function is called from the parser on an error. It zaps the rest
   of the current command (unless the current token is an end-of-line mark).
*/
lex_reset () {
    int c;
    if (*yytext != '\n' && *yytext != ';')
	while ((c = yyinput()) != '\n' && c != ';') /* null statement */;
    BEGIN BOL;
}

static char *
quotestring(s)
char *s;
{ /* process escape sequences in quoted string */
  /* YYLMAX is the size of LEX's token buffer, which is the maximum size
     string the lexical analyzer can pass to quotestring */
    char *p, buf[YYLMAX];

    p = buf;
    while (*s) {
	if (*s != '\\') *p++ = *s++;
	else { *p++ = slash (*++s); s++; }
    }
    *p = '\0';
    return savestring(buf);
}

/* handle beasts like \n in strings and character constants */
slash(c)
char c;
{
    switch (c) {
		case 'n': return '\n';
		case 'r': return '\r';
		case 'b': return '\b';
		case 'e': return '\033';
		case '0': return '\0';
		case 't': return '\t';
		case 'f': return '\f';
	        default:  return c;
    }
}
