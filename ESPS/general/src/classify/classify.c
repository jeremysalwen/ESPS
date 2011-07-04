/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
| 									|
|  Program:	main.c							|
| 									|
|  Written by:  Rodney Johnson.						|
|  Checked by:								|
| 									|
|  Main module of classify.						|
|									|
|  Classifies records in an ESPS feature file by the maximum-likeli-	|
|  hood method, according to information in a statistics file.		|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)classify.c	3.10	6/9/93	ESI";
#endif
#define VERSION "3.10"
#define DATE "6/9/93"
/*
 * system include files
 */
#include <stdio.h>
#include <sys/types.h>
/*
 * ESPS include files
 */
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/spsassert.h>
#include <esps/feastat.h>
/*
 * defines
 */
#define DEBUG(n) if (debug_level >= n) Fprintf
#define Fprintf (void)fprintf
#define PROG Fprintf(stderr, "%s: ", ProgName)
#define EXIT Fprintf(stderr, "\n"); exit(1);
#define ERROR_EXIT(text) {PROG; Fprintf(stderr, (text)); EXIT}
#define ERROR_EXIT1(fmt,a) {PROG; Fprintf(stderr, (fmt), (a)); EXIT}
#define ERROR_EXIT2(fmt,a,b) {PROG; Fprintf(stderr, (fmt), (a), (b)); EXIT}
#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num),sizeof(type))) == NULL) \
    ERROR_EXIT1("Can't allocate memory--%s", (msg))}
#define SYNTAX \
USAGE("classify [-P param_file][-d][-e elements][-f field][-h file.his][-x debug_level]\n [-C field][-L field] input.stat input.fea output.fea") \
;
/*
 * system functions and variables
 */
extern int  optind;
extern char *optarg;
time_t	time();
char	*ctime();
/*
 * external ESPS functions
 */
char	*get_cmd_line();
void	write_header();
struct header
	*read_header();
int	lin_search();
void	add_comment();
char	*fea_decode();
long	*grange_switch();
char	**get_fea_deriv();
long	get_fea_siz();
short	get_fea_type();
char	*get_deriv_vec();
void	copy_fea_rec();
/*
 * global function declarations
 */
void	pr_farray();
int	classify();
int	num_enums();
/*
 * global variable declarations
 */
int     debug_level = 0;       
char    *ProgName = "classify";
char    *Version = VERSION;
char	*Date = DATE;
char	*cmd_line;
/*
 * main program
 */
