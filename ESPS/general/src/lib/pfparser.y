%token <ival> INT CHR STRING FLT INTVAL
%token <dval> FLOVAL
%token <sval> IDENT STRVAL
%token <cval> CHRVAL
%type <ival> file statement idlist fdlist sdlist idecl fdecl sdecl string
%type <ival> ival cdlist cdecl assop limits
%type <dval> dval
%type <flist> farray fvlist
%type <ilist> iarray ivlist
%type <clist> carray cvlist
%type <cval> cval
%type <sval> prompt
%{
/*---------------------------------------------------------------------------+
| pfparser.y - Joseph T. Buck						     |
|              minor modifications by Ajaipal S. Virdy                       |
| This is a YACC parser for parameter files. It calls three external	     |
| functions:								     |
| make_symbol, which creates a new symbol; yylex, the lexical analyzer	     |
| (built by LEX); and yyerror, for error messages.			     |
|									     |
| v1.2: fix lint complaints						     |
| Support for SPS Common file added                                          |
+---------------------------------------------------------------------------*/
static char *sccsid = "%W% %G% ERL";

#include <stdio.h>
#include <esps/param.h>
#include <esps/spsassert.h>
#include <esps/epaths.h>
#include "pfparser.h"

#define PROMPT 1
#define NOPROMPT 0
#define CVMAX 127
#define CVMIN 0
static YYSTYPE tmpsym;
char msgbuf[80]; /* buffer for error messages */
#define newval(val,type) (tmpsym.type = val, tmpsym)
#define NULL_FNODE ((struct fnode *) 0)
#define NULL_INODE ((struct inode *) 0)
#define NULL_CNODE ((struct cnode *) 0)
#if !defined(IBM_RS6000) && !defined(OS5) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
#if defined(SG) || defined(hpux)
int sprintf ();
#else
char *sprintf();
#endif
char *malloc();
#endif
static double *getlimit();
static int setlimit();
static char **choice_list = NULL;
static char **get_choice();
char *savestring();

%}
%%
/* value returned by 'file' rule (and thus by yyparse) is 1 for success,
   0 if any syntax error, type mismatch, or redeclared symbol occurred.
   For many rules, the action is to return 1 for OK, 0 for error; they
   often do this by ANDing the error status of rules below.
*/
file :		/* nothing */		{ $$ = 1; }
	|	statement file		{ $$ = $1 && $2; }
	;

statement :	FLT fdlist ';'	{ $$ = $2; }
	|	INT idlist ';'		{ $$ = $2; }
	|	CHR cdlist ';'		{ $$ = $2; }
	|	string sdlist ';'	{ $$ = $2; }
/* This rule attempts to skip over the next semicolon on an error. */
	|	error ';'
			{ yyerror("Illegal statement");
			  yyerrok;
			  $$ = 0;
			}
	|	';'
			{ yyerror("Null statement ignored");
			  $$ = 1;
		        }
	;
string :	CHR '*' | STRING
	;

/* fdlist is a comma - separated declaration list of floats. Similarly,
   idlist is a declaration list of ints, and sdlist is a list
   of string declarations. 1 is returned if all symbols are OK, else 0. */
fdlist:		fdecl
	|	fdlist ',' fdecl	{ $$ = $1 && $3; }
	;
idlist:		idecl
	|	idlist ',' idecl	{ $$ = $1 && $3; }
	;
sdlist:		sdecl
	|	sdlist ',' sdecl	{ $$ = $1 && $3; }
	;
cdlist:		cdecl
	|	cdlist ',' cdecl	{ $$ = $1 && $3; }
	;
/* Here come the declarations. Making separate declaration lists
   allow us to do type checking. The newval macro is needed because
   make_symbol expects a union. make_symbol returns 1 if things are
   OK, 0 if the same symbol is defined twice */
fdecl :		IDENT assop dval prompt limits
			{ $$ = make_symbol($1,newval($3,dval),
				ST_FLOAT,$2,$4,$5,getlimit(),get_choice()); }
	|	IDENT assop farray prompt limits
			{ $$ = make_symbol($1,newval($3,flist),
				ST_FARRAY,$2,$4,$5,getlimit(),get_choice());}
	|	IDENT assop error	{ $$ = bad_decl($1);}
	;
idecl :		IDENT assop ival prompt limits
			{ $$ = make_symbol($1,newval($3,ival),
				ST_INT,$2,$4,$5,getlimit(),get_choice()); }
	|	IDENT assop iarray prompt limits
			{ $$ = make_symbol($1,newval($3,ilist),
				ST_IARRAY,$2,$4,$5,getlimit(),get_choice());}
	|	IDENT assop error	{ $$ = bad_decl($1);}
	;
cdecl :		IDENT assop cval prompt limits
			{ $$ = make_symbol($1,newval($3,cval),
				ST_CHAR,$2,$4,$5,getlimit(),get_choice()); }
	|	IDENT assop error	{ $$ = bad_decl($1);}
	;
