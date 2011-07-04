/* saved lex output from the Sun %W% %G% */
# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
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

char *lexan_sccs = "@(#)lexan.l	3.1	9/8/87 ESI";
#include "y.tab.h"
#include "ptype.h"
#include "message.h"
#include "unix.h"

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
# define S_KEY 2
# define EXPR 4
# define FILEARG 6
# define BOL 8
# define EAT 10
# define ROL 12
# define FINDEX 14
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{ yylval.sval = savestring(yytext);
			  BEGIN FILEARG; return STRING;
			}
break;
case 2:
{ (void) command_file (yytext + 1);}
break;
case 3:
		{ BEGIN BOL; return ';'; /* statement separator */ }
break;
case 4:
	{ /* shell escape : no token returned */
			  yytext[yyleng-1] = '\0'; /* zap the newline */
			  (void) system(yytext+1);
			}
break;
case 5:
	{	/* look up the keyword, select next state.
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
break;
case 6:
	{ int v = key_lookup(kshow,"show ");
			  if (v==BAD) BEGIN EAT;
			  else if (v==SELECT) BEGIN EXPR;
			  else BEGIN FILEARG;
			  return v;
			}
break;
case 7:
	{ /* EAT state eats rest of line after bad keyword */
			  BEGIN BOL;
			  return 0;
			}
break;
case 8:
{ /* Filename: any sequence of nonblanks, but ; */
			  yylval.sval = savestring(yytext);
			  return STRING;
			}
break;
case 9:
	{ return (yylval.ival = ',');}
break;
case 10:
{ return yylval.ival = WITH;}
break;
case 11:
{ return yylval.ival = WITH;}
break;
case 12:
{ return yylval.ival = WITHOUT;}
break;
case 13:
{ return yylval.ival = WITHOUT;}
break;
case 14:
	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
break;
case 15:
	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
break;
case 16:
	{ yylval.dval = atof(yytext);
			  return VALUE;
			}
break;
case 17:
{ yylval.dval = atof(yytext);
			  return VALUE;
			}
break;
case 18:
{ yylval.sval = savestring(yytext);
			  return FIELD;
			}
break;
case 19:
	{ yytext[strlen(yytext)-1] = '\0';
			  yylval.ival= atoi(&yytext[1]);
			  return INDEX;
			}
break;
case 20:
{ /* to put a quote in a string, precede with \ */
			  yytext[yyleng-1] = '\0';
			  yylval.sval = quotestring (yytext + 1);
			  return STRING;
			}
break;
case 21:
{ /* catch mismatched strings: no multiline strings */
			  errmsg ("Missing \'.\n");
			  BEGIN EAT;
			  return BAD;
			}
break;
case 22:
{ /* to put a quote in a string, precede with \ */
			  yytext[yyleng-1] = '\0';
			  yylval.sval = quotestring (yytext + 1);
			  return STRING;
			}
break;
case 23:
{ /* catch mismatched strings: no multiline strings */
			  errmsg ("Missing \".\n");
			  BEGIN EAT;
			  return BAD;
			}
break;
case 24:
{ yylval.sval = savestring(&yytext[1]);
			  return FUNCTION_1;
			}
break;
case 25:
		{ return (yylval.ival = EQL); }
break;
case 26:
		{ return (yylval.ival = NE); }
break;
case 27:
		{ return (yylval.ival = GE); }
break;
case 28:
		{ return (yylval.ival = LE); }
break;
case 29:
		{ return (yylval.ival = '&'); }
break;
case 30:
		{ return (yylval.ival = '|'); }
break;
case 31:
		{ return (yylval.ival = EQL); }
break;
case 32:
		{ return (yylval.ival = '+'); }
break;
case 33:
		{ return (yylval.ival = '-'); }
break;
case 34:
		{ return (yylval.ival = '*'); }
break;
case 35:
		{ return (yylval.ival = '/'); }
break;
case 36:
		{ return (yylval.ival = '^'); }
break;
case 37:
		;
break;
case 38:
		;
break;
case 39:
		{ /* newline ends command */
				BEGIN BOL; return 0;}
break;
case 40:
		{ return (yylval.ival = yytext[0]);}
break;
case 41:
		{ errmsg1("Unexpected character '%c'\n",yytext[0]);
			  BEGIN EAT;
			  return BAD;
			}
break;
case 42:
	;
