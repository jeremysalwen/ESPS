/* spec_distort - compute distortion measures between two ESPS SPEC files
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Module Name:  spec_distort
 *
 * Written By:  Ajaipal S. Virdy
 *
 *
 * Purpose:  compute distortion measure for speech processes
 *
 *
 */

#ifdef SCCS
	static char *sccs_id = "%W%	%G%	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/feaspec.h>
#include <math.h>
#include <esps/unix.h>

/* declare external variables for modules in distort */

#include  "distort.h"

/*
 *   G L O B A L    V A R I A B L E S
 *
 *   used in spec_distort and print_distort.
 *
 */

char	**frame;	/* string for "Voiced" or "Unvoiced" frames */

int	*freq_len;	/* length of array re_spec_val, im_spec_val,
			 * and frqs_len for each record */

double	*SpecPower1;	/* Spectral Powers */
double	*SpecPower2;	/* Spectral Powers */

float	**diff_real;	/* difference between real part of spectral value */
float	**diff_imag;	/* difference between imag. part of spectral value */
float	**diff_frqs;	/* difference between frequencies of spectral value */

float	tot_real = 0.0;	/* variable to sum all re_spec_val */
float	tot_imag = 0.0;	/* variable to sum all im_spec_val */
float	tot_frqs = 0.0;	/* variable to sum all frqs */

double	*IS_dist;	/* Itakura-Saito (IS) distortion measure vector */
double	*PNIS_dist;	/* Power-Normalized IS distortion measure vector */

#define	ERROR  1.0e-05	/* anything less than this is zero */

int	freq_format;
int	spec_type;
int	num_of_freqs;

int	i_rec;		/* current record */
int	rec_num;	/* current record number */
int	ele_num;	/* current element number */

short   *voicing1,
        *voicing2;       /*holds voicing info, if it exists*/

#define	MAG(A,B)	( sqrt ((A)*(A) + (B)*(B)) )

/*
 * U N I X
 *  R O U T I N E S
 *   R E F E R E N C E D
 */


double	log();
double	pow();
double	sqrt();

/*
 * E S P S
 *  R O U T I N E S
 *   R E F E R E N C E D
 */

short	get_rec_len();

/*
 * L O C A L
 *  R O U T I N E S
 *   R E F E R E N C E D
 */

void	gen_distort();
static char	*check_frames();
static void	allo_memory();


