/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Alan Parker
 * Checked by:  John Shore
 * Revised by:  Richard Goldhor
 *
 * Brief description: converts ascii or binary files to ESPS
 *
 */

static char *sccs_id = "@(#)addfeahd.c	1.19	3/27/97	ESI/ERL";

#define DATE "3/27/97"
#define VERSION "1.19"


#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#define SYNTAX USAGE("addfeahd [-P params] [-S skip] [-F] [-t subtype_code] [-c comment]\n\t[-C comment_file] [-E] [-a] format_file input_data output_file");
#define LINE_SIZE 256
#define COMMENT_LINE_SIZE 256

char   *get_cmd_line ();
extern int  optind;
extern char *optarg;
char *add_lf();
char *savestring();

int debug_level=0;

#define error(s,f) error2(s," ",f)
void
error2 (s, t, flag)
char   *s,
       *t;
int     flag;
{
    Fprintf (stderr, "addfeahd: %s %s\n", s, t);
    if (flag)
	SYNTAX;
    exit (1);
}

void
get_comment (file, hd)
char   *file;
struct header  *hd;
{
    FILE * strm;
    char   *buffer = malloc ((unsigned) COMMENT_LINE_SIZE);

    spsassert (buffer, "malloc failed!");
    if (strcmp ("<stdin>", file) == 0) {
	Fprintf (stderr,
	 "addfeahd: Please enter comment, ended with blank line:\n");
	strm = stdin;
    }
    else
    if ((strm = fopen (file, "r")) == NULL)
	error2 ("cannot open comment file:", file, 0);

    while (fgets (buffer, COMMENT_LINE_SIZE, strm) != NULL) {
	if (strm == stdin && strlen (buffer) == 1)
	    return;
	add_comment (hd, buffer);
    }
    free (buffer);
}

char *
add_lf(line)
char *line;			/* line to add a line feed to */
{
  char *newline = malloc((unsigned) strlen(line) + 1);
  return(savestring(strcat(strcpy(newline, line),"\n")));
}

#define	SCNINFO_MAX (1024)

typedef	struct tSINFO
	{
	char	*SI_Name;
	int	SI_Type;
	int	SI_Size;
	int	SI_Count;
	char	*SI_Skip;
	char	*SI_Scan;
	char	*SI_Ptr;
	char	*SI_Buf;
	} SINFO;

