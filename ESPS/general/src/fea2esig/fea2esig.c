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
 * Write FEA file data to Esignal file.
 *
 */

static char *sccs_id = "@(#)fea2esig.c	1.4	10/4/97	ERL";

#include <esignal.h>
/* NAME CONFLICT:  THe Esignal data-type codes named
 * DOUBLE, FLOAT, LONG, SHORT, and CHAR
 * are different from the ESPS data-type codes with the same names,
 * and the Esignal type code BOOL has the same name as an ESPS macro.
 * While the Esignal definitions are in effect, declare variables so
 * that the Esignal codes can be referred to as
 * EDOUBLE, EFLOAT, ELONG, ESHORT, ECHAR, and EBOOL
 * instead.  After the inclusion of <esps/esps.h> below,
 * the ESPS definitions of 
 * DOUBLE, FLOAT, LONG, SHORT, CHAR, and BOOL
 * take effect.
 */
const static int EDOUBLE = DOUBLE;
#undef	DOUBLE
const static int EFLOAT = FLOAT;
#undef	FLOAT
const static int ELONG = LONG;
#undef	LONG
const static int ESHORT = SHORT;
#undef	SHORT
const static int ECHAR = CHAR;
#undef	CHAR
const static int EBOOL = BOOL;
#undef BOOL
#include <esps/esps.h>
#include <esps/fea.h>

#define ESPS_TAG "Tag"

#define REQUIRE(test, text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}

#define SYNTAX \
USAGE("fea2esig [-a format] [-f field]... [-r range] [-x debug_level]\n\t[-A width] [-F] input.fea output.esig");

void	lrange_switch();
char	*savestring();
void	skiprec();
void	fea_skiprec();
int	size_rec();


static FieldList    fea_to_FieldList(struct header *hd, struct fea_data *rec,
				     char **fnames, int copy_sources);
static int	    FindStr(char *str, char **arr);
static int	    EspsTypeToElib(int spstype);
static void	    StrArrToRect(char **strarr,
				 long **dimenp, void **datap);
static FieldSpec    *AddGlobalField(FieldList *list, char *name,
				    int rank, long *dim, int type, void *ptr);
static FieldSpec    *AddSource(FieldList *list,
			      int srcnum, char *name, FieldList source);
static FieldSpec    *AddCommandLine(FieldList *list, char *line);
static char	    *GetCommandLine(int argc, char **argv);


int	    debug_level = 0;
char	    *ProgName = "fea2esig";

