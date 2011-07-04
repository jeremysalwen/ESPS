/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)efree.c	1.6	12/14/95	ERL";

#include <stdio.h>
#include <esps/unix.h>

char *get_stat_file();

#ifdef ESPS
char *program = "efree";
#define CODE 'e'
#endif

#ifdef HTK
char *program = "hfree";
#define CODE 'h'
#endif

int proc_id;
char cmd[100];
char *statfile;
FILE *statfile_fp;

main()
{

	if(!getenv("HOME")) {
		fprintf(stderr,"%s: Cannot find your home directory!\n",
			program);
		msg();
	}

	statfile = get_stat_file(CODE);

	statfile_fp = fopen(statfile,"r");
	if(!statfile_fp) {
		fprintf(stderr,"%s: Cannot read license status file (%s).\n",
			program,statfile);
		msg();
	}

	fscanf(statfile_fp,"%d",&proc_id);
	sprintf(cmd,"kill %d",proc_id);
	system(cmd);
	unlink(statfile);
}

msg()
{
	fprintf(stderr,"To free the license, you will need to find the");
#ifdef ESPS
	fprintf(stderr," process id of the echeckout and kill it.\n");
#else
	fprintf(stderr," process id of the hcheckout and kill it.\n");
#endif
	exit(1);
}