break;
case 43:
		;
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
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
int yyvstop[] = {
0,

-1,
0,

-1,
0,

41,
0,

38,
41,
0,

39,
0,

41,
0,

41,
-43,
0,

41,
0,

34,
41,
0,

32,
41,
0,

33,
41,
0,

35,
41,
0,

3,
41,
0,

41,
0,

31,
41,
0,

41,
0,

36,
41,
0,

41,
0,

41,
0,

6,
41,
0,

40,
41,
0,

38,
40,
41,
0,

40,
41,
0,

40,
41,
-23,
0,

40,
41,
-43,
0,

40,
41,
0,

40,
41,
0,

40,
41,
-21,
0,

34,
40,
41,
0,

32,
40,
41,
0,

33,
40,
41,
0,

40,
41,
0,

35,
40,
41,
0,

14,
40,
41,
0,

3,
40,
41,
0,

40,
41,
0,

31,
40,
41,
0,

40,
41,
0,

18,
40,
41,
0,

40,
41,
0,

36,
40,
41,
0,

40,
41,
0,

40,
41,
0,

40,
41,
0,

8,
41,
0,

8,
41,
0,

8,
41,
-43,
0,

8,
41,
0,

8,
34,
41,
0,

8,
32,
41,
0,

9,
41,
0,

8,
33,
41,
0,

8,
35,
41,
0,

8,
41,
0,

8,
31,
41,
0,

8,
41,
0,

8,
41,
0,

8,
36,
41,
0,

8,
41,
0,

8,
41,
0,

8,
41,
0,

37,
39,
0,

41,
-43,
0,

41,
0,

5,
41,
0,

41,
0,

38,
41,
0,

7,
39,
0,

41,
0,

41,
-43,
0,

41,
0,

34,
41,
0,

32,
41,
0,

33,
41,
0,

35,
41,
0,

3,
41,
0,

41,
0,

31,
41,
0,

41,
0,

36,
41,
0,

41,
0,

41,
0,

41,
-1,
0,

38,
41,
-1,
0,

1,
39,
0,

41,
-1,
0,

41,
-1,
-43,
0,

41,
-1,
0,

34,
41,
-1,
0,

32,
41,
-1,
0,

33,
41,
-1,
0,

35,
41,
-1,
0,

3,
41,
-1,
0,

41,
-1,
0,

31,
41,
-1,
0,

41,
-1,
0,

36,
41,
-1,
0,

41,
-1,
0,

41,
-1,
0,

26,
0,

-43,
0,

43,
0,

29,
0,

28,
0,

25,
0,

27,
0,

30,
0,

4,
0,

26,
0,

6,
0,

-23,
0,

23,
0,

22,
0,

-23,
0,

24,
0,

-21,
0,

21,
0,

20,
0,

-21,
0,

15,
0,

14,
0,

14,
0,

18,
0,

18,
0,

8,
0,

8,
26,
0,

8,
-43,
0,

8,
29,
0,

8,
28,
0,

8,
25,
0,

8,
27,
0,

8,
0,

8,
0,

8,
30,
0,

8,
0,

8,
26,
0,

-43,
0,

42,
43,
0,

2,
0,

5,
0,

7,
0,

26,
0,

-43,
0,

7,
43,
0,

29,
0,

28,
0,

25,
0,

27,
0,

30,
0,

4,
7,
0,

26,
0,

-1,
0,

1,
0,

26,
-1,
0,

-1,
-43,
0,

1,
43,
0,

29,
-1,
0,

28,
-1,
0,

25,
-1,
0,

27,
-1,
0,

30,
-1,
0,

-1,
0,

1,
4,
0,

26,
-1,
0,

22,
-23,
0,

24,
0,

20,
-21,
0,

16,
0,

19,
0,

8,
0,

8,
0,

17,
0,

8,
10,
0,

8,
11,
0,

10,
0,

8,
0,

11,
0,

8,
0,

8,
0,

8,
0,

8,
12,
0,

8,
13,
0,

12,
0,

13,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,17,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,18,	1,19,	
0,0,	0,0,	198,198,	200,200,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,20,	1,17,	
1,21,	198,198,	200,200,	1,22,	
1,17,	22,117,	85,164,	1,23,	
1,24,	1,17,	1,25,	102,177,	
1,26,	1,17,	46,135,	46,135,	
46,135,	46,135,	46,135,	46,135,	
46,135,	46,135,	46,135,	46,135,	
1,27,	1,28,	1,29,	1,30,	
20,114,	28,118,	1,17,	29,119,	
30,120,	58,124,	1,17,	2,33,	
83,161,	2,21,	91,165,	92,166,	
2,22,	3,20,	93,167,	3,21,	
2,23,	100,174,	3,22,	2,25,	
108,178,	2,26,	3,23,	109,179,	
110,180,	3,25,	135,188,	3,26,	
0,0,	206,206,	134,187,	1,31,	
1,17,	126,129,	2,28,	2,29,	
2,30,	129,185,	131,134,	136,138,	
3,28,	3,29,	3,30,	141,191,	
185,129,	3,34,	157,0,	157,0,	
4,33,	3,34,	4,21,	187,134,	
206,206,	4,22,	0,0,	0,0,	
0,0,	4,23,	135,188,	0,0,	
4,25,	1,32,	4,26,	32,121,	
95,168,	112,181,	0,0,	0,0,	
2,31,	157,0,	0,0,	136,138,	
0,0,	5,35,	3,31,	4,28,	
4,29,	4,30,	60,0,	60,0,	
4,34,	5,36,	5,19,	134,134,	
4,34,	54,141,	54,141,	54,141,	
54,141,	54,141,	54,141,	54,141,	
54,141,	54,141,	54,141,	129,129,	
157,0,	0,0,	2,32,	0,0,	
0,0,	60,0,	0,0,	0,0,	
3,32,	5,37,	5,38,	5,39,	
5,40,	4,31,	5,41,	5,42,	
207,207,	60,0,	5,43,	5,44,	
5,35,	5,45,	5,46,	5,47,	
5,48,	0,0,	205,207,	205,0,	
0,0,	63,0,	63,0,	0,0,	
60,0,	0,0,	60,144,	5,49,	
5,50,	5,51,	5,52,	207,207,	
0,0,	5,53,	0,0,	4,32,	
0,0,	5,53,	6,58,	0,0,	
6,39,	6,40,	7,59,	6,41,	
63,0,	0,0,	0,0,	6,43,	
0,0,	0,0,	6,45,	6,46,	
6,47,	205,0,	0,0,	0,0,	
63,0,	0,0,	0,0,	5,54,	
0,0,	0,0,	5,55,	5,56,	
0,0,	6,50,	6,51,	6,52,	
205,0,	0,0,	0,0,	63,0,	
0,0,	0,0,	7,60,	7,59,	
7,61,	0,0,	0,0,	7,62,	
7,59,	0,0,	0,0,	7,63,	
7,64,	7,65,	7,66,	0,0,	
7,67,	7,59,	0,0,	0,0,	
5,57,	0,0,	0,0,	0,0,	
6,54,	0,0,	0,0,	6,55,	
0,0,	7,68,	7,69,	7,70,	
0,0,	61,145,	7,59,	8,59,	
0,0,	48,136,	7,59,	48,137,	
48,137,	48,137,	48,137,	48,137,	
48,137,	48,137,	48,137,	48,137,	
48,137,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
7,71,	6,57,	0,0,	0,0,	
48,138,	0,0,	0,0,	7,72,	
7,59,	0,0,	61,145,	8,75,	
8,59,	8,61,	0,0,	61,145,	
8,62,	8,59,	0,0,	61,145,	
8,63,	8,64,	8,65,	8,66,	
61,145,	8,67,	8,59,	9,76,	
0,0,	0,0,	0,0,	0,0,	
7,73,	0,0,	62,0,	62,0,	
48,138,	7,74,	8,68,	8,69,	
8,70,	61,145,	0,0,	8,59,	
0,0,	61,145,	0,0,	8,59,	
0,0,	0,0,	9,20,	0,0,	
9,77,	0,0,	0,0,	9,22,	
0,0,	62,0,	0,0,	9,23,	
64,0,	64,0,	9,25,	62,146,	
9,26,	8,71,	10,76,	0,0,	
0,0,	62,0,	0,0,	61,145,	
8,72,	8,59,	0,0,	0,0,	
0,0,	9,28,	9,29,	9,30,	
0,0,	9,78,	9,79,	64,0,	
62,0,	0,0,	9,79,	0,0,	
0,0,	10,33,	0,0,	10,77,	
0,0,	0,0,	10,22,	64,0,	
11,80,	8,73,	10,23,	0,0,	
0,0,	10,25,	8,74,	10,26,	
11,81,	11,82,	0,0,	0,0,	
0,0,	0,0,	64,0,	9,31,	
66,0,	66,0,	0,0,	0,0,	
10,28,	10,29,	10,30,	0,0,	
10,78,	10,79,	0,0,	67,0,	
67,0,	10,79,	68,0,	68,0,	
11,83,	11,80,	11,84,	0,0,	
0,0,	11,85,	11,80,	66,0,	
0,0,	11,86,	11,87,	11,80,	
11,88,	9,32,	11,89,	11,80,	
0,0,	0,0,	67,0,	66,0,	
0,0,	68,0,	10,31,	0,0,	
69,0,	69,0,	11,90,	11,91,	
11,92,	11,93,	67,0,	0,0,	
11,80,	68,0,	66,0,	21,115,	
11,80,	12,96,	0,0,	12,84,	
0,0,	0,0,	12,85,	21,115,	
21,116,	67,0,	12,86,	69,0,	
68,0,	12,88,	68,147,	12,89,	
10,32,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	69,0,	
0,0,	11,94,	11,80,	13,97,	
12,91,	12,92,	12,93,	0,0,	
21,115,	0,0,	0,0,	13,98,	
13,99,	21,115,	69,0,	0,0,	
69,148,	21,115,	21,115,	0,0,	
0,0,	0,0,	21,115,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	11,95,	
0,0,	21,115,	0,0,	13,100,	
13,97,	13,101,	12,94,	21,115,	
13,102,	13,97,	0,0,	21,115,	
13,103,	13,104,	13,97,	13,105,	
0,0,	13,106,	13,97,	189,190,	
189,190,	189,190,	189,190,	189,190,	
189,190,	189,190,	189,190,	189,190,	
189,190,	13,107,	13,108,	13,109,	
13,110,	0,0,	0,0,	13,97,	
12,95,	21,115,	0,0,	13,97,	
14,113,	0,0,	14,101,	70,0,	
70,0,	14,102,	15,20,	0,0,	
15,21,	14,103,	0,0,	15,22,	
14,105,	0,0,	14,106,	15,23,	
0,0,	0,0,	15,25,	0,0,	
15,26,	0,0,	0,0,	0,0,	
13,111,	13,97,	70,0,	14,108,	
14,109,	14,110,	0,0,	0,0,	
0,0,	15,28,	15,29,	15,30,	
33,122,	16,33,	70,0,	16,21,	
72,0,	72,0,	16,22,	0,0,	
33,122,	33,123,	16,23,	0,0,	
0,0,	16,25,	0,0,	16,26,	
0,0,	70,0,	13,112,	70,149,	
0,0,	143,0,	143,0,	0,0,	
0,0,	14,111,	0,0,	72,0,	
16,28,	16,29,	16,30,	15,31,	
0,0,	33,122,	0,0,	0,0,	
0,0,	0,0,	33,122,	72,0,	
144,0,	144,0,	33,122,	33,122,	
143,0,	0,0,	0,0,	33,122,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	72,0,	14,112,	
143,0,	0,0,	33,122,	0,0,	
33,124,	15,32,	16,31,	144,0,	
33,122,	0,0,	0,0,	0,0,	
33,122,	0,0,	0,0,	143,0,	
0,0,	0,0,	0,0,	144,0,	
0,0,	0,0,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
0,0,	0,0,	144,0,	0,0,	
16,32,	0,0,	33,122,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	0,0,	0,0,	0,0,	
0,0,	34,125,	0,0,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	34,125,	34,125,	34,125,	
34,125,	38,126,	71,0,	71,0,	
0,0,	146,0,	146,0,	147,0,	
147,0,	38,126,	38,127,	0,0,	
74,0,	74,0,	0,0,	0,0,	
0,0,	0,0,	148,0,	148,0,	
0,0,	0,0,	0,0,	149,0,	
149,0,	71,0,	73,0,	73,0,	
146,0,	0,0,	147,0,	0,0,	
0,0,	0,0,	38,128,	74,0,	
0,0,	71,0,	0,0,	38,126,	
146,0,	148,0,	147,0,	38,126,	
38,126,	0,0,	149,0,	74,0,	
38,126,	73,0,	0,0,	0,0,	
71,0,	148,0,	0,0,	146,0,	
0,0,	147,0,	149,0,	38,126,	
0,0,	73,0,	74,0,	0,0,	
0,0,	38,126,	71,150,	0,0,	
148,0,	38,126,	0,0,	0,0,	
0,0,	149,0,	0,0,	0,0,	
73,0,	130,186,	130,186,	130,186,	
130,186,	130,186,	130,186,	130,186,	
130,186,	130,186,	130,186,	0,0,	
0,0,	0,0,	0,0,	0,0,	
38,129,	0,0,	0,0,	38,126,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	73,151,	0,0,	
130,186,	0,0,	75,153,	74,152,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	40,130,	40,130,	
40,130,	40,130,	42,131,	0,0,	
0,0,	0,0,	0,0,	75,153,	
0,0,	0,0,	42,131,	42,132,	
75,153,	152,0,	152,0,	0,0,	
75,153,	138,189,	0,0,	138,189,	
0,0,	75,153,	138,190,	138,190,	
138,190,	138,190,	138,190,	138,190,	
138,190,	138,190,	138,190,	138,190,	
0,0,	0,0,	75,154,	42,131,	
152,0,	0,0,	75,153,	0,0,	
42,133,	0,0,	75,153,	0,0,	
42,131,	42,131,	0,0,	0,0,	
152,0,	42,131,	194,195,	194,195,	
194,195,	194,195,	194,195,	194,195,	
194,195,	194,195,	194,195,	194,195,	
42,131,	0,0,	0,0,	152,0,	
0,0,	0,0,	42,131,	0,0,	
75,153,	0,0,	42,131,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
53,139,	53,139,	53,139,	53,139,	
53,139,	53,139,	53,139,	53,139,	
53,139,	53,139,	0,0,	0,0,	
0,0,	42,134,	0,0,	0,0,	
42,131,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	0,0,	
0,0,	0,0,	0,0,	53,139,	
0,0,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	53,140,	
53,140,	53,140,	53,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	0,0,	0,0,	0,0,	
0,0,	56,142,	0,0,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	56,140,	56,140,	56,140,	
56,140,	59,143,	0,0,	77,155,	
0,0,	0,0,	0,0,	150,0,	
150,0,	59,0,	59,0,	77,155,	
77,156,	0,0,	151,0,	151,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	150,0,	0,0,	
59,0,	0,0,	59,143,	0,0,	
77,155,	151,0,	0,0,	59,143,	
0,0,	77,155,	150,0,	59,143,	
59,0,	77,155,	77,155,	78,157,	
59,143,	151,0,	77,155,	192,0,	
192,0,	0,0,	0,0,	78,0,	
78,0,	150,0,	0,0,	59,0,	
0,0,	77,155,	0,0,	0,0,	
151,0,	59,143,	0,0,	77,155,	
0,0,	59,143,	0,0,	77,155,	
0,0,	0,0,	192,0,	0,0,	
0,0,	0,0,	78,0,	0,0,	
78,157,	0,0,	150,192,	0,0,	
0,0,	78,157,	192,0,	0,0,	
0,0,	78,157,	78,157,	0,0,	
0,0,	0,0,	78,157,	59,143,	
0,0,	77,155,	0,0,	0,0,	
0,0,	192,0,	0,0,	0,0,	
0,0,	78,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	78,157,	
0,0,	0,0,	192,196,	78,157,	
0,0,	0,0,	0,0,	0,0,	
0,0,	151,193,	0,0,	0,0,	
0,0,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	78,157,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
0,0,	0,0,	0,0,	0,0,	
79,158,	0,0,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
79,158,	79,158,	79,158,	79,158,	
80,159,	0,0,	84,162,	0,0,	
0,0,	0,0,	193,0,	193,0,	
80,159,	80,160,	84,162,	84,163,	
188,194,	0,0,	188,194,	0,0,	
0,0,	188,195,	188,195,	188,195,	
188,195,	188,195,	188,195,	188,195,	
188,195,	188,195,	188,195,	201,0,	
201,0,	193,0,	0,0,	0,0,	
0,0,	80,159,	0,0,	84,162,	
0,0,	0,0,	80,159,	0,0,	
84,162,	193,0,	80,159,	80,159,	
84,162,	84,162,	96,169,	80,159,	
0,0,	84,162,	201,0,	0,0,	
0,0,	0,0,	96,169,	96,170,	
193,0,	0,0,	80,159,	0,0,	
84,162,	0,0,	201,0,	0,0,	
80,159,	97,172,	84,162,	0,0,	
80,159,	0,0,	84,162,	0,0,	
0,0,	97,172,	97,173,	0,0,	
0,0,	201,0,	101,175,	96,169,	
0,0,	0,0,	199,0,	199,0,	
96,169,	0,0,	101,175,	101,176,	
96,169,	96,169,	0,0,	0,0,	
0,0,	96,169,	80,159,	0,0,	
84,162,	0,0,	97,172,	0,0,	
0,0,	193,197,	0,0,	97,172,	
96,169,	199,0,	96,171,	97,172,	
97,172,	0,0,	96,169,	101,175,	
97,172,	0,0,	96,169,	0,0,	
101,175,	199,0,	113,182,	0,0,	
101,175,	101,175,	0,0,	97,172,	
0,0,	101,175,	113,182,	113,183,	
0,0,	97,172,	0,0,	0,0,	
199,0,	97,172,	0,0,	201,203,	
101,175,	0,0,	0,0,	0,0,	
96,169,	122,122,	101,175,	0,0,	
0,0,	0,0,	101,175,	0,0,	
0,0,	122,122,	122,123,	113,182,	
202,0,	202,0,	0,0,	0,0,	
113,182,	0,0,	199,202,	97,172,	
113,182,	113,182,	0,0,	0,0,	
0,0,	113,182,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
101,175,	0,0,	122,122,	202,0,	
113,182,	0,0,	113,184,	122,122,	
0,0,	0,0,	113,182,	122,122,	
122,122,	0,0,	113,182,	202,0,	
122,122,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	122,122,	
0,0,	0,0,	202,0,	0,0,	
0,0,	122,122,	0,0,	0,0,	
0,0,	122,122,	0,0,	0,0,	
113,182,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	202,204,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	122,122,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	145,145,	153,153,	
154,153,	0,0,	0,0,	0,0,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	139,139,	139,139,	
139,139,	139,139,	0,0,	145,145,	
153,153,	154,153,	203,0,	203,0,	
145,145,	153,153,	154,153,	0,0,	
145,145,	153,153,	154,153,	169,169,	
0,0,	145,145,	153,153,	154,153,	
0,0,	0,0,	0,0,	169,169,	
169,170,	0,0,	0,0,	0,0,	
0,0,	203,0,	0,0,	0,0,	
0,0,	0,0,	145,145,	153,153,	
154,153,	0,0,	145,145,	153,153,	
154,153,	203,0,	182,182,	0,0,	
0,0,	0,0,	0,0,	0,0,	
169,169,	0,0,	182,182,	182,183,	
0,0,	169,169,	0,0,	0,0,	
203,0,	169,169,	169,169,	0,0,	
0,0,	0,0,	169,169,	0,0,	
145,145,	153,153,	154,153,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	169,169,	0,0,	182,182,	
0,0,	0,0,	0,0,	169,169,	
182,182,	0,0,	0,0,	169,169,	
182,182,	182,182,	0,0,	0,0,	
0,0,	182,182,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
182,182,	0,0,	0,0,	0,0,	
0,0,	0,0,	182,182,	0,0,	
0,0,	169,169,	182,182,	0,0,	
0,0,	203,205,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
182,182,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	0,0,	
0,0,	0,0,	0,0,	186,186,	
0,0,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	186,186,	
186,186,	186,186,	186,186,	196,143,	
0,0,	197,143,	0,0,	0,0,	
0,0,	0,0,	0,0,	196,198,	
196,0,	197,200,	197,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
196,143,	0,0,	197,143,	0,0,	
0,0,	196,143,	0,0,	197,143,	
0,0,	196,143,	196,0,	197,143,	
197,0,	204,143,	196,143,	0,0,	
197,143,	0,0,	0,0,	0,0,	
0,0,	204,206,	204,0,	0,0,	
0,0,	196,0,	0,0,	197,0,	
0,0,	0,0,	0,0,	196,143,	
0,0,	197,143,	0,0,	196,143,	
0,0,	197,143,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	196,199,	204,143,	0,0,	
0,0,	0,0,	0,0,	204,143,	
0,0,	0,0,	0,0,	204,143,	
204,0,	0,0,	0,0,	0,0,	
204,143,	196,143,	0,0,	197,143,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	204,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	204,143,	0,0,	197,201,	
0,0,	204,143,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	204,143,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-38,	yysvec+1,	0,	
yycrank+-44,	yysvec+1,	0,	
yycrank+-79,	yysvec+1,	0,	
yycrank+-136,	0,		0,	
yycrank+-173,	yysvec+5,	0,	
yycrank+-209,	yysvec+1,	0,	
yycrank+-274,	yysvec+1,	0,	
yycrank+-313,	yysvec+1,	0,	
yycrank+-352,	yysvec+1,	0,	
yycrank+-391,	0,		0,	
yycrank+-428,	yysvec+11,	0,	
yycrank+-486,	0,		yyvstop+1,
yycrank+-523,	yysvec+13,	yyvstop+3,
yycrank+-529,	yysvec+1,	0,	
yycrank+-560,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+5,
yycrank+0,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+10,
yycrank+3,	0,		yyvstop+12,
yycrank+-458,	0,		yyvstop+14,
yycrank+3,	0,		yyvstop+17,
yycrank+0,	0,		yyvstop+19,
yycrank+0,	0,		yyvstop+22,
yycrank+0,	0,		yyvstop+25,
yycrank+0,	0,		yyvstop+28,
yycrank+0,	0,		yyvstop+31,
yycrank+4,	0,		yyvstop+34,
yycrank+6,	0,		yyvstop+36,
yycrank+7,	0,		yyvstop+39,
yycrank+0,	0,		yyvstop+41,
yycrank+3,	0,		yyvstop+44,
yycrank+-591,	0,		yyvstop+46,
yycrank+622,	0,		yyvstop+48,
yycrank+0,	0,		yyvstop+51,
yycrank+0,	0,		yyvstop+54,
yycrank+0,	yysvec+20,	yyvstop+58,
yycrank+-744,	0,		yyvstop+61,
yycrank+0,	yysvec+21,	yyvstop+65,
yycrank+775,	0,		yyvstop+69,
yycrank+0,	yysvec+22,	yyvstop+72,
yycrank+-897,	0,		yyvstop+75,
yycrank+0,	0,		yyvstop+79,
yycrank+0,	0,		yyvstop+83,
yycrank+0,	0,		yyvstop+87,
yycrank+2,	0,		yyvstop+91,
yycrank+0,	0,		yyvstop+94,
yycrank+231,	0,		yyvstop+98,
yycrank+0,	0,		yyvstop+102,
yycrank+0,	yysvec+28,	yyvstop+106,
yycrank+0,	yysvec+29,	yyvstop+109,
yycrank+0,	yysvec+30,	yyvstop+113,
yycrank+928,	0,		yyvstop+116,
yycrank+101,	0,		yyvstop+120,
yycrank+0,	0,		yyvstop+123,
yycrank+986,	0,		yyvstop+127,
yycrank+0,	yysvec+32,	yyvstop+130,
yycrank+-8,	yysvec+33,	yyvstop+133,
yycrank+-1108,	0,		yyvstop+136,
yycrank+-133,	yysvec+59,	yyvstop+139,
yycrank+-272,	yysvec+21,	yyvstop+142,
yycrank+-321,	yysvec+59,	yyvstop+146,
yycrank+-180,	yysvec+59,	yyvstop+149,
yycrank+-347,	yysvec+59,	yyvstop+153,
yycrank+0,	0,		yyvstop+157,
yycrank+-399,	yysvec+59,	yyvstop+160,
yycrank+-410,	yysvec+59,	yyvstop+164,
yycrank+-413,	yysvec+59,	yyvstop+168,
yycrank+-439,	yysvec+59,	yyvstop+171,
yycrank+-550,	yysvec+59,	yyvstop+175,
yycrank+-737,	yysvec+59,	yyvstop+178,
yycrank+-587,	yysvec+59,	yyvstop+181,
yycrank+-761,	yysvec+59,	yyvstop+185,
yycrank+-747,	yysvec+59,	yyvstop+188,
yycrank+-869,	yysvec+33,	yyvstop+191,
yycrank+0,	0,		yyvstop+194,
yycrank+-1110,	0,		yyvstop+197,
yycrank+-1154,	0,		yyvstop+200,
yycrank+1185,	0,		yyvstop+202,
yycrank+-1307,	0,		yyvstop+205,
yycrank+0,	yysvec+80,	yyvstop+207,
yycrank+0,	0,		yyvstop+210,
yycrank+-11,	yysvec+80,	yyvstop+213,
yycrank+-1309,	0,		yyvstop+215,
yycrank+-4,	yysvec+80,	yyvstop+218,
yycrank+0,	yysvec+80,	yyvstop+220,
yycrank+0,	yysvec+80,	yyvstop+223,
yycrank+0,	yysvec+80,	yyvstop+226,
yycrank+0,	yysvec+80,	yyvstop+229,
yycrank+0,	yysvec+80,	yyvstop+232,
yycrank+-13,	yysvec+80,	yyvstop+235,
yycrank+-14,	yysvec+80,	yyvstop+237,
yycrank+-17,	yysvec+80,	yyvstop+240,
yycrank+0,	yysvec+80,	yyvstop+242,
yycrank+-4,	yysvec+80,	yyvstop+245,
yycrank+-1353,	0,		yyvstop+247,
yycrank+-1372,	0,		yyvstop+249,
yycrank+0,	yysvec+97,	yyvstop+252,
yycrank+0,	0,		yyvstop+256,
yycrank+-20,	yysvec+97,	yyvstop+259,
yycrank+-1385,	0,		yyvstop+262,
yycrank+-9,	yysvec+97,	yyvstop+266,
yycrank+0,	yysvec+97,	yyvstop+269,
yycrank+0,	yysvec+97,	yyvstop+273,
yycrank+0,	yysvec+97,	yyvstop+277,
yycrank+0,	yysvec+97,	yyvstop+281,
yycrank+0,	yysvec+97,	yyvstop+285,
yycrank+-23,	yysvec+97,	yyvstop+289,
yycrank+-26,	yysvec+97,	yyvstop+292,
yycrank+-27,	yysvec+97,	yyvstop+296,
yycrank+0,	yysvec+97,	yyvstop+299,
yycrank+-5,	yysvec+97,	yyvstop+303,
yycrank+-1425,	0,		yyvstop+306,
yycrank+0,	0,		yyvstop+309,
yycrank+0,	yysvec+21,	yyvstop+311,
yycrank+0,	0,		yyvstop+313,
yycrank+0,	0,		yyvstop+315,
yycrank+0,	0,		yyvstop+317,
yycrank+0,	0,		yyvstop+319,
yycrank+0,	0,		yyvstop+321,
yycrank+0,	0,		yyvstop+323,
yycrank+-1448,	0,		0,	
yycrank+0,	0,		yyvstop+325,
yycrank+0,	yysvec+122,	yyvstop+327,
yycrank+0,	yysvec+34,	yyvstop+329,
yycrank+-5,	yysvec+38,	yyvstop+331,
yycrank+0,	0,		yyvstop+333,
yycrank+0,	0,		yyvstop+335,
yycrank+-67,	yysvec+38,	yyvstop+337,
yycrank+773,	yysvec+40,	yyvstop+339,
yycrank+-10,	yysvec+42,	yyvstop+341,
yycrank+0,	0,		yyvstop+343,
yycrank+0,	0,		yyvstop+345,
yycrank+-55,	yysvec+42,	yyvstop+347,
yycrank+21,	yysvec+46,	yyvstop+349,
yycrank+34,	yysvec+46,	yyvstop+351,
yycrank+0,	yysvec+48,	yyvstop+353,
yycrank+870,	0,		0,	
yycrank+1479,	yysvec+53,	yyvstop+355,
yycrank+0,	yysvec+53,	yyvstop+357,
yycrank+14,	yysvec+54,	0,	
yycrank+0,	yysvec+56,	0,	
yycrank+-604,	yysvec+59,	yyvstop+359,
yycrank+-623,	yysvec+59,	yyvstop+361,
yycrank+-1569,	yysvec+21,	yyvstop+364,
yycrank+-740,	yysvec+59,	yyvstop+367,
yycrank+-742,	yysvec+59,	yyvstop+370,
yycrank+-753,	yysvec+59,	yyvstop+373,
yycrank+-758,	yysvec+59,	yyvstop+376,
yycrank+-1106,	yysvec+59,	yyvstop+379,
yycrank+-1113,	yysvec+59,	yyvstop+381,
yycrank+-900,	yysvec+59,	yyvstop+383,
yycrank+-1570,	yysvec+122,	yyvstop+386,
yycrank+-1571,	yysvec+122,	yyvstop+388,
yycrank+0,	yysvec+77,	yyvstop+391,
yycrank+0,	0,		yyvstop+393,
yycrank+-101,	yysvec+78,	yyvstop+396,
yycrank+0,	yysvec+79,	yyvstop+398,
yycrank+0,	yysvec+80,	0,	
yycrank+0,	0,		yyvstop+400,
yycrank+0,	yysvec+80,	yyvstop+402,
yycrank+0,	yysvec+84,	yyvstop+404,
yycrank+0,	0,		yyvstop+406,
yycrank+0,	yysvec+80,	yyvstop+409,
yycrank+0,	yysvec+80,	yyvstop+411,
yycrank+0,	yysvec+80,	yyvstop+413,
yycrank+0,	yysvec+80,	yyvstop+415,
yycrank+0,	yysvec+80,	yyvstop+417,
yycrank+-1614,	0,		0,	
yycrank+0,	0,		yyvstop+419,
yycrank+0,	yysvec+169,	yyvstop+422,
yycrank+0,	yysvec+97,	yyvstop+424,
yycrank+0,	0,		yyvstop+426,
yycrank+0,	yysvec+97,	yyvstop+428,
yycrank+0,	yysvec+101,	yyvstop+431,
yycrank+0,	0,		yyvstop+434,
yycrank+0,	yysvec+97,	yyvstop+437,
yycrank+0,	yysvec+97,	yyvstop+440,
yycrank+0,	yysvec+97,	yyvstop+443,
yycrank+0,	yysvec+97,	yyvstop+446,
yycrank+0,	yysvec+97,	yyvstop+449,
yycrank+-1641,	0,		yyvstop+452,
yycrank+0,	0,		yyvstop+454,
yycrank+0,	yysvec+182,	yyvstop+457,
yycrank+-16,	yysvec+38,	yyvstop+460,
yycrank+1672,	0,		yyvstop+463,
yycrank+-23,	yysvec+42,	yyvstop+465,
yycrank+1277,	0,		0,	
yycrank+487,	0,		0,	
yycrank+0,	yysvec+189,	yyvstop+468,
yycrank+0,	0,		yyvstop+470,
yycrank+-1150,	yysvec+59,	yyvstop+472,
yycrank+-1305,	yysvec+59,	yyvstop+474,
yycrank+898,	0,		0,	
yycrank+0,	yysvec+194,	yyvstop+476,
yycrank+-1794,	0,		yyvstop+478,
yycrank+-1796,	0,		yyvstop+481,
yycrank+5,	0,		yyvstop+484,
yycrank+-1381,	yysvec+59,	yyvstop+486,
yycrank+6,	0,		yyvstop+488,
yycrank+-1326,	yysvec+59,	yyvstop+490,
yycrank+-1451,	yysvec+59,	yyvstop+492,
yycrank+-1597,	yysvec+59,	yyvstop+494,
yycrank+-1840,	0,		yyvstop+496,
yycrank+-177,	yysvec+204,	yyvstop+499,
yycrank+84,	0,		yyvstop+502,
yycrank+167,	0,		yyvstop+504,
0,	0,	0};
struct yywork *yytop = yycrank+1935;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,01  ,01  ,01  ,047 ,
01  ,01  ,01  ,'+' ,',' ,'+' ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,';' ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,'_' ,
01  ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,1,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,1,0,1,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,1,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
