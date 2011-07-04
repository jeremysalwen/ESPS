/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Ajaipal S. Virdy
 * Checked by:
 * Revised by:
 *
 * Brief description: compute various statistics on an ESPS FEA file
 */

static char *sccs_id = "@(#)fea_stats.c	1.16	6/18/98	ESI/ERL";

#define VERSION "1.16"
#define DATE "6/18/98"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/spsassert.h>
#include <assert.h>
#include <esps/feastat.h>
#include <esps/esignal_fea.h>


#define SYNTAX \
USAGE ("fea_stats [-b] [-d] [-f field] [-h file] [-i list] [-n class] [-r range]\n\
 [-x debug_level] [-A] [-C] [-E] [-I] [-M] [-R] infile [outfile]")

#define REALLOC(typ,num,var) { \
    if ((var) == NULL) \
	(var) = (typ *) emalloc((unsigned) (num) * sizeof(typ)); \
    else (var) = (typ *) realloc((char *) (var), \
				    (unsigned) (num) * sizeof(typ)); \
    spsassert((var), "can't allocate memory."); \
}

#define	NTYPES 9	/* number of data types available */

/*
 *  S T A T   O U T
 *   V A R I A B L E S
 */

long	statsize;

/*
 *  N U M B E R
 *   O F
 *    D A T A
 *     T Y P E S
 */

int	ndouble;
int	nfloat;
int	nlong;
int	nshort;
int	nchar;

/*
 *  E X T E R N A L
 *   V A R I A B L E S
 *    used by getopt(3-ESPS)
 */

extern int	optind;
extern char	*optarg;
char *ecalloc();
/* SDI 26/6/06 added LINUX_OR_MAC to avoid compiler error */
#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char  *calloc(), *realloc();
#endif

/*
 *  S P S
 *   F U N C T I O N S
 *    R E F E R E N C E D
 */

char	**get_fea_deriv();
int	getopt();
long	*grange_switch();
long	*fld_range_switch();
char	*get_cmd_line();
float	**f_mat_alloc();
void	lrange_switch();
char	*savestring();
char   *e_temp_name();

/*
 *  L O C A L
 *   F U N C T I O N S
 *    R E F E R E N C E D
 */

int	Numeric_Field();
void	Allo_Memory();
void	Process_non_stat_output();
void	Print_non_stat_output();
void	Process_stat_output();

/*
 *  G L O B A L
 *   V A R I A B L E S
 */

char	*Version = VERSION;
char	*Date = DATE;
char	*ProgName = "fea_stats";
int	debug_level = 0;


FILE	*instrm;	/* input file pointer */
FILE	*outstrm;	/* output file pointer */

char	*infile;	/* input file name */
char	*outfile = NULL; /* output file name */
char	*histfile;	/* histogram file name */

struct header
	*esps_hdr;	/* input file header */

struct header
	*fea_oh;	/* output file header (if stat-out == YES) */

struct fea_header
	*fea_hdr;	/* FEA file header */

struct fea_data
	*fea_rec;	/* FEA file record pointer */

long	ndrec;		/* number of records in file */
long	s_rec;		/* starting record number */
long	e_rec;		/* end record number */
long	n_rec;		/* total number of records to process */

int	*stat_field;	/* fields in FEA file to do statistics on */
int	nstat = 0;	/* number of fields to do statistics on */

int     bflag = 0;      /* use brief output format for basic statictics */
int	iflag = 0;	/* item list specified if iflag set */
long	*irange;	/* array containing item list specified with -i */
long	nitem = 0;	/* number of items specified with -i */
long	**item_arrays;	/* arrays of items in each field */
long	*n_items;	/* number of items to process in each field */
long	*max_elems;	/* max index of items to process in each field */

int	Rflag = 0;	/* treat each item in a FEA field seperately */

int	Mflag = 0;	/* Compute MEAN VECTOR if Mflag set */
int	Iflag = 0;	/* Compute INVERSE COVARIANCE MATRIX if Iflag set */
int	Cflag = 0;	/* Compute COVARIANCE MATRIX if Cflag set */
int	Eflag = 0;	/* Compute EIGENVALUES and EIGENVECTORS if Eflag set */
int	Aflag = 0;	/* Compute Non-Stat-Output also if Aflag set */

