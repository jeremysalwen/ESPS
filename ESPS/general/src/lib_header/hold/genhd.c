/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This module contains support for the generic header items.  This
 * allows header items that aren't built into header.h to be created
 * at run-time.
 *
 */

static char *sccs_id = "@(#)genhd.c	1.29	28 Oct 1999	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define PWD_SIZE 512

char	*uniq_name();
static char *hostname();
static char *filename();
char *savestring();
char *add_genhd_xfile();


/* very simple hash function
*/
static int
hash (s)
char   *s;
{
    int     sum = 0;
    while (*s)
	sum += *s++;
    return (sum % GENTABSIZ);
}


/* look up an entry in the hash table.   Return NULL if not found, else
   return the address of the node
*/

static struct gen_hd   *
lookup (s, tab)
char   *s;
struct gen_hd  *tab[];
{
    struct gen_hd  *np;

    spsassert (tab != NULL && s != NULL,"error in lookup");
    for (np = tab[hash (s)]; np != NULL; np = np -> next)
	if (strcmp (s, np -> name) == 0)
	    return (np);	/* found it */
    return (NULL);		/* not found */
}

/* add_genhd creates a new header item and returns a pointer to
   it.    The pointer is returned as char *, so you must cast it to the
   right type.
*/

char   *
add_genhd (name, type, size, ptr, codes, hd)
char   *name;
int     type;
int     size;
char   *ptr;
char  **codes;
struct header  *hd;


{
    struct gen_hd  *np,
                   *lookup ();
    int     hashval;

/* be sure arguments are reasonable */
    spsassert (hd != NULL && name != NULL && size >= 1,"error in add_genhd");
    spsassert (type >= DOUBLE && type <= AFILE,"Bad type in add_genhd");
    spsassert (type != UNDEF,"Bad type in add_genhd");
    if (type == CODED)
	spsassert (codes != NULL,"Bad codes in add_genhd");
    spsassert (type != BYTE,"Type BYTE not implemented for generics yet");


/* see if name already defined, if it is, just wipe it out */
    np = lookup (name, hd -> variable.gentab);
    if (np == NULL) {
/* allocate a node if this name is not defined */
    	np = (struct gen_hd *) calloc (1, sizeof (*np));
    	spsassert (np != NULL,"calloc failed");

/* save the name */
   	np -> name = savestring (name);
    	spsassert (np -> name != NULL,"savestring failed");

/* insert the node into the hashtable */
    	hashval = hash (np -> name);
    	np -> next = hd -> variable.gentab[hashval];
    	hd -> variable.gentab[hashval] = np;

/* increment the counter */
    	hd -> variable.ngen++;
    }

/* save the size and type */
    np -> size = size;
    np -> type = type;

/* if its a CODED type, save the array of possible values */
    if (type == CODED)
	np -> codes = codes;

    if (ptr == NULL) {
/* allocate the data area and return a pointer to it */
	np -> d_ptr = calloc ((unsigned) size, (unsigned) typesiz (type));
	spsassert (np -> d_ptr != NULL,"calloc failed");
    }
    else {
/* else use the user supplied location */
	np -> d_ptr = ptr;
    }
    return np -> d_ptr;
}



/* genhd_type returns the type and size of a generic header item.
 */


int
genhd_type (name, size, hd)
char   *name;
int    *size;
struct header  *hd;

{

    struct gen_hd  *np,
                   *lookup ();

/* check the arguments */
    spsassert (name != NULL && hd != NULL,"Bad args to genhd_type");

/* if not defined, just return HD_UNDEF */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return HD_UNDEF;

/* return the size and type */
    if (size != NULL)
	*size = np -> size;
    return np -> type;
}


/* get_genhd returns a pointer to the data for a generic header item.  
   The fuction returns char *, so it must be cast into the right type.
*/

char   *
get_genhd (name, hd)
char   *name;
struct header  *hd;

{

    struct gen_hd  *np,
                   *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args to get_genhd");

/* return NULL if name not defined */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;

/* return pointer */
    return np -> d_ptr;
}


