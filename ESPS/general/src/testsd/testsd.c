/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987, 1988, 1989, 1990  Entropic Speech, Inc.; 
 *                   All rights reserved"
 *
 * Program:	testsd	
 *
 * Written by:  John Shore
 * Checked by:  Alan Parker
 *		Revised and checked for ESPS 3.0 by John Shore
 *              Revised for complex support and FEA_SD by John Shore
 *
 * This is the testsd program, which generates test data and puts it into
 * a FEA_SD file.  Either a sine wave, Gaussian noise, or a pulse train can
 * be generated.  Data can also be taken from an ASCII file
 */

#ifndef lint
static char *sccs_id = "@(#)testsd.c	3.25	1/5/96 ESI";
#endif
/*
 * include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/feasd.h>
#include <math.h>
/*
 * defines
 */
#define Fprintf (void)fprintf
#define ABS(a) (((a) >= 0) ? (a) : (- a))

#define SYNTAX USAGE ("testsd [-P param_file] [-x debug_level] [-T signal_type] [-f frequency]\n\t[-C sweep_rate] [-A phase] [-l level] [-d decay_time] [-r sampling_rate]\n\t[-{ps} length] [-a asciifile] [-S seed] [-i] [-c] [-t data_type] sdfile")
#define MAXLINE 512	/*max line length for input ascii file*/
#define PI 3.14159265358979323846
#define TWOPI 6.28318530717958647692

#define BIGRAND 2147483647.0	/*maximum value returned by random()*/
#define SEED 1234567	/*seed for random number generator*/

#define BUFLEN BUFSIZ		/* buffer length for test data */
				/* for efficiency, use BUFSIZ from stdio.h */

/*
 * defines for test data type
 */
#define SINE 0
#define GAUSS 1
#define PULSES 2
#define ASCII 3
#define CONSTANT 4
#define SQUARE 5
#define SAWTOOTH 6
#define TRIANGLE 7
#define UNIFORM 8

/*
 * system functions and variables
 */
#ifndef DEC_ALPHA
double sqrt(), exp();
int getopt ();
extern  optind;
extern	char *optarg;
double	atof(), sin(), cos();
int atoi();
long atol();
char *strcpy();
void rewind();
char *calloc();
char *realloc();
char *strtok();
char *fgets();
#endif
/*
 * external ESPS functions
 */
char *get_cmd_line();
char *type_convert();
struct feasd *allo_feasd_recs();
void write_header();
float gauss();
float *atoarrayf();
char *atoarray();
char *savestring();
/*
 * global declarations
 */

