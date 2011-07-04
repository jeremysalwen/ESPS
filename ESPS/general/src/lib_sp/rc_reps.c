 /*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 *
 * Function: rc_reps
 *
 * Written by:  David Burton
 * Checked by:  Alan Parker
 *
 * rc_reps -- transform RCs to another spectral representation
 */


#ifndef lint
	static char *sccs_id = "@(#)rc_reps.c	1.12	12/21/96 ESI";
#endif


/*
 * system include files
 */

#include <stdio.h>

/*
 * defines
 */

#define ERROR_EXIT(text) {(void) fprintf(stderr, "rc_reps: %s - exiting\n", text); return(-1);}
#define Fprintf (void)fprintf
/*
 * system functions and variables
 */

#ifndef DEC_ALPHA
    char *calloc();
#endif
void free();


/*
 * ESPS Include Files
 */
#include <esps/esps.h>
#include <esps/fea.h>    /*to define data_rec*/
#include <esps/anafea.h> /*only need this parameter type definition*/
#include <esps/spsassert.h>

/*
 * ESPS Functions
 */

int rc_to_lar();

/*
 * Local functions
 */
static void rc2cep();
static void rc2lar();
static void rc2afc();
static void rc2lsf();
static void rc2auto();


/*
 * function 
*/
int
rc_reps(rc, output, output_type, size, bwidth, frq_res)
     float *rc, *output;       /* data arrays*/
     int output_type;          /*LAR, AUTO, AFC, CEP, or LSF*/
     float bwidth, frq_res;    /*needed to do LSF conversion*/
     int size;                 /*number of reflection coefficients*/
{
    int  i;

	
/*
 * Transform from Reflection Coefficients to desired spectral type
*/

   switch (output_type) {
       case CEP:
	    (void)rc2cep(rc, output, size);
	    break;
       case LAR:
	    (void)rc2lar(rc, output, size);
	    break;
	case AFC:
	    (void)rc2afc(rc, output, size);
	    break;
	case LSF:
	    (void)rc2lsf(rc, output, size, bwidth, frq_res);
	    break;
	case AUTO:
	    (void)rc2auto(rc, output, size);
	    break;
	case RC:
	    Fprintf(stderr, "rc_reps: Output type (RC) is same as input");
	    for(i=0; i < size; i++)
	        output[i] = rc[i];
	    return(0);
	default: /*shouldn't get here*/
	    Fprintf(stderr, 
		    "RC_REPS: Invalid output spectral type specified %d\n", 
		    output_type);
	    return(-1);
   }
   return(0);
}

static void
rc2cep(input, output, size)
float *input, *output;
int size;
{
    float *tmp;
    int afc2cep();
    /*
     * Allocate memory
    */
    tmp = (float *) calloc((unsigned) size, sizeof(float));    
    
    /*first convert to AFCs */
    rc2afc(input, tmp, size);

    /* Now convert AFCs to CEPs*/
    (void)afc2cep(tmp, output, size);
    
    free((char *)tmp);
}


static void
rc2lar(input, output, size)
float *input, *output;
int size;
{
    int i;
    float lar;    

	for(i=0; i<size; i++) {
	    (void)rc_to_lar((double)input[i], &lar);
	    output[i] = lar;
	}

}

static void
rc2afc(input, output, size)
float *input, *output;
int size;
{
    float *afc;
    int refl_to_filter();
    int i;


/*
 * allocate memory
*/
    afc = (float *) calloc((unsigned)(size+1),  sizeof(float));
    spsassert(afc != NULL, "rc2afc: Couldn't calloc memory for afc values");


/*
 * process frame
*/
	(void)refl_to_filter(input - 1, afc, (int)(size+1));
	for(i=0; i<size; i++)
	    output[i] = afc[i+1];

/*
 * free memory
*/
    free((char *)afc);
}

static void
rc2lsf(input, output, size, bwidth, frq_res)

float bwidth, frq_res;
float *input, *output;
int size;

{
    
    float *lsf;
    int i;
    int pc_to_lsf();


/*
 * allocate memory
*/

    lsf = (float *) calloc((unsigned)(size),  sizeof(float));
    spsassert(lsf != NULL, "rc2lsf: Couldn't calloc memory for lsf values");


/*
 * First convert to autoregressive filter coefficients
*/
    rc2afc(input, output,  size);


/*
 * Now convert to line spectral frequencies
*/

        (void)pc_to_lsf(output, lsf, (int)size, bwidth, frq_res);
	for(i=0; i<size; i++)
	    output[i] = lsf[i];


/*
 * free memory
*/
    free((char *)lsf);    
}

static void
rc2auto(input, output,  size)
float *input, *output;
int size;
{
    float *autoc;
    int i;
    void refl_to_auto();
/*
 * allocate memory
*/
    autoc = (float *)calloc ((unsigned)(size+2),  sizeof(float));

/*
 * Process spectral parameters
*/

	(void)refl_to_auto(input - 1, 1., autoc, (int)size);
	for(i=0; i<size; i++)
	    output[i] = autoc[i+2]/autoc[1];

/*
 * free memory
*/
    free((char *)autoc);
}
    
