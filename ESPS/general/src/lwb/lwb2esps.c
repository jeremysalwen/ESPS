/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright (c) 1990 Entropic Speech, Inc."
 *
 * Program:	lwb2esps
 *
 * Written by:  David Burton
 *
 * This is the lwb2esps program, which converts a LWB data file (ascii 
 * header and data) into an ESPS sampled data file.
 */

#ifndef lint
static char *sccs_id = "@(#)lwb2esps.c	1.5 7/16/91 ESI";
#endif
/*
 * include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>
/*
 * defines
 */
#define SYNTAX USAGE ("lwb2esps [-x debug_level] [-c channel_num] infile outfile")
#define ZERO 0
#define ONE 1
#define TWO 2
#define MAXLINE 512	/*max line length for input file lines and names*/
#define BIGNUMBER 100000

/*
 * system functions and variables
*/
int getopt ();
#ifndef IBM_RS6000
int atoi();
char *strcat();
char *strtok();
char *fgets();
void rewind();
char *calloc();
char *strcpy();
int sscanf();
char *savestring();
int fscanf();
char *realloc();
#endif

/*
 * external ESPS functions
 */
char *get_cmd_line();
void write_header();
void add_comment();
char *add_genhd_c();
char *eopen();
float *atoarrayf();
struct feasd *allo_feasd_recs();

/*
 * global declarations
 */
int debug_level = 0;
extern  optind;
extern	char *optarg;

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
long i;			    /*loop counter*/
char inf[MAXLINE];	    /*temporary storage for input file name*/
char *infileH = NULL;	    /*file name for input ASCII header file*/
FILE *instrmH;
char *infileD = NULL;	    /*file name for input binary data file*/
FILE *instrmD;
struct header *oh;	    /*pointer to SD file header*/
char *sd_file = NULL;	    /*file name for output SD file*/
FILE *ostrm = stdout;	    /*output SD file stream*/
int c_flag = 0;		    /*flag for -c option*/
float *sdata;		    /*array to hold test data*/
int channel_num = 0;	    /*holds channel number of multiplexed data to
				convert*/
char *H = ".H";		    /*suffix for header file*/
char *D = ".D";		    /*suffix for data file*/
static char header[BIGNUMBER];	    /*contains ascii file*/
char *cptr;		    /*character pointer for ascii file input*/
long points;		    /*holds total number of sampled data points*/
char keyword[MAXLINE];	    /*holds line identifier*/
int nchan = 1;		    /*holds number of multiplexed channels*/
float sfreq = 0;	    /*holds sampling period*/
short conv_flag = 0;	    /*flag for reading converter gain*/
short high_flag = 0;	    /*flag for reading high value */
short slope_flag = 0;	    /*flag for sampling frequency*/
float conv_gain = 0;	    /*converter gain*/
int high_limit = 0;	    /*high limit of A/D*/
int items_read = 0;	    /*holds sscanf return value*/
int dimn = 0;		    /*dimension of SIZEs field*/
float max;		    /*needed by atoarrayf*/
int ascii = 0;		    /* flag for data type */
int binary = 0;	    /* flag for binary data */
double max_value;           /* holds maximum possible value in file*/
double sf;                  /* holds sampling frequency*/
double start_time;          /* waves generic*/
struct feasd *data_rec;     /* data strcuture for output file*/
float *fdata;               /*pointer to data record data*/
int nchan_out;              /* holds number of output channels*/

char *program = "@(#)lwb2esps.c	1.5";
char *version = "1.5";
char *date = "7/16/91";



/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "x:c:")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'c':
		c_flag++;
		channel_num = atoi (optarg);
		break;
	    default:
		SYNTAX;
	}
    }
/*
 * process file arguments and open input and output files
 */

/*
 * First get input files
*/
    if(optind < argc) {
	(void)strcpy(inf, argv[optind]);
	infileH = strcat(inf, H);
	if(debug_level>0)
	    Fprintf(stderr, "Header file is %s\n", infileH);
	TRYOPEN(argv[0], infileH, "r", instrmH);
	(void)strcpy(inf, argv[optind]);
	infileD = strcat(inf,  D);
	if(debug_level>0)
	    Fprintf(stderr, "Data file is %s\n", infileD);
	TRYOPEN(argv[0], infileD, "r", instrmD);
    }
    else {
	Fprintf(stderr, "LWB2ESPS: no input file name specified.\n");
	SYNTAX;
    }
/*
 * Now output file
*/
    
    if (++optind < argc) {
	sd_file = argv[optind++];
	if(debug_level>0){
	    Fprintf(stderr, "Output sampled data file is %s\n", sd_file);
	}
	sd_file = eopen("lwb2esps", sd_file, "w", FT_FEA, FEA_SD,
			(struct header **)NULL, &ostrm);
    }
    else {
	Fprintf(stderr, "LWB2ESPS: no output file specified.\n");
	SYNTAX;
        }

