/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

#ifndef lint
static char *sccs_id = "@(#)rmhead.c	1.9	9/26/95	ATT/ERL";
#endif

#include <Objects.h>
#define BSIZE 20000

int	debug_level = 0;

main(argc, argv)
     int argc;
     char **argv;
{
  Signal *s, *get_signal();
  Header *h, *w_new_header(), *w_write_header(), *get_header(), *h2;
  List *l;
  int i, j, fd, length;
  char *p, *q, name[100], buf[BSIZE];
  double freq;
  Selector *se;

  for(i=1; i < argc; i++) {
    if((s = get_signal(argv[i],0.0,0.0,NULL))) {
      sprintf(name,"%s.nh",s->name);
      if((fd = open(name,O_CREAT|O_RDWR|O_TRUNC,0666)) >= 0) {
	while((length = read(s->file,buf,BSIZE)) > 0) {
	  write(fd,buf,length);
	}
	close(fd);
      } else
	printf("Can't open header-stripped output file  %s\n",name);
      free_signal(s);
    } else printf("Couldn't get_signal() %s\n",argv[i]);
  }

}



