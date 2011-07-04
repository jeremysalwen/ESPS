/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:    Alan Parker, ESI, Washington, DC.
 * Checked by:
 * Revised by: David Burton
 *
 * Brief description: 
 *
 */
#ifndef lint
            static char *sccs_id = "@(#)vqclassify.c	1.4	6/16/94	ESI/ERL";
#endif


#include <stdio.h> 
#include <esps/esps.h> 
#include <esps/fea.h>
#include <esps/feadst.h> 
#include <esps/vq.h>

#define PROG "vqclassify"
#define SYNTAX USAGE("vqclassify [-x debug_level] [-m method] [-v] infile")

#define TAB_SIZE 100	/* initial tables size, it expands so
			   this is not a maximum, just the staring point */
#define SYMTAB_SIZE 1024 /* symbol table size, for signal_name and
			   source_name.  This is a fixed size table */


/* this structure is used to hold the distortion value and the
   source_name and is indexed by signal_name 
*/

struct sig_list {
	float average_dst;
	int source_name;
};

/* externals */
extern char *optarg;
extern int optind;
int getopt();
char *eopen();
#ifndef DEC_ALPHA
char *calloc(), *realloc();
#endif
int atoi();
int lin_search2();
char *savestring();
void exit(),  free();


/* globals */
int debug_level=0;
int tab_size = TAB_SIZE;
char *get_sym();
int symtab();
char *my_realloc();
void sort();




main(argc, argv)
int argc;
char **argv;
{

	struct feadst *in_rec=NULL;
	struct header *in_hd=NULL;
	FILE *in_strm=NULL;
	char *in_file=NULL;
	int verbose=0;		/* verbose flag */
	int i=0;
	int c;

	int num_records = 0;	/* number of input records */

	int num_src_names = 0;	/* number of unique source names */
	int num_sig_names = 0;	/* number of unique signal names */

/* the following tables correspond to the input records */
	int *source_names;	/* list of source names indexes */
	int *signal_names;	/* list of signal names indexes */
	float *average_dst;	/* list of average distortions */

/* these two tables are symbol tables for the character strings */
	char *sig_name_symtab[SYMTAB_SIZE];
	char *src_name_symtab[SYMTAB_SIZE];

	int *votes;		/* table of votes, indexed by source_name */
	struct sig_list *signal_list = NULL;
	int max;
	int winner;

	while ((c = getopt(argc, argv, "x:vm:")) != EOF) {
	 switch (c) {
		case 'x':
		 debug_level = atoi(optarg);
		 break;
		case 'v':
		 verbose=1;
		 break;
		case 'm':
		 if (strcmp(optarg, "vote") != 0) {
		  Fprintf(stderr,"vqclassify: only method vote is implemented\n");
		  exit(1);
		 }
		default:
		 SYNTAX;
	 }
	}
	
	if(argc-optind == 0) {
	 Fprintf(stderr,"vqclassify: no input file specified.\n");
	 SYNTAX;
	}


	in_file = eopen(PROG,argv[optind],"r",FT_FEA,FEA_DST,&in_hd,&in_strm);
	if(debug_level) Fprintf(stderr,"input file: %s\n",in_file);

	in_rec = allo_feadst_rec(in_hd);
	
	source_names = (int *)calloc((unsigned)tab_size, sizeof(char *));
	signal_names = (int *)calloc((unsigned)tab_size, sizeof(char *));
	average_dst = (float *)calloc((unsigned)tab_size, sizeof(float));
	assert(source_names);
	assert(signal_names);
	assert(average_dst);


/* read the records into the tables */
	while (get_feadst_rec(in_rec, in_hd, in_strm) != EOF) {
	 if(i>tab_size) {
	  tab_size = tab_size * 2;
	  source_names= (int *)realloc((char *)source_names,
	   (unsigned)(tab_size*sizeof(int)));
	  signal_names= (int *)realloc((char *)signal_names, 
	   (unsigned)(tab_size*sizeof(int)));
	  average_dst = (float *)realloc((char *)average_dst, 
	   (unsigned)(tab_size*sizeof(float)));
	 }
	 source_names[i] = 
	  symtab(in_rec->cbk_source,src_name_symtab,&num_src_names);
	 signal_names[i] = 
	  symtab(in_rec->cbk_signal,sig_name_symtab,&num_sig_names);
	 average_dst[i] = *in_rec->average_dst;
	 i++;
	}
	num_records = i;
	
	votes = (int *)calloc((unsigned)num_src_names, sizeof(int));
	assert(votes);

/* for each signal_name, get records of distortion vs source_name */

	for (i=0; i<num_sig_names; i++) {
	 int k=0,j;
	 for (j=0; j<num_records; j++) {
	  if (signal_names[j] == i) {
	 	signal_list = (struct sig_list *)
		 my_realloc((char *)signal_list,(unsigned)((k+1)*sizeof(struct sig_list)));
		assert(signal_list);
		signal_list[k].average_dst = average_dst[j];
		signal_list[k].source_name = source_names[j];
		k++;
	   }
	 }
	 sort(signal_list,k);
	 votes[signal_list[0].source_name]++;
	 if (verbose) {
		int l=0;
		(void)printf("\nSignal_name: %s\n", get_sym(sig_name_symtab,i));
		(void)printf("Source_name\tAvg Distortion\t\tRatio\n");
		while (l<k) {
		 if(signal_list[0].average_dst == 0) {
		  Fprintf(stderr,"Avg Distortion is zero for %s %f\n",
		   signal_list[0].source_name, get_sym(sig_name_symtab,i));
		  exit(1);
		 }
		 (void)printf("%s\t\t%f\t\t%f\n",get_sym(src_name_symtab,
		  signal_list[l].source_name),
		  signal_list[l].average_dst,
		  signal_list[l].average_dst/signal_list[0].average_dst);
		 l++;
		}
	 }
	 free((char *)signal_list);
	}

	winner=0;
	max = 0;
	if (verbose)
	 (void)printf("\nVotes:\n");
	for (i=0; i<num_src_names; i++) {
	 if (verbose) 
		(void)printf("%s\t%d\n",get_sym(src_name_symtab,source_names[i]), 
		 votes[i]);
	 if (votes[i] >= max) {
		max = votes[i];
	 }
	}

	for (i=0; i<num_src_names; i++)
	 if (votes[i] == max) winner++;

	if (winner > 1)
	 (void)printf("\nVote results in tie: ");
	if (verbose) 
	 (void)printf("\nThe source name is: ");

	for (i=0; i<num_src_names; i++)
	 if (votes[i] == max)
	  (void)printf("%s ",get_sym(src_name_symtab,source_names[i]));
	(void)printf("\n");
	return 0;
}


