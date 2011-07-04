/**************************************************************************
*
* File name:	sf.c
*
* Author:	Paul Lew, General Systems Group, Inc. Salem NH
* Created at:	05/08/86  10:11 AM
* Last update:	02/08/88  00:23 AM  (Edition: 29)
*
* Description:	This program will take standard input  (if no argument
*		specified in  command line) or  the specified  file as
*		form  template and  display   it  on  the terminal  to
*		perform basic form editing function.  It then generate
*		the Csh  or Bourne shell set script  on  stdout  to be
*		executed (ala tset -s).
*
*		This program is designed to  give programmer  an  easy
*		way of doing form filling in shell level.
*
* Update History:
*
*    Date	Modification Description				By
*  --------	------------------------------------------------------	---
*  05/08/86	Initial version						Lew
*  08/04/87	added Help_display flag					Lew
*  12/29/87	modify to add Bourne shell output flag, use getopt()	Lew
*  01/12/88	added init_sno(), added CTRL L redisplay function,	Lew
*		added signal trap handling.
*  01/26/88	added AUTOTAB at end of a field, added NUMERIC field	Lew
*		attribute
*  02/08/88	modified to add perl script output flag			Lew
*
***************************************************************************/
#define	EXTERN
#include	<stdio.h>
#include	<ctype.h>
#include	"form.h"
#include	"basic.h"

char	*Version = "1.8  02/08/88  00:21 AM";
char	*Bugs = "Bug report send to: decvax!gsg!lew (UUCP)";
char	*Copyright = "Copyright by Paul Lew (1987,1988) All rights reserved";
char	*Prgname;			/* program name */

extern	unsigned char	Shell;
extern	int		Help_display;
int			Debug = NO;
int			num_lines = 24;
int			erase_screen = 1;

/*------------------------------------------------------------07/13/84--+
|									|
|	    M a i n    R o u t i n e    S t a r t s    H e r e		|
|									|
+----------------------------------------------------------------------*/
main (argc, argv)
int	argc;		/* number of argument passed */
char	**argv;		/* pointer to argument list */
	{
	int		n;		/* number of files (so far 2)	*/
	char		*fname;		/* form file name */

	Prgname = *argv;
	n = procarg (argc, argv);
	if (n == argc) fname = NULL;	/* read from stdin if no input	*/
	else fname = argv[n];
	edit_form (fname, (int (*)())NULL);
	exit (0);
	}

/*----------------------------------------------------------------------+
|									|
|	proc_arg : process input argument and set global flags		|
|									|
+----------------------------------------------------------------------*/
procarg (argc, argv)
int	argc;
char	**argv;
	{
	int		rvideo = 0;
	int		undline = 0;
	int		hilite = 0;
	char		*fname = NULL;
	int		c;
	extern	char	*optarg;	/* ptr to argument */
	extern	int	optind;		/* remember which one to process next */

	ENTER (procarg);
	while ((c = getopt (argc, argv, "Hbdhmo:prunl:")) != EOF) {
		switch (c) {
			when 'b': Shell = BOURNE;
			when 'd': Debug = YES;
			when 'h': hilite = YES;
			when 'm': Help_display = YES;
			when 'o': fname = optarg;
			when 'p': Shell = PERL;
			when 'r': rvideo = YES;
			when 'u': undline = YES;
			when 'n': erase_screen = NO;
			when 'l': num_lines = atoi(optarg);
			otherwise:help ();
				  exit (1);
			}
		}
	set_options (hilite, rvideo, undline, fname);
	RETURN (optind);
	}

/*-------------------------------------------------------------05/08/86-+
|									|
|		       help : display help message			|
|									|
+----------------------------------------------------------------------*/
help ()
	{
	ENTER(help);
	fprintf (stderr, "%s Version %s\r\n", Prgname, Version);
	fprintf (stderr, "Command Options:\r\n");
	fprintf (stderr, "  -H        display this help message\r\n");
	fprintf (stderr, "  -b        generate Bourne shell output [Csh]\r\n");
	fprintf (stderr, "  -d        debug mode, will show CTRL chars\r\n");
	fprintf (stderr, "  -h        input in highlight mode\r\n");
	fprintf (stderr, "  -m        display selection help automatically\r\n");
	fprintf (stderr, "  -o file   use file as output file\r\n");
	fprintf (stderr, "  -p        generate perl script output\r\n");
	fprintf (stderr, "  -r        input in reverse video mode\r\n");
	fprintf (stderr, "  -u        input in underline mode\r\n");
	EXIT;
	}
