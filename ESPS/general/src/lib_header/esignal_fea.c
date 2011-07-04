/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:  Functions for use by ESPS I/O routine in dealing
 * with files in Esignal format.
 *
 */

static char *sccs_id = "@(#)esignal_fea.c	1.4	10/6/97	ERL";


#include <esps/esps.h>
#include <esps/fea.h>
/*
 * NAME CONFLICT:  The ESPS data-type codes named
 * DOUBLE, FLOAT, LONG, SHORT, and CHAR
 * are different from the Esignal data-type codes with the same names.
 * While the ESPS definitions are in effect, declare variables so that
 * the ESPS codes can be referred to as 
 * FDOUBLE, FFLOAT, FLONG, FSHORT, and FCHAR
 * instead.  (Think of "fea-double", "fea-float", etc.)
 * After the inclusion of <esignal.h> below, the Esignal definitions of
 * DOUBLE, FLOAT, LONG, SHORT, and CHAR
 * take effect.
 */
static int FDOUBLE = DOUBLE;
#undef	DOUBLE
static int FFLOAT = FLOAT;
#undef	FLOAT
static int FLONG = LONG;
#undef	LONG
static int FSHORT = SHORT;
#undef	SHORT
static int FCHAR = CHAR;
#undef	CHAR
#undef FLT_MAX
#undef FLT_MIN
#undef BOOL
#include <esignal.h>
#include <esps/esignal_fea.h>

char	    *savestring(char *); /* ESPS library function.  Could replace
				  * with "strdup" if available. */
long	    get_genhd_val_l(char *, struct header *, long); /* See genhd.c */


extern int  EspsDebug;		/* set in headers.c for header debugging */
int	    EsignalPrefixSize = sizeof MAGIC;

/*
 * LOCAL TYPEDEFS AND STRUCTURES
 */

struct esignal_hd {
    int		    arch;	/* format: ASCII, NATIVE, EDR1, or EDR2 */
    long	    pre_size;	/* preamble size */
    long	    hdr_size;	/* header size (bytes) */
    long	    rec_size;	/* record size (or -1 for "variable") */
    FieldList	    fields;	/* field list */
    FieldSpec	    **lin_list;	/* input fields in field or type order */
    int		    one_fld;	/* just one input field? */
    long	    *fea_tag_ptr;
				/* pointer to "tag" in last FEA record read */
    struct fea_data fea_rec;	/* copy of pointers in last FEA record read */
};

/*
 *  LOCAL FUNCTION DECLARATIONS
 */

/* used by esignal_to_fea */
FieldList		GetHeader(char **version, int *arch, long *pre_size,
				  long *hdr_size, long *rec_size,
				  char *prefix, int num_read, FILE *file);
static struct header	*FieldList_to_fea(FieldList list,
					  struct fea_data **rec,
					  char **fnames, int copy_sources);

/* used by GetHeader */
static int		GetPreamble(char **version, char **arch,
				    long *pre_size, long *hdr_size,
				    long *rec_size, char *str);

/* used by GetPreamble */
static int		GetLine(char *buf, int len, char **src);
static int		GetLong(long *val, int len, char **src);

/* used by FieldList_to_fea */
static int		FindStr(char *str, char **arr);
static char		**StrArrayFromRect(long *dim, void *data);
static int		FieldIsTag(FieldSpec *fld);
static int		FieldIsFeaSubtype(FieldSpec *fld);
static int		ElibTypeToEsps(int type);


/*
 * Check whether "prefix" points to a correct sequence of bytes for
 * the beginning of an Esignal file preamble.  If so, return TRUE;
 * otherwise return FALSE.
 */

int
esignal_check_prefix(char *prefix)
{
    return (prefix != NULL
	    && strncmp(prefix, MAGIC, (sizeof MAGIC) - 1) == 0
	    && prefix[(sizeof MAGIC) - 1] == '\n');
}


/*
 * Assume "prefix" contains "num_read" bytes read from the beginning of
 * file "file" and that "file" is positioned for reading the following
 * characters.  If the catenation of "prefix"  with the remainder of "file"
 * begins with an Esignal file header, and if the part in "prefix" does
 * not go beyond the end of the preamble, the function reads the rest
 * of the header, converts it to an ESPS header insofar as is possible,
 * and leaves the file positioned at the end of the header, ready for
 * reading of any data records that follow.  The return value is a
 * pointer to the ESPS header, or NULL in case of error.
 * This function is intended for use when it has been necessary to read
 * a number of characters at the beginning of a file to determine whether
 * it is in an Esignal external format or some other (such as ESPS or NIST).
 * If "file" were known to be seekable, we could just rewind, call
 * ReadHeader, and convert the result.  However, this function is
 * intended to work on pipes.
 */

