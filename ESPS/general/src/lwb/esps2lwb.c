/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright (c) 1990 Entropic Speech, Inc."
 *
 * Program:	esps2lwb
 *
 * Written by:  David Burton
 *
 * This is the esps2lwb program, which converts an ESPS sampled data file
 * into an LWB data file (ascii header and data) 
 */


static char *sccs_id = "@(#)esps2lwb.c	1.5 2/22/91 ESI";

/*
 * include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <time.h>

/*
 * Local includes
*/
 /*
  * include boiler plate for LWB header file
 */
#include "header.h"

/*
 * defines
 */

#define SYNTAX USAGE ("esps2lwb [-x debug_level] infile outfile")
#define MAXLINE 512	/*max line length for input file lines and names*/
#define BUFFER 1600	/*Max. buffer length for transfering data*/

/*
 * system functions and variables
 */

extern  optind;
extern	char *optarg;
int atoi();
char *calloc();
char *ctime();
long time();
char *strcat();
char *strcpy();   
/*
 * external SPS functions
 */
int is_field_complex();
int getopt ();
char *eopen();
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
char outf[MAXLINE];	    /*temporary storage for output file name*/
char *outfileH;	    	    /*file name for output ASCII header file*/
FILE *outstrmH;
char *outfileD = NULL;	    /*file name for output ASCII data file*/
FILE *outstrmD;
struct header *ih;	    /*pointer to SD file header*/
char *sd_file = NULL;	    /*file name for input SD file*/
FILE *istrm = stdin;	    /*input SD file stream*/
short *sdata;		    /*array to hold data*/
float *fdata;		    /* array to hold float data*/
char *H = ".H";		    /*suffix for header file*/
char *D = ".D";		    /*suffix for data file*/
int nchan = 1;		    /*holds number of multiplexed channels*/
float sfreq = 0;	    /*holds sampling frequency*/
float conv_gain;	    /*converter gain*/
int buffer_size = BUFFER;    /*number of items retrieved from input sampled
			      data file and number of samples from each 
			      channel written to output LWB data file*/
int num_samps = 0;	    /*number of times nchan channels were sampled*/
long tloc;		    /*used to get current time*/
float slope;		    /*one/sampling_frequency*/
int points;		    /* number of points returned by get_sd_recs*/
int type = 0;		    /*LWB data type code*/
short *equip = NULL;        /* hold equipment code name*/
struct feasd *data_rec = NULL;/* fea_sd data structure*/
/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
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
	sd_file = eopen("esps2lwb", argv[optind++], "r", FT_FEA, FEA_SD,
			&ih, &istrm);
	if(debug_level > 0)
	    Fprintf(stderr, "ESPS2LWB: input file is %s\n", sd_file);
    }
    else{
	Fprintf(stderr, "ESPS2LWB: No input file specified.\n");
	SYNTAX;
    }
/*
 * Now output file - stdout cannot be written because there are two files
*/
    
    if (optind < argc) {

	(void)strcpy(outf, argv[optind]);
	outfileH = strcat(outf, H);
	if(debug_level>0)
	    Fprintf(stderr, "ESPS2LWB: Output header file is %s\n",outfileH);
	TRYOPEN(argv[0], outfileH, "w", outstrmH);
	(void)strcpy(outf, argv[optind]);
	outfileD = strcat(outf,  D);
	if(debug_level>0)
	    Fprintf(stderr, "ESPS2LWB: Output data file is %s\n", outfileD);
	TRYOPEN(argv[0], outfileD, "w", outstrmD);
    }
    else {
	Fprintf(stderr, "ESPS2LWB: no output file name specified.\n");
	SYNTAX;
        }