/* returns the number of generic header items and a pointer to an
   array of char pointers of the names.  The array has one more element
   than the number of items, the last element is NULL.
*/

char  **
genhd_list (size, hd)
int    *size;
struct header  *hd;

{
    struct gen_hd  *np;
    int     j = 0,
            i;
    char  **array;

/* check arguments */
    spsassert (hd != NULL,"hd is NULL in genhd_list");

/* if there are no items, just return NULL in array */
    if (hd -> variable.ngen == 0) {
	*size = 0;
	return NULL;
    }
/* allocate memory for array */
    array = (char **) calloc ((unsigned) (hd -> variable.ngen + 1),
	    sizeof (char *));
    spsassert (array != NULL,"calloc failed in genhd_list");

/* go through data structure and collect names */
    j = 0;
    for (i = 0; i < GENTABSIZ; i++) {
	np = hd -> variable.gentab[i];
	while (np != NULL) {
	    /* be sure count right */
	    spsassert (j < hd -> variable.ngen,"internal error in genhd_list");
	    array[j++] = np -> name;
	    np = np -> next;
	}
    }
    array[j] = NULL;
    *size = j;
    return (array);
}

/* add_genhd_d creates a header item of type double and returns a
   pointer to the data.  The user can supply a pointer through ptr */

double 
*add_genhd_d (name, ptr, size, hd)
char   *name;
double *ptr;
int     size;
struct header  *hd;
{

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size>0,"Bad args in add_genhd_d");

/* call add_genhd with the right arguments */
    return (double *) add_genhd (name, DOUBLE, size, (char *) ptr,
	    (char **) NULL, hd);
}

/* add_genhd_f creates a header item of type float and returns a
   pointer to the data.  The user can supply a pointer through ptr */

float  *add_genhd_f (name, ptr, size, hd)
char   *name;
float  *ptr;
int     size;
struct header  *hd;
{

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size>0,"Bad args in add_genhd_f");

/* call add_genhd with the right arguments */
    return (float *) add_genhd (name, FLOAT, size, (char *) ptr,
	    (char **) NULL, hd);
}

/* add_genhd_l creates a header item of type long and returns a
   pointer to the data.  The user can supply a pointer through ptr */

long   *add_genhd_l (name, ptr, size, hd)
char   *name;
long   *ptr;
int     size;
struct header  *hd;
{

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size>0,"Bad args in add_genhd_l");

/* call add_genhd with the right arguments */
    return (long *) add_genhd (name, LONG, size, (char *) ptr,
	    (char **) NULL, hd);
}

/* add_genhd_s creates a header item of type short and returns a
   pointer to the data.  The user can supply a pointer through ptr */

short  *add_genhd_s (name, ptr, size, hd)
char   *name;
short  *ptr;
int     size;
struct header  *hd;
{

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size>0,"Bad args in add_genhd_s");

/* call add_genhd with the right arguments */
    return (short *) add_genhd (name, SHORT, size, (char *) ptr,
	    (char **) NULL, hd);
}

/* add_genhd_c creates a header item of type char and returns a
   pointer to the data.  The user can supply a pointer through ptr */

char   *add_genhd_c (name, ptr, size, hd)
char   *name;
char   *ptr;
int     size;
struct header  *hd;
{

/* if ptr != NULL and size == 0, then assume a NULL terminated string */
    if (ptr != NULL && size == 0)
	size = strlen (ptr) + 1;

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size>0,"Bad args in add_genhd_c");

/* call add_genhd with the right arguments */
    return (char *) add_genhd (name, CHAR, size, (char *) ptr,
	    (char **) NULL, hd);
}


/* add_genhd_e creates a header item of type coded and returns a
   pointer to the data.  The user can supply a pointer to the data
   through ptr.   The user must supply a pointer to an array of
   strings that are the possible values.
*/
short  *
add_genhd_e (name, ptr, size, codes, hd)
char   *name;
short  *ptr;
int     size;
char  **codes;
struct header  *hd;
{

/* check the arguments */
    spsassert (name != NULL && hd != NULL && size > 0 && codes != NULL,
    "Bad args in add_genhd_e");

/* call add_genhd with the right arguments */
    return (short *) add_genhd (name, CODED, size, (char *) ptr, codes, hd);
}

