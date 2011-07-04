/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Virdy
 * Checked by:
 * Revised by:  Ken Nelson
 *
 * Brief description: pplain - print data from ESPS file in "plain" format
 *
 */

static char *sccs_id = "@(#)pplain.c	3.17	9/21/98	ESI/ERL";
int debug_level = 0;

/* pplain - print data from ESPS file in "plain" format
 * Purpose: good for programs that expect data in ASCII
 *	    converted from the ESPS program of the same name
 * Usage:   pplain [-i] [-rrange] [-erange] file
 * Notes:   If the file has tags, element 0 counts as the tag
 *
 * Updated by:	Ajaipal S. Virdy, Entropic Speech, Inc.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

#define SYNTAX USAGE("pplain [-x debug_level] [-r range] [-e grange]\n\t [-f field_range] [-n] [-i] file")

char	*Program = "pplain";
char	*in_file = NULL;

#ifndef DEC_ALPHA
char	*calloc();
#endif

/* external esps functions */

long	*grange_switch();
long	*fld_range_switch();
long	*fea_index_to_elem();
long	get_fea_element();
int	get_gen_recd();
short	get_rec_len();
char    *eopen();
void	lrange_switch();
char    *get_sphere_hdr();

extern  optind;
extern char *optarg;


static void
pr_plain_fea_recf (rec, hd, file, fields)
struct fea_data *rec;
struct header  *hd;
FILE *file;
char   *fields[];
{
    int     i,
            count = 0,
            i_addr = 0;
    long    psize,
           *l_ptr;
    short  *s_ptr;
    double *d_ptr;
    float  *f_ptr;
    char   *c_ptr;
    double_cplx	*dc_ptr;
    float_cplx	*fc_ptr;
    long_cplx	*lc_ptr;
    short_cplx	*sc_ptr;
    byte_cplx	*bc_ptr;

    struct fea_header  *fea = hd -> hd.fea;

    spsassert(rec != NULL, "print_fea_rec: rec is NULL");
    spsassert(hd != NULL, "print_fea_rec: hd is NULL");
    spsassert(hd->common.type == FT_FEA, "print_fea_rec: file not FEA");
    spsassert(file != NULL, "print_fea_rec: file is NULL");

    if (hd -> common.tag && ((fields == NULL) ||
	(fields != NULL && (lin_search2 (fields, "tag") != -1))))
	fprintf (file, "Tag: %ld\n", rec -> tag);
    for (i = 0; i < fea -> field_count; i++) {
	if ((fields == NULL) ||
	  (fields != NULL && (lin_search2 (fields, fea -> names[i]) != -1))) {
	    if (fea -> types[i] == DOUBLE)
		d_ptr = (double *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == FLOAT)
		f_ptr = (float *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == LONG)
		l_ptr = (long *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == SHORT || fea -> types[i] == CODED)
		s_ptr = (short *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == CHAR || fea -> types[i] == BYTE)
		c_ptr = get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == DOUBLE_CPLX)
		dc_ptr = (double_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == FLOAT_CPLX)
		fc_ptr = (float_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == LONG_CPLX)
		lc_ptr = (long_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == SHORT_CPLX)
		sc_ptr = (short_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    if (fea -> types[i] == BYTE_CPLX)
		bc_ptr = (byte_cplx *) get_fea_ptr (rec, fea -> names[i], hd);
	    count = 0;
	    psize = fea -> sizes[i];
	    if (psize == 1) {
		if (fea -> types[i] == DOUBLE)
		    fprintf (file, "%.8lg\n", *d_ptr++);
		if (fea -> types[i] == FLOAT)
		    fprintf (file, "%.8g\n", *f_ptr++);
		if (fea -> types[i] == LONG)
		    fprintf (file, "%ld\n", *l_ptr++);
		if (fea -> types[i] == SHORT)
		    fprintf (file, "%d\n", *s_ptr++);
		if (fea -> types[i] == BYTE)
		    fprintf (file, "%d\n", (int)*c_ptr++);
		if (fea -> types[i] == CHAR)
		    fprintf (file, "%c", *c_ptr);
		if (fea -> types[i] == DOUBLE_CPLX) {
		    fprintf (file, "[%lg, %lg]\n",dc_ptr->real,dc_ptr->imag);
		    dc_ptr++;}
		if (fea -> types[i] == FLOAT_CPLX) {
		    fprintf (file, "[%g, %g]\n",fc_ptr->real,fc_ptr->imag);
		    fc_ptr++;}
		if (fea -> types[i] == LONG_CPLX) {
		    fprintf (file, "[%ld, %ld]\n",lc_ptr->real,lc_ptr->imag);
		    lc_ptr++;}
		if (fea -> types[i] == SHORT_CPLX) {
		    fprintf (file, "[%d, %d]\n",sc_ptr->real,sc_ptr->imag);
		    sc_ptr++;}
		if (fea -> types[i] == BYTE_CPLX) {
		    fprintf (file, "[%d, %d]\n",bc_ptr->real,bc_ptr->imag);
		    bc_ptr++;}
		if (fea -> types[i] == CODED) {
		    if (idx_ok (*s_ptr, fea -> enums[i]))
			fprintf (file, "%s", fea -> enums[i][*s_ptr++]);
		    else
			fprintf (file, "bad code: %d\n", *s_ptr++);
		}
	    }
	    else {
		i_addr = 0;
		while (psize--) {
		    if (fea -> types[i] == DOUBLE)
			fprintf (file, "%13.8lg\n", *d_ptr++);
		    if (fea -> types[i] == FLOAT)
			fprintf (file, "%13.8g\n", *f_ptr++);
		    if (fea -> types[i] == LONG)
			fprintf (file, "%ld\n", *l_ptr++);
		    if (fea -> types[i] == SHORT)
			fprintf (file, "%d\n", *s_ptr++);
		    if (fea -> types[i] == BYTE)
			fprintf (file, "%d\n", (int)*c_ptr++);
		    if (fea -> types[i] == CHAR) {
			fprintf (file, "%s", c_ptr);
			psize = 0;
		    }
		    if (fea -> types[i] == DOUBLE_CPLX) {
			fprintf (file, "[%lg, %lg]\n", 
			 dc_ptr->real,dc_ptr->imag);
			dc_ptr++;}
		    if (fea -> types[i] == FLOAT_CPLX) {
			fprintf (file, "[%g, %g]\n", 
			 fc_ptr->real,fc_ptr->imag);
			fc_ptr++;}
		    if (fea -> types[i] == LONG_CPLX) {
			fprintf (file, "[%ld, %ld]\n", 
			 lc_ptr->real,lc_ptr->imag);
			lc_ptr++;}
		    if (fea -> types[i] == SHORT_CPLX) {
			fprintf (file, "[%d, %d]\n", 
			 sc_ptr->real,sc_ptr->imag);
			sc_ptr++;}
		    if (fea -> types[i] == BYTE_CPLX) {
			fprintf (file, "[%d, %d]\n", 
			 bc_ptr->real,bc_ptr->imag);
			bc_ptr++;}
		    if (fea -> types[i] == CODED) {
			if (idx_ok (*s_ptr, fea -> enums[i]))
			    fprintf (file, "%s\n", fea -> enums[i][*s_ptr++]);
			else
			    fprintf (file, "bad code: %d ", *s_ptr++);
			count++;
		    }
		    i_addr++;
		    if (fea -> types[i] == SHORT || fea -> types[i] == BYTE)
			count += 2;
		    else if (fea -> types[i] == CHAR)
			count++;
		    else
			count += 5;
		    if (psize && count > 20) {
			fprintf (file, "\n%3d: ", i_addr);
			count = 0;
		    }
		}
	    }
	}
    }
}





