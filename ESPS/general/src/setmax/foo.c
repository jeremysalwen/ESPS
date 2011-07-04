/*----------------------------------------------------------------------+
|									
|   This material contains proprietary software of Entropic Speech,	
|   Inc.  Any reproduction, distribution, or publication without the	
|   prior written permission of Entropic Speech, Inc. is strictly	
|   prohibited.  Any public distribution of copies of this work		
|   authorized in writing by Entropic Speech, Inc. must bear the	
|   notice								
|									
|   "Copyright (c) 1989 Entropic Speech, Inc.  All rights reserved."	
|									
+-----------------------------------------------------------------------+
|									
|  Program: setmax.c
|									
|  This program takes a pure FEA_SD file, finds the maximum and minimum
|  values in the file, and puts the larger of the two absolute values 
|  in the max_value SD header field.
|									
|  John Shore, Entropic Speech, Inc.; based on frame.c
|									
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)setmax.c	1.5	2/22/90	ESI";
#endif
#define VERSION "1.5"
#define DATE "2/22/90"

#define BUFSIZE 8192

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/fea.h>

#define REQUIRE(test,text) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", ProgName, text); exit(1);}}
#define SYNTAX \
USAGE("setmax [-x debug_level] input.sd output.sd") ;


int	get_sd_recd();
double  fabs();
char	*get_cmd_line();

int	findlim();

char	*ProgName = "setmax";
char	*Version = VERSION;
char	*Date = DATE;
int     debug_level = 0;		/* debug level; global for library*/
double  *buf = NULL;			/* buffer for sampled data */

/*
 * MAIN PROGRAM
 */

main(argc, argv)
    int  argc;
    char **argv;
{
    extern int
	    optind;		/* for use of getopt() */
    extern char
	    *optarg;		/* for use of getopt() */
    int	    ch;			/* command-line option letter */
    char    *iname;		/* input file name */
    FILE    *ifile;		/* input stream */
    struct header
	    *ihd;		/* input file header */
    char    *oname;		/* output file name */
    FILE    *ofile;		/* output stream */
    struct header
	    *ohd;		/* output file header */
    int     num_read = 0;		/* number samples read  */
    double  max_value = -DBL_MAX;
    double  min_value = DBL_MAX;
    
/* Parse command-line options. */

    while ((ch = getopt(argc, argv, "x:")) != EOF)
        switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	default:
	    SYNTAX
	    break;
	}

/* Process file names and open files. */

    if (argc - optind > 2) {
	Fprintf(stderr,
	    "%s: too many file names specified.\n", ProgName);
	SYNTAX
    }
    if (argc - optind < 2) {
	Fprintf(stderr,
	    "%s: too few file names specified.\n", ProgName);
	SYNTAX
    }
    iname = eopen(ProgName,
	    argv[optind++], "r", FT_FEA, FEA_SD, &ihd, &ifile);
    oname = eopen(ProgName,
	    argv[optind++], "w", NONE, NONE, &ohd, &ofile);

    if (debug_level)
	Fprintf(stderr, "%s: Input file = %s\n\tOutput file = %s\n",
	    ProgName, iname, oname);

    REQUIRE(get_fea_siz("samples", ihd, (short) NULL, (short) NULL) == 1,
	    "sorry, can't deal with multi-channel files yet");

    REQUIRE(is_file_complex(ihd) == 0, 
	    "sorry, can't deal with complex data yet");

/* Allocate buffer */

    buf = (double *) calloc((unsigned) BUFSIZE, sizeof(double));
    REQUIRE( buf != NULL, "can't allocate memory for input data");

    if (findlim(&ifile, &ihd, &max_value, &min_value) == 0) {
      Fprintf(stderr, "%s: no records in %s or error reading it - exiting.\n",
	      ProgName, iname);
      Fclose(ofile);
      Fclose(ifile);
      exit (1);
    }

    if (debug_level) {
	Fprintf(stderr, "%s: max_value = %lg\n", ProgName, max_value);
	Fprintf(stderr, "%s: min_value = %lg\n", ProgName, min_value);
    }

/* Create output-file header */

    ohd = copy_header(ihd);
    add_source_file(ohd, iname, ihd);
    (void) strcpy(ohd->common.prog, ProgName);
    (void) strcpy(ohd->common.vers, Version);
    (void) strcpy(ohd->common.progdate, Date);
    ohd->variable.refer = ihd->variable.refer;
    add_comment(ohd, get_cmd_line(argc, argv));
    *add_genhd_d("max_value", (double *) NULL, 1, ohd) = 
      MAX(fabs(max_value), fabs(min_value));

    if (debug_level)
	Fprintf(stderr, "%s: writing output header to file\n", ProgName);
    update_waves_gen(ihd, ohd, 1.0, 0.0);
    write_header(ohd, ofile);

    /* copy data  */

    do  {
	num_read = get_sd_recd(buf, BUFSIZE, ihd, ifile);
	if (num_read != 0) put_sd_recd(buf, num_read, ohd, ofile);
    } while (num_read == BUFSIZE);

    Fclose(ofile);
    Fclose(ifile);

    exit(0);
    /*NOTREACHED*/
}    

int
findlim(file, hd, max, min)
    FILE **file;		
    struct header **hd;
    double *max, *min;
/* find max and min of data in file; if input is a pipe, then 
   write data to temporary file and change the corresponding file
   pointer; this is so the data can be read again in order to copy 
   it from input to output in the main program*/
{
  FILE	*tmpstrm;		/* temp file; might be return to main */
  int     first = 1;		/* flag for first time read from file */
  int i;
  int num_read = 0;
  if ((*hd)->common.ndrec == -1) {
    tmpstrm = tmpfile();
    if (debug_level)
      Fprintf(stderr, "%s: input is pipe; creating temp file\n", ProgName);
  }
  do  {
    num_read = get_sd_recd(buf, BUFSIZE, *hd, *file);
    if (debug_level > 1) 
      Fprintf(stderr, "%s: num_read = %d\n", ProgName, num_read);
    if (first) {
      if (num_read == 0) return 0;
      first = 0;
    }
    for (i = 0; i < num_read; i++) {
      if (buf[i] < *min) *min = buf[i];
      if (buf[i] > *max) *max = buf[i];
    }
    if (num_read != 0 && (*hd)->common.ndrec == -1) {
      put_sd_recd(buf, num_read, *hd, tmpstrm);
      if (debug_level > 1) 
	Fprintf(stderr, "%s: wrote %d samples to temp file\n",
		ProgName, num_read);
    }
  } while (num_read == BUFSIZE);
  
  if ((*hd)->common.ndrec == -1)	{    /* Input was a pipe */
    (void) rewind(tmpstrm);
    *file = tmpstrm;
  }
  else {
    (void) rewind(*file);
    *hd = read_header(*file);
  }
  return 1;
}