struct header *
esignal_to_fea(char *prefix,
	       int  num_read,
	       FILE *file)
{
    FieldList	    list;	    /* input field list */
    char	    *version;	    /* version from input preamble */
    int		    arch;	    /* architecture from input preamble */
    long	    pre_size;	    /* preamble size */
    long	    hdr_size;	    /* header size (bytes) from preamble */
    long	    rec_size;	    /* record size from preamble */
    int		    inord;	    /* input: field order or type order? */
    FieldSpec       **ifields;	    /* input fields in field or type order */
    FieldSpec	    *fld;
    struct header   *ohd;	    /* output ESPS file header */
    double	    rec_freq;
    double	    start_time;
    esignal_hd	    *hd_ptr;


    DebugMsgLevel = EspsDebug;
    DebugMsgFunc = DebugPrint;

    /*
     * Get Esignal field list and other header information from prefix
     * and file.
     */

    list = GetHeader(&version, &arch, &pre_size, &hdr_size, &rec_size,
		     prefix, num_read, file);
    if (list == NULL)
    {
	DebugMsg(1, "esignal_to_fea: get header failed.");
	return NULL;
    }

    /*
     * Check that record size in preamble is consistent with field
     * list.
     */

    if (RecordSize(list, arch) != rec_size)
    {
	DebugMsg(1, "esignal_to_fea: inconsistent record size.");
	return NULL;
    }

    /*
     * Make linear list of REQUIRED and OPTIONAL fields in the order
     * (type order or field order) in which they occur in records.
     */

    if (!GetFieldOrdering(list, &inord))
    {
	DebugMsg(1, "esignal_to_fea: can't get field ordering of input.");
	return NULL;
    }

    switch (inord)
    {
    case TYPE_ORDER:
	if (DebugMsgLevel)
	    fprintf(stderr, "making type-ordered field array.\n");
	ifields = TypeOrder(list);
	break;
    case FIELD_ORDER:
	if (DebugMsgLevel)
	    fprintf(stderr, "making field-ordered field array.\n");
	ifields = FieldOrder(list);
	break;
    default:
	DebugMsg(1, "input order neither TYPE_ORDER nor FIELD_ORDER");
	return NULL;
    }

    /*
     * Convert Esignal field list to ESPS FEA file header.
     */

    ohd = FieldList_to_fea(list, NULL, (char **) NULL, TRUE);
    if (ohd == NULL)
    {
	DebugMsg(1, "failure converting Esignal field list to ESPS header.");
	return NULL;
    }

    /*
     * Set field_order flag.
     */

    if (inord == FIELD_ORDER)
	ohd->hd.fea->field_order = TRUE;

    /*
     * Set FEA subtype code if (1) GLOBAL field "FeaSubtype" so indicates
     * or (2) REQUIRED field "samples" implies FEA_SD.
     */

    if ((fld = FindField(list, "FeaSubtype")) != NULL
	&& fld->occurrence == GLOBAL
	&& fld->type == SHORT
	&& fld->data != NULL)
    {
	ohd->hd.fea->fea_type = *(short *) fld->data;
    }
    else if ((fld = FindField(list, "samples")) != NULL
	     && fld->occurrence == REQUIRED
	     && is_type_numeric(ElibTypeToEsps(fld->type)))
    {
	ohd->hd.fea->fea_type = FEA_SD;
    }

    /*
     * Set generics "record_freq" and "start_time"
     */

    if ((fld = FindField(list, "recordFreq")) != NULL
	&& fld->occurrence == GLOBAL
	&& fld->data != NULL)
    {
	(void) type_convert(1L, (char *) fld->data, ElibTypeToEsps(fld->type),
			    (char *) &rec_freq, FDOUBLE, (void (*)()) NULL);

	*add_genhd_d("record_freq", NULL, 1, ohd) = rec_freq;
    }

    if ((fld = FindField(list, "startTime")) != NULL
	&& fld->occurrence == GLOBAL
	&& fld->data != NULL)
    {
	(void) type_convert(1L, (char *) fld->data, ElibTypeToEsps(fld->type),
			    (char *) &start_time, FDOUBLE, (void (*)()) NULL);

	*add_genhd_d("start_time", NULL, 1, ohd) = start_time;
    }

    /*
     * Show conversion from Esignal in program field and comment.
     */

    strcpy(ohd->common.prog, "esignal_to_fea");
    add_comment(ohd, "Converted from Esignal header");

    /*
     * Make a generic "esignal_hd_ptr" containing a pointer to a
     * structure containing the Esignal field list and other
     * information needed for reading data.
     */

    hd_ptr = (esignal_hd *) malloc(sizeof(esignal_hd));
    hd_ptr->arch = arch;
    hd_ptr->pre_size = pre_size;
    hd_ptr->hdr_size = hdr_size;
    hd_ptr->rec_size = rec_size;
    hd_ptr->fields = list;
    hd_ptr->lin_list = ifields;
    hd_ptr->one_fld =
	(ifields != NULL && ifields[0] != NULL
	 && ifields[1] == NULL && ifields[0]->occurrence == REQUIRED);
    hd_ptr->fea_tag_ptr = NULL;
    {
	static struct fea_data	nulls;	/* Initialized to zeros and NULLs. */
	hd_ptr->fea_rec = nulls;
    }
    *add_genhd_l("esignal_hd_ptr", (long *) NULL, 1, ohd) = (long) hd_ptr;

    return ohd;
}


