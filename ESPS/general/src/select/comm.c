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
 * Command module for feature select.
 *
 */

char *comm_sccs = "@(#)comm.c	3.12	1/20/97	ESI/ERL";

#include "select.h"

FILE		*e_temp_file();

extern char	*ifile[];
extern FILE 	*istrm[], *more_pipe, *ostrm;
extern char 	*ofile;
extern struct header *ihd[], *buffhd, *outhd;
extern struct fea_data *irec[];
extern int	nflag, n_ifiles;
extern long 	data_ptr;
extern char	*select_sccs, *io_sccs, *parser_sccs, *lexan_sccs;
extern char	*interp_sccs, *math_sccs;
extern int	n_wfields, n_wofields, debug_level;
extern char	*with_fields[], *wo_fields[];
extern int	interrupt, query_error;
extern int	fatal_error;
extern int	eflag;
void 		run_psps(), rewind_in(), more_begin(), more_end();
int		cfr_flag1, cfr_flag2;
long		in_count = 0;
void 		flush_buffer();
extern char	*eval_format;



/* show the help file
*/
int
help()
{
	(void)show_file(HELPFILE);
	return 1;
}

extern char *progversion, *progdate;

/* show the sccs versions of all
   modules
*/
int
show_version()
{
	message2("select version %s, compile date %s\n",progversion,progdate);
	message1("select:\t%s\n", select_sccs);
	message1("comm:\t%s\n", comm_sccs);
	message1("io:\t%s\n", io_sccs);
	message1("interp:\t%s\n", interp_sccs);
	message1("parser:\t%s\n", parser_sccs);
	message1("lexan:\t%s\n", lexan_sccs);
	message1("math:\t%s\n", math_sccs);
	return 1;
}

/* close all input files
*/
int
close_infile()
{
	if (debug_level) 
		Fprintf(stderr, "close_infile: n_ifiles is %d\n", n_ifiles);
	while (n_ifiles > 0)
		(void)fclose(istrm[--n_ifiles]);
	in_count = 0;
	return 1;
}

/* run sps on the output file
*/
int
show_to(s)
char	*s;
{
	if (debug_level)
		Fprintf(stderr,"show_to: called with %s",s);
	if(ofile == NULL) {
		errmsg("No output file open.\n");
		return 0;
	}
	run_psps(s,ofile);
	return 1;
}

struct fea_data *query_rec;
struct header *query_hd;

double
doquery(r, hd)
struct fea_data *r;
struct header *hd;
{
	double run_interp();

	assert(r != NULL && hd != NULL);
	query_rec = r;
	query_hd = hd;
	return run_interp();
}



/* get an input file and open it
   An array of file names, records, and headers is filled in.
   n_ifiles is the count of open files
*/
int
in_files(name)
char	*name;
{
	long	nrec;

	if (debug_level) 
		Fprintf(stderr, "in_files: called with %s\n", name);

	if (n_ifiles >= MAXIFILES) {
		errmsg("Too many input files given\n");
		return 0;
	}

	if (ofile != NULL && strcmp(ofile, name) == 0) {
		errmsg1(
		"Input file cannot be the same as the output file: %s\n",ofile);
		return 0;
	}

	ifile[n_ifiles] = name;
	istrm[n_ifiles] = fopen(name, "r");
	if (istrm[n_ifiles] == NULL) {
		errmsg1("Cannot open input file %s\n", name);
		return 0;
	}

	ihd[n_ifiles] = read_header(istrm[n_ifiles]);
	if (ihd[n_ifiles] == NULL) {
		errmsg1("File %s not an ESPS file\n", name);
		return 0;
	}
	if (ihd[n_ifiles]->common.type != FT_FEA) {
		errmsg1("File %s not an ESPS feature file\n", name);
		return 0;
	}

	irec[n_ifiles] = allo_fea_rec(ihd[n_ifiles]);
	assert(irec[n_ifiles] != NULL);
	if(outhd) {
		add_comment(outhd, "from ");
		add_comment(outhd, name);
		add_comment(outhd, "\n");
		add_source_file(outhd, name, ihd[n_ifiles]);
	}

	nrec = ihd[n_ifiles]->common.ndrec;

	/* Files with variable record length (e.g. Esignal Ascii format)
	 * will have nrec = -1.  In that case, copy to a regular FEA file;
	 * count records.
	 */
	if (nrec < 0)
	{				
		char		*tmpname;
		FILE		*tmpstrm;
		struct header	*tmphdr;
		struct fea_data	*rec = irec[n_ifiles];
		struct header	*hdr = ihd[n_ifiles];
		FILE		*strm = istrm[n_ifiles];

		if (debug_level)
			Fprintf(stderr, "in_files: copying %s to temp file\n",
				name);

		tmpstrm = e_temp_file("selectXXXXXX", &tmpname);
		if (tmpstrm == NULL)
		{
			errmsg1("Couldn't create temp file while opening %s\n",
				name);
			fclose(istrm[n_ifiles]);
			return 0;
		}

		unlink(tmpname);
		free(tmpname);

		tmphdr = copy_header(ihd[n_ifiles]);
		write_header(tmphdr, tmpstrm);

		for (nrec = 0; get_fea_rec(rec, hdr, strm) != EOF; nrec++)
			put_fea_rec(rec, tmphdr, tmpstrm);

		istrm[n_ifiles] = tmpstrm;
		rewind(tmpstrm);
		ihd[n_ifiles] = read_header(tmpstrm);
	}

	in_count += nrec;
	n_ifiles++;
	return 1;
}


