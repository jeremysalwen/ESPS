/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1991-1996 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * This program computes the dynamic time warping distances between
 * sequences.  The sequences are read from FEA files specified in
 * ASCII files.
 */

static char *sccs_id = "%W%	%G%	ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
#include <esps/spsassert.h>
#include <esps/limits.h>

#define PROG "dtw_rec"
#define VERSION "%I%"
#define DATE "%G%"

#define REQUIRE(test, text) {if (!(test)) { \
  (void) fprintf(stderr, "%s: %s  Exiting.\n", PROG, text); \
  exit(1);}}

#define SYNTAX {\
fprintf(stderr, "%s: dynamic time warp distance between FEA files.\n", PROG); \
fprintf(stderr,"usage: %s [options] reference_list test_list results\n", PROG);\
fprintf(stderr, " -P parameter file (default: params)\n");\
fprintf(stderr, " -f sequence_field\n");\
fprintf(stderr, " -b best_so_far\n"); \
fprintf(stderr, " -l best_list_length (default: 1)\n"); \
fprintf(stderr, " -d delta (default: 0)\n"); \
fprintf(stderr, " -c distance_table_file\n"); \
fprintf(stderr, " -t distance_table_field\n"); \
fprintf(stderr, " -r distance_table_recno\n"); \
fprintf(stderr, " -x debug_level (default: 0) \n");\
exit(1);}

#define Fprintf (void)fprintf
#define Sprintf (void)sprintf

/* global declarations */
int 		debug_level=0;
unsigned long    flags;
char *	     ptr;

#if !defined(IBM_RS6000) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
    char             *calloc();
    int              fprintf();
    int              fscanf();
    void             exit();
#endif