void spec_distort()
{
   struct feaspec	*spec_rec1;	/* data structure for file1 */
   struct feaspec 	*spec_rec2;	/* data structure for file2 */

/*   struct spec_header	*spec1 = f1h->hd.spec;
   struct spec_header	*spec2 = f2h->hd.spec;*/

   int	rows;		/* size of row to allocate */
   int	columns;	/* size of column to allocate */

   int	j;		/* temporary indices */

/*
 * Accessed:	char *spec_type_codes[]; from <esps/feaspec.h>
 *
 */

/*
 *  If the -e option was given then treat the FEASPEC file as a
 *  "generic" file.
 *
 */

   if ( eflag ) {
      (void) gen_distort();
      return;
   }

/*
 * Save freq_format and spec_type of the spectral record file.
 * The following should be the same in both files;  they have already
 * been checked in check_spec ();
 *
 */

   freq_format = *get_genhd_s("freq_format", f1h);
   spec_type = *get_genhd_s("spec_type", f1h);
   num_of_freqs = get_genhd_val("num_freqs", f1h, (double)0.);/* maximum number of freqs */


   if ( debug_level > 3)
      (void) fprintf (stderr,
      "\n");

   if ( get_rec_len (f1h) != get_rec_len (f2h) ) {
      (void) fprintf (stderr,
      "spec_distort: warning, elements per record not equal in both files.\n");
    } else
	if (debug_level > 4)
	   (void) fprintf (stderr,
	   "spec_distort: same number of elements in these SPEC files.\n");

   /* skip appropiate number of records (as specified by the -f option) */

   spec_rec1 = allo_feaspec_rec (f1h, FLOAT);
   spec_rec2 = allo_feaspec_rec (f2h, FLOAT);

   if (debug_level > 1)
      (void) fprintf (stderr,
      "spec_distort: skipping %d records.\n", s_rec - 1);

/* already skipped appropriate number of records */

/*
 * This is the way I originally skipped records:
 *
 *   for ( rec_num = 1; rec_num < s_rec; rec_num++ ) {
 *
 *       if ( get_spec_rec (spec_rec1, f1h, f1p) == EOF ) {
 *	  (void) fprintf (stderr,
 *	  "spec_distort: not enough records in %s.\n", f1str);
 *	  exit (1);
 *       }
 *       if ( get_spec_rec (spec_rec2, f2h, f2p) == EOF ) {
 *	  (void) fprintf (stderr,
 *	  "spec_distort: not enough records in %s.\n", f2str);
 *	  exit (1);
 *       }
 *
 *   } \* end for *\
 */

   if (get_genhd_val("order_vcd", f1h, (double)0.) !=
                    get_genhd_val("order_vcd", f2h, (double)0.)){
	   (void) fprintf (stderr,
	   "spec_distort: warning, order_vcd not equal in %s and %s.\n",
	   f1str, f2str);
	}

   if (get_genhd_val("order_unvcd", f1h, (double)0.) !=
                    get_genhd_val("order_unvcd", f2h, (double)0.)){
	   (void) fprintf (stderr,
	   "spec_distort: warning, order_unvcd not equal in %s and %s.\n",
	   f1str, f2str);
	}

   (void)fflush (stderr);

/*
 * The following variables are used to compute the size of
 * our multi-dimensional arrays
 */

/*
 * Now allocate:
 *
 *   number of records (rows)
 * 	    by
 *   number of frequencies (num_of_freqs) [columns]
 *
 * memory space to store all
 * differences between elements in each record for computing the Element
 * Average, Record Average, and File Average.
 *
 */

   rows = (e_rec - s_rec + 1);	/* number of rows to allocate */

   columns = num_of_freqs;

   (void) allo_memory (rows, columns);	/* allocate memory for all arrays */

/*
 * Here is the big loop for ESPS SPEC files.
 * Note: we have to check for consistency
 *       each time through the loop.
 */


   if (debug_level > 4)
      (void) fprintf (stderr,
      "spec_distort: for (rec_num = %d; rec_num <= %d; rec_num++) {\n",
      s_rec, e_rec);

   (void)fflush (stderr);

/*
 *
 *  M A I N    L O O P:
 *
 */

   for ( rec_num = s_rec, i_rec = 0; rec_num <= e_rec; rec_num++, i_rec++ ) {

     if (debug_level > 4)
	(void) fprintf (stderr,
	"\n  spec_distort: getting record no. %d.\n", rec_num);

     if( get_feaspec_rec (spec_rec1, f1h, f1p) == EOF){
	 Fprintf(stderr,
	    "distort: specdistort: Hit end of file in input file 1\n");
     }
     if( get_feaspec_rec (spec_rec2, f2h, f2p) == EOF){
	 Fprintf(stderr,
	    "distort: specdistort: Hit end of file in input file 2\n");
     }


     /* check for consistency in file1 and file2 */

     if (debug_level > 4)
	(void) fprintf (stderr,
	"  spec_distort: checking for consistency in this record.\n");

     frame[i_rec] = check_frames (spec_rec1, spec_rec2);

/*
 * Compare the lengths of all the arrays in feaspec data structure
 * (i.e. the following arrays must be of the same size in each record,
 *       although the lengths can vary among different records).
 *
 */

      if (freq_format == ARB_VAR) {
	 if (*spec_rec1->n_frqs != *spec_rec2->n_frqs) {
	    (void) fprintf (stderr,
	    "spec_dist: num of freqs in record number %d not equal\n",
	    rec_num);
	    exit (1);
	 }
	 else
	    num_of_freqs = *spec_rec1->n_frqs;
      }
     else
	 num_of_freqs = get_genhd_val("num_freqs", f1h, (double)0.);

/*
 * Determine the maximum element number for this record
 *
 */

      e_ele = freq_len[i_rec] = num_of_freqs;


/*
 * Now begin to compute:
 *   Difference, Difference Magnitude, and Difference Magnitude Squared.
 */

	if (debug_level > 4) {
	   (void) fprintf (stderr, "\n\n E L E M E N T     L O O P :\n\n");
	   (void) fprintf (stderr,
	   "  spec_distort: for (ele_num = %d, j = 0;\n", s_ele - 1);
	   (void) fprintf (stderr,
	   "                      ele_num < %d; ele_num++,j++) {\n",
	   e_ele);
	}
        (void)fflush (stderr);

/*
 *
 *	E L E M E N T    L O O P:
 *
 */

	for (ele_num = s_ele - 1, j = 0; ele_num < e_ele; ele_num++, j++) {

	   if (debug_level > 5) {
	      (void) fprintf (stderr,
	      "\n    spec_distort: ele_num = %d, j = %d\n", ele_num, j);

	      (void) fprintf (stderr,
	      "    spec_distort: spec_rec: real1 = %f, real2 = %f\n",
	      spec_rec1->re_spec_val[ele_num], spec_rec2->re_spec_val[ele_num]);
	      (void) fprintf (stderr,
	      "    spec_distort: spec_rec: imag1 = %f, imag2 = %f\n",
	      spec_rec1->im_spec_val[ele_num], spec_rec2->im_spec_val[ele_num]);
	      (void) fprintf (stderr,
	      "    spec_distort: spec_rec: frqs1 = %f, frqs2 = %f\n",
	      spec_rec1->frqs[ele_num], spec_rec2->frqs[ele_num]);
	      (void) fflush (stderr);

	   }

	   diff_real[i_rec][j] = spec_rec1->re_spec_val[ele_num] -
				 spec_rec2->re_spec_val[ele_num];

	   tot_real += diff_real[i_rec][j];

	   diff_imag[i_rec][j] = spec_rec1->im_spec_val[ele_num] -
				 spec_rec2->im_spec_val[ele_num];

	   tot_imag += diff_imag[i_rec][j];

	   diff_frqs[i_rec][j] = spec_rec1->frqs[ele_num] -
				 spec_rec2->frqs[ele_num];

	   tot_frqs += diff_frqs[i_rec][j];

	   if (debug_level > 4) {
	      (void) fprintf (stderr,
	      "    spec_distort: tot_diff: real = %f, imag = %f, frqs = %f\n",
	      tot_real, tot_imag, tot_frqs);
	      (void) fflush (stderr);
	   }

	   switch (spec_type) {
		case ST_PWR:
		case ST_DB:
		case ST_REAL:
		   SpecPower1[j] = (double) spec_rec1->re_spec_val[ele_num];
		   SpecPower2[j] = (double) spec_rec2->re_spec_val[ele_num];
		   break;
		case ST_CPLX:
			SpecPower1[j] = MAG (spec_rec1->re_spec_val[ele_num],
					spec_rec1->im_spec_val[ele_num]);
			SpecPower2[j] = MAG (spec_rec2->re_spec_val[ele_num],
					spec_rec2->im_spec_val[ele_num]);
			break;
		default:
			(void) fprintf (stderr,
			"spec_dist: unknown spectral type.\n");
			exit (1);
			break;
	   }

	   if (debug_level > 3) {
	      (void) fprintf (stderr,
	      "    spec_distort: SpecPower1 = %g, SpecPower2 = %g\n",
	      SpecPower1[ele_num], SpecPower2[ele_num]);
	      (void) fflush (stderr);
	   }

	} /* end for */


	/*
	 * Compute the Itakura-Saito Distortion measures for this frame
	 *
	 */

     /* Commnet out the IS distortion code until it can be checked*/
/*        (void) ComputeSpec_IS_dist ();*/

   }  /* end big for loop */


   if ( Eflag )		/* then print results for each element */
      pr_spec_ele();

   if ( rflag && !Eflag ) {	/* print only Record and File Averages */
      pr_spec_rec_avg();	  	/* print Record Average */
      pr_spec_file_avg();  		/* print File Average */
      return;
   }

   if (!nflag)
      pr_spec_ele_avg();	/* print Element Average */

   if ( rflag )
      pr_spec_rec_avg();  	/* print Record Average */

   pr_spec_file_avg();  	/* print File Average */

   /* Again, comment out IS stuff until it can be checked */
/*   pr_spec_is_dist();*/		/* print Spectral Distortions */
/*   pr_spec_tot_is_dist();*/	/* print TOTAL Spectral Distortions */

} /* end spec_distort() */


