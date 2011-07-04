/*---------------------------------------------------------------------------+
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

| This file supports the hash table, which is invisible from outside.	     |
|   It has the following entry points:					     |
|									     |
|   make_symbol: called to create a new symbol in the symbol table.	     |
|   init_table:  called to clear the symbol table.			     |
|   symtype:	returns the type of a symbol ("undefined" counts as a type)  |
|   symsize:	returns the size of a symbol 
|   getsym_i:	gets an integer symbol					     |
|   getsym_d:	gets a real symbol, as a double				     |
|   getsym_c:	gets a character symbol					     |
|   getsym_s:	gets a string symbol					     |
|   getsym_ia:	gets an integer array symbol				     |
|   getsym_fa:  gets a float array symbol				     |
|   getsym_da:  gets a double array symbol				     |
|   symerr_exit: exit if there have been any getsym_xx errors		     |
+---------------------------------------------------------------------------*/
#ifndef lint
	static char *sccsid = "@(#)symbols.c	1.31 10/7/96 ERL";
#endif
#include <ctype.h>
#include <stdio.h>
#include <esps/esps.h>

/* table size should be a prime number */
#define TABLESIZ 299
static struct symbol htable[TABLESIZ];
static int n_entries = 0;

/* globals for Common stuff */
extern int Common_on;
extern int Esps_verbose;
extern int In_common;

#ifndef DEC_ALPHA
char *malloc(), *realloc();
#endif
int global_prompt;
extern int debug_level;

char *getenv();
int atoi();
double atof();

static int  hash();
static void undef();
static void badtype();
static void toolong();
static int  goodint();
static int  goodfloat();
static int  check_tty_ok();


extern int read_params_called;

static struct symbol *
hash_lookup(name)
char *name;
{
/* hash_lookup returns an pointer to a cell in the hash table.
   If the s_used field is set, the symbol has been found.
   If not, this is the appropriate cell to insert the symbol. */
    int idx, step = 1;

    spsassert(name,"hash_lookup: name is NULL");
    idx = hash(name) % TABLESIZ;
    while (htable[idx].s_used) {
	if (strcmp(name,htable[idx].s_name) == 0) return &htable[idx];
	idx = (idx + step) % TABLESIZ;
	step++;
    }
    return &htable[idx];
}

static int
compare_sym(a,b)
char *a,*b;
{
  if (((struct symbol *)a)->s_sequence == ((struct symbol *)b)->s_sequence)
   return 0;
  if (((struct symbol *)a)->s_sequence > ((struct symbol *)b)->s_sequence)
   return 1;
  if (((struct symbol *)a)->s_sequence < ((struct symbol *)b)->s_sequence)
   return -1;
}

char **
symlist(nparams)
int * nparams;
{
	struct symbol htable_copy[TABLESIZ];

	char **list = (char **)malloc((TABLESIZ+1)*sizeof(char **));
	int idx1=0, idx2=0, idx3=0;
	spsassert(list,"symlist(): cannot allocate symbol table");

	for (idx1=0; idx1<TABLESIZ; idx1++) 
	  if (htable[idx1].s_used) {
	   htable_copy[idx2++] = htable[idx1];
	  }

	qsort((char*)htable_copy, idx2, sizeof(struct symbol), compare_sym);

	for (idx1=0; idx1<idx2; idx1++) {
	    list[idx3++] = htable_copy[idx1].s_name;
	}
	list[idx3] = NULL;
	list = (char **)realloc(list,(idx3+1)*sizeof(char **));
	spsassert(list,"symlist(): cannot reallocate symbol table");
	if (nparams) 
	    *nparams = idx3;

	return list;
}

	
	

static struct symbol *
get_symbol(name)
char *name;
{
    struct symbol *p;
    if(debug_level && !read_params_called) {
	fprintf(stderr, "symbols.c: warning: a param file function has been called before\n");
	fprintf(stderr,"read_params was called.\n");
    }
    p = hash_lookup(name);
    return (p->s_used ? p : NULL);
}

