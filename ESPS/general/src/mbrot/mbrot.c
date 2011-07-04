/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
 				

      Compute Mandelbrot set

  David Burton, ESI
*/



#ifndef lint
static char *sccs_id = "@(#)mbrot.c	1.8 2/28/96	ESI";
#define DATE "2/28/96"
#define VERS "1.8"
#else
#define DATE "none"
#define VERS "none"
#endif

#define MAX_COLOR 114 /*number of usable levels in the colormap*/
#define GRD_PTS 100   /*minimum number of grid cells*/
#define SYNTAX USAGE("mbrot [-g grid_size] [-l left_edge] [-r right_edge]\
\n[-b bottom_edge] [-t top_edge] [-c iterations] [-x] [-C] feafile.out")


#include <stdio.h>
#include <esps/esps.h>
#include <esps/complex.h>
#include <esps/fea.h>
#include <esps/unix.h>

extern int  optind;
extern char *optarg;

FILE * fopen ();
char   *get_cmd_line ();

short mandelbrot();
COMPLEX cadd(), cmult();

int debug_level = 0;

main (argc, argv)
int     argc;
char  **argv;
{
    char   *out_file = NULL;
    FILE * out_strm = NULL;

    struct header   *out_hd;
    struct fea_data *out_rec;

    COMPLEX input;             /*input to set computation*/
    int conf = 3*(MAX_COLOR);           /*number of iterations to do*/
    int num_grd_pts = GRD_PTS;           /*number of points in each slice*/
    double left_edge = -1.9;   
    double right_edge = .6;
    double bottom_edge = -1.;
    double top_edge = 1.;
    int color = NO;
    int thresh;
    long i, j;
    int     c;
    double   incX, incY;
    short   *data_ptr;
    double   *pos_ptr;


/* default parameters */

    char   *param_file = "params";/* default parameter file name */
    short   field_type = SHORT;/* Default field_type */
    long    field_size = 1;	/* default field_size */
    char   *field_name = "mbrot";


    while ((c = getopt (argc, argv, "c:g:l:r:b:t:x:C")) != EOF) {
	switch (c) {
            case 'C':
	        color = YES;
		break;
            case 'c':
	        conf = atoi(optarg);
		if(conf < MAX_COLOR){
		  Fprintf(stderr, "iterations must be >=1 %d\n", MAX_COLOR);
		  exit(1);
		}
		break;
            case 'g':
	        num_grd_pts = atoi(optarg);
		if(num_grd_pts < 100){
		  Fprintf(stderr, "grid_size must be >= %d\n", GRD_PTS);
		  exit(1);
		}
		break;
            case 'l':
	        left_edge  = atof(optarg);
		break;
            case 'r':
	        right_edge = atof(optarg);
		break;
            case 'b':
	        bottom_edge = atof(optarg);
		break;
            case 't':
	        top_edge = atof(optarg);
		break;
	    case 'x': 
		debug_level = 1;
		break;
            default:
		SYNTAX;
		exit(1);
	}
    }

/*
 * Silly inputs
*/
    if(left_edge > right_edge){
      Fprintf(stderr, "left_edge must be < right_edge\n");
      exit(1);
    }

    if(bottom_edge > top_edge){
      Fprintf(stderr, "bottom_edge must be < top_edge\n");
      exit(1);
    }

    if ((argc - optind) < 1){
      Fprintf(stderr, "Output file must be given\n");
      SYNTAX;
      exit(1);
    }

/*
 * First determine vertical and horizontal increments
*/

    incX = (right_edge - left_edge)/(double)num_grd_pts;
    incY = (top_edge - bottom_edge)/(double)num_grd_pts;

/*
 * Check debug stuff
*/
    if (debug_level) {
	(void)fprintf (stderr, "number of grid points = %d\n", num_grd_pts);
	(void)fprintf (stderr, "out_file: %s\n", argv[argc-1]);
	(void)fprintf(stderr, "left_edge = %g\n", left_edge);
	(void)fprintf(stderr, "right_edge = %g\n", right_edge);
	(void)fprintf(stderr, "bottom_edge = %g\n", bottom_edge);
	(void)fprintf(stderr, "top_edge = %g\n", top_edge);
	(void)fprintf(stderr, "num of iterations = %d\n", conf);
        (void)fprintf(stderr, 
		"Do color processing? (1 = YES, 0 = NO): %d\n", color);
    }



    out_file = eopen ("mbrot", argv[argc-1], "w", FT_FEA, NONE,
		&out_hd, &out_strm);

    out_hd = new_header (FT_FEA);

    (void)strcpy (out_hd -> common.prog, "mbrot");
    (void)strcpy (out_hd -> common.progdate, DATE);
    (void)strcpy (out_hd -> common.vers, VERS);

    add_comment (out_hd, get_cmd_line (argc, argv));

    field_size = num_grd_pts + 1;

    if (field_size < 1){
	Fprintf(stderr, "the field size cannot be less than 1");
	exit(1);
      }

/*
 * Create fields in output file
*/
    if(add_fea_fld (field_name, field_size, (short) 1, (long *) NULL, 
	field_type, (char **) NULL, out_hd) == -1)
    {
      Fprintf(stderr, "Couldn't allocate field %s\n", field_name);
       exit(1);
     }
    if(add_fea_fld("X_pos", 1L, (short)1, (long *) NULL, DOUBLE,
		(char **) NULL, out_hd) == -1)
    {
      Fprintf(stderr, "Couldn't allocate field X_pos\n");
       exit(1);
     }
/*
 * Add generics
*/
    *add_genhd_l("num_grd_pts", (long *)NULL, 1, out_hd) = (long) num_grd_pts;
    *add_genhd_d("left_edge", (double *)NULL, 1, out_hd) =  left_edge;
    *add_genhd_d("right_edge", (double *)NULL, 1, out_hd) =  right_edge;
    *add_genhd_d("bottom_edge", (double *)NULL, 1, out_hd) =  bottom_edge;
    *add_genhd_d("top_edge", (double *)NULL, 1, out_hd) =  top_edge;
    *add_genhd_d("X_inc", (double *)NULL, 1, out_hd) =  incX;
    *add_genhd_d("Y_inc", (double *)NULL, 1, out_hd) =  incY;
    *add_genhd_l("num_iterations", (long *)NULL, 1, out_hd) =  (long)conf;    
/*    *add_genhd_d("start_time", (double *)NULL, 1, out_hd) =  left_edge;
    *add_genhd_d("record_freq", (double *)NULL, 1, out_hd) =  1./incX;*/
    if(color == YES)
       (void)add_genhd_c("color_processing", "YES", 0, out_hd); 
    else
       (void)add_genhd_c("color_processing", "NO", 0, out_hd); 
/*
 * Write output header
*/
    write_header (out_hd, out_strm);

/*
 *Allocate output record and get pointer to field
*/
    out_rec = allo_fea_rec (out_hd);
    data_ptr = (short *)get_fea_ptr (out_rec, field_name, out_hd);
    spsassert (data_ptr != NULL, "get_fea_ptr returned null");
    pos_ptr = (double *)get_fea_ptr (out_rec, "X_pos", out_hd);
    spsassert (pos_ptr != NULL, "get_fea_ptr returned null");

/*
 * Determine if values are in Mandelbrot set
*/

    /*
     * compute scaling value for maximum MAX_COLOR value - really only works 
     * for multiples of MAX_COLOR
    */
    thresh = conf/(MAX_COLOR);
    if((thresh * (MAX_COLOR)) < conf)
      thresh++;

/*
 * More debug stuff
*/
    if(debug_level){
      (void)fprintf(stderr, "incX = %g, incY = %g, thresh = %d\n", 
	      incX, incY, thresh);
    }

    /*
     * compute mandelbrot set values
    */

    for(i=0; i<=num_grd_pts; i++){
      input.real = left_edge + i*incX;
      for(j=0;j<=num_grd_pts; j++){
	input.imag = bottom_edge + j*incY;
	data_ptr[j] = mandelbrot(input, conf, color, thresh);
       }
      *pos_ptr = input.real;
      put_fea_rec (out_rec, out_hd, out_strm);
    }

    exit(0);
    /*NOTREACHED*/
}

short
mandelbrot(input, conf, color, thresh)
COMPLEX input;
int conf, color, thresh;
{
COMPLEX result;
int i;
   result.real = 0.;
   result.imag = 0.;

  if(color == NO){
    for(i=0; i<conf; i++){
      result = cadd(cmult(result,result), input);
      if(result.real>2 || result.real<-2 || result.imag>2 || result.imag<-2)
        return (0);
    }
    return (1);
  }
  else{
    for(i=0; i<conf; i++){
      result = cadd(cmult(result,result), input);
      if(result.real>2 || result.real<-2 || result.imag>2 || result.imag<-2)
        return ((short)(i/thresh - 64));
    }
    return ((short)(conf/thresh - 64));
  }
}





