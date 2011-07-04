/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights		|
|     reserved."							|
|									|
+-----------------------------------------------------------------------+
|									|
|  man_name.c								|
|									|
|  Rodney Johnson							|
|									|
|  normalize names of ESPS manual-page files for eman			|
|									|
+----------------------------------------------------------------------*/
#ifdef SCCS
static char *sccsid = "@(#)man_name.c	3.1 10/20/87 ESI";
#endif

#include <stdio.h>

#define BUFLEN 100  /* length of buffer for reading from stdin */
#define FMT "%100s" /* max width equals buffer length */
#define MAXLEN 12   /* max length of <name>.<ext> */
#define MINEXT 2    /* min length of .<ext> */
#define MAXEXT 3    /* max length of .<ext> */

void	exit();
int	strlen();
int	strcmp();
int	rename();

void	printnorm();
void	renamnorm();
int	norm();
void	rem_us();

static char *ProgName = "man_name";

main(argc, argv)
    int	    argc;
    char    **argv; 
{
    char    buf[BUFLEN+1];
    int     i;

    if (argc == 1)
	while(scanf(FMT, buf) != EOF)
	{
	    printnorm(buf);
	}
    else if (strcmp(argv[1], "-r") != 0)
	for (i = 1; i < argc; i++)
	{
	    printnorm(argv[i]);
	}
    else if (argc == 2)
	while(scanf(FMT, buf) != EOF)
	{
	    renamnorm(buf);
	}
    else
	for (i = 2; i < argc; i++)
	{
	    renamnorm(argv[i]);
	}
	
    exit(0);
    /*NOTREACHED*/
}

void
printnorm(manp)
    char    *manp;
{
    char    file[MAXLEN+1];

    if (norm(file, manp))
	(void) printf("%s\n", file);
    else
	(void) fprintf(stderr,
		"%s: Badly formed input: \"%s\".\n", ProgName, manp);
}

void renamnorm(manp)
    char *manp;
{
    char    file[MAXLEN+1];

    if (norm(file, manp))
    {
	if (strcmp(file, manp) != 0 && rename(manp, file) != 0)
	{
	    (void) fprintf(stderr,
		"%s: Can't rename %s to %s.\n", ProgName, manp, file);
	    exit(1);
	}
    }
    else
    {
	(void) fprintf(stderr,
		"%s: Badly formed input: \"%s\".\n", ProgName, manp);
	exit(1);
    }
}

int
norm(file, manp)
    char    *file;
    char    *manp;
{
    char    name[MAXLEN-MINEXT+1];
    char    *ext = NULL;
    char    *p;
    int	    elen;

    for (p = manp; *p != '\0'; p++) if (*p == '.') ext = p;
    if (ext == NULL) return 0;
    elen = strlen(ext);
    if (elen < MINEXT || elen > MAXEXT) return 0;
    *ext = '\0';
    rem_us(name, manp, MAXLEN - elen);
    (void) sprintf(file, "%s.%s", name, ext+1);
    *ext = '.';
    return 1;
}
/*
 * Copy up to n non-underscore characters from s2 to s1.
 * Terminate with a null.
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
