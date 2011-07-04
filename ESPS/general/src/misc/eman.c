/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  eman - print esps manual pages					|
|									|
|  Joseph T. Buck							|
|	  added -k option - Ajaipal S. Virdy			        |
|	  various revisions - Rodney W. Johnson				|
|									|
|  A man-like program for examining ESPS documentation, but not all	|
|  features of man are implemented.  If a section number is given,	|
|  search the ESPS man directory tree for a file man?/program.? or	|
|  man?/program.?t, where ? stands for the section number.  Otherwise	|
|  search all sections.							|
|									|
|  Before the search, normalize the file name to at most 12		|
|  characters.								|
|									|
|  If a match is found, look for a preformatted version of the		|
|  document, cat?/program.? or cat?/program.?t.  If this is found and	|
|  is more recent than the source page, run a pager program to print	|
|  it.  Otherwise, generate a new formatted version.  If the -t		|
|  option is given, typeset the source.					|
|									|
|  If t follows the section number in the filename, run the source	|
|  through tbl before nroff or troff.					|
|									|
|  If the -k option is given, search the ESPS "whatis" file for		|
|  entries that match.							|
|									|  
+----------------------------------------------------------------------*/
#ifdef SCCS
static char *sccsid = "@(#)eman.c	3.20 10/7/97 ERL";
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <esps/esps.h>
#include <esps/unix.h>


/* #define VTROFF "/usr/esp/etroff" */	/* EPI local version for Eunice */
/* #define Eunice */			/* for the creat() further down */


#ifndef VTROFF
#define VTROFF "vtroff"
#endif

#ifndef DEFAULT_PAGER
#if defined(LINUX)
#define DEFAULT_PAGER "less"
#else
#define DEFAULT_PAGER "more"
#endif
#endif

#ifndef GREP
#define	GREP "grep"
#endif

extern char *build_esps_path();

#ifndef MANDIR
#define MANDIR build_esps_path("/man/man")
#endif

#ifndef CATDIR
#define CATDIR build_esps_path("/man/cat")
#endif

#ifndef ESPS_DBASE
#define	ESPS_DBASE build_esps_path("/man/whatis");
#endif

#ifndef LIBDIR
#define LIBDIR build_esps_path("/lib");
#endif

#if defined(CONVEX) || defined(SONY_RISC) || defined(APOLLO_68K) || defined(LINUX)
#define MAN_MACRO "-man"
#endif

#ifndef MAN_MACRO
#define MAN_MACRO "/usr/lib/tmac/tmac.an"
#endif

#ifdef DEMO
#define MANDIR "./esps/man/man"
#define CATDIR "./esps/man/cat"
#define LIBDIR "./esps/lib"
#define ESPS_DBASE "./esps/man/whatis"
#endif

#define	SYNTAX	USAGE("eman [x debug_lvl] [-t] [-k keyword] [-m dir] [section] program")

#define LOSECT 1
#define HISECT 8

#define MAXLEN 10

#define N_SUFF 2			/* number of suffixes */
static char *suffix[] = {"", "t"};	/* the suffixes */
static int  suflen[] =  { 0,  1 };	/* lengths of the suffixes */


void	rem_us(), copy_s();
char	*savestring();

extern char *optarg;
extern int  optind;

char	*ProgName = "eman";

