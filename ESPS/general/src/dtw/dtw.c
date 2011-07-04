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
 * This program computes the dynamic time warping distance between two
 * sequences read from FEA files.
 */

static char *sccs_id = "@(#)dtw.c	1.6	1/21/97	ERL";

#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>
#include <esps/spsassert.h>
#include <esps/limits.h>

#define PROG "dtw"
#define VERSION "1.6"
#define DATE "1/21/97"

#define REQUIRE(test, text) {if (!(test)) { \
  (void) fprintf(stderr, "%s: %s  Exiting.\n", PROG, text); \
  exit(1);}}

#define SYNTAX {\
fprintf(stderr, "%s: dynamic time warp distance between two FEA files.\n", PROG); \
fprintf(stderr,"usage: %s [options] reference test [results]\n", PROG);\
fprintf(stderr, " -P parameter file (default: params)\n");\
fprintf(stderr, " -f sequence_fieldname (default: spec_param)\n");\
fprintf(stderr, " -b best_so_far (default: 0)\n"); \
fprintf(stderr, " -d delta (default: 0)\n"); \
fprintf(stderr, " -c cbk_fea_vq\n"); \
fprintf(stderr, " -r cbk_recno\n"); \
fprintf(stderr, " -t distance_table_fieldname \n"); \
fprintf(stderr, " -x debug_level (default: 0) \n");\
exit(1);}		

#define Fprintf (void)fprintf
#define Sprintf (void)sprintf

static long n_rec();

/* global declarations */
int debug_level=0;