/*
 * Under the same assumptions on "prefix", "num_read", and "file" as
 * are made for "esignal_to_fea", read the remainder of the Esignal
 * header from "file", and leave the file positioned at the end of the
 * header, ready for reading of any data records that follow.  Return
 * value is the field list (NULL on failure).  Information from the
 * preamble is returned via pointer arguments.  This function is
 * similar to "ReadHeader", but is for use when it has been necessary
 * to read ahead some number of characters in the file.
 */

#define PRE_SIZE ((size_t) 48)	/* preamble size: 6 lines of 8 characters
				 * each, including terminal '\n' characters */
FieldList
GetHeader(char	**version,	/* version (output) */
	  int	*arch,		/* architecture (output) */
	  long	*pre_size,	/* preamble size  (output) */
	  long	*hdr_size,	/* header size (output) */
	  long	*rec_size,	/* record size (output) */
	  char	*prefix,	/* initial bytes of preamble */
	  int	num_read,	/* number of initial bytes */
	  FILE	*file)		/* input file */
{
    FieldList	list;
    char	*arch_string;
    int		architecture;
    char	*pre;		/* characters of full preamble */

    if (num_read > PRE_SIZE)
    {
	DebugMsg(1, "GetHeader: read too far ahead.");
	return NULL;
    }
    pre = malloc(PRE_SIZE);
    memcpy(pre, prefix, (size_t) num_read);
    if (num_read < PRE_SIZE
	&& fread(pre + num_read, PRE_SIZE - num_read, 1, file) == 0)
    {
	DebugMsg(1, "GetHeader: couldn't read rest of Esignal preamble.");
	return NULL;
    }

    if (!GetPreamble(version,
		     &arch_string, pre_size, hdr_size, rec_size, pre))
	return NULL;	/* Bad preamble. */

    if (strcmp(arch_string, EsignalArch) == 0)    /* native architecture */
	architecture = NATIVE;
    else if (strcmp(arch_string, "EDR1") == 0)
	architecture = EDR1;
    else if (strcmp(arch_string, "EDR2") == 0)
	architecture = EDR2;
    else if (strcmp(arch_string, "ASCII") == 0)
	architecture = ASCII;
    else
    {
	if (arch != NULL)
	    *arch = UNKNOWN;
	return NULL;		/* Unsupported architecture. */
    }

    if (arch != NULL)
	*arch = architecture;

    if (!ReadFieldList(&list, architecture, file))
	return NULL;

    return list;
}


/*
 * Retrieve a pointer "hd_ptr" to an "esignal_hd" structure that has
 * been converted to long and added as a generic to a ESPS header by
 * code like the following:
 *
 *   *add_genhd_l("esignal_hd_ptr", (long *) NULL, 1, hdr) = (long) hd_ptr;
 *
 * where "hd_ptr" has type (esignal_hd *) (as in "esignal_to_fea"
 * above).  Return the pointer, or NULL if the input is NULL.
 */

esignal_hd *
get_esignal_hdr(struct header *hdr)
{
    long    gen;

    if (hdr == NULL)
	return NULL;
    else
    {
	gen = get_genhd_val_l("esignal_hd_ptr", hdr, 0L);
	if (gen == 0)
	    return NULL;
	else
	    return (esignal_hd *) gen;
    }
}


/*
 * Get the size, in bytes, of a record of a file in an Esignal external
 * format.  This is the value stored in the rec_size member of the
 * esignal_hd structure after checking consistency of the value in the
 * preamble with the result of RecordSize.  This value is -1 for Esignal
 * files with variable-sized records.  The return value is also -1 in case
 * of error (NULL argument), though size_rec and size_rec2 won't call
 * this function in that case.
 */
/*!*/
/* Current ESPS and waves don't check for the -1 return from
 * size_rec or size_rec2.  Check all uses of those functions.
 * Put a note in the man page.
 */

long
esignal_rec_size(esignal_hd *esig_hd)
{
    if (esig_hd == NULL)
    {
	DebugMsg(1, "esignal_rec_size: Esignal header pointer is NULL.");
	return -1;
    }

    return esig_hd->rec_size;
}