make_symbol (name,value,type,prompt,pstring,limits,MINMAX,choices)
char *name;
YYSTYPE value;
int prompt,type;
char *pstring;
double *MINMAX;
int limits;
char **choices;
{ /* Makes a symbol in a hash table */
    char msg[80]; /* for yyerror */
    struct symbol *p;
    static tty_msg = 0;

    if (n_entries == TABLESIZ) {
	fprintf(stderr, 
	"Symbol table full! Too many parameter file symbols defined.\n");
	exit(1);
    }

    p = hash_lookup(name);

    p->s_name = name;
    p->s_type = type;
    if(!In_common || (In_common && !p->s_used))
        p->s_pstring = pstring;
    switch (type) {
	case ST_FLOAT: p->s_value.dval=value.dval; break;
	case ST_INT: p->s_value.ival=value.ival; break;
	case ST_STRING: p->s_value.sval=value.sval; break;
	case ST_CHAR: p->s_value.cval=value.cval; break;
	case ST_FARRAY: p->s_value.flist=value.flist; break;
	case ST_IARRAY: p->s_value.ilist=value.ilist; break;
	default    : yyerror("Funny type"); 
    };
    if(prompt)   
	p->s_prompt = 2;
	

/* If the symbol wasn't used, only then increment n_entries */
    if (!p->s_used) {
	n_entries++;
        p->s_sequence = n_entries;
    }

    if (limits && choices) {
    	p->s_choices = choices;
    	p->s_nchoices = limits;
	p->s_minmax = 0;
    } else if (limits) {
	p->s_minmax = 1;
	p->s_min = MINMAX[0];
	p->s_max = MINMAX[1];
    } else {
	p->s_minmax = 0;
	p->s_choices = NULL;
	p->s_nchoices = 0;
    }


    if (In_common)
	p->s_com = 1;

    p->s_used = 1;
    return 1;
} 
void
init_table()
{
    int i;
    for(i=0; i < TABLESIZ; i++) {
	htable[i].s_used = 0;
	htable[i].s_com = 0;
	htable[i].s_name = NULL;
    }
}
static int
hash(s)
char *s;
{ /* This is a lousy hash function. Oh well. */
    int sum = 0;
    while (*s) sum += *s++;
    return sum;
}

#define prompt(sym) (sym->s_pstring ? sym->s_pstring : sym->s_name)
#define SIZ 128
#define NONBLANK (ansbuf[0]!='\0' && ansbuf[0]!='\n')
#define BLANK (ansbuf[0]=='\0' || ansbuf[0]=='\n')
static char ansbuf[SIZ];

int
symtype(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"symtype: name is NULL");
    p = get_symbol(name);
    return p == NULL ? ST_UNDEF : p->s_type;
}

char
getsym_c(name)
char *name;
{
    struct symbol *p;
    int i;
    spsassert(name,"getsym_c: name is NULL");

    p = get_symbol(name);

    if (p == NULL) {
	undef(name);
	return 0;
    }
    else if (p->s_type != ST_CHAR) {
	badtype(name,"char");
	return 0;
    }
    else if (p->s_prompt == 2) {
       global_prompt = p->s_prompt;
       if (check_tty_ok(&global_prompt)){/* this has side effects, see below */
                p->s_prompt = global_prompt;

		fprintf(stderr,"%s [%c]",prompt(p),p->s_value.cval);
		if (p->s_nchoices)
			fprintf(stderr," (Choices are:");
		for (i=0; i<p->s_nchoices; i++)
			fprintf(stderr," %s",p->s_choices[i]);
		if (p->s_nchoices)
			fprintf(stderr,")");
		fprintf(stderr,":");
		fgets(ansbuf,SIZ,stdin);
		if(NONBLANK) p->s_value.cval = ansbuf[0];
	}
    }
    if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	fprintf(stderr,"getsym_c: symbol %s, value %c, taken from Common.\n",
	  name, p->s_value.cval);

    if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	fprintf(stderr,"getsym_c: symbol %s, value %c taken from param file.\n",
	  name, p->s_value.cval);

    if(p->s_prompt) p->s_prompt = 1;

    return p->s_value.cval;
}

char
getsymdef_c(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"getsymdef_c: name is NULL");

    p = get_symbol(name);

    if (p == NULL) {
	undef(name);
	return 0;
    }
    else if (p->s_type != ST_CHAR) {
	badtype(name,"char");
	return 0;
    }
    return p->s_value.cval;
}

