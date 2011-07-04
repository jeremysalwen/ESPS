/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987,1990 Entropic Speech, Inc.; All rights reserved"
 				
  Module Name:  get_deriv_vec

  Written By:  Ajaipal S. Virdy
 	
  Checked By:

  Purpose:  create a derived from an ESPS FEA file.

  Secrets:  None
  
 */

#ifndef lint
static char *sccs_id = "@(#)getderiv_vec.c	1.2  2/14/96 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>


#define PACK_IT(type_from, ptr, format) { \
    switch (type) { \
       case DOUBLE: PACK_DATA(type_from, double, ptr, format); \
		    break; \
       case FLOAT:  PACK_DATA(type_from, float, ptr, format); \
		    break; \
       case LONG:   PACK_DATA(type_from, long, ptr, format); \
		    break; \
       case CODED:  \
       case SHORT:  PACK_DATA(type_from, short, ptr, format); \
		    break; \
       case CHAR:   \
       case BYTE:   \
		    PACK_DATA(type_from, char, ptr, format); \
		    break; \
    } \
}

#define PACK_DATA(type_from, type_to, ptr, format) \
	{ \
	    ptr = (type_from *) get_fea_ptr (fea_rec, fea_hdr->names[j], hd); \
	    for (k = 0; k < n_item; k++) { \
		((type_to *)vec)[index] = (type_to) *(ptr + item_array[k]); \
		index++; \
	    } \
	}

/*
 * The following was used for debugging PACK_DATA:
 *
		Fprintf (stderr, \
		"%s: PACK_DATA: ((type_to *)vec)[%d] = %format\n", \
		Module, index, ((type_to *)vec)[index]); \
*/

#define ALLOCATE(type, size) { \
    vec = calloc ((unsigned int) size, sizeof (type)); \
}

#define PRINT(type, format) { \
	for (i = 0; i < total_len; i++) { \
	    Fprintf (stderr, \
	    "%s: PRINT: ((type *)vec)[%d] = %format\n", \
	    Module, i, ((type *)vec)[i]); \
	} \
}

#define Fflush (void)fflush

typedef enum { False, True } BOOLEAN;

/*
 *  U N I X
 *   R O U T I N E S
 *    R E F E R E N C E D
 */
#ifndef DEC_ALPHA
char    *calloc();
char    *realloc();
char    *strtok();
#endif
/*
 *  E S P S
 *   R O U T I N E S
 *    R E F E R E N C E D
 */

long    *grange_switch();
long    get_fea_siz();
char    *get_fea_ptr();
char    *savestring();

char	*Module = "get_deriv_vec";