double fmod();
#ifndef DEC_ALPHA
long random();
#endif
double sawtooth(), triangle();
double square();
int debug_level = 0;
char *program = "testsd";
char *version = "3.25";
char *date = "1/5/96";
char *sig_types[] = {"SINE", "GAUSS", "PULSES", "ASCII", "CONSTANT", "SQUARE", "SAWTOOTH", "TRIANGLE", "UNIFORM", NULL};
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
  long i;			    /*loop counter for block of samples*/
  long j;			/* sample counter */
  char *param_file = NULL;  /*parameter file name*/
  char *sd_file = NULL;	    /*file name for output SD file*/
  FILE *ostrm = stdout;	    /*output SD file stream*/
  char *infile = NULL;	    /*file name for input ASCII file*/
  FILE *instrm = stdin;	    /* input ASCII file stream */
  struct header *oh;	    /*pointer to SD file header*/
  short data_format;		/*data type for FEA_SD file*/
  int num_type = FLOAT;		/* data type from -t option */
				/* might be modified to complex type*/
				/* by -c option */
  int freq_flag = 0;	    /*flag for -f option*/
  int sec_flag = 0;	    /*flag for -s option*/
  int pnt_flag = 0;	    /*flag for -p option*/
  int a_flag = 0;		/*flag for -a option*/
  int l_flag = 0;		/*flag for -l option*/
  int r_flag = 0;		/*flag for -r option*/
  int seed_flag = 0;	    /*flag for -S option*/
  int T_flag = 0;		/*flag for -T (signal type)  option*/
  int t_flag = 0;		/* flag for -t (data type) option */
  int c_flag = 0;		/*flag for -C option*/
  int C_flag = 0;		/*flag for -C option*/
  int d_flag = 0;		/* flag for -d (data type) option */
  int i_flag = 0;		/*flag for -i option*/
  int A_flag = 0;		/*flag for -A option*/
  float freq = 500;	    /*frequency of periodic signal*/
  float sweep_rate = 0;		/* rate of frequency change */
  int swept = 0;		/* flag for swept generation */
  float ifreq;			/* instantaneous frequency (if swept) */
  float srate = 8000;	    /*sampling rate of SD file*/
  float level = 500;	    /*max level of test data (interp. diff. per type)*/
  float ampl;		    /* actual level (might have decay) */
  float decay_time = 0;		/* decay time (0 means no decay) */
  int decay = 0;		/* flag for decaying amplitude */
  float phase = 0;          /*phase for sines*/
  long points = 8000;	    /*number of points to generate*/
  long c_points;		/* number of points in current block */
  long tot_points = 0;		/* total points written */
  long points_left;		/* remaining points to generate */
  long block_start;		/* first point in current_block  */
  int noise;			/* flag for noise types */
  int periodic;			/* flag for periodic waveforms */
  double delta;		    /*time interval between sampled data points*/
  double cycles;	    /* count cycles in pulse computation */
  long old_cycles = 0;		/* pulse cycles at previous sample */
  float seconds;	    /*number of seconds of data to generate*/
  float rms_amp;	    /*rms amplitude of sine wave or Gaussian noise*/
  int type = SINE;	    /*type of test data*/
  double maxval = 0;	    /*maximum value of generated test data*/
  int seed = SEED;	    /*seed for random numbers*/
  double *fsdata;	    /*array to generated test data*/
  double_cplx *csdata;	    /*array to hold generated complex test data  */
  float value;		    /*used as temp storage for Gauss value*/
  struct feasd *sd_rec;	    /*FEA_SD record structure */
  double start_time=0.0;		/* dummy */

/*
 * process command line options
 */
  if (debug_level) Fprintf (stderr, "testsd: processing options\n");
  while ((c = getopt (argc, argv, "P:x:f:A::l:r:p:s:T:S:a:icC:d:t:")) != EOF) {
    switch (c) {
            case 'P':
		param_file = optarg;
		break;
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	    case 'f':
		freq = atof(optarg);
		freq_flag++;
		break;
	    case 'A':
		phase = TWOPI * atof(optarg) / 360;
		A_flag++;
		break;
	    case 'l':
		level = atof(optarg);
		l_flag++;
		break;
	    case 'r':
		srate = atof(optarg);
		r_flag++;
		break;
	    case 'p':
		points = atol(optarg);
		pnt_flag++;
		break;
	    case 's':
		seconds = atof(optarg);
		sec_flag++;
		break;
	    case 'T':
		T_flag++;
		if((type = lin_search(sig_types, optarg)) == -1) 
		    {
			Fprintf(stderr, "testsd: unknown signal_type %s\n",
			    optarg);
			exit(1);
		    }
		if (debug_level) 
		    Fprintf (stderr, 
			     "testsd: signal_type = %s\n", sig_types[type]);
		break;
	    case 'S':
		seed = atoi(optarg);
		seed_flag++;
		break;
	    case 'a':
		infile = optarg;
		a_flag++;
		type = ASCII;
		break;
	    case 'i':
		i_flag++;
		break;
	    case 'c':
		c_flag++;
		break;
	    case 'C':
		C_flag++;
		sweep_rate = atof(optarg);
		break;
	    case 'd':
		d_flag++;
		decay_time = atof(optarg);
		break;
	    case 't':
		t_flag++;
		num_type = lin_search(type_codes, optarg); 
		break;
	    default:
		SYNTAX;
	}
    }