char *
getsym_s(name)
char *name;
{
    struct symbol *p;
    char *savestring();
    int i;
    spsassert(name,"getsym_s: name is NULL");

    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return NULL;
    }
    else if (p->s_type != ST_STRING) {
	badtype(name,"string");
	return NULL;
    }
    else if (p->s_prompt == 2) {
       global_prompt = p->s_prompt;
       if (check_tty_ok(&global_prompt)){/* this has side effects, see below */
                p->s_prompt = global_prompt;

		fprintf(stderr,"%s [%s] ",prompt(p),p->s_value.sval);
		if (p->s_nchoices)
			fprintf(stderr," (Choices are:");
		for (i=0; i<p->s_nchoices; i++)
			fprintf(stderr," %s",p->s_choices[i]);
		if (p->s_nchoices)
			fprintf(stderr,")");
		fprintf(stderr,":");
		fgets(ansbuf,SIZ,stdin);
		if(NONBLANK) {
/* This ugly stuff is to kill the newline at the end */
	    	  int n = strlen(ansbuf);
	    	  if (ansbuf[n-1]=='\n') ansbuf[n-1] = 0;
	    	   p->s_value.sval = savestring(ansbuf);
		}
	}
    }
    if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	fprintf(stderr,"getsym_s: symbol %s, value %s, taken from Common.\n",
	  name, p->s_value.sval);

    if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	fprintf(stderr,"getsym_s: symbol %s, value %s taken from param file.\n",
	  name, p->s_value.sval);

    if(p->s_prompt) p->s_prompt = 1;
    return p->s_value.sval;
}

char *
getsymdef_s(name)
char *name;
{
    struct symbol *p;
    char *savestring();

    spsassert(name,"getsymdef_s: name is NULL");
    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return NULL;
    }
    else if (p->s_type != ST_STRING) {
	badtype(name,"string");
	return NULL;
    }
    return p->s_value.sval;
}

double
getsym_d(name)
char *name;
{
    struct symbol *p;
    int i;
    spsassert(name,"getsym_d: name is NULL");
    
    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return 0.0;
    }
    else if (p->s_type != ST_FLOAT) {
	badtype(name,"float");
	return 0.0;
    }
    else if (p->s_prompt == 2) {
       global_prompt = p->s_prompt;
       if (check_tty_ok(&global_prompt)){/* this has side effects, see below */
        p->s_prompt = global_prompt;

	while (1) {		/* loop till we get valid input */
	    fprintf(stderr,"%s [%g] ",prompt(p),p->s_value.dval);
		if (p->s_nchoices)
			fprintf(stderr," (Choices are:");
		for (i=0; i<p->s_nchoices; i++)
			fprintf(stderr," %s",p->s_choices[i]);
		if (p->s_nchoices)
			fprintf(stderr,")");
		fprintf(stderr,":");
	    fgets(ansbuf,SIZ,stdin);
	    if(BLANK) break;
	    if(goodfloat(ansbuf)) {
		p->s_value.dval = atof(ansbuf);
		break;
	    }
	    fprintf(stderr,"Not a valid real value\n");
	}
	}
    }
    if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	fprintf(stderr,"getsym_d: symbol %s, value %lg, taken from Common.\n",
	  name, p->s_value.dval);

    if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	fprintf(stderr,"getsym_d: symbol %s, value %lg taken from param file.\n",
	  name, p->s_value.dval);

    if(p->s_prompt) p->s_prompt = 1;
    return p->s_value.dval;
}

double
getsymdef_d(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"getsymdef_d: name is NULL");
    
    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return 0.0;
    }
    else if (p->s_type != ST_FLOAT) {
	badtype(name,"float");
	return 0.0;
    }
    return p->s_value.dval;
}