static
void
allo_memory (rows, columns)	/* allocate memory for all arrays */
int	rows;
int	columns;
{

   double	**d_mat_alloc();	/* memory allocator */

   if (debug_level > 4)
	(void) fprintf (stderr,
	"allo_memory:  rows = %d, columns = %d\n", rows, columns);

   if ( (diff_real = (float **) d_mat_alloc (rows, columns)) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: d_mat_alloc: diff_real: could not allocate memory.\n");
	exit (1);
   }

   if ( (diff_imag = (float **) d_mat_alloc (rows, columns)) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: d_mat_alloc: diff_imag: could not allocate memory.\n");
	exit (1);
   }

   if ( (diff_frqs = (float **) d_mat_alloc (rows, columns)) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: d_mat_alloc: diff_frqs: could not allocate memory.\n");
	exit (1);
   }

   if ( (frame = (char **) d_mat_alloc (rows, 9)) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: d_mat_alloc: frame: could not allocate memory.\n");
	exit (1);
   }

   if ( (freq_len = (int *) calloc ((unsigned int) rows, sizeof (int))) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: calloc: freq_len: could not allocate memory.\n");
	exit (1);
   }

   if ( (IS_dist = (double *) calloc ((unsigned int) rows, sizeof (double))) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: calloc: IS_dist: could not allocate memory.\n");
	exit (1);
   }

   if ( (PNIS_dist = (double *) calloc ((unsigned int) rows, sizeof (double))) == NULL ) {
	(void) fprintf (stderr,
	"spec_distort: calloc: PNIS_dist: could not allocate memory.\n");
	exit (1);
   }

   if ((SpecPower1 = (double *) calloc ((unsigned int) columns, sizeof (double))) == NULL) {
	(void) fprintf (stderr,
	"spec_distort: calloc: SpecPower1: could not allocate memory.\n");
	exit (1);
   }

   if ((SpecPower2 = (double *) calloc ((unsigned int) columns, sizeof (double))) == NULL) {
	(void) fprintf (stderr,
	"spec_distort: calloc: SpecPower2: could not allocate memory.\n");
	exit (1);
   }


}


