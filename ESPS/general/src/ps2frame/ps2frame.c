/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: Awful hack to fixup xprinter output for Frame
 * This program deletes the BoundingBox: (atend), moves the other
 * BoundingBox: statement to the top of the file
 *
 */

static char *sccs_id = "@(#)ps2frame.c	1.1	6/28/93	ERL";

#include <stdio.h>
#define LINE_SIZE 100000

FILE *tmpfile();

main(argc,argv)
int argc;
char **argv;
{
	char line[LINE_SIZE];
        char real_bbox[256];
	int got_bb_atend=0;
	FILE *tmp = tmpfile();
	FILE *infile=NULL;

	if (argc != 2) {
	  fprintf(stderr,"usage: ps2frame file\n");
	  exit(1);
	}
	if(!tmp) {
	  fprintf(stderr,"Sorry, cannot open tmp file\n");
          exit(1);
        }
	infile = fopen(argv[1], "r");
	while (fgets(line, LINE_SIZE, infile)) {
	  if(strcmp("%%BoundingBox: (atend)\n",line) == 0)
	   got_bb_atend=1;
	  else if(got_bb_atend && 
	   line[0]=='%' && line[1]=='%' && line[2]=='B' && line[3]=='o' &&
	   line[4]=='u' && line[5]=='n' && line[6]=='d' && line[7]=='i' &&
	   line[8]=='n' && line[9]=='g' && line[10]=='B' && line[11]=='o' &&
	   line[12]=='x' && line[13]==':')
	    strcpy(real_bbox, line);
 	  else
	   fputs(line,tmp);
	}

	rewind(tmp);
	(void)fclose(infile);
	infile = fopen(argv[1], "w");
	if(!infile) {
	  fprintf(stderr,"Cannot open file %s for writing -\n",argv[1]);
	  fprintf(stderr,"will leave result in /var/tmp/ps2frame\n");
	  infile = fopen("/var/tmp/ps2frame","w");
	  if(!infile) {
	    fprintf(stderr,"Sorry, cannot open tmp file\n");
	    exit(1);
	  }
	}
	while(fgets(line, LINE_SIZE, tmp)) {
	  fputs(line,infile);
	  if(real_bbox[0]) {
	   fputs(real_bbox, infile);
	   real_bbox[0] = 0;
	  }
        }

}