main(int   argc,
     char  **argv)
{
    extern int	    optind;	/* for use of getopt() */
    extern char	    *optarg;	/* for use of getopt() */
    int		    ch;		/* command-line option letter */

    int		    outarch = NATIVE;

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

    int		    Aflag = NO;	/* annotate Ascii output? */
    Annot	    *annotate = NULL;
    int		    annwidth = 70;

    char	    *iname;	/* input file name */
    FILE	    *ifile;	/* input stream */
    struct header   *ihd;	/* input file header */
    struct fea_data *irec;	/* input record */

    char	    *oname;	/* output file name */
    FILE	    *ofile;	/* output stream */
    FieldList	    list;	/* output field list */
    int		    outord = TYPE_ORDER;
    FieldSpec	    **ofields;	/* output fields in field or type order */

    double	    rec_freq;
    double	    start_time_offset;
    char	    *fdata;
    double	    *edata;
    int		    type;
    int		    len, i;
    long	    dim[1];
    FieldSpec	    *field;

    FieldList	    source;	/* field list of source file */

    while ((ch = getopt(argc, argv, "a:f:r:x:A:F")) != EOF)
	switch (ch)
	{
	case 'a':
	    outarch = ((!strcmp(optarg, "EDR1")) ? EDR1
		       : (!strcmp(optarg, "EDR2")) ? EDR2
		       : (!strcmp(optarg, "NATIVE")) ? NATIVE
		       : (!strcmp(optarg, "ASCII")) ? ASCII
		       : UNKNOWN);
	    break;
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
	case 'A':
	    Aflag = YES;
	    annwidth = atoi(optarg);
	    break;
	case 'F':
	    outord = FIELD_ORDER;
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

    iname = eopen(ProgName,
		  argv[optind++], "r", FT_FEA, NONE, &ihd, &ifile);

    oname = argv[optind++];

    start_rec = 1;
    end_rec = LONG_MAX;
    num_recs = 0;

    if (rflag)
    {
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	if (end_rec != LONG_MAX)
	    num_recs = end_rec - start_rec + 1;
    }

    REQUIRE(start_rec >= 1, "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec, "empty range of records specified");

    if (debug_level)
	fprintf(stderr,
		"start_rec: %ld.  end_rec: %ld.  num_recs: %ld.\n",
		start_rec, end_rec, num_recs);

    DebugMsgLevel = debug_level;
    DebugMsgFunc = DebugPrint;

    irec = allo_fea_rec(ihd);

    REQUIRE(irec != NULL, "can't allocate memory for input record");

    list = fea_to_FieldList(ihd, irec, field_names, FALSE);

    REQUIRE(list != NULL,
	    "failure converting input header to field list");

    switch (outord)
    {
    case TYPE_ORDER:
	if (debug_level)
	    fprintf(stderr, "making type-ordered field array.\n");
	ofields = TypeOrder(list);
	break;
    case FIELD_ORDER:
	if (debug_level)
	    fprintf(stderr, "making field-ordered field array.\n");
	ofields = FieldOrder(list);
	break;
    default:
	REQUIRE(0, "output order neither TYPE_ORDER nor FIELD_ORDER");
	break;
    }

    if (debug_level)
	fprintf(stderr, "setting field ordering.\n");

    REQUIRE(SetFieldOrdering(&list, outord),
	    "can't set field ordering of output");

    rec_freq = get_genhd_val("record_freq", ihd, 1.0);

    type = genhd_type("record_freq", &len, ihd);

    if (type != HD_UNDEF)
    {
	field = AddGlobalField(&list, "recordFreq", 0, NULL, EDOUBLE, NULL);
	*(double *) field->data = rec_freq;
    }

    type = genhd_type("start_time", &len, ihd);

    if (type != HD_UNDEF && rec_freq != 0)
    {
	start_time_offset = (start_rec - 1) / rec_freq;

	fdata = (char *) get_genhd("start_time", ihd);
	edata = (double *) type_convert((long) len, fdata, type,
					(char *) NULL, DOUBLE,
					(void (*)()) NULL);

	if (start_time_offset != 0)
	{
	    for (i = 0; i < len; i++)
		edata[i] += start_time_offset;
	}

	dim[0] = len;
	AddGlobalField(&list, "startTime", 1, dim, EDOUBLE, edata);
    }

    source = fea_to_FieldList(ihd, NULL, NULL, TRUE);
    (void) AddSource(&list, 0, iname, source);

    (void) AddCommandLine(&list, GetCommandLine(argc, argv));

    if (debug_level)
	fprintf(stderr, "annwidth %d.\n", annwidth);

    if (Aflag)
    {
	annotate = (Annot *) malloc(sizeof(Annot));
	annotate->position = 0;
	annotate->indent = 0;
	annotate->width = annwidth;
	annotate->recnum = 0;
    }

    if (debug_level)
	fprintf(stderr, "writing Esignal header.\n");

    REQUIRE(OpenOut(oname, list, outarch, &ofile, annotate),
	    "write header failed");
    if (ofile == stdout)
	oname = "<stdout>";

    num_read = start_rec - 1;

    if (debug_level)
	fprintf(stderr, "skipping %ld records.\n", num_read);

/*    skiprec(ifile, num_read, size_rec(ihd)); */
    fea_skiprec(ifile, num_read, ihd);

    while (num_read++ < end_rec && get_fea_rec(irec, ihd, ifile) != EOF)
    {
	if (debug_level > 2)
	    fprintf(stderr, "Record number %ld read.\n", num_read);

	WriteRecord(ofields, outarch, ofile, annotate);
    }

    if (--num_read < end_rec && num_recs != 0)
	fprintf(stderr, "fea2esig: only %ld records read.\n",
		num_read - (start_rec - 1));

    exit(0);
    /*NOTREACHED*/
}


static FieldList
fea_to_FieldList(struct header	    *hdr,
		 struct fea_data    *rec,
		 char		    **field_names,
		 int		    copy_sources)
{
    struct fea_header	*fea;
    FieldList		list;
    int			i, j;
    FieldSpec		*field, *subfield;
    char		*name;
    int			spstype;
    long		*dim;
    int			rank;
    int			type;
    char		**codes;
    char		**gnames;
    int			gnum;
    struct varsize	*var;
    int			nsrc;
    FieldList		source;
    char		*line;
    long		len;


    if (hdr == NULL)
	return NULL;

    if (hdr->common.type != FT_FEA)
	return NULL;

    fea = hdr->hd.fea;

    list = NULL;

    if (hdr->common.tag && FindStr(ESPS_TAG, field_names))
    {
	field = NewFieldSpec(ELONG, 0);
	field->name = savestring(ESPS_TAG);
	field->occurrence = REQUIRED;
	if (rec != NULL)
	    field->data = &rec->tag;
	AddField(&list, field);
    }

    if (debug_level)
	fprintf(stderr, "fea_to_FieldList: field count %d.\n",
		fea->field_count);

    for (i = 0; i < (int) fea->field_count; i++)
    {
	long	size;

	name = fea->names[i];

	if (FindStr(name, field_names))
	{
	    spstype = fea->types[i];
	    size = fea->sizes[i];
	    rank = fea->ranks[i];
	    dim = fea->dimens[i];

	    if (debug_level >= 2)
		fprintf(stderr,
			"fea_to_FieldList: field[%d]: \"%s\".\n",
			i, name);

	    type = EspsTypeToElib(spstype);
	    field = NewFieldSpec(type, rank);
	    if (rank == 1)
		field->dim[0] = size;
	    else
		for (j = 0; j < rank; j++)
		    field->dim[j] = dim[j];
	    field->name = savestring(name);
	    field->occurrence = REQUIRED;
	    if (spstype == CODED)
	    {
		codes = fea->enums[i];
		subfield = NewFieldSpec(ECHAR, 2);
		subfield->name = savestring("enumStrings");
		subfield->occurrence = GLOBAL;
		StrArrToRect(codes, &subfield->dim, &subfield->data);
		AddSubfield(field, subfield);
	    }
	    if (rec != NULL)
		field->data = get_fea_ptr(rec, name, hdr);
	    AddField(&list, field);
	}
    }

    gnames = genhd_list(&gnum, hdr);

    for (i = 0; i < gnum; i++)
    {
	int	size;

	name = gnames[i];
	spstype = genhd_type(name, &size, hdr);

	if (debug_level >= 2)
	    fprintf(stderr,
		    "fea_to_FieldList: global field[%d]: \"%s\".\n",
		    i, name);

	type = EspsTypeToElib(spstype);
	field = NewFieldSpec(type, 1);
	field->dim[0] = size;
	field->name = savestring(name);
	field->occurrence = GLOBAL;
	if (spstype == CODED)
	{
	    codes = genhd_codes(name, hdr);
	    subfield = NewFieldSpec(ECHAR, 2);
	    subfield->name = savestring("enumStrings");
	    subfield->occurrence = GLOBAL;
	    StrArrToRect(codes, &subfield->dim, &subfield->data);
	    AddSubfield(field, subfield);
	}
	field->data = get_genhd(name, hdr);
	AddField(&list, field);
    }

    if (fea->fea_type != NONE)
    {
	field = AddGlobalField(&list, "FeaSubtype",
			       0, NULL, ESHORT, NULL);
	*(short *) field->data = fea->fea_type;
	subfield = NewFieldSpec(ECHAR, 2);
	subfield->name = savestring("enumStrings");
	subfield->occurrence = GLOBAL;
	StrArrToRect(fea_file_type, &subfield->dim, &subfield->data);
	AddSubfield(field, subfield);
    }

    var = &hdr->variable;

    if (copy_sources)
    {
	nsrc = MAX(var->nnames, var->nheads);
	for (i = 0; i < nsrc; i++)
	{
	    source = (i >= var->nheads) ? NULL
		: fea_to_FieldList(var->srchead[i], NULL, NULL, TRUE);
	    name = (i >= var->nnames) ? NULL
		: var->source[i];
	    (void) AddSource(&list, i, name, source);
	}
    }

    if (var->comment != NULL)
    {
	line = savestring(var->comment);
	len = strlen(line);
	if (line[len-1] == '\n')
	    line[len-1] = '\0';
	(void) AddCommandLine(&list, line);
    }

    return list;
}


static int
FindStr(char	*str,
	char	**arr)
{
    int     i;

    if (str == NULL)
	return FALSE;

    if (arr == NULL)
	return TRUE;		/* NULL array implies all fields */

    for (i = 0; arr[i] != NULL; i++)
	if (strcmp(str, arr[i]) == 0)
	    return TRUE;

    return FALSE;
}


/*!*//* NAME CONFLICT BETWEEN ESPS AND ESIGNAL PUBLIC DATA TYPES */

static int
EspsTypeToElib(int spstype)
{
    switch (spstype)
    {
    case DOUBLE:
	return EDOUBLE;
    case FLOAT:
	return EFLOAT;
    case LONG:
	return ELONG;
    case SHORT:
    case CODED:
	return ESHORT;
    case BYTE:
	return SCHAR;
    case CHAR:
    case AFILE:
    case EFILE:
	return ECHAR;
    case DOUBLE_CPLX:
	return DOUBLE_COMPLEX;
    case FLOAT_CPLX:
	return FLOAT_COMPLEX;
    case LONG_CPLX:
	return LONG_COMPLEX;
    case SHORT_CPLX:
	return SHORT_COMPLEX;
    case BYTE_CPLX:
	return SCHAR_COMPLEX;
    default:
	return NO_TYPE;
    }
}


static void
StrArrToRect(char  **strarr,
	     long  **dimenp,
	     void  **datap)
{
    long	len, slen, wid, *dim, siz;
    void	*data;
    long	i, j;
    char	*str, *row;

    wid = 0;
    if (strarr == NULL)
	len = 0;
    else
	for (len = 0; strarr[len] != NULL; len++)
	{
	    if ((slen = strlen(strarr[len])) > wid)
		wid = slen;
	}
    wid += 1;

    dim = (long *) malloc(2*sizeof(long));
    dim[0] = len;
    dim[1] = wid;

    siz = len * wid;
    data = malloc(siz*sizeof(char));
    row = data;
    for (i = 0; i < len; i++)
    {
	str = strarr[i];
	for (j = 0; str[j] != '\0'; j++)
	    row[j] = str[j];
	for ( ; j < wid; j++)
	    row[j] = '\0';
	row += wid;
    }

    if (dimenp)
	*dimenp = dim;
    if (datap)
	*datap = data;
}


/* MOVE TO ESIGNAL PUBLIC I/O LIBRARY */


static FieldSpec *
AddGlobalField(FieldList    *list,
	       char	    *name,
	       int	    rank,
	       long	    *dim,
	       int	    type,
	       void	    *ptr)
{
    FieldSpec	*field;
    int		i;
    long	len;
    size_t	size;
    

    if (name == NULL)
	return NULL;

    if (rank > 0 && dim == NULL)
    {
	DebugMsg(1, "AddGlobalField: NULL dimensions but rank > 0.");
	return NULL;
    }

    field = FindField(*list, name);

    if (field == NULL)
    {
	field = NewFieldSpec(type, rank);

	if (field == NULL)
	{
	    DebugMsg(1, "AddGlobalField: Couldn't create field spec.");
	    return NULL;
	}

	field->name = savestring(name);
	field->occurrence = GLOBAL;

	if (!AddField(list, field))
	{
	    DebugMsg(1, "AddGlobalField: Couldn't add field spec.");
	    return NULL;
	}
    }
    else			/* field != NULL */
    {
	if (field->occurrence != GLOBAL)
	{
	    DebugMsg(1, "AddGlobalField: non-GLOBAL field with same name.");
	    return NULL;
	}

	field->type = type;

	if (field->rank != rank)
	{
	    if (rank == 0)
	    {
		free(field->dim);
		field->dim = NULL;
	    }
	    else
	    {
		field->dim =
		    (long *) ((field->dim == NULL)
			      ? malloc(rank * sizeof(long))
			      : realloc(field->dim, rank * sizeof(long)));

		if (field->dim == NULL)
		{
		    DebugMsg(1,
			     "AddGlobalField: couldn't allocate dimensions.");
		    return NULL;
		}
	    }

	    field->rank = rank;
	}
    }

    len = 1;
    for (i = 0; i < rank; i++)
    {
	field->dim[i] = dim[i];
	len *= dim[i];
    }

    size = len * InternTypeSize(type);

    if (size == 0)
	field->data = NULL;
    else if (ptr != NULL)
	field->data = ptr;
    else
    {
	field->data = malloc(size);

	if (field->data == NULL)
	{
	    DebugMsg(1, "AddGlobalField: couldn't allocate data storage.");
	    return NULL;
	}
    }

    return field;
}


static FieldSpec *
AddSource(FieldList *list,
	  int	    srcnum,
	  char	    *name,
	  FieldList source)
{
    FieldSpec	*field;
    char	srcname[30];
    int		type;
    int		rank;

    if (name == NULL && source == NULL)
	return NULL;

    sprintf(srcname, "source_%d", srcnum);

    if (name == NULL)
    {
	type = NO_TYPE;
	rank = 0;
    }
    else
    {
	type = ECHAR;
	rank = 1;
    }

    field = FindField(*list, srcname);

    if (field == NULL)
    {
	field = NewFieldSpec(type, rank);
	if (field == NULL)
	{
	    DebugMsg(1, "AddSource: Couldn't create field spec.");
	    return NULL;
	}

	field->name = savestring(srcname);
	field->occurrence = VIRTUAL;

	if (!AddField(list, field))
	{
	    DebugMsg(1, "AddSource: Couldn't add field spec.");
	    return NULL;
	}
    }
    else			/* field != NULL */
    {
	if (field->occurrence != VIRTUAL)
	{
	    DebugMsg(1, "AddSource: non-VIRTUAL source_<n> field.");
	    return NULL;
	}

	field->type = type;
	field->rank = rank;

	if (field->dim != NULL)
	{
	    free(field->dim);
	    field->dim = NULL;
	}

	if (name != NULL)
	    field->dim = (long *) malloc(sizeof(long));
    }

    field->subfields = source;

    if (name != NULL)
    {
	field->dim[0] = strlen(name);
	field->data = name;
    }

    return field;
}


static FieldSpec *
AddCommandLine(FieldList    *list,
	       char	    *line)
{
    FieldSpec	*field;


    if (line == NULL)
	return NULL;

    field = FindField(*list, "commandLine");

    if (field == NULL)
    {
	field = NewFieldSpec(ECHAR, 1);
	if (field == NULL)
	{
	    DebugMsg(1, "AddCommandLine: Couldn't create field spec.");
	    return NULL;
	}

	field->dim[0] = strlen(line);
	field->name = savestring("commandLine");
	field->occurrence = GLOBAL;

	if (!AddField(list, field))
	{
	    DebugMsg(1, "AddCommandLine: Couldn't add field spec.");
	    return NULL;
	}
    }
    else			/* field != NULL */
    {
	if (field->occurrence != GLOBAL)
	{
	    DebugMsg(1, "AddCommandLine: non-GLOBAL field \'commandLine\".");
	    return NULL;
	}

	field->type = ECHAR;
	field->rank = 1;
	field->dim = (long *) ((field->dim == NULL)
				? malloc(sizeof(long))
				: realloc(field->dim, sizeof(long)));

	if (field->dim == NULL)
	{
	    DebugMsg(1, "AddCommandLine: couldn't (re)allocate dimension.");
	    return NULL;
	}

	field->dim[0] = 1 + strlen(line);
    }

    field->data = line;

    return field;
}


static char *
GetCommandLine(int  argc,
	       char **argv)
{
    long    len;
    int	    i;
    char    *line;
    char    *arg;


    len = 0;
    for (i = 0; i < argc; i++)
	if (argv[i] != NULL)
	    len += strlen(argv[i]);

    len += 3*argc;		/* Allow for quotes around each item,
				 * blanks between, and null at end. */

    line = (char *) malloc(len * sizeof(char));

    if (line == NULL)
    {
	DebugMsg(1, "GetCommandLine: couldn't allocate string.");
	return NULL;
    }

    line[0] = '\0';

    for (i = 0; i < argc; i++)
	if ((arg = argv[i]) != NULL)
	{
	    if (strpbrk(arg, " \t\n?*$~[];&()|^<>\\\'\"") == NULL)
	    {
		(void) strcat(line, arg);
		(void) strcat(line, " ");
	    }
	    else
	    {
		/* Quote args that contain whitespace or shell special
		 * characters.
		 * (This doesn't handle arguments correctly if they contain
		 * right single quotes (') or, under some shells, newlines).
		 */

		(void) strcat(line, "\'");
		(void) strcat(line, arg);
		(void) strcat(line, "\' ");
	    }
	}

    if (i > 0)
	line[strlen(line) - 1] = '\0'; /* Kill final blank. */

    return line;
}
