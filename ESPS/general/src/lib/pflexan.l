%{
/*---------------------------------------------------------------------------+
| pflexan.l - Joseph T. Buck, Entropic Processing, Inc.			     |
| Lexical analyzer for parameter files. It recognizes the following tokens:  |
|    -	keywords int, char, float, string				     |
|    -	the characters * , ; ? = . { } (one-character tokens)		     |
|    -	identifiers. They start with a letter, followed by any number of     |
|	letters, digits, and underscores. The whole name (up to the size     |
|	of LEX's yytext buffer, which is big) is saved.			     |
|    -   character constants 'x'. Most c escape sequences like '\n' work.    |
|	Control characters may also be entered like '^a'.		     |
|    -   integers. Whatever atoi takes (+20, 1002, -12345)		     |
|    -   floating point values (returned as doubles).			     |
|    -   string constants "string". Quotes are included by \". Other C	     |
|	escape sequences also work. Multiline strings aren't allowed.	     |
|									     |
|	This should be included in the parser by putting an #include	     |
|	line in the YACC source.					     |
+---------------------------------------------------------------------------*/
#ifndef lint
    /* we say lex_sccsid since there's an sccsid in pfparser.y */
	static char *lex_sccsid = "@(#)pflexan.l	1.8	12/19/95 ESI";
#endif
static FILE *fd_param; /* copied to yyin by lex analyzer */
static int line_num = 1; /* current line number for error messages */

static char *quotestring();
static	    yywrap();
char *savestring();
double atof();
int atoi();

#define YYLMAX 1000

/* D: digit. S: optional sign. E: exponent. Q: quote. EQ: escaped quote \".
    NQ: anything but quote or newline. A: apostrophe (single quote) */
%}
D	[0-9]
S	[-+]?
E	[eE]{S}{D}+
Q	\"
EQ	\\\"
NQ	[^\"\n]
A	"'"
%%
%{
yyin = fd_param;	/* make lexical analyzer read from parameter file */
%}
int		      { return INT; }
float		      { return FLT; }
double		      { return FLT; }
char		      { return CHR; }
string		      { return STRING; }
yes|true	      { yylval.ival = 1; return INTVAL; }
no|false	      { yylval.ival = 0; return INTVAL; }
[a-zA-Z][a-zA-Z0-9_]* { yylval.sval = savestring(yytext); return IDENT; }
{S}{D}*"."{D}+	|
{S}{D}+"."{D}*	|
{S}{D}+{E}	|
{S}{D}*"."{D}*{E}     { yylval.dval = atof(yytext); return FLOVAL; }
{S}{D}+               { yylval.ival = atoi(yytext); return INTVAL; }
{A}[^\n{A}]{A}	      { yylval.cval = yytext[1]; return CHRVAL; }
{A}\^.{A}	      { yylval.cval = yytext[2] & 037; return CHRVAL; }
{A}\\.{A}	      { yylval.cval = slash(yytext[2]); return CHRVAL; }
\n		      { line_num++; }
[ \t]+  		;
["\[\]*?,;=:{}"] 	      { yylval.ival = yytext[0]; return yytext[0]; }
{Q}({NQ}|{EQ})*{Q}    { /* to put a quote in a string, precede with \ */
			yytext[yyleng-1] = '\0';
		        yylval.sval = quotestring (&yytext[1]);
			return STRVAL;
		      }
{Q}({NQ}|{EQ})*$      { /* catch mismatched strings: no multiline strings */
			yyerror ("Mismatched quotes");
		      }
\#.*			; /*comment*/
.		      { yyerror("Unexpected character"); }
%%
static char *
quotestring(s)
char *s;
{ /* process escape sequences in quoted string */
  /* YYLMAX is the size of LEX's token buffer, which is the maximum size
     string the lexical analyzer can pass to quotestring */
    char *p, buf[YYLMAX];
    int cvt;
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

static int yacc_errors = 0;
static char *param_name = NULL;

yyerror(s)
char *s;
{
    if (yacc_errors == 0)
	fprintf(stderr,"Errors in parameter file %s:\n",param_name);
    fprintf (stderr,"Line %d: %s\n",line_num,s);
    yacc_errors++;
}

/* This routine lets other modules tell lex where to read from. */
void
lex_input(fp)
FILE *fp;
{
    fd_param = fp;
}

extern int Esps_verbose;

/* This routine supplies a filename, rather than just a descriptor */
int
lex_open(name)
char *name;
{
    FILE *fd = fopen(name,"r");
    if (fd == NULL) {
	if (Esps_verbose)
	    fprintf(stderr,
		"Warning (read_params): parameter file %s cannot be opened\n",
		name);
	return -1;
    }
    param_name = name;
    lex_input(fd);
    return 0;
}

/*
 yywrap is called on EOF. For now it just returns 1, telling LEX to quit.
 */
static
yywrap() {
    return 1;
}