main(argc, argv)
int argc;
char **argv;
{
    extern char      *optarg;
    extern int       optind;
    int              opt;

    char             *param_file = "params";
    char             *vfieldname = "spec_param";
    char             *ifieldname = "spec_param_cwndx";
    char             *fieldname = NULL;
    int              fieldname_flag=0;
    int              debug_level_flag=0;
    int              delta_flag=0;
    long             delta=0;
    long             best_list_len=1;
    long             list_len;
    int              best_list_len_flag=0;
    double           *best_list=NULL;
    long             *best_id=NULL;
    int              best_so_far_flag=0;
    long             dim=0;
    double           dist;
    double           *dist_ptr;
    long             *mapping=NULL;
    int              i, j, k, ref_to_test_ptr;
    float            *fptr;
    long             *lptr;
    double           *dptr;

    FILE             *tbl_fp;
    char             *tbl_fname=NULL;
    char             *tbl_fieldname = "distance_table";
    int              tflag=0;
    int              rflag=0;
    int              cflag=0;                    /* if set, expect sequences of integers */
    long             tbl_recno = LONG_MAX;
    struct header    *tbl_hd;
    struct vqcbk     *tbl_rec;
    double           **distances=NULL;
    double           *fea_dist_ptr;
    int              cbk_dim=0;

    FILE             *res_fp;
    char             *res_fname;

    char             name[200];
    char             *tname;

    FILE             *ref_list_fp;
    char             *ref_list_fname;
    int              ref_list_len=0;
    char             **ref_fnames;
    float            ***ref_vseqs;
    long             **ref_iseqs;
    char             **ref_seq_id;
    long             *ref_seq_len;
    long             ref_ptr;

    FILE             *test_list_fp;
    char             *test_list_fname;
    int              test_list_len=0;
    char             **test_fnames;
    float            **test_vseq;
    long             *test_iseq;
    char             *test_seq_id;
    long             test_seq_len;
    long             test_ptr;

    double           dtw_l2();
    double           dtw_tl();
    double           l2_dist();
    double           atof();
    int              atoi();
    long             get_fea_siz();
    char             *savestring();
    char             *get_cmd_line();
    char             *strcpy();
    char             *get_fea_ptr();
    struct fea_data  *allo_fea_rec();
    int              getopt();
    double           **d_mat_alloc();
    float            **f_mat_alloc();
    FILE             *fopen();
    long             *fea_read_int_sequence();
    float            **fea_read_vec_sequence();


    while ( (opt = getopt(argc, argv,"P:f:bd:t:l:c:r:x:")) != EOF )
      switch(opt) {
      case 'P':
	param_file = optarg;
	break;
      case 'b':
	best_so_far_flag++;
	break;
      case 'd':
        delta = atoi(optarg);
	spsassert(delta>=0,"delta must be positive.\n");
	delta_flag++;
	break;
      case 'l':
        best_list_len = atoi(optarg);
	spsassert(best_list_len>0, "best_list_length must be positive.\n");
	best_list_len_flag++;
	break;
      case 'f':
        fieldname = optarg;
	fieldname_flag++;
	break;
      case 'c':
        tbl_fname = optarg;
        cflag++;
        break;
      case  'r':
        tbl_recno  = atoi(optarg);
        rflag++;
        break;
      case 't' :
	tbl_fieldname = optarg;
        tflag++;
        break;
      case 'x': debug_level=atoi(optarg);
	debug_level_flag++;
	break;
  }

    /* process parameters */
    (void) read_params(param_file, SC_NOCOMMON, (char *)NULL);
    if ( debug_level_flag == 0 && symtype("debug_level") != ST_UNDEF )
      debug_level = getsym_i("debug_level");
    if ( fieldname_flag == 0 && symtype("sequence_field") != ST_UNDEF ) {
      fieldname = getsym_s("sequence_field");
      fieldname_flag++;
    }
    if ( delta_flag == 0 && symtype("delta") != ST_UNDEF )
      delta = getsym_i("delta");
    if ( best_so_far_flag == 0 && symtype("best_so_far") != ST_UNDEF )
	if (getsym_i("best_so_far") == 1 )
	    best_so_far_flag++;
    if ( best_list_len_flag == 0 && symtype("best_list_length") != ST_UNDEF ) {
      best_list_len = getsym_i("best_list_length");
      best_list_len_flag++;
    }
    if ( tflag == 0 && symtype("distance_table_field") != ST_UNDEF )
      tbl_fieldname = getsym_s("distance_table_field");
    if ( rflag == 0 && symtype("distance_table_recno") != ST_UNDEF )
      tbl_recno = getsym_i("distance_table_recno");
    if ( cflag == 0 && symtype("distance_table_file") != ST_UNDEF ) {
	tbl_fname = getsym_s("distance_table_file");
	cflag++;
    }
    if ( !fieldname_flag ) {
	if (cflag)
	    fieldname = ifieldname;
	else
	    fieldname = vfieldname;
    }

    /* get distance table */
    if (cflag) {
	if (debug_level)
	    Fprintf(stderr, "%s: Getting distance table from file %s.\n",
		    PROG, tbl_fname);
	(void) eopen(PROG, tbl_fname, "r", FT_FEA, FEA_VQ, &tbl_hd, &tbl_fp);

	tbl_rec = allo_vqfea_rec(tbl_hd);

	if (tbl_recno == LONG_MAX && tbl_hd->common.ndrec != -1)
	    tbl_recno = tbl_hd->common.ndrec;

	if (tbl_recno != LONG_MAX)
	{
	    fea_skiprec(tbl_fp, tbl_recno - 1, tbl_hd);

	    REQUIRE(get_vqfea_rec(tbl_rec, tbl_hd, tbl_fp) != EOF,
		    "Premature end of file in FEA_VQ distortion file.");
	}
	else	/* Read records to find the last. */
	{
	    struct vqcbk	*next_rec, *tmp_rec;

	    next_rec = allo_vqfea_rec(tbl_hd);
	
	    REQUIRE(get_vqfea_rec(tbl_rec, tbl_hd, tbl_fp) != EOF,
		    "EOF when trying to read first distortion record.");

	    while (get_vqfea_rec(next_rec, tbl_hd, tbl_fp) != EOF)
	    {
		tmp_rec = tbl_rec;
		tbl_rec = next_rec;
		next_rec = tmp_rec;
	    }

	    free_fea_rec(tmp_rec->fea_rec);
	}

	fea_dist_ptr =
	    (double *) get_fea_ptr(tbl_rec->fea_rec, tbl_fieldname, tbl_hd);
	REQUIRE(fea_dist_ptr != NULL,
		"can't get field distance_table from distance file.");
	
	cbk_dim = *tbl_rec->current_size;

	if (debug_level>1)
	    Fprintf(stderr, "%s: codebook dimension %d.\n", PROG, cbk_dim);

	distances = d_mat_alloc( (unsigned) cbk_dim, (unsigned) cbk_dim);
	spsassert( distances != NULL, "can't allocate memory table.");

	for (i=0; i<cbk_dim; i++)
	    for (j=0; j<cbk_dim; j++)
		distances[i][j] = fea_dist_ptr[ i * cbk_dim + j];
	fclose(tbl_fp);
	free_fea_rec( tbl_rec->fea_rec );
    }

    /* open reference list file */
    if (optind == argc) {
	Fprintf(stderr, "No filename reference_list specified.\n");
	SYNTAX;
    }
    ref_list_fname = argv[optind++];
    spsassert( strcmp(ref_list_fname, "-")!=0, "standard input not allowed.");
    ref_list_fp = fopen( ref_list_fname, "r");
    spsassert( ref_list_fp != NULL, "Can't open reference_list.");
    if (debug_level)
	Fprintf(stderr, "%s: Reading reference list from %s.\n", PROG, ref_list_fname);

    /* open test list file */
    if (optind == argc) {
	Fprintf(stderr, "No filename test_list specified.\n");
	SYNTAX;
    }
    test_list_fname = argv[optind++];
    spsassert( strcmp(test_list_fname, "-")!=0, "standard input not allowed.");
    test_list_fp = fopen(test_list_fname, "r");
    spsassert( test_list_fp != NULL, "Can't open test_list.");
    if (debug_level)
	Fprintf(stderr, "%s: Reading test list from %s.\n", PROG, test_list_fname);

    /* read reference filenames */
    ref_list_len=0;
    while ( fscanf( ref_list_fp, "%s", name) > 0 )
	ref_list_len++;
    ref_fnames = (char **) malloc( ref_list_len * sizeof(char *));
    rewind(ref_list_fp);
    ref_list_len=0;
    while ( fscanf( ref_list_fp, "%s", name) > 0 ) {
	ref_fnames[ref_list_len] = savestring(name);
	if (debug_level>1)
	    Fprintf(stderr, "%s: FEA reference file %s .\n", PROG, ref_fnames[ref_list_len]);
	ref_list_len++;
    }
    fclose(ref_list_fp);
    if (debug_level)
	Fprintf(stderr, "%s: %d reference sequences.\n", PROG, ref_list_len);

    /* read test filenames */
    test_list_len=0;
    while ( fscanf( test_list_fp, "%s", name) > 0 )
	test_list_len++;
    test_fnames = (char **) malloc( test_list_len * sizeof(char *));
    rewind(test_list_fp);
    test_list_len=0;
    while ( fscanf( test_list_fp, "%s", name) > 0 ) {
	test_fnames[test_list_len] = savestring(name);
	if (debug_level>1)
	    Fprintf(stderr, "%s: FEA test file %s .\n", PROG, test_fnames[test_list_len]);
	test_list_len++;
    }
    fclose(test_list_fp);
    if (debug_level)
	Fprintf(stderr, "%s: %d test sequences.\n", PROG, test_list_len);

    if (optind < argc) {
	res_fname = argv[optind++];
	eopen(PROG, res_fname, "w", NONE, NONE, (struct header **)NULL, &res_fp);
	if (debug_level)
	    Fprintf(stderr, "%s: results file: %s\n", PROG, res_fname);
    }
    else {
	Fprintf(stderr, "%s: File name for results must be specified.\n", PROG);
	SYNTAX;
    }

    /* allocate memory and read reference sequences */
    if (debug_level>1)
	Fprintf(stderr, "%s: Allocating memory for reference sequences.\n", PROG);
    ref_seq_len = (long *) calloc( (unsigned) ref_list_len, sizeof(long));
    ref_seq_id = (char **) calloc( (unsigned) ref_list_len, sizeof(char **));
    spsassert(ref_seq_len != NULL && ref_seq_id != NULL,
	      "Can't allocate memory for reference sequences.");
    if (cflag) {
	ref_iseqs = (long **) calloc( (unsigned)ref_list_len, sizeof(long *));
	spsassert( ref_iseqs != NULL, "Can't allocate memory for reference sequences.");
    }
    else {
	ref_vseqs = (float ***) calloc( (unsigned)ref_list_len, sizeof(float **));
	spsassert( ref_vseqs != NULL, "Can't allocate memory for reference sequences.");
    }
    for (i=0; i<ref_list_len; i++)
	if (cflag)
	    ref_iseqs[i] = fea_read_int_sequence(ref_fnames[i], fieldname,
						 &(ref_seq_len[i]), &ref_seq_id[i]);
	else
	    ref_vseqs[i] = fea_read_vec_sequence(ref_fnames[i], fieldname,
						 &dim, &(ref_seq_len[i]), &ref_seq_id[i]);

    best_list = (double *) calloc( (unsigned)best_list_len, sizeof(double));
    best_id = (long *) calloc( (unsigned)best_list_len, sizeof(long));
    spsassert(best_list != NULL && best_id != NULL, "can't allocate memory for best_so_far list.");
    dptr = (best_so_far_flag) ? &(best_list[best_list_len-1]) : (double *) NULL;

    /* perform dtw matches */
    for (test_ptr=0; test_ptr<test_list_len; test_ptr++) {
	/* get test sequence */
	if (cflag)
	    test_iseq = fea_read_int_sequence(test_fnames[test_ptr], fieldname,
					      &test_seq_len, &test_seq_id);
	else
	    test_vseq = fea_read_vec_sequence(test_fnames[test_ptr], fieldname, &dim,
					      &test_seq_len, &test_seq_id);
	list_len=0;
	for (k=0; k<best_list_len; k++)
	    best_list[k] = DBL_MAX;
	for (ref_ptr=0; ref_ptr<ref_list_len; ref_ptr++) {
	    if (debug_level>2)
		Fprintf(stderr, "Comparing %s %s\n", test_seq_id, ref_seq_id[ref_ptr]);

	    /* global warping constraint */
	    if (2*test_seq_len > ref_seq_len[ref_ptr] && 2*ref_seq_len[ref_ptr] > test_seq_len ) {

		/* keep track of number of sequences compared */
		if (list_len < best_list_len)
		    list_len++;

		/* dtw */
		if (cflag)
		    dist = dtw_tl(test_iseq, ref_iseqs[ref_ptr],
				  test_seq_len, ref_seq_len[ref_ptr],
				  cbk_dim, delta, dptr, (long *)NULL, distances);
		else
		    dist = dtw_l2(test_vseq, ref_vseqs[ref_ptr],
				  test_seq_len, ref_seq_len[ref_ptr],
				  dim, delta, dptr, (long *)NULL, NULL);
		
		/* sort distances */
		k = list_len-1;
		while ( dist < best_list[k] ) {
		    if ( k == 0 ) {
			best_list[k] = dist;
			best_id[k] = ref_ptr;
		    } else {
			if ( dist < best_list[k-1] ) {
			    best_list[k] = best_list[k-1];
			    best_id[k] = best_id[k-1];
			    k--;
			} else {
			    best_list[k] = dist;
			    best_id[k] = ref_ptr;
			}
		    }
		}
		if (debug_level>2)
		    Fprintf(stderr, "%s: Compared %s %s %e\n",
			    PROG, test_seq_id, ref_seq_id[ref_ptr], dist);
	    } else
	    if (debug_level>2)
		Fprintf(stderr, "%s: %s and %s violate global warping constraint\n",
			PROG, test_seq_id, ref_seq_id[ref_ptr]);
	}					
	if (list_len) {
	    if (debug_level>1)
		for (k=0; k<list_len; k++)
		    Fprintf(stderr, "%s: %s %s %e\n",
			    PROG, test_seq_id, ref_seq_id[best_id[k]], best_list[k]);
	    for (k=0; k<list_len; k++)
		Fprintf(res_fp, "%s: %s %s %e\n",
			PROG, test_seq_id, ref_seq_id[best_id[k]], best_list[k]);
	}
	else
	    {
	    if (debug_level>1)
		Fprintf(res_fp, "%s: %s: NO COMPARABLE REFERENCE SEQUENCES.\n",
			PROG, test_seq_id);
	    Fprintf(res_fp, "%s: NO COMPARABLE REFERENCE SEQUENCES.\n", test_seq_id);
	}	

	if (cflag)
	    free(test_iseq);
	else
	    f_mat_free(test_vseq, test_seq_len);
		
    }
    fclose(res_fp);

    exit(0);

} /* end main */