int
getsym_i(name)
char *name;
{
    struct symbol *p;
    int i;
    spsassert(name,"getsym_i: name is NULL");
    
    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return 0;
    }
    else if (p->s_type != ST_INT) {
	badtype(name,"int");
	return 0;
    }
    else if (p->s_prompt == 2) {
       global_prompt = p->s_prompt;
       if (check_tty_ok(&global_prompt)){/* this has side effects, see below */
        p->s_prompt = global_prompt;

	while (1) {		/* loop till we get good input */
	    fprintf(stderr,"%s [%d] ",prompt(p),p->s_value.ival);
		if (p->s_nchoices)
			fprintf(stderr," (Choices are:");
		for (i=0; i<p->s_nchoices; i++)
			fprintf(stderr," %s",p->s_choices[i]);
		if (p->s_nchoices)
			fprintf(stderr,")");
		fprintf(stderr,":");
	    fgets(ansbuf,SIZ,stdin);
	    if(BLANK) break;
	    if(goodint(ansbuf)) {
		p->s_value.ival = atoi(ansbuf);
		break;
	    }
	    fprintf(stderr,"Not a valid integer\n");
	}
	}
    }
    if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	fprintf(stderr,"getsym_i: symbol %s, value %d, taken from Common.\n",
	  name, p->s_value.ival);

    if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	fprintf(stderr,"getsym_i: symbol %s, value %d taken from param file.\n",
	  name, p->s_value.ival);

    if(p->s_prompt) p->s_prompt = 1;
    return p->s_value.ival;
}

int
getsymdef_i(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"getsymdef_i: name is NULL");
    
    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return 0;
    }
    else if (p->s_type != ST_INT) {
	badtype(name,"int");
	return 0;
    }
    return p->s_value.ival;
}

int
symsize(name)
char *name;
{
    struct symbol *p;
    int n = 0;
    struct inode *dp;
    
    spsassert(name,"symsize: name is NULL");
    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_IARRAY && p->s_type != ST_FARRAY) {
	badtype(name, "int array");
	return 1;
    }
    else {
	dp = p->s_value.ilist;
	while (dp) {
	    dp = dp->i_cdr;
	    n++;
	}
    }
    return n;
}

int
symdefinite(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"symdefinite: name is NULL");

    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    if(p->s_prompt)
	return 0;
    else
	return 1;
}

char **
symchoices(name,nchoices)
char *name;
int *nchoices;
{
    struct symbol *p;
    spsassert(name,"symchoices: name is NULL");

    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return NULL;
    }
    if (nchoices) 
	*nchoices = p->s_nchoices;
    if (p->s_choices) 
	return p->s_choices;
    else
	return NULL;
}

int
symrange(name,min, max)
char *name;
float *min, *max;
{
    struct symbol *p;
    spsassert(name,"symrange: name is NULL");
    spsassert(min, "symrange: min is NULL");
    spsassert(max, "symrange: max is NULL");
    
    p  = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    if (p->s_minmax) {
	*min = p->s_min;
	*max = p->s_max;
    }
    return p->s_minmax;
}
    
char *
symprompt(name)
char *name;
{
    struct symbol *p;
    spsassert(name,"symprompt: name is NULL");
    
    p  = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return NULL;
    }
    if (p->s_prompt)
	return p->s_pstring;
    else
	return NULL;
}


int
getsymdef_ia(name,arr,maxval)
char *name;
int arr[], maxval;
{
    struct symbol *p;
    int n = 0, *ap = arr;
    struct inode *dp;
    spsassert(name,"getsymdef_ia: name is NULL");
    spsassert(arr,"getsymdef_ia: arr is NULL");
    
    p  = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_IARRAY) {
	badtype(name, "int array");
	return -1;
    }
    else {
	dp = p->s_value.ilist;
	while (maxval-- && dp) {
	    *ap++ = dp->i_car;
	    dp = dp->i_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_ia: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
	return n;
    }
}