sdecl :		IDENT assop STRVAL prompt limits
			{ $$ = make_symbol($1,newval($3,sval),
				ST_STRING,$2,$4,$5,getlimit(),get_choice()); }
	|	IDENT assop error	{ $$ = bad_decl($1);}
	;
/* The value from assop tells make_symbol whether or not to set the
   prompt flag for that symbol. */
assop :		'='			{ $$ = NOPROMPT; }
	|	'=' '?'			{ $$ = PROMPT; }
	|	'?' '='			{ $$ = PROMPT; }
	;
/* Prompt returns a prompt string. */
prompt :	/* nothing */		{ $$ = NULL; }
	|	':' STRVAL		{ $$ = $2; }
	;

limits :	/* nothing */		{ $$ = 0; }
	|	':' '[' dval ',' dval ']' { $$ = setlimit($3,$5); }
	| 	':' carray		{ $$ = set_choice($2); }
	;


dval :		FLOVAL
	|	INTVAL			{ $$ = (double)$1;}
	|	CHRVAL			{ $$ = (double)$1;}
	;
ival :		INTVAL
	|	CHRVAL			{ $$ = (int)$1; }
	;
cval :		CHRVAL
	|	INTVAL
			{ if ($1 <= CVMAX && $1 >= CVMIN) $$ = (char)$1;
			  else {
			    sprintf (msgbuf,"%d won't fit in a char",$1);
			    yyerror (msgbuf);
			    $$ = 0;
			}}
	;
farray :	'{' fvlist '}'		{ $$ = $2; }
	|	'{' error '}'
			{ yyerror("Bad array declaration"); $$ = NULL; }
	;
fvlist :	dval			{ $$ = fcons($1, NULL_FNODE); }
	|	dval ',' fvlist		{ $$ = fcons($1,$3); }
	;
iarray :	'{' ivlist '}'		{ $$ = $2; }
	|	'{' error '}'
			{ yyerror("Bad array declaration"); $$ = NULL; }
	;
ivlist :	ival			{ $$ = icons($1,NULL_INODE); }
	|	ival ',' ivlist		{ $$ = icons($1,$3); }
	;
carray :	'{' cvlist '}'		{ $$ = $2; }
	|	'{' error '}'
			{ yyerror("Bad array declaration"); $$ = NULL; }
	;
cvlist :	STRVAL			{ $$ = ccons($1,NULL_CNODE); }
	|	STRVAL ',' cvlist	{ $$ = ccons($1,$3); }
	;
%%
/* fcons and icons build up linked lists of values, like Lisp cons. */
static struct fnode *
fcons(v,list)
#ifdef SUN386i
float v;
#else
double v;
#endif
struct fnode *list;
{

    struct fnode *p = (struct fnode *) malloc(sizeof(struct fnode));
    p->f_car = v;
    p->f_cdr = list;
    return p;
}

static struct inode *
icons(v,list)
int v;
struct inode *list;
{

    struct inode *p = (struct inode *) malloc(sizeof(struct inode));
    p->i_car = v;
    p->i_cdr = list;
    return p;
}

static struct cnode *
ccons(v,list)
char *v;
struct cnode *list;
{

    struct cnode *p = (struct cnode *) malloc(sizeof(struct cnode));
    p->c_car = v;
    p->c_cdr = list;
    return p;
}






static bad_decl(name)
char *name;
{
sprintf(msgbuf,"Bad declaration for %s",name);
yyerror(msgbuf);
return 0;
}

/* include the lexical analyzer */
#include "pflexan.c"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

char	*ctime();
char	*getenv();
int	stat();
char	*strcpy(), *getsym_s();

#ifdef XXXX
int	yacc_errors;
#endif

char	*home;
char	*com;
char	common_file[80];
FILE	*com_fp = NULL;	 /* file pointer to SPS Common file */

/* codes for use between here and lex_open ONLY */

#define FAIL -1
#define NOPARAM -1
#define NOCOMM -2
#define NOBOTH -3

/* globals used by parameter/common stuff */
int Common_on;		/* 1 if common processing is enabled */
int Esps_verbose;	/* Esps verbosity level */
int In_common;

extern int debug_level;

#define DEFAULT_VERBOSITY 2	/* default common verbosity level */
int read_params_called=0;