/*
 * Get number of records in a file.
 * Replace input stream with temporary file if input is a pipe
 * or record length is variable.
 */

static long
n_rec(file, hd)
    FILE	    **file;
    struct header   **hd;
{
    if ((*hd)->common.ndrec != -1)  /* Input is file with fixed record size. */
	return (*hd)->common.ndrec; /* Get ndrec from header. */
    else			    /* Input is pipe, or record length
				     * is variable. */
    {
	FILE		*tmpstrm = tmpfile();
	struct header	*tmphdr; /* header for writing and reading temp file */
	struct fea_data	*tmprec; /* record for writing and reading temp file */
	long		ndrec = 0;

	/*
	 * Get version of header without any Esignal header, mu-law
	 * flag, etc.  Otherwise we risk getting garbage by writing the
	 * temp file as an ESPS FEA file and reading it back as some
	 * other format.
	 */
	tmphdr = copy_header(*hd);
	write_header(tmphdr, tmpstrm);
	tmprec = allo_fea_rec(tmphdr);

	for (ndrec = 0; get_fea_rec(tmprec, *hd, *file) != EOF; ndrec++)
	    put_fea_rec(tmprec, tmphdr, tmpstrm);

	Fclose(*file);
	(void) rewind(tmpstrm);
	*hd = read_header(tmpstrm);
	*file = tmpstrm;
	return ndrec;
    }
}


