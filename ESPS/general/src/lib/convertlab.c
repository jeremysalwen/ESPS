/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore (using some code from D.Talkin)
 * Checked by:
 * Revised by:
 *
 * Brief description: convert TIMIT (sphere) label file to waves+ label 
 *
 */

static char *sccs_id = "@(#)convertlab.c	1.6	2/20/96	ERL/ATT";

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif

#define UNL_MARK  "UNL"
#define Fprintf (void)fprintf

#if !defined(DEC_ALPHA) && !defined(HP700)
char *malloc();
FILE *fopen();
#endif

int
convertlab(file_in, file_out, sf, mode, lab_color, unl_color)
char *file_in;			/* input file name */
char *file_out;			/* output file name */
int mode;			/* whether or not to include both endpoints*/
				/* mode == 2 for both; mode == 1 for end */
int  lab_color;			/* color for labels */
int  unl_color;			/* color for UNL marks */
double sf;			/* sampling rate */
{
  int s, e;
  int eold = 0;
  char *line = malloc(500);
  char *labstr = NULL;
  int alloc_size = 0;
  char *label = malloc(100); 
  double time = 0;
  FILE *fp_in = stdin;
  FILE *fp_out = stdout;
  int nlab; 
  int line_no = -1;

  spsassert(file_in != NULL, "NULL file_in passed to convertlab");
  spsassert(file_out != NULL, "NULL file_out passed to convertlab");
  spsassert(sf > 0, "sampling rate to convertlab is not positive");

  if (lab_color < 0) 
    lab_color = 121;

  if (unl_color < 0)
    unl_color = 125;

  if((fp_in = fopen(file_in,"r")) && (fp_out = fopen(file_out,"w"))) {

      Fprintf(fp_out,"#\n", file_in);

      while(fgets(line,500,fp_in)) {

	  /* remove trailing line feed */

	  line[strlen(line) -1] = '\0';

	  line_no++;

	  if ( alloc_size < strlen(line) ) {
	      alloc_size = strlen(line);
	      if (labstr != NULL)
		  free(labstr);
	      labstr = (char *) calloc( alloc_size, sizeof(char));
	      spsassert( labstr != NULL, "can't alloc label memory.");
	  }

	  (void) sscanf(line,"%d %d %s",&s,&e,labstr);

	  if (mode == 1) {

	      if((line_no == 0) && (s != 0)) {

		  Fprintf(stderr, 
		     "1st label in file %s doesn't start at time 0\n",file_in);

		  time = (double)(s)/sf;
		  Fprintf(fp_out,
			  "%12.6lf %4d  %s\n",time, unl_color, UNL_MARK);
		  s = 0;
		}
	      time = (double)(e)/sf;	  
	      if(s != eold) {
		  Fprintf(stderr,
		    "Labels at %d in file %s are not contiguous\n",s, file_in);
		  if (s < eold) 
		    Fprintf(stderr, "\tWARNING: labels overlap\n");
		  Fprintf(fp_out,"%12.6lf %4d  %s\n",
			  (double)(s)/sf, unl_color, UNL_MARK);
		}
	      Fprintf(fp_out,"%12.6lf %4d  %s\n",time, lab_color, labstr);
	      eold = e;
	    }
	  else { /* mode = 2; start and end labels */
	      if (strlen(label) < (strlen(labstr) + 4)) {
		  free(label);
		  label = malloc((unsigned) strlen(labstr) + 4);
		}
	      (void) strcpy(label, labstr);
	      (void) strcat(label, "_S");
	      Fprintf(fp_out,
		      "%12.6lf %4d  %s\n",(double)(s)/sf, unl_color, label);
	      (void) strcpy(label, labstr);
	      (void) strcat(label, "_E");
	      Fprintf(fp_out,
		      "%12.6lf %4d  %s\n",(double)(e)/sf, lab_color, label);
	    }
	}
      (void) fclose(fp_in);
      (void) fclose(fp_out);
      return(1);
    }
  return(0);
}
