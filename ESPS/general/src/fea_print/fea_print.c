/*
 *  This program was created by Richard Goldhor of Sensimetrics Corp.,
 *  by modifying a program (fea_deriv) originally created by Entropic
 *  Speech, Inc.
 *
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the prior	   
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice			
 * 								
 *      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved."
 *
 * 				
 *  Module Name:  fea_print
 *
 *  Written By:  Rich Goldhor
 * 	
 *  Checked By: John Shore
 *
 *  Purpose:  print FEA FILE records in a convenient format
 *
 *  Secrets:  Don't I wish
 *  
 */
#ifndef lint
    static char *sccs_id = "@(#)fea_print.c	1.3	8/31/95	ESI";
#endif
#define VERSION "1.3"
#define DATE "8/31/95"

#include <stdio.h>
#include <ctype.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define SYNTAX \
USAGE("fea_print [-r range] [-x debug-level] [-l layout] layoutfile infile.fea")

#define REALLOC(typ,num,var) { \
    if ((var) == NULL) \
	(var) = (typ *) malloc((unsigned) (num) * sizeof(typ)); \
    else (var) = (typ *) realloc((char *) (var), \
				    (unsigned) (num) * sizeof(typ)); \
    spsassert((var), "can't allocate memory."); \
}

#define Fflush (void) fflush
#define	MAXSTRLEN	(1024)	/* Max chars in various strings. */
#define	MAXPRINTVARS	(100)	/* Max # of variables to print per record. */

extern  int	optind;
extern	char	*optarg;

/*
 *  E S P S
 *   R O U T I N E S
 *    R E F E R E N C E D
 */

char	*get_cmd_line();
char	*get_deriv_vec();
int	lin_search();
void	lrange_switch();
char	*savestring();
char    *fea_decode();

char	*ProgName = "fea_print";

int	debug_level = 0;		/* global debug flag */


