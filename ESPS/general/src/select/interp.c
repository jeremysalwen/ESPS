/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*/

 	
#ifndef lint
	char *interp_sccs = "@(#)interp.c	3.14	11/11/94 ESI";
#endif

#include "select.h"
#include "interp.h"
#include "y.tab.h"

#define SN (short*)NULL
#define LLN (long **)NULL


/* this code is based on the hoc interpreter in "Unix Programming
Environment", by Kerningham and Pike */

extern int debug_level;

#define STACKSIZ 256
static 	Stack 	stack[STACKSIZ];	/* interpreter stack */
static 	Stack 	*stackptr;		/* next free spot on stack */

#define PROGSIZ 1000
Inst 	prog[PROGSIZ];			/* the interpreter program memory */
Inst 	*progptr;			/* next free spot in memory */
Inst 	*pc;				/* program counter */

int	run_error;			/* error detected during run */
int	fatal_error;			/* bad error ends select */


void
initcode()
{
	if (debug_level >1) print_func();
	if (debug_level) Fprintf(stderr,"initcode called\n");
	stackptr = stack;
	progptr = prog;
}

void
push(data)
Stack data;
{
	if (stackptr >= &stack[STACKSIZ]) {
		errmsg("Stack overflow handling query expression\n");
		errmsg("Internal error; contact maintainer.\n");
		exit(1);
	}

	*stackptr++ = data;
}

Stack
pop()
{
	if (stackptr <= stack) {
		errmsg("Stack underflow handling query expression\n");
		errmsg("Internal error; contact maintainer.\n");
		exit(1);
	}
	return *--stackptr;
}

void
code(instr)
int (*instr)();
{
	if (debug_level)
		Fprintf(stderr,"code: called with %x.\n",instr);
	if (progptr >= &prog[PROGSIZ]) {
		errmsg("Query expression too complex\n");
		errmsg("Internal error; contact maintainer.\n");
		exit(1);
	}
	progptr->inst = instr;
	progptr++;
	return;
}

void
operand(type,c,d)
int type;
char *c;
double d;
{
	if (debug_level) {
		Fprintf(stderr,"operand: called with code %d.\n",code);
		if(type == CHAR)
		 Fprintf(stderr,"--string %s\n",c);
		else
		 Fprintf(stderr,"--double %lf\n",d);
	}
	if (progptr >= &prog[PROGSIZ]) {
		errmsg("Query expression too complex\n");
		errmsg("Internal error; contact maintainer.\n");
		exit(1);
	}
	if (type == CHAR) {
	 	progptr->sval = c;
		progptr->dval = 0;
		progptr->type = CHAR;
	}
	else if (type == DOUBLE) {
		progptr->dval = d;
		progptr->sval = NULL;
		progptr->type = DOUBLE;
	} else {
		errmsg("Internal error in operand, bad type\n");
		errmsg("Internal error; contact maintainer.\n");
		exit(1);
	}

	progptr->inst = NOOP;
	progptr++;
	return;
}

double
run_interp()
{
	if(debug_level > 1) {
	  for (pc = prog; pc->inst != STOP;) {
		Fprintf(stderr,"prog: i:%x f:%lf s:%s\n",
			pc->inst, pc->dval, pc->sval);
		pc++;
	  }
	  return 0;
	}
	
	stackptr = stack;
	run_error = 0;
	fatal_error = 0;
	for (pc = prog; pc->inst != STOP && !run_error && !fatal_error;)
		{pc++; (*((pc-1)->inst))();}
	if (run_error || fatal_error)  {
		errmsg("expression aborted.\n");
		return 0;
	}
	else
		return stack[0].dval;
}

int
Const()	
{
	Stack data;
	if (debug_level) Fprintf(stderr,"Const: called\n");
	if (pc->type == CHAR) {
		data.sval = (pc++)->sval;
		data.dval = 0;
		data.type = CHAR;
	}
	else {
		data.dval = (pc++)->dval;
		data.sval = NULL;
		data.type = DOUBLE;
	}
	push(data);
}