main (argc, argv)
int     argc;
char  **argv;
{

    struct header  *in_hd, *out_hd;
    FILE    *format_file, *in_file, *out_file;
    char    field_name[256],
            field_type[100];
    long    field_size;
    char    line_buff[LINE_SIZE];
    long    out_count = 0;
    struct fea_data *rec;
    short   type;
    int     c;
    char   *format_name, *input_name, *output_name;
    char   *param_file = NULL;
    char   *comment = NULL;
    char   *comment_file = NULL;
    char    combuf[100];
    int     skip = 0;
    char   *subtype = NULL;
    int	   edr_flag = 0;
    int	   aflag = 0;
    long   Status;
    int	   e, f;
    char   *Ptr;
    int	   field_cnt;
    char   skip_format[1024], element_format[1024];
    SINFO  ScnInfo[SCNINFO_MAX];
    int	   foreign_header = 0;
    long   foreign_hd_length;
    long   foreign_hd_ptr;

    while ((c = getopt (argc, argv, "x:P:S:c:C:t:EaF")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi(optarg);
		break;
	    case 'P':
		param_file = optarg;
		break;
	    case 'S':
		skip = atoi (optarg);
		break;
	    case 'c':
		comment = add_lf(optarg);
		break;
	    case 'C':
	        comment_file = optarg;
		break;
	    case 't':
		subtype = optarg;
		break;
	    case 'E':
		edr_flag = 1;
		break;
	    case 'a':
		aflag = 1;
		break;
	    case 'F':
		foreign_header++;
		break;
	    default:
		SYNTAX;
	}
    }

    if (foreign_header && !skip) {
	fprintf(stderr,"addfeahd: S option must be used if F option used.\n");
	exit(1);
    }

    if (skip && aflag) {
	fprintf(stderr,"addfeahd: makes no sense to skip on an ascii file.\n");
	exit(1);
    }

    if ((argc - optind) < 3)
	error ("at least three files must be given", 1);

    if (comment != NULL && comment_file != NULL)
	error ("cannot give both -C and -c options", 1);

    if (!(skip && (comment || comment_file))) {
/* need to read parameter file */
	(void) read_params (param_file, SC_NOCOMMON, NULL);
	if (!skip && (symtype ("skip") == ST_INT))
	    skip = getsym_i ("skip");
	if (!subtype && (symtype ("subtype") == ST_STRING))
	    subtype = getsym_s ("subtype");
	if (!(comment || comment_file) && symtype ("comment") ==
		ST_STRING)
	    comment = getsym_s ("comment");
    }
	
/* get three filenames */

    format_name = argv[optind];
    input_name = argv[optind + 1];
    output_name = argv[optind + 2];

    if (strcmp (format_name, "-") == 0)
	format_file = stdin;
    else
	TRYOPEN (argv[0], format_name, "r", format_file);

    if (strcmp (input_name, "-") == 0) {
	if (format_file == stdin)
	    error("Two input files cannot be stdin.",0);
	in_file = stdin;
    }
    else
	TRYOPEN (argv[0], input_name, "r", in_file);

    if (strcmp (output_name, "-") == 0)
	out_file = stdout;
    else
	TRYOPEN (argv[0], output_name, "w", out_file);

    out_hd = new_header (FT_FEA);

    (void) sprintf (combuf,
		    "Headerless file %s converted to ESPS FEA.\n", input_name);

/*
 the scratch header used for input will be in field_order.  It will
 usually be in native format, but that can be changed by the -E flag
*/
    in_hd = new_header (FT_FEA);
    in_hd->hd.fea->field_order = YES;
    in_hd->common.edr = edr_flag;
    in_hd->common.machine_code = MACH_CODE;

    if (subtype)
       if ((type = lin_search(fea_file_type, subtype)) == -1)
	  Fprintf(stderr,"addfeahd: unknown FEA file subtype %s ignored.\n",
		subtype);
       else
	  out_hd->hd.fea->fea_type = type;

/* read the lines from the format file
*/
    field_cnt = 0;
    while (fgets (line_buff, LINE_SIZE, format_file) != NULL) {
	if (aflag)
		{
		if ((Status = sscanf (line_buff, "%s %s %ld %s %s",
		    field_name, field_type, &field_size,
			    skip_format, element_format)) == 0)
			break;
		if (Status == 4)
			{
			strcpy(element_format, skip_format);
			skip_format[0] = '\0';
			}
		/*
		fprintf(stderr,
			"field=%s; type=%s; count=%d; skip=%s; scan=%s\n",
			field_name, field_type, field_size,
			skip_format, element_format);
		*/
		}
	else	{
		if (sscanf (line_buff, "%s %s %ld",
		    field_name, field_type, &field_size) == 0)
			break;
		}


/* do some simple validation of input
*/
	if (field_size <= 0) {
	    Fprintf (stderr,
		    "addfeahd: field size <= zero for field %s\n",
		    field_name);
	    exit (1);
	}

	if ((type = lin_search (type_codes, field_type)) == -1) {
	    Fprintf (stderr,
		    "addfeahd: bad type given for field %s\n",
		    field_name);
	    exit (1);
	}
	if (skip < 0) {
		Fprintf(stderr,"addfeahd: skip must be >= 0\n");
	 	exit(1);
	}

/* create the field
*/
	if(add_fea_fld (field_name, field_size, (field_size != 1), (long *) NULL,
		type, (char **) NULL, out_hd) == -1)
	    error2("error adding field:",field_name,0);

	if(add_fea_fld (field_name, field_size, (field_size != 1), (long *) NULL,
		type, (char **) NULL, in_hd) == -1)
	    error2("error adding field:",field_name,0);


/* if the input is ASCII, save the current field information for later use.
 */
	if (aflag)
		{
		if	(field_cnt == SCNINFO_MAX)
			{
			Fprintf(stderr,
		"addfeahd: with ascii input you can only specify %d fields.\n",
				SCNINFO_MAX);
			exit(1);
			}
		ScnInfo[field_cnt].SI_Name = savestring(field_name);
		ScnInfo[field_cnt].SI_Type = type;
		switch (type)
			{
		case (DOUBLE) :
			ScnInfo[field_cnt].SI_Size = sizeof(double);
			break;
		case (FLOAT) :
			ScnInfo[field_cnt].SI_Size = sizeof(float);
			break;
		case (LONG) :
			ScnInfo[field_cnt].SI_Size = sizeof(long);
			break;
		case (SHORT) :
			ScnInfo[field_cnt].SI_Size = sizeof(short);
			break;
		case (CHAR) :
			ScnInfo[field_cnt].SI_Size = sizeof(char);
			ScnInfo[field_cnt].SI_Buf = malloc(field_size+1);
			break;
		case (BYTE) :
			ScnInfo[field_cnt].SI_Size = sizeof(char);
			break;
		default :
			Fprintf(stderr,
	"addfeahd: ascii input for %s type data not implemented.\n",
				field_type);
			exit(1);
			}
		ScnInfo[field_cnt].SI_Count = field_size;
		ScnInfo[field_cnt].SI_Skip = savestring(skip_format);
		ScnInfo[field_cnt].SI_Scan = savestring(element_format);
		field_cnt += 1;
		}
    }
/* fix up the output header
*/
    (void)strcpy (out_hd -> common.prog, "addfeahd");
    (void)strcpy (out_hd -> common.vers, VERSION);
    (void)strcpy (out_hd -> common.progdate, DATE);
    add_comment (out_hd, get_cmd_line (argc, argv));

    (void) add_comment (out_hd, combuf);

    if (comment_file)
	get_comment(comment_file, out_hd);
    else if (comment)
	add_comment (out_hd, comment);
    else if (strcmp("-",format_name) && strcmp("-",input_name))
	get_comment("<stdin>", out_hd);
    else
	error (
	 "you must give a comment on the command line or in the parameter file"
	 ,0);

/*
  if foreign_header is defined, then read the foreign_header size
  given by the skip option into a buffer and then save a pointer
  to it as the generic foreign_hd_ptr
*/

    if(foreign_header) {
    	*add_genhd_l("foreign_hd_length", (long *)NULL, 1, out_hd) = skip;
#if !defined(IBM_RS6000) && !defined(DS3100) && !defined(SG) && !defined(HP400) && !defined(M5600) && !defined(OS5) && !defined(LINUX)
	(char *)foreign_hd_ptr = malloc((unsigned)skip);
#else
	foreign_hd_ptr = malloc((unsigned)skip);
#endif
	spsassert(foreign_hd_ptr, "btosps: malloc failed");
	*add_genhd_l("foreign_hd_ptr", (long *)NULL, 1, out_hd) =
			(long)foreign_hd_ptr;
	if(!fread((char *)foreign_hd_ptr,1,skip,in_file)) {
	  fprintf(stderr,"btosps: cannot read foreign header!\n");
	  exit(1);
	}
    }

    write_header (out_hd, out_file);

    rec = allo_fea_rec (out_hd);

/* Allocate field pointers if input is ascii.
 */
    if (aflag)
	    {
	    for	(f = 0; f < field_cnt; ++f)
		    {
		    ScnInfo[f].SI_Ptr
			    = get_fea_ptr(rec, ScnInfo[f].SI_Name, out_hd);
		    }
	    }

/* skip some input if the skip option used */

    if (skip)
    	(void)fseek(in_file, (long)skip, 0);

/* Now produce the output records.
 * If the input file is binary, this is easy:
 * just copy data using the new header.
 * If the input file is ascii, we have to scan the input, and
 * piece together the record contents.
 */

    if	(aflag)
	    {
	    for	(out_count = 0, Status = 0; Status != EOF ; )
		    {
		    for	(f = 0; f < field_cnt; ++f)
			    {
			    if	(ScnInfo[f].SI_Skip[0] != '\0')
				    if	((Status = fscanf(in_file,
							  ScnInfo[f].SI_Skip))
					 == EOF)
					    break;
			    Ptr = ScnInfo[f].SI_Ptr;
			    if	(ScnInfo[f].SI_Type == CHAR)
				    {
				    /*
				    fprintf(stderr,
    "record = %d; field = %s; scan=%s, ptr=0x%x\n",
						    out_count,
						    ScnInfo[f].SI_Name,
						    ScnInfo[f].SI_Scan,
						    Ptr
					    );
				    */
				    Status = fscanf(in_file,
						    ScnInfo[f].SI_Scan,
						    ScnInfo[f].SI_Buf);
				    if	(Status == 0)
					    {
					    Fprintf(stderr,
		      "addfeahd: error scanning input file:\n");
					    Fprintf(stderr,
		      "\trecord = %d; field = %s\n",
						    out_count,
						    ScnInfo[f].SI_Name);
					    Fprintf(stderr,
						    "\tinput file at \"");
					    for	(e = 0; e < 20; ++e)
						    {
						    c = getc(in_file);
						    if	(c != EOF)
							    Fprintf(stderr, "%c", c);
						    }
					    Fprintf(stderr, "\"\n");
					    exit(1);
					    }
				    if	( Status == EOF && (f != 0) )
					    {
					    Fprintf(stderr,
		      "addfeahd: unexpected end of input file:\n");
					    Fprintf(stderr,
		      "\trecord = %d; field = %s\n",
						    out_count,
						    ScnInfo[f].SI_Name);
					    exit(1);
					    }
				    if	( Status == EOF )
					    {
					    /*
					    fprintf(stderr,
		      "addfeahd: end of input file:\n");
					    fprintf(stderr,
		      "\trecord = %d; field = %s\n",
						    out_count,
						    ScnInfo[f].SI_Name);
					    */
					    break;
					    }
				    strncpy(Ptr, ScnInfo[f].SI_Buf,
					    ScnInfo[f].SI_Count);
				    }
			    else for	(e = 0; e < ScnInfo[f].SI_Count;
				 Ptr += ScnInfo[f].SI_Size, ++e)
				    {
				    /*
				    fprintf(stderr,
    "record = %d; field = %s; element = %d; scan=%s, ptr=0x%x\n",
						    out_count,
						    ScnInfo[f].SI_Name,
						    e,
						    ScnInfo[f].SI_Scan,
						    Ptr
					    );
				    */
				    Status = fscanf(in_file,
						    ScnInfo[f].SI_Scan,
						    Ptr);
				    if	(Status == 0)
					    {
					    Fprintf(stderr,
		      "addfeahd: error scanning input file:\n");
					    Fprintf(stderr,
		      "\trecord = %d; field = %s; element = %d\n",
						    out_count,
						    ScnInfo[f].SI_Name,
						    e);
					    Fprintf(stderr,
						    "\tinput file at \"");
					    for	(e = 0; e < 20; ++e)
						    {
						    c = getc(in_file);
						    if	(c != EOF)
							    Fprintf(stderr, "%c", c);
						    }
					    Fprintf(stderr, "\"\n");
					    exit(1);
					    }
				    if	( Status == EOF && (f != 0 || e != 0) )
					    {
					    Fprintf(stderr,
		      "addfeahd: unexpected end of input file:\n");
					    Fprintf(stderr,
		      "\trecord = %d; field = %s; element = %d\n",
						    out_count,
						    ScnInfo[f].SI_Name,
						    e);
					    exit(1);
					    }
				    if	( Status == EOF )
					    {
					    /*
					    fprintf(stderr,
		      "addfeahd: end of input file:\n");
					    fprintf(stderr,
		      "\trecord = %d; field = %s; element = %d\n",
						    out_count,
						    ScnInfo[f].SI_Name,
						    e);
					    */
					    break;
					    }

				    /*
				    fprintf(stderr, "value = ");
				    if	(ScnInfo[f].SI_Type == SHORT)
					    fprintf(stderr,
						    ScnInfo[f].SI_Scan,
						    *(short *)Ptr);
				    else if (ScnInfo[f].SI_Type == FLOAT)
					    fprintf(stderr,
						    ScnInfo[f].SI_Scan,
						    *(float *)Ptr);
				    else if (ScnInfo[f].SI_Type == CHAR)
					    fprintf(stderr,
						    ScnInfo[f].SI_Scan,
						    *(char *)Ptr);
				    fprintf(stderr, "\n");
				    */
				    }
			    if	(Status == EOF)
				    break;
			    }
		    if	(Status != EOF)
			    {
			    put_fea_rec (rec, out_hd, out_file);
			    out_count += 1;
			    }
		    }
	    }
    else    {
	    while (get_fea_rec (rec, in_hd, in_file) != EOF) {
		    put_fea_rec (rec, out_hd, out_file);
		    out_count++;
		    }
	    }

    Fprintf (stderr, "addfeahd: %ld records processed.\n", out_count);
    return(0);

}
