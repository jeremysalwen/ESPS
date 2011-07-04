/*--------------------------------------------------------------+
| ENTROPIC SPEECH, INC.					|
|								|
| This material contains proprietary software of Entropic	|
| Speech, Inc.  Any reproduction, distribution, or		|
| publication without the the prior written permission of	|
| Entropic Speech, Inc.  is strictly prohibited.  Any	|
| public distribution of copies of this work authorized in	|
| writing by Entropic Speech, Inc.  must bear the notice 	|
|								|
|          "Copyright 1986, 1987 Entropic Speech, Inc."           |
|								|
+---------------------------------------------------------------+
|								|
| bhd -- behead ESPS file					|
|								|
| Reads and discards the header of an ESPS file and writes the	|
| remainder without change to the standard output. 		|
|								|
| Can also produce an ESPS file with just a header (no data)	|
| Rodney Johnson, Entropic Speech, Inc.			|
|								|
+--------------------------------------------------------------*/
#ifdef SCCS
    static char *sccs_id = "@(#)bhd.c	3.11 3/14/94 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define SYNTAX USAGE("bhd [-h] [-F] infile [outfile]")
#define THISPROG "bhd"

extern char *optarg;
extern int optind;
int debug_level=0;

main(argc, argv)
    int  argc;
    char **argv;
{
    char    *in_name = NULL;
    FILE    *in_file = stdin;
    struct header *ih;
    char    *out_name = NULL;
    FILE    *out_file = stdout;
    int     c, hflag=0;
    int     Fflag=0;
    long    foreign_hd_length;
    long    *foreign_hd_ptr;

    while ((c = getopt(argc, argv, "hF")) != EOF) {
      switch (c) {
	case 'h':
	  hflag=1;
	  break;
        case 'F':
	  Fflag=1;
	  break;
	default:
	  SYNTAX;
	  break;
	}
      }


    if (argc - optind == 1) {
	out_file = stdout;
    }
    else if (argc - optind == 2)
    	out_name = eopen(THISPROG, argv[optind+1], "w", NONE, NONE, 
		(struct header **)NULL, &out_file);
    else 
	SYNTAX;

    in_name = eopen(THISPROG, argv[optind], "r", NONE, NONE, &ih, &in_file);

#if defined(DS3100) || defined(SUN386i)
    if(ih->common.edr)
      Fprintf(stderr,"bhd: warning, this file is in EDR machine format.\n");
#endif

   foreign_hd_length = get_genhd_val("foreign_hd_length",ih,0.0);

/*
  disable the foreign header so that the foreign header will not be written
  setting the foreign header length to zero is good enough
*/
   if (hflag && foreign_hd_length && !Fflag) 
    *get_genhd_l("foreign_hd_length", ih) = 0;

    if (Fflag && !hflag) {
      foreign_hd_length = get_genhd_val("foreign_hd_length",ih,0.0);
      if (foreign_hd_length) {
        foreign_hd_ptr = get_genhd_l("foreign_hd_ptr",ih);
        if(foreign_hd_ptr && *foreign_hd_ptr) 
         (void)fwrite((char *)(*foreign_hd_ptr),foreign_hd_length,1,out_file);
        else
         Fprintf(stderr,
      "bhd: warning, foreign_hd_length non-zero, but foreign_hd_ptr is NULL\n");
      }
    }

    if (!hflag) 
      while ((c = getc(in_file)) != EOF)
       (void) putc(c, out_file);
    else
      write_header (ih, out_file);
    return 0;
}