/*
 * process file argument and open output file
 */
  if (optind < argc) {
    sd_file = argv[optind++];
    if (strcmp (sd_file, "-") == 0)
      sd_file = "<stdout>";
    else
      TRYOPEN (argv[0], sd_file, "w", ostrm);
  }
  else {
    Fprintf(stderr, "testsd: no output file specified.\n");
    SYNTAX;
  }
/*
 * check parameter file for values not on command line and 
 * complete initialization based on options
 */

  (void) read_params(param_file, SC_NOCOMMON, (char *) NULL);

  if (!r_flag) 
    if (symtype("samp_rate") != ST_UNDEF) 
      srate = getsym_i("samp_rate");

  if (!a_flag && !T_flag) 
    if (symtype("type") != ST_UNDEF) {
      type = lin_search(sig_types, getsym_s("type"));
      if (type == -1) {
	Fprintf(stderr, "testsd: unknown test signal type\n");
	exit(1);
      }
    }
 
  if (!t_flag && !i_flag && !c_flag)
    if (symtype("data_type") != ST_UNDEF)
      num_type = lin_search(type_codes, getsym_s("data_type")); 

  if (debug_level) 
    Fprintf(stderr, "testsd: test type is %s\n", sig_types[type]);

   noise = ((type == GAUSS) || (type == UNIFORM));
   periodic = ((type == SINE) || (type == SQUARE) || (type == TRIANGLE) 
              || (type == SAWTOOTH) || (type == PULSES));

  if (!a_flag && (type == ASCII)) {
      if (symtype("ascii_file") != ST_UNDEF) 
	infile = getsym_s("ascii_file");
      else
	infile = savestring("-");
      a_flag++;
      if (strcmp(infile, "-") == 0)
	Fprintf(stdout, 
		"Enter data, end with ^D at the start of a line.\n");
    }

  if (!a_flag && !l_flag) 
    if (symtype("level") != ST_UNDEF) 
      level = getsym_d("level");

  if (noise) { /*get and set the seed*/
    if (!seed_flag) 
      if (symtype("seed") != ST_UNDEF) 
	seed = getsym_i("seed");
    (void) srandom(seed);
  }
  else if ((type != ASCII) && !freq_flag) {
    if (symtype("freq") != ST_UNDEF) 
      freq = getsym_d("freq");
  }

  if (!a_flag && !pnt_flag && !sec_flag) 
    if (symtype("length") != ST_UNDEF) 
      points = getsym_i("length");

  if (!A_flag && periodic)
    if (symtype("phase") != ST_UNDEF) 
      phase = TWOPI * getsym_d("phase") / 360;

  if (!C_flag && periodic)
    if (symtype("sweep_rate") != ST_UNDEF) 
      sweep_rate = getsym_d("sweep_rate");

  if (!a_flag && !d_flag)
    if (symtype("decay_time") != ST_UNDEF) 
      decay_time = getsym_d("decay_time");

/*
 * open input ASCII file for -a option
 */
  if (a_flag) {
    if (strcmp (infile, "-") == 0)
      infile = "<stdin>";
    else 
      TRYOPEN(argv[0], infile, "r", instrm);
  }
