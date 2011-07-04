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
 * Written by:  Joseph T. Buck, modified for select by Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

char *io_sccs = "@(#)io.c	3.9	1/20/97	ESI/ERL";

#include "select.h"
#include <signal.h>
#include <esps/esignal_fea.h>

#define SCREEN_HEIGHT 23

extern char	*get_sphere_hdr();

extern char	*ifile[], *ofile;
extern FILE	*istrm[], *ostrm;
extern struct header *ihd[];
extern struct fea_data *irec[];
extern int	nflag, n_ifiles, debug_level;
extern int	cfr_flag1, cfr_flag2;
extern char	*select_sccs, *io_sccs, *parser_sccs, *lexan_sccs;
int		n_wfields=0, n_wofields=0;
char 		*with_fields[MAXWFIELDS], *wo_fields[MAXWFIELDS];
extern int	lin_search2(), copy_fea_fld();
char		*savestring();
void 		open_buff(), copy_fea_rec(), set_header();
char		msgbuf[200], *strcpy();
long		data_ptr;
long		undone_count=0;
static char	*pager_prog;
int		isatty();
struct header 	*buffhd=NULL;	/* header for select buffer */
struct header 	*outhd;		/* header for output file */
static FILE 	*buffd=NULL;	/* stream for select buffer */
static FILE 	*tstrm=NULL;	/* stream for temporary file */
static struct fea_data *bufrec; /* select buffer data pointer */
static FILE 	*logfd;		/* fd for log file. NULL if none open */
FILE 		*more_pipe;	/* stream for piping to more */
static char 	*logname;	/* log file name. NULL if none open */
static int 	bol_out;	/* true at bol in output */
static int 	changes;	/* true if changes made to buffer since
				   last write */
static long 	out_count = 0;	/* number of records in buffer */
static int 	in_tty;		/* true if stdin is a tty */
static int 	out_tty;	/* true if stdout is a tty */
int		eof_flag;	/* true if eof on a non-tty */
static int	sigtrap;	/* if signal caught, contains its code */
static int	piping;		/* true if piping to more */
extern int	com_file_depth;	/* nonzero if in a command file */
int		pr_commands;	/* if nonzero, lines in command files are
				   printed */
/*
   Eunice has no SIGPIPE. It does have SIGCHLD though, which we use to
   find out if more dies.
*/
#ifdef Eunice
#define WHICH_SIG SIGCHLD
#else
#define WHICH_SIG SIGPIPE
#endif

/* The trap routine catches signals resulting from the pager terminating
   early.
*/
static int	
trap (n) 
{
	sigtrap = n;
#ifdef Eunice
	/* For Eunice, we must close the broken pipe */
	if (piping) 
		pclose (more_pipe);
#endif
}


void
init_output() 
{
	bol_out = 1;
	in_tty = isatty(0);
	out_tty = isatty(1);
	logfd = NULL;
	logname = NULL;
	eof_flag = 0;
	if ((pager_prog = getenv ("PAGER")) == NULL) 
		pager_prog = PAGER;
}


/* more_begin causes output to be passed through "more" if the argument is
   greater than or equal to SCREEN_HEIGHT.
   It does not call more unless both stdin and stdout are a terminal.
 */
void
more_begin(nlines)
int	nlines;
{
	if (nlines < SCREEN_HEIGHT || !out_tty || !in_tty || piping) 
		return;
	sigtrap = 0;
	piping++;
	more_pipe = popen(pager_prog, "w");
	(void) signal (WHICH_SIG, trap);
}


/*
  more_end closes the "more" pipe, if one is open.
 */
void
more_end() 
{
	if (!piping) 
		return;
	piping = 0;
	(void) signal (WHICH_SIG, SIG_IGN);
	if (!sigtrap) 
		(void) pclose (more_pipe);
	sigtrap = 0;
}


/* show_file displays a file using the pager. */
show_file (file)
char	*file;
{
	char	line[BUFSIZ];
	(void) sprintf (line, "%s %s", pager_prog, file);
	return !system (line);
}


void
message(msg)
char	*msg;
{
	if (piping) {
		if (!sigtrap) 
			(void) fputs (msg, more_pipe);
	} else 
		(void) fputs (msg, stdout);
	if (logfd)
		Fprintf(logfd, bol_out ? "# %s" : "%s", msg);
	bol_out = (msg[strlen(msg)-1] == '\n');
}


