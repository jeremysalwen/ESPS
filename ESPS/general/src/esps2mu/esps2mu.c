/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright (c) 1989 Entropic Speech, Inc."
 *
 * Program:	esps2mu
 *
 * Written by:  David Burton
 *
 */


static char *sccs_id = "@(#)esps2mu.c	1.3 7/8/96	 ESI";

/*
 * include files
 */
#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/sd.h>
#include <esps/feasd.h>

/*
 * defines
 */

#define SYNTAX USAGE ("esps2mu [-x debug_level] [-s shift] infile outfile")
#define BUFFER 8192	/*Max. buffer length for transfering data*/

/*
 * system functions and variables
 */

extern  optind;
extern	char *optarg;
int atoi();
#if !defined(DEC_ALPHA) && !defined(HP700) && !defined(LINUX_OR_MAC)
char *calloc();
#endif
double pow();

/*
 * external ESPS functions
 */

int getopt ();
char *eopen();
int linear_to_mu();


/*
 * global declarations
 */
int debug_level = 0;

/*
 * main program
 */
main (argc, argv)
int argc;
char **argv;
{
/*
 * setup and initialization
*/

int c;			    /*for getopt return*/
int i;			    /*loop counter*/
char *outfileD = NULL;	    /*file name for output data file*/
FILE *outstrmD;
struct header *ih;	    /*pointer to SD file header*/
char *sd_file = NULL;	    /*file name for input SD file*/
FILE *istrm = stdin;	    /*input SD file stream*/
short *sdata;		    /*array to hold data*/
float *fdata;		    /* array to hold float data*/
int buffer_size = BUFFER;    /*number of items retrieved from input sampled
			      data file*/
int points;		    /* number of points returned by get_sd_recs*/
char *mudata;               /* buffer to hold mu-encoded data */
float shift = 0;            /* scaling factor for data */
long num_channels;          /* holds number of input data channels*/

/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:s:")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi (optarg);
		break;
            case 's':
		shift = pow((double)2,(double)atoi(optarg));
		break;
	    default:
		SYNTAX;
	}
    }
/*
 * process file arguments and open input and output files
 */

/*
 * First get input file
*/
    if(optind < argc) {
	sd_file = eopen("esps2mu", argv[optind++], "r", FT_FEA, FEA_SD,
			&ih, &istrm);
	if(debug_level > 0)
	    Fprintf(stderr, "esps2mu: input file is %s\n", sd_file);
    }
    else{
	Fprintf(stderr, "esps2mu: No input file specified.\n");
	SYNTAX;
    }

/*
 * check if input file is multichannel
*/
    if((num_channels =
        get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) != 1){
           Fprintf(stderr, "esps2mu: Multichannel data not supported yet - exiting.\n");
           exit(1);
         }

/*
 * Check if input data is complex
*/
    if(is_field_complex(ih, "samples") == YES)
      {
           Fprintf(stderr, "esps2mu: Complex data not supported - exiting.\n");
           exit(1);
         }


/*
 * Now output file
*/

    if (optind < argc) {
      outfileD = eopen("esps2mu", argv[optind], "w", NONE, NONE,
		       (struct header **) NULL, &outstrmD);
    }
    else {
	Fprintf(stderr, "esps2mu: no output file name specified.\n");
	SYNTAX;
        }

    /*
     * allocate memory for data transfer
    */

    mudata = (char *) calloc ((unsigned) buffer_size, sizeof(char));
    spsassert(mudata != NULL, "Cannot allocate needed space for output buffer");
    switch(get_sd_type(ih)){
	case FLOAT:
	    fdata = (float*) calloc ((unsigned) buffer_size, sizeof(float));
	    spsassert(fdata != NULL, "Cannot allocate needed space for input");
	    sdata = (short*) calloc ((unsigned) buffer_size, sizeof(short));
	    spsassert(sdata != NULL, "Cannot allocate needed space for input");
	    break;
	case SHORT:
	    sdata = (short*) calloc ((unsigned) buffer_size, sizeof(short));
	    spsassert(sdata != NULL, "Cannot allocate needed space for input");
	    break;
	default:
	    Fprintf(stderr,
	    "esps2mu: Convert the input file to SHORT or FLOAT\n");
	    exit(1);
    }

    if (debug_level> 0) Fprintf(stderr,
		"esps2mu:reading blocks of %d points from data file %s\n",
		buffer_size,  sd_file);
    /*
     * Now pack data array and write output file
    */
    switch(get_sd_type(ih)){
      case SHORT:
	while ((points = get_sd_recs(sdata, buffer_size, ih, istrm)) != 0){
	 /* shift the data, if appropriate */
	  if(debug_level > 1)
	    Fprintf(stderr, "esps2mu: Got %d points\n", points);
	
	  if (shift > 0){
	  for(i=0; i<points; i++)
	    sdata[i] = sdata[i]/shift;
	}
	  /*convert to mu encoded data*/
	  linear_to_mu(sdata, mudata, (long)points);
	  if (debug_level >1){
	     for(i=0;i<points; i++){
	       Fprintf(stderr,"%d", mudata[i]);
	       Fprintf(stderr,"\n");
	     }
	   }

	 (void) fwrite(mudata, sizeof (*mudata), points, outstrmD);
	}
	break;
      case FLOAT:
	while ((points = get_sd_recf(fdata, buffer_size, ih, istrm)) != 0){

	 /*copy to shorts and shift, if appropriate*/
         if (shift > 0)
	  for(i=0; i<points; i++)
	    sdata[i] = fdata[i]/shift;
         else
	  for(i=0; i<points; i++)
	    sdata[i] = fdata[i];

	  /*convert to mu encoded data*/
	  linear_to_mu(sdata, mudata, (long)points);
	  (void)fwrite(mudata, sizeof (*mudata), points, outstrmD);
       }
	break;
      default:
	Fprintf(stderr, "Invalid input data type");
	exit(1);
      }
 /*
 * clean up and exit
 */
    exit(0);
/*NOTREACHED*/
}
