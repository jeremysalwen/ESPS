/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Write Esignal file data to FEA file.
 *
 */

static char *sccs_id = "@(#)esig2fea.c	1.4	2/5/97	ERL";

#include <esps/esps.h>
#include <esps/fea.h>
/* NAME CONFLICT:  The ESPS data-type codes named
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
const static int FDOUBLE = DOUBLE;
#undef	DOUBLE
const static int FFLOAT = FLOAT;
#undef	FLOAT
const static int FLONG = LONG;
#undef	LONG
const static int FSHORT = SHORT;
#undef	SHORT
const static int FCHAR = CHAR;
#undef	CHAR
#undef	BOOL
#include <esignal.h>

#define SCCS_VERSION	    "1.4"
#define SCCS_DATE	    "2/5/97"

#define ESPS_TAG	    "Tag"
#define ESPS_FEA_SUBTYPE    "FeaSubtype"

#define REQUIRE(test, text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("esig2fea [-f field]... [-r range] [-x debug_level] [-F] [-T subtype]\n\tinput.esig output.fea");

void	lrange_switch();
char	*savestring();
char	*get_cmd_line();


static struct header	*FieldList_to_fea(FieldList list,
					  struct fea_data **rec,
					  char **fnames, int copy_sources);
static int		FindStr(char *str, char **arr);
static char		**StrArrayFromRect(long *dim, void *data);
static int		FieldIsTag(FieldSpec *fld);
static int		FieldIsFeaSubtype(FieldSpec *fld);
static long		SkipRecs(FILE *file, long skip,
				 long size, FieldSpec **fields, int arch);
static int		ElibTypeToEsps(int type);


int	    debug_level = 0;


main(int   argc,
     char  **argv)
{
    extern int	    optind;	/* for use of getopt() */
    extern char	    *optarg;	/* for use of getopt() */
    int		    ch;		/* command-line option letter */

    static char	    *ProgName = "esig2fea";	/* name of this program */
    static char	    *Version = SCCS_VERSION;	/* program SCCS version */
    static char	    *Date = SCCS_DATE;		/* program SCCS date */

    char	    **field_names = NULL;
    int		    num_fields = 0;
    int		    alloc_fields = 0;

    int		    rflag = NO;	/* -r option specified? */
    char	    *rrange;	/* arguments of -r option */
    long	    start_rec;	/* starting record number */
    long	    end_rec;	/* ending record number */
    long	    num_recs;	/* number of records to read
				   (0 means all up to end of file) */
    long	    num_read;	/* number of records actually read */

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    FieldList	    list;	/* input field list */
    int		    inord;	/* input: field order or type order? */
    FieldSpec	    **ifields;	/* input fields in field or type order */

    char	    *subtype = NULL;		/* FEA subtype name */
    int		    subtype_code = NONE;	/* numeric subtype code */
    FieldSpec	    *fld;	/* spec of various special fields */

    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    struct header   *ohd;	/* output file header */
    struct fea_data *orec;	/* output record */
    int		    outord = TYPE_ORDER;

    char	    *version;	/* version from input preamble */
    int		    arch;	/* architecture from input preamble */
    long	    pre_size;	/* preamble size */
    long	    hdr_size;	/* header size (bytes) from preamble */
    long	    rec_size;	/* record size from preamble */

    double	    rec_freq;
    double	    start_time_offset;
    double	    *data;
    long	    len, i;


    struct header   *source;	/* embedded source-file header */


    while ((ch = getopt(argc, argv, "f:r:x:FT:")) != EOF)
	switch (ch)
	{
	case 'f':
	    if (num_fields >= alloc_fields)
	    {
		size_t	size;

		alloc_fields = num_fields + 1 + num_fields/2;
		size = (alloc_fields + 1) * sizeof(char *);
		field_names = (char **)
		    ((field_names == NULL)
		     ? malloc(size)
		     : realloc(field_names, size));
	    }
	    field_names[num_fields++] = optarg;
	    field_names[num_fields] = NULL;
	    break;
	case 'r':
	    rflag = YES;
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'F':
	    outord = FIELD_ORDER;
	    break;
	case 'T':
	    subtype = optarg;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    if (argc - optind > 2)
    {
	fprintf(stderr,
		"%s: too many file names specified.\n", ProgName);
	SYNTAX;
    }
    if (argc - optind < 2)
    {
	fprintf(stderr,
		"%s: too few file names specified.\n", ProgName);
	SYNTAX;
    }


    DebugMsgLevel = debug_level;
    DebugMsgFunc = DebugPrint;

    iname = argv[optind++];
    list = OpenIn(iname, &version,
		  &arch, &pre_size, &hdr_size, &rec_size, &ifile);
    REQUIRE(list != NULL, "read header failed");
    if (ifile == stdin)
	iname = "<stdin>";

    oname = argv[optind++];

    start_rec = 0;
    end_rec = LONG_MAX;
    num_recs = 0;	/* 0 means continue to end of file */

    if (rflag)
    {
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	if (end_rec != LONG_MAX)
	    num_recs = end_rec - start_rec + 1;
    }

    REQUIRE(start_rec >= 0, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    if (debug_level)
	fprintf(stderr,
		"start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    REQUIRE(GetFieldOrdering(list, &inord),
	    "cant get field ordering of input");

    switch (inord)
    {
    case TYPE_ORDER:
	if (debug_level)
	    fprintf(stderr, "making type-ordered field array.\n");
	ifields = TypeOrder(list);
	break;
    case FIELD_ORDER:
	if (debug_level)
	    fprintf(stderr, "making field-ordered field array.\n");
	ifields = FieldOrder(list);
	break;
    default:
	REQUIRE(0, "input order neither TYPE_ORDER nor FIELD_ORDER");
	break;
    }

    ohd = FieldList_to_fea(list, &orec, field_names, FALSE);

    REQUIRE(ohd != NULL,
	    "failure converting input field list to header & record struct");

    if (subtype != NULL)
    {
	subtype_code = lin_search(fea_file_type, subtype);

	if (subtype_code == -1)
	    fprintf(stderr, "%s: unknown FEA file subtype \"%s\" ignored.\n",
		    ProgName, subtype);
	else
	    ohd->hd.fea->fea_type = subtype_code;
    }

    if (outord == FIELD_ORDER)
	ohd->hd.fea->field_order = YES;

    fld = FindField(list, "recordFreq");

    if (fld != NULL && fld->occurrence == GLOBAL && fld->data != NULL)
    {
	(void) type_convert(1L, (char *) fld->data, ElibTypeToEsps(fld->type),
			    (char *) &rec_freq, FDOUBLE, (void (*)()) NULL);

	*add_genhd_d("record_freq", NULL, 1, ohd) = rec_freq;
    }
    else
	rec_freq = 1.0;

    fld = FindField(list, "startTime");

    if (fld != NULL
	&& fld->occurrence == GLOBAL && fld->data != NULL && rec_freq != 0)
    {
	start_time_offset = start_rec / rec_freq;

	len = FieldLength(fld);
	data = (double *) type_convert(len, (char *) fld->data,
				       ElibTypeToEsps(fld->type),
				       (char *) NULL, FDOUBLE,
				       (void (*)()) NULL);

	if (start_time_offset != 0)
	{
	    for (i = 0; i < len; i++)
		data[i] += start_time_offset;
	}

	(void) add_genhd_d("start_time", data, len, ohd);
    }

    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);

    source = FieldList_to_fea(list, NULL, NULL, TRUE);
    add_source_file(ohd, savestring(iname), source);

    add_comment(ohd, get_cmd_line(argc, argv));

    oname = eopen(ProgName,
		  oname, "w", NONE, NONE, NULL, &ofile);
    write_header(ohd, ofile);

    num_read = SkipRecs(ifile, start_rec,
			RecordSize(list, arch), ifields, arch);

    if (num_read != start_rec)
    {
	fprintf(stderr,
		"%s: couldn't reach starting record; only %ld skipped.\n",
		ProgName, num_read);
	exit(0);
    }

    for ( ;
	 num_read <= end_rec && ReadRecord(ifields, arch, ifile);
	 num_read++)
    {
	put_fea_rec(orec, ohd, ofile);
    }

    if (num_read <= end_rec && num_recs != 0)
	fprintf(stderr, "esig2fea: only %ld records read.\n",
		num_read - start_rec);

    exit(0);
    /*NOTREACHED*/
}


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
		    if (debug_level)
			fprintf(stderr,
				"FieldList_to_fea: subfields "
				"not supported in ESPS FEA files.\n");
		}
		else if (fld->type != SHORT)
		{
		    if (debug_level)
			fprintf(stderr,
				"FieldList_to_fea: Non-SHORT field "
				"has subfield enumStrings.\n");
		}
		else if (subf->type != CHAR)
		{
		    if (debug_level)
			fprintf(stderr,
				"FieldList_to_fea: enumStrings not CHAR.\n");
		}
		else if (subf->rank != 2)
		{
		    if (debug_level)
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

		    if (debug_level >= 2)
			fprintf(stderr,
				"FieldList_to_fea: "
				"global field[%d]: \"%s\".\n",
				i, name);

		    if (fld->rank > 1)
		    {
			if (debug_level)
			    fprintf(stderr,
				    "FieldList_to_fea: rank %d globals "
				    "not supported in ESPS FEA files.\n",
				    fld->rank);
		    }

		    if (size == 0)
		    {
			if (debug_level)
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
			    if (debug_level)
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

		    if (debug_level >= 2)
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
		if (debug_level)
		    fprintf(stderr,
			    "FieldList_to_fea: optional fields "
			    "not supported in ESPS FEA files.\n");
		break;
	    case VIRTUAL:
		if (copy_sources)
		{
		    if (strncmp(name, "source_", 7) != 0)
		    {
			if (debug_level)
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
		if (debug_level)
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


/*!*//* Same function FindStr defined in fea2esig.c. */

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


static int
FieldIsTag(FieldSpec *fld)
{
    if (strcmp(fld->name, ESPS_TAG) != 0)
	return FALSE;
    if (fld->occurrence != REQUIRED)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsTag: non-REQUIRED field named \"%s\".\n",
		    ESPS_TAG);
	return FALSE;
    }
    if (fld->type != LONG)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsTag: type of field named \"%s\" not LONG.\n",
		    ESPS_TAG);
	return FALSE;
    }
    if (fld->rank != 0)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsTag: rank of field named \"%s\" not 0.\n",
		    ESPS_TAG);
	return FALSE;
    }
    return TRUE;
}


static int
FieldIsFeaSubtype(FieldSpec *fld)
{
    if (strcmp(fld->name, ESPS_FEA_SUBTYPE) != 0)
	return FALSE;
    if (fld->occurrence != GLOBAL)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: non-GLOBAL field named \"%s\".\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->type != SHORT)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: "
		    "type of field named \"%s\" not SHORT.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->rank != 0)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: rank of field named \"%s\" not 0.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    if (fld->data == NULL)
    {
	if (debug_level)
	    fprintf(stderr,
		    "FieldIsFeaSubtype: "
		    "field named \"%s\" has NULL data pointer.\n",
		    ESPS_FEA_SUBTYPE);
	return FALSE;
    }
    return TRUE;
}


static long
SkipRecs(FILE	    *file,
	 long	    skip,
	 long	    size,
	 FieldSpec  **fields,
	 int	    arch)
{
    long    n;

    if (skip == 0)
	return 0;

    if (size != -1)
    {
	skiprec(file, skip, size);
	return skip;
    }
    else
    {
	for (n = 0; n < skip && ReadRecord(fields, arch, file); n++)
	{ }

	return n;
    }
}


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