int
add()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"add: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = d1.dval += d2.dval;
	d1.type = DOUBLE;
	push(d1);
}

int
sub()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"sub: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = d1.dval -= d2.dval;
	d1.type = DOUBLE;
	push(d1);
}

int
mul()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"mul: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = d1.dval *= d2.dval;
	d1.type = DOUBLE;
	push(d1);
}

int
divide()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"divide: called\n");
	d2 = pop();
	d1 = pop();
	if (d2.dval == 0.0) {
		errmsg("Divide by zero; ");
		run_error++;
	}
	else {
		d1.dval = d1.dval /= d2.dval;
		d1.type = DOUBLE;
		push(d1);
	}
}


int
power()
{
	extern double Pow();
	Stack d1, d2;
	if (debug_level) Fprintf(stderr,"power: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = Pow(d1.dval, d2.dval);
	push(d1);
}

int
negate()
{
	Stack d1;
	if (debug_level) Fprintf(stderr,"negate: called\n");
	d1 = pop();
	d1.dval = -d1.dval;
	push(d1);
}

int
le()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"le: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval <= d2.dval);
	d1.type = DOUBLE;
	push(d1);
}

int
and()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"and: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval && (d2.dval != 0));
	d1.type = DOUBLE;
	push(d1);
}

int
or()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"or: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval || d2.dval);
	d1.type = DOUBLE;
	push(d1);
}

int
lt()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"lt: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval < d2.dval);
	d1.type = DOUBLE;
	push(d1);
}

int
gt()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"gt: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval > d2.dval);
	d1.type = DOUBLE;
	push(d1);
}

int
eq()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"eq: called\n");
	d2 = pop();
	d1 = pop();
	if (d2.type == CHAR && d1.type == CODED) 
		d1.dval = cmp_coded(d2.sval,d1.sval,(short)d1.dval);

	else if (d1.type == CHAR && d2.type == CODED)
		d1.dval = cmp_coded(d1.sval,d2.sval,(short)d2.dval);

	else if (d1.type == CHAR && d2.type == CHAR)
		d1.dval = (strcmp(d1.sval,d2.sval) == 0);

	else
		d1.dval = (double)(d1.dval == d2.dval);

	d1.type = DOUBLE;
	push(d1);
}

int
ne()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"ne: called\n");
	d2 = pop();
	d1 = pop();
	if (d2.type == CHAR && d1.type == CODED) 
		d1.dval = !cmp_coded(d2.sval,d1.sval,(short)d1.dval);

	else if (d1.type == CHAR && d2.type == CODED)
		d1.dval = !cmp_coded(d1.sval,d2.sval,(short)d2.dval);

	else if (d1.type == CHAR && d2.type == CHAR)
		d1.dval = (strcmp(d1.sval,d2.sval) != 0);

	else
		d1.dval = (double)(d1.dval != d2.dval);
	d1.type = DOUBLE;
	push(d1);
}

int
ge()
{
	Stack d1,d2;
	if (debug_level) Fprintf(stderr,"ge: called\n");
	d2 = pop();
	d1 = pop();
	d1.dval = (double)(d1.dval >= d2.dval);
	d1.type = DOUBLE;
	push(d1);
}


int
not()
{
	Stack d1;
	if (debug_level) Fprintf(stderr,"not: called\n");
	d1 = pop();
	d1.dval = (double)(!d1.dval);
	d1.type = DOUBLE;
	push(d1);
}

extern struct fea_data *query_rec;
extern struct header *query_hd;
extern char *query_file;
extern long data_ptr;
extern int gf_flag;