void
errmsg(msg)
char	*msg;
{
	(void) fputs(msg, stderr);
	if (logfd)
		(void) Fprintf(logfd, bol_out ? "# %s" : "%s", msg);
	bol_out = (msg[strlen(msg)-1] == '\n');
}


/* close_logfile msg flag:
	0: No messages.
	1: Message if we close a file.
	2: Message regardless.
 */
close_logfile(msg)
int	msg;
{
	if (logfd) {
		if (msg) 
			message1("Closing log file \"%s\"\n", logname);
		(void) fclose(logfd);
		logfd = NULL;
		logname = NULL;
		return 1;
	} else if (msg == 2) 
		message("Logging is already off\n");
	return 0;
}


open_logfile(name)
char	*name;
{
	(void) close_logfile(1);
	logfd = fopen(name, "w");
	if (logfd == NULL) {
		errmsg1("Cannot open log file %s\n", name);
		return 0;
	} else 
		message1("Opening log file %s\n", name);
	logname = name;		/* Assumes name will stay around! */
	return 1;
}


show_log_status()
{
	if (logfd) 
		message1("Writing to log file \"%s\"\n", logname);
	else 
		message("No log file open\n");
	return 1;
}


/* This is the function called by the lexical analyzer to read input.
   It reads a line at a time, but returns a character at a time.
   It prints prompts and copies typed commands to the log file, if one
   is open.
 */
char	line[LINSIZ];
static char	*lp = line;

Getc(fd)
FILE *fd;
{
	if (*lp) 
		return * lp++;
	if (in_tty && !com_file_depth) 
		Fprintf(stderr, "-> ");
	else if (eof_flag) 
		exit(0);
	if (fgets (line, LINSIZ - 1, fd)) {
		if (logfd) 
			(void) fputs (line, logfd);
		if (com_file_depth && pr_commands) 
			(void) fputs (line, stdout);
		lp = line;
		return * lp++;
	} else {
		if (in_tty && !com_file_depth) 
			clearerr (fd);
		return EOF;
	}
}


/* This function pushes back a string onto the standard input */

void
push_back_line (text, addnl)
char	*text;
int	addnl;
{
	if (text == NULL) 
		return;	/* safety */
	if (*lp == 0) {
		lp = line;
		(void) strncpy (line, text, LINSIZ - 2);
	} else 
		(void) strncat (line, text, LINSIZ - strlen (line) - 2);
	if (addnl) 
		(void) strcat (line, "\n");
	return;
}


/* this function runs psps on file using flags s
*/
void
run_psps(s, file)
char	*s, *file;
{

	char	*flags = " ";
	if (debug_level) 
		Fprintf(stderr, "run_psps: called with %s and %s\n", s, file);

	if (s) 
		flags = s;
	if (flags[0] != ' ' && flags[0] != '-') {
		(void) sprintf(msgbuf, "-%s", flags);
		flags = savestring(msgbuf);
	}

	(void)sprintf(msgbuf,"%s %s %s | %s\n",PSPS, flags, file, pager_prog);
	if (debug_level) 
		Fprintf(stderr, msgbuf);
	(void) system(msgbuf);
}

int tags_to_seg = NO;