/*
 * Skip forward "nrec" records in a file "strm" in the Esignal external
 * format specified by the field list and other header information in the
 * esignal_hd structure indicated by "esig_hd".  For fixed-sized records,
 * just use the ESPS function "skiprec".  For variable-sized records
 * (indicated by a return of -1 from esignal_rec_size) read and discard
 * records one at a time.
 */

void
esignal_skip_recs(FILE *strm, long nrec, esignal_hd *hd)
{
    long	size;

    if (hd == NULL)
    {
	DebugMsg(1, "esignal_skip_recs: Esignal header pointer is NULL.");
	return;
    }

    if (nrec == 0)
	return;

    size = hd->rec_size;

    if (size != -1)
	skiprec(strm, nrec, (int) size);
    else
    {
	FieldSpec   **lin_list = hd->lin_list;
	int	    arch = hd->arch;
	int	    i;

	if (nrec < 0)
	    DebugMsg(1, "esignal_skip_recs: "
		     "Can't back up in file with variable record size.");

	/* Avoid reading to possibly freed memory. */

	for (i = 0; lin_list[i]; i++)
	    lin_list[i]->data = NULL;

/*!*//* Consider using ReadSamples when possible */

	for ( ; nrec > 0 && ReadRecord(lin_list, arch, strm); nrec--)
	{ }

	if (nrec > 0)
	    DebugMsg(1, "esignal_skip_recs: ReadRecord failed.");
    }
}


/*
 * Read a record from an Esignal file and store the contents (or some
 * of the contents) in an ESPS FEA record structure.  The file "file"
 * should be in Esignal format and positioned at the beginning of the
 * record to be read, and "rec" should point to the record structure.
 * The argument "hd" should point to an esignal_hd structure such as
 * "esignal_to_fea" creates when reading the Esignal file header.
 * The argument "fea_hd" should point to an ESPS FEA header structure
 * whose fields correspond in name, data type, rank, and dimensions
 * to some of the REQUIRED fields of the Esignal file.  (Such a header
 * is returned by "esigal_to_fea".)  The record record structure must
 * be compatible with the FEA header.  The typical sequence of events
 * is:
 *
 *	fea_hd = esignal_to_fea( ..., file);
 *	hd = get_esignal_hdr(fea_hd);
 *	rec = allo_fea_rec(fea_hd);
 *
 * followed by a loop that repeatedly executes:
 *
 *	esignal_get_rec(rec, hd, fea_hd, file);
 * 
 * The contents of Esignal fields that do not correspond to fields in
 * the FEA header are read and discarded.  The function returns 1 upon
 * success and EOF upon error or end of file.
 */
/*!*//* MEMORY LEAK: if the Esignal file contains fields of type ARRAY,
 * memory for the contents will be allocated and abandoned unfreed.
 * Should save a list of such fields and free them, or make provisions
 * not to create the array values in the first place.
 */

int
esignal_get_rec(struct fea_data	*rec,
		esignal_hd	*hd,
		struct header	*fea_hd,
		FILE		*file)
{
    if (!rec || !hd || !file)
    {
	DebugMsg(1, "esignal_get_rec: NULL argument.");
	return EOF;
    }

    /* Setup code to make the "data" members of the field specs of the
     * fields to be read point to the right locations in the FEA
     * record to receive the data.  Since get_fea_rec for a given
     * input file is typically called repeatedly with the same
     * fea_data structure, we want to avoid repeating the setup for
     * every call.  We save a copy of *rec (i.e. the actual data pointers
     * in the fea_data structure, not the pointer "rec") and a pointer
     * to the "tag" member.  If these pointers are unchanged from
     * the previous call we skip the setup.
     * Things will BREAK if the contents of the "data" member of an input
     * field spec are changed.  Code that does so should first null out
     * "fea_tag_ptr" or part of "fea_rec" in the esignal_hd structure
     * so that the next call of this function will redo the setup.
     * Cf. "esignal_getsd_recs".
     */

    /* Here we rely on the fact that the data pointers in a fea_data
     * structure comprise the part following the tag, which is a long
     * and is the first member.
     */
    if (hd->fea_tag_ptr != &rec->tag
	|| memcmp((char *) &hd->fea_rec + sizeof(long),
		  (char *) rec + sizeof(long),
		  sizeof(struct fea_data) - sizeof(long)))
    {
	FieldSpec	    *fld;
	int		    i, fld_count;
	char		    *name, **fld_names;

/*!*//* Essentially the same code near the end of FieldList_to_fea
  * (this file and esig2fea.c).  Consolidate?
  */
	hd->fea_tag_ptr = &rec->tag;
	hd->fea_rec = *rec;

	if (fea_hd->common.tag)
	{
	    fld = FindField(hd->lin_list, ESPS_TAG);
	    fld->data = &rec->tag;
	}

	fld_count = fea_hd->hd.fea->field_count;
	fld_names = fea_hd->hd.fea->names;

	for (i = 0; i < fld_count; i++)
	{
	    name = fld_names[i];
	    fld = FindField(hd->lin_list, name);
	    fld->data = get_fea_ptr(rec, name, fea_hd);
	}
    }

    if (ReadRecord(hd->lin_list, hd->arch, file))
	return 1;
    else
	return EOF;
}