static char *
check_frames (spec_rec1, spec_rec2)
struct feaspec  	*spec_rec1;
struct feaspec   	*spec_rec2;
{

/*
 * This module checks if two frames are compatible
 * (i.e. both frames must be either "Voiced" or "Unvoiced" or nothing)
 * if one is "Voiced" and the other "Unvoiced", then
 * check_frames prints a message on stderr and exits with exit (1)
 * otherwise it returns a string of type of frame (" Voiced " or "Unvoiced").
 * If neither "Vocied" nor "Unvoiced", "Unvoiced" is returned.
 */

	static char	*Voiced = " Voiced ";
	static char	*Unvoiced = "Unvoiced";

/*
 *	External Variable Used:
 *
 *   int	debug_level;	print messages for debugging purpose
 *
 */


   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");
 if((voicing1=(short *)get_fea_ptr(spec_rec1->fea_rec,"voiced",f1h))!=NULL &&
    (voicing2=(short *)get_fea_ptr(spec_rec2->fea_rec, "voiced", f2h))!=NULL){
   if ( (*voicing1 == YES) && (*voicing2 == YES) )
      {
	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "check_frames: voiced frame\n");
	return (Voiced);
      }
   else
     if ( (*voicing1 == NO) && (*voicing2 == NO) )
	{
	  if (debug_level > 3)
	     (void) fprintf (stderr,
	     "check_frames: unvoiced frame\n");
	  return (Unvoiced);
	}
     else
       {
	  /* inconsistent frames */

	  (void) fprintf (stderr,
	  "check_spec: fatal error, one voiced frame other unvoiced frame.\n");
	  exit (1);
       }
 }
 else{/*voicing field is not in both files*/
  return(Unvoiced);
 }
}	