main(argc, argv)
    int	    argc;
    char    **argv;
{
    char    *statfield = NULL;	    /* Field name.  Get features from this */
				    /* field or its source fields. */
    long    feasize;		    /* Size of named field in input. */
    long    statsize;		    /* Field size from statistics file. */
    long    *elements;		    /* Indices specified with -e option. */
    long    num_elems;		    /* Number of indices specified. */
    char    *e_arg = NULL;	    /* Argument of -e option. */
    short   d_flag = NO,	    /* -d option specified? */
	    e_flag = NO,	    /* -e option specified? */
	    f_flag = NO,	    /* -f option specified? */
            C_flag = NO,
            L_flag = NO;

    char    *param_file = NULL;
    char    *hist_name = NULL;	    /* History-file name. */
    FILE    *hist_strm;		    /* History file. */
    time_t  tloc;		    /* Current time. */

    char    *stat_name;		    /* Statistics-file name. */
    FILE    *stat_strm;		    /* Statistics file. */
    struct header   *stat_ih;	    /* Statistics-file header. */
    struct feastat  *stat_rec,	    /* Statistics-file record. */
	    **stat_recs;	    /* All statistics-file records. */
    float   **means;		    /* Mean vectors from statistics file. */
    float   ***invcovars;	    /* Inverse covariance matrices from */
				    /* statistics file. */
    int	    max_clas,		    /* Upper bound on number of classes. */
	    num_clas;		    /* Actual number of classes. */
    char    **fields;		    /* Source-field definitions. */

    char    *infea_name;	    /* Name of input feature file. */
    FILE    *infea_strm;	    /* Input feature file. */
    struct header   *infea_ih;	    /* Input feature-file header. */
    struct fea_data *infea_rec;	    /* Input feature-file record. */
    char    *fea_ptr;		    /* Pointer to data in input record. */
    float   *feavec = NULL;	    /* Features from input record. */
    long    length;		    /* Length of feavec. */

    char    *outfea_name;	    /* Output feature-file name. */
    FILE    *outfea_strm;	    /* Output feature-file.  */
    struct header   *outfea_oh;	    /* Output feature-file header. */
    struct fea_data *outfea_rec;    /* Output feature-file record. */
    char    **enums;		    /* Class names for output header. */
    char    *clasfield = "class";   /* Output field name for class. */
    short   *class;		    /* Pointer to output field for class. */
    char    *postfield = "posteriors";
				    /* Output field name for likelihoods. */
    float   *posteriors;	    /* Output vector for likelihoods. */
    long    *count;		    /* Count records in each class. */
    long    nrec;		    /* Number of records. */

    int	    c;			    /* Input option letter. */
    int	    i, j;		    /* Loop indices. */

    cmd_line = get_cmd_line(argc, argv);
/*
 * process command line options
 */
    while ((c = getopt(argc, argv, "de:f:h:x:C:L:P:")) != EOF)
    {
	switch (c)
	{
	case 'P':
	  param_file = optarg;
	  break;
	case 'd':
	    d_flag = YES;
	    break;
	case 'e':
	    e_flag = YES;
	    elements = grange_switch(e_arg = optarg, &num_elems);
	    break;
	case 'f':
	    f_flag = YES;
	    statfield = optarg;
	    break;
	case 'h':
	    hist_name = optarg;
	    break;
	case 'x': 
	    debug_level = atoi(optarg);
	    break;
	case 'C':
	    clasfield = optarg;
	    C_flag = YES;
	    break;
	case 'L':
	    postfield = optarg;
	    L_flag = YES;
	    break;
	default:
	    SYNTAX
	    break;
	}
    }
    if (d_flag && (e_flag || f_flag))
	ERROR_EXIT("The -d option is incompatible with -e and -f.")

    (void) read_params(param_file, SC_NOCOMMON, (char *)NULL);

    if(!d_flag && !e_flag && symtype("elements") != ST_UNDEF){
      e_arg = getsym_s("elements");
      elements = grange_switch(e_arg, &num_elems);
      e_flag = YES;
    }
    if(!d_flag && !f_flag && symtype("in_field") != ST_UNDEF){
      statfield = getsym_s("in_field");
      f_flag = YES;
    }
    if(!C_flag && symtype("class_fld_name") != ST_UNDEF)
      clasfield = getsym_s("class_fld_name");
    if(!L_flag && symtype("like_fld_name") != ST_UNDEF)
      postfield = getsym_s("like_fld_name");

/*
 * process file arguments
 */
    if (optind != argc - 3) SYNTAX

    stat_name = argv[optind++];
    if (strcmp(stat_name, "-") == 0)
    {
	stat_name = "<stdin>";
	stat_strm = stdin;
    }
    else
	TRYOPEN(ProgName, stat_name, "r", stat_strm);
    if ((stat_ih = read_header(stat_strm)) == NULL)
	NOTSPS(ProgName, stat_name)
	;
    if (stat_ih->common.type != FT_FEA
	    || stat_ih->hd.fea->fea_type != FEA_STAT)
	ERROR_EXIT1("%s is not an ESPS statistics file.", stat_name)

    infea_name = argv[optind++];
    if (strcmp(infea_name, "-") == 0)
    {
	if (stat_strm == stdin)
	    ERROR_EXIT("Input files can't both be standard input.")
	infea_name = "<stdin>";
	infea_strm = stdin;
    }
    else
	TRYOPEN(ProgName, infea_name, "r", infea_strm);
    if ((infea_ih = read_header(infea_strm)) == NULL)
	NOTSPS(ProgName, infea_name)
    ;
    if (infea_ih->common.type != FT_FEA)
	ERROR_EXIT1("%s is not an ESPS feature file.", infea_name)

    outfea_name = argv[optind++];
    if (strcmp(outfea_name, "-") == 0)
    {
	outfea_name = "<stdout>";
	outfea_strm = stdout;
    }
    else
	TRYOPEN(ProgName, outfea_name, "w", outfea_strm);
/*
 * open optional history file and initial output
 */
    if (hist_name != NULL)
    {
	TRYOPEN(ProgName, hist_name, "w", hist_strm);
	tloc = time((long *) NULL);
	Fprintf(hist_strm, "Classify history output on %s", ctime(&tloc));
	Fprintf(hist_strm, "Classify version %s of %s\n", Version, Date);
	Fprintf(hist_strm, "Command line:\n%s", cmd_line);
    }
/*
 * Check for complex valued fields; these not supported yet
*/
    if (is_field_complex(stat_ih, "invcovar") == YES){
      Fprintf(stderr, 
      "classify: Complex inverse covariance matrix not supported - check %s\n",
	      stat_name);
      exit(1);
    }
    if (is_field_complex(stat_ih, "mean") == YES){
      Fprintf(stderr, 
      "classify: Complex valued mean vectors not supported yet - check %s\n",
	      stat_name);
      exit(1);
    }
 
    if(f_flag && is_field_complex(infea_ih, statfield) == YES){
      Fprintf(stderr, 
      "classify: Complex valued fields not supported yet - check %s in %s\n", 
	      statfield, infea_name);
      exit(1);
    }
/*
 * read statistics records
 */
    if (get_fea_siz("invcovar", stat_ih, (short *) NULL, (long **) NULL)
	<= 0) ERROR_EXIT(
	    "Statistics file does not have inverse covariance matrices.")
    max_clas = num_enums("class", stat_ih);
    TRYALLOC(struct feastat *, max_clas, stat_recs, 
	    "statistics record pointers.")
    ;
    stat_rec = allo_feastat_rec(stat_ih);
    num_clas = 0;
    while (get_feastat_rec(stat_rec, stat_ih, stat_strm) != EOF
	    && num_clas < max_clas)
    {
	stat_recs[num_clas++] = stat_rec;
	stat_rec = allo_feastat_rec(stat_ih);
    }
    if (num_clas == 0) ERROR_EXIT("Empty statistics file.")

    DEBUG(1)(stderr, "Read %d statistics records.\n", num_clas);

    for (i = 0; i < num_clas; i++)
	for (j = 0; j < i; j++)
	if (*stat_recs[i]->class == *stat_recs[j]->class)
	ERROR_EXIT("Duplicate classes in statistics file.")
    TRYALLOC(float *, num_clas, means, "mean-vector pointers.")
    TRYALLOC(float **, num_clas, invcovars,
	    "inverse-covariance-matrix pointers.")
    TRYALLOC(long, num_clas, count, "count.")
    for (i = 0; i < num_clas; i++)
    {
      j = *stat_recs[i]->class;
      means[j] = stat_recs[i]->mean;
      invcovars[j] = stat_recs[i]->invcovar;
      count[j] = 0;
    }
/*
 * interpret options & check consistency
 */
    statsize = *(long *) get_genhd("statsize", stat_ih);
    if (!f_flag)
	statfield = get_genhd("statfield", stat_ih);

    DEBUG(1)(stderr, "statsize = %ld.  statfield = \"%s\".\n",
	    statsize, statfield);

    if (!d_flag)
    {
	feasize =
	    get_fea_siz(statfield, infea_ih, (short *) NULL, (long **) NULL);
	if(!f_flag && !e_flag && feasize <= 0)
	    d_flag = YES;
    }

    if (d_flag)	    /* will use derived-field machinery to get features */
    {
	if ((fields = get_fea_deriv(statfield, stat_ih->variable.refhd))
		== NULL)
	    ERROR_EXIT1( "%s is not a derived field in reference header.",
		    statfield);
/*!*//* check sum of sizes of source fields against statsize */
	if (debug_level >= 1)
	{
	    Fprintf(stderr,  "Using derived-field mechanism.  Fields:\n");
	    for (i = 0; fields[i] != NULL; i++)
		Fprintf(stderr, "\t\"%s\"\n", fields[i]);
	}
	if (hist_name != NULL)
	{
	    Fprintf(hist_strm, "Source fields of derived field %s:\n",
		    statfield);
	    for (i = 0; fields[i] != NULL; i++)
		Fprintf(hist_strm, "\t%s\n", fields[i]);
	}
    }
    else	    /* will get features from field named in statfield */
    {
	if (feasize <= 0)
	    ERROR_EXIT1("No field %s in input file.", statfield);
	if (e_flag)
	{
	    for (i = 0; i < num_elems; i++)
		if (elements[i] >= feasize)
		    ERROR_EXIT2("No element %ld in field %s.",
			    elements[i], statfield);
	    if (statsize != num_elems)
		ERROR_EXIT2("%ld input features--should be %ld.\n",
			num_elems, statsize);
	    if (hist_name != NULL)
		Fprintf(hist_strm, "Using field %s[%s].\n",
			statfield, e_arg);
	}
	else
	{
	    if (statsize != feasize)
		ERROR_EXIT2("%ld input features--should be %ld.\n",
			feasize, statsize);
	    if (hist_name != NULL)
		Fprintf(hist_strm, "Using field %s.\n", statfield);
	}
	DEBUG(1)(stderr, "Not using derived-field mechanism.\n");
    }
/*
 * make and write output-file header
 */
    outfea_oh = copy_header(infea_ih);
    add_comment(outfea_oh, cmd_line);
    Strcpy(outfea_oh->common.prog, ProgName);
    Strcpy(outfea_oh->common.vers, Version);
    Strcpy(outfea_oh->common.progdate, Date);
    add_source_file(outfea_oh, stat_name, stat_ih);
    add_source_file(outfea_oh, infea_name, infea_ih);
    TRYALLOC(char *, num_clas+1, enums, "enums array for output header")
    for (i = 0; i < num_clas; i++)
    {
      j = *stat_recs[i]->class;
      enums[j] = fea_decode(j, "class", stat_ih);
    }
    enums[num_clas] = (char *) NULL;
    if (add_fea_fld(clasfield, 1L, 0, (long *) NULL, CODED,
		enums, outfea_oh) != 0)
	ERROR_EXIT1("Can't add field %s to output header", clasfield)
    if (add_fea_fld(postfield, (long) num_clas, 1, (long *) NULL, FLOAT,
		(char **) NULL, outfea_oh) != 0)
	ERROR_EXIT1("Can't add field %s to output header", postfield)
    (void)update_waves_gen(infea_ih, outfea_oh, 1.0, 1.0);

    if( e_flag )
      (void)add_genhd_c("elements",e_arg, strlen(e_arg), outfea_oh);
    if(f_flag)
      (void)add_genhd_c("in_field",statfield, strlen(statfield), outfea_oh);
    write_header(outfea_oh, outfea_strm);
/*
 * allocate data records and feature vector
 */
    infea_rec = allo_fea_rec(infea_ih);
    outfea_rec = allo_fea_rec(outfea_oh);
    posteriors = (float *) get_fea_ptr(outfea_rec, postfield, outfea_oh);
    class = (short *) get_fea_ptr(outfea_rec, clasfield, outfea_oh);
    TRYALLOC(float, statsize, feavec, "feature vector")
/*
 * main processing loop---classify records
 */
    while (get_fea_rec(infea_rec, infea_ih, infea_strm) != EOF)
    {
/*
 * assemble feature vector
 */
	if (d_flag)
	{
	    DEBUG(1)(stderr, "Calling get_deriv_vec.\n");

	    feavec = (float *) get_deriv_vec(fields, infea_rec, infea_ih,
			FLOAT, &length, (char *) feavec);

	    if (debug_level >= 2)
		pr_farray("Feature vector", (int) statsize, feavec);
	}
	else
	{
	    fea_ptr = get_fea_ptr(infea_rec, statfield, infea_ih);

	    DEBUG(1)(stderr, "Getting features.\n");

	    switch(get_fea_type(statfield, infea_ih))
	    {
	    case BYTE:
	    case CHAR:
		{
		    char  *p = (char *) fea_ptr;
		    for (i = 0; i < statsize; i++)
			feavec[i] = p[e_flag ? elements[i] : i];
		}
		break;
	    case SHORT:
		{
		    short  *p = (short *) fea_ptr;
		    for (i = 0; i < statsize; i++)
			feavec[i] = p[e_flag ? elements[i] : i];
		}
		break;
	    case LONG:
		{
		    long  *p = (long *) fea_ptr;
		    for (i = 0; i < statsize; i++)
			feavec[i] = p[e_flag ? elements[i] : i];
		}
		break;
	    case FLOAT:
		{
		    float  *p = (float *) fea_ptr;
		    for (i = 0; i < statsize; i++)
			feavec[i] = p[e_flag ? elements[i] : i];
		}
		break;
	    case DOUBLE:
		{
		    double  *p = (double *) fea_ptr;
		    for (i = 0; i < statsize; i++)
			feavec[i] = p[e_flag ? elements[i] : i];
		}
		break;
	    }

	    if (debug_level >= 2)
		pr_farray("Feature vector", (int) statsize, feavec);
	}
/*
 * classify
 */
	/* The following works because the strings in enums are in the */
	/* same order as the vectors and matrices in means and */
	/* invcovars (that of the records in the statistics file). */

	*class = 
	    classify(feavec, (int) statsize, num_clas, means,
		    invcovars, (float *) NULL, posteriors);

	DEBUG(1)(stderr, "Class number %d.\n", *class);
	if (debug_level >= 2)
	    pr_farray("Posteriors", num_clas, posteriors);
	
	count[*class]++;
/*
 * copy data from input record
 */
	if (infea_ih->common.tag)
	    outfea_rec->tag = infea_rec->tag;
	copy_fea_rec(infea_rec, infea_ih, outfea_rec, outfea_oh,
		(char **) NULL, (short **) NULL);

	DEBUG(1)(stderr, "Copying data from input record.\n");
/*
 * output record
 */	
	put_fea_rec(outfea_rec, outfea_oh, outfea_strm);	
    }
/*
 * end of main processing loop
 *
 * final history output---summary statistics
 */
    if (hist_name != NULL)
    {
	Fprintf(hist_strm, "\n%-20s%10s%15s\n%-20s%10s%15s\n\n",
		"", "Number of", "Relative", 
		"Class", "records", "frequency");

	nrec = 0;
	for (i = 0; i < num_clas; i++)
	    nrec += count[i];

	for (i = 0; i < num_clas; i++)
	    Fprintf(hist_strm, "%-20s%10ld%15g\n",
		    enums[i], count[i], count[i] / (double) nrec);

	Fprintf(hist_strm, "%-20s%10s%15s\n%-20s%10ld%15g\n\n",
		"", "---------", "---------", 
		"Total", nrec, 1.0);
    }
    exit(0);
    /*NOTREACHED*/	
}

/*
 * for debug printout of floating arrays
 */

void pr_farray(text, n, arr)
    char    *text;
    int	    n;
    float   *arr;
{
    int	    i;

    Fprintf(stderr, "%s - %d points:\n", text, n);
    for (i = 0; i < n; i++)
    {
	Fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == n - 1) Fprintf(stderr,  "\n");
    }
}

int
num_enums(fld, hdr)
    char    *fld;
    struct header *hdr;
{
    int	    i, n;
    char    **p;

    i = lin_search2(hdr->hd.fea->names, fld);
    spsassert(i != -1, "num_enums: field not found.");
    p = hdr->hd.fea->enums[i];
    n = 0;
    while (*p++ != NULL) n++;
    return n;
}
