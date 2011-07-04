/* tpipe.c -- tee a pipeline into two pipelines. Like tee(1) but 
   argument is a command or pipeline rather than a file.

   See the man page tpipe(1) supplied with this software.

   This version uses the unix system calls popen(3), read(2), and
   write(2).  It uses write(2) to write directly to the fileno() of
   of the file pointer stream returned by popen.

   I've tried it out under BSD, System V, and an older version of unix,
   but:

   THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT EXPRESS OR IMPLIED 
   WARRANTY.

   Version 1.02 (4 Mar 1989) (Use fileno())

--
David B Rosen, Cognitive & Neural Systems                  rosen@bucasb.bu.edu
Center for Adaptive Systems                 rosen%bucasb@{buacca,bu-it}.bu.edu
Boston University              {mit-eddie,harvard,uunet}!bu.edu!thalamus!rosen

*/

#include <stdio.h>

/*#define NOHACK*/

#ifndef BUFSIZ
#define BUFSIZ 2048
#endif /*BUFSIZ*/

int main(argc, argv)
     int argc;
     char *argv[];
{
  char buf[BUFSIZ];
  register FILE *subpipeline = NULL;
  register unsigned n;

  if (argc == 2){
    if (*argv[1]) {
      if ((subpipeline = popen(argv[1],"w")) == NULL) {
	fprintf(stderr, "%s: can't create subpipeline %s\n", argv[0], argv[1]);
	exit(1);
      }
    }
  } else if (argc > 2) {
    fprintf(stderr, "usage: %s [pipeline]\n", argv[0]);
    exit(2);
  }

  while ((n = read(0, buf, BUFSIZ)) > 0) {
    write(1, buf, n); /* write to standard output */
    if (subpipeline) {  /* write to subpipeline: */
      write((int)fileno(subpipeline), buf, n);
    }
  }

  if (subpipeline) pclose(subpipeline);
  return 0;
}
