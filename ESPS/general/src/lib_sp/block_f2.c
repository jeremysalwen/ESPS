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
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)block_f2.c	1.3	9/12/96	ERL";

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feafilt.h>
#include <esps/feasd.h>

extern int debug_level;

#define CDATA 10
#define MCHAN 100
  
#define FIR_S_R 1    /* filtertype - single or multichannel - real or complex*/
#define FIR_S_C 11
#define FIR_M_R 101
#define FIR_M_C 111
#define IIR_S_R 2
#define IIR_S_C 12
#define IIR_M_R 102
#define IIR_M_C 112

/*
 * ESPS Function
 */

int is_type_complex();

/*
 * Filter2 Utility Functions
 */

int iirfunc();
int firfunc();


/* takes data as double or double_cplx from calling program for efficiency */

int
block_filter2( nsamp, in, out, filtrec)
     long nsamp;
     struct sdata *in, *out;
     struct fdata **filtrec;
{

  int no_channel;
  short in_dtype, out_dtype;            /* input/output data type */
  void *inptr, *outptr;                 /* will point to in->rec or in->data */
  void **inptrm, **outptrm;
  int code = 0, codetmp;
  int chan;

/* 
 * check in/output data type, get pointer to data 
 */

  if( in->rec != NULL && in->data != NULL){
    Fprintf(stderr,"block_filter2: both in->rec, in->data are non-NULL\n");
    return(ERR);
  }
  if( in->rec == NULL && in->data == NULL){
    Fprintf(stderr,"block_filter2: both in->rec, in->data are NULL\n");
    return(ERR);
  }

  if( in->rec != NULL ){
    in_dtype = in->rec->data_type;   /* either double or double_cplx */
    inptr = (void *)in->rec->data;
    inptrm = (void **) in->rec->ptrs;
    no_channel = in->rec->num_channels;
  }
  else{
    in_dtype = in->data_type;    /* either double or double_cplx */
    inptr = (void *)in->data;
    inptrm = (void **)in->ptrs;
    no_channel = in->no_channel;
  }

 if( out->rec != NULL ){
    out_dtype = out->rec->data_type;
    outptr = (void *)out->rec->data;
    outptrm = (void **)out->rec->ptrs;
  }
  else{
    out_dtype = out->data_type;
    outptr = (void *)out->data;
    outptrm = (void **)out->ptrs;
  }

  if(debug_level > 1)
    Fprintf(stderr, "block_filter2: no_channel %d, input_type %d output_type %d (must be 1 or 11 for DOUBLE or DOUBLE_CPLX)\n", 
	    no_channel, in_dtype, out_dtype);