int	stat_out = NO;	/* flag for determining if stat-file output required */

float	**Data;		/* matrix to hold all data read in */

char	*statfield;	/* string denoting the field on which to
				   perform statistics */

int	update_file = NO;
char	*class_name;

char	*command_line;

/*
 *  M A I N
 *   P R O G R A M
 */

main(argc, argv)
    int   argc;
    char  **argv;
{
    int             c;		/* used for parsing command line */

    int             num_of_files;	/* number of files given on
					 * command line */

    /*
     *  C O M M A N D 
     *   L I N E 
     *    O P T I O N
     *     V A R I A B L E S 
     */

    char	    *rrange = NULL;	/* record range string */

    /* array of strings containing FEA fields */
    char	    **field_name = NULL;
    int             fflag = 0;	/* FEA field name given if fflag set */
    int             fnum = 0;	/* number of field names given */

    int             field_count;/* number of fields in a FEA file
				 * record */

    int		    nfield;	/* fflag ? fnum : field_count */

    int             dflag = NO;	/* take difference between adjacent
				 * items */

    int             nflag = NO;	/* specify a classification name if
				 * nflag set */

    int             hflag = NO;	/* produce raw ASCII data output if
				 * hflag set */


    /*
     *  M I S C E L L A N E O U S
     *   V A R I A B L E S 
     */

    int             i, j;	/* indexing variables for arrays */

    struct
    stat            buf;	/* used to determine if outfile (of
				 * type FEA_STAT) already exists or not */

    int             num_of_types = NTYPES;	/* number of data types
						 * available, like
						 * DOUBLE, FLOAT, LONG,
						 * etc.. */

    /*
     *  B E G I N
     *   M A I N
     *    P R O G R A M 
     */

    /* get options */

    while ((c = getopt(argc, argv, "bdf:h:i:n:r:x:ACIEMR")) != EOF)
	switch (c) {
       case 'b':
           bflag = YES;
           break;
	case 'd':
	    dflag = YES;
	    {
		Fprintf(stderr,
		    "%s: The -d option has not been implemented yet.\n",
		    ProgName);
		exit(1);
	    }
	    break;
	case 'f':
	    fnum++;
	    REALLOC(char *, fnum, field_name)
	    field_name[fnum - 1] = optarg;
	    fflag = YES;
	    break;
	case 'h':
	    hflag = YES;
	    histfile = optarg;
	    {
		Fprintf(stderr,
		    "%s: The -h option has not been implemented yet.\n",
		    ProgName);
		exit(1);
	    }
	    break;
	case 'i':
	    irange = grange_switch(optarg, &nitem);
	    iflag = YES;
	    break;
	case 'n':
	    if (nflag) {
		Fprintf(stderr,
		    "%s: The -n option can only be given once.\n",
		    ProgName);
		exit(1);
	    }
	    class_name = optarg;
	    nflag++;
	    break;
	case 'r':
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'A':
	    Aflag++;
	    break;
	case 'C':
	    Cflag++;
	    Mflag++;
	    Rflag = YES;
	    stat_out = YES;
	    break;
	case 'E':
	    Eflag++;
	    Rflag = YES;
	    stat_out = YES;
	    {
		Fprintf(stderr,
		    "%s: The -E option has not been implemented yet.\n",
		    ProgName);
		exit(1);
	    }
	    break;
	case 'I':
	    Iflag++;
	    Cflag++;
	    Mflag++;
	    Rflag = YES;
	    stat_out = YES;
	    break;
	case 'M':
	    Mflag++;
	    Rflag = YES;
	    stat_out = YES;
	    break;
	case 'R':
	    Rflag = YES;
	    break;
	default:
	    SYNTAX;
	}

    num_of_files = argc - optind;

    if ((num_of_files < 1) || (num_of_files > 2))
	SYNTAX;

    if (stat_out) {
	if (num_of_files != 2) {
	    Fprintf(stderr,
		"%s: please specify an output stat file with -{MICE} option(s).\n",
		ProgName);
	    exit(1);
	}
	if (!nflag) {
	    Fprintf(stderr,
		"%s: please specify classification name using the -n option.\n",
		ProgName);
	    exit(1);
	}
	if ((fnum > 1) || !fflag) {
	    Fprintf(stderr,
		"%s: please specify ONE field name with -{MICE} option(s).\n",
		ProgName);
	    exit(1);
	}

	statfield = field_name[0];

	if (iflag || strchr(statfield, '['))
	{
	    Fprintf(stderr, "%s%s%s",
		"For stat-file output for subranges, ",
		"please use the fea_deriv program ",
		"to collect the desired elements into a new field.\n");
	    exit(1);
	}

	if (debug_level > 2)
	    Fprintf(stderr, "%s: statfield = %s\n", ProgName, statfield);
	(void) fflush(stderr);

	outfile = argv[argc - 1];

	if (strcmp(outfile, "-") == 0) {
	    outstrm = stdout;
	    outfile = "<stdout>";
	    if (Aflag) {
		Fprintf(stderr,
		    "%s: Can't write stat file and ascii output both to stdout.\n",
		    ProgName);
		exit(1);
	    }
	} else {
	    if (stat(outfile, &buf) == 0) {	/* output file exists */
		update_file = YES;
		if (debug_level > 0)
		    Fprintf(stderr,
			"%s: %s already exists - update file.\n",
			ProgName, outfile);
		TRYOPEN(ProgName, outfile, "r+", outstrm);
	    } else {
		if (debug_level > 0)
		    Fprintf(stderr,
			"%s: %s does not exist - create file.\n",
			ProgName, outfile);
		TRYOPEN(ProgName, outfile, "w", outstrm);
	    }
	}

    } else {	/* !stat_out */
	if (num_of_files != 1) {
	    Fprintf(stderr,
		    "Output file specified without -{MICE} option.\n",
		    ProgName);
	    exit(1);
	}
    }

    /* open input file and read header */

    if (debug_level > 0)
	Fprintf(stderr,
	    "%s: Opening input FEA file and reading header...\n", ProgName);

    infile =
	eopen(ProgName, argv[optind], "r", FT_FEA, NONE, &esps_hdr, &instrm);

    /*
     * Check if complex fields are involved - exit if they are
    */
    if(!fflag){
      if(is_file_complex(esps_hdr) == YES){
	Fprintf(stderr, 
               "fea_stats: Complex data files not supported yet - exiting.\n");
	exit(1);
      }
    }else{/* check each specified field - exit if complex*/
      for (i=0; i<fnum; i++){
	if(is_field_complex(esps_hdr, field_name[i]) == YES){
	  Fprintf(stderr,
        "fea_stats: \"%s\" is a complex field; not supported yet - exiting.\n",
	     field_name[i]);
	  exit(1);
	}
      }
    }


    fea_hdr = esps_hdr->hd.fea;

    command_line = savestring(get_cmd_line(argc, argv));

    /* print out everything we have done */

    if (debug_level > 5) {
	Fprintf(stderr, "%s: command line = %s\n",
	    ProgName, command_line);
	Fprintf(stderr, "%s: infile = %s\n", ProgName, infile);
	Fprintf(stderr, "%s: outfile = %s\n",
	    ProgName, outfile ? outfile : "NULL");
	Fprintf(stderr, "%s: stat-file option has %sbeen specified.\n",
	    ProgName, (stat_out) ? "" : "not ");
	Fprintf(stderr, "%s: fnum = %d\n", ProgName, fnum);
	for (i = 0; i < fnum; i++)
	    Fprintf(stderr,
	    "%s: field_name[%d] = %s\n", ProgName, i, field_name[i]);
	Fprintf(stderr, "%s: nitem = %d\n", ProgName, nitem);
	for (i = 0; i < nitem; i++)
	    Fprintf(stderr,
	    "%s: irange[%d] = %ld\n", ProgName, i, irange[i]);
    }

    /* get number of records in file */

    fea_rec = allo_fea_rec(esps_hdr);

    if (esps_hdr->common.ndrec != -1)	/* input is fixed-record-size file */
	ndrec = esps_hdr->common.ndrec;	/* get ndrec from header */
    else				/* input is pipe or
					 * record size is variable */
    {
	char		*tmpname;
	FILE		*tmpstrm;
	struct header	*tmphdr;

	tmpname = e_temp_name("fstatsXXXXXX");
	TRYOPEN(ProgName, tmpname, "w+", tmpstrm);
	(void) unlink(tmpname);

	tmphdr = copy_header(esps_hdr);
	write_header(tmphdr, tmpstrm);
	ndrec = 0;
	while (get_fea_rec(fea_rec, esps_hdr, instrm) != EOF)
	{
	    put_fea_rec(fea_rec, tmphdr, tmpstrm);
	    ndrec++;
	}
	(void) rewind(tmpstrm);
	esps_hdr = read_header(tmpstrm);
	Fclose(instrm);
	instrm = tmpstrm;
    }

    /*
     * Find out if number of records in range is specified;
     * default is all frames.
     */

    if (debug_level > 0)
	Fprintf(stderr,
	    "%s: Checking record range in %s ...\n", ProgName, infile);

    s_rec = 1;
    e_rec = ndrec;

    lrange_switch(rrange, &s_rec, &e_rec, 1);

    if (s_rec < 1) {
	Fprintf(stderr, "%s: First record precedes 1.\n", ProgName);
	exit(1);
    }
    if (s_rec > e_rec) {
	Fprintf(stderr, "%s: Empty range specified.\n", ProgName);
	exit(1);
    }
    if (e_rec > ndrec)
    { /*changed by david burton June 1998 to make EnSig mods work*/

       /* reset last point to process to end of file */
       e_rec = ndrec;
    }

    n_rec = e_rec - s_rec + 1;

    field_count = fea_hdr->field_count;

    if (debug_level > 2)
	Fprintf(stderr,
	    "%s: n_rec = %d, field_count = %d\n",
	    ProgName, n_rec, field_count);

    /* Read output FEA_STAT file header if file existed */

    if (update_file) {	/* stat_file output to existing file */

	if (debug_level > 0)
	    Fprintf(stderr,
		"%s: Updating output FEA_STAT file, %s ...\n",
		ProgName, outfile);

	if ((fea_oh = read_header(outstrm)) == NULL)
	    NOTSPS(ProgName, outfile);

	if (get_esignal_hdr(fea_oh))
	{
	    Fprintf(stderr,
		    "%s: sorry, can't update Esignal files\n", ProgName);
	    exit(1);
	}
	if ((fea_oh->common.type) != FT_FEA) {
	    Fprintf(stderr,
		"%s: file %s is not an ESPS FEA file\n", ProgName, outfile);
	    exit(1);
	}
	if ((fea_oh->hd.fea->fea_type) != FEA_STAT) {
	    Fprintf(stderr,
		"%s: file %s is not an ESPS FEA_STAT file\n",
		ProgName, outfile);
	    exit(1);
	}
	if (strcmp(statfield, get_genhd("statfield", fea_oh)) != 0) {
	    Fprintf(stderr,
		"%s: incompatible statfields: infile = %s, outfile = %s\n",
		ProgName, statfield, get_genhd("statfield", fea_oh));
	    exit(1);
	}
	if (debug_level > 4)
	    Fprintf(stderr,
		"%s: Input FEA statfield is %s, output FEA_STAT statfield is %s\n",
		ProgName, statfield, get_genhd("statfield", fea_oh));

    } else
    if (stat_out) { /* stat-file output to new file */

	if (debug_level > 4)
	    Fprintf(stderr,
		"%s: Making new_header for output FEA_STAT file %s\n",
		ProgName, outfile);

	fea_oh = new_header(FT_FEA);
	spsassert(fea_oh, "Can't allocate space for output file header.");
    }

    /*
     * Allocate memory for stat_field. stat_field holds the indices of
     * fields in the feature file on which we will be performing
     * statistics. 
     */

    nfield = fflag ? fnum : field_count;

    if ((stat_field = (int *) ecalloc((unsigned) nfield, sizeof(int)))
	== NULL) {
	Fprintf(stderr,
	    "%s: calloc: cannot allocate memory for stat_field array.\n",
	    ProgName);
	exit(1);
    }

    if ((item_arrays = (long **) ecalloc((unsigned) nfield, sizeof(long *)))
	== NULL) {
	Fprintf(stderr,
	    "%s: calloc: cannot allocate memory for item_arrays array.\n",
	    ProgName);
	exit(1);
    }

    if ((n_items = (long *) ecalloc((unsigned) nfield, sizeof(long)))
	== NULL) {
	Fprintf(stderr,
	    "%s: calloc: cannot allocate memory for n_items array.\n",
	    ProgName);
	exit(1);
    }

    if ((max_elems = (long *) ecalloc((unsigned) nfield, sizeof(long)))
	== NULL) {
	Fprintf(stderr,
	    "%s: calloc: cannot allocate memory for max_elems array.\n",
	    ProgName);
	exit(1);
    }

    /*
     * If field names were specified on the command line, then
     * determine if these fields exist in the input feature file.  Then
     * also update stat_field to correspond to the fields selected. 
     */

    if (fflag) {

	if (debug_level > 1)
	    Fprintf(stderr,
		"%s: check if fields exist and statistics can be computed.\n",
		ProgName);

	/*
	 * fnum is the number of fields that were given on the
	 * command line.  Check through all the fields in the input
	 * feature file to see if a match occurs, if not, then exit. 
	 */

	for (j = 0; j < fnum; j++)
	{
	    item_arrays[j] =
		fld_range_switch(field_name[j], &field_name[j],
						&n_items[j], esps_hdr);

	    i = lin_search2(fea_hdr->names, field_name[j]);

	    if (i == -1)	/* bogus name given on command line */
	    {
		Fprintf(stderr,
		    "%s: field name %s does not exist in %s.\n",
		    ProgName, field_name[j], infile);
		exit(1);
	    }

	    /* The field selected must be a numeric field. */

	    if (!Numeric_Field(fea_hdr->types[i])) {
		Fprintf(stderr,
		    "%s: cannot compute statistics on %s field, %s.\n",
		    ProgName, type_codes[fea_hdr->types[i]],
		    fea_hdr->names[i]);
		exit(1);
	    }

	    /* We do not handle fields of rank greater than 2. */

	    if (fea_hdr->ranks[i] >= 2) {
		Fprintf(stderr,
		    "%s: cannot compute statistics on rank %d field, %s\n",
		    ProgName, fea_hdr->ranks[i], fea_hdr->names[i]);
		exit(1);
	    }

	    /*
	     * i is the index of the feature-field name that matches
	     * the name specified with the -f option.  Save the index
	     * in stat_field, and update nstat, which is the number of
	     * fields we will perform statistics on. 
	     */

	    

	    stat_field[nstat++] = i;

	    /*
	     * Save the count of each type of field for dynamic
	     * memory allocation. 
	     */

	    switch (fea_hdr->types[i]) {
	    case DOUBLE:
		ndouble++;
		break;
	    case FLOAT:
		nfloat++;
		break;
	    case LONG:
		nlong++;
		break;
	    case SHORT:
		nshort++;
		break;
	    case BYTE:
		nchar++;
		break;
	    default:
		Fprintf(stderr,
		    "%s: incorrect data type.\n", ProgName);
		exit(1);
	    }

	}   /* end for (j = 0; j < fnum; j++)  */

    }	/* end if (fflag) */ 
    else
    {	/* if (!fflag) */

	/*
	 * The -f option was not given, so now scan through all the
	 * fields in the input feature file which contain numeric
	 * data. 
	 */

	if (debug_level > 1)
	    Fprintf(stderr,
		"%s: determine which fields we can compute statistics on.\n",
		ProgName);

	/*
	 * num_of_types is the number ESPS data-types codes.  These
	 * are DOUBLE, FLOAT, LONG, SHORT, CHAR, UNDEF, CODED, BYTE, and NONE.
	 * We compute statistics only on fields of the first 4 types.
	 */

	for (j = 0; j < num_of_types; j++)

	    /*
	     * field_count is the number of fields in our input feature
	     * file. 
	     */

	    for (i = 0; i < field_count; i++)
		if (j == fea_hdr->types[i])
		{
		    if (debug_level > 5)
			Fprintf(stderr,
			    "%s: data type for field %s is %s.\n", ProgName,
			    fea_hdr->names[i], type_codes[fea_hdr->types[i]]);

		    if ( Numeric_Field(fea_hdr->types[i])
					    && fea_hdr->ranks[i] < 2 )
		    {
			long	k;
			long	len = fea_hdr->sizes[i];

			n_items[nstat] = len;
			item_arrays[nstat] = malloc_l((unsigned) len);
			spsassert(item_arrays[nstat],
			    "can't allocate space for array of item numbers");
			for (k = 0; k < len; k++)
			    item_arrays[nstat][k] = k;

			stat_field[nstat++] = i;

			switch (fea_hdr->types[i]) {
			case DOUBLE:
			    ndouble++;
			    break;
			case FLOAT:
			    nfloat++;
			    break;
			case LONG:
			    nlong++;
			    break;
			case SHORT:
			    nshort++;
			    break;
			case BYTE:
			    nchar++;
			    break;
			default:
			    Fprintf(stderr,
				"%s: incorrect data type.\n",
				ProgName);
			    exit(1);
			}
		    }
		}   /* end  if (j == ...) */
    }	/* end if ( !fflag ) */

    (void) fflush(stderr);

    /*
     * All done checking options and opening files. Now initialize
     * statistics' arrays, then read data, then compute statistics, and
     * finally write results. 
     */

    if (debug_level > 1)
	Fprintf(stderr,
	    "\n%s: computing statistics for nstat = %d fields.\n",
	    ProgName, nstat);

    if (debug_level > 2)
	for (i = 0; i < nstat; i++)
	    Fprintf(stderr,
		"%s: field name: %s, type: %s, size %d, rank %d\n",
		ProgName, fea_hdr->names[stat_field[i]],
		type_codes[fea_hdr->types[stat_field[i]]],
		fea_hdr->sizes[stat_field[i]],
		fea_hdr->ranks[stat_field[i]]);

    /*
     * We will be computing statistic on nstat number of items, unless
     * the -i option or the -R option was given.  Note: -i option has
     * higher priority. 
     */

    if (iflag)
    {
	Rflag = YES;

	for (i = 0; i < nstat; i++)
	{
	    item_arrays[i] = irange;
	    n_items[i] = nitem;
	}
    }

    /*
     * Determine how much memory we need to allocate. 
     */

    if (Rflag)
    {
	int	item_error = NO;

	if (debug_level > 1) {
	    Fprintf(stderr,
		"\n%s: checking for valid item range specification.\n",
		ProgName);
	}

	ndouble = 0;
	nfloat = 0;
	nlong = 0;
	nshort = 0;
	nchar = 0;

	for (i = 0; i < nstat; i++)
	{


/* Work around compiler bug. */
	    int	    s_f_i = stat_field[i];
	    long    m_e_i;

	    
	    max_elems[i] = 0;
	    for (j = 0; j < n_items[i]; j++)
		if (max_elems[i] < item_arrays[i][j])
		    max_elems[i] = item_arrays[i][j];


/* Work around compiler bug. */
	    m_e_i = max_elems[i];
	    if (fea_hdr->sizes[s_f_i] <= m_e_i)

/*
	    if (fea_hdr->sizes[stat_field[i]] <= max_elems[i])
*/
	    {
		item_error = YES;
		Fprintf(stderr, "%s: field %s has %d items only.\n",
		    ProgName, fea_hdr->names[stat_field[i]],
		    fea_hdr->sizes[stat_field[i]]);
	    }
	    if (fea_hdr->types[stat_field[i]] == DOUBLE)
		ndouble += n_items[i];
	    if (fea_hdr->types[stat_field[i]] == FLOAT)
		nfloat += n_items[i];
	    if (fea_hdr->types[stat_field[i]] == LONG)
		nlong += n_items[i];
	    if (fea_hdr->types[stat_field[i]] == SHORT)
		nshort += n_items[i];
	    if (fea_hdr->types[stat_field[i]] == BYTE)
		nchar += n_items[i];

	}

	if (item_error) {
	    Fprintf(stderr,
		"%s: incorrect item range specified.\n", ProgName);
	    exit(1);
	}
    }				/* end if (Rflag) */

    /*
     * Allocate memory.
     */

    if (debug_level > 2) {
	Fprintf(stderr,
	    "%s: ndouble = %d, nfloat = %d, nlong = %d, nshort = %d, nchar = %d\n",
	    ProgName, ndouble, nfloat, nlong, nshort, nchar);
    }

    Allo_Memory();

    /*
     * Now read the FEA data. 
     */

    if (debug_level > 0)
	Fprintf(stderr,
	    "%s: skipping %d records in %s.\n", ProgName, s_rec - 1, infile);

    fea_skiprec(instrm, s_rec - 1, esps_hdr);

    /*
     * If we have to compute stat-file output, read data. 
     */

    if (stat_out)
    {
	statsize = n_items[0];

	if (update_file) {

	    char          **derived_in;
	    char          **derived_ref;

	    if (statsize != *(long *) get_genhd("statsize", fea_oh)) {
		Fprintf(stderr,
		    "%s: incompatible statsizes: infile = %ld, outfile = %ld\n",
		    ProgName, statsize, *(long *) get_genhd("statsize", fea_oh));
		exit(1);
	    }
	    derived_in = get_fea_deriv(statfield, esps_hdr);
	    derived_ref = get_fea_deriv(statfield, fea_oh->variable.refhd);

	    if (((derived_in == NULL) && (derived_ref != NULL)) ||
	      ((derived_in != NULL) && (derived_ref == NULL))) {
		Fprintf(stderr,
		    "%s: field derived in %s or %s->ref_hdr but not both\n",
		    ProgName, infile, outfile);
		exit(1);
	    }
	    if (derived_in != NULL)
	    {
		i = 0;
		while ((derived_in[i] != NULL) && (derived_ref[i] != NULL)) {
		    if (strcmp(derived_in[i], derived_ref[i]) != 0) {
			Fprintf(stderr,
			    "%s: derived fields not equal in %s and %s->ref_hdr:\n",
			    ProgName, infile, outfile);
			Fprintf(stderr,
			    "\t   derived field names: input = %s, output->ref_hdr = %s.\n",
			    derived_in[i], derived_ref[i]);
			exit(1);
		    }
		    i++;
		}
		if ((derived_in[i] != NULL) || (derived_ref[i] != NULL)) {
		    Fprintf(stderr,
			"%s: different number of derived fields in %s and %s.\n",
			ProgName, infile, outfile);
		    exit(1);
		}
	    }
	}
    }
    if (Aflag || !stat_out) {

	if (debug_level > 2)
	    Fprintf(stderr,
		 "\n%s: computing non-stat-file output.\n", ProgName);

	if (stat_out) {

	    if (debug_level > 2)
		Fprintf(stderr,
		    "%s: allocating memory: statsize = %ld\n",
		    ProgName, statsize);

	    /*
	     * A single field should have been specified with the -f
	     * option if STAT_OUT == YES. Allocate n_rec by statsize
	     * memory to store all data in a two-dimensional array if
	     * the -{CI} has been specified. 
	     */

	    if (Cflag || Iflag)
		if ((Data = (float **) f_mat_alloc((unsigned) n_rec,
				      (unsigned) statsize)) == NULL) {
		    Fprintf(stderr,
			"%s: f_mat_alloc: could not allocate memory for Data.\n",
			ProgName);
		    exit(1);
		}
	}   /* end if (stat_out)  */
	Process_non_stat_output();
	Print_non_stat_output();

    }	/* end if (Aflag || !stat_out) */

    if (Aflag) { /* get to beginning of file again */

	rewind(instrm);
	esps_hdr = read_header(instrm);

	if (debug_level > 0)
	    Fprintf(stderr,
		"%s: skipping %d records in %s.\n",
		ProgName, s_rec - 1, infile);
	fea_skiprec(instrm, s_rec - 1, esps_hdr);
    }
    /*
     * Now let the external routine do all the stat-out processing. 
     */

    if (stat_out)
	Process_stat_output();

    exit(0);
    /* NOTREACHED */
}


int
Numeric_Field(type)
    int	    type;
{
    return (type == DOUBLE || type == FLOAT || type == LONG || type == SHORT
		|| type == BYTE);
}
