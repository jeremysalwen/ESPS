/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:  John Shore, Rod Johnson, Derek Lin
 *
 * Brief description: merges two feature files 
 *
 */

static char *sccs_id = "@(#)mergefea.c	1.16	3/14/94	ESI/ERL";


#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>

#define REQUIRE(test,text) {if (!(test)) {Fprintf(stderr, \
"mergefea: %s - exiting\n", text); exit(1);}}

#define SYNTAX \
USAGE("mergefea [-f field_name]... [-t] [-u] [-a] [-x debug_level] [-z] [-T subtype]\n fea1 fea2 [fea3]") ;

FILE   *fopen ();

extern int  optind;
extern char *optarg;
char   *get_cmd_line ();
void copy_fea_rec ();
int copy_genhd();
int copy_genhd_uniq();
int     debug_level = 0;

#define error(s) error2(s," ")
void
error2 (s1, s2)
char   *s1, *s2;
{
    Fprintf(stderr, "mergefea: %s %s\n", s1, s2);
    Fprintf(stderr, "mergefea: exiting.\n");
    exit (1);
}


#define FIELD_SIZ_INC 50

main (argc, argv)
     int     argc;
     char  **argv;
{
  char   *fea1 = NULL;		/* names of argument files */
  char   *fea2 = NULL;
  char   *fea3 = NULL;
  
  FILE   *fea1_strm = NULL;	/* streams for argument files */
  FILE   *fea2_strm = NULL;
  FILE   *fea3_strm = NULL;
  
  struct header  *fea1_hd = NULL;	/* headers for argument files */
  struct header  *fea2_hd = NULL;
  struct header  *fea3_hd = NULL;
   
  struct fea_data *fea1_rec;	/* fea record structures */
  struct fea_data *fea2_rec;
  struct fea_data *fea3_rec;
  
  char    *subtype = NULL;
  int     subtype_code = NONE;
  
  int     update = 0;
  int     new_fea2 = 0;
  int     c;
  
  char    *prog = "mergefea";
  char    *DATE = "3/14/94";
  char    *VERS = "1.16";
  
  int     num_fields = 0;
  int     field_name_size=0;
  char    **field_name, **field_name_ptr;
  int     t_flag = 0, uflag = 0, z_flag = 0, a_flag = 0;
  double  rec_freq1 = -99, rec_freq2 = -99, start_time1, start_time2;
  float   fratio = 1.0;
  int     idx1 = 0, idx2 = 0;
  int     i;
  
  field_name = (char **) calloc(FIELD_SIZ_INC + 1, sizeof (char *));
  REQUIRE(field_name != NULL, "can't allocate memory for field name");
  field_name_size = FIELD_SIZ_INC;
 
  while ((c = getopt(argc, argv, "f:tux:azT:")) != EOF) {
    switch (c) {
    case 'f': 
      if (num_fields == field_name_size){
	field_name_size += FIELD_SIZ_INC;
	field_name = (char **)realloc((char *)field_name,
			      (unsigned) 1 + field_name_size*sizeof (char *));
	REQUIRE(field_name != NULL, "can't reallocate memory for field name");
      }
      field_name[num_fields++] = optarg;
      break;
    case 't':
      t_flag = 1;
      break;
    case 'u':
      uflag = 1;
      break;
    case 'x': 
      debug_level = atoi (optarg);
      break;
    case 'z':
      z_flag = 1;
      break;
    case 'T':
      subtype = optarg;
      break;
    case 'a':
      a_flag = 1;
      break;
    default:
      SYNTAX
      break;
    }
  }
 
  if ((argc - optind) < 2) {
    Fprintf(stderr, "mergefea: at least two files are required\n");
    SYNTAX;
    exit(1);
  }

  if ((argc - optind) > 3) {
    Fprintf(stderr, "mergefea: no more than three files are allowed\n");     
    SYNTAX;
    exit(1);
  }

  if ((argc - optind) == 3)
    fea3 = argv[optind + 2];
  
  field_name[num_fields] = NULL;
  
  if (debug_level) for(i=0; i<num_fields; i++)
    Fprintf(stderr,"field_name[%d]: %s\n",i,field_name[i]);
  
  fea1 = argv[optind];
  fea2 = argv[optind + 1];
  
  if (fea3 && !strcmp(fea1,"-") && !strcmp(fea2,"-")) {
    Fprintf(stderr, "mergefea: can't use \"-\" for both fea1 and fea2 when fea3 specified\n");
    SYNTAX;
    exit(1);
  }

  fea1 = eopen (prog, fea1, "r", FT_FEA, NONE, &fea1_hd, &fea1_strm);
  fea1_rec = allo_fea_rec (fea1_hd);
  
  if (t_flag && !fea1_hd->common.tag) {
    Fprintf(stderr,
	    "mergefea: Warning: -t used but input file %s has no tag.\n",
	    fea1);
    t_flag = 0;
  }
  
  for (i=0; i<num_fields; i++)
    if (get_fea_type(field_name[i], fea1_hd) == UNDEF) {
      Fprintf(stderr,"mergefea: field %s not in input file %s.\n",
		    field_name[i], fea1);
      Fprintf(stderr,"mergefea: exiting.\n");
      exit(1);
    }
  
  if (fea3) {
    fea3 = eopen (prog, fea3, "w", FT_FEA, NONE, &fea3_hd, &fea3_strm);
    fea2 = eopen (prog, fea2, "r", FT_FEA, NONE, &fea2_hd, &fea2_strm);
    fea2_rec = allo_fea_rec (fea2_hd);
    if (debug_level)
	Fprintf(stderr, "mergefea: merging %s into %s to create %s.\n",
		fea1, fea2, fea3);
  }
  else {
    if ((fea2_strm = fopen (fea2, "r")) == NULL) {
      new_fea2 = 1;
      fea2 = eopen (prog, fea2, "w", FT_FEA, NONE,
		    &fea2_hd, &fea3_strm);
      if (debug_level)
	Fprintf(stderr, "mergefea: selecting fields from %s to create %s.\n",
		fea1, fea2);
    }
    else {
      /* update */
      if (fclose (fea2_strm) == EOF)
	error2("I/O error closing update file", fea2);
      fea2 = eopen (prog, fea2, "r", FT_FEA, NONE,
		    &fea2_hd, &fea2_strm);
      fea2_rec = allo_fea_rec (fea2_hd);
      update = 1;
      fea3_strm = tmpfile ();
      if (debug_level)
	Fprintf(stderr, "mergefea: merging %s into %s.\n",
		fea1, fea2);
    }
  }

  fea3_hd = new_header(FT_FEA);
  
  if (new_fea2)
  {
      if (debug_level)
	Fprintf(stderr, "new_fea2\n");
      (void)copy_genhd(fea3_hd, fea1_hd, (char *) NULL);
      add_source_file(fea3_hd, fea1, fea1_hd);
  }
  else
  {
      copy_fea_fields(fea3_hd, fea2_hd);
      fea3_hd->hd.fea->fea_type = fea2_hd->hd.fea->fea_type;
      if (uflag)
      {
	  (void)copy_genhd(fea3_hd, fea2_hd, (char *) NULL);
	  (void)copy_genhd_uniq(fea3_hd, fea1_hd, (char *) NULL);
      }
      else
      {
	  (void)copy_genhd(fea3_hd, fea1_hd, (char *) NULL);
	  (void)copy_genhd(fea3_hd, fea2_hd, (char *) NULL);
      }
      if ( !z_flag
	  && genhd_type("record_freq", NULL, fea1_hd) != HD_UNDEF
	  && genhd_type("record_freq", NULL, fea2_hd) != HD_UNDEF
	  && (rec_freq1 = get_genhd_val("record_freq", fea1_hd, 1.0))
	     != (rec_freq2 = get_genhd_val("record_freq", fea2_hd, 1.0)))
      {
	if( !a_flag) 
	  Fprintf(stderr,"%s: Warning: record_freq values %g, %g differ.\n", 
		   argv[0], rec_freq1, rec_freq2);
      }
      if( a_flag ){
	i = 1;
	REQUIRE( genhd_type("start_time", &i, fea1_hd) != HD_UNDEF &&
		genhd_type("start_time", &i, fea2_hd) != HD_UNDEF, "start_time header items must be defined in input files for the -a option.");
	start_time1 = *get_genhd_d("start_time", fea1_hd);
	start_time2 = *get_genhd_d("start_time", fea2_hd);
	REQUIRE( genhd_type("record_freq", &i, fea1_hd) != HD_UNDEF &&
		genhd_type("record_freq", &i, fea2_hd) != HD_UNDEF, "record_freq header items must be defined in input files for the -a option");
	rec_freq1 = *get_genhd_d("record_freq", fea1_hd);
	rec_freq2 = *get_genhd_d("record_freq", fea2_hd);
	REQUIRE( rec_freq1 !=0 && rec_freq2 !=0, 
		"record_freq header items must be non-zero");
	if(start_time1 >= start_time2)
	  add_genhd_d("start_time", &start_time1, 1, fea3_hd);
	else
	  add_genhd_d("start_time", &start_time2, 1, fea3_hd);
      }
      add_source_file(fea3_hd, fea2, fea2_hd);
      add_source_file(fea3_hd, fea1, fea1_hd);
  }
 
  (void) strcpy(fea3_hd->common.prog, prog);
  (void) strcpy(fea3_hd->common.progdate, DATE);
  (void) strcpy(fea3_hd->common.vers, VERS);
  add_comment(fea3_hd, get_cmd_line(argc, argv));
 
  fea3_hd->common.tag = (t_flag || !new_fea2 && fea2_hd->common.tag);
  if (debug_level) 
      Fprintf(stderr,
	      "mergefea: t_flag %d, new_fea2 %d, fea2_hd->common.tag %d.\n", 
	      t_flag, new_fea2, fea2_hd->common.tag);

  if (num_fields == 0) {
    copy_fea_fields (fea3_hd, fea1_hd);
    field_name_ptr = NULL;
  }
  else {
    field_name_ptr = field_name;
    for(i=0; i<num_fields; i++) {
      int stat;
      if (debug_level)
	Fprintf(stderr,"copy_fea_fld(fea3_hd, fea1_hd, %s)\n",
		      field_name[i]);
      stat = copy_fea_fld(fea3_hd, fea1_hd, field_name[i]);
      if (debug_level)
	Fprintf(stderr, "return from copy_fea_fld: %d\n",stat);
    }
  }
 
  if (subtype) {
      if ((subtype_code = lin_search(fea_file_type, subtype)) == -1)
	  Fprintf(stderr, "mergefea: unknown FEA file subtype %s ignored.\n",
		  subtype);
      else
	  fea3_hd->hd.fea->fea_type = subtype_code;
  }

  fea3_rec = allo_fea_rec (fea3_hd);
  write_header (fea3_hd, fea3_strm);

  if (new_fea2)
    while (get_fea_rec(fea1_rec, fea1_hd, fea1_strm) != EOF) 
    {
	copy_fea_rec(fea1_rec, fea1_hd, fea3_rec,
		     fea3_hd, (char **) NULL, (short **) NULL);
	if (t_flag)
	    fea3_rec->tag = fea1_rec->tag;
	put_fea_rec(fea3_rec, fea3_hd, fea3_strm);
    }
  else{
    if( a_flag){
      long n1, n2, n1_should_be;
      double stime;
      if((stime = start_time1) < start_time2){
	idx1 = 0.5 + (start_time2 - stime) * rec_freq1;
	stime = start_time2;
      }
      else
	idx2 = 0.5 + (stime - start_time2) * rec_freq2;
      for(i=0, n1=idx1-1; i< idx1; i++) 
	REQUIRE(get_fea_rec(fea1_rec, fea1_hd, fea1_strm) !=EOF, 
		"insufficient number of data in the first input file");
      for(i=0, n2=idx2-1; i< idx2; i++)
	REQUIRE(get_fea_rec(fea2_rec, fea2_hd, fea2_strm) !=EOF,
		"insufficient number of data in the second input file");
      fratio = rec_freq1 / rec_freq2;

      if( fratio <= 1){           /* fea2 is faster, fea1 is rep to fea2, #? */
	while (get_fea_rec(fea2_rec, fea2_hd, fea2_strm) != EOF){
	  n2++;
	  n1_should_be = idx1 + n2 * fratio;
	  if( n1 < n1_should_be){
	    if(get_fea_rec(fea1_rec, fea1_hd, fea1_strm) == EOF) 
	      break;
	    n1++;
	  }
	  copy_fea_rec(fea2_rec, fea2_hd, fea3_rec,
		       fea3_hd, (char **) NULL, (short **) NULL);
	  copy_fea_rec(fea1_rec, fea1_hd, fea3_rec,
		       fea3_hd, field_name_ptr, (short **) NULL);
	  if (t_flag)
	    fea3_rec->tag = fea1_rec->tag;
	  if (!t_flag && fea3_hd->common.tag)
	    fea3_rec->tag = fea2_rec->tag;
	  put_fea_rec (fea3_rec, fea3_hd, fea3_strm);
	}
      }
      if( fratio > 1){          /* fea2 is slower */
	while (get_fea_rec(fea2_rec, fea2_hd, fea2_strm) != EOF){
	  n2++;
	  n1_should_be = idx1 + n2 * fratio;
	  while( n1 < n1_should_be){
	    if(get_fea_rec(fea1_rec, fea1_hd, fea1_strm) == EOF) 
	      break;
	    copy_fea_rec(fea2_rec, fea2_hd, fea3_rec,
			 fea3_hd, (char **) NULL, (short **) NULL);
	    copy_fea_rec(fea1_rec, fea1_hd, fea3_rec,
			 fea3_hd, field_name_ptr, (short **) NULL);
	    if (t_flag)
	      fea3_rec->tag = fea1_rec->tag;
	    if (!t_flag && fea3_hd->common.tag)
	      fea3_rec->tag = fea2_rec->tag;
	    put_fea_rec (fea3_rec, fea3_hd, fea3_strm);
	    n1++;
	  }
	}
      }
    }
    else    /* if (a_flag) */
      while (get_fea_rec(fea2_rec, fea2_hd, fea2_strm) != EOF)
	{
	  copy_fea_rec(fea2_rec, fea2_hd, fea3_rec,
		       fea3_hd, (char **) NULL, (short **) NULL);
	  if (get_fea_rec(fea1_rec, fea1_hd, fea1_strm) != EOF)
	    {
	      copy_fea_rec(fea1_rec, fea1_hd, fea3_rec,
			   fea3_hd, field_name_ptr, (short **) NULL);
	      if (t_flag)
		fea3_rec->tag = fea1_rec->tag;
	    }
	  
	  if (!t_flag && fea3_hd->common.tag)
	    fea3_rec->tag = fea2_rec->tag;
	  put_fea_rec (fea3_rec, fea3_hd, fea3_strm);
	}
  }  /* if(new_fea2) */
  
  if (update) {
    FILE   *t_strm;
    
    rewind (fea3_strm);
    if(fclose (fea2_strm) == EOF) 
      error2("I/O error trying to close update file", fea2);
    t_strm = fea3_strm;
    if ((fea3_strm = fopen (fea2, "w")) == NULL)
      error2 ("cannot open input file for update: ", fea2);
    else
      while ((c = getc (t_strm)) != EOF)
	(void)putc (c, fea2_strm);
  }
  return(0);
}