int
getfld()
{
	Stack d1;
	long real_index, *dimen, size, index, *lptr;
	short type, rank, *sptr;
	float *fptr;
	double *dptr;
	char *bptr;
	char *ptr, *name;
	double_cplx *dcptr;
	float_cplx *fcptr;
	long_cplx *lcptr;
	short_cplx *scptr;
	byte_cplx *bcptr;
	

	assert(query_rec != NULL);
	assert(query_hd != NULL);
	
	name = (pc++)->sval;
	index = (long)(pc++)->dval;
	d1.type = DOUBLE;	/* default type */
	if (debug_level)
	 Fprintf(stderr,"getfld: name: %s, index: %d\n",name,index);
	if (strcmp(name,"tag") == 0) {
	  d1.dval = query_rec->tag;
	  push(d1);
	  return;
	}
	if (strcmp(name,"FILE") == 0) {
	  d1.sval = query_file;
	  d1.type = CHAR;
	  push(d1);
	  return;
	}
	if (strcmp(name,"REC") == 0) {
	  d1.dval = data_ptr;
	  push(d1);
	  return;
	}
	type = get_fea_type(name,query_hd);
	if (type == UNDEF) {
	  if(!gf_flag) {
	    errmsg2("Warning, field %s not in file %s.\n",name, query_file);
	    errmsg ("Zero returned as a value.\n");
	  }
	  d1.dval = 0;
	  push(d1);
	  return;
	}
	if (is_field_complex(query_hd,name)) {
	  if(!gf_flag) {
	    errmsg1("Warning, field %s is complex.\n",name);
	    errmsg ("Real part returned as value.\n");
	  }
	}
	ptr = get_fea_ptr(query_rec,name,query_hd);
	assert(ptr != NULL);
	size = get_fea_siz(name,query_hd,&rank,&dimen);
	assert(size != 0);
	if(index != -1 && index > size-1) {
	  if(!gf_flag) {
	   errmsg4(
	    "Index %ld out of range for field %s in file %s, its size is %ld\n",
            index,name, query_file, size);
	   errmsg ("Zero returned as a value.\n");
	  }
	  d1.dval = 0;
	  push(d1);
	  return;
	}
	real_index = (index == -1) ? 0 : index;
	d1.type = DOUBLE;
	d1.sval = NULL;
	switch (type) {
	case DOUBLE_CPLX:
	  dcptr = (double_cplx *)ptr;
	  d1.dval = dcptr[real_index].real;
	  break;
	case FLOAT_CPLX:
	  fcptr = (float_cplx *)ptr;
	  d1.dval = fcptr[real_index].real;
	  break;
	case LONG_CPLX:
	  lcptr = (long_cplx *)ptr;
	  d1.dval = lcptr[real_index].real;
	  break;
	case SHORT_CPLX:
	  scptr = (short_cplx *)ptr;
	  d1.dval = scptr[real_index].real;
	  break;
	case BYTE_CPLX:
	  bcptr = (byte_cplx *)ptr;
	  d1.dval = bcptr[real_index].real;
	  break;
	case DOUBLE:
	  dptr = (double *)ptr;
	  d1.dval = dptr[real_index];
	  break;
	case FLOAT:
	  fptr = (float *)ptr;
	  d1.dval = fptr[real_index];
	  break;
	case LONG:
	  lptr = (long *)ptr;
	  d1.dval = lptr[real_index];
	  break;
	case CODED:
	  sptr = (short *)ptr;
	  d1.dval = sptr[real_index];
	  d1.type = CODED;
	  d1.sval = name;
	  break;
	case SHORT:
	  sptr = (short *)ptr;
	  d1.dval = sptr[real_index];
	  break;
	case BYTE:
	  bptr = ptr;
	  d1.dval = bptr[real_index];
	  break;
	case CHAR:
	  if(index == -1) {
	     d1.sval = ptr;	/* when using CHAR, be sure dval is zero */
	     d1.type = CHAR;
	  }
	  else
	     d1.dval = ptr[real_index];
	  break;
	}
	push(d1);
}
	
	 