  if( is_type_complex(in_dtype) == YES) code += CDATA;
  if( no_channel != 1) code += MCHAN;
  codetmp = code;
  for (chan = 0; chan < no_channel; chan ++){
    
    code = codetmp + filtrec[chan]->filtertype;
    if(debug_level > 1)
      Fprintf(stderr,"block_filter2: code type: %d\n", code);

    switch(code){
    case FIR_S_R:                           /* FIR, single chan, real data */
      {
	struct firfilt *arpt;
	
	arpt = (struct firfilt *) filtrec[chan]->arch;

	if( 0 != firfunc( nsamp, (double *) inptr, (double *) outptr, 
			 NULL, NULL, arpt)){
	  Fprintf(stderr,"block_filter2: error in FIR_S_R\n");
	  return(ERR);
	}
	break;
      }
    case FIR_S_C:                       /* FIR, single chan, complex data */
      {
	struct firfilt *arpt;
	arpt = (struct firfilt *) filtrec[chan]->arch;
	if( 0 != firfunc( nsamp, NULL, NULL, (double_cplx *) inptr,
			 (double_cplx *) outptr, arpt)){
	  Fprintf(stderr,"block_filter2: error in FIR_S_C\n");
	  return(ERR);
	}
	break;
      }
    case FIR_M_R:                          /* FIR, multi chan, real data */
      {
	struct firfilt *arpt;
	static double *input, *output, **data;
	static long allo_size;
	int i;

	arpt = (struct firfilt *) filtrec[chan]->arch;
	data = (double **) inptrm;

	if( !input) input = (double *) malloc(nsamp *sizeof(double));
	spsassert(input, "block_filter2: input malloc failed");
	if(allo_size !=nsamp)
	  input = (double *) realloc(input, nsamp * sizeof(double));

	if( !output) output = (double *) malloc(nsamp *sizeof(double));
	spsassert(output, "block_filter2: output malloc failed");
	if(allo_size !=nsamp)
	  output = (double *) realloc(output, nsamp * sizeof(double));

	for( i=0; i<nsamp; i++) input[i] = data[i][chan];

	if( 0!= firfunc( nsamp, input, output, NULL, NULL, arpt)){
	  Fprintf(stderr,"block_filter2: error in FIR_M_R\n");
	  return(ERR);
	}
	data = (double **) outptrm;
	for( i=0; i<nsamp; i++) data[i][chan] = output[i];
	break;
      }
    case FIR_M_C:                          /* FIR, multi chan, complex data*/
      {
	struct firfilt *arpt;
	static double_cplx *input, *output;
	register double_cplx **data;
	static long allo_size;
	register int i;

	arpt = (struct firfilt *) filtrec[chan]->arch;
	data = (double_cplx **) inptrm;

	if( !input) input = (double_cplx *) malloc(nsamp *sizeof(double_cplx));
	if(allo_size !=nsamp)
	  input = (double_cplx *) realloc(input, nsamp * sizeof(double_cplx));

	if( !output)output =(double_cplx *) malloc(nsamp *sizeof(double_cplx));
	if(allo_size !=nsamp)
	  output =(double_cplx *) realloc(output, nsamp * sizeof(double_cplx));

	for( i=0; i<nsamp; i++)  input[i] = data[i][chan];

	if( 0!= firfunc( nsamp, NULL, NULL, input, output, arpt)){
	  Fprintf(stderr,"block_filter2: error in FIR_M_C\n");
	  return(ERR);
	}
	data = (double_cplx **) outptrm;
	for( i=0; i<nsamp; i++) data[i][chan] = output[i];
	break;
      }
    case IIR_S_R:                            /* IIR, sigle chan, real data */
      {
	struct iirfilt *arpt;
	arpt = (struct iirfilt *) filtrec[chan]->arch;
	if( 0 != iirfunc( nsamp, (double *)inptr, (double *)outptr, 
			 NULL, NULL, arpt)){
	  Fprintf(stderr,"block_filter2: error in IIR_S_R\n");
	  return(ERR);
	}
	break;
      }
    case IIR_S_C:                      /* IIR, single chan, complex data */
      {
	struct iirfilt *arpt;
	arpt = (struct iirfilt *) filtrec[chan]->arch;
	if( 0 != iirfunc( nsamp, NULL, NULL, (double_cplx *) inptr,
			 (double_cplx *) outptr, arpt)){
	  Fprintf(stderr,"block_filter2: error in IIR_S_C\n");
	  return(ERR);
	}
	break;
      }
    case IIR_M_R:                          /* IIR, multichan, real data */
      {
	struct iirfilt *arpt;
	static double *input, *output, **data;
	static long allo_size;
	int i;

	arpt = (struct iirfilt *) filtrec[chan]->arch;
	data = (double **) inptrm;

	if( !input) input = (double *) malloc(nsamp *sizeof(double));
	spsassert(input, "block_filter2: input malloc failed");
	if(allo_size !=nsamp)
	  input = (double *) realloc(input, nsamp * sizeof(double));

	if( !output) output = (double *) malloc(nsamp *sizeof(double));
	spsassert(output, "block_filter2: output malloc failed");
	if(allo_size !=nsamp)
	  output = (double *) realloc(output, nsamp * sizeof(double));

	for( i=0; i<nsamp; i++) input[i] = data[i][chan];

	if( 0 != iirfunc( nsamp, input, output, NULL, NULL, arpt)){
	  Fprintf(stderr,"block_filter2: error in IIR_M_R\n");
	  return(ERR);
	}
	data = (double **) outptrm;
	for( i=0; i<nsamp; i++) data[i][chan] = output[i];
	break;
      }
    case IIR_M_C:                          /* IIR, multichan, complex data */
      {
	struct iirfilt *arpt;
	static double_cplx *input, *output, **data;
	static long allo_size;
	int i;

	arpt = (struct iirfilt *) filtrec[chan]->arch;
	data = (double_cplx **) inptrm;

	if( !input) input = (double_cplx *) malloc(nsamp *sizeof(double_cplx));
	if(allo_size !=nsamp)
	  input = (double_cplx *) realloc(input, nsamp * sizeof(double_cplx));

	if( !output)output =(double_cplx *) malloc(nsamp *sizeof(double_cplx));
	if(allo_size !=nsamp)
	  output =(double_cplx *) realloc(output, nsamp * sizeof(double_cplx));

	for( i=0; i<nsamp; i++) input[i] = data[i][chan];

	if( 0 != iirfunc( nsamp, NULL, NULL, input, output, arpt)){
	  Fprintf(stderr,"block_filter2: error in IIR_M_C\n");
	  return(ERR);
	}
	data = (double_cplx **) outptrm;
	for( i=0; i<nsamp; i++) data[i][chan] = output[i];
	break;
      }
    default:
      {
	Fprintf(stderr,"UNKNOWN CODE IN block_filter2\n");
	return(ERR);
      }
    }
  }
  return(0);
}