/* genhd_codes returns the array of possible value strings associated
   with a generic header item of type coded */

char  **
genhd_codes (name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in genhd_codes");

/* return NULL if name not defined or not a CODED */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != CODED)
	return NULL;

/* return pointer to codes */
    return np -> codes;
}

/* returns the list of coded values for name */

char  **
get_genhd_coded (name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();
    char  **val;
    int     i;
    short  *s_ptr;

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in get_genhd_coded");

/* return NULL if name not defined or not a CODED */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != CODED)
	return NULL;

/* get values into the array of character pointers val */
    val = (char **) malloc ((unsigned) np->size * sizeof (char **));
    s_ptr = (short *) np->d_ptr;
    for (i = 0; i < np->size; i++)
	if (idx_ok(*s_ptr, np->codes))
	    val[i] = np->codes[*s_ptr++];
	else {
	    val[i] = "invalid code";  s_ptr++;
	}

    return val;
}


/* returns a header pointer for the external ESPS file */

struct header  *
get_genhd_efile(name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();
    struct header  *ext_hd;
    FILE * strm;
    int tmp_flag=0;
    char *tmp_file = "/tmp/genhdXXXXXX";

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in get_genhd_efile");

/* return NULL if name not defined or not a EFILE */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != EFILE)
	return NULL;

/* try and open the referenced file and read the header */
/* if it has a hostname, try and rcp the file from the remote host */

    if (hostname(np->d_ptr)) {
	char *cmd = malloc(80);
	(void)sprintf(cmd,"rcp %s %s",np->d_ptr,mktemp(tmp_file));
	if (system(cmd) != 0) {
		Fprintf(stderr,
		 "get_genhd_efile: problem getting file %s from host %s.\n",
		 np->d_ptr, hostname(np->d_ptr));
	}
	(void)free(cmd);
        if ((strm = fopen (tmp_file, "r")) == NULL)
		return NULL;
	tmp_flag=1;
    } else
    if ((strm = fopen (np -> d_ptr, "r")) == NULL)
	return NULL;

    ext_hd = read_header (strm);

/* close the file and return */
    (void) fclose (strm);
    if (tmp_flag) (void)unlink(tmp_file);
    return ext_hd;
}

/* returns a file stream pointer for the external file */

FILE *
get_genhd_afile(name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();
    FILE * strm;
    int tmp_flag=0;
    char *tmp_file = "/tmp/genhdXXXXXX";

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in get_genhd_efile");

/* return NULL if name not defined or not a AFILE */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != AFILE)
	return NULL;

/* try and open the referenced file and read the header */
/* if it has a hostname, for now just punt */

    if (hostname(np->d_ptr)) {
	char *cmd = malloc(80);
	(void)sprintf(cmd,"rcp %s %s",np->d_ptr,mktemp(tmp_file));
	if (system(cmd) != 0) {
		Fprintf(stderr,
		 "get_genhd_efile: problem getting file %s from host %s.\n",
		 np->d_ptr, hostname(np->d_ptr));
	}
	(void)free(cmd);
        if ((strm = fopen (tmp_file, "r")) == NULL)
		return NULL;
	tmp_flag=1;
    } else
    if ((strm = fopen (np -> d_ptr, "r")) == NULL)
	return NULL;

/* close the file and return */
    if (tmp_flag) (void)unlink(tmp_file);
    return strm;
}

/* returns the name for an external file */

char   *
get_genhd_efile_name (name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in get_genhd_efile_name");

/* return NULL if name not defined or not a EFILE */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != EFILE)
	return NULL;

    return np -> d_ptr;
}

/* returns the name for an external file */

char   *
get_genhd_afile_name (name, hd)
char   *name;
struct header  *hd;
{
    struct gen_hd  *np,
                   *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args in get_genhd_afile_name");

/* return NULL if name not defined or not a AFILE */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return NULL;
    if (np -> type != AFILE)
	return NULL;

    return np -> d_ptr;
}