int
read_params (param, flag, check_file)
char *param;	
int flag;
char *check_file;
{

  /* routine to read and parse a parameter file */

  struct stat	combuf;		/* struct for determining status */
  struct stat	parambuf;	/* struct for determining status */
  char *fp_param = NULL;		/* full path to param file */
  int param_ok=0;
  int did_common=0;
  int status = 0;
  FILE *junk_fp;
  int with_param;	/* non-zero means param != null */

  In_common = 0;
  read_params_called = 1;
  if(getenv("USE_ESPS_COMMON") != NULL &&
     strcmp(getenv("USE_ESPS_COMMON"),"off") == 0)
    Common_on = 0;
  else
    Common_on = 1;

  if(getenv("ESPS_VERBOSE") != NULL)
    Esps_verbose = atoi(getenv("ESPS_VERBOSE"));
  else
    Esps_verbose = DEFAULT_VERBOSITY;	/* default verbosity level */

  if(Common_on) {
    if((com = getenv("ESPSCOM")) == NULL) {
      if((home = getenv("HOME")) == NULL)
	home = ".";
      (void) strcpy(common_file,home);
      (void) strcat(common_file,"/");
      (void) strcat(common_file,SPS_COMMON);
    }
    else
      (void) strcpy(common_file,com);
  }

  if (param == NULL) {
    with_param = 0;
    /*    param = savestring("params");  */
  }  else
    with_param = 1;

  if(param)
    fp_param = FIND_ESPS_PARAM_FILE(NULL,param);

  if (fp_param != NULL) {
	
    if (debug_level || (Esps_verbose > 1))
      fprintf(stderr, "read_params: using param file %s\n", fp_param);

    if(lex_open(fp_param) == FAIL) 	/* process parameter file */
      status = NOPARAM;
    else {
      yacc_errors = 0;
      yyparse();
      fclose(fd_param);
      if (yacc_errors > 0)
	exit(2);
      param_ok = 1;
    }
  } else {
    if ( with_param )
      fprintf(stderr,
	      "read_params: warning - could not find param file %s\n",
	      param);
  }

  if(!Common_on || flag == SC_NOCOMMON)
    return status;	/* don't process SPS Common */

  if((junk_fp = fopen(common_file,"r")) != NULL) { /* Common exists */

    int verbose_tmp = Esps_verbose;

    (void)fclose(junk_fp);
    if (stat(common_file, &combuf) != 0) {
      fprintf(stderr,"read_params: cannot stat common file %s\n",
	      common_file);
      fprintf(stderr,"This should not happen.\n");
      exit (1);
    }


    if (param_ok && stat(fp_param, &parambuf) != 0) {
      fprintf(stderr,"read_params: cannot stat param file %s\n",
	      fp_param);
      fprintf(stderr,"This should not happen.\n");
      exit (1);
    }

    if (combuf.st_mode & S_IFDIR) {
      fprintf(stderr,
              "read_params: Warning, ESPS Common file %s is a directory.\n",
	      common_file);
      fprintf(stderr, "It is not being processed.\n");
    } else {
      if ((param_ok &&
	   (combuf.st_mtime>=parambuf.st_mtime)) || !param_ok) {
	status = lex_open(common_file);	/* process common file */
	spsassert(status == 0,"read_params: lex_open failed");
	yacc_errors = 0;
	In_common = 1;  /* this tells make_symbol that we are in common */
	yyparse();
	(void)fclose(fd_param);
	if (yacc_errors > 0)
	  exit(2);
	did_common = 1;
	In_common = 0;
      }
    }

    Esps_verbose = 0;
    if((check_file != NULL) &&  ( (symtype("filename") == ST_UNDEF) ||
				  (strcmp(check_file,getsym_s("filename")) != 0)) ) {
      init_table();		/* clear symbol table */
      if(param_ok) {
	status = lex_open(fp_param);	/* process param again */
	spsassert(status == 0,"read_params: lex_open failed");
	yacc_errors = 0;
	yyparse();
	(void)fclose(fd_param);
	if (yacc_errors > 0)
	  exit(2);
      }
      did_common = 0;
    }
    Esps_verbose = verbose_tmp;
  }
  else {
    if (status == NOPARAM) return NOBOTH;
    if (status == 0) return NOCOMM;
  }
  if (did_common && debug_level)
    fprintf(stderr,"read_params: Common processed.\n");
  return status;

} /* end read_param() */

int
read_param(param)
char *param;
{
    read_params(param,SC_NOCOMMON,NULL);
    return 0;
}

double MinMax[2];

static
int
setlimit(min,max)
double min, max;
{
	MinMax[0]=min;
	MinMax[1]=max;
	return 1;
}

static
double *
getlimit()
{
	return MinMax;
}
	

static
set_choice(v)
struct cnode *v;
{
	int i=0;
	struct cnode *ptr = v;

	while(ptr != NULL) {
		i++;
		ptr = ptr->c_cdr;
	}
		
	choice_list = (char **)malloc(sizeof(char *)*(i+1));
		
	ptr = v;
	i=0;
	while(ptr != NULL) {
		choice_list[i++] = savestring(ptr->c_car);
		ptr = ptr->c_cdr;
	}
	choice_list[i] = NULL;
	return i;
}

static
char **
get_choice()
{
	char **list = choice_list;

	choice_list = NULL;
	return list;
}
