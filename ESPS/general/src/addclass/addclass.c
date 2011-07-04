/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: to build recognition codebooks
 *
 */
#ifndef lint
static char *sccs_id = "@(#)addclass.c	1.5	9/28/98	ESI/ERL";
#define VERS "%V%"
#define DATE "9/28/98"
#else
#define VERS "no sccs"
#define DATE "no sccs"
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/vq.h>


/* externals */
long	atol();		/* ascii to long */
extern char *optarg;	
extern int optind;	/* option index from optarg */
char 	*tname;			/* name for temp file */
char	*eopen();	/* esps open routine */
char	*mktemp();	/* creates temp file name */
char	*get_cmd_line();/* returns command line for comment */
char	*strcpy(), *strncpy();
void	rewind();
char *e_temp_name();

int debug_level=0;		/* if > 0, debug output */

#ifndef SUN3
void	exit();
#endif

#define SYNTAX USAGE("addclass [-r record_num] [-x debug_level] [-p field_rep] [-f field] [-s source_name] [-t source_type] [-n signal_name] infile outfile")

/* linux requires ANSI-style "stringization"*/
#ifdef LINUX_OR_MAC
#define COPYFIELD(s) (void)strncpy(in_rec->s,s,(int)get_fea_siz(#s,hd1,(short *)NULL, (long **)NULL))
#else
#define COPYFIELD(s) (void)strncpy(in_rec->s,s,(int)get_fea_siz("s",hd1,(short *)NULL, (long **)NULL))
#endif