int
copy_genhd (dest, src, name)
struct header  *dest,
               *src;
char *name;
{

    int     i,
            count = 0,
            k;
    char   *ptr;
    struct gen_hd  *np, *lookup();

/* check arguments */
    spsassert (dest && src,"Bad args in copy_genhd");

/* check for no items in src */
    if (src -> variable.ngen == 0)
	return 0;

    if (name == NULL) {
/* find entries */
       for (i = 0; i < GENTABSIZ; i++) {
	np = src -> variable.gentab[i];
	while (np != NULL) {
/* create new item and copy data */
	    ptr = add_genhd (np -> name, np -> type, (int) np -> size, 
		(char *)NULL, np -> codes, dest);
	    for (k = 0; k < np -> size * typesiz (np -> type); k++)
		ptr[k] = np -> d_ptr[k];
	    np = np -> next;
	    count++;
	}
       }
    return count;
   } else {
       if ((np = lookup(name, src->variable.gentab)) == NULL)
	return 0;
/* create new item and copy data */
       ptr = add_genhd (name, np -> type, (int) np -> size, 
		(char *)NULL, np -> codes, dest);
       for (k = 0; k < np -> size * typesiz (np -> type); k++)
		ptr[k] = np -> d_ptr[k];
       return 1;
   }
}

int
copy_genhd_uniq (dest, src, name)
struct header  *dest,
               *src;
char *name;
{

    char    	**dest_names;	/* names of header items
				   from init_feaspec_hd() */
    char    	*one_name[2];	/* NULL-terminated list containing name */
    char	**src_names;	/* names of input header items */
    char    	*uname;		/* header item name, possibly
				   altered by uniq_name() */
    int	    	type;		/* type of header item */
    int	    	size;		/* size of header item */
    int	    	nitems = 0;	/* number of src header items */
    int     	i;

/* check arguments */
    spsassert (dest && src,"Bad header args in copy_genhd_uniq");

/* check for no items in src */
    if (src -> variable.ngen == 0)
	return(0);

/* check for no items in dest */

    if (dest -> variable.ngen == 0) 
      return(copy_genhd(dest, src, name));

/* Copy generic header items, renaming to avoid conflicts*/

    dest_names = genhd_list(&nitems, dest);

    if (name != NULL) {
	one_name[0] = name;
	one_name[1] = NULL;
	src_names = one_name;
	nitems = (genhd_type(name, &size, src) == HD_UNDEF) ? 0 : 1;
    }
    else
	src_names = genhd_list(&nitems, src);

    for (i = 0; i < nitems; i++) {
	    if (genhd_type(*src_names, &size, dest) == HD_UNDEF)
		uname = *src_names;
	    else
		uname = uniq_name(*src_names, dest_names);

	    type = genhd_type(*src_names, &size, src);
	    (void) add_genhd(uname, type, size,
			     get_genhd(*src_names, src),
			     genhd_codes(*src_names, src),
			     dest);
	    src_names++;
	  }
    return(nitems);
}



/* this function returns a from a:/foo/bar */

static
char *
hostname(s)
char *s;
{
   if (strchr (s,':') == NULL) return NULL;

   return strtok(savestring(s),":");

}

/* this function returns /foo/bar from a:/foo/bar */

static
char *
filename(s)
char *s;
{

   return strchr(s,':')+1;
}

/* add_genhd_efile creates a header item of type EFILE and returns a
   pointer to the file_name */

char *
add_genhd_efile (item_name, file_name, hd)
char   *item_name;
char   *file_name;
struct header *hd;

{
    
   spsassert(item_name && file_name && hd,"Bad args in add_genhd_efile");
   return add_genhd_xfile(item_name, file_name, hd, EFILE);

}

/* add_genhd_afile creates a header item of type AFILE and returns a
   pointer to the file_name */

char *
add_genhd_afile (item_name, file_name, hd)
char   *item_name;
char   *file_name;
struct header *hd;

{
    
   spsassert(item_name && file_name && hd,"Bad args in add_genhd_afile");
   return add_genhd_xfile(item_name, file_name, hd, AFILE);

}