char *
get_deriv_vec (fields, fea_rec, hd, type, length, vec)
char		    **fields;
struct fea_data	    *fea_rec;
struct header	    *hd;
int		    type;
long		    *length;
char		    *vec;
{
    /* indexing variables */

    int	    i = 0;
    int	    j = 0;
    int	    k = 0;

    struct fea_header *fea_hdr;	    /* pointer to ESPS FEA Header */

    char    *field_name;	/* string to hold field name */
    char    *tmp_string;	/* temporary string to hold field name */
    char    *gen_range;		/* string to hold generic range */

    long    *item_array;	/* array containing items given in gen_range */
    long    n_item;		/* number of items given in gen_range */

    long    total_len = 0;	/* total length of "derived" vector */
    long    index = 0;		/* temporary index */

    long    max_size;		/* maximum element in gen_range requested */
    long    size;		/* maximum size of feature field in hd */

/* the following are used to access feature fields: see get_fea_ptr(3-ESPSu) */

    double  *d_ptr;
    float   *f_ptr;
    long    *l_ptr;
    short   *s_ptr;
    char    *c_ptr;

    BOOLEAN	name_found;	/* flag for matching feature field name */

/*
 *  B E G I N
 *   P R O G R A M
 */

    fea_hdr = hd->hd.fea;   /* makes life easier */

/* Allocate memory if vec = NULL */

    if (vec == NULL) {

    /* Compute the amount of memory required */

	while (fields[i] != NULL) {

	    /* make a copy of feature field name because
	     * strtok(3-UNIX) tends to destroy strings */

	    tmp_string = savestring (fields[i]);
	    field_name = strtok (tmp_string, "[");

	    gen_range = strtok (0, "]");
	    item_array = grange_switch (gen_range, &n_item);
	    max_size = item_array[n_item - 1];
	    total_len += n_item;

	    size = get_fea_siz (field_name, hd, (short *) NULL, (long **)NULL);

	    if (max_size >= size) {	/* incorrect generic range given */
		Fprintf (stderr,
		"%s: range of field %s is 0:%d, requesting element %ld.\n",
		Module, field_name, size - 1, max_size);
		exit(1);
	    }

#ifdef DEBUG
	    Fprintf (stderr, "%s: field_name = %s, gen_range = %s.\n",
	    Module, field_name, gen_range);
	    Fflush (stderr);
	    Fprintf (stderr, "%s: n_item = %ld, total_len = %ld\n",
	    Module, n_item, total_len);
	    Fflush (stderr);
#endif

	    i++;

            free(tmp_string);
	    free(item_array);
	}	/* end while (fields[i] != NULL) */

#ifdef DEBUG
	Fprintf (stderr, "%s: allocating memory: total_len = %ld\n",
	Module, total_len);
	Fflush (stderr);
#endif

    /* Allocate memory */

	switch (type) {

	    case DOUBLE:  ALLOCATE(double, total_len);
			  break;
	    case FLOAT:   ALLOCATE(float, total_len);
			  break;
	    case LONG:    ALLOCATE(long, total_len);
			  break;
            case CODED:
	    case SHORT:   ALLOCATE(short, total_len);
			  break;
	    case CHAR:    
	    case BYTE:    
			  ALLOCATE(char, total_len);
			  break;
	    default:
			  Fprintf (stderr,
			  "%s: Unknown type.\n", Module);
			  exit(1);

	}  /* end switch (type) */
    }  /* end if (vec == NULL) */
/*
 * B E G I N
 *  P R O C E S S I N G
 */

    i = 0;
    while (fields[i] != NULL) {
	tmp_string = savestring (fields[i]);
	field_name = strtok (tmp_string, "[");
	gen_range = strtok (0, "]");
        item_array = grange_switch (gen_range, &n_item);

	name_found = False;

	for (j = 0; j < fea_hdr->field_count; j++) {

	    if (strcmp (field_name, fea_hdr->names[j]) == 0) {

		name_found = True;

		/* Pack feature fields into a single vector
		 * which is a pointer to a char */

		switch (fea_hdr->types[j]) {

		    case DOUBLE: PACK_IT(double, d_ptr, g);
				 break;
		    case FLOAT:  PACK_IT(float, f_ptr, f);
				 break;
		    case LONG:   PACK_IT(long, l_ptr, ld);
				 break;
		    case CODED:
		    case SHORT:  PACK_IT(short, s_ptr, d);
				 break;
		    case CHAR:   
		    case BYTE:   
				 PACK_IT(char, c_ptr, c);
				 break;
		    default:	 Fprintf (stderr,
				 "%s: incorrect data type.\n", Module);

		}  /* end switch (fea_hdr->types[j]) */

		break;	/* get out of for loop */

	    }	/* end if (strcmp (field_name, ...)) */

	}   /* end for (j = 0; j < fea_hdr->field_count; j++) */

	if (name_found == False) {
	    Fprintf (stderr,
	    "%s: field name %s not found in ESPS Feature file.\n",
	    Module, field_name);
	    exit(1);
	}

	i++;
        free(tmp_string);
        free(item_array);
    }	/* end while (fields[i] != NULL) */


#ifdef DEBUG
    switch (type) {
	case DOUBLE:  PRINT(double, g);
		      break;
	case FLOAT:   PRINT(float, f);
		      break;
	case LONG:    PRINT(long, ld);
		      break;
	case SHORT:   PRINT(short, d);
		      break;
	case CHAR:    
	case BYTE:    
		      PRINT(char, c);
		      break;
    }
#endif

    *length = index;
    return ((char *) vec);	    /* return successfully */

}   /* end get_deriv_vec() */