int
getsym_ia(name,arr,maxval)
char *name;
int arr[], maxval;
{
    struct symbol *p;
    int n = 0, *ap = arr;
    struct inode *dp;
    spsassert(name,"getsym_ia: name is NULL");
    spsassert(arr,"getsym_ia: arr is NULL");
    
    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_IARRAY) {
	badtype(name, "int array");
	return -1;
    }
    else {
	dp = p->s_value.ilist;
	while (maxval-- && dp) {
	    *ap++ = dp->i_car;
	    dp = dp->i_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_ia: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
/* If prompt is set, prompt for each argument */
	if (p->s_prompt == 2) {
          int i;
	  char *pr = prompt(p);

          global_prompt = p->s_prompt;
          if (check_tty_ok(&global_prompt)){
	    /* this has side effects, see below */
            p->s_prompt = global_prompt;

		    
	    for (i=0;i<n;i++) {
		while (1) {	/* loop till we get good input */
		    fprintf(stderr,"%s[%d] <%d>: ",pr,i,arr[i]);
		    fgets(ansbuf,SIZ,stdin);
		    if(BLANK) break;
		    if(goodint(ansbuf)) {
			arr[i] = atoi(ansbuf);
			break;
		    }
		    fprintf(stderr,"Not a valid integer\n");
		}
	    }
	  }
     }
        if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	  fprintf(stderr,"getsym_ia: symbol %s taken from Common.\n",
	  name);

        if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	  fprintf(stderr,"getsym_ia: symbol %s taken from param file.\n",
	  name);

        if(p->s_prompt) p->s_prompt = 1;
	return n;
    }
}

int
getsymdef_da(name,arr,maxval)
char *name;
double arr[];
int maxval;
{
    struct symbol *p;
    int n = 0;
    double *ap = arr;
    struct fnode *dp;
    spsassert(name,"getsymdef_da: name is NULL");
    spsassert(arr,"getsymdef_da: arr is NULL");
    
    p = get_symbol(name);
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_FARRAY) {
	badtype(name,"real array");
	return -1;
    }
    else {
	dp = p->s_value.flist;
	while (maxval-- && dp) {
	    *ap++ = dp->f_car;
	    dp = dp->f_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_da: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
	return n;
    }
}

int
getsym_da(name,arr,maxval)
char *name;
double arr[];
int maxval;
{
    struct symbol *p;
    int n = 0;
    double *ap = arr;
    struct fnode *dp;
    spsassert(name,"getsym_da: name is NULL");
    spsassert(arr,"getsym_da: arr is NULL");

    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_FARRAY) {
	badtype(name,"real array");
	return -1;
    }
    else {
	dp = p->s_value.flist;
	while (maxval-- && dp) {
	    *ap++ = dp->f_car;
	    dp = dp->f_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_da: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
	if (p->s_prompt == 2) {
	 int i;
	 char *pr = prompt(p);
         global_prompt = p->s_prompt;
         if (check_tty_ok(&global_prompt)){
	   /* this has side effects, see below */
            p->s_prompt = global_prompt;

   
	    for (i=0;i<n;i++) {
		while (1) {	/* loop till we get valid input */
		    fprintf(stderr,"%s[%d] <%f>: ",pr,i,arr[i]);
		    fgets(ansbuf,SIZ,stdin);
		    if(BLANK) break;
		    if(goodfloat(ansbuf)) {
			arr[i] = atof(ansbuf);
			break;
		    }
		    fprintf(stderr,"Not a valid real value\n");
		}
	    }
	}
	}
        if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	  fprintf(stderr,"getsym_da: symbol %s taken from Common.\n",
	  name);

        if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	  fprintf(stderr,"getsym_da: symbol %s taken from param file.\n",
	  name);

        if(p->s_prompt) p->s_prompt = 1;
	return n;
    }
}

int
getsymdef_fa(name,arr,maxval)
char *name;
float arr[];
int maxval;
{
    struct symbol *p;
    int n = 0;
    float *ap = arr;
    struct fnode *dp;
    spsassert(name,"getsymdef_fa: name is NULL");
    spsassert(arr,"getsymdef_fa: arr is NULL");
    
    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_FARRAY) {
	badtype(name,"real array");
	return -1;
    }
    else {
	dp = p->s_value.flist;
	while (maxval-- && dp) {
	    *ap++ = dp->f_car;
	    dp = dp->f_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_fa: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
	return n;
    }
}

