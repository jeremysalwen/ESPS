/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Ajaipal S. Virdy
 * Checked by:
 * Revised by:
 *
 * Brief description:  print non_stat_output for fea_stats(1-ESPS)
 *
 */

static char *sccs_id = "@(#)pr_non_sta.c	1.8 9/8/98 ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feastat.h>

#include "global.h"
#include "non_stat.h"

double	sqrt();

void
Print_non_stat_output()
{
    int	    i;
    int	    k;
    int	    ptr;

    int	    d_index = 0;
    int	    f_index = 0;
    int	    l_index = 0;
    int	    s_index = 0;
    int	    b_index = 0;

    double  bias;
    double  scale;
    double  operand;


    /* The bflag was to added to globals.h to support a brief output
     * format needed for EnSig computation. In the brief format, the
     * element locations of the max and min are not printed. This was
     * added June 18, 1998 by David Burton. The previous version is
     * equivalent to bflag = NO.
     */
    if(bflag == NO) {
	Fprintf(stdout,
	"Field Name   Elemt    Min      at        Max      at       Mean       Std. Dev\n");

	Fprintf(stdout,
	"----------   -----  --------   --      --------   --     --------     --------\n");
      }

    for (i = 0; i < nstat; i++) {

	ptr = stat_field[i];

	if (debug_level > 8) {
	    Fprintf (stderr,
	    "\t%s: size of field %s is %d.\n",
	    ProgName, fea_hdr->names[ptr], fea_hdr->sizes[ptr]);
	}


       /*
	* bias is the factor that converts the biased estimate of the
	* variance to the unbiased estimate: n_records / (n_records - 1)
	*/

	bias = 0.0;

	if (Rflag) {
	    scale = 1.0 / (double) n_rec;
	    if (n_rec > 1)
	       bias = (double) n_rec / (double) (n_rec - 1);
	}
	else {
	    scale = 1.0 / (double) (n_items[i] * n_rec);
	    if (n_items[i] * n_rec > 1)
	       bias = (double) (n_items[i] * n_rec) /
		      (double) (n_items[i] * n_rec - 1);
	}

	if (debug_level > 1)
	    Fprintf (stderr,
	    "%s: scale = %g, bias = %g\n", ProgName, scale, bias);

	if((i % 4) == 0 && bflag == YES) {
	    Fprintf(stdout,
	    "==========================================================\n");
	    Fprintf(stdout,
	    "Field_Name Elemt     Min        Max      Mean     Std. Dev\n");
	    Fprintf(stdout,
	    "==========================================================\n\n");
	  }
	if ((fea_hdr->types[ptr] == DOUBLE) && ndouble)
	    for (k = 0; k < (Rflag ? n_items[i] : 1); k++, d_index++) {
		d_info[d_index].mean *= scale;
		operand = 
		    bias * (d_info[d_index].stndev * scale -
		    d_info[d_index].mean * d_info[d_index].mean);

		if (debug_level > 15)
		   Fprintf (stderr,
		   "%s: operand = %g\n", ProgName, operand);

		if (operand < 0.0) /* practically zero */
		   d_info[d_index].stndev = 0.0;
		else
		   d_info[d_index].stndev = sqrt (operand);

		if(bflag == NO) {
		    /* full output - default*/
		    Fprintf (stdout,
		    "%-12s %-5d %10.3e   %-5d %10.3e   %-5d%10.3e   %10.3e\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    d_info[d_index].min, d_info[d_index].min_at,
		    d_info[d_index].max, d_info[d_index].max_at,
		    d_info[d_index].mean, d_info[d_index].stndev);
		} else {
		    /* brief output - the -b option*/
		    Fprintf(stdout,
   		    "%-12.11s %2.2d %10.3e %10.3e %10.3e %9.3e\n\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    d_info[d_index].min, 
		    d_info[d_index].max, 
		    d_info[d_index].mean, d_info[d_index].stndev);
		}
	    }

	if ((fea_hdr->types[ptr] == FLOAT) && nfloat)
	    for (k = 0; k < (Rflag ? n_items[i] : 1); k++, f_index++) {
		f_info[f_index].mean *= scale;
		operand = 
		   (bias * (f_info[f_index].stndev * scale -
		   f_info[f_index].mean * f_info[f_index].mean));

		if (debug_level > 15)
		   Fprintf (stderr,
		   "%s: operand = %g\n", ProgName, operand);

		if (operand < 0.0) /* practically zero */
		   f_info[f_index].stndev = 0.0;
		else
		   f_info[f_index].stndev = sqrt (operand);

		if(bflag == NO) {
		    /* full output - default*/
		    Fprintf (stdout,
		    "%-12s %-5d %10.3e   %-5d %10.3e   %-5d%10.3e   %10.3e\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    f_info[f_index].min, f_info[f_index].min_at,
		    f_info[f_index].max, f_info[f_index].max_at,
		    f_info[f_index].mean, f_info[f_index].stndev);
		} else {
		    /* brief output - the -b option*/
		    Fprintf(stdout,
		    "%-12.11s %2.2d %10.3e %10.3e %10.3e %9.3e\n\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    f_info[f_index].min, 
		    f_info[f_index].max, 
		    f_info[f_index].mean, f_info[f_index].stndev);
		}
	    }

	if ((fea_hdr->types[ptr] == LONG) && nlong)
	    for (k = 0; k < (Rflag ? n_items[i] : 1); k++, l_index++) {
		l_info[l_index].mean *= scale;
		operand = 
		   (bias * (l_info[l_index].stndev * scale -
		   l_info[l_index].mean * l_info[l_index].mean));

		if (debug_level > 15)
		   Fprintf (stderr,
		   "%s: operand = %g\n", ProgName, operand);

		if (operand < 0.0) /* practically zero */
		   l_info[l_index].stndev = 0.0;
		else
		   l_info[l_index].stndev = sqrt (operand);
		if(bflag == NO) {
		    /* full output - default*/
		    Fprintf (stdout,
		    "%-12s %-5d %10ld   %-5d %10ld   %-5d%10.3e   %10.3e\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    l_info[l_index].min, l_info[l_index].min_at,
		    l_info[l_index].max, l_info[l_index].max_at,
		    l_info[l_index].mean, l_info[l_index].stndev);
		} else {
		    /* brief output - the -b option*/
		    Fprintf(stdout,
		    "%-12.11s %2.2d %10.5ld %10.5ld %10.3e %9.3e\n\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    l_info[l_index].min, 
		    l_info[l_index].max, 
		    l_info[l_index].mean, l_info[l_index].stndev);
		}
	    }

	if ((fea_hdr->types[ptr] == SHORT) && nshort)
	    for (k = 0; k < (Rflag ? n_items[i] : 1); k++, s_index++) {
		s_info[s_index].mean *= scale;
		operand = 
		   (bias * (s_info[s_index].stndev * scale -
		   s_info[s_index].mean * s_info[s_index].mean));

		if (debug_level > 15)
		   Fprintf (stderr,
		   "%s: operand = %g\n", ProgName, operand);

		if (operand < 0.0) /* practically zero */
		   s_info[s_index].stndev = 0.0;
		else
		   s_info[s_index].stndev = sqrt (operand);
		if(bflag == NO) {
		    /* full output - default*/
		    Fprintf (stdout,
		    "%-12s %-5d %10d   %-5d %10d   %-5d%10.3e   %10.3e\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    s_info[s_index].min, s_info[s_index].min_at,
		    s_info[s_index].max, s_info[s_index].max_at,
		    s_info[s_index].mean, s_info[s_index].stndev);
		} else {
		    /* brief output - the -b option*/
		    Fprintf(stdout,
		    "%-12.11s %2.2d %10.5d %10.5d %10.3e %9.3e\n\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    s_info[s_index].min, 
		    s_info[s_index].max, 
		    s_info[s_index].mean, s_info[s_index].stndev);
	       }
	    }

	if ((fea_hdr->types[ptr] == BYTE) && nchar)
	    for (k = 0; k < (Rflag ? n_items[i] : 1); k++, b_index++) {
		b_info[b_index].mean *= scale;
		operand = 
		   (bias * (b_info[b_index].stndev * scale -
		   b_info[b_index].mean * b_info[b_index].mean));

		if (debug_level > 15)
		   Fprintf (stderr,
		   "%s: operand = %g\n", ProgName, operand);

		if (operand < 0.0) /* practically zero */
		   b_info[b_index].stndev = 0.0;
		else
		   b_info[b_index].stndev = sqrt (operand);
		if(bflag == NO) {
		    /* full output - default*/
		    Fprintf (stdout,
		    "%-12s %-5d %10d   %-5d %10d   %-5d%10.3e   %10.3e\n",
		    fea_hdr->names[ptr], Rflag ? item_arrays[i][k] : 0,
		    b_info[b_index].min, b_info[b_index].min_at,
		    b_info[b_index].max,  b_info[b_index].max_at,
		    b_info[b_index].mean, b_info[b_index].stndev);
		} else {
		    /* brief output - the -b option*/
		    Fprintf(stdout,
		    "%-12.11s %2.2d %10.5d %10.5d %10.3e %9.3e\n\n",
		    b_info[b_index].min,
		    b_info[b_index].max,
		    b_info[b_index].mean, b_info[b_index].stndev);
               }
	    }

    }  /* end for (i = 0; i < nstat; i++) */

}