int
symtab(word, table, count)
char *word;
char *table[];
int *count;
{
	int k;
	assert(word);
	assert(table);
	assert(count);

	if (*count >= SYMTAB_SIZE) {
	 Fprintf(stderr,"%s: symbol table overflow\n",PROG);
	 exit(1);
	}
	table[*count] = NULL;	/* be sure lin_search works */
	if ((k = lin_search2(table, word)) == -1) {
	 table[*count] = savestring(word);
	 *count += 1;
	 return *count-1;
	}
	else {
	 return k;
	}
}

char *
get_sym(table, index)
char *table[];
int index;
{
	assert(table);
	if (idx_ok(index, table))
	 return table[index];
	else
	 return "NOT IN SYMBOL TABLE";
}

char *
my_realloc(ptr, size)
char *ptr;
unsigned size;

{
	if (ptr == NULL)
	 return calloc((unsigned)1,(unsigned)size);
	else
	 return realloc(ptr,(unsigned)size);
}

void
sort(list, n)
struct sig_list *list;
int n;
{
	struct sig_list temp;
	int i, gap, j;

	for (gap = n/2; gap > 0; gap /= 2)
	 for (i = gap; i<n; i++)
		for (j=i-gap; j>=0 && 
		 list[j].average_dst>list[j+gap].average_dst; j-=gap) {
			temp = list[j];
			list[j] = list[j+gap];
			list[j+gap] = temp;
		}
}