print_func()
{
	Fprintf(stderr,"and %x\n",and);
	Fprintf(stderr,"or %x\n",or);
	Fprintf(stderr,"le %x\n",le);
	Fprintf(stderr,"ge %x\n",ge);
	Fprintf(stderr,"eq %x\n",eq);
	Fprintf(stderr,"not %x\n",not);
	Fprintf(stderr,"getfld %x\n",getfld);
	Fprintf(stderr,"Const %x\n",Const);
	Fprintf(stderr,"gt %x\n",gt);
	Fprintf(stderr,"lt %x\n",lt);
}

extern int cflag;
short fea_encode();

int
cmp_coded(s1,s2,value)
char *s1,*s2;
short value;
{

	if (cflag)
		return (fea_encode_ci(s1,s2,query_hd) == value);
	else
		return (fea_encode(s1,s2,query_hd) == value);
}

char template[20] = "/tmp/selfXXXXX";

int
func1()
{
	Stack d1;
	char *name, command[80];
	FILE *out_fp, *in_fp;
	int stat;
	

	assert(query_rec != NULL);
	assert(query_hd != NULL);

	name = (pc++)->sval;
	if (debug_level) Fprintf(stderr,"func1: called, name: %s ",name);
	out_fp = fopen(mktemp(template),"w");
	assert(out_fp != NULL);
	
	write_header(query_hd, out_fp);
	put_fea_rec(query_rec, query_hd, out_fp);
	(void)fclose(out_fp);

	(void)sprintf(command, "%s %s", name, template);
	in_fp = popen(command, "r");
	if(in_fp == NULL) {
		errmsg2("Problem running function %s using [%s]\n",
		 name, command);
		fatal_error++;
		d1.dval = 0;
	}
	else {
		(void)fscanf(in_fp,"%lf",&d1.dval);
		if (debug_level) Fprintf(stderr,",value: %lf\n",d1.dval);
		if ((stat = pclose(in_fp)) != 0) 
			errmsg2("program %s returned non-zero exit code: %d\n",
			  name, stat);
	}
	(void)unlink(template);
	push(d1);
}

extern double Log(), Exp(), Log10(), Sqrt(), integer();


struct func_tab {
	char *name;
	double (*func)();
};
static struct func_tab bltin_tab[] = {
	"sin", sin,
	"cos", cos,
	"atan", atan,
	"tan", tan,
	"log", Log,
	"log10", Log10,
	"exp", Exp,
	"sqrt", Sqrt,
	"int", integer,
	"abs", fabs,
	0 , 0
};

double (*blt_lookup(name))()
char *name;
{
	int i=0;
	while (bltin_tab[i++].name != NULL)
		if (strcmp(bltin_tab[i-1].name,name) == 0) 
			return bltin_tab[i-1].func;
	return NULL;

}
	
			
int
bltin()
{
	char *name;
	Stack d1;
	double (*i_ptr)();

	name = (pc++)->sval;
	if ((i_ptr = blt_lookup(name)) == NULL) {
		errmsg1("Unknown expression function %s; ",name);
		fatal_error++;
	}
	else {
		d1 = pop();
		d1.dval = (*i_ptr)(d1.dval);
		push(d1);
	}
}

double sum_f2(), mean_f2(), prod_f2(), min_f2(), max_f2(), size_f2();

static struct func_tab f2_tab[] = {
	"sum", sum_f2,
	"mean", mean_f2,
	"prod", prod_f2,
	"min", min_f2,
	"max", max_f2,
	"size", size_f2,
	0, 0
};

double (*f2_lookup(name))()
char *name;
{
	int i=0;
	while (f2_tab[i++].name != NULL)
	 if (strcmp(f2_tab[i-1].name, name) == 0)
		return f2_tab[i-1].func;
	return NULL;
}

