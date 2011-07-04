/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
       "(c) Copyright 1987 Entropic Speech, Inc.; All rights reserved"

  R. Alan Parker, ESI, Washington, DC
  Modified by John Shore to work with split general and local libraries
*/

#ifndef lint
	static char *sccs_id = "@(#)echeck.c	3.3	9/9/87 ESI";
#endif

#ifdef lint
#define LINTFILE "dummy"
#define LINTFILEL "dummy"
#endif


#include <stdio.h>

int lintsearch();

#if defined(M5500) || defined(M5600)
void exit();
void rewind();
#endif


main(argc,argv)
int argc;
char **argv;
{
    int gret, lret;
    int i=1;
    FILE *strm, *strml;

    if((strm = fopen(LINTFILE,"r")) == NULL) {
    	(void) fprintf(stderr,"%s: can't open %s\n",argv[0],LINTFILE);
    	exit(1);
    }
#ifdef LINTFILEL
    if((strml = fopen(LINTFILEL,"r")) == NULL) {
    	(void) fprintf(stderr,"%s: can't open %s\n",argv[0],LINTFILEL);
    	exit(1);
    }
#endif
    while(argc-- > 1) {
        gret = lret = 0;
        gret = lintsearch(strm, argv[i]);
#ifdef LINTFILEL
        lret = lintsearch(strml, argv[i]);
        if (lret) (void) fprintf(stdout, 
	  "  Function %s found in local library.\n", argv[i]);
#endif
        if (gret == 0 && lret == 0) (void) fprintf(stdout,
	  "  Function %s not found in general or local libraries.\n", argv[i]);
        i++;
        (void) printf("\n");
    }
    return 0;
}
		
int
lintsearch(file, name)
/*
 * search a lint library and print the lines associated with
 * the function named function
 */
FILE *file;
char *name;
{
    char buf[256];
    int found = 0;
    rewind(file);
    while (fgets(buf,256,file) != NULL) {
	if(found && (*buf != ' ' && *buf != '\t')) break;
	if(found)
	  (void)printf(buf);
	else {
	    if((*buf != ' ' && *buf != '\t') && (strmatch(buf,name) > -1)) {
		(void)printf(buf);
		found++;
	    }
	}
    }
    return found;
}

	  
int
strmatch(s1,s2)
char *s1,*s2;
{
	int i,j,k;

	for (i=0; s1[i] != '\0' && s1[i] != '(' ; i++) {
		for (j=i, k=0; s2[k] != '\0' && s1[j]==s2[k]; j++, k++)
			;
		if (s2[k] == '\0')
			return (i);
	}
	return(-1);
}