/*
 * check for inconsistencies
 */
  if (pnt_flag && sec_flag) {
    Fprintf(stderr, "testsd: can't use both -p and -s\n");
    exit (1);
  }
  if (freq_flag && (type == GAUSS)) {
    Fprintf(stderr, "testsd: can't use -f with type gauss\n");
    exit(1);
  }
  if (a_flag && (pnt_flag || sec_flag)) {
    Fprintf(stderr, "testsd: can't use -p or -s with -a\n");
    exit(1);
  }
  if (a_flag && T_flag) {
    Fprintf(stderr, "testsd: can't use -t with -a\n");
    exit(1);
  }
  if (a_flag && l_flag) {
    Fprintf(stderr, "testsd: can't use -l with -a\n");
    exit(1);
  }

  if (sec_flag) points = seconds * srate;

  if (debug_level && !a_flag) 
    Fprintf(stderr, "testsd: number of points is %d\n", points);

  if (debug_level)
    Fprintf(stderr, "testsd: sampling rate is %g\n", srate);

  /* determine data_format of FEA_SD output */

  if (c_flag) {
    switch(num_type) {
    case BYTE:
      data_format = BYTE_CPLX;
      break;
    case FLOAT:
      data_format = FLOAT_CPLX;
      break;
    case LONG:
      data_format = LONG_CPLX;
      break;
    case SHORT:
      data_format = SHORT_CPLX;
      break;
    case DOUBLE:
      data_format = DOUBLE_CPLX;
      break;
    /*double types are redundent for c_flag*/
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
      break;
    default:
      Fprintf(stderr, "testsd: unsupported data type\n");
      exit(1);
    }
  }
  else {
    switch (num_type) {
    case BYTE:
    case FLOAT:
    case LONG:
    case SHORT:
    case DOUBLE:
      data_format = num_type;
      break;
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
      data_format = num_type;
      c_flag = 1;
      break;
    default:
      Fprintf(stderr, "testsd: unsupported data type\n");
      exit(1);
    }
  }

  /* holdover -i option can still override all for short type */

  if (i_flag) {
    if (c_flag) 
      data_format = SHORT_CPLX;
    else 
      data_format = SHORT;
  }

  /*compute value for max_value in header*/
  switch (type) {
  case SINE:
    maxval = level; /*true for both real and complex*/
    break;
  case SQUARE:
  case TRIANGLE:
  case SAWTOOTH:
  case CONSTANT:
  case PULSES:
  case UNIFORM:
    /*sqrt(2) * level for complex*/
    maxval = (c_flag ? level * 1.414213562 : level);
    break;
  case ASCII: /*for ascii, we'll know when we get there; for 
		gauss, we just don't know*/
  case GAUSS:
    maxval = 0;
    break;
  default:
    Fprintf(stderr, "testsd: unknown test data type\n");
    exit(1);
    break;
  }

/*
 *  create and write FEA_SD header
 */
  if (debug_level) 
    Fprintf(stderr, "testsd: creating ESPS FEA_SD header\n");

  oh = new_header(FT_FEA);
  start_time = 0.0;
  (void) init_feasd_hd(oh, data_format, (int) 1, 
		       &start_time, (int) 0, (double) srate);
  (void) strcpy (oh->common.prog, program);
  (void) strcpy (oh->common.vers, version);
  (void) strcpy (oh->common.progdate, date);
  add_comment (oh, get_cmd_line(argc,argv));
  oh->common.tag = NO;

  *add_genhd_e("test_type", (short *) NULL, 1, sig_types, oh) = type;

  if (noise) *add_genhd_l("seed", (long *) NULL, 1, oh) = seed;

  if (periodic) {
    *add_genhd_f("frequency", (float *) NULL, 1, oh) = freq;
    *add_genhd_f("phase", (float *) NULL, 1, oh) = phase;
  }

  if (!a_flag)
    *add_genhd_f("level", (float *) NULL, 1, oh) = level;

  *add_genhd_d("max_value", (double *) NULL, 1, oh) = maxval;

  if (sweep_rate != 0) 
    *add_genhd_f("sweep_rate", (float *) NULL, 1, oh) = sweep_rate;

  if (decay_time != 0) 
    *add_genhd_f("decay_time", (float *) NULL, 1, oh) = decay_time;

  if (debug_level) 
    Fprintf(stderr, "testsd: writing ESPS FEA_SD header\n");

/* if we are not converting ASCII data, we write the header now; this is 
 * because the output is buffered in the case of non-ascii data.  For 
 * ascii data, the output is not buffered; memory is allocated for the
 * entire data in atoarray(); since atoarray() also returns the max value, 
 * we delay writing the header in the case of ASCII output*/

  if (type != ASCII) 
    (void) write_header(oh, ostrm);

