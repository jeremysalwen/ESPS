/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
  
  Alan Parker, ESI

  sccs: @(#)unix.h	3.5 10/20/93 ESI

*/
 				
/* This file contains declarations for the Unix functions */

#include <stdio.h>
#if !defined(IBM_RS6000) && !defined(DEC_ALPHA)

#if defined(M5500) || defined(M5600)
void exit();
void free();
void rewind();
void clearerr();
void longjmp();
#endif

#if defined(hpux)
void exit();
void free();
#endif

long lseek();
int fclose();
int fflush();
FILE *fopen();
int fseek();
long ftell();
char *malloc(), *calloc(), *realloc();
char *mktemp();
FILE *popen();
int pclose();
char *strcat(), *strncat(); 
int strcmp(), strncmp();
char *strcpy(),  *strncpy();
int strlen();
char *strchr(), *strrchr(), *strpbrk();
int strspn(), strcspn();
char *strtok();
double atof();
long atol();
int atoi();
char *getenv();

#endif