/* open the output file.  The file might already exist 
*/
open_outfile(s)
char	*s;
{
	int	i = 0, tags;
	char	*name;
	if (debug_level) 
		Fprintf(stderr, "open_outfile: called with %s\n", s);

	if (n_ifiles == 0) {
		errmsg(
		"Cannot specify output file when no input files are open.\n");
		n_wfields = n_wofields = 0;
		return 0;
	}
	ofile = NULL;
	for (i = 0; i < n_ifiles; i++) {
		if (strcmp(s, ifile[i++]) == 0) {
		   errmsg1(
		    "Output file %s cannot be the same as an input file.\n", s);
		   n_wfields = n_wofields = 0;
		   return 0;
		}
	}
/*
 check and see if the output file exists, if so create header
 for buffer that is the same as the output file. 
*/

	if ((ostrm = fopen(s, "r+")) != NULL) {
		if ((outhd = read_header(ostrm)) == NULL
		    || outhd->common.type != FT_FEA
		    || get_esignal_hdr(outhd) != NULL
		    || get_sphere_hdr(outhd) != NULL)
		{
			errmsg1("Output file %s is not an ESPS FEA file.\n", s);
			return 0;
		}
		if (!check_names(outhd, ihd[0])){
			errmsg1("Output file %s has no field names in common with the first input file.\n",s);
			return 0;
		}
		errmsg1("Output file %s exists; will add records to it.\n",s);
		ofile = savestring(s);
		if (n_wfields != 0 || n_wofields != 0)
		 errmsg(
		 "WITH or WITHOUT clause ignored since file already exists.\n");
		
		buffhd = copy_header(outhd);
		set_header(outhd);
		n_wfields = n_wofields = 0;
		(void) clear_buffer();
		if (fseek(ostrm,0L,2) != 0) {
		  Fprintf(stderr,"fseek failed!");
		  exit (1);
		}
		return 1;
	}

/*
 the output files doesn't exist.   Open it, and set up the header
 of the buffer, depending on fields in with_fields and wo_fields.
*/

	if ((ostrm = fopen(s, "w+")) == NULL) {
		errmsg1("Cannot open output file %s\n", s);
		return 0;
	}
	if (n_wfields == 0 && n_wofields == 0)
		buffhd = copy_header(ihd[0]);
	else if (n_wfields != 0) {
		buffhd = new_header(FT_FEA);
		buffhd->common.tag = ihd[0]->common.tag;
		for (i = 0; i < n_wfields; i++) {
			name = with_fields[i];
			if (lin_search2(ihd[0]->hd.fea->names, name) == -1)
			    errmsg1("Field %s not in first input file\n", name);
			else
			  if (copy_fea_fld(buffhd, ihd[0], name) == -1) {
				errmsg1("Trouble adding field %s\n", name);
				exit(1);
			  }
		}
	} else if (n_wofields != 0) {
		buffhd = new_header(FT_FEA);
		buffhd->common.tag = ihd[0]->common.tag;
		for (i = 0; i < ihd[0]->hd.fea->field_count; i++) {
			name = ihd[0]->hd.fea->names[i];
			if (lin_search2(wo_fields, name) == -1)
				if (copy_fea_fld(buffhd, ihd[0], name) == -1) {
					errmsg1("Trouble adding field %s\n", 
						name);
					exit(1);
				}
		}
	}

	n_wfields = n_wofields = 0;
	if (buffhd->hd.fea->field_count == 0) {
		errmsg("No fields in output file; file not open.\n");
		buffhd = NULL;
		return 1;
	}
	tags = 0;
	for(i=0; i<n_ifiles; i++)
	   if(ihd[i]->common.tag) tags++;

	if(tags >1) {
	   char **names = (char **)malloc((n_ifiles+1)*sizeof(char *));
	   int j=1;
	   names[0] = "NONE";
	   for (i=0; i<n_ifiles; i++)
	     if(ihd[i]->common.tag)
		names[j++] = savestring(ihd[i]->variable.refer);
	   names[j] = NULL;
	   if(buffhd->common.tag)
		buffhd->common.tag = NO;
	   set_seg_lab(buffhd, names);
	   tags_to_seg = YES;
	   buffhd->variable.refer = NULL;
	   message("Creating output file as segment labeled.\n");
	}
	(void) clear_buffer();
	ofile = savestring(s);
	outhd = copy_header(buffhd);
	outhd->common.ndrec = 0;
	set_header(outhd);
	write_header(outhd,ostrm);
	return 1;
}


char	buffile[20] = "selbufXXXXXX";
char	tempfile[20] = "seltmpXXXXXX";

/* open the buffer file.  First time through, create the buffer and temp
   file
*/
void
open_buff()
{
	extern char	*mktemp();
	static first = 1;

	spsassert(buffhd, "buffhd botch");
	if (first) {
		(void)mktemp(buffile);
		(void)mktemp(tempfile);
		first = 0;
	}
	buffd = fopen(buffile, "w+");
	spsassert(buffd, "Trouble opening temporary file");
	tstrm = fopen(tempfile, "w+");
	spsassert(tstrm, "Trouble opening temporary file");

	write_header(buffhd, buffd);
	(void)fflush(buffd);
	changes = 0;
	out_count = 0;
	bufrec = allo_fea_rec(buffhd);
	assert(bufrec != NULL);
	
}