/* just list the input files
*/
int
show_files()
{

	int	i = 0;
	if (debug_level) 
		Fprintf(stderr, "show_files: called\n");

	if (n_ifiles == 0) 
		message("No input files open.\n");
	else {
		message("Input files: \n");
		while (i < n_ifiles)
			message1("%s\n", ifile[i++]);
	}
	if (ofile == NULL)
		message("No output file open.\n");
	else 
		message1("Output file: %s\n",ofile);
		
	return 1;
}


/* run psps on the input files
*/
int
psps_in(s)
char	*s;
{

	int	i = 0;
	if (debug_level) 
		Fprintf(stderr, "psps_in: called with %s\n", s);

	if (n_ifiles == 0) 
		message("No input files open.\n");

	while (i < n_ifiles) {
		run_psps(s, ifile[i++]);
		if (i < n_ifiles) {
		  message("\nNext <return for yes, q then return for quit> ?");
		  if (getchar() == 'q') 
			break;
		}
	}
	return 1;
}


/* process a WITH clause on the TO command
*/
int
towith(name)
char	*name;
{
	if (debug_level) 
		Fprintf(stderr, "towith: called with %s\n", name);
	if (n_wfields >= MAXWFIELDS) {
	   errmsg1("Too many WITH fields given, seek help. name: %s\n", name);
	   return 0;
	}

	with_fields[n_wfields++] = name;
	return 1;
}


/* process a WITHOUT clause on the TO command
*/
int
towithout(name)
char	*name;
{
	if (debug_level) 
		Fprintf(stderr, "towithout: called with %s\n", name);
	if (n_wofields >= MAXWFIELDS) {
	  errmsg1("Too many WITHOUT fields given, seek help. name: %s\n", name);
	  return 0;
	}

	wo_fields[n_wofields++] = name;
	return 1;
}

int gf_flag = 0;
long sel_count;

char *query_file;
extern char line[];