float **fea_read_vec_sequence( filename, fieldname, dim, ndrec, id)
char  *filename;
char  *fieldname;
long  *dim;
long  *ndrec;
char  **id;
{
    FILE             *fp;
    struct header    *hd;
    struct fea_data  *rec;
    int              i, j;
    float            *fptr;
    static long      ldim=0;
    float            **vseq;

    float            **f_mat_alloc();
    char             *get_genhd_c();
    char             *savestring();

    if (debug_level)
	Fprintf(stderr, "%s: Reading field %s from file %s.\n",
		PROG, fieldname, filename);
    (void) eopen(PROG, filename, "r", FT_FEA, NONE, &hd, &fp);
    spsassert( FLOAT == get_fea_type(fieldname, hd),
	      "Data type must be FLOAT.");
    *ndrec = n_rec(&fp, &hd);
    *dim = get_fea_siz(fieldname, hd, (short *)NULL, (long **)NULL);
    spsassert(*dim > 0, "Empty field.");
    if (ldim == 0)
	ldim = *dim;
    else
	spsassert( ldim == *dim, "Field must have same dimension in each file.");
    vseq = f_mat_alloc( (unsigned)(*ndrec), (unsigned)(*dim));
    spsassert(vseq != NULL, "Unable to allocate memory for vector sequence.");
    rec = allo_fea_rec(hd);
    fptr = (float *) get_fea_ptr(rec, fieldname, hd);
    spsassert(fptr != NULL, "Unable to get field from input record.");
    if ( genhd_type("sequence_id", &i, hd) != HD_UNDEF )
	*id = savestring( get_genhd_c("sequence_id", hd));
    else
	*id = filename;

    j=0;
    while( get_fea_rec( rec, hd, fp) != EOF ) {
	for (i=0; i < *dim; i++)
	    vseq[j][i] = fptr[i];
	j++;
    }
    spsassert(j == *ndrec, "Number of records read inconsistent with header info.");
    if (debug_level>1)
	Fprintf(stderr, "%s: %d records read.\n", PROG, *ndrec);

    free_fea_rec(rec);
    free_header(hd,flags,ptr);
    fclose(fp);
    return(vseq);
}