/*
 *  allocate FEA_SD record (we use FLOAT for the in-memory records) 
 */

  if (c_flag) {
    sd_rec = allo_feasd_recs(oh, DOUBLE_CPLX, (long) BUFLEN,(char *) NULL, NO);
    if (sd_rec == NULL) {
      Fprintf(stderr, "testsd: can't allocate FEA_SD records\n");
      exit(1);
    }
    csdata = (double_cplx *) sd_rec->data;
  }
  else {
    sd_rec = allo_feasd_recs(oh, DOUBLE, (long) BUFLEN, (char *) NULL, NO);
   if (sd_rec == NULL) {
      Fprintf(stderr, "testsd: can't allocate FEA_SD records\n");
      exit(1);
    }    
    fsdata = (double *) sd_rec->data;
  }

  if (debug_level) {

    Fprintf(stderr, "testsd: allocated size %ld %s buffer\n", 
	    BUFLEN, (c_flag ? "DOUBLE_CPLX" : "DOUBLE"));
    Fprintf(stderr, 
	    "testsd: output is %d points of signal type %s\n", 
	    points, sig_types[type]);
    if (periodic)
      Fprintf(stderr, 
      "   freq = %g, phase = %g, sweep_rate = %g, decay_time = %g, level = %g\n", 
	      freq, phase, sweep_rate, decay_time, level);    
    if (type == UNIFORM) 
      Fprintf(stderr, "\tuniform noise in interval [-%g,%g]\n", 
	      level, level);
    if (type == GAUSS)
      Fprintf(stderr, "\tnoise with rms level %g\n", level);
  }
  