int
select_rec()
{
	struct fea_data *r;
	struct header *hd, *last_hd=NULL;
	long	i, f_count=0;
	int j, first=1;
	char buf[LINSIZ+100];
	int i_tmp;

	if (debug_level) 
		Fprintf(stderr, "select_rec: called\n");
	if (query_error) {
		errmsg("Query not executed due to syntax errors.\n");
		return 0;
	}
	if (in_count == 0) {
		errmsg("No input files open.\n");
		return 0;
	}
	if (buffhd == NULL) {
		errmsg("No output file setup.\n");
		return 0;
	}
	sel_count = 0;
	rewind_in();
	j = 0;
	for (i = 0; i < in_count && !interrupt && !fatal_error; i++) {
		if (get_rec(&r, &hd, &query_file) == EOF) 
			break;
		if (hd != last_hd) gf_flag = cfr_flag1 = cfr_flag2 = 0;
		if (hd == last_hd) gf_flag = 1;
		if (!first && (hd != last_hd)) {
		    message2(
		    "%ld records from file %s selected.\n",f_count,ifile[j++]);
		    f_count = 0;
		}
		last_hd = hd;
/* the doquery call with a cast to (int) used to be inside the if
   expression, but that doesn't work on some Sun 4s
*/
		i_tmp = doquery(r, hd);
		if (i_tmp) {
			if (put_rec(r, hd) == 0)
				return 0;
			sel_count++;
			f_count++;
		}
		if (i > 0 && !(i % 1000)) Fprintf(stderr,".");
	first = 0;
	}
	message2("%ld records from file %s selected.\n",f_count,ifile[j]);
	if(i > 1000) Fprintf(stderr,"\n");
	if(interrupt)
		message1("%ld records processed before interrupt, ",i);
	message1("%ld total records selected.\n",sel_count);
	if(sel_count && !nflag) {
		int len = strlen(line);
		(void) sprintf(buf,
			"%s ; %ld records selected\n",line,sel_count);
		buf[len-1] = ' '; /* get rid of newline */
		(void)add_comment(outhd,buf);
	}
	flush_buffer();
	return 1;
}


/* like SELECT, but print selected records instead of adding
   them to the buffer
*/
int
show_select()
{
	int	i;
	struct fea_data *r;
	struct header *hd;
	long show_count=0;

	if (debug_level) 
		Fprintf(stderr, "show_select: called\n");
	if (query_error) {
		errmsg("Query not executed due to syntax errors.\n");
		return 0;
	}
	if (in_count == 0) {
		errmsg("No input files open.\n");
		return 0;
	}
	rewind_in();
	more_begin(24);
	for (i = 0; i < in_count && !(interrupt > 1) && !fatal_error; i++) {
		if (get_rec(&r, &hd, &query_file) == EOF) 
			return 0;
		if ((int)doquery(r, hd)){
			show_count++;
			if (!interrupt) {
				message("\n");
				print_fea_rec(r, hd, more_pipe);
			}
		}
	}
	more_end();
	message1("%ld records match.\n",show_count);
	return 1;
}


/* rewind the input files
*/
void
rewind_in()
{
	int	i;
	for (i = 0; i < n_ifiles; i++) {
		rewind(istrm[i]);
		(void) read_header(istrm[i]);
	}
	data_ptr = 0;
}


int
eval()
{
	long	i,k=0;
	struct fea_data *r;
	struct header *hd, *last_hd=NULL;
	int j=0;

	if (debug_level) 
		Fprintf(stderr, "eval: called\n");
	if (query_error) {
		errmsg("Query not executed due to syntax errors.\n");
		return 0;
	}
	if (in_count == 0) {
		errmsg("No input files open.\n");
		return 0;
	}
	rewind_in();
	for (i = 0; i < in_count && !(interrupt > 1) && !fatal_error; i++) {
		if (get_rec(&r, &hd, &query_file) == EOF) 
			return 0;
		if (hd != last_hd) {
			gf_flag = cfr_flag1 = cfr_flag2 = 0;
			k = 0;
			if(!eflag)
				message1("File: %s\n",ifile[j++]);
		}
		if (hd == last_hd) gf_flag = 1;
		last_hd = hd;
		if(!eflag)
			message2("Record %ld [%ld]: ",1+k++,1+i);
		message1(eval_format,doquery(r, hd));
	}
	return 1;
}

int
show_fields()
{
	int i=0,j;
	char **ptr;

	while(i < n_ifiles) {
		struct fea_header *p = ihd[i]->hd.fea;
	        more_begin(50);		/* always use more for this */
		message2("\nFile: %s %d fields\n\n",ifile[i],p->field_count);
		message("Field\t\tSize\tType\n\n");
		for (j=0; j<p->field_count; j++) {
		   message2("%s\t%ld\t",p->names[j],p->sizes[j]);
		   message1("%s\n",type_codes[p->types[j]]);
		   if(p->types[j] == CODED) {
			message(   " Possible values: ");
			ptr = p->enums[j];
			while (ptr && *ptr)
			  message1("%s\n                 ",*(ptr++));
			message("\n");
		   }
		}
	        more_end();
		if (++i < n_ifiles) {
		  message("\nNext <return for yes, q then return for quit> ?");
		  if (getchar() == 'q') 
			break;
		}
	}
	return 1;
}