/*
 *set up header for output sampled data file
*/
    oh = new_header(FT_FEA);
    (void) strcpy (oh->common.prog, program);
    (void) strcpy (oh->common.vers, version);
    (void) strcpy (oh->common.progdate, date);
    (void) add_comment (oh, get_cmd_line(argc,argv));
    oh->common.tag = NO;
/*
 * complete initialization based on input header file
 */
  /*
  * First read header file and store in comment field
  */
    cptr = header;
    while((c=fgetc(instrmH))!=EOF){
	*cptr++ = c;
    }
    if((feof(instrmH)) == ZERO){
	Fprintf(stderr, "LWB2ESPS: Error in reading LWB header file %s\n", 
		infileH);
    exit(1);
    }
    *cptr = NULL;
    if(debug_level > 1){
	Fprintf(stderr, "LWB2ESPS: Copy of Header File.\n");
	Fprintf(stderr, "%s", header);
    }

 /*
  * Store LWB header as generic header item
 */
    (void)add_genhd_c("LWB_header", savestring(header), 0, oh);	

/*
 * rewind file and get relevant header values
*/
    rewind(instrmH);
    while(fgets(header, MAXLINE, instrmH) != NULL) {
	if(header[0] != '#') {/* this is not a comment line*/
	    items_read = sscanf(header, "%s", keyword);
	    if((strcmp(keyword, "sizes")) == ZERO){ /* get number of 
						    channels*/
		items_read=sscanf(header, 
			    "%*s %*s %*d,%d,%*d", &nchan);
		if(items_read == EOF){
		    Fprintf(stderr, 
                    "Error in getting length of data message and number of channels\n");
		    exit(1);
		}
		if(debug_level>0){
		    Fprintf(stderr, 
		    "LWB2ESPS: # read = %d,   nchan = %d\n\n", 
			    items_read,  nchan);
		}
	    }
	    if((strcmp(keyword, "dimension")) == ZERO){/*get number of 
							significant fields 
							in SIZES*/
		items_read = sscanf(header, "%*s %*s %d", &dimn);
		if(items_read == EOF){
		    Fprintf(stderr, 
                    "Error in getting number of data messages.\n");
		    exit(1);
		}
		if(debug_level>0){
		    Fprintf(stderr, "LWB2ESPS: # read=%d, dimn.=%d\n\n",  
			items_read, dimn);
		}
	    }
	    if((strcmp(keyword, "slope")) == ZERO) {/*get sampling
						frequency*/
		slope_flag++;
		if(slope_flag == ONE){
			items_read = sscanf(header, "%*s%*s%f", &sfreq);
        		if(items_read == EOF){
			    Fprintf(stderr, 
			    "Error in getting sampling frequency.\n");
			    exit(1);
			}
			if(debug_level>0){
			   Fprintf(stderr, "LWB2ESPS: sfreq = %f\n\n", sfreq);
			}
		}
	    }
	    if((strcmp(keyword, "conv"))==ZERO){/* get max_value info*/
	    	conv_flag++;
		if(conv_flag == TWO){
		    items_read = sscanf(header, "%*s%*s%*s%f",&conv_gain);
		}
		else{
		    /*do nothing*/;
		}
       		if(items_read == EOF){
		    Fprintf(stderr, 
		    "Error in getting conv_gain.\n");
		    exit(1);
		}
		if(debug_level > 0 && conv_flag == TWO){
		    Fprintf(stderr, 
		    "LWB2ESPS: conv_gain = %f\n\n", conv_gain);
		}
	    }
	    if((strcmp(keyword, "high")) == ZERO){
		high_flag++;
		if(high_flag == TWO){
		items_read = 
		    sscanf(header, "%*s%*s%*s%d",&high_limit);
		}
		else{
		/*do nothing*/;
		}
       		if(items_read == EOF){
		    Fprintf(stderr, 
		    "Error in getting high limit.\n");
		    exit(1);
		}
		if(debug_level > 0 && high_flag == TWO){
		    Fprintf(stderr, 
		    "LWB2ESPS: high_limit = %d,  # items read = %d\n", 
		   high_limit,  items_read);
		}
	    }
	    if((strcmp(keyword, "record:0")) == ZERO){/*determine data type*/
		char *token, *oldtoken;
		token = strtok(header, " ");
		/*
		 * assume last token on string is data type
		*/
		while((token = strtok(0, " \n")) != NULL) {
			if(debug_level>0)
			    Fprintf(stderr, "LWB2ESPS: record token = %s\n",
					     token);	
			oldtoken = token;

		}
		if((strncmp(oldtoken, "ascii_data:", 10)) != ZERO){
        	    if(debug_level>0){
			Fprintf(stderr, "data type is %s\n", token);
		    }
		    binary = 1;
		}
		else
		    ascii = 1;
	    }

	}
    }