main(argc, argv)
int argc;
char **argv;
{
    extern char      *optarg;
    extern int       optind;
    int              opt;

    char             *param_file = NULL;
    char             *fieldname=NULL;
    char             *vfieldname = "spec_param";
    char             *ifieldname = "spec_param_cwndx";
    int              fieldname_flag=0;
    int              debug_level_flag=0;
    int              delta_flag=0;
    long             delta=0;
    double           best_so_far=DBL_MAX;
    int              best_so_far_flag=0;
    int              results_flag=0;
    double           dist;
    long             *mapping=NULL;
    int              i, j, k;
    long             dim;
    float            *fptr;
    long             *lptr;
    double           *dptr;
    double           *dist_ptr;
    long             *map_ptr;

    FILE             *tbl_fp;
    char             *tbl_fname=NULL;
    char             *tbl_fieldname = "distance_table";
    int              tflag=0;
    int              rflag=0;
    int              cflag=0;	/* if set, expect sequences of integers */
    long             tbl_recno = LONG_MAX;
    struct header    *tbl_hd;
    struct vqcbk     *tbl_rec;
    double           **distances=NULL;
    double           *fea_dist_ptr;
    int              cbk_dim=0;

    FILE             *ref_fp;
    char             *ref_fname=NULL;
    struct header    *ref_hd;
    struct fea_data  *ref_rec;
    float            **ref_seq_v;
    long             *ref_seq_l;
    long             ref_ndrec;

    FILE             *test_fp;
    char             *test_fname=NULL;
    struct header    *test_hd;
    struct fea_data  *test_rec;
    float            **test_seq_v;
    long             *test_seq_l;
    long             test_ndrec;

    FILE             *res_fp;
    char             *res_fname;
    struct header    *res_hd;
    struct fea_data  *res_rec;

    double           dtw_l2();
    double           dtw_tl();
    double           euclidean_dist();
    double           table_lookup();
    double           atof();
    int              atoi();
    long             get_fea_siz();
    char             *get_cmd_line();
    char             *strcpy();
    char             *get_fea_ptr();
    struct fea_data  *allo_fea_rec();
#if !defined(IBM_RS6000) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
    char             *calloc();
    int              fprintf();
    void             exit();
#endif
    int              getopt();
    double           **d_mat_alloc();
    float            **f_mat_alloc();
    void             free_fea_rec();

    while ( (opt = getopt(argc, argv,"P:f:b:d:t:c:r:x:")) != EOF )
      switch(opt) {
      case 'P': param_file = optarg;
	break;
      case 'b': best_so_far = atof(optarg);
	REQUIRE(best_so_far>=0.0, "best_so_far must be positive.");
	best_so_far_flag++;
	break;
      case 'd': delta = atoi(optarg);
	REQUIRE(delta>=0, "delta must be positive.");
	delta_flag++;
	break;
      case 'f': fieldname = optarg;
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
    if ( best_so_far_flag == 0 && symtype("best_so_far") != ST_UNDEF ) {
      best_so_far = getsym_d("best_so_far");
      best_so_far_flag++;
    }
    if ( tflag == 0 && symtype("distance_fieldname") != ST_UNDEF )
      tbl_fieldname = getsym_s("distance_fieldname");
    if ( rflag == 0 && symtype("distance_recno") != ST_UNDEF )
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
    if (cflag)
    {
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
	REQUIRE(distances != NULL, "can't allocate memory table.");

	for (i = 0; i < cbk_dim; i++)
	    for (j = 0; j < cbk_dim; j++)
		distances[i][j] = fea_dist_ptr[i*cbk_dim + j];

	fclose(tbl_fp);
	free_fea_rec(tbl_rec->fea_rec);
    }

    /* open reference file */
    if (optind < argc) {
      REQUIRE(strcmp(argv[optind],"-")!=0, "standard input not allowed.");
      ref_fname =
	eopen(PROG, argv[optind++], "r", FT_FEA, NONE, &ref_hd, &ref_fp);
    }
    else {
	Fprintf(stderr, "%s: no reference filename specified.\n", PROG);
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr, "%s: reference file: %s\n", PROG, ref_fname);

    /* open test file */
    if (optind < argc) {
      REQUIRE(strcmp(argv[optind],"-")!=0, "standard input not allowed.");
      test_fname =
	eopen(PROG, argv[optind++], "r", FT_FEA, NONE, &test_hd, &test_fp);
    }
    else {
	Fprintf(stderr, "%s: no test filename specified.\n", PROG);
	SYNTAX
    }
    if (debug_level)
	Fprintf(stderr, "%s: test file: %s\n", PROG, test_fname);

    if (optind < argc) {
	results_flag++;
	res_fname = argv[optind++];
	if (debug_level)
	    Fprintf(stderr, "%s: results file: %s\n", PROG, res_fname);
    }

    test_ndrec = n_rec(&test_fp, &test_hd);
    ref_ndrec = n_rec(&ref_fp, &ref_hd);

    if (debug_level)
	Fprintf(stderr, "%s: Reference: %s  records: %d\n",
		PROG, ref_fname, ref_ndrec);
    /* check to see that test and ref files are of comparable lengths */
    if ( 2*ref_ndrec <= test_ndrec || 2*test_ndrec <= ref_ndrec ) {
	if (debug_level)
	    Fprintf(stderr,
		    "%s: data lengths violate global warping constraint.\n",
		    PROG, ref_fname, test_fname);
	else
	    Fprintf(stdout, "%e\n", best_so_far);
	exit(0);
    }

    /* get reference data */
    if (debug_level>1)
	Fprintf(stderr, "Getting reference data from field %s.\n", fieldname);
    ref_rec = allo_fea_rec(ref_hd);
    dim = get_fea_siz( fieldname, ref_hd, (short *)NULL, (long **)NULL);
    if ( cflag ) {
	/* prepare to read sequence of long integers */
	REQUIRE(LONG == get_fea_type(fieldname,ref_hd),
		"Data type must be LONG.");
	REQUIRE(dim == 1, "Field must have dimension 1.");
	lptr = (long *) get_fea_ptr(ref_rec, fieldname, ref_hd);
	REQUIRE(lptr != NULL,
		"Unable to get specified field from reference file.");
	ref_seq_l = (long *) calloc( (unsigned) ref_ndrec, sizeof(long) );
	REQUIRE(ref_seq_l != NULL,
		"Can't allocate memory for reference input - type long.");
    }
    else {
	/* prepare to read sequence of float vectors */
	REQUIRE(FLOAT == get_fea_type(fieldname,ref_hd),
		"Data type must be FLOAT.");
	REQUIRE(dim > 0, "Empty field.");
	fptr = (float *) get_fea_ptr(ref_rec, fieldname, ref_hd);
	REQUIRE(fptr != NULL,
		"Unable to get specified field from reference file.");
	ref_seq_v =
	    (float **) f_mat_alloc((unsigned)ref_ndrec, (unsigned)dim);
	REQUIRE(ref_seq_v != NULL,
		"Can't allocate memory for reference input. - type float.");
    }
    j=0;
    while( get_fea_rec(ref_rec, ref_hd, ref_fp) != EOF ) {
	REQUIRE(j<ref_ndrec,
		"more records in reference file than specified in header.");
	if (cflag)
	    ref_seq_l[j] = *lptr;
	else
	    for (i=0; i<dim; i++)
		ref_seq_v[j][i] = fptr[i];
	j++;
    }
    (void)fclose(ref_fp);
    free_fea_rec(ref_rec);

    /* get test data */
    if (debug_level>1)
	Fprintf(stderr, "Getting test data from field %s.\n", fieldname);
    test_rec = allo_fea_rec(test_hd);
    REQUIRE(dim == get_fea_siz(fieldname, test_hd,
			       (short *) NULL, (long **) NULL),
	      "Incompatible field dimensions.");
    if ( cflag ) {
	/* prepare to read sequence of long integers */
	REQUIRE(LONG == get_fea_type(fieldname,test_hd),
		"Data type must be LONG.");
	lptr = (long *) get_fea_ptr(test_rec, fieldname, test_hd);
	REQUIRE(lptr != NULL,
		"Unable to get specified field from test file.");
	test_seq_l = (long *) calloc( (unsigned) test_ndrec, sizeof(long) );
	REQUIRE(test_seq_l != NULL,
		"Can't allocate memory for test input.");
    }
    else {
	/* prepare to read sequence of float vectors */
	REQUIRE(FLOAT == get_fea_type(fieldname,test_hd),
		"Data type must be FLOAT.");
	fptr = (float *) get_fea_ptr(test_rec, fieldname, test_hd);
	REQUIRE(fptr != NULL,
		"Unable to get specified field from test file.");
	test_seq_v =
	    (float **) f_mat_alloc((unsigned)test_ndrec, (unsigned)dim);
	REQUIRE(test_seq_v != NULL,
		"Can't allocate memory for test input.");
    }
    j=0;
    while( get_fea_rec(test_rec, test_hd, test_fp) != EOF ) {
	REQUIRE(j<test_ndrec,
		"more records in test file than specified in header.");
	if (cflag)
	    test_seq_l[j] = *lptr;
	else
	    for (i=0; i<dim; i++)
		test_seq_v[j][i] = fptr[i];
	j++;
    }
    (void)fclose(test_fp);
    free_fea_rec(test_rec);
    if (debug_level)
	Fprintf(stderr, "%s: Test: %s  records: %5d\n",
		PROG, test_fname, test_ndrec);

    if (results_flag) {
      mapping = (long *) calloc( (unsigned) test_ndrec, sizeof(long));
      REQUIRE(mapping!=NULL, "can't allocate mapping array.");
    }

    if (debug_level) {
	if (best_so_far_flag)
	    Fprintf(stderr,"%s: best_so_far: %f\n", PROG, best_so_far);
	else
	    Fprintf(stderr,"%s: best_so_far not set.\n", PROG);
	Fprintf(stderr, "%s: delta : %d.\n", PROG, delta);
    }

    if (debug_level>1)
	Fprintf(stderr, "%s: Calling dtw routines.\n", PROG);
    dptr = (best_so_far_flag) ? &best_so_far : (double *)NULL;
    if (cflag)
	dist = dtw_tl(test_seq_l, ref_seq_l, test_ndrec, ref_ndrec,
		      (long) cbk_dim, delta, dptr, mapping, distances);
    else
	dist = dtw_l2(test_seq_v, ref_seq_v, test_ndrec, ref_ndrec,
		      (long) dim, delta, dptr, mapping, NULL);

    if (debug_level)
      Fprintf(stderr, "%s: %s .vs. %s: Distance = %e\n",
	      PROG, test_fname, ref_fname, dist);
    else
      Fprintf(stdout, "%e\n", dist);

    if (results_flag && best_so_far_flag && dist == best_so_far) {
	/* intermediate distance exceeds best so far value;
	 * don't write results */
	if (debug_level)
	  Fprintf(stderr, "%s: Not writing results file.\n", PROG);
	results_flag = 0;
    }

    if (results_flag) {
      if (debug_level>1)
	  Fprintf(stderr, "Writing reference file %s.\n", res_fname);
      fclose(ref_fp);
      ref_fname = eopen(PROG, ref_fname, "r", FT_FEA, NONE, &ref_hd, &ref_fp);
      ref_rec = allo_fea_rec(ref_hd);
      res_fname = eopen(PROG, res_fname, "w", NONE, NONE, &res_hd, &res_fp);
      res_hd = (struct header *) copy_header(ref_hd);
      add_source_file(res_hd, test_fname, test_hd);
      add_source_file(res_hd, ref_fname,  ref_hd);
      res_hd->common.tag = YES;
      (void) strcpy(res_hd->common.prog, PROG);
      (void) strcpy(res_hd->common.vers, VERSION);
      (void) strcpy(res_hd->common.progdate, DATE);
      add_comment(res_hd, get_cmd_line(argc, argv));
      *add_genhd_d("dtw_best_so_far",
		   (double *)NULL, 1, res_hd) = best_so_far;
      *add_genhd_l("dtw_delta", (long *)NULL, 1, res_hd) = delta;
      *add_genhd_d("dtw_result", (double *)NULL, 1, res_hd) = dist;
      (void) add_genhd_c("dtw_sequence_field", fieldname, 0, res_hd);
      if (cflag) {
	  (void) add_genhd_c("distance_table_file", tbl_fname, 0, res_hd);
	  (void) add_genhd_c("distance_table_field", tbl_fieldname, 0, res_hd);
	  *add_genhd_c("distance_table_recno",
		       (long *)NULL, 1, res_hd) = tbl_recno;
      }
      (void) add_fea_fld("dtw_distance", (long)1, (short)1,
			 (long *)NULL, DOUBLE, (char **)NULL, res_hd);
      (void) add_fea_fld("mapping", (long)1, (short)1,
			 (long *)NULL, LONG, (char **)NULL, res_hd);
      write_header(res_hd, res_fp);
      res_rec = allo_fea_rec(res_hd);
      REQUIRE(res_rec != NULL, "Can't allocate results record.");
      dist_ptr = (double *) get_fea_ptr( res_rec, "dtw_distance", res_hd);
      REQUIRE(dist_ptr!=NULL, "Can't allocate results record.");
      map_ptr = (long *) get_fea_ptr( res_rec, "mapping", res_hd);
      REQUIRE(map_ptr!=NULL, "Can't allocate results record.");

      for (j=0; j<ref_ndrec; j++) {
	  REQUIRE(get_fea_rec(ref_rec, ref_hd, ref_fp) != EOF,
		  "error copying reference data to results file.");
	  for (k=0; k<test_ndrec; k++) {
	      if ( j == mapping[k] ) {
		  copy_fea_rec(ref_rec, ref_hd, res_rec,
			       res_hd, (char **)NULL, (short **)NULL);
		  if (cflag)
		    *dist_ptr = table_lookup(ref_seq_l[j], test_seq_l[k],
					     (long)cbk_dim, distances);
		  else
		    *dist_ptr = euclidean_dist(ref_seq_v[j],
					       test_seq_v[k], dim);
		  *map_ptr = j+1;
		  put_fea_rec( res_rec, res_hd, res_fp);
		  if (debug_level>2)
		    fprintf(stderr,
			    "test rec:  %d  ref rec: %d  distance %e \n",
			    k+1, mapping[k]+1, *dist_ptr);
	      }
	  }
      }
      (void)fclose(res_fp);
      (void)fclose(ref_fp);
      update_waves_gen(res_hd, ref_hd, (float) mapping[0], (float) 0.0);
    }

    exit(0);

} /* end main */

double euclidean_dist(v1, v2, dim)
float *v1, *v2;
long dim;
{
  double sqrt();

  int j;
  double result, dummy;
  REQUIRE(v1 != NULL && v2 != NULL,
	  "euclidean distance called with null pointer(s).");
  REQUIRE(dim > 0, "euclidean distance called with dimension 0.");
  for (j=0, result=0.0; j<dim; j++) {
    dummy = v1[j] - v2[j];
    result += dummy*dummy;
  }
  result = sqrt( result );

  return(result);
}

double table_lookup(v1, v2, dim, table)
long v1, v2;
long dim;
double **table;
{
  REQUIRE(0<=v1 && v1<dim && 0<=v2 && v2<dim,
	  "integer in sequence outside range [0,dim-1].");
  REQUIRE(table != NULL, "table lookup called with null table.");

  return(table[v1][v2]);
}


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