/*
 * Read sampled data from an Esignal file and store the samples in a buffer
 * array.  The file "file" should be in Esignal format and positioned at
 * the beginning of the sequence of records to be read, and "buffer"
 * should point to the beginning of the buffer array.  The number of
 * records be read is given by "num_records".  The argument "hd"
 * should point to an esignal_hd structure such as "esignal_to_fea"
 * creates when reading the Esignal file header.  If the file has just
 * one REQUIRED field, and no OPTIONAL fields, the contents of that field
 * are read.  If it has more than one field, one field must be named
 * "samples", and its contents are used; the contents of other fields are
 * discarded.  The number of records actually read is returned; this
 * will be less than "num_records" in case of error or end of file.
 */

long
esignal_getsd_recs(char		*buffer,
		   long		num_records,
		   esignal_hd	*hd,
		   FILE		*file)
{
    FieldSpec   **lin_list;
    int		arch;


    if (buffer == NULL || hd == NULL || file == NULL)
    {
	DebugMsg(1, "esignal_getsd_recs: NULL arguments.");
	return 0;
    }

    if (num_records == 0)
	return 0;

    lin_list = hd->lin_list;

    if (lin_list == NULL)
    {
	DebugMsg(1, "esignal_getsd_recs: no fields to be read.");
	return 0;
    }

    arch = hd->arch;
    
    if (hd->one_fld)		/* Can read with ReadSamples. */
    {
	return ReadSamples(buffer, num_records, lin_list, arch, file);
    }
    else			/* Read single records repeateldy. */
    {
	FieldSpec   *spec;
	long	    step;
	long	    num_read;
	

	/*
	 * We are about to invalidate hd->fea_rec by changing the "data"
	 * pointer in a field spec that may point into the fea_data
	 * structure.  (See comment in esignal_get_rec.)  Null part of
	 * hd->fea_rec to avoid fouling up the next call of esignal_get_rec.
	 */

	hd->fea_tag_ptr = NULL;
	hd->fea_rec.d_data = NULL;

	spec = FindField(hd->fields, "samples");

	if (spec == NULL)
	{
	    DebugMsg(1, "esignal_getsd_recs: "
		     "couldn't find field \"samples\".");
	}

	spec->data = buffer;
	step = InternTypeSize(spec->type);
    
	for (num_read = 0;
	     num_read < num_records && ReadRecord(lin_list, arch, file);
	     num_read++, spec->data = (char *) spec->data + step)
	{ }

	return num_read;
    }
}

/* LOCAL FUNCTION DEFINITIONS */

/*
 * Similar to ReadPreamble, but takes input from a character string in
 * memory instead of a file.  Return Esignal preamble information via
 * pointer arguments.  Return TRUE on success, FALSE on failure.
 */

static int
GetPreamble(char    **version,	/* version (output) */
	    char    **arch,	/* architecture (output) */
	    long    *pre_size,	/* preamble size (output) */
	    long    *hdr_size,	/* header size (output) */
	    long    *rec_size,	/* record size (output) */
	    char    *str)	/* start of input characters */
{
    char	    buf[PREAM_MAX + 1]; /* preamble line + null */

    /* Check magic number, MAGIC. */

    if (!GetLine(buf, 8, &str))
	return FALSE;

    if (strcmp(buf, MAGIC) != 0)
	return FALSE;

    if (!GetLine(buf, 8, &str))
	return FALSE;
    if (version != NULL)
	*version = StrDup(buf);

    /* Get architecture. */

    if (!GetLine(buf, 8, &str))
	return FALSE;
    if (arch != NULL)
	*arch = StrDup(buf);

    /* Get preamble size */

    if (!GetLong(pre_size, 8, &str))
	return FALSE;

    /* Could check *pre_size here. */

    /* Get header size */

    if (!GetLong(hdr_size, 8, &str))
	return FALSE;

    /* Get record size */

    if (!GetLong(rec_size, 8, &str))
	return FALSE;

    return TRUE;		/* success */
}


/*
 * Extract a line of length len (including '\n') into buf from
 * the string indicated by the variable whose addresss is "src".
 * Increment the variable to point to the next character after
 * the extracted substring.
 * Check length of line, presence of terminating newline,
 * and absence of trailing blanks.
 * Trim newline and leading blanks and supply terminating null.
 * Return TRUE on success, FALSE on failure.
 * This is like the function "GetLine" in esignal.c, but takes
 * characters from a string in memory rather than a file.
 */