main (argc, argv)
int	argc;
char  **argv;
{
    int     rparam_processed = 0;
    int	    c;

    FILE    *istrm = stdin;
    struct header  *h;

    long     i_ele, s_ele, e_ele;
    int	    tags, ldrec, tag_ele;
    long    pos, irec, s_rec=1, e_rec=LONG_MAX;

    char   *r_range = NULL;
    char   *e_range = NULL;
    int	   e_flag = 0;
    int	   f_flag = 0;
    int    n_flag = 0;
    char   *format = "%g ";

    double  *data;	/* array to store data from get_gen_recd(3-ESPS) */

    long    *elem_array;    /* array containing elements to print */
    long    n_elems = 0;    /* number of elements to print */
    char    *name;	    /* holds name from fld_range_switch */
    struct fea_data *recdata; /* holds data when reading in fea file */
    char    *fieldarr[2];


    while ((c = getopt (argc, argv, "x:r:e:inf:")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi (optarg);
		break;
	    case 'r': 
		r_range = optarg;
		break;
	    case 'e': 
		e_range = optarg;
		if (!isdigit(e_range[strlen(e_range)-1])) {
		    Fprintf (stderr,
		    "%s: please specify last element explicitly.\n",
		    Program);
		    exit (1);
		}
		elem_array = grange_switch (e_range, &n_elems);
		e_flag++;
		break;
	    case 'f':
		e_range = optarg;
		f_flag++;
		break;
	    case 'n':
		n_flag++;
		break;
	    case 'i': 
		format = "%.0f ";
		break;
	    default: 
		SYNTAX;
	}
    }

    if (e_flag && f_flag) {
	Fprintf(stderr,"%s: cannot use both -e and -f options\n",Program);
	exit(1);
    }
	

    if (argc - optind > 1){
      Fprintf(stderr, 
	      "pplain: Only one input file name is allowed - exiting.\n");
      exit(1);
    }
    if(argc == optind + 1)
	in_file = argv[optind];

    if(!(in_file && (strcmp(in_file, "-") == 0))) {
	(void)read_params((char *)NULL, SC_CHECK_FILE, in_file);
	rparam_processed = 1;
	if (in_file == NULL) {
	  if (symtype("filename") == ST_UNDEF) {
		Fprintf(stderr,"%s: no input file.\n",Program);
		SYNTAX;
	  }
	  in_file = getsym_s("filename");
	}
    }

    in_file = eopen(Program,in_file,"r",NONE,NONE,&h,&istrm);

    /* bail on pc wave, sphere, and esignal input files */
    if (get_esignal_hdr(h) || get_sphere_hdr(h) || get_pc_wav_hdr(h))  {
        (void) fprintf(stderr, "%s: Input file %s is not an ESPS FEA file.\n",
               Program, argv[optind]);
        exit(1);
    }
    
    if (!r_range && rparam_processed) {
        if(symtype("start") != ST_UNDEF) 
	    s_rec = getsym_i("start");
        if(symtype("nan") != ST_UNDEF) 
    	    e_rec = s_rec + getsym_i("nan") - 1;
    }
    else
        lrange_switch (r_range, &s_rec, &e_rec, 1);

    if (e_rec == 0) e_rec = LONG_MAX;

    if (f_flag) {
	long i, base, size;
	if (h->common.type != FT_FEA) {
		Fprintf(stderr,
	        "%s: -f option can only be used with feature files.\n",Program);
		exit(1);
	}
		
	elem_array = fld_range_switch(e_range, &name, &n_elems, h);
/*
	for(i=0; i<n_elems; i++)
		fprintf(stderr,"%d ",elem_array[i]);
	fprintf(stderr,"\n");
*/
	if (elem_array == NULL) {
		Fprintf(stderr,"%s: no such field as %s\n",Program,name);
		exit(1);
	}
	elem_array = fea_index_to_elem(elem_array,&n_elems, name,h);
/*
	for(i=0; i<n_elems; i++)
		fprintf(stderr,"%d ",elem_array[i]);
	fprintf(stderr,"\n");
*/
	base = get_fea_element(name,h);
	size = get_fea_elem_count(name,h);

/*
	fprintf(stderr,"base: %d\n",base);
	fprintf(stderr,"size: %d\n",size);
*/
	for(i=0; i<n_elems; i++) {
		if(elem_array[i] >= size){
		 Fprintf(stderr,
                  "%s: there %s only %ld element%c in field %s\n",
	   	  Program, (size == 1) ? "is" : "are", size, 
	 	  (size == 1) ? ' ' : 's', name);
		 exit(1);
		}
		elem_array[i] += base;
	}
	e_flag = 1;
    }

    if (e_flag) {
	s_ele = elem_array[0];
	e_ele = elem_array[n_elems - 1];
    } else {
	s_ele = 1;
	e_ele = get_rec_len (h);
    }
    
    ldrec = get_rec_len (h);

    data = (double *) calloc ((unsigned) ldrec, sizeof (double));
    spsassert(data, "pplain: calloc failed!");

    tags = BOOL (h -> common.tag);

    if (!tags && (s_ele == 0)) {
	Fprintf (stderr, "%s: no tags in %s; cannot print element 0.\n",
	Program, in_file);
	exit (1);
    }
    if (e_ele > ldrec) {
	Fprintf (stderr, "%s: only %d elements per record in %s\n",
	Program, ldrec, in_file);
	exit (1);
    }

    fea_skiprec (istrm, s_rec - 1, h);

    if (s_ele == 0)	/* print tags */
	tag_ele = 1;	/* begin printing data from index 1 in elem_array */
    else		/* do not print tags */
	tag_ele = 0;	/* begin printing data from index 0 in elem_array */

    if (debug_level > 3) {
	Fprintf (stderr,
	"%s: tag_ele = %d, now process main loop.\n", Program, tag_ele);
    }

    /* Are we doing native data formats, or forcing to double ??? */

    if ( ! (n_flag && f_flag)) /* if we aren't printing native */
    {
     irec = s_rec;
     while((irec++ <= e_rec) && (get_gen_recd (data, &pos, h, istrm) != EOF)) 
     {
	if (s_ele == 0)
	    (void) printf ("%ld ", pos);
	if (e_flag)
	    for (i_ele = tag_ele; i_ele < n_elems; i_ele++)
		(void) fprintf (stdout, format, data[elem_array[i_ele] - 1]);
	else
	    for (i_ele = s_ele; i_ele <= e_ele; i_ele++)
		(void) fprintf (stdout, format, data[i_ele - 1]);
	(void) fputs ("\n", stdout);
      }
    }
    else
    {
      /* read in record from file, but only do between irec and newrec */

      recdata=allo_fea_rec(h);
   
      /* Build up an array of list the only field I want printed. */
      fieldarr[0]=name;
      fieldarr[1]=NULL;

      irec = s_rec;
      while (get_fea_rec(recdata,h,istrm) != EOF && irec++ <= e_rec)
      {
	pr_plain_fea_recf(recdata,h,stdout,fieldarr);
      }
   }
  
    if (strcmp(in_file,"<stdin>") != 0) {
    	(void) putsym_s("filename",in_file);
    	(void) putsym_s("prog",Program);
    	(void) putsym_i("start",(int)s_rec);
    	(void) putsym_i("nan",(int)(irec-s_rec-1));
    }

    return 0;
}
