/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.		*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* convert.c */
/* convert data type of input file to new data type and write to new file */
/*
  ===========================  convert.doc  ==================================
		Help for the Waves "convert" program
		------------------------------------

convert -{au} -{fbcsdilt} filename

Output type
-----------
a = ascii
u = binary

Sample type
-----------
f = 	float
b or c=	unsigned char
s =	short
d =	double
i or l=	integer
tx = x is single-digit hex number representing a type:
     -------------------------------------------------
#define SIG_DATA_TYPE	0xf	 specifies the (homogeneous) C type 
#define P_CHARS		0x1
#define P_UCHARS	0x2
#define P_USHORTS	0x3
#define P_SHORTS	0x4
#define P_INTS		0x5
#define P_UINTS		0x6
#define P_FLOATS	0x7
#define P_DOUBLES	0x8
#define P_BOOLEAN 	0x9
*/


#ifndef lint
static char *sccs_id = "@(#)convert.c	1.7	9/26/95	ATT/ERL";
#endif

#include <Objects.h>

Signal *convert();

main(argc, argv)
     int argc;
     char **argv;
{
  Signal *s, *s2, *new_signal();
  Header *h, *w_new_header(), *w_write_header();
  int i, j, fd, length, fdj, type, format, newtype;
  char *p, *q;
  double freq, band, strt_time;
  short *data, **dpp;
  char count[9], name[200],t[3];
  
  freq = 10000.0;
  band = -1.0;
  strt_time = 0.0;
  type = P_SHORTS;
  format = SIG_BINARY;
  t[0] = 's';
  t[1] = 'u';
  t[2] = 0;
  for(i=1; i < argc; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'a':
	format = SIG_ASCII;
	break;
      case 'u':
	format = SIG_BINARY;
	break;
      case 'f':
	type = P_FLOATS;
	break;
      case 'c':
      case 'b':
	type = P_UCHARS;
	break;
      case 's':
	type = P_SHORTS;
	break;
      case 'd':
	type = P_DOUBLES;
	break;
      case 'i':
	type = P_INTS;
	break;
      case 'l':
	type = P_INTS;
	break;
      case 't':
	sscanf(&(argv[i][2]),"%x",&type);
	break;
      default:
	printf("Type %s unknown.\n",argv[i]);
	exit(-1);
      }
      if((argv[i][1] == 'a') || (argv[i][1] == 'u'))
	t[1] = argv[i][1];
      else
	t[0] = argv[i][1];
    } else {
      if(band < 0.0) band = freq/2.0;
      s = s2 = NULL;
      data = NULL;
      if(!(s = (Signal*)get_signal(argv[i],0.0,0.0, NULL))) {
	if((fd = open(argv[i],O_RDONLY)) >= 0) {
	  if((length = lseek(fd,0,2)/sizeof(short)) >0) {
	    if((dpp = (short**)malloc(sizeof(short*))) &&
	       (data = (short*)malloc(sizeof(short)*length))) {
	       lseek(fd,0,0);
	       dpp[0] = data;
	       if(read(fd,data,sizeof(short)*length) == (sizeof(short)*length)){
		 close(fd);
		 if((s = new_signal(argv[i], fd, NULL, dpp, length, freq, 1))) {
		   s->band = band;
		   s->start_time = strt_time;
		 } else
		   printf("new_signal failed\n");
	       } else
	       printf("data read operation failed\n");
	     } else
	       printf("data array allocation failure\n");
	  } else
	    printf("seek to end of file failed\n");
	} else
	  printf("File open on %s failed\n",argv[i]);
      }
      if(s) {
	if(s->utils->read_data(s,0.0,-1.0)) {
	  newtype = (s->type & ~(VECTOR_SIGNALS | SIG_FORMAT)) | type | format;
	  printf("Old type:%x;  New type:%x\n",s->type,newtype);
	  sprintf(name,"%s.%s",s->name,t);
	  if(!(s2 = convert(s,newtype))) {
	    printf("Couldn't do type conversion\n");
	    exit(-1);
	  }
	  rename_signal(s2,name);
	  s2->type = (~SIG_FORMAT & s2->type) | format;
	  head_printf(s2->header,"type_code",&(s2->type));
	  head_printf(s2->header,"time",get_date());
	  put_signal(s2);
	  if((s != s2)) free_signal(s2);
	  s2 = NULL;
	} else
	  printf("Problems with read_data()\n");
	if(s) free_signal(s);
	s = NULL;
      } else
	printf("Couldn't create a Signal by any means\n");
    }
  }
}
	    
  
