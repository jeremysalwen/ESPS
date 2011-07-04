/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *     "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 * stat.c - process stat_output in fea_stats(1-ESPS)
 * 
 * Author: Ajaipal S. Virdy, Entropic Speech, Inc.
 */

#ifdef SCCS
    static char *sccsid = "@(#)stat.c	1.13	7/8/96	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <assert.h>
#include <esps/spsassert.h>
#include <esps/feastat.h>

#include "global.h"
#include "stat.h"

#define REALLOC(typ,num,var) { \
    if ((var) == NULL) \
	(var) = (typ *) emalloc((unsigned) (num) * sizeof(typ)); \
    else (var) = (typ *) realloc((char *) (var), \
				    (unsigned) (num) * sizeof(typ)); \
    spsassert((var), "can't allocate memory."); \
}

char *emalloc();
/* SDI 26/6/06 added LINUX_OR_MAC to avoid compiler error */
#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char  *ecalloc(), *calloc(), *realloc();
#endif
/*
 * S P S
 *  F U N C T I O N S
 *   R E F E R E N C E D
 */

double	matrix_inv_d();
int	lin_search2();
char	*get_fea_ptr();
float	**f_mat_alloc();
double	**d_mat_alloc();
char	*savestring();

char	template[20] = "/usr/tmp/fsXXXXXX";
void
Process_stat_output()
{

    float	*mean;
    double	**covarmat;
    double	**invcovar;

/*
 * The -E option has not been implemented yet, so don't declare these
 * variables:
 *
 *  float	*eigenval;
 *  float	**eigenvec;
 *
 */

    int		i_rec;

    int		i, j, k;	/* indexing variables for arrays */
    int		ptr;		/* pointer to fields in a FEA file record */


    double	*d_ptr; 	/* pointer to a double */
    float	*f_ptr; 	/* pointer to a float */
    long	*l_ptr; 	/* pointer to a long */
    short	*s_ptr; 	/* pointer to a short */
    char	*b_ptr; 	/* pointer to a short */

    double	value;		/* temporary variables */

    int		n_class = 0;	/* number of classifications */
    short	class;
    char	**classification = NULL;

    char	**s;

    struct
    feastat	*stat_rec;	/* FEA_STAT file record pointer */

    struct
    header	*tmp_oh;	/* temporary file header */

    struct
    feastat	*tmp_stat_rec;	/* Temporary FEA_STAT file record pointer */

    FILE	*tstrm; 	/* temporary file pointer */


    int	add_covar = NO;	     /* add COVARIANCE field if flag set */
    int	add_invcovar = NO;   /* add INVERSE COVARIANCE field if flag set */
    int	add_eigen = NO;	     /* add EIGENVALUES, EIGENVECTORS if flag set */

    int	covar_exists = NO;    /* does COVARIANCE field exist? */
    int	invcovar_exists = NO; /* does INVERSE COVARIANCE field exist? */
    int	eigen_exists = NO;    /* does EIGENVALUES, EIGENVECTORS field exist? */


    char	*Module = "stat_out";

    if (debug_level > 9) {
	Fprintf (stderr, "Version = %s, Date = %s\n", Version, Date);
	Fprintf (stderr, "command_line = %s\n", command_line);
        (void) fflush (stderr);
    }

    if (Mflag) {
	if (debug_level > 1)
	    Fprintf (stderr,
	    "%s: Computing Mean Vector ...\n", Module);
    }

    ptr = stat_field[0];

    statsize = n_items[0];

    if (debug_level > 1)
	Fprintf (stderr,
	"%s: allocating memory: statsize = %ld\n", Module, statsize);

    if (Mflag)
	if ( (mean = (float *) ecalloc((unsigned) statsize, sizeof(float)))
	     == NULL )
	{
	    Fprintf (stderr,
		"%s: calloc: could not allocate memory for mean.\n",
		Module);
	    exit(1);
	}

    if (Cflag || Iflag)
	if (( covarmat = (double **) d_mat_alloc((unsigned) statsize,
		    (unsigned) statsize) ) == NULL)
	{
	    Fprintf (stderr,
		"%s: d_mat_alloc: could not allocate memory for covarmat.\n",
		Module);
	    exit(1);
	}

    if (Iflag){
      if( n_rec < statsize ){
	Fprintf(stderr, "ERROR: %s: Number of sample vector (%d) < number of features (%d).\n\tWill get singular covariance matrix.\n\tCan NOT do inverse covariance matrix.\n\tNeed at least %d linearly independent sample vectors -- exiting\n", Module, n_rec, statsize, statsize);
	if(update_file == NO) unlink(outfile);
	exit(1);
      }
      if (( invcovar = (double **) d_mat_alloc((unsigned) statsize,
					      (unsigned) statsize) ) == NULL)
	{
	  Fprintf (stderr, "%s: d_mat_alloc: invcovar malloc failed.\n", 
		   Module);
	  exit(1);
	}
    }

    /*
     * if (stat_out && !Aflag) then
     *    memory for Data has not been allocated, nor data stored.
     */

    if (!Aflag)
	if (( Data = (float **) f_mat_alloc((unsigned) n_rec,
		   (unsigned) statsize) ) == NULL)
	{
	    Fprintf (stderr,
		"%s: f_mat_alloc: could not allocate memory for Data.\n",
		Module);
	    exit(1);
	}

    for (k = 0; k < statsize; k++)
	mean[k] = 0.0;

    for (i_rec = 0; i_rec < n_rec; i_rec++) {

	if (debug_level > 8)
	    Fprintf (stderr,
		"\t%s: getting record number %d.\n", Module, s_rec + i_rec);

	if (get_fea_rec (fea_rec, esps_hdr, instrm) == EOF) {
	    Fprintf (stderr,
		"%s: ran out of data after record number %d.\n",
		Module, s_rec + i_rec);
	    exit(1);
	}

       /*
	* Position the pointers to the correct places.
	*/

	switch (fea_hdr->types[ptr]) {
	    case DOUBLE: d_ptr = (double *) get_fea_ptr (fea_rec,
			fea_hdr->names[ptr], esps_hdr);
			break;
	    case FLOAT: f_ptr = (float *) get_fea_ptr (fea_rec,
			fea_hdr->names[ptr], esps_hdr);
			break;
	    case LONG:  l_ptr = (long *) get_fea_ptr (fea_rec,
			fea_hdr->names[ptr], esps_hdr);
			break;
	    case SHORT: s_ptr = (short *) get_fea_ptr (fea_rec,
			fea_hdr->names[ptr], esps_hdr);
			break;
	    case BYTE: b_ptr = (char *) get_fea_ptr (fea_rec,
			fea_hdr->names[ptr], esps_hdr);
			break;
	    default:    Fprintf(stderr,
			    "%s: incorrect data type.\n", Module);
	}

	for (k = 0; k < statsize; k++) {

	    switch (fea_hdr->types[ptr])
	    {
	    case DOUBLE:
		value = (float) *(d_ptr + item_arrays[0][k]);
		break;
	    case FLOAT:
		value = *(f_ptr + item_arrays[0][k]);
		break;
	    case LONG:
		value = (float) *(l_ptr + item_arrays[0][k]);
		break;
	    case SHORT:
		value = (float) *(s_ptr + item_arrays[0][k]);
		break;
	    case BYTE:
		value = (float) *(b_ptr + item_arrays[0][k]);
		break;
	    default:
		Fprintf(stderr, "%s: incorrect data type.\n", Module);
	    }

	    mean[k] += value / n_rec;

	    if (!Aflag)
		Data[i_rec][k] = value;

	    if (debug_level > 20)
	    {
		Fprintf (stderr, "\n\trecord #%d, k = %d, value = %g\n",
		    s_rec + i_rec, k, value);
		Fprintf (stderr, "\tmean[%d] = %g\n",
		    k, mean[k]);
	    }

	}  /* for (k = 0; k < n_item; k++) */

    }  /* end for (i_rec = 0; i_rec < n_rec; i_rec++) */

    if (debug_level > 20)
	for (k = 0; k < statsize; k++)
	    Fprintf (stderr, "mean[%d] = %g\n", k, mean[k]);

    if (debug_level > 2)
	Fprintf (stderr, "%s: Update or create output FEA_STAT file.\n",
	    Module);

    /*
     * Update FEA_STAT file or Create output FEA_STAT file.
     */

    if (update_file) {

	if (debug_level > 1) {
	    Fprintf (stderr,
	    "%s: Updating file %s.\n", Module, outfile);
	    (void) fflush (stderr);
	}

	if (debug_level > 8) {
	    Fprintf (stderr,
	    "%s: checking -{MICE} options.\n", Module);
	    (void) fflush (stderr);
	}

	stat_rec = allo_feastat_rec (fea_oh);

       /*
	* If an option field does not exist in the existing FEA_STAT
	* file, then create that optional field if and only if the
	* corresponding flag was given on the command line.
	* For example, if the existing FEA_STAT file only contains the
	* mean vector and the user requested the -C option, then the
	* covariance field must be created in the updated FEA_STAT
	* file.  In the context of this program, set add_covar equal to
	* the value of Cflag if the covariance field does not exist,
	* otherwise set add_covar to YES.  add_covar will then be used
	* in the call to init_feastat_hd to allocate the right amount
	* of memory.
	*/

	if (get_fea_ptr (stat_rec->fea_rec, "covar", fea_oh) == NULL) {
	    add_covar = Cflag;
	    covar_exists = NO;
	} else {
	    add_covar = YES;
	    covar_exists = YES;
	}


	if (get_fea_ptr(stat_rec->fea_rec, "invcovar", fea_oh) == NULL) {
	    add_invcovar = Iflag;
	    invcovar_exists = NO;
	} else {
	    add_invcovar = YES;
	    invcovar_exists = YES;
	}

	if (get_fea_ptr(stat_rec->fea_rec, "eigenvec", fea_oh) == NULL) {
	    add_eigen = Eflag;
	    eigen_exists = NO;
	} else {
	    add_eigen = YES;
	    eigen_exists = YES;
	}

       /* Open a temporary file for writing out data. */

	if ((tstrm = fopen (mktemp (template), "w+")) == NULL)
	    CANTOPEN (Module, template);

       /* Create new header for our updated FEA_STAT file */

	tmp_oh = new_header (FT_FEA);
	assert (tmp_oh);

       /*
	* Get all possible classifications from the header of the
	* the existing FEA_STAT file (feaoh).
	*/

	fea_hdr = fea_oh->hd.fea;

	if (debug_level > 10)
	    Fprintf (stderr, 
		"%s: number of feature fields in %s = %d.\n",
		Module, outfile, fea_hdr->field_count);

	i = lin_search2(fea_hdr->names, "class");
	s = fea_hdr->enums[i];

	if (debug_level > 10)
	    Fprintf (stderr, 
		"\t%s: *s = *fea_hdr->enums[%d] = %s.\n",
		Module, i, *s);

	while (*s != NULL) {
	    n_class++;
	    REALLOC(char *, n_class, classification)
	    classification[n_class-1] = savestring (*s);
	    assert (classification[n_class-1]);

	    if (debug_level > 10)
		Fprintf (stderr, 
		    "\t%s: fea_hdr->enums[%d] = %s, n_class = %d.\n",
		    Module, i, *s, n_class);

	    s++;
	}

       /*
	* If the class name which was specified on the command line
	* with the -n option is not present in the enums array, add
	* it as the last non-NULL element.  This way
	* the stat_rec->class values do not need to be modifed.  What
	* we are basically doing is adding the new classification names
	* to the end of the stack.
	*/

	if ((class = lin_search2(fea_hdr->enums[i], class_name)) == -1)
	{
	    n_class++;
	    REALLOC(char *, n_class, classification)
	    classification[n_class-1] = savestring (class_name);
	    class = n_class - 1;
	}

       /* Assign NULL as the last element of the classification array */

	n_class++;
	REALLOC(char *, n_class, classification)
	classification[n_class-1] = NULL;

	if (debug_level > 8)
	    for (i = 0; i < n_class - 1; i++)
		Fprintf (stderr,
		    "%s: classification[%d] = %s\n",
		    Module, i,  classification[i]);

       /* Initialize the new header for the existing FEA_STAT file. */

	if ( init_feastat_hd(tmp_oh, statfield, statsize,
		classification, add_covar, add_invcovar, add_eigen) != 0 )
	{
	    Fprintf (stderr,
		"%s: init_feastat_hd: could not initialize FEA_STAT header.\n",
		Module);
	    exit (1);
	}

	tmp_oh->common.tag = NO;
	tmp_oh->common.ndrec = fea_oh->common.ndrec + 1;
	tmp_oh->variable.refhd = fea_oh->variable.refhd;

	if (debug_level > 10)
	    Fprintf (stderr, 
		"%s: number of data records in FEA_STAT file = %d.\n",
		Module, fea_oh->common.ndrec);

	(void) strcpy (tmp_oh->common.prog, ProgName);
	(void) strcpy (tmp_oh->common.vers, Version);
	(void) strcpy (tmp_oh->common.progdate, Date);

	add_comment (tmp_oh, fea_oh->variable.comment);
	add_comment (tmp_oh, command_line);

	for (i = 0; i < fea_oh->variable.nnames; i++)
	    tmp_oh->variable.source[i] = fea_oh->variable.source[i];
	tmp_oh->variable.nnames = fea_oh->variable.nnames;
	for (i = 0; i < fea_oh->variable.nheads; i++)
	    tmp_oh->variable.srchead[i] = fea_oh->variable.srchead[i];
	tmp_oh->variable.nheads = fea_oh->variable.nheads;
	add_source_file(tmp_oh, infile, esps_hdr);

	write_header(tmp_oh, tstrm);

       /*
	* stat_rec points to the new header which is given by tmp_oh
	* and tmp_stat_rec points to the old header which is given by
	* fea_oh.
	*/

	stat_rec = allo_feastat_rec (tmp_oh);
	tmp_stat_rec = allo_feastat_rec (fea_oh);

	for (i = 0; i < fea_oh->common.ndrec; i++) {
	    if (get_feastat_rec (tmp_stat_rec, fea_oh, outstrm) == EOF) {
		Fprintf (stderr,
		"%s: premature EOF on %s.\n", Module, outfile);
		exit (1);
	    }
	    if (debug_level > 10) {
		Fprintf (stderr, "Record %d ", i + 1);
		print_fea_rec (tmp_stat_rec->fea_rec,  fea_oh, stderr);
	    }

	    /*
	     * If any of the fields had to be created, signified by the
	     * value of add_xxx, then we must use stat_rec for writing
	     * out the data.  Also the mean vector must be
	     * transferred over.
	     */

	    if (add_covar || add_invcovar || add_eigen) {
		*stat_rec->class = *tmp_stat_rec->class;
		*stat_rec->nsamp = *tmp_stat_rec->nsamp;
		for (j = 0; j < statsize; j++) {
		    stat_rec->mean[j] = tmp_stat_rec->mean[j];
		    if (eigen_exists)
			stat_rec->eigenval[j] = tmp_stat_rec->eigenval[j];
		    for (k = 0; k < statsize; k++) {
			if (covar_exists)
			    stat_rec->covar[j][k] =
					    tmp_stat_rec->covar[j][k];
			if (invcovar_exists)
			    stat_rec->invcovar[j][k] =
					    tmp_stat_rec->invcovar[j][k];
			if (eigen_exists)
			    stat_rec->eigenvec[j][k] =
					    tmp_stat_rec->eigenvec[j][k];
		    }
		}
		put_feastat_rec (stat_rec, tmp_oh, tstrm);
	    } else
		put_feastat_rec (tmp_stat_rec, fea_oh, tstrm);
	}

       /* Flush stat_rec buffer */

	stat_rec = allo_feastat_rec (tmp_oh);

       /* stat_rec is pointing to the correct place now */

    } else {

       /* Create new FEA_STAT file, don't update outfile  */

	if (debug_level > 1) {
	    Fprintf (stderr,
	    "%s: Create new FEA_STAT file %s.\n", Module, outfile);
	    (void) fflush (stderr);
	}

	if (debug_level > 4) {
	    Fprintf (stderr, "%s: Check Classifications:\n",
	    Module);
	    (void) fflush (stderr);
	}

       /*
	* Since we are creating a new FEA_STAT file, there can only be
	* one classification type.  This is given by class_name which
	* was specified on the command line.
	*/

	n_class++;
	REALLOC(char *, n_class, classification)
	classification[n_class-1] = savestring (class_name);
	class = n_class - 1;

       /* Assign NULL as the last element of the classification array */

	n_class++;
	REALLOC(char *, n_class, classification)
	classification[n_class-1] = NULL;

	if (debug_level > 4)
	    for (i = 0; i < n_class; i++)
		Fprintf (stderr,
		"\t%s: classification[%d] = %s\n",
		Module, i,  classification[i]);

	if (debug_level > 4) {
	    Fprintf (stderr, "%s: Initializing FEA_STAT file %s\n",
		Module, outfile);
	    (void) fflush (stderr);
	}

       /* Initialize the new FEA_STAT header */

	if (init_feastat_hd (fea_oh, statfield, statsize,
	    classification, Cflag, Iflag, Eflag) != 0) {
	    Fprintf (stderr,
		"%s: init_feastat_hd: could not initialize FEA_STAT header.\n",
		Module);
	    exit (1);
	}

	fea_oh->common.tag = NO;
	fea_oh->common.ndrec = 1;	    /* only one data record */
	fea_oh->variable.refhd = esps_hdr;

	if (debug_level > 4) {
	    Fprintf (stderr, "%s: Initializing common part of header.\n",
		Module);
	    (void) fflush (stderr);
	}

	(void) strcpy (fea_oh->common.prog, ProgName);
	(void) strcpy (fea_oh->common.vers, Version);
	(void) strcpy (fea_oh->common.progdate, Date);

	add_comment(fea_oh, command_line);

	/* Hack around write_header bug that zaps refhd embedded headers in
	 * memory, and thus clobbers the source header when that is also
	 * the reference header.  We make a separate copy by writing to a
	 * temp file and reading back.  Just remove the next several lines
	 * when writeheader is fixed.
	 */

	if ((tstrm = fopen (mktemp (template), "w+")) == NULL)
	    CANTOPEN (Module, template);
	write_header(esps_hdr, tstrm);
        rewind (tstrm);
	esps_hdr = read_header(tstrm);
        (void) fclose (tstrm);
        (void) unlink (template);

	/* End of hack. */

	add_source_file(fea_oh, infile, esps_hdr);

	if (debug_level > 4) {
	    Fprintf (stderr, "%s: Writing output header for %s.\n",
		Module, outfile);
	    (void) fflush (stderr);
	}

	write_header (fea_oh, outstrm);

	stat_rec = allo_feastat_rec (fea_oh);

    }  /* end if (update_file) */


    if (Cflag || Iflag) {

	if (debug_level > 2)
	    Fprintf (stderr,
		"%s: Computing %sCovariance Matrix ...\n",
		Module, (Iflag != 0) ? "Inverse " : "");

	for (j = 0; j < statsize; j++)
	    for (k = 0; k < statsize; k++)
	    covarmat[j][k] = 0.0;

	for (i_rec = 0; i_rec < n_rec; i_rec++) {

	    ptr = stat_field[0];	/* only one field name can be given */

	    for (j = 0; j < statsize; j++) {

		for (k = 0; k < statsize; k++) {

		    if (k >= j)
			covarmat[j][k] += (Data[i_rec][k] - mean[k]) *
			    (Data[i_rec][j] - mean[j]) / (n_rec - 1);
		    else
			covarmat[j][k] = covarmat[k][j];

		    if (debug_level > 50) {
			Fprintf (stderr,
			    "\n\trecord #%d, j = %d, k = %d, i_rec = %d\n",
			    s_rec + i_rec, j, k, i_rec);
			Fprintf (stderr,
			    "\tData[i_rec:%d][j:%d] = %g, Data[i_rec:%d][k:%d] = %g\n",
			    i_rec, j, Data[i_rec][j], i_rec, k, Data[i_rec][k]);
			Fprintf (stderr,
			    "\tcovar[%d][%d] = %g\n",
			    j, k, covarmat[j][k]);
		    }

		}  /* end for (k = 0; k < statsize; k++) */

	    }  /* end for (j = 0; j < statsize; j++) */

	}  /* end for (i_rec = 0; i_rec < n_rec; i_rec++) */

    }  /* end if (Cflag || Iflag) */

    if (debug_level > 20) {
	Fprintf (stderr, "\n C O V A R I A N C E    M A T R I X\n");
	Fprintf (stderr, "\n *******************    ***********\n");
	Fprintf (stderr, "\n");
	for (j = 0; j < statsize; j++) {
	    for (k = 0; k < statsize; k++)
		Fprintf (stderr, "(%d,%d): %g   ", j+1, k+1, covarmat[j][k]);
	    Fprintf (stderr, "\n");
	}
    }

    if (debug_level > 2)
	Fprintf (stderr, "%s: Writing results into FEA_STAT record.\n",
	    Module);

    *stat_rec->class = class;
    *stat_rec->nsamp = n_rec;

    if (debug_level > 5)
	Fprintf (stderr, "%s: n_class = %d, *stat_rec->class = %d.\n",
	    Module, n_class, *stat_rec->class);

    if (Iflag)
    {
	int	row, col;

	if (debug_level > 8)
	    for (row = 0; row < statsize; row++) {
		for (col = 0; col < statsize; col++)
		    Fprintf (stderr,
			"C(%d,%d): %4.2f   ", row, col, covarmat[row][col]);
		Fprintf(stderr, "\n");
	    }

	if (matrix_inv_d(covarmat, invcovar, (int) statsize)==-1.0) {
	    Fprintf (stderr,
		"\nERROR: %s: Covariance matrix may be singular to working precision. -- exiting.\n", Module);
	    if(update_file == NO) unlink(outfile);
	    exit(1);
	}

	if (debug_level > 8)
	    for (row = 0; row < statsize; row++) {
		for (col = 0; col < statsize; col++)
		    Fprintf (stderr,
			"I(%d,%d): %4.2f   ", col, row, invcovar[col][row]);
		Fprintf(stderr, "\n");
	    }

    }  /* end if (Iflag) */

    for (k = 0; k < statsize; k++) {
	stat_rec->mean[k] = mean[k];
	if (Eflag)
	    stat_rec->eigenval[k] = 0.0;
	for (j = 0; j < statsize; j++) {
	    if (Cflag)
		stat_rec->covar[j][k] = covarmat[j][k];
	    if (Iflag)
		stat_rec->invcovar[j][k] = invcovar[j][k];
	    if (Eflag)
		stat_rec->eigenvec[j][k] = 0.0;
	}
    }

    if (debug_level > 2)
	Fprintf (stderr, "%s: Putting FEA_STAT results in %s.\n",
	    Module, outfile);

    /*
     * If the FEA_STAT was updated, then write results into the
     * temporary file, otherwise, write them directly to the output
     * FEA_STAT file.
     */

    if (update_file)
	put_feastat_rec (stat_rec, tmp_oh, tstrm);
    else
	put_feastat_rec (stat_rec, fea_oh, outstrm);


    if (update_file) {

	/*
	 * If the FEA_STAT file had to be updated, the results must be
	 * copied back into our feature file from the temporary file.  So
	 * rewind the temporary file and copy data back into the FEA_STAT
	 * file.
	 */

        rewind (tstrm);

	if ((fea_oh = read_header (tstrm)) == NULL)
	    NOTSPS (Module, template);

	/*
	 * Everything has been stored for updating original file.
	 * Erase original file for updating.
	 */

	TRYOPEN (Module, outfile, "w", outstrm);

	fea_oh->variable.refhd = esps_hdr;

	write_header (fea_oh, outstrm);

	fea_rec = allo_fea_rec (fea_oh);

	if (debug_level > 10)
	    Fprintf (stderr, 
		"%s: number of data records in temporary file %s = %d.\n",
		Module, template, fea_oh->common.ndrec);

	for (i = 0; i < fea_oh->common.ndrec; i++) {
	    if (get_fea_rec (fea_rec, fea_oh, tstrm) == EOF) {
		Fprintf (stderr,
		    "%s: premature EOF on %s.\n", Module, template);
		exit(1);
	    }
	    if (debug_level > 10) {
		Fprintf (stderr, "Record %d ", i + 1);
		print_fea_rec (fea_rec,  fea_oh, stderr);
	    }

	    put_fea_rec (fea_rec, fea_oh, outstrm);
	}

        (void) fclose(outstrm);
        (void) fclose(tstrm);
        (void) unlink(template);

    }  /* end if (update_file) */

}