static int
GetLine(char    *buf,
	int     len,
	char    **src)
{
    int		i, j;

    if (src == NULL || *src == NULL)
	return FALSE;
    memcpy(buf, *src, (size_t) len);
    buf[len] = '\0';
    *src += len;

    if (strlen(buf) != len || buf[len-1] != '\n')
	return FALSE;
    buf[len-1] = '\0';

    /* count leading blanks */
    for (j = 0; buf[j] == ' '; j++)
    { }

    if (j < len - 1 && buf[len-2] == ' ')
	return FALSE;		/* not right justified */

    if (j == 0)
	return TRUE;

    /* remove leading blanks */
    for (i = 0; buf[j] != '\0'; i++,j++)
	buf[i] = buf[j];

    buf[i] = '\0';

    return TRUE;		/* success */
}


/*
 * Extract a line of length len (including '\n') from the string
 * indicated by the variable whose addresss is "src".  Increment the
 * variable to point to the next character after the extracted
 * substring.  Check length of line and presence of terminating
 * newline.  Convert to long integer, assign result through pointer
 * "val" if non-NULL.  Check for garbage characters following the
 * constant.  Return TRUE on success, FALSE on failure.
 */

static int
GetLong(long    *val,
	int     len,
	char    **src)
{
    char    *buf;
    char    *ptr;		/* end-of-scan pointer from strtol */
    long    value;		/* converted value */

    buf = (char *) malloc(len+1);   /* len + 1 for terminating null */
    if (buf == NULL)
	return FALSE;		/* Allocation failure. */

    /* Get line; check length. */

    if (src == NULL || *src == NULL)
	return FALSE;
    memcpy(buf, *src, (size_t) len);
    buf[len] = '\0';
    *src += len;

    if (strlen(buf) != len || buf[len-1] != '\n')
    {
	free(buf);
	return FALSE;
    }

    /* Convert; check for bad format. */

    value = strtol(buf, &ptr, 10);

    if (ptr != buf + (len-1))
    {
	free(buf);
	return FALSE;
    }

    /* Clean up; return. */

    free(buf);

    if (val != NULL)
	*val = value;

    return TRUE;		/* success */
}


/*!*/
/* The functions "FieldList_to_fea", "FindStr", "StrArrayFromRect",
 * "FieldIsTag", "FieldIsFeaSubtype", and "ElibTypeToEsps" are verbatim
 * copies of functions in esig2fea.c (except for the conditions that
 * control printing of debug messages).  Move to a separate file that
 * can be shared by the ESPS library and esig2fea.
 */

/*
 * Convert an Esignal field list "list" to an ESPS FEA file header
 * that contains, generally speaking (1) a field definition for each
 * top-level REQUIRED field whose type corresponds to a possible FEA
 * field data type, and (2) a generic header item for each top-level
 * GLOBAL field whose type corresponds to a possible ESPS generic
 * header-item data type.  However, if "fnames" is non-NULL, it is a
 * NULL-terminated string array that specifies names of a subset of
 * the REQUIRED and GLOBAL fields to be represented in the output;
 * other fields in "list" are ignored.  If "copy_sources" is TRUE,
 * VIRTUAL fields in "list" with names of the form "source_<n>", are
 * translated into embedded source headers in the resulting FEA
 * header; otherwise the FEA header contains no embedded headers.  The
 * return value is a pointer to the ESPS header, or NULL in case of
 * error.  If "rec" is non-NULL, it gives the address of a variable in
 * which to return a pointer to an ESPS fea_data structure allocated
 * by calling "allo_fea_rec" on the returned header.  In the Esignal
 * REQUIRED field specs that correspond to FEA fields in the ESPS
 * header, the "data" members are made to point to corresponding
 * locations in the fea_data structure (found with "get_fea_ptr"), so
 * that calling ReadRecord will read a record in Esignal format and
 * store valuse from its REQUIRED fields appropriately in the fea_data
 * structure.
 */