int
getsym_fa(name,arr,maxval)
char *name;
float arr[];
int maxval;
{
    struct symbol *p;
    int n = 0;
    float *ap = arr;
    struct fnode *dp;
    spsassert(name,"getsym_fa: name is NULL");
    spsassert(arr,"getsym_fa: arr is NULL");
    
    p = get_symbol(name);
    
    if (p == NULL) {
	undef(name);
	return -1;
    }
    else if (p->s_type != ST_FARRAY) {
	badtype(name,"real array");
	return -1;
    }
    else {
	dp = p->s_value.flist;
	while (maxval-- && dp) {
	    *ap++ = dp->f_car;
	    dp = dp->f_cdr;
	    n++;
	}
	if (dp) {
	    if (Esps_verbose >= 0) 
	      fprintf(stderr,
	      "getsym_fa: some unread values remain in %s; only %d were returned.\n",
	      name, n);
	}
	if (p->s_prompt == 2) {
	  int i;
	   char *pr = prompt(p);
	    
           global_prompt = p->s_prompt;
           if (check_tty_ok(&global_prompt)){
	     /* this has side effects, see below */
            p->s_prompt = global_prompt;

	    for (i=0;i<n;i++) {
		while (1) {	/* loop till we get valid input */
		    fprintf(stderr,"%s[%d] <%f>: ",pr,i,arr[i]);
		    fgets(ansbuf,SIZ,stdin);
		    if(BLANK) break;
		    if(goodfloat(ansbuf)) {
			arr[i] = atof(ansbuf);
			break;
		    }
		    fprintf(stderr,"Not a valid real value\n");
		}
	    }
	}
	}
        if(!p->s_prompt && Esps_verbose >=1 && p->s_com)
	  fprintf(stderr,"getsym_fa: symbol %s taken from Common.\n",
	  name);

        if(!p->s_prompt && Esps_verbose >= 2 && !p->s_com)
	  fprintf(stderr,"getsym_fa: symbol %s taken from param file.\n",
	  name);

        if(p->s_prompt) p->s_prompt = 1;
	return n;
    }
}


static int st_errors = 0;

void
symerr_exit()
{
    if (st_errors) exit(1);
}

int sym_quiet = 0;		/* if 1, don't make undefined symbol an error */

static void
undef(name)
char *name;
{
    if (!sym_quiet) {
        fprintf(stderr,"Undefined symbol: %s\n",name);
        st_errors++;
    }
}
static void
badtype(name,type)
char *name,*type;
{
    fprintf(stderr,"Symbol %s is not '%s'\n",name,type);
    st_errors++;
}

static void
toolong(name)
char *name;
{
    fprintf(stderr,"Array %s is too long to fit into supplied buffer\n",name);
    st_errors++;
}

static int
goodint(s)
char *s;
{
    if(*s=='\0' || *s=='\n') return 0;
    if(*s=='+' || *s=='-') s++;
    while (isdigit(*s)) s++;
    return (*s == '\0' || *s == '\n');
}
/* Somewhat ugly floating point format checker. */
static int
goodfloat(s)
char *s;
{
    if(*s=='\0' || *s=='\n') return 0;
    if(*s=='+' || *s=='-') s++;
    if(*s=='e' || *s=='E') return 0;
    while (isdigit(*s)) s++;
    if(*s=='\0' || *s=='\n') return 1;
    if(*s=='.') {
	s++;
	while (isdigit(*s)) s++;
	if (*s == '\0' || *s == '\n') return 1;
    }
    if(*s!='e' && *s!='E') return 0;
    s++;
    if(*s=='+' || *s=='-') s++;
    if(!isdigit(*s)) return 0;
    while (*s && isdigit(*s)) s++;
    return (*s == '\0' || *s == '\n');
}

static int
check_tty_ok(ptr)
int *ptr;
{
        if(isatty(fileno(stdin)))
		return 1;
        else if(Esps_verbose >0 ) {
                Fprintf(stderr,"%s%s%s%s",
"read_params: This program is reading its data file from standard input,\n",
"read_params:  and a prompt request (?=) was found in the parameter file.\n",
"read_params:  Terminal input cannot be done in this case, so the default\n",
"read_params:  value (from the parameter file) will be used.\n");
        }
        else 
                Fprintf(stderr,
"read_params: Cannot prompt when standard input is not your terminal.\n");

	*ptr = 0;
	return 0;
}

