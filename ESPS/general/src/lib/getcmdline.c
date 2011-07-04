/* 
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
  This routine is used to collect the command line arguments
  together into a single string.   It could be used for any
  other purpose if one can think of it.
  If the target string files up, then just return what we can.
  Alan Parker, ESI

 	
 */

#ifndef lint
 static char *sccs_id = "@(#)getcmdline.c	1.7	4/27/88 ESI";
#endif

#define SIZ 256

char *
get_cmd_line(argc,argv)
int argc;
char **argv;
{
  int i;
  static char string[SIZ+3] = {'\0'};
  static int done = 0;		/* set to 1 first time function called */

  if (done) return(string);	/* no need to do this again, since the
				   cmd line is in a static */
  done = 1;

  for (i=0; i<argc; i++) {
    if((strlen(string)+strlen(argv[i])) > SIZ) return(string);
    (void) strcat(string,argv[i]);
    (void) strcat(string," ");
  }
  (void) strcat(string,"\n");
  return(string);
}