ComputeSpec_IS_dist ()
{

	double	*Fi;
	double	*PNFi;

	double	PF1 = 0.0;
	double	PF2 = 0.0;

	double	*ratio;
	double	PFratio;

	int	N;	/* number of frequencies minus one */

	int	i;	/* temporary variables */
	int	loop;


/*
 *	External Variable Used:
 *
 *   int	debug_level;	print messages for debugging
 *   int	num_of_freqs;
 *   int	freq_format;
 *   int	spec_type;
 *
 *   double	*SpecPower1;	Spectral Powers
 *   double	*SpecPower2;	Spectral Powers
 *
 */

   /*
    * allocate memory for:  Fi, ratio, and PNFi
    *
    */

   N = num_of_freqs - 1;

   if ((Fi = (double *) calloc((unsigned int)N + 1, sizeof(double))) == NULL ) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for Fi.\n");
	exit (1);
   }
   if ((PNFi = (double *) calloc((unsigned int)N + 1, sizeof(double))) == NULL ) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for PNFi.\n");
	exit (1);
   }
   if ((ratio = (double *) calloc((unsigned int)N + 1, sizeof(double))) == NULL ) {
	(void) fprintf (stderr,
	"compute_is_dist: calloc: could not allocate memory for ratio.\n");
	exit (1);
   }


   if (debug_level > 2)
	(void) fprintf (stderr,
	"compute_is_dist: memory allocated, now do proper conversions.\n");


   if (sflag)
	loop = 2;
   else
	loop = 1;


   while (loop--)
   {

     if (debug_level > 2) {
	(void) fprintf (stderr,
	"ComputeSpec_IS_dist: %s time around loop:",
	(sflag && (loop == 1)) ? "first" : "second");
	if (sflag)
	   (void) fprintf (stderr,
	   " computing Symmetric distortion\n");
	else
	   (void) fprintf (stderr,
	   " do not compute Symmetric distortion\n");
     }

     for (i = 0; i < N + 1; i++) {
	 switch (spec_type) {
	    case ST_DB:
		ratio[i] = pow ((double) 10, SpecPower1[i]/10) /
			   pow ((double) 10, SpecPower2[i]/10);
		break;
	    case ST_PWR:
	    case ST_REAL:
	    case ST_CPLX:
		ratio[i] = SpecPower1[i] / SpecPower2[i];
		break;
	    default:
		(void) fprintf (stderr,
		"spec_dist: spectral type not implemented yet.\n");
		exit (1);
		break;
	 }

	 if (sflag && (loop == 0))	/* second time around */
	    ratio[i] = 1 / ratio[i];

	 Fi[i] = ratio[i] +  log ( 1 / ratio[i] ) - 1.0;

	 switch (freq_format) {
	    case SYM_CTR:
	    case ASYM_CEN:
		PF1 += SpecPower1[i] / (N + 1);
		PF2 += SpecPower2[i] / (N + 1);
	        break;
	    case SYM_EDGE:
	    case ASYM_EDGE:
		if (i != N) {
		   PF1 += (0.5 / N) * (SpecPower1[i] + SpecPower1[i+1]);
		   PF2 += (0.5 / N) * (SpecPower2[i] + SpecPower2[i+1]);
		}
		break;
	    default:
		(void) fprintf (stderr,
		"ComputeSpec_IS_dist: frequency type not supported yet.\n");
		break;
	 }

     } /* end for (i = 0; i < N + 1; i++) */


     if (debug_level > 5)
	for (i = 0; i < num_of_freqs; i++)
	    (void) fprintf (stderr,
	    "Fi[%d] = %f\n", i, Fi[i]);

     if (sflag && (loop == 0))	/* second time around */
	 PFratio = PF1 / PF2;
     else
	 PFratio = PF2 / PF1;

     for (i = 0; i < N + 1; i++) {
	 PNFi[i] = ratio[i] * PFratio - log ( ratio[i] * PFratio ) - 1.0;
     }


     switch (freq_format) {
	case SYM_CTR:
	   for (i = 0; i < N + 1; i++) {
		IS_dist[i_rec] += (1 / (N + 1)) * Fi[i];
		PNIS_dist[i_rec] += PNFi[i] / (N + 1);
	   }
	   break;

	case SYM_EDGE:
	   for (i = 0; i < N; i++) {
		IS_dist[i_rec] += (0.5 / N) * (Fi[i] + Fi[i+1]);
		PNIS_dist[i_rec] += (0.5 / N) * (PNFi[i] + PNFi[i+1]);
	   }
	   break;

	case ASYM_CEN:
	   for (i = 0; i < N + 1; i++) {
		IS_dist[i_rec] += (1 / (N + 1)) * Fi[i];
		PNIS_dist[i_rec] += PNFi[i] / (N + 1);
	   }
	   break;

	case ASYM_EDGE:
	   for (i = 0; i < N; i++) {
		IS_dist[i_rec] += (0.5 / N) * (Fi[i] + Fi[i+1]);
		PNIS_dist[i_rec] += (0.5 / N) * (PNFi[i] + PNFi[i+1]);
	   }
	   break;

	case ARB_VAR:
	   (void) fprintf (stderr,
	   "ComputeSpec_IS_dist: frequency format not supported yet.\n");
	   exit (1);
	   break;

	case ARB_FIXED:
	   (void) fprintf (stderr,
	   "ComputeSpec_IS_dist: frequency format not supported yet.\n");
	   exit (1);
	   break;

	default:
	   (void) fprintf (stderr,
	   "ComputeSpec_IS_dist: unknown frequency format.\n");
	   exit (1);
	   break;

     }

     if (debug_level > 2) {
	(void) fprintf (stderr,
	"ComputeSpec_IS_dist: i_rec = %d, IS_dist = %g, PNIS_dist = %g\n",
	i_rec, IS_dist[i_rec], PNIS_dist[i_rec]);
     }

   } /* end while (loop--) loop */


   /*
    * If the -s option was given, then compute the symmetric version of the
    * Itakura-Saito distortion measure.
    *
    */
     if (sflag) {

	if (debug_level > 3)
	   (void) fprintf (stderr,
	   "compute_is_distort: now computing symmetric IS distortions.\n");

	IS_dist[i_rec] /= 2;
	PNIS_dist[i_rec] /= 2;

     }


     if (ABS (IS_dist[i_rec]) <= ERROR) {
	IS_dist[i_rec] = 0.0;
	PNIS_dist[i_rec] = 0.0;
     }

     free ((char *) Fi);
     free ((char *) PNFi);
     free ((char *) ratio);
}
