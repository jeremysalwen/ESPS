/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
  compares the modify time of two files.  returns 0 only if file 1
  is older than file2.   If file2 is missing, or younger than file1
  0 is returned

*/
 	
static char *sccs_id = "@(#)newer.c	3.1 10/20/87 ESI";


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>




main(argc, argv)
char **argv;
int argc;

{

struct stat buf1, buf2;



	if (argc != 3) {
		fprintf(stderr,"usage: newer file1 file2\n");
		exit (-1);
	}

	if (stat(argv[1], &buf1) != 0) {
		perror(argv[1]);
		exit (-1);
	}

	if (stat(argv[2], &buf2) != 0)
		exit (0);

	if (buf1.st_mtime > buf2.st_mtime) exit (0);
	exit (1);
}