/*
 * generate test data
 */

  swept = (sweep_rate != 0.0);
  decay = (decay_time != 0.0);

  /* for freq = A and sweep_rate = B, the periodic functions essentially 
   * are f(TWOPI*(A + Bt/2)t), since the "instantaneous 
   * frequency" is A + Bt (derivative of phase); hence for convenience 
   * in the computations, we divide sweep_rate by 2 here; note that the 
   * variable ifreq below isn't really the instantaneous frequency, but
   * rather a sort of average over the entire waveform to that point
   */

  if (swept) 
    sweep_rate /= 2;

  points_left = points;
  block_start = -BUFLEN;

  while (points_left > 0) {
    
    c_points = ( points_left >= BUFLEN ? BUFLEN : points_left);
    points_left -= c_points;
    block_start += BUFLEN;
    if (debug_level > 1) 
      Fprintf(stderr, "testsd: generating %ld points\n", c_points);
    
    switch (type) {
    case SINE:
      /*compute time interval between SD points*/
      delta = 1 / srate;
      /*fill in data array*/
      if (c_flag) {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  csdata[i].real = ampl * cos(TWOPI * j * ifreq * delta + phase);
	  csdata[i].imag = ampl * sin(TWOPI * j * ifreq * delta + phase);
	}
      }
      else {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  value = ampl * sin(TWOPI * j * ifreq * delta + phase);
	  fsdata[i] = value;
	}
      }
      break;
    case SQUARE:
      delta = 1 / srate;
      for (i = 0; i < c_points; i++) {
	j = i + block_start;
	ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	value = ampl * square( j * ifreq * delta + TWOPI *phase);
	if (c_flag) 
	  csdata[i].real = csdata[i].imag = value;
	else
	  fsdata[i] = value;
      }
      break;
    case TRIANGLE:
      delta = 1 / srate;
      for (i = 0; i < c_points; i++)  {
	j = i + block_start;
	ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	value = ampl * triangle( j * ifreq * delta + TWOPI * phase);
	if (c_flag) 
	  csdata[i].real = csdata[i].imag = value;
	else
	  fsdata[i] = value;
      }
      break;
    case SAWTOOTH:
      delta = 1 / srate;
      for (i = 0; i < c_points; i++) {
	j = i + block_start;
	ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	value = level * sawtooth( j * ifreq * delta + TWOPI * phase);
	if (c_flag) 
	  csdata[i].real = csdata[i].imag = value;
	else
	  fsdata[i] = value;
      }
      break;
    case PULSES:
      delta = 1 / srate;
      for (i = 0; i < c_points; i++) {
	j = i + block_start;
	ifreq = (swept ? freq + sweep_rate * j * delta : freq);
	ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	cycles = j * ifreq * delta + phase/TWOPI;
	if ((long) cycles > old_cycles) {
	  value = ampl;
	  old_cycles = (long) cycles;
	}
	else 
	  value = 0;

	if (cycles == 0) /*special case for first point*/
	  value = ampl;

	if (c_flag) 
	  csdata[i].real = csdata[i].imag = value;
	else
	  fsdata[i] = value;
      }
      break;
    case GAUSS:
      /*compute time interval between SD points*/
      delta = 1 / srate;
      /*fill in data array*/
      if (c_flag) {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  rms_amp = (decay ? level * exp(- j * delta / decay_time) : level);
	  value = rms_amp * gauss();
	  csdata[i].real = value;
	  value = rms_amp * gauss();
	  csdata[i].imag = value;
	}
      }
      else {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  rms_amp = (decay ? level * exp(- j * delta / decay_time) : level);
	  value = rms_amp * gauss();
	  fsdata[i] = value;
	}
      }
      break;
    case UNIFORM:
      /*compute time interval between SD points*/
      delta = 1 / srate;
      /*fill in data array*/
      if (c_flag) {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  value = 2.0*ampl*((float)random() / BIGRAND - .5);
	  csdata[i].real = value;
	  value = 2.0*ampl*((float)random() / BIGRAND - .5);
	  csdata[i].imag = value;
	}
      }
      else {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  value = 2.0*ampl*((float)random() / BIGRAND - .5);
	  fsdata[i] = value;
	}
      }
      break;
    case CONSTANT:
      if (c_flag) {
	for (i = 0; i < c_points; i++) {
	  j = i + block_start;
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  csdata[i].real = csdata[i].imag = ampl;
	}
      }
      else {
	maxval = level;
	for (i = 0; i < c_points; i++)  {
	  ampl = (decay ? level * exp(- j * delta / decay_time) : level);
	  fsdata[i] = ampl;
	}
      }
      break;
    case ASCII:  
      if (c_flag) 
	sd_rec->data =  atoarray(instrm, DOUBLE_CPLX, &c_points, &maxval);
      else
	sd_rec->data =  atoarray(instrm, DOUBLE, &c_points, &maxval);
      
      if (debug_level) 
	Fprintf(stderr, 
	  "testsd: read %ld %s points from ASCII file %s with maxval %g\n", 
		c_points, (c_flag ? "complex" : "real"), infile, maxval);

      /* note that the allocated sd_rec->data is now irrelevant; furthermore
       * we must change num_records to be the amount returned by atoarray*/

      sd_rec->num_records = c_points; 

      /* now we can fill in max_value and write the header */
      
      *add_genhd_d("max_value", (double *) NULL, 1, oh) = maxval;

      (void) write_header(oh, ostrm);

      /*make sure loop stops since atoarray reads it all in*/


      points_left = 0;
      break;
    default:
      Fprintf(stderr, "testsd: unknown test data type\n");
      exit(1);
      break;
    }
    
    /* note that put_feasd_recs does the appropriate conversion 
     * for the type specified in the header */
    
    (void) put_feasd_recs(sd_rec, 0L,  c_points, oh, ostrm);
    
    tot_points += c_points;

    /*end of output loop*/
    
  }

/*
 * put info in ESPS common
 */

  if (strcmp(sd_file, "<stdout>") != 0) {
    (void) putsym_s("filename", sd_file);
    (void) putsym_s("prog","testsd");
    (void) putsym_i("start", (int) 1);
    (void) putsym_i("nan", (int) tot_points);
  }

/*
 * clean up and exit
 */
  exit(0);
}

double
square (angle)
double angle;
{
  angle = fmod(angle, 1.0); 

  if (angle < 0.5)
    return 1.0;

  return -1.0;
}

double
triangle(angle)
double angle;
{
  angle = fmod(angle, 1.0); 
 
  if (angle <= 0.25)
    return (4 * angle);

  if ((angle > 0.25) && (angle <= 0.75))
    return (2 - 4 * angle);

  return (-4.0 + 4 * angle);
}

double
sawtooth(angle)
double angle;
{
  return(fmod(angle, 1.0)); 
}