main(argc, argv)
    int     argc;
    char    **argv;
{
    char    *pager;
    int     c;
    int     debug_level = 0,
	    sect = 0,
	    losect = LOSECT,
	    hisect = HISECT,
	    i, j,
	    mfound,
	    tflag = 0,
	    kflag = 0,
	    tbl,
	    err;

    static char	cmd[2000],
		buf[MAXLEN+1],
		cname[500],
		mname[500];

    struct stat	mbuf, cbuf;

    char    *keyword;
    char    *mandir = MANDIR;
    char    *catdir = CATDIR;
    char    *whatis = ESPS_DBASE;

    if ((pager = getenv("PAGER")) == NULL)
	pager = DEFAULT_PAGER;

    while ( (c = getopt(argc, argv, "x:k:tm:")) != EOF )
	switch (c)
	{
	case 'x':
	    debug_level++;
	    break;
	case 't':
	    tflag++;
	    break;
	case 'k':
	    kflag++;
	    keyword = optarg;
	    break;
	case 'm':
	    mandir = savestring(optarg);
	    mandir=realloc(mandir,(unsigned)(strlen(mandir)+strlen("/man")+2));
	    (void)strcat(mandir,"/man");
	    catdir = savestring(optarg);
	    catdir=realloc(catdir,(unsigned)(strlen(catdir)+strlen("/cat")+2));
	    (void)strcat(catdir,"/cat");
	    whatis = savestring(optarg);
	    whatis=realloc(whatis,(unsigned)(strlen(whatis)+strlen("/whatis")+2));
	    (void)strcat(whatis,"/whatis");
	    break;
	default:
	    SYNTAX;
	}
    if (kflag) {
	(void) sprintf(cmd, "%s -i %s %s | %s",
		GREP, keyword, whatis, pager);
	exit(system(cmd));
    }

    if (argc < optind + 1 || argc > optind + 2)
	SYNTAX;

    if (isdigit(*argv[argc - 2]))
    {
	hisect = losect = sect = atoi(argv[argc - 2]);
	if (sect < LOSECT || sect > HISECT)
	{
	    (void) fprintf(stderr, "eman: bad section number: %d\n", sect);
	    exit(1);
	}
    }

/*
 * On System V Unix the maximum length of a file name is 14.
 * The man page file name length must be at most 12 to allow for
 * the SCCS prefix "s.".  
 * Because this includes a period and a section number at the end,
 * at most 10 characters of the command-line argument can be used.
 * That is reduced to 9 if the "t" suffix is present.
 */
    if (debug_level >= 1)
	(void) fprintf(stderr,
		"%s: strlen(argv[argc - 1]) = %d, argv[argc - 1] = \"%s\"\n",
		ProgName, strlen(argv[argc - 1]), argv[argc - 1]);

    mfound = NO;
    for (i = losect; i <= hisect; i++)
    {
        for (j = 0; j < N_SUFF; j++)
        {
	    /* remove underscores and truncate to max length. */

	    rem_us(buf, argv[argc - 1], MAXLEN - suflen[j]);

	    if (debug_level >= 1)
	    	(void) fprintf(stderr, "%s: buf = \"%s\"\n", ProgName, buf);

            (void) sprintf(mname, "%s%d/%s.%d%s",
		        mandir, i, buf, i, suffix[j]);

	    if (debug_level >= 1)
                (void) fprintf(stderr, "%s: mname = \"%s\"\n", ProgName, mname);

            if (stat(mname, &mbuf) == 0)
            {
	        mfound = YES;
	        break;
            }

	    /* leave underscores in and truncate to max length. */

	    copy_s(buf, argv[argc - 1], MAXLEN - suflen[j]);
  
	    if (debug_level >= 1)
	    	(void) fprintf(stderr, "%s: buf = \"%s\"\n", ProgName, buf);

            (void) sprintf(mname, "%s%d/%s.%d%s",
		        mandir, i, buf, i, suffix[j]);

	    if (debug_level >= 1)
                (void) fprintf(stderr, "%s: mname = \"%s\"\n", ProgName, mname);

            if (stat(mname, &mbuf) == 0)
            {
	        mfound = YES;
	        break;
            }
        }
        if (mfound)
	{
	    tbl = (j == 1);
            break;
	}
    }

    if (!mfound)
    {
        if (sect > 0)
            (void) fprintf(stderr, "eman: %s not found in section %d\n",
	            argv[argc - 1], sect);
        else
            (void) fprintf(stderr, "eman: %s not found\n", argv[argc - 1]);
        exit(1);
    }

/* -t option: troff it */

    if (tflag)
    {
        if (tbl)
            (void) sprintf(cmd, "tbl %s | %s %s - ", mname, VTROFF, MAN_MACRO);
        else
	    (void) sprintf(cmd, "%s %s %s", VTROFF, MAN_MACRO, mname);
	exit(system(cmd));
    }

/*
 * See if the preformatted version is there and younger than the nroff
 *  version.
 */

    (void) sprintf(cname, "%s%d/%s.%d", catdir, i, buf, i);

    if (debug_level >= 1)
	(void) fprintf(stderr, "%s: cname = \"%s\"\n", ProgName, cname);

    if (stat(cname, &cbuf) == 0 && cbuf.st_mtime > mbuf.st_mtime)
    {
        (void) sprintf(cmd, "%s -s %s", pager, cname);
        exit(system(cmd));
    }
    else
    {
        (void) fputs("Reformatting page, please wait... ", stderr);
/*
 * Create a publicly readable/writable file.
 */
        (void) umask(0);

#ifdef Eunice
        if (creat(cname, 0666, "txt", "1version") < 0)
#else
        if (creat(cname, 0666) < 0)
#endif
        {
            (void) fprintf(stderr, "eman: can't create ");
            perror(cname);
            err = 1;
        }
        else if (tbl)
        {
#if defined(HP300) || defined(M5600) || defined(M5500)
            (void) sprintf(cmd, "tbl -TX %s | nroff -u0 %s - | col >> %s",
#else
            (void) sprintf(cmd, "tbl -TX %s | nroff %s - | col >> %s",
#endif
	            mname, MAN_MACRO, cname);
	    if (debug_level)
		(void)fprintf(stderr,"cmd: %s\n",cmd);
            err = system(cmd);
	}
	else
        {
#if defined(HP300) || defined(M5600) || defined(M5500)
	    (void) sprintf(cmd, "nroff -u0 %s %s | col >> %s", MAN_MACRO, mname,
#else
	    (void) sprintf(cmd, "nroff  %s %s | col >> %s", MAN_MACRO, mname,
#endif
		cname);
	    if (debug_level)
		(void)fprintf(stderr,"cmd: %s\n",cmd);
	    err = system(cmd);
	}

/*
 * If we weren't able to update the file, try going straight to the terminal
 */
	if (err)
	{
	    (void) fprintf(stderr, "nroff to file failed!\n");
	    (void) unlink(cname);
	    if (tbl)
		(void) sprintf(cmd, "tbl -TX %s | nroff %s - | col | %s -s",
		    mname, MAN_MACRO, pager);
	    else
		(void) sprintf(cmd, "nroff %s %s | col | %s -s",
		    MAN_MACRO, mname, pager);
	    exit(system(cmd));
	}
	(void) fputs("done\n", stderr);
	(void) sprintf(cmd, "%s -s %s", pager, cname);
	exit(system(cmd));
    }
    return 0;
}

/*
 * Copy up to n non-underscore characters from s2 to s1.
 * Terminate with a null
 */
void
rem_us(s1, s2, n)
    char *s1, *s2;
    int  n;
{
    for ( ; n > 0 && *s2 != '\0'; s2++)
	if (*s2 != '_')
	{
	    *s1++ = *s2;
	    n--;
	}
    *s1 = '\0';
}

/*
 * Copy up to n characters from s2 to s1.
 * Terminate with a null
 */
void
copy_s(s1, s2, n)
    char *s1, *s2;
    int  n;
{
    for ( ; n > 0 && *s2 != '\0'; s2++)
    {
	*s1++ = *s2;
	n--;
    }
    *s1 = '\0';
}
