/*
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the prior	   
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice			
 * 								
 * "Copyright (c) 1987, 1989, 1990 Entropic Speech, Inc.; All rights reserved."
 *
 * 				
 *  Module Name:  fea_deriv
 *
 *  Written By:  Ajaipal S. Virdy; John Shore
 * 	
 *  Checked By: Rod Johnson
 *
 *  Purpose:  derive a new ESPS FEA file containing elements from an
 *	    existing one.
 *
 *  Secrets:  None
 *  
 */
#ifndef lint
    static char *sccs_id = "@(#)fea_deriv.c	3.13	2/14/96	 ESI";
#endif
#define VERSION "3.13"
#define DATE "2/14/96"

#include <stdio.h>
#include <ctype.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define SYNTAX \
USAGE("fea_deriv [-r range] [-T subtype] [-t] [-x debug-level] fieldfile infile.fea outfile.fea")

#define REALLOC(typ,num,var) { \
    if ((var) == NULL) \
	(var) = (typ *) malloc((unsigned) (num) * sizeof(typ)); \
    else (var) = (typ *) realloc((char *) (var), \
				    (unsigned) (num) * sizeof(typ)); \
    spsassert((var), "can't allocate memory."); \
}

#define PRINT_RESULT(type, format) { \
    for (k = 0; k < length; k++) \
	Fprintf(stderr, "%s: ((type *) vec)[%d] = ", ProgName, k); \
	Fprintf(stderr, format, ((type *)vec)[k]); \
	Fprintf(stderr, "\n"); \
}

#define WRITE_RESULT(type) { \
    type *ptr = (type *) get_fea_ptr(o_fea_rec, derived_name[j], oh); \
    for (k = 0; k < length; k++) \
	*ptr++ = ((type *)vec)[k]; \
}

#define Fflush (void) fflush

extern  int	optind;
extern	char	*optarg;

/*
 *  E S P S
 *   R O U T I N E S
 *    R E F E R E N C E D
 */

char	*get_cmd_line();
char	*get_deriv_vec();
int	get_high_field();
int	lin_search();
void	lrange_switch();
char	*savestring();

char	*ProgName = "fea_deriv";

int	debug_level = 0;		/* global debug flag */


