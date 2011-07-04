/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1988-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by: Alan Parker 
 * Checked by:
 * Revised by: Ken Nelson
 *
 * Brief description: addgen - add generic header item to an ESPS file.
 *
 */

static char *sccs_id = "@(#)addgen.c	1.21	10/1/98	ESI/ERL";

int debug_level = 0;

#include <stdio.h>
#include <pwd.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

#define MAXGEN 100
#define SYNTAX \
USAGE("addgen [-g generic_name]... [-t generic_type]... [-v generic_value]...\n\t[-x debug_level] [-F] [-P params] espsfile.in [espsfile.out]\n")

extern int  optind;
extern char *optarg,
	    *mktemp(), *ctime();
extern char *type_codes[];
char	    *savestring();
char	    *get_username();
char	    *e_temp_name();
char	    *get_sphere_hdr();
char	    *get_cmd_line();

char *Program = "addgen";
char *Date = "10/1/98";
char *Version = "1.21";

main (argc, argv)
char  **argv;
int     argc;
{
    int     getopt (), c, X;
    int     gflag = 0;		/* flag for generic name */
    int     tflag = 0;		/* flag for generic type */
    int     vflag = 0;		/* flag for generic value */
    int     gcount = 0;		/* count for generic name */
    int     tcount = 0;		/* count for generic name */
    int     vcount = 0;		/* coutn for generic name */
    int     Fflag = 0;          /* flag for overwriting existing generic */
    long    time (), tloc;	/* needed to get time stamp */
    FILE * in;			/* input file stream */
    FILE * out;			/* output file stream */
    FILE * tstrm;		/* temporary file stream */
    FILE * fopen ();
    struct header   *ih;	/* ptr to input file header */
    struct fea_data *fea_rec;	/* ptr to input data record */
    struct header   *oh;	/* ptr to output file header */
    char   *template = "adgXXXXXX";
    char   *tmp_file;
    char   *param = NULL;	/* holds parameter file name */
    char   *gname[MAXGEN];	/* holds generic name */
    char   *gtype[MAXGEN];	/* holds generic type */
    char   *gvalue[MAXGEN];	/* holds generic value */
    double  value_d;		/* holds generic value */
    float   value_f;		/* holds generic value */
    long    value_l;		/* holds generic value */
    short   value_s;		/* holds generic value */
    char   *value_c;		/* holds generic value */
    int     num_files = 0;	/* holds number of command line file names */
    char    combuf[100];	/* comment buffer */
    int new_file = 0;           /* true if a new file is to be created */
    int    i;

    while ((c = getopt (argc, argv, "g:t:v:x:FP:")) != EOF) {
      switch (c) {
      case 'g': 
	gname[gcount] = optarg;
	gflag++;
	gcount++;
	break;
      case 't': 
	gtype[tcount] = optarg;
	tflag++;
	tcount++;
	break;
      case 'v': 
	gvalue[vcount] = optarg;
	vflag++;
	vcount++;
	break;
      case 'x':
	debug_level = atoi(optarg);
	break;
      case 'F':
	Fflag++;
	break;
      case 'P': 
	param = optarg;
	break;
      default: 
	SYNTAX;
      }
    }
    
    if(tcount!=vcount || gcount != vcount){
      Fprintf(stderr, "%s: inconsistent number of -g, -t, and -v options.\n",
	      Program);
      SYNTAX;
      exit(1);
    }
    
    /* Open files and read input file header*/
    num_files = argc - optind;
    if (num_files < 1 || num_files > 2)
      SYNTAX;
    
    /* Open input file and read header */
    if (strcmp (argv[optind], "-") == 0)
      in = stdin;
    else {
      if ((in = fopen (argv[optind], "r")) == NULL) {
	if (num_files == 1) {
	  TRYOPEN(Program, argv[optind], "w", out);
	  new_file = 1;
	}
	else {
	  Fprintf (stderr, "%s: Cannot open input file %s.\n",
		   Program, argv[optind]);
	  exit (1);
	}
      }
    }
    
    if (!new_file) {
      if (!(ih = read_header (in)))
	NOTSPS(Program, argv[optind]);
      if (get_esignal_hdr(ih) || get_sphere_hdr(ih) || get_pc_wav_hdr(ih))
      {
	(void) fprintf(stderr, "%s: input file %s is not an ESPS FEA file.\n",
		       Program, argv[optind]);
	exit(1);
      }

      oh = copy_header(ih);
/*
      oh->common.edr = ih->common.edr;
*/
    }
    else {
      oh = new_header(FT_FEA);
      if(add_fea_fld("ADDGEN",1L,0,(long *)NULL,SHORT,(char **)NULL,oh) != 0)
	Fprintf(stderr,
		"%s: cannot create dummy field - will try to continue.\n",
		Program);
    }

    (void) strcpy(oh->common.prog, Program);
    (void) strcpy(oh->common.progdate, Date);
    (void) strcpy(oh->common.vers, Version);
    
    if (num_files == 2) {	/* make sure input and output files are  
				   different */
      char   *tmp = argv[optind];
      if (strcmp (tmp, argv[++optind]) == 0 && strcmp (tmp, "-") != 0) {
	Fprintf (stderr,
		 "addgen: Input and output filename should not be identical - exiting.\n");
	exit (1);
      }
    }

    /* Allocate data record, if needed */

    if (!new_file)
    {
      fea_rec = allo_fea_rec(ih);
    }

    /* Open temporary file, if needed */

    if (!new_file && num_files == 1) 
    {
      tmp_file = e_temp_name(template);

      if ((tstrm = fopen(tmp_file, "w+")) == NULL)
	CANTOPEN(Program, tmp_file);

      (void) unlink(tmp_file);
    }

    /* read parameter file and get option values if needed*/

    (void) read_params (param, SC_NOCOMMON, (char *) NULL);

    if (gflag == 0) {		/* generic name not specified on command */
      if (symtype ("generic_name") == ST_UNDEF) {
	Fprintf (stderr,
		 "addgen: Name of generic item not specified - exiting\n");
	exit (1);
      }
      else{
	gname[gcount] = getsym_s ("generic_name");
	gcount++;
      }
    }
    
    
    if (tflag == 0) {		/* generic type not specified on command */
      if (symtype ("generic_type") == ST_UNDEF) {
	Fprintf (stderr,
		 "addgen: Type of generic item not specified - exiting\n");
	exit (1);
      }
      else{
	gtype[tcount] = getsym_s ("generic_type");
	tcount++;
      }
    }
    
    
    if (vflag == 0) {		/* generic value not specified on command */
      
      if (symtype ("gen_value_f") == ST_UNDEF && symtype ("gen_value_i")
	  == ST_UNDEF && symtype ("gen_value_s") == ST_UNDEF) {
	Fprintf (stderr,
		 "addgen: value of generic item not specified - exiting\n");
	exit (1);
      }
      else{
	gvalue[vcount] = (char *) malloc( sizeof(char)*100);
	spsassert(gvalue[vcount], "gvalue[vcount] malloc failed");
	if (symtype("gen_value_f") != ST_UNDEF)
	  sprintf( gvalue[vcount], "%g", getsym_d("gen_value_f"));
	else if (symtype("gen_value_i") != ST_UNDEF)
	  sprintf( gvalue[vcount], "%d", getsym_i("gen_value_i"));
	else if (symtype("gen_value_s"))
	  gvalue[vcount] = getsym_s("gen_value_s");
	vcount++;
      }
    }

    if( gcount > MAXGEN ){
      Fprintf(stderr, "%s: too many generic header items---more than %d.\n",
	      Program, MAXGEN);
      exit(1);
    }

    /* add comments*/
    add_comment (oh, get_cmd_line(argc, argv));
      
    
    for(i=0; i< gcount; i++) {
      
      /* 
       * Check for existence of generic header item
       */
      
      {
	int size;
	if(genhd_type(gname[i], &size, ih) != HD_UNDEF && !Fflag){
	  Fprintf(stderr, "Generic header item %s already exists.\nIf you wish to overwrite it, use the -F option.\n", gname[i]);
	  exit(1);
	}
      }

      /* Get generic value in appropriate format*/
      
      switch ((X = lin_search (type_codes, gtype[i]))) {
      case DOUBLE: 
	value_d = atof (gvalue[i]);
	break;
      case FLOAT: 
	value_f = atof (gvalue[i]);
	break;
      case LONG: 
	value_l = atol (gvalue[i]);
	break;
      case SHORT: 
	value_s = atoi (gvalue[i]);
	break;
      case CHAR: 
	value_c = gvalue[i];
	break;
      case EFILE: 
      case AFILE: 
	value_c = gvalue[i];
	break;
      default: 
	Fprintf (stderr,
		 "addgen: Invalid generic header type\n");
	exit (1);
      }
      

      /* add generic header item */
      
      switch (X) {
	case DOUBLE: 
	    *add_genhd_d(gname[i], (double *) NULL, 1, oh) = value_d;
	    break;
	case FLOAT: 
	    *add_genhd_f(gname[i], (float *) NULL, 1, oh) = value_f;
	    break;
	case LONG: 
	    *add_genhd_l(gname[i], (long *) NULL, 1, oh) = value_l;
	    break;
	case SHORT: 
	    *add_genhd_s(gname[i], (short *) NULL, 1, oh) = value_s;
	    break;
	case CHAR: 
	    (void) add_genhd_c(gname[i], savestring(value_c), 0, oh);
	    break;
	case EFILE: 
	    (void) add_genhd_efile(gname[i], value_c, oh);
	    break;
	case AFILE: 
	    (void) add_genhd_afile(gname[i], value_c, oh);
	    break;
	default: 
	    Fprintf (stderr, "addgen: Invalid generic header type\n");
	    exit (1);
	  }
    }

    if (!new_file && num_files == 1) {	/* keep input file name */
      /* write header and data to temporary file */
      inhibit_hdr_date();
      (void) write_header(oh, tstrm);

      while (get_fea_rec(fea_rec, ih, in) != EOF)
	put_fea_rec(fea_rec, oh, tstrm);
/*
      while ((c = getc(in)) != EOF)
	(void) putc(c, tstrm);
*/

      /* close input */
      (void) fclose(in);
      
      /* rewind temporary file */
      rewind(tstrm);
      
      /* open input file for writing */
      TRYOPEN(Program, argv[optind], "w", out);
      
      /* copy temporary file to input file name */
      while ((c = getc(tstrm)) != EOF)
	(void) putc(c, out);
    }
    else if (new_file) {
      write_header(oh, out);
      /* no data in this case */
    }
    else {			/* separate input and output file names */
      /* open output file for writing */
      
      if (strcmp(argv[optind], "-") == 0)
	out = stdout;
      else
	TRYOPEN(Program, argv[optind], "w", out);

      /* write header and data to output file */
      
      (void) write_header(oh, out);

      while (get_fea_rec(fea_rec, ih, in) != EOF)
	put_fea_rec(fea_rec, oh, out);
/*
      while ((c = getc (in)) != EOF)
	(void) putc (c, out);
*/
    }
    
    exit(0);
    /* NOTREACHED */
  }

char *
get_username()
{
  char   *user, *getlogin();
  struct passwd *pass, *getpwnam(), *getpwuid();
  int userid;
  if ((user = getlogin()) == NULL || (pass = getpwnam(user)) == NULL) {
    userid = getuid();
    pass = getpwuid(userid);
    user = pass->pw_name;
  }
  return(savestring(user));
}
