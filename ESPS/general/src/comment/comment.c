/* comment - read/write comment field from an SPS file.
 
  This material contains proprietary software of Entropic Processing, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Processing, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Processing, Inc. must bear the notice			
 								
               "Copyright 1986,1990 Entropic Processing, Inc."
*/

#ifdef SCCS
static char *sccs_id = "@(#)comment.c	3.9 3/14/94 ESI";
#endif

#include <stdio.h>
#include <pwd.h>
#include <esps/esps.h>
#include <esps/unix.h>
#define SYNTAX USAGE ("comment: [-C commfile] [-c comment] [-t] [-a] [-S] espsfile\n")

/* Globals and Externals */
int     tflag;		/* non-zero if typtxt field used instead of comment */
void print_comment ();
void Add_comment ();
void print_txtcomment();
void tab();

int debug_level = 0;

char *e_temp_name();
char   *get_username();
char   *savestring ();
extern  optind;
extern char *optarg, *mktemp (), *ctime ();

main (argc, argv)
char  **argv;
int     argc;
{
    int     getopt (), c, Cflag = 0, aflag = 0;
    int     Sflag = 0;
    long    time (), tloc;
    FILE * in, *out, *tstrm, *comment, *fopen ();
    struct header  *ih;
    char   *tempfile,
            buf[1000],
           *comfile,
            combuf[100];
    char   *string = NULL;

    tflag = 0;
    while ((c = getopt (argc, argv, "C:c:taS")) != EOF) {
	switch (c) {
	    case 'C': 
		Cflag++;
		comfile = optarg;
		break;
	    case 'c': 
		string = optarg;
		break;
	    case 't': 
		tflag = 1;
		break;
	    case 'a': 
		aflag = 1;
		break;
	    case 'S': 
		Sflag = 1;
		break;
	    default: 
		SYNTAX;
	}
    }

    if (argc - optind < 1)
	SYNTAX;
    TRYOPEN (argv[0], argv[optind], "r", in);
    if (!(ih = read_header (in)))
	NOTSPS (argv[0], argv[optind]);

    tloc = time (0);

    if (Cflag || string != NULL) {
        tempfile = e_temp_name("cmntXXXXXX");

	if ((tstrm = fopen (tempfile, "w")) == NULL)
	    CANTOPEN (argv[0], tempfile);
	if (!Sflag) {
	    (void) sprintf (combuf, "comment added by %s: ", get_username());
	    Add_comment (ih, combuf);
	    Add_comment (ih, ctime (&tloc));
	  }
	if (Cflag) {
	    if (strcmp (comfile, "-") == 0) {
		(void) printf ("%s: enter comment, end with blank line\n", 
		argv[0]);
		comment = stdin;
	    }
	    else
		TRYOPEN (argv[0], comfile, "r", comment);
	    while (fgets(buf, sizeof buf, comment) != NULL && strlen(buf) != 1)
		Add_comment (ih, buf);
	}
	if (string != NULL) {
	    Add_comment (ih, string);
	    Add_comment (ih, "\n");
	}
	inhibit_hdr_date();
	(void) write_header (ih, tstrm);
	while ((c = getc (in)) != EOF)
	    (void)putc (c, tstrm);
	(void) fclose (tstrm);
	(void) fclose (in);
	tstrm = fopen (tempfile, "r");
	spsassert(tstrm,"comment: cannot reopen temp file!");
	(void) unlink (tempfile);
	TRYOPEN(argv[0], argv[optind], "w", out);
		
	while ((c = getc (tstrm)) != EOF)
	    (void)putc (c, out);
	exit (0);
    }

    print_comment (aflag, ih, 0);
    return 0;

}

/* this function is called recursively to print the comment or
   typtxt field 
*/

void
print_comment (flag, hd, level)
int     flag,
        level;
struct header  *hd;
{
    int     i;
    char   *p;

    if (tflag)
	p = hd -> variable.typtxt;
    else
	p = hd -> variable.comment;

    print_txtcomment(p,level); 
    if (flag == 0)
	return;
    level++;
    for (i = 0; hd -> variable.srchead[i] && i < hd -> variable.nheads; i++) {
	print_comment (flag, hd -> variable.srchead[i], level);
    }
    return;
}

/* this is called to add the comment to the text field.  If the field
   is the standard comment field, it just calls add_comment.  There is
   no function like add_comment for the typtxt field
*/

void
Add_comment (hd, str)
struct header  *hd;
char   *str;
{
    if (tflag == 0) {
	add_comment (hd, str);
	return;
    }

    if (hd -> variable.typtxt == NULL) {
	hd -> variable.typtxt = savestring (str);
	return;
    }

    hd -> variable.typtxt = realloc (hd -> variable.typtxt,
	    (unsigned) (strlen (str) + strlen (hd -> variable.typtxt) + 2));
    (void) strcat (hd -> variable.typtxt, str);
}

#define LINE_LENGTH 79


/* this function prints a text field in a block limited by the
   line_length and the indent level.  This was lefted from psps.
*/

void
print_txtcomment(text, indent)
char *text;			/* text to print */
int  indent;			/* number of spaces to indent */
{
/* used to print out the text field and comment field in the
 * header; if any text exceeds the LINE_LENGTH limit, a line-feed
 * is put out and the indentation of the continuation is set to
 * an additional 2 spaces
 */
    int count = 0;		/* position in line */
    if(text == NULL) return;
    while (*text != '\0') {
	if (count == 0) {
	    tab(indent);
	    (void) fputc(*text, stdout);
	    count = indent + 1;	    
	}
	else if (count < LINE_LENGTH) {
	    (void) fputc(*text, stdout);
	    count++;
	}
	else {
	    (void) putchar('\n');
	    tab(indent + 2);
	    (void) fputc(*text, stdout);
	    count = indent + 3;
	}
    if (*text == '\n') count = 0;
    text++;
    }
    if(text[strlen(text)-1] != '\n') (void)putchar('\n');
}

void tab(indent)
int indent;

{
   while(indent--) (void)printf(" ");
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