/*
 * get relevant header items
*/
    /* check number of channels*/
    if((nchan = 
        get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) == 0){
          Fprintf(stderr, 
                  "filter: No sampled data field in %s - exiting.\n", sd_file);
           exit(1);
         }
     
     /* check for complex data*/
     if(is_field_complex(ih, "samples") == YES)
      {
           Fprintf(stderr, "filter: Complex data not supported - exiting.\n");
           exit(1);
         }    

     /*get sampling frequency*/
     sfreq = get_genhd_val("record_freq", ih, (double) -1);

    /*get equipment code*/
    equip = get_genhd_s("equip", ih);

    if (equip != NULL ) {
	if(debug_level > 0)
	  Fprintf(stderr, "equip = %d\n", *equip);

	if(*equip == EF12M){
	    conv_gain = .004882812;
	    Fprintf(stderr, 
		    "ESPS2LWB: default upper and lower limits used;\n\t +10 and -10 respectively\n");
	    Fprintf(stderr, "\tEdit %s if not correct\n", outfileH);
	  }
	else if(*equip == AD12F){
	    conv_gain = .000305175;
	    Fprintf(stderr, 
		    "ESPS2LWB: default upper and lower limits used;\n\t +10 and -10 respectively\n");
	    Fprintf(stderr, "\tEdit %s if not correct\n", outfileH);
	  }
	else if(*equip == DSC){
	    conv_gain = .000305175;
	    Fprintf(stderr, 
		    "ESPS2LWB: default upper and lower limits used;\n\t +10 and -10 respectively\n");
	    Fprintf(stderr, "\tEdit %s if not correct\n", outfileH);
	  }
	else if(*equip ==  LPA11){
	    conv_gain = .004882812;
	    Fprintf(stderr, 
		    "ESPS2LWB: default upper and lower limits used;\n\t +10 and -10 respectively\n");
	    Fprintf(stderr, "\tEdit %s if not correct\n", outfileH);
	  }
      } else {
	  conv_gain = .004882812;
	  (void)strcpy(outf, argv[optind]);
	  Fprintf(stderr, 
		  "ESPS2LWB: default conv gain = .004882812 used.\n\tEdit %s if incorrect.\n\n", strcat(outf, H));
	  Fprintf(stderr, 
		  "ESPS2LWB: default upper and lower limits used:  +10 and -10 respectively.\n");
	  Fprintf(stderr, "\tEdit %s if incorrect\n", outfileH);
	}

    /*
     * allocate memory for data transfer
    */

    switch(get_sd_type(ih)){
	case FLOAT:
	    fdata = (float*) calloc ((unsigned) buffer_size, sizeof(float));
	    spsassert(fdata != NULL, "Cannot allocate needed space");
	    type = 5;
	    break;
	case SHORT:
	    sdata = (short*) calloc ((unsigned) buffer_size, sizeof(short));
	    spsassert(sdata != NULL, "Cannot allocate needed space");
	    type = 2;
	    break;
	default:
	    Fprintf(stderr, "LWB does not support this ESPS data type\n");
	    Fprintf(stderr, 
	    "Convert the input file to SHORT or FLOAT\n");
	    exit(1);
    }
/*
 * copy data into temporary array and then into outfile.D
*/
    /*
     * first compute appropriate block size for this file
    */

	num_samps = buffer_size/nchan; /*using truncating integer division*/
	buffer_size = num_samps*nchan;

    if (debug_level> 0) Fprintf(stderr, 
		"ESPS2LWB:reading blocks of %d points from data file %s\n", 
		buffer_size,  sd_file);
    /* 
     * Now pack data array and write output file
    */
    switch(get_sd_type(ih)){
      case SHORT:
        data_rec = 
	  allo_feasd_recs(ih, SHORT, (long)buffer_size, (char *)NULL, NO);
	sdata = (short *)data_rec->data;
	while ((points = 
	     get_feasd_recs(data_rec, 0L,(long)buffer_size, ih, istrm)) != 0){
	    int j, k;
	    for(j=0;j<points/nchan;j++){
		for(k=0;k<nchan; k++){
		    Fprintf(outstrmD, "%d ", sdata[j*nchan+k]);
		}
		Fprintf(outstrmD, "\n");
	    }
	}
	/* All done copying data */    
	break;
      case FLOAT:
	data_rec = 
	  allo_feasd_recs(ih, FLOAT, (long)buffer_size, (char *)NULL, NO);
	fdata = (float *)data_rec->data;
	while ((points = 
             get_feasd_recs(data_rec, 0L, (long)buffer_size, ih, istrm)) != 0){
	    int j, k;
	    for(j=0;j<points/nchan;j++){
		for(k=0;k<nchan; k++){
		    Fprintf(outstrmD, "%f ", fdata[j*nchan+k]);
		}
		Fprintf(outstrmD, "\n");
	    }
	}
	/* All done copying data */    
	break;
      default:
	Fprintf(stderr, "Invalid input data type");
	exit(1);
    }

/*
 * write LWB header output file
 */
    slope = 1.0/sfreq; 
    
    Fprintf(outstrmH, "%s", header1);
    Fprintf(outstrmH, "%s\n", argv[optind]);
    Fprintf(outstrmH, "%s",  header2);
    Fprintf(outstrmH, "%d,%d,0\n", buffer_size, nchan);
    Fprintf(outstrmH, "%s",  header3);
    Fprintf(outstrmH, "%d\n", type);
    Fprintf(outstrmH, "%s", header3a);
    Fprintf(outstrmH, "%s",  header10);
    tloc = time(0);
    Fprintf(outstrmH, "%s", ctime(&tloc));
    Fprintf(outstrmH, "%s",  header4);
    Fprintf(outstrmH, "%f\n", slope);
    Fprintf(outstrmH, "%s", header5);

    /*
     * Finished static stuff, now do each channel of multiplexed data
    */

    for(i=0; i<nchan; i++){
	    Fprintf(outstrmH, "%s", header6);
	    Fprintf(outstrmH, "%d\n", i);
	    Fprintf(outstrmH, "%s", header7);
	    Fprintf(outstrmH, "%d\n", i);
	    Fprintf(outstrmH, "%s", header8);
	    Fprintf(outstrmH, "%f", conv_gain);
	    Fprintf(outstrmH, "%s", header9);
    }
/*
 * Now write out data record description
*/
	Fprintf(outstrmH,"\nrecord:0 ascii_data:\n");

 /*
 * clean up and exit
 */
    exit(0);
    return(0);
}