int
func2()
{
	Stack d1;
	char *fieldname, *funcname;
	long siz;
	double (*i_ptr)();
	double *get_fea_db();

	funcname = (pc++)->sval;
	fieldname = (pc++)->sval;

	if ((siz=get_fea_siz(fieldname,query_hd, SN, LLN)) == 0) {
		if(!gf_flag) {
	  	  errmsg2("Function %s called on non-existent field %s; ",
	    	   funcname, fieldname);
		  errmsg("Zero returned as value.\n");
		}
		d1.dval = 0;
		push(d1);
		return;
	}
	if ((i_ptr = f2_lookup(funcname)) == NULL)
		if ((i_ptr = blt_lookup(funcname)) == NULL) {
			errmsg1("Unknown function %s; ", funcname);
			fatal_error++;
			return;
		} 
		else {  /* its not a field function, but a value function */
			d1.dval=(*i_ptr)(*get_fea_db(query_rec,fieldname,
				query_hd));
			push(d1);
			return;
		}

	if (siz == 1 && !gf_flag) 
		errmsg3("Warning, field %s in file %s is a scaler being used with %s.\n",
		 fieldname,query_file,funcname);

	d1.dval = (*i_ptr)(fieldname, siz);
	push(d1);
}

double *
get_fea_db(rec, name, hd)
struct fea_data *rec;
char *name;
struct header *hd;
{
	unsigned siz,i; 
	char *ptr;
	double *data;
	float *fptr;
	long *lptr;
	short *sptr, type;
	
	
	siz = get_fea_siz(name, hd, SN, LLN);
	assert(siz != 0);
	data = (double *)calloc(siz, sizeof(double));
	type = get_fea_type(name, hd);
	ptr = get_fea_ptr(rec, name, hd);

	switch (type) {
	 case DOUBLE:
		free((char *)data); 
		return (double *)ptr;
	 case FLOAT:
		fptr = (float *)ptr;
		for (i=0; i<siz; i++)
		  data[i] = fptr[i];
		break;
	 case LONG:
		lptr = (long *)ptr;
		for (i=0; i<siz; i++)
		  data[i] = lptr[i];
		break;
	 case SHORT:
		sptr = (short *)ptr;
		for (i=0; i<siz; i++)
		  data[i] = sptr[i];
		break;
	 case CHAR:
	 case BYTE:
		for (i=0; i<siz; i++)
		  data[i] = ptr[i];
		break;
	}
	
	return data;
}

double
sum_f2(name, siz)
char *name;
long siz;
{
	double *data, sum=0;
	long i;
	
	data = get_fea_db(query_rec, name, query_hd);
	for (i=0; i<siz; i++)
		sum += data[i];
	if (get_fea_type(name,query_hd) != DOUBLE) free((char *)data);
	return sum;
}

double
mean_f2(name, siz)
char *name;
long siz;
{
	double *data, sum=0;
	long i;
	
	data = get_fea_db(query_rec, name, query_hd);
	for (i=0; i<siz; i++)
		sum += data[i];
	if (get_fea_type(name,query_hd) != DOUBLE) free((char *)data);
	return sum/siz;
}

double
prod_f2(name, siz)
char *name;
long siz;
{
	double *data, prod=1;
	long i;
	
	data = get_fea_db(query_rec, name, query_hd);
	for (i=0; i<siz; i++)
		prod *= data[i];
	if (get_fea_type(name,query_hd) != DOUBLE) free((char *)data);
	return prod;
}

double
min_f2(name, siz)
char *name;
long siz;
{
	double *data, min;
	long i;
	
	data = get_fea_db(query_rec, name, query_hd);
	min = data[0];
	for (i=0; i<siz; i++)
		if (data[i] < min) min = data[i];
	if (get_fea_type(name,query_hd) != DOUBLE) free((char *)data);
	return min;
}

double
max_f2(name, siz)
char *name;
long siz;
{
	double *data, max;
	long i;
	
	data = get_fea_db(query_rec, name, query_hd);
	max = data[0];
	for (i=0; i<siz; i++)
		if (data[i] > max) max = data[i];
	if (get_fea_type(name,query_hd) != DOUBLE) free((char *)data);
	return max;
}

/* ARGSUSED */

double
size_f2(name, siz)
char *name;	
long siz;
{
	return siz;
}