/* add_genhd_xfile creates a header item of type EFILE or AFILE and returns a 
   pointer to the file_name */

char   *
add_genhd_xfile (item_name, file_name, hd, type)
char   *item_name;
char   *file_name;
struct header  *hd;
int    type;

{
    char   *pwd, *name;
    int     size;

    name = savestring(file_name);
    if (hostname (name)) {
	if (*filename (name) != '/') {
	    Fprintf (stderr,
	     "add_genhd_xfile: when using a host name, a full path must be given.\n");
	    exit (1);
	}
	pwd = name;
    }
    else
    if (*name != '/') {
	pwd = malloc ((unsigned) PWD_SIZE);
	spsassert (pwd,"malloc failed in add_genhd_xfile");
#if !defined(hpux) & !defined(OS5) & !defined(LINUX_OR_MAC)
	if (getwd (pwd) == NULL) {
#else
	if (getcwd (pwd,PWD_SIZE) == NULL) {
#endif
	    Fprintf (stderr, "add_genhd_xfile: cannot get cwd!\n");
	    (void)strcpy (pwd, " ");
	}
	(void) strcat (pwd, "/");
	(void) strcat (pwd, name);
    }
    else
	pwd = name;

    size = strlen (pwd) + 1;

/* call add_genhd with the right arguments */
    return (char *) add_genhd (item_name, type, size, pwd, (char **) NULL, hd);
}

long
get_genhd_val_l(name, hd, def_val)
char *name;
struct header *hd;
long def_val;

{

    struct gen_hd  *np, *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args to get_genhd_val");

/* return def_val value if name not defined */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return def_val;

/* return value as a long for numeric types */

    switch (np -> type) {
	case BYTE:
	 return *np -> d_ptr;
	case SHORT:
	 return *(short *)np -> d_ptr;
	case LONG:
	 return *(long *)np -> d_ptr;
	case FLOAT:
	 return *(float *)np -> d_ptr;
	case DOUBLE:
	 return *(double *)np -> d_ptr;
	default:
	 Fprintf(stderr,
          "get_genhd_val_l: default value returned for %s because type is %s\n",
	  name, type_codes[np->type]);
	 return def_val;
    }
}

double
get_genhd_val(name, hd, def_val)
char *name;
struct header *hd;
double def_val;

{

    struct gen_hd  *np, *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args to get_genhd_val");

/* return def_val value if name not defined */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return def_val;

/* return value as a double for numeric types */

    switch (np -> type) {
	case BYTE:
	 return *np -> d_ptr;
	case SHORT:
	 return *(short *)np -> d_ptr;
	case LONG:
	 return *(long *)np -> d_ptr;
	case FLOAT:
	 return *(float *)np -> d_ptr;
	case DOUBLE:
	 return *(double *)np -> d_ptr;
	default:
	 Fprintf(stderr,
          "get_genhd_val: default value returned for %s because type is %s\n",
	  name, type_codes[np->type]);
	 return def_val;
    }
}

double
get_genhd_val_array(name, index, hd, def_val)
char *name;
int index;
struct header *hd;
double def_val;

{

    struct gen_hd  *np, *lookup ();

/* check arguments */
    spsassert (name != NULL && hd != NULL,"Bad args to get_genhd_val");
    spsassert (index > -1, "index less than zero in get_genhd_val");

/* return def_val value if name not defined */
    if ((np = lookup (name, hd -> variable.gentab)) == NULL)
	return def_val;

/* return def_val value if index is greater than number of items */
    if (index > (np->size-1))
	return def_val;

/* return value as a double for numeric types */

    switch (np -> type) {
	case BYTE:
	 return *(np -> d_ptr + index);
	case SHORT:
	 return * ((short *)np -> d_ptr + index);
	case LONG:
	 return *((long *)np -> d_ptr + index);
	case FLOAT:
	 return *((float *)np -> d_ptr + index);
	case DOUBLE:
	 return *((double *)np -> d_ptr + index);
	default:
	 Fprintf(stderr,
          "get_genhd_val: default value returned for %s because type is %s\n",
	  name, type_codes[np->type]);
	 return def_val;
    }
}