/* run psps on the buffer file
*/
int
show_buffer()
{
	if (debug_level) 
		Fprintf(stderr, "show_buffer: called.\n");
	if (out_count == 0) 
		message("Nothing in buffer.\n");
	else
		run_psps("-H", buffile);
	if (undone_count) { 
		message1(
		  "%ld extra records shown at the end have been undone,\n",undone_count);
		message(
		  "these records will not be output by the write command.\n");
	}
	return 1;
}

	
/* clear the buffer
*/
int
clear_buffer()
{
	if (debug_level) 
		Fprintf(stderr, "clear_buffer: called.\n");
	if (buffhd == NULL) {	/* no buffer setup yet */
		message("There is no buffer setup yet.\n");
		return 1;
	}
	if (changes)
		if (!confirm(
			"Buffer has changed since last write - clear it?"))
			return 0;
	if(bufrec != NULL) {
		free((char *)bufrec);
		bufrec = NULL;
	}
	if (buffd != NULL) {
		(void)fclose(buffd);
		(void)unlink(buffile);
	}
	if (tstrm != NULL) {
		(void)fclose(tstrm);
		(void)unlink(tempfile);
	}
	open_buff();
	return 1;
}



/* get another record from the input file set.   
*/
int
get_rec(rec_ptr, hd_ptr, name_ptr)
struct fea_data **rec_ptr;
struct header **hd_ptr;
char **name_ptr;
{
	int	i;
	long total=0;
	if (debug_level) 
		Fprintf(stderr, "get_rec called.\n");
	assert(name_ptr && rec_ptr && hd_ptr);
	if (n_ifiles == 0) {
		errmsg("No input files open.\n");
		return EOF;
	}
	if (data_ptr < ihd[0]->common.ndrec) { /* special case */
		data_ptr++;
		*hd_ptr = ihd[0];
		*rec_ptr = irec[0];
		*name_ptr = ifile[0];
		return get_fea_rec(irec[0], ihd[0], istrm[0]);
	}
	for (i = 1; i < n_ifiles; i++) {
		total += ihd[i-1]->common.ndrec;
		if (data_ptr < ihd[i]->common.ndrec+total) {
			data_ptr++;
			*hd_ptr = ihd[i];
			*rec_ptr = irec[i];
			*name_ptr = ifile[i];
			return get_fea_rec(irec[i], ihd[i], istrm[i]);
		}
	}
	errmsg1("data_ptr got fouled up: %d, seek help\n", data_ptr);
	return EOF;
}

extern long sel_count;

/* print the current record in the buffer record 
*/
int
show_last()
{
	if (debug_level)
		Fprintf(stderr,"show_last called.\n");
	if (out_count == 0) 
		message("Nothing in buffer.\n");
	else if (sel_count == 0)
		message("Cannot show last record after undo.\n");
	else
		print_fea_rec(bufrec, buffhd, stdout);
	return 1;
}

void zero_fea_rec();
	
/* put a record to the buffer
*/
int
put_rec(rec, hd)
struct fea_data *rec;
struct header *hd;
{
	if (debug_level) 
		Fprintf(stderr, "put_rec called.\n");
	assert(rec && hd);
	if (bufrec == NULL) {
		errmsg("No output file setup.\n");
		return 0;
	}
	zero_fea_rec(bufrec, buffhd);
	copy_fea_rec(rec, hd, bufrec, buffhd, buffhd->hd.fea->names, 
			(short **)NULL);
/* handle tags to segment_labeled
   if called for
*/
	if (hd->common.tag && tags_to_seg) {
		*(long *)get_fea_ptr(bufrec,"segment_start",buffhd) = rec->tag;
		*(short *)get_fea_ptr(bufrec,"source_file",buffhd) =
		  fea_encode(hd->variable.refer,"source_file",buffhd);
	}
	out_count++;
	changes++;
	undone_count = 0;
	put_fea_rec(bufrec, buffhd, buffd);
	return 1;
}

/* flush the buffer.  This updates the header also
*/
void
flush_buffer()
{
	buffhd->common.ndrec = out_count;
	(void)fflush(buffd);
	rewind(buffd);
	write_header(buffhd, buffd);
	if (fseek(buffd,0L,2) != 0) {
	  Fprintf(stderr,"fseek failed!");
	  exit (1);
	}
	(void)fflush(buffd);
}

/* ask the user for confirmation of something
*/
int
confirm(s)
char	*s;
{
#define CLINSIZ 5
	char	buf[CLINSIZ];
	if (com_file_depth != 0 || !in_tty) 
		return 1;
	while (1) {
		message1
		    ("%s\n [y then return to confirm, n then return to abort] ", s);
		(void) fgets(buf, CLINSIZ, stdin);
		if (strlen(buf) == 2 && buf[0] == 'y') 
			return 1;
		if (strlen(buf) == 2 && buf[0] == 'n') 
			return 0;
	}
}

/* show number of output records in buffer
*/
int
show_size()
{
	message1("%ld records in output buffer.\n",out_count);
	return 1;
}