static struct header *
FieldList_to_fea(FieldList	    list,
		 struct fea_data    **rec,
		 char		    **fnames,
		 int		    copy_sources)
{
    struct header	*hdr;
    int			i, j;
    FieldSpec		*fld;
    char		**codes;
    FieldList		subfields;
    FieldSpec		*subf;
    char		*name;
    void		*ptr;
    int			spstype;
    long		*dim;
    struct fea_header	*fea;
    struct header	*source;

    if (list == NULL)
	return NULL;

    hdr = new_header(FT_FEA);

    for (i = 0; (fld = list[i]) != NULL; i++)
    {
	codes = NULL;
	if (fld->occurrence != VIRTUAL
	    && (subfields = fld->subfields) != NULL)
	{
	    for (j = 0; (subf = subfields[j]) != NULL; j++)
	    {
		if (strcmp(subf->name, "enumStrings") != 0)
		{
		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: subfields "
				"not supported in ESPS FEA files.\n");
		}
		else if (fld->type != SHORT)
		{
		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: Non-SHORT field "
				"has subfield enumStrings.\n");
		}
		else if (subf->type != CHAR)
		{
		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: enumStrings not CHAR.\n");
		}
		else if (subf->rank != 2)
		{
		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: enumStrings "
				"not of rank 2.\n");
		}
		else
		    codes = StrArrayFromRect(subf->dim, subf->data);
	    }
	}

	if (FieldIsTag(fld) && FindStr(ESPS_TAG, fnames))
	    hdr->common.tag = TRUE;
	else if (FieldIsFeaSubtype(fld))
	    hdr->hd.fea->fea_type = *(short *) fld->data;
	else
	{
	    name = fld->name;

	    switch(fld->occurrence)
	    {
	    case GLOBAL:
		{
		    int     size = (int) FieldLength(fld);

		    ptr = fld->data;

		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: "
				"global field[%d]: \"%s\".\n",
				i, name);

		    if (fld->rank > 1)
		    {
			if (DebugMsgLevel)
			    fprintf(stderr,
				    "FieldList_to_fea: rank %d globals "
				    "not supported in ESPS FEA files.\n",
				    fld->rank);
		    }

		    if (size == 0)
		    {
			if (DebugMsgLevel)
			    fprintf(stderr,
				    "FieldList_to_fea: empty globals "
				    "not supported in ESPS FEA files.\n");
		    }
		    else if (codes != NULL)
			(void) add_genhd_e(name,
					   (short *) ptr, size, codes, hdr);
		    else
			switch (fld->type)
			{
			case DOUBLE:
			    (void) add_genhd_d(name,
					       (double *) ptr, size, hdr);
			    break;
			case FLOAT:
			    (void) add_genhd_f(name,
					       (float *) ptr, size, hdr);
			    break;
			case LONG:
			    (void) add_genhd_l(name,
					       (long *) ptr, size, hdr);
			    break;
			case SHORT:
			    (void) add_genhd_s(name,
					       (short *) ptr, size, hdr);
			    break;
			case SCHAR:
			case UCHAR:
			case CHAR:
			    (void) add_genhd_c(name,
					       (char *) ptr, size, hdr);
			    break;
			default:
			    if (DebugMsgLevel)
				fprintf(stderr,
					"FieldList_to_fea: global type %d "
					"not supported in ESPS FEA files.\n",
					fld->type);
			}
		}
		break;
	    case REQUIRED:
		{
		    long    size = FieldLength(fld);

		    if (DebugMsgLevel)
			fprintf(stderr,
				"FieldList_to_fea: "
				"required field[%d]: \"%s\".\n",
				i, name);

		    if (FindStr(name, fnames))
		    {
			spstype = (codes != NULL) ? CODED
			    : ElibTypeToEsps(fld->type);
			if (spstype != UNDEF)
			{
			    dim = (long *) malloc(fld->rank * sizeof(long));
			    for (j = 0; j < fld->rank; j++)
				dim[j] = fld->dim[j];
			    add_fea_fld(name, size, fld->rank,
					dim, spstype, codes, hdr);
			}
		    }
		}
		break;
	    case OPTIONAL:
		if (DebugMsgLevel)
		    fprintf(stderr,
			    "FieldList_to_fea: optional fields "
			    "not supported in ESPS FEA files.\n");
		break;
	    case VIRTUAL:
		if (copy_sources)
		{
		    if (strncmp(name, "source_", 7) != 0)
		    {
			if (DebugMsgLevel)
			    fprintf(stderr, "Field_List_to_fea: VIRTUAL "
				    "field other than source_<n>.\n");
		    }
		    else if ((subfields = fld->subfields) != NULL
			     || fld->type == CHAR)
		    {
			size_t	len;
			char	*data;

			source = FieldList_to_fea(subfields,
						  NULL, NULL, TRUE);
			len = FieldLength(fld);
			data = (char *) malloc(len + 1);
			strncpy(data, fld->data, len);
			data[len] = '\0';

			add_source_file(hdr, data, source);
		    }
		}
		break;
	    case INCLUDED:
		if (DebugMsgLevel)
		    fprintf(stderr,
			    "FieldList_to_fea: included fields "
			    "not supported in ESPS FEA files.\n");
		break;
	    default:
		spsassert(0, "FieldList_to_fea: "
			  "unrecognized occurrence code.\n");
		break;
	    }
	}
    }

    if (rec != NULL)
    {
	*rec = allo_fea_rec(hdr);
	fea = hdr->hd.fea;

	if (hdr->common.tag)
	{
	    fld = FindField(list, ESPS_TAG);
	    fld->data = &(*rec)->tag;
	}

	for (i = 0; i < (int) fea->field_count; i++)
	{
	    name = fea->names[i];
	    fld = FindField(list, name);
	    fld->data = get_fea_ptr(*rec, name, hdr);
	}
    }

    return hdr;
}