main(argc, argv)
char **argv;
int argc;
{

	struct header *hd1, *hd2;	/* input and output headers */
	struct header *hd_out;		/* output header when appending */
	FILE *strm1, *strm2;		/* input and output streams */
	FILE *tstrm=NULL;		/* temporary stream */
	char *file1, *file2;		/* input and output files */
	char *field_rep="none";		/* field_rep from -p arg */
	char *field="none";		/* field from -f arg */
	char *source_name="none";	/* source_name from -s arg */
	char *source_type="none";	/* source_type from -t arg */
	char *signal_name="none";	/* signal_name from -n arg */
	long record_num= -1;		/* record from input to get */
	struct vqcbk *in_rec;		/* the input record */
	struct vqcbk *t_rec;		/* for temp use */
	int c;

	
	while ((c = getopt(argc, argv, "x:p:f:r:s:t:n:")) != EOF) {
	  switch (c) {
		case 'x':
		  debug_level = atoi(optarg);
		  break;
		case 'p':
		  field_rep = optarg;
		  break;
		case 'f':
		  field = optarg;
		  break;
		case 'r':
		  record_num = atol(optarg);
		  break;
		case 's':
		  source_name = optarg;
		  break;
		case 't':
		  source_type = optarg;
		  break;
		case 'n':
		  signal_name = optarg;
		  break;
		default:
		  SYNTAX;
	  }
	}

/* there must be two filenames given */

	if (argc - optind < 2) {
		Fprintf(stderr,"addclass: need two filenames\n");
		SYNTAX;
	}
	
/* open the input and output files */

	file1 = eopen("addclass", argv[optind++], "r", FT_FEA, FEA_VQ, 
		  &hd1, &strm1);

/* the output file may or may not exist already, try to open it first */

	strm2 = fopen(argv[optind], "r");
	if (strm2 == NULL) 	/* it doesn't exist */
		file2 = eopen("addclass", argv[optind], "w", FT_FEA,
			NONE, &hd2, &strm2);
	else {			/* it does already exist */
	        tname = e_temp_name("addclXXXXXX");
		tstrm = fopen(tname, "w+");
		(void)unlink(tname);	/* make file go away when closed */
		(void)fclose(strm2);
		spsassert(tstrm, "cannot open temp file");
		file2 = eopen("addclass", argv[optind], "r", FT_FEA,
			FEA_VQ, &hd2, &strm2);
		if (*(long *)get_genhd("design_size",hd1) != 
		    *(long *)get_genhd("design_size",hd2)) {
			Fprintf(stderr,
			 "design_size different on input files.\n");
			exit(1);
		}
		if (*(long *)get_genhd("codeword_dimen",hd1) != 
		    *(long *)get_genhd("codeword_dimen",hd2)) {
			Fprintf(stderr,
			 "codeword_dimen different on input files.\n");
			exit(1);
		}
	}

/* allocate the input record and the record we need */

	in_rec = allo_vqfea_rec(hd1);	
	if (hd1->common.ndrec == -1) { 	/* input is a pipe or
					 * record length is variable */
		int i = record_num;
		struct vqcbk *prev_rec = allo_vqfea_rec(hd1);
		struct vqcbk *temp_rec;
		while(i-- && get_vqfea_rec(in_rec, hd1, strm1) != EOF) {
			temp_rec = prev_rec;	/* save the previous record */
			prev_rec = in_rec;
			in_rec = temp_rec;
		}
		in_rec = prev_rec;	/* in_rec is now the one we want */
	}
	else {				/* input is a file, use skiprec */
		if (record_num == -1)
			record_num = hd1->common.ndrec;
		if (record_num > hd1->common.ndrec) {
			Fprintf(stderr,
			 "addclass: you asked for record %ld",record_num);
			Fprintf(stderr,
			 "but there are only %ld records on the file.\n",
			  hd1->common.ndrec);
			 exit (1);
		}

		if (record_num > 1)
			fea_skiprec(strm1, record_num-1, hd1);

		if (get_vqfea_rec(in_rec, hd1, strm1) == EOF) {
			Fprintf(stderr,"addclass: trouble reading input\n");
			exit(1);
		}
		
	}

/* modify the affected fields in the input record */


	COPYFIELD(field_rep);
	COPYFIELD(field);
	COPYFIELD(source_name);
	COPYFIELD(source_type);
	COPYFIELD(signal_name);

/* the easiest case is that of a new output file */

		
	if (tstrm == NULL) {   /* this means there is no existing output file */
			hd2 = copy_header(hd1);
		add_source_file(hd2, file1, hd1);
		(void)add_comment(hd2, get_cmd_line(argc,argv));
		(void)strcpy(hd2->common.prog, "addclass");
		(void)strcpy(hd2->common.vers, VERS);
		(void)strcpy(hd2->common.progdate, DATE);
		write_header(hd2, strm2);
		put_vqfea_rec(in_rec, hd2, strm2);
		(void)fclose(strm2);
	}
	else {			/* there is a temp file */
		t_rec = allo_vqfea_rec(hd2);
		if(debug_level) Fprintf(stderr,"%s->temp",file2);
		while(get_vqfea_rec(t_rec, hd2, strm2) != EOF) {
			put_vqfea_rec(t_rec, hd2, tstrm);
			if(debug_level) Fprintf(stderr,".");
		}
		if(debug_level) Fprintf(stderr,"\n");
		(void)fclose(strm2);

/* check for validity of certain fields */
		if(*in_rec->cbk_struct != *t_rec->cbk_struct) {
			Fprintf(stderr,
			 "addclass: cbk_struct different on files\n");
			Fprintf(stderr," in: %d, out: %d\n",
			*in_rec->cbk_struct, *t_rec->cbk_struct);
			exit(1);
		}
		if(strcmp(in_rec->field, t_rec->field) != 0) {
			Fprintf(stderr,"field different on files\n");
			exit(1);
		}
		if(strcmp(in_rec->field_rep, t_rec->field_rep) != 0) {
			Fprintf(stderr,"field_rep different on files\n");
			exit(1);
		}

		strm2 = fopen(file2, "w");

		hd_out = copy_header(hd2);
		add_source_file(hd_out, file1, hd1);
		add_source_file(hd_out, file2, hd2);

		(void)add_comment(hd_out, get_cmd_line(argc,argv));
		(void)strcpy(hd_out->common.prog, "addclass");
		(void)strcpy(hd_out->common.vers, VERS);
		(void)strcpy(hd_out->common.progdate, DATE);
		write_header(hd_out, strm2);
		rewind(tstrm);
		if(debug_level) Fprintf(stderr,"temp->%s",file2);
		while(get_vqfea_rec(t_rec, hd2, tstrm) != EOF) {
			put_vqfea_rec(t_rec, hd2, strm2);
			if(debug_level) Fprintf(stderr,".");
		}
		if(debug_level) Fprintf(stderr,"\n");
		put_vqfea_rec(in_rec, hd2, strm2);
		(void)fclose(strm2);
	}
	return 0;
}