main(argc, argv)
    int     argc;
    char    **argv;
{
    int		c;	    /* used for parsing command line */

    FILE 	   *ffp, *inp, *outp;		/* file pointers */
    struct header  *ih, *oh;			/* pointers to structure */
    char	   *ffstr, *instr, *outstr;	/* string pointers */

    struct fea_data	*i_fea_rec;	/* input ESPS Feature file record */
    struct fea_data	*o_fea_rec;	/* output ESPS Feature file record */

    int		rflag = 0;		/* range flag */
    char 	*range;			/* string to hold range */

    char	combuf[100];	/* command buffer for writing to header */

    long	s_rec; 		/* starting record position */
    long	e_rec;		/* ending record position */
    long	n_rec;  	/* record counter */

    int		i = 0;		/* indexing variables */
    int		j;
    int		k;

    char	field_line[80],	    /* one line from input file */
		*keyword,	    /* keyword (either field or type) */
		*new_field_name;

    int		line_num;	/* current line number in field file */

    int		keyword_found = NO;	    /* keyword found or not */
    int		component_found = NO;	    /* component field found or not */
    int		component_error = NO;	    /* component field error status */

    int		n_field = 0;	    /*  number of field names within a
					derived field */
    int		n_deriv = 0;	    /*  total number of fields to derive */

    char	**derived_name = NULL;	/* derived field name */

    int 	*derived_type = NULL;

    int		*data_type = NULL;	/* data type array */
    char	***srcfields = NULL;	/* array of field names */

    char	**genhd;	/* generic header stuff */
    int		ngen;

    int		type = -1; 

    long	length;		/* length of derived vector */
    char	*vec;		/* vector containing results */
    char   *subtype = NULL;  /*for -T option*/
    int    subtype_code = 0;
    int         tflag=0;	/* for -t option */

/*
 *  B E G I N
 *   P R O G R A M
 */

/* get options from command line and set various flags */

    while ((c = getopt(argc, argv, "r:x:T:t")) != EOF)
	switch (c)
	{
	case 'r': 
	    rflag++;
	    range = optarg;
	    break;
	case 'x': 
	    debug_level = atoi(optarg);
	    break;
	case 'T':
	    subtype = optarg;
	    break;
	case 't':
	    tflag++;
	    break;
	default:
	    SYNTAX;
	}

/* user must supply three file names: fieldfile infile.fea outfile.fea */

    if (argc - optind != 3)
	SYNTAX;

/* open output file or make output stdout */

    outstr = eopen(ProgName, argv[argc - 1],
		"w", NONE, NONE, (struct header **) NULL, &outp);

/* now open the field file */

    ffstr = argv[optind++];
    TRYOPEN (argv[0], ffstr, "r", ffp);

/* open input file or make input stdin; read header; check type */

    instr = eopen(ProgName, argv[optind++], "r", FT_FEA, NONE, &ih, &inp);

    if (debug_level > 0)
        Fprintf (stderr,
            "%s: fieldfile = %s, infile = %s, outfile = %s.\n",
	    ProgName, ffstr, instr, outstr);

/* make sure the given range (if there is one) exists in file1 */

    s_rec = 1;
    e_rec = LONG_MAX;

    if (rflag)
        lrange_switch (range, &s_rec, &e_rec, 1);

    if (debug_level > 1)
        Fprintf (stderr, 
             "%s: range of %s starts at %ld and ends at %ld.\n",
	     ProgName, instr, s_rec, e_rec);

    if (s_rec < 1) {
        Fprintf (stderr, 
	    "%s: start record less than 1.\n", ProgName);
        exit(1);
    }

    if ( s_rec > e_rec ) {
        Fprintf (stderr, 
	     "%s: start record greater than end record.\n", ProgName);
        exit(1);
    }

/* make output header */

    oh = new_header (FT_FEA);

    (void) strcpy (oh->common.prog, ProgName);
    (void) strcpy (oh->common.vers, VERSION);
    (void) strcpy (oh->common.progdate, DATE);

    add_source_file (oh, instr, ih);

    add_comment (oh, get_cmd_line(argc, argv));

    if (tflag) {
      oh->common.tag = ih->common.tag;
      oh->variable.refer = ih->variable.refer;
    }

    /* Copy generic header items from input file to output file */

    genhd = genhd_list (&ngen, ih);
    while (ngen--)
    {
        int size;
        short type;
	type = genhd_type(genhd[ngen], &size, ih);
	(void) add_genhd(genhd[ngen], type, size, 
			    get_genhd(genhd[ngen], ih),
			    genhd_codes(genhd[ngen], ih), oh);
    }

    update_waves_gen (ih, oh, (float)s_rec, (float)1.0);

    (void) fea_skiprec (inp, s_rec - 1, ih);

    if (debug_level > 0) {
	Fprintf (stderr, "%s: skipped %ld records.\n",
	ProgName, s_rec - 1);
    }
    (void) sprintf (combuf,
        "Derived fields from input file %s\n", instr);

    add_comment (oh, combuf);

    if (debug_level > 0) {
       Fprintf (stderr,
	   "%s: writing header to %s.\n", ProgName, outstr);
       Fflush (stderr);
    }

    if (debug_level > 10)
	Fprintf (stderr, "\n");

/* Now begin processing fieldfile */

    keyword_found = NO;
    line_num = 0;

    while (fgets (field_line, 80, ffp) != NULL) {

	++line_num;

	if (debug_level > 15) {
	    Fprintf (stderr, "%s: field_line: %s", ProgName, field_line);
	    Fflush (stderr);
	}

        /* make sure first character of component field is valid */

	if (!isalpha (field_line[0]) && (field_line[0] != '_'))
	    component_error = YES;

	/* check all elements as well */

	for (i = 1; i < strlen(field_line) - 1; i++)
	    if (!isspace (field_line[i]) && !isalnum (field_line[i])
			&& (field_line[i] != '_')
			&& (field_line[i] != '=')
			&& (field_line[i] != '[')
			&& (field_line[i] != ',')
			&& (field_line[i] != ':')
			&& (field_line[i] != ']')) {
		component_error = YES;
		break;
	    }

	if (component_error) {
	    Fprintf (stderr,
	    "%s: encountered error in field file %s, line %d: %s",
	    ProgName, ffstr, line_num, field_line);
	    exit(1);
	}
	component_error = NO;

	if ((new_field_name = strchr (field_line, '=')) != NULL) {

	    /* keyword found */

	    keyword_found = YES;
	    keyword = strtok (field_line, " =");

	    component_found = NO;

	    /* We must also determine the type of the data field. */

	    if (strcmp (keyword, "type") == 0) {
		new_field_name++;
		while (isspace (*new_field_name)) new_field_name++;
                new_field_name[ strlen(new_field_name) - 1 ] = '\0';
 	    	type = lin_search (type_codes, new_field_name);
		if(n_deriv > 0)
	        	derived_type[n_deriv-1] = type;
	    }
 	    else {

	        if (strcmp (keyword, "field") != 0) {
		    Fprintf (stderr,
		    "%s: unknown keyword: \"%s\".\n",
		    ProgName, keyword);
		    exit(1);
	        }


	    /*
	     * new_field_name points to the equal sign, advance the pointer
	     * by one element.  Also, remove all leading white space and
	     * the terminating new line character.
	     *
	     */

	        new_field_name++;
	        while (isspace (*new_field_name))
		    new_field_name++;
    
	        new_field_name[ strlen(new_field_name) - 1 ] = '\0';
    
	        if (debug_level > 10) {
		    Fprintf (stderr, "\n%s: keyword = %s, new_field_name = %s.\n",
		    ProgName, keyword, new_field_name);
	            Fflush (stderr);
	        }
    
	    /* We'll have srcfields for each new keyword,
	     * therefor allocate memory for ***srcfields.
	     */

	        n_deriv++;
    
	        REALLOC(char *, n_deriv, derived_name)
	        derived_name[n_deriv-1] = savestring (new_field_name);

	        REALLOC(int, n_deriv, derived_type);
	        derived_type[n_deriv-1] = type;

	        REALLOC(char **, n_deriv, srcfields)
	        srcfields[n_deriv-1] = NULL;

	        n_field = 0;

	    }
	    } else {

	   /* component field_name[<element_range>] found */

	    component_found = YES;

	    if (strpbrk (field_line, "[") == NULL)
		component_error = YES;
	    if (strpbrk (field_line, "]") == NULL)
		component_error = YES;

	    if (component_error) {
		Fprintf (stderr,
	        "%s: encountered error in parsing component field in %s, line %d: %s",
		ProgName, ffstr, line_num, field_line);
	        exit(1);
	    }

	    /* get rid of the trailing newline character */

	    field_line[ strlen(field_line) - 1 ] = '\0';

	    n_field++;
	    REALLOC(char *, n_field + 1, srcfields[n_deriv-1])
	    srcfields[n_deriv-1][n_field-1] = savestring (field_line);
	    assert (srcfields[n_deriv-1][n_field-1]);
	    srcfields[n_deriv-1][n_field] = NULL;

	    if (debug_level > 10) {
		Fprintf (stderr, "\n%s: total number of fields = %d\n",
		ProgName, n_field);
		for (i = 0; srcfields[n_deriv-1][i] != NULL; i++) {
		    Fprintf (stderr, "%s: srcfields[%d][%d] = %s\n",
		    ProgName, n_deriv - 1, i, srcfields[n_deriv-1][i]);
		}
		Fprintf (stderr, "\n");
		Fflush (stderr);
	    }

	}   /* end if ((new_field_name = strchr (field_line, ...) */

    }	/* end while (fgets (field_line, 80, ffp)) */

    /* If no keyword found print error and die */

    if (!keyword_found) {
	Fprintf (stderr,
	"%s: no keyword found in field file %s.\n", ProgName, ffstr);
	exit(1);
    }

    if (!component_found) {
	Fprintf (stderr,
	"%s: no component fields found for field name %s.\n",
	ProgName, new_field_name);
	exit(1);
    }

    if (debug_level > 5)
	Fprintf (stderr,
	"%s: total number of fields to derive is n_deriv = %d\n",
	ProgName, n_deriv);

    if ((data_type = (int *) calloc((unsigned) n_deriv, sizeof(int)))
	== NULL) {
	Fprintf (stderr,
	    "%s: calloc: could not allocate memory for data_type.\n",
	    ProgName);
	exit(1);
    }

    for (j = 0; j < n_deriv; j++) {

	if (debug_level > 5) {
	    Fprintf (stderr,
		"%s: working on derived_name[%d] = %s\n",
		ProgName, j, derived_name[j]);
	    for (k = 0; srcfields[j][k] != NULL; k++) {
		Fprintf (stderr, "%s: srcfields[%d][%d] = %s\n",
		ProgName, j, k, srcfields[j][k]);
	    } 
	    Fprintf (stderr, "\n");
	    Fflush (stderr);
	}  /* end if (debug_level > 5) */

	if (derived_type[j] == -1) {
          if((data_type[j] = get_high_field(srcfields[j], ih, &length)) == -1) {
	    Fprintf (stderr,
	    "%s: incorrect type returned by get_high_field.\n",
	    ProgName);
	    exit(1);
	  }
	}
	else
	  data_type[j] = derived_type[j];
          (void)get_high_field(srcfields[j], ih, &length);

	/* Add the feature field to output ESPS FEA file. */

	if (add_fea_fld (derived_name[j], length, 1, (long *) NULL,
			 data_type[j], (char **) NULL, oh) == -1) {
	    Fprintf (stderr,
	    "%s: add_fea_fld: could not add field %s.\n",
	    ProgName, derived_name[j]);
	    exit(1);
	}
	if (set_fea_deriv (derived_name[j], srcfields[j], oh) == -1) {
	    Fprintf (stderr,
	    "%s: %s is not a defined field in %s.\n",
	    ProgName, derived_name[j], outstr);
	    exit(1);
	}

    }	/* end for (j = 0; j < n_deriv; j++) */

    /*set subtype code if called for */

    if (subtype) {
       if ((subtype_code = lin_search(fea_file_type, subtype)) == -1)
	  Fprintf(stderr,"fea_deriv: unknown FEA file subtype %s ignored.\n",
		subtype);
       else
	  oh->hd.fea->fea_type = subtype_code;
     }

    /* Write out the header */

    write_header (oh, outp);

    /* Allocate memory for output ESPS Feature file */

    i_fea_rec = allo_fea_rec (ih);
    o_fea_rec = allo_fea_rec (oh);

    /* Begin Processing */

    for (   n_rec = s_rec;
	    n_rec <= e_rec && get_fea_rec(i_fea_rec, ih, inp) != EOF;
	    n_rec++ )
    {

	/* get a record from the input ESPS Feature file */

	if (debug_level > 8)
	    Fprintf (stderr,
	    "%s: got record number %ld from input ESPS FEA file %s.\n",
	    ProgName, n_rec, instr);

	for (j = 0; j < n_deriv; j++) {

	    if (debug_level > 8)
		Fprintf (stderr,
		"%s: gathering data for derived field %s.\n",
		ProgName, derived_name[j]);

	    if ((vec = get_deriv_vec(srcfields[j], i_fea_rec, ih, data_type[j],
				     &length, (char *) NULL)) == NULL)
	    {
		Fprintf (stderr,
		"%s: get_deriv_vec: encountered an error getting derived field.\n",
		ProgName);
		exit(1);
	    }

	    if (debug_level > 10)
		switch (data_type[j]) 
		{
		case DOUBLE: PRINT_RESULT(double, "%g");
			     break;
		case FLOAT:  PRINT_RESULT(float, "%f");
			     break;
		case LONG:   PRINT_RESULT(long, "%ld");
			     break;
		case SHORT:  PRINT_RESULT(short, "%d");
			     break;
		case CHAR:   PRINT_RESULT(char, "%c");
			     break;
		case BYTE:   PRINT_RESULT(char, "%d");
			     break;
		}  /* end switch (type) */

	    /* Write the vector out into the new ESPS Feature file. */ 

	    switch (data_type[j])
	    {
	    case DOUBLE: WRITE_RESULT(double);
			 break;
	    case FLOAT:  WRITE_RESULT(float);
			 break;
	    case LONG:   WRITE_RESULT(long);
			 break;
	    case SHORT:  WRITE_RESULT(short);
			 break;
	    case CHAR:   WRITE_RESULT(char);
			 break;
	    case BYTE:   WRITE_RESULT(char);
			 break;
	    }  /* end switch (type) */

            free(vec);
	}  /* end for (j = 0; j < n_deriv; j++) */

	if (tflag)
	    o_fea_rec->tag = i_fea_rec->tag;

	/* put the record into output ESPS Feature file */

	put_fea_rec (o_fea_rec, oh, outp);

    }  /* end for (n_rec = 0; i <= e_rec && ... ; n_rec++) */

    exit(0);
    /*NOTREACHED*/
}