/*!*/
/* Same function FindStr defined in fea2esig.c as well as
 * here and esig2fea.c.
 */

/*
 * If "arr" is non-NULL, it should point to the beginning of a
 * NULL-terminated string array; search for the string "str" in the
 * array, return TRUE if it is found and FALSE otherwise.  If "arr"
 * is NULL, just return TRUE.  (NULL is not treated as an empty string
 * array; it is an indicator that all strings are to be accepted,
 */

static int
FindStr(char	*str,
	char	**arr)
{
    int     i;

    spsassert(str != NULL, "FindStr: NULL string.");

    if (arr == NULL)
	return TRUE;		/* NULL array implies all fields */

    for (i = 0; arr[i] != NULL; i++)
	if (strcmp(str, arr[i]) == 0)
	    return TRUE;

    return FALSE;
}


/*
 * Convert a rectangular array of characters into a NULL-terminated
 * string array in which each string holds a copy of the contents of a
 * corresponding row of the character array (up to the first null
 * character---the rows are assumed to be null-padded on the right).
 * The beginning of the rectangular array is indicated by "data", and
 * its dimensions are given by dim[0] and dim[1].  The return value is
 * a pointer to the beginning of the string array.
 */

static char **
StrArrayFromRect(long	*dim,
		 void	*data)
{
    char	**strarr;
    long	len, wid, i;
    char	*row;

    len = 1 + dim[0];

    strarr = (char **) malloc(len * sizeof(char *));
    len -= 1;

    wid = dim[1];
    row = data;
    for (i = 0; i < len; i++)
    {
/*!*//* Should check for null termination of rows. */
	strarr[i] = savestring(row);
	row += wid;
    }

    strarr[len] = NULL;

    return strarr;
}


/*
 * Return TRUE if "fld" points to an Esignal field specification for a
 * LONG, scalar, REQUIRED field named "Tag"; return FALSE otherwise.
 * (Such a field in an Esignal file represents the ESPS tag in a FEA
 * file, not an ordinary FEA field named "Tag".)
 */

static int
FieldIsTag(FieldSpec *fld)
{
    if (strcmp(fld->name, ESPS_TAG) != 0)
	return FALSE;
    if (fld->occurrence != REQUIRED)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsTag: non-REQUIRED field named \"%s\".\n",
		    ESPS_TAG);
	return FALSE;
    }
    if (fld->type != LONG)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsTag: type of field named \"%s\" not LONG.\n",
		    ESPS_TAG);
	return FALSE;
    }
    if (fld->rank != 0)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsTag: rank of field named \"%s\" not 0.\n",
		    ESPS_TAG);
	return FALSE;
    }
    return TRUE;
}


/*
 * Return TRUE if "fld" points to an Esignal field specification for a
 * SHORT, scalar, GLOBAL field named "FeaSubtype"; return FALSE
 * otherwise.  (Such a occurs field in an Esignal file that results from
 * converting an ESPS FEA file that belongs to one of the special FEA
 * subtypes (FEA_SD, FEA_SPEC, etc.)  For the transformation to be
 * reversible, the field should give a value to hd.fea->fea_type in the
 * FEA header instead of being translated into a generic header item
 * named "FeaSubtype".
 */

static int
FieldIsFeaSubtype(FieldSpec *fld)
{
    if (strcmp(fld->name, ESPS_FEA_SUBTYPE) != 0)
	return FALSE;
    if (fld->occurrence != GLOBAL)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: non-GLOBAL field named \"%s\".\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->type != SHORT)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: "
		    "type of field named \"%s\" not SHORT.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->rank != 0)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: rank of field named \"%s\" not 0.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->data == NULL)
    {
	if (DebugMsgLevel)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: "
		    "field named \"%s\" has NULL data pointer.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    return TRUE;
}


/*
 * If "type" is an Esignal data-type code, return the corresponding
 * ESPS data-type code, or UNDEF if no such ESPS data type exists.
 */

static int
ElibTypeToEsps(int type)
{
    switch (type)
    {
    case DOUBLE:
	return FDOUBLE;
    case FLOAT:
	return FFLOAT;
    case LONG:
	return FLONG;
    case SHORT:
	return FSHORT;
    case SCHAR:
	return BYTE;
    case UCHAR:
    case CHAR:
	return FCHAR;
    case DOUBLE_COMPLEX:
	return DOUBLE_CPLX;
    case FLOAT_COMPLEX:
	return FLOAT_CPLX;
    case LONG_COMPLEX:
	return LONG_CPLX;
    case SHORT_COMPLEX:
	return SHORT_CPLX;
    case SCHAR_COMPLEX:
	return BYTE_CPLX;
    default:
	return UNDEF;
    }
}