long *fea_read_int_sequence( filename, fieldname, ndrec, id)
char *filename;
char *fieldname;
long *ndrec;
char **id;
{
    FILE             *fp;
    struct header    *hd;
    struct fea_data  *rec;
    int              i, j;
    long             *iptr;
    long             *iseq;

    char             *savestring();

    if (debug_level>1)
	Fprintf(stderr, "%s: Reading field %s from file %s.\n",
		PROG, fieldname, filename);
    (void) eopen(PROG, filename, "r", FT_FEA, NONE, &hd, &fp);
    spsassert( LONG == get_fea_type(fieldname,hd), "Data type must be LONG.");
    *ndrec = n_rec(&fp, &hd);
    i = get_fea_siz( fieldname, hd, (short *)NULL, (long **)NULL);
    spsassert(i == 1, "Field must be dimension 1.");
    iseq = (long *) calloc( (unsigned) *ndrec, (unsigned) sizeof(long));
    spsassert(iseq != NULL, "Unable to allocate memory for integer sequence.");
    rec = allo_fea_rec(hd);
    iptr = (long *) get_fea_ptr(rec, fieldname, hd);
    spsassert(iptr != NULL, "Unable to get field from input record.");
    if ( genhd_type("sequence_id", &i, hd) != HD_UNDEF )
	*id = savestring( get_genhd_c("sequence_id", hd));
    else
	*id = filename;

    j=0;
    while( get_fea_rec( rec, hd, fp) != EOF )
	iseq[j++] = *iptr;
    spsassert(j == *ndrec, "Number of records read inconsistent with header info.");

    if (debug_level>2)
	Fprintf(stderr, "%s: %d records read.\n", PROG, *ndrec);

    free_fea_rec(rec);
    free_header(hd,flags,ptr);
    fclose(fp);
    return(iseq);
}
