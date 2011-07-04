/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:  Rod Johnson
 *
 * Brief description: convert range in seconds to start/end records
 *
 */

static char *sccs_id = "@(#)trngswitch.c	1.3	12/3/91	ERL";

#include <stdio.h>
#include <esps/esps.h>

extern int debug_level; 

static long sec_to_rec();

void
trange_switch(text, hd, startp, endp)
char *text;			/* text of range string */
struct header *hd;		/* header of relevant file */
long *startp, *endp;		/* output start and end records */
{
    double srate;		/* sampling rate */
    double range_start, range_end, start_time;
    char   *p, *index();
    double atof();
    if (text == NULL || *text == '\0')
	return;

    if (hd == NULL) {
	srate = 1;
	start_time = 0;
	if (debug_level) 
	(void) fprintf(stderr, 
		       "trange_switch: WARNING - NULL ESPS header\n"); 
    }
    else {
	srate = get_genhd_val("record_freq", hd, (double) 1);
	start_time = get_genhd_val("start_time", hd, (double) 0); 
    }

    if (*text == ':') 
	range_start = start_time;
    else
	range_start = atof(text);

    *startp = sec_to_rec(range_start, start_time, srate);

    if ((p = index(text, ':'))) {
	if (*(p + 1)) {
	    if (*(p + 1) == '+' && *(p + 2))
		range_end = range_start + atof(p + 2);
	    else
		range_end = atof(p + 1);
	    *endp = sec_to_rec(range_end, start_time, srate);
	}
    }
    else
	*endp = *startp;
    return;
}

static long
sec_to_rec(time, start, rec_per_sec)
double time;			/* the desired time */
double start;			/* time of starting record */
double rec_per_sec;		/* no. records per second */
{
    return((long) (1 + (time - start) * rec_per_sec));
}

