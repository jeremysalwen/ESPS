/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by: Jay Virdy  
 * Checked by:
 * Revised by: John Shore (removed d_mat_alloc(500x500) !!) 
 *
 * Brief description: print table of field info from header
 *
 */

static char *sccs_id = "@(#)fea_element.c	3.11	3/14/94	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>

#define SYNTAX USAGE("fea_element [-x debug_level] [-c] [-n] [-f field_name] [-i item] [file]")

extern int  optind;
extern char *optarg;


/* functions used in fea_element */

long get_fea_elem_count();

char   *ProgName = "fea_element";

int     debug_level = 0;

main (argc, argv)
int     argc;
char  **argv;
{
    int     c;

    struct header  *sps_hdr;
    struct fea_header  *fea_hdr;

    char   *fea_file;

    FILE * istrm = stdin;

    int     cflag = 0;
    char  **field_name = NULL;
    int     item = 0;
    int     fflag = 0;
    int     iflag = 0;
    int	    nflag = 0;	/* if 1 suppress printing of headers */

    /* 
     * miscellaneous variables used in this program
     *
     */

    int     print_info = YES;
    int     element = 1;
    int     i = 0;
    int     j;

    static short    types[] = {
	DOUBLE, DOUBLE_CPLX, FLOAT, FLOAT_CPLX, 
        LONG, LONG_CPLX, SHORT, SHORT_CPLX, 
	CHAR, BYTE_CPLX };
    int     num_of_types = 10;	/* number of data types in types[] */

    int     name_len;		/* length of string in fea_hdr->names[i] 
				*/

    int     ifield = 0;		/* keep track of field_name matrix */

    struct feature {
	char   *name;
	int     type;
	int     size;
	int     element;


    }              *fea_ele;

    int     name_found;


    /* parse command line options */

    while ((c = getopt (argc, argv, "x:cf:i:sn")) != EOF)
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'c': 
		cflag = 1;
		print_info = NO;
		break;
	    case 'f': 
		addstr(optarg, &field_name); 
		ifield++;
		print_info = YES;
		fflag = 1;
		break;
	    case 'i': 
		item = atoi (optarg);
		iflag = 1;
		break;
	    case 'n':
		nflag = 1;
		break;
	    default: 
		SYNTAX;
	}

    if (argc - optind != 1) {
	(void) fprintf (stderr,
		"%s: no input file specified.\n", ProgName);
	SYNTAX;
    }


    fea_file = eopen (ProgName, argv[optind], "r", FT_FEA, NONE, &sps_hdr, &istrm);

    fea_hdr = sps_hdr -> hd.fea;

    fea_ele = (struct feature  *) malloc (sizeof (*fea_ele) * (fea_hdr -> field_count + 1));
    spsassert (fea_ele, "fea_element: malloc failed");

    if (cflag && fflag)
	print_info = NO;

    if (print_info) {
	if (!nflag) {
		(void) fprintf (stdout,
		      "Name\t\tType\t\tSize\t\tElement\n");
		(void) fprintf (stdout,
		      "----\t\t----\t\t----\t\t-------\n");
	}

	if ((sps_hdr -> common.tag == YES) && !fflag)
	    (void) fprintf (stdout,
		    "tag \t\tLONG\t\t1   \t\t0\n");

    }

    if (fea_hdr -> field_order == YES) {
	for (i = 0; i < fea_hdr -> field_count; i++) {
	    fea_ele[i].name = fea_hdr -> names[i];
	    fea_ele[i].type = fea_hdr -> types[i];
	    fea_ele[i].size = fea_hdr -> sizes[i];
	    fea_ele[i].element = element;

	    element += fea_hdr -> sizes[i];

	    name_len = strlen (fea_hdr -> names[i]);

	    if (!cflag && !fflag) {
		(void) fprintf (stdout, "%s%s\t%s%s\t%ld\t\t%d\n",
			fea_ele[i].name, (name_len >= 8) ? "" : "\t",
			type_codes[fea_ele[i].type],
			is_type_complex(fea_ele[i].type) ? "" : "\t",
			fea_ele[i].size, fea_ele[i].element);
	    }
	}
    }
    else {

	for (j = 0; j < num_of_types; j++)
	    for (i = 0; i < fea_hdr -> field_count; i++)
		if (types[j] == fea_hdr -> types[i]
			|| types[j] == SHORT && fea_hdr -> types[i] == CODED
			|| types[j] == CHAR && fea_hdr -> types[i] == BYTE) {
		    fea_ele[i].name = fea_hdr -> names[i];
		    fea_ele[i].type = fea_hdr -> types[i];
		    fea_ele[i].size = fea_hdr -> sizes[i];
		    fea_ele[i].element = element;

		    element +=  get_fea_elem_count(fea_ele[i].name,sps_hdr);

		    name_len = strlen (fea_hdr -> names[i]);

		    if (!cflag && !fflag) {
			(void) fprintf (stdout, "%s%s\t%s\t%s%ld\t\t%d\n",
				fea_ele[i].name, (name_len >= 8) ? "" : "\t",
				type_codes[fea_ele[i].type],
				is_type_complex(fea_ele[i].type) ? "" : "\t",
				fea_ele[i].size, fea_ele[i].element);
		    }
		}
    }


    /* 
     * item 0 corresponds to the first element, but
     * element 0 corresponds to the tag.
     */

    if (item < 0) {
	(void) fprintf (stderr,
		"%s: incorrect item specified.\n", ProgName);
	exit (1);
    }

    for (j = 0; j < ifield; j++) {

	name_found = NO;

	for (i = 0; i < fea_hdr -> field_count; i++) {

	    name_len = strlen (fea_hdr -> names[i]);

	    if (strcmp (field_name[j], fea_ele[i].name) == 0) {

		name_found = YES;

		if (item >= fea_ele[i].size) {
		    (void) fprintf (stderr,
			    "%s: incorrect item specified for this field: %s\n",
			    ProgName, field_name[j]);
		    exit (1);
		}

		if (!cflag) {
		    (void) fprintf (stdout,
			    "%s%s\t%s\t\t%ld\t\t", fea_ele[i].name,
			    (name_len >= 8) ? "" : "\t",
			    type_codes[fea_ele[i].type], fea_ele[i].size);
		}

		if (fea_ele[i].size == 1)
		    (void) fprintf (stdout, "%d\n", fea_ele[i].element);
		else if (!cflag)
		    (void) fprintf (stdout,
			    "%d\n", fea_ele[i].element + item);
		else if (iflag)
		    (void) fprintf (stdout,
			    "%d\n", fea_ele[i].element + item);
		else
		    (void) fprintf (stdout,
			    "%d%s%d\n",
			    fea_ele[i].element,
			    ":", fea_ele[i].element + fea_ele[i].size - 1);


	    }			/* end if (strcmp(field_name ...)) */

	}			/* end inner for loop */

	if (!name_found) {
	    (void) fprintf (stderr,
		    "%s: %s does not exist in %s\n",
		    ProgName, field_name[j], fea_file);
	    exit (1);
	}
    }				/* end outer for loop */
    return 0;
}