main(argc, argv)
    int     argc;
    char    **argv;
{
    int		c;	    /* used for parsing command line */

    FILE 	   *sfp, *inp;			/* file pointers */
    struct header  *ih;				/* pointers to structure */
    char	   *sfstr, *instr, *lstr;	/* string pointers */

    char	layout[MAXSTRLEN+1];
    char	header1[MAXSTRLEN+1],header2[MAXSTRLEN+1],header3[MAXSTRLEN+1];

    struct fea_data	*i_fea_rec;	/* input ESPS Feature file record */

    int		rflag = 0;		/* range flag */
    char 	*range;			/* string to hold range */
    char	*format;		/* current format string. */

    long	s_rec; 		/* starting record position */
    long	e_rec;		/* ending record position */
    long	n_rec;  	/* record counter */

    int		i = 0;		/* indexing variables */
    int		j;
    char	*s;

    char	field_line[256],	    /* one line from input file */
		*keyword,	    	    /* keyword */
                *KeyString,	    	    /* keyword value. */
		*new_field_name;

    int		line_num;	/* current line number in field file */

    int		keyword_found = NO;	    /* keyword found or not */
    int		component_found = NO;	    /* component field found or not */
    int		component_error = NO;	    /* component field error status */

    int		n_deriv = 0;	    /*  total number of fields to derive */
    int		DSize;			/* Size of current data type. */

    /* int 	*derived_type = NULL;	*/

    char        *field_names[MAXPRINTVARS]; /* field names */
    int		data_type[MAXPRINTVARS];	/* data type array */
    char	*formats[MAXPRINTVARS];	/* input format strings. */
    char	***srcfields = NULL;	/* array of field names */
    int		ppcnt = 0;
    char	*printptrs[MAXPRINTVARS];/* print arg pointers. */
    int		printtypes[MAXPRINTVARS];	/* print arg types. */
    char	*printfrms[MAXPRINTVARS];	/* print format strings. */
    char	*vvec[MAXPRINTVARS];	/* vector of ptrs to data storage. */

    /*  int	type; */

    long	length;		/* length of derived vector */
    char	*vec;		/* vector containing results */

/*
 *  B E G I N
 *   P R O G R A M
 */

/* Initialization. */

layout[0] = '\0';
header1[0] = header2[0] = header3[0] = '\0';
lstr = NULL;

/* get options from command line and set various flags */

    while ((c = getopt(argc, argv, "r:x:l:")) != EOF)
	switch (c)
	{
	case 'r': 
	    rflag++;
	    range = optarg;
	    break;
	case 'x': 
	    debug_level = atoi(optarg);
	    break;
	case 'l': 
	    lstr = optarg;
	    break;
	default:
	    SYNTAX;
	}

/* user must supply two file names: the stylefile and infile.fea */

    if (argc - optind != 2)
	SYNTAX;

/* now open the style file */

    sfstr = argv[optind++];
    if	(strcmp(sfstr, "-") == 0)
	    sfp = stdin;
    else    TRYOPEN (argv[0], sfstr, "r", sfp);

/* open input file or make input stdin; read header; check type */

    instr = argv[optind++];
    if	(sfp == stdin && strcmp(instr, "-") == 0)
	    {
	    Fprintf(stderr,
		    "%s: style and feature files can't BOTH be from stdin!\n",
		    ProgName);
	    exit(1);
	    }
    instr = eopen(ProgName, instr, "r", FT_FEA, NONE, &ih, &inp);

    if (debug_level > 0)
        Fprintf (stderr,
            "%s: style file = %s, infile = %s, layout = %s.\n",
	    ProgName, sfstr, instr, lstr);

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


    (void) fea_skiprec (inp, s_rec - 1, ih);

    if (debug_level > 0) {
	Fprintf (stderr, "%s: skipped %ld records.\n",
	ProgName, s_rec - 1);
    }

/* Now begin processing fieldfile */

    keyword_found = NO;
    line_num = 0;

    while (fgets (field_line, 256, sfp) != NULL) {

	++line_num;

	if (debug_level > 15) {
	    Fprintf (stderr, "%s: field_line: %s", ProgName, field_line);
	    Fflush (stderr);
	}

	/* Is this a blank line?
	 * If so, break out if we've collected our components,
	 * otherwise just go get the next line.
	 */
	if (strcmp(field_line, "\n") == 0)
	   if	(component_found == YES)
		break;
	   else continue;

	if ((new_field_name = strchr (field_line, '=')) != NULL) {

	    /* keyword found */

	    keyword_found = YES;
	    keyword = strtok (field_line, " =");

	    component_found = NO;

	    /* Find out which keyword this is. */

	    if (strcmp (keyword, "layout") == 0) {
	        KeyString = layout;
	    }
	    else if (strcmp (keyword, "header1") == 0) {
	        KeyString = header1;
	    }
	    else if (strcmp (keyword, "header2") == 0) {
	        KeyString = header2;
	    }
	    else if (strcmp (keyword, "header3") == 0) {
	        KeyString = header3;
	    }
	    else {
		Fprintf (stderr,
		"%s: unknown keyword \"%s\" found on line %d.\n",
		ProgName, keyword, line_num);
		exit(1);
	    }


	    /*
	     * new_field_name points to the equal sign, advance the pointer
	     * by one element.
	     * Also, remove the terminating new line character.
	     */

	    new_field_name++;
	    new_field_name[ strlen(new_field_name) - 1 ] = '\0';
	    strcpy(KeyString, new_field_name);

	    if (debug_level > 10) {
		Fprintf (stderr, "\n%s: keyword = %s, value = %s.\n",
			 ProgName, keyword, KeyString);
	        Fflush (stderr);
	    }

	    if	(KeyString == layout)
		    {
		    /* This is a layout name.
		     * It's the right one if it equals lstr,
		     * or if no layout string was specified.
		     */
		    if	(lstr == NULL || strcmp(lstr, KeyString) == 0)
			    {
			    }
		    else    {
			    keyword_found = NO;
			    layout[0] = '\0';
			    header1[0] = header2[0] = header3[0] = '\0';
			    while (fgets(field_line, 80, sfp) != NULL)
				    {
				    ++line_num;
				    if	(strcmp(field_line, "\n") == 0) break;
				    }
			    }
		    }


	} else {

	   /* component field_name[<element_range>] found */

	    component_found = YES;

	    /* Components must be preceded by keywords. */

	    if	(keyword_found == NO)
		    {
		    fprintf(stderr,
"%s: error in style file %s:\n\tcomponent name %s on line %d precedes layout keywords.\n",
			    ProgName, sfstr, field_line, line_num);
		    exit(1);
		    }

	    ++n_deriv;
	    if	(n_deriv > MAXPRINTVARS)
		    {
		    Fprintf(stderr,
			    "%s: more than %d elements to print per record.\n",
			    ProgName, MAXPRINTVARS);
		    exit(1);
		    }

	    /* get rid of the trailing newline character, if there is one. */

	    if	(field_line[ strlen(field_line) - 1 ] == '\n')
		    field_line[ strlen(field_line) - 1 ] = '\0';

	    /* Remove format string. */

	    s = strpbrk(field_line, " ");
	    if	(s == NULL)
		    {
		    Fprintf(stderr, "%s: couldn't find format string in %s.\n",
			    ProgName, field_line);
		    exit(1);
		    }
	    format = s+1;
	    *s = '\0';

	    /* Store the trimmed string. */

	    vvec[n_deriv-1] = (char *)NULL;
	    REALLOC(char **, n_deriv, srcfields)
	    srcfields[n_deriv-1] = NULL;
	    REALLOC(char *, 2, srcfields[n_deriv-1])
	    srcfields[n_deriv-1][0] = savestring (field_line);
	    srcfields[n_deriv-1][1] = NULL;

	    /* See if remaining string is a valid field name. */

	    component_error = NO;

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

	    if ((s=strpbrk (field_line, "[")) == NULL)
		component_error = YES;
	    if (strpbrk (field_line, "]") == NULL)
		component_error = YES;

	    
	    /* If the "component" specification doesn't look like a component,
	     * mark the type as UNDEF and (later) treat the component string
	     * as an "immediate" string variable.
	     */

	    if	(component_error == NO)
		    {
		    *s = '\0';
		    i = get_fea_type(field_line, ih);
                    field_names[n_deriv-1] = savestring(field_line);
                    if (debug_level > 10) 
                       Fprintf(stderr, "%s: new field name = %s\n", 
			       field_names[n_deriv-1], ProgName);
		    if (is_field_complex(ih, field_line)) {
		      Fprintf(stderr, "%s: can't handle complex fields (%s)\n",
			      ProgName, field_line);
		      exit(1);
		    }
		    *s = '[';
		    }
	    else    i = UNDEF;
	    data_type[n_deriv-1] = i;
	    formats[n_deriv-1] = savestring(format);


	    if (debug_level > 10) {
		    Fprintf(stderr, "component %d: %s; format: %s\n",
			n_deriv, srcfields[n_deriv-1][0], formats[n_deriv-1]);
		Fprintf (stderr, "\n");
		Fflush (stderr);
	    }

	}   /* end if ((new_field_name = strchr (field_line, ...) */

    }	/* end while (fgets (field_line, 80, sfp)) */

    /* If no keyword found print error and die */

    if (!keyword_found) {
	Fprintf (stderr,
	"%s: layout %s not found in style file %s.\n", ProgName, lstr, sfstr);
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
	"%s: total number of fields to print is %d\n",
	ProgName, n_deriv);

    /* Scan headers and format strings for escaped characters. */

    Escape(header1);
    Escape(header2);
    Escape(header3);
    for (i = 0; i < n_deriv; ++i)
	    Escape(formats[i]);

    /* Write out the headers. */

    printf(header1);
    printf(header2);
    printf(header3);
    fflush(stdin);

    i_fea_rec = allo_fea_rec (ih);

    ppcnt = 0;

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

	/* Get pointers to each vector of desired elements in this record.
	 */
	for (j = 0; j < n_deriv; j++) {

	    if (debug_level > 8)
		Fprintf (stderr,
		"%s: getting value of %s.\n",
		ProgName, srcfields[j][0]);

	    /* This isn't real component: treat as immediate string var. */
	    if	(data_type[j] == UNDEF)
		    {
		    vec = srcfields[j][0];
		    length = 1;
		    }
	    else    vec = get_deriv_vec(srcfields[j], i_fea_rec, ih,
				     data_type[j], &length, vvec[j]);
	    if (vec == NULL)
	    {
		Fprintf (stderr,
	       "%s: get_deriv_vec: encountered error getting field.\n",
		ProgName);
		exit(1);
	    }

	    if	(ppcnt+length > MAXPRINTVARS)
		    {
		    Fprintf(stderr,
		     "%s: more than %d elements to print per record.\n",
		     ProgName, MAXPRINTVARS);
		    exit(1);
		    }

	    switch (data_type[j]) 
	    {
	    case DOUBLE: DSize = 8;
		     break;
	    case FLOAT:  DSize = 4;
		     break;
	    case LONG:   DSize = 4;
		     break;
	    case CODED:
	    case SHORT:  DSize = 2;
		     break;
            case UNDEF:  DSize = -1;
		    break;
	    case CHAR:   DSize = 0;
		    if (n_rec == s_rec) 
		      vec = realloc(vec, (unsigned) length+1);
		    vec[length] = '\0';
		    length = 1;
		    break;
	    case BYTE:   DSize = 1;
		    break;
	    }  /* end switch (type) */


	    /* If this is the first record, set up the pointers to
	     * each element location, element type, and print format string.
	     */
	    if	(n_rec == s_rec)
		    {
		    vvec[j] = vec;
		    for (i = 0; i < length; ++i, ++ppcnt)
			    {
			    printptrs[ppcnt] = vec + DSize*i;
			    printtypes[ppcnt] = data_type[j];
			    printfrms[ppcnt] = formats[j];
			    }
		    }


	}  /* end for (j = 0; j < n_deriv; j++) */

	/* Print the record. */

	for	(i = 0; i < ppcnt; ++i)
		{
		if (debug_level > 10) 
		  printf("printing element %d of type %d using format %s\n",
		       i, printtypes[i], printfrms[i]);
		switch (printtypes[i]) 
			{
		case DOUBLE: printf(printfrms[i], *(double *)printptrs[i]);
			break;
		case FLOAT:  printf(printfrms[i], *(float *)printptrs[i]);
			break;
		case LONG:   printf(printfrms[i], *(long *)printptrs[i]);
			break;
		case SHORT:  printf(printfrms[i], *(short *)printptrs[i]);
			break;
		case UNDEF:
		case CHAR:   printf(printfrms[i], (char *)printptrs[i]);
			break;
		case CODED: 
		        printf(printfrms[i], 
			   fea_decode(*(short *) printptrs[i], 
				       (char *) field_names[i], ih));
		        break;
		case BYTE:   printf(printfrms[i], *(char *)printptrs[i]);
			break;
			}  /* end switch */
		}


    }  /* end for (n_rec = 0; i <= e_rec && ... ; n_rec++) */

    exit(0);
    /*NOTREACHED*/
}
Escape(s)
	char *s;
/*================================================================

FUNCTION:	Escape() scans a text string and converts C-like
		escape sequences to their proper ASCII character.

ARGUMENTS:	s:	String to be converted.

NOTES:		The conversions that are handled are:
			\n -> newline
			\s -> space
			\t -> tab
			\f -> formfeed

==================================================================
!*/
{
char	*t = s, c;

while	(*s)
	{
	if	(*s == '\\')
		{
		switch	(*(++s))
			{
		case ('n'): c = '\n'; break;
		case ('s'): c = ' '; break;
		case ('e'): c = '='; break;
		case ('t'): c = '\t'; break;
		case ('f'): c = '\f'; break;
		default: c = *s; break;
			}
		*t++ = c;
		++s;
		}
	else	*t++ = *s++;
	}
*t = '\0';
}
