/*
**  GETOPT PROGRAM AND LIBRARY ROUTINE
**
**  I wrote main() and AT&T wrote getopt() and we both put our efforts into
**  the public domain via mod.sources.
**	Rich $alz
**	Mirror Systems
**	(mirror!rs, rs@mirror.TMC.COM)
**	August 10, 1986
*/

#ifndef lint
	static char *sccs_id = "@(#)getopt.c	1.1 12/1/87 PD";
#endif

#include <stdio.h>

#if defined(M5500) || defined(M5600)
void exit();
#endif




extern char	*index();
extern int	 optind;
extern char	*optarg;


main(ac, av)
    register int	 ac;
    register char 	*av[];
{
    register char 	*flags;
    register int	 c;

    /* Check usage. */
    if (ac < 2) {
	(void)fprintf(stderr, "usage: %s flag-specification arg-list\n", av[0]);
	exit(2);
    }

    /* Play games; remember the flags (first argument), then splice
       them out so it looks like a "standard" command vector. */
    flags = av[1];
    av[1] = av[0];
    av++;
    ac--;

    /* Print flags. */
    while ((c = getopt(ac, av, flags)) != EOF) {
	if (c == '?')
	    exit(1);
	/* We assume that shells collapse multiple spaces in `` expansion. */
	(void)printf("-%c %s ", c, index(flags, c)[1] == ':' ? optarg : "");
    }

    /* End of flags; print rest of options. */
    (void)printf("-- ");
    for (av += optind; *av; av++)
	(void)printf("%s ", *av);
    exit(0);
    return 0;
}
