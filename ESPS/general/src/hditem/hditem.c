/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Joseph T. Buck
 * Checked by:
 * Revised by:  David Burton (for ESPS 3.2)
 *
 * Brief description:
 *  print a generic header item from a file
 */

static char *sccs_id = "%W%	%G%	ESI/ERL";

/*
 * This program is designed mainly for use in shell scripts.
 *
 *	hditem -i item file
 *
 * prints the value of item from a file.  The following items are
 * special and are obtained from the common header:
 *
 *	program version progdate date typtxt comment refer current_path
 *	ndrec
 *
 *	hditem -s -i item file
 *
 * returns a success status if item is present and failure (exit 1) if
 * it is not.  There is no message unless there is a failure from eopen.
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/sd.h>     /*added for sdtofea call*/
#include <esps/feasd.h>  /*added for sdtofea call*/
#include <esps/unix.h>

extern char *optarg;
extern int optind;
char **get_genhd_coded();
struct header *sdtofea();
int debug_level=0;

#ifdef DEC_ALPHA
static char pad[10000];
#endif

main (argc, argv)
char **argv;
{
    char *p = NULL, *in_file = NULL, *eopen (), *item = NULL;
    char *host = NULL;
    int type, size, i, silent = 0, err = 0, hostlen;
    short *p_s;
    long *p_l;
    float *p_f;
    double *p_d;
    char **p_c;
    struct header *h;
    FILE *strm;

 
    while ((i = getopt (argc, argv, "si:")) != EOF) {
	switch (i) {
	    case 's': 
		silent = 1; 
		break;
	    case 'i':
		item = optarg;
		break;
	    default: 
		err++;
	}
    }
    if (err || argc != optind + 1) {
	Fprintf (stderr, "Usage: %s [-s] -i item  file\n", *argv);
	exit (1);
    }

    in_file = argv[optind];
    in_file = eopen (*argv, in_file, "r", 0, 0, &h, &strm);
    (void) fclose (strm);
    /* special case handling for SD files and record_freq or max_value*/
    /* This should be removed when SD files dissapear*/
    if(h->common.type == FT_SD && 
       (strcmp(item, "record_freq") == 0 || strcmp(item, "max_value") == 0)){
      h = sdtofea(h);
    }
    p = get_genhd (item, h);
    size = 0;
    type = CHAR;
    if (p == NULL) {
	if (strcmp (item, "program") == 0)
	    p = h->common.prog;
	else if (strcmp (item, "version") == 0)
	    p = h->common.vers;
	else if (strcmp (item, "progdate") == 0)
	    p = h->common.progdate;
	else if (strcmp (item, "date") == 0)
	    p = h->common.date;
	else if (strcmp (item, "typtxt") == 0)
	    p = h->variable.typtxt;
	else if (strcmp (item, "comment") == 0)
	    p = h->variable.comment;
	else if (strcmp (item, "refer") == 0)
	    p = h->variable.refer;
	else if (strcmp (item, "current_path") == 0)
	    p = h->variable.current_path;
	else if (strcmp (item, "hdvers") == 0)
	    p = h->common.hdvers;
	else if (strcmp (item, "user") == 0)
	    p = h->common.user;
#ifndef CONVEX
	else if (strcmp (item, "hostname") == 0){
	    p = h->variable.current_path;
	    if (p) {
		hostlen = strcspn(p, ":");
		host = (char *)calloc((unsigned)(hostlen+1), sizeof(char));
		(void)strncpy(host, p, hostlen);
		(void)strcat(host,"\0");
		p = host;
	    }
	}
#endif
	else if (strcmp (item, "cwd") == 0){
	    p = h->variable.current_path;
	    if (p) {
		hostlen = strcspn(p,":");
		for(i=0; i <= hostlen; i++)
		    p++;
	    }
	}
	else if (strcmp (item, "ndrec") == 0) {
	    p = (char *)&h->common.ndrec;
	    size = 1;
	    type = LONG;
	}
	else if (strcmp (item, "type") == 0){
	    p = (char *)&h->common.type;
	    size = 1;
	    type = SHORT;
	}
	else {
	    if (!silent) Fprintf (stderr, "%s: item '%s' not found in %s\n",
				*argv, item, in_file);
	    exit (1);
	}
    }
    else type = genhd_type (item, &size, h);
    if (p == NULL && type == CHAR) p = "";	/* prevent *0 */
    if (silent) exit (0);
    switch (type) {
	case DOUBLE:
	    p_d = (double *) p;
	    if (0 < size) Fprintf(stdout, "%f", *p_d++);
	    for (i = 1; i < size; i++) Fprintf(stdout, " %g", *p_d++);
	    break;
	case FLOAT:
	    p_f = (float *) p;
	    if (0 < size) Fprintf(stdout, "%f", *p_f++);
	    for (i = 1; i < size; i++) Fprintf(stdout, " %g", *p_f++);
	    break;
	case LONG:
	    p_l = (long *) p;
	    if (0 < size) Fprintf(stdout, "%ld", *p_l++);
	    for (i = 1; i < size; i++) Fprintf(stdout, " %ld", *p_l++);
	    break;
	case SHORT:
	    p_s = (short *) p;
	    if (0 < size) Fprintf(stdout, "%d", *p_s++);
	    for (i = 1; i < size; i++) Fprintf(stdout, " %d", *p_s++);
	    break;
	case CHAR:
	    if (size == 0) (void) printf("%s", p); /* assume null terminated */
	    else for (i = 0; i < size; i++) (void) putchar(*p++);
	    break;
	case CODED: 
	    p_c = get_genhd_coded(item,h);
	    if (0 < size) Fprintf (stdout, "%s", *p_c++);
	    for (i = 1; i < size; i++) Fprintf (stdout, " %s", *p_c++);
	    break;
        default:
	    Fprintf(stderr, 
	    "hditem: generic header item is an unsupported type - exiting.\n");
	    exit(1);
    }
    (void)putchar ('\n');
    return 0;
}
