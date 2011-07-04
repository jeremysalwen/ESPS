/*
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the prior	   
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice			
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved."
 *
 *  Module Name:  get_high_field
 *
 *  Written By:  Ajaipal S. Virdy
 *
 *  Checked By:
 *
 *  Purpose:  Computes "highest" type and total length of subranges
 *	      of component fields from an ESPS FEA file.
 *	      Returns -1 on error.
 *
 *  Secrets:  None
 *
 */

#ifndef lint
    static char *sccs_id = "@(#)gethighfie.c	3.4	2/14/96	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define Fflush (void)fflush

/*
 * define the "levels" of the possible scalar numeric data types
 * for ESPS FEA fields (cf. <esps/esps.h>).  We will return the
 * type with the highest level.
 */

#define MAX_LVL    4	/* highest */
#define DOUBLE_LVL 4
#define FLOAT_LVL  3
#define LONG_LVL   2
#define SHORT_LVL  1
#define BYTE_LVL   0
#define CHAR_LVL   0
#define MIN_LVL    0	/* lowest */
#define MIN_TYPE   CHAR	/* type with lowest level */

/*
 *  E S P S
 *   R O U T I N E S
 *    R E F E R E N C E D
 */

char    *savestring();
long    *grange_switch();

char	*Procedure = "get_high_field";


int
get_high_field (fields, hd, length)
    char    **fields;
    struct header   *hd;
    long    *length;
{
    struct fea_header *fea_hdr;	/* pointer to ESPS FEA Header */

    int	    i;	    		/* indexing variables */
    int	    j;

    int	    data_type;		/* data type code as given in <esps/esps.h> */
    int     data_level;		/* data type "level" as defined above */

    char    *field_name;	/* string to hold field name */
    char    *tmp_string;	/* temporary string to hold field name */
    char    *gen_range;		/* string to hold generic range */

    long    n_item;		/* number of items given in gen_range */

    long    total_len = 0;	/* total length of "derived" vector */

/*
 *  B E G I N
 *   P R O G R A M
 */

    fea_hdr = hd->hd.fea;	/* make life easier */

/*
 * B E G I N
 *  P R O C E S S I N G
 */

    data_level = MIN_LVL;	/* defauls data level: lowest */
    data_type = MIN_TYPE;	/* default data type: "lowest" type */

    for (i = 0; fields[i] != NULL; i++)
    {
	/* Parse <field_name> [ <gen_range> ]
	 * Make a copy of feature field name because
	 * strtok(3-UNIX) tends to destroy strings.
	 */

	tmp_string = savestring (fields[i]);
	field_name = strtok (tmp_string, "[");
	gen_range = strtok ((char *) 0, "]");

	/* Accumulate total length. */

        free(grange_switch (gen_range, &n_item));
	total_len += n_item;

	/*
	 * check if field is complex. If it is, abort
	*/
	if(is_field_complex(hd, field_name) == YES){
	  Fprintf(stderr, 
	  "fea_deriv: field \"%s\" is complex; not supported yet - exiting.\n", field_name);
	  exit(1);
	}

	/* Check that field is defined in header.
	 * Determine "highest" field type seen so far.
	 */

	j = lin_search2(fea_hdr->names, field_name);

	if (j == -1)	/* not found */
	{
	    Fprintf (stderr,
	    "%s: field name %s not found in ESPS Feature file.\n",
	    Procedure, field_name);
	    return (-1);
	}
	else /* found */
	    switch (fea_hdr->types[j])
	    {
	    case DOUBLE:
		if (data_level < DOUBLE_LVL)
		{
		    data_type = DOUBLE;
		    data_level = DOUBLE_LVL;
		}
		break;
	    case FLOAT:
		if (data_level < FLOAT_LVL)
		{
		    data_type = FLOAT;
		    data_level = FLOAT_LVL;
		}
		break;
	    case LONG:
		if (data_level < LONG_LVL)
		{
		    data_type = LONG;
		    data_level = LONG_LVL;
		}
		break;
	    case SHORT:
		if (data_level < SHORT_LVL)
		{
		    data_type = SHORT;
		    data_level = SHORT_LVL;
		}
		break;
	    case BYTE:
		if (data_level < BYTE_LVL)
		{
		    data_type = BYTE;
		    data_level = BYTE_LVL;
		}
		break;
	    case CHAR:
		if (data_level < CHAR_LVL)
		{
		    data_type = CHAR;
		    data_level = CHAR_LVL;
		}
		break;
	    default:
		Fprintf (stderr, "%s: unknown data type.\n", Procedure);
		return(-1);

	    }  /* end switch (fea_hdr->types[j]) */

    free(tmp_string);
    }	/* end for (i = 0; ... ) */
	    
#ifdef DEBUG
    Fprintf (stderr, "%s: Data type is %s, level is %d, length is %ld\n",
	Procedure, type_codes[data_type], data_level, total_len);
    Fflush (stderr);
#endif

    *length = total_len;
    return (data_type);
}