/* write the buffer to the output files
*/
int
write_buffer()
{
	long i;

	if (debug_level)
		Fprintf(stderr,"write_buffer called.\n");
	
	if(changes == 0) {
		errmsg("No changes to write.\n");
		return 1;
	}
	if (out_count == 0) {
		errmsg("Nothing in buffer.\n");
		return 1;
	}
	if (ofile == NULL) {
		errmsg("No output file specified.\n");
		return 0;
	}

	rewind(tstrm);
	rewind(buffd);
	(void)read_header(buffd);
	if(!nflag) {
		rewind(ostrm);
		(void)read_header(ostrm);
		while (get_fea_rec(bufrec,buffhd,ostrm) != EOF)
			put_fea_rec(bufrec,buffhd,tstrm);
		rewind(ostrm);
		outhd->common.ndrec += out_count;
		write_header(outhd, ostrm);
		rewind(tstrm);
		while (get_fea_rec(bufrec,buffhd,tstrm) != EOF)
			put_fea_rec(bufrec,buffhd,ostrm);
	}
	i = out_count;
	while (i-- && (get_fea_rec(bufrec,buffhd,buffd) != EOF))
		put_fea_rec(bufrec,buffhd,ostrm);
	if(nflag) {
		rewind(ostrm);
		outhd->common.ndrec += out_count;
		write_header(outhd, ostrm);
		if (fseek(buffd,0L,2) != 0) {
	  	  Fprintf(stderr,"fseek failed!");
	  	  exit (1);
		}
	}
	(void)fflush(ostrm);
	changes=0;
	(void) clear_buffer();
	return 1;
	
	
}

/* close the output file
*/
int
close_outfile()
{
	if(ofile == NULL)
		return 1;
	(void) fclose(ostrm);
	ofile = NULL;
	return 1;
}


extern char *progversion, *progdate;
extern int Argc;
extern char **Argv;

/* set the version, program  name, and program date 
   in the output header.  It also adds source files.
   If the -n flag is set, then don't add source files.
*/
void
set_header(hd)
struct header *hd;
{
	int i;
	char *get_cmd_line();

	(void)strcpy(hd->common.prog,PROG);
	(void)strcpy(hd->common.vers,progversion);
	(void)strcpy(hd->common.progdate,progdate);

	if(!nflag) {
		for(i=0; i<n_ifiles; i++) 
			add_source_file(hd, ifile[i], ihd[i]);
		add_comment(hd, get_cmd_line(Argc, Argv));
	}
}

/* quit   If changes haven't been written out, confirm from the
   user
*/
int
terminate()
{
	if (debug_level) 
		Fprintf(stderr, "terminate: called.\n");
	if (changes)
		if (!confirm(
			"Buffer has changed since last write - quit?"))
			return 0;
	(void) unlink(buffile);
	(void) unlink(tempfile);
	exit (0);
	return 1; 	/* lint */
}

		
int
undo()
{
	if(sel_count != 0) {
	  buffhd->common.ndrec -= sel_count;
	  out_count -= sel_count;
	  (void)fflush(buffd);
	  rewind(buffd);
	  write_header(buffhd, buffd);
	  fea_skiprec(buffd,buffhd->common.ndrec,buffhd);
	  message1("%ld records removed from end of buffer.\n",sel_count);
	  undone_count = sel_count;
	  sel_count = 0;
	  if (out_count == 0) changes = 0;
	}
	else
	  message("Nothing to undo.\n");
	return 1;
}

int
check_names(a,b)
struct header *a, *b;
{
	char **afields, **bfields;
	int k;
	
	assert(a); assert(b);
	assert(a->common.type == FT_FEA);
	assert(b->common.type == FT_FEA);

	afields = a->hd.fea->names;
	bfields = b->hd.fea->names;
	
	for (k=0; bfields[k] != NULL; k++) 
		if(lin_search2(afields, bfields[k]) != -1) return 1;

	return 0;
}

void
zero_fea_rec(rec,hd)
struct fea_data *rec;
struct header *hd;
{
	int i;
	spsassert(hd && rec,"hd or rec is NULL");

	for(i=0; i<hd->common.ndouble; i++)
		rec->d_data[i] = 0;
	for(i=0; i<hd->common.nfloat; i++)
		rec->f_data[i] = 0;
	for(i=0; i<hd->common.nlong; i++)
		rec->l_data[i] = 0;
	for(i=0; i<hd->common.nshort; i++)
		rec->s_data[i] = 0;
	for(i=0; i<hd->common.nchar; i++)
		rec->b_data[i] = 0;
}
