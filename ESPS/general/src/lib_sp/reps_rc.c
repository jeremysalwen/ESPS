 /*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
 *
 * Function: reps_rc
 *
 * Written by:  David Burton
 * Checked by: 
 *
 * reps_rc -- transform spectreal coefficients to reflection coefficients
 */

#ifndef lint
	static char *sccs_id = "@(#)reps_rc.c	1.9 12/1/93 ESI";
#endif

/*
 * system include files
 */

#include <stdio.h>

/*
 * sps include files
*/
#include <esps/esps.h>
#include <esps/fea.h>    /*to define data_rec*/ 
#include <esps/anafea.h> /*only need this for input_type*/

/*
 * defines
 */

#define ERROR_EXIT(text) {(void) fprintf(stderr, "reps_rc: %s - exiting\n", text); return(-1);}
#define Fprintf (void)fprintf

/*
 * system functions and variables
 */


#ifndef DEC_ALPHA
    char *calloc();
#endif
void free();

void cep2rc();
void lar2rc();
void afc2rc();
void lsf2rc();
void auto2rc();

/*
 * function
*/

int
reps_rc(input, input_type, rc, size, bwidth)
float *input, *rc;/*data arrays*/
float bwidth;/*needed for conversion from LSFs*/
int input_type;/*LAR, AUTO, LSF, CEP, or AFC*/
int size;/*number of reflection coeffricients*/
{
    float lar_to_rc();
    int pef_autorc(), get_atal(), lsf_to_pc(), cep2afc();

    int  i;

/*
 * Transform to reflection coefficients 
*/
   switch (input_type) {
       case CEP:
	    (void)cep2rc(input, rc, size);
	    break;
       case LAR:
	    (void)lar2rc(input, rc,  size);
	    break;
	case AFC:
	    (void)afc2rc(input, rc,  size);
	    break;
	case LSF:
	    (void)lsf2rc(input, rc, size, bwidth);
	    break;
	case AUTO:
	    (void)auto2rc(input, rc, size);
	    break;
	case RC:
	    Fprintf(stderr, 
		"reps_rc: Input type (RC) same as output type\n");
	    for(i=0; i < size; i++)
		rc[i] = input[i];
	    return(0);
	default:/*shouldn't get here*/
	    Fprintf(stderr, "Invalid input spectral type %d\n", input_type);
	    return(-1);
   }
   return(0);
}

void
cep2rc(input, rc, order)
float *input, *rc;
int order;
{
    float *tmp;

    /*
     * Allocate memory
    */
    tmp = (float *)calloc((unsigned)order, sizeof(float));

    /*first convert to AFCs*/
    (void)cep2afc(input, tmp, order);

    /*now convert AFCs to RCs*/
    afc2rc(tmp, rc, order);

    /*free memory*/
    free((char *)tmp);
}

void
lar2rc(input, rc, order)
float *input, *rc;
int order;
{
    float lar_to_rc();
    int i;
    for(i=0; i < order; i++)
	    rc[i] = lar_to_rc((double)input[i]);
}

void
afc2rc(input, rc, order)
float *input, *rc;
int order;
{
    double *tmprc, *afc, *autoc; /*needed for pef_autorc()*/
    int i;

/*
 * allocate memory
*/
    autoc = (double *)calloc((unsigned)(order+1),  sizeof (double));
    tmprc = (double *)calloc((unsigned)(order+1),  sizeof(double));
    afc = (double *)calloc((unsigned)(order+1),  sizeof (double));

    spsassert(autoc != NULL,"afc2rc: calloc failed!");
    spsassert(tmprc != NULL,"afc2rc: calloc failed!");
    spsassert(afc != NULL,"afc2rc: calloc failed!");
/*
 * transform spectral parameters
*/

	afc[0] = 1.;/* Fake residual doesn't matter because we throw
			away the autocorrelations*/
	for(i=0; i<order; i++)
	    afc[i+1] = (double)input[i];
	(void)pef_autorc(order, afc, autoc, tmprc);
	for(i=0; i<order;i++)
	    rc[i] = (float)tmprc[i+1];

    free((char *) tmprc);
    free((char *) afc);
    free((char *) autoc);
}

void
lsf2rc(input, rc, order, bwidth)
float *input, *rc;
float bwidth;
int order;
{
    float *pc;

/*
 * allocate memory for pc[]
*/
    pc = (float *)calloc((unsigned)order,  sizeof (float));
    spsassert(pc != NULL, "lsf2rc: pc calloc failed!");
 /*
  * First convert to AFC's 
 */

	(void)lsf_to_pc(input, pc, (int)order, bwidth);

/*
 * Now convert to reflection coefficients
*/
    afc2rc(pc, rc, order);

/*
 * Free memory
*/
    free((char *)pc);
}

void
auto2rc(input, rc, order)
float *input, *rc;
int order;
{
    double *autoc;
    float *afc;
    float *tmprc;
    float gainptr;
    int i;
    
    /*
     * allocate memory for arrays
    */

    autoc = (double *)calloc((unsigned)(order+1), sizeof(double));
    afc = (float *) calloc ((unsigned)(order+1),  sizeof(float));
    tmprc = (float *)calloc((unsigned)(order+1),  sizeof(float));

    spsassert(autoc != NULL, "auto2rc: calloc failed!");
    spsassert(afc != NULL, "auto2rc: calloc failed!");
    spsassert(tmprc != NULL, "auto2rc: calloc failed!");
  
    /*
     * copy data into autoc[] and convert to reflection coefficients
    */

	for(i=0; i < order; i++)
	    autoc[i+1] = (double)input[i];
	(void)get_atal(autoc, (int)order, afc, tmprc, &gainptr);
	for(i=0; i < order; i++)
	    rc[i] = tmprc[i+1];

/*
 * free memory
*/
    free((char *)autoc);
    free((char *)afc);
    free((char *)tmprc);
}