/*
 * Compare the number of channels in the file with the channel number 
 * requested
*/
    if(dimn == 1)
	nchan = 1;
    if(channel_num > nchan){
	Fprintf(stderr, "lwb2esps: Only %d channels in input file\n", nchan);
	exit(1);
    }

/*
 * fill in sampled data header items and write header
*/
    nchan_out = nchan;
    max_value = high_limit/conv_gain;
    if(c_flag > ZERO)
	nchan_out = ONE;
    sf =    1.0/sfreq;
    start_time = 0;
    if((init_feasd_hd(oh, FLOAT, nchan_out, &start_time, NO , sf)) != 0){
     Fprintf(stderr, "lwb2esps: Couldn't allocate FEA_SD header - exiting.\n");
     exit(1);
    }
    (void) write_header(oh, ostrm);
    if(debug_level>0){
	Fprintf(stderr, 
	    "max_value = %lf, nchan_out = %d, nchan = %d, sf =%lf\n", 
	    max_value, nchan_out, nchan, sf);
    }

if (ascii == 1){

/*
 * write data to output file
 */
    /*
     * copy all data into sdata 
    */
	sdata = atoarrayf(instrmD, &points, &max);

    if(c_flag == ZERO){
	if(debug_level>0){
	    Fprintf(stderr, "LWB2ESPS: Putting out all data\n");
	}
        data_rec = allo_feasd_recs(oh, FLOAT, points, (char *) NULL, NO);
	fdata = (float *)data_rec->data;
	for(i=0; i<points; i++)
           fdata[i] = sdata[i];
        if((put_feasd_recs(data_rec, 0L, points, oh, ostrm)) != 0){
	  Fprintf(stderr, 
		  "lwb2esps: Couldn't write output data records - exiting.\n");
	  exit(1);
	}

    }
    else {
	/*
	 * Copy desired points to temporary array
	 * before copying to sampled data file
	 * This is done for efficiency resaons
	*/
	float *tmp;
	tmp = (float *) calloc((unsigned)points/nchan, sizeof(float));
	spsassert(tmp != NULL, "Cannot allocate needed space");
	if(debug_level>0){
	    Fprintf(stderr, 
	    "LWB2ESPS: Putting out only channel %d\n", channel_num);
	}
        data_rec = 
	  allo_feasd_recs(oh, FLOAT,(long)(points/nchan), (char *) NULL, NO);
	tmp = (float *)data_rec->data;
	for(i=0;i<points/nchan;i++){
	    tmp[i] = sdata[channel_num-1+i*nchan];
	}
        if((put_feasd_recs(data_rec, 0L, (long)(points/nchan), oh, ostrm)) 
	   != 0){
	  Fprintf(stderr, 
		  "lwb2esps: Couldn't write output data records - exiting.\n");
	  exit(1);
	}
    }
}
else if(binary == 1){
    int n = 0;
    short *shortdata;
    if(c_flag == ZERO){/* Copy all the data*/
	if(debug_level>0){
	    Fprintf(stderr, "LWB2ESPS: Putting out all data\n");
	}
        data_rec = 
	  allo_feasd_recs(oh, SHORT, 512L, (char *) NULL, NO);
	shortdata = (short *)data_rec->data;
	while((n=
	   fread((char *)shortdata, sizeof(short), 512, instrmD)) != 0){
                  if((put_feasd_recs(data_rec, 0L, 512L, oh, ostrm)) 
	               != 0){
	               Fprintf(stderr, 
		  "lwb2esps: Couldn't write output data records - exiting.\n");
	               exit(1);
		     }
		}
      }
    else {/* only copy desired channel*/
	if(debug_level>0){
	    Fprintf(stderr, 
	    "LWB2ESPS: Only putting out channel %d\n, channel_num");
	}
	i = 0;
        data_rec = 
	  allo_feasd_recs(oh, SHORT, 1L, (char *) NULL, NO);
	shortdata = (short *)data_rec->data;
	while(fread((char *)shortdata, sizeof(short), 1, instrmD) != 0){
	    if(i == channel_num - 1 + n*nchan){
	      if((put_feasd_recs(data_rec, 0L, 1L, oh, ostrm)) 
		 != 0){
		Fprintf(stderr, 
		  "lwb2esps: Couldn't write output data records - exiting.\n");
		exit(1);
	      }
		n++;
	    }
	    i++;
	}
    }
}
else{
    Fprintf(stderr, "lwb2esps: Invalid data format - exiting\n");
    exit(1);
}
/*
 * clean up and exit
 */
    exit(0);
/*NOTREACHED*/
}

