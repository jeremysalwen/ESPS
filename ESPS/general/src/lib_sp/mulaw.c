/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
 				

 *
 * Functions: mu_to_linear() and linear_to_mu()
 *
 * Written by:  David Burton
 * Checked by:  
 *
 * mulaw -- convert linear (PCM coded) data to mu-law data and back
 */


#ifndef lint
	static char *sccs_id = "@(#)mulaw.c	1.5 2/20/96	 ESI";
#endif

/*
 * System include files
*/
#include <stdio.h>

/*
 * Globals
*/

int mu_debug = 0;

/*
 * tables for conversions
*/

static short mu_enc_table[] = {
1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
35, 39, 43, 47, 51, 55, 59, 63, 67, 71, 75, 79, 83, 87, 91, 95,
103, 111, 119, 127, 135, 143, 151, 159, 167, 175, 183, 191, 199, 207, 215, 223,
239, 255, 271, 287, 303, 319, 335, 351, 367, 383, 399, 415, 431, 447, 463, 479,
511, 543, 575, 607, 639, 671, 703, 735, 767, 799, 831, 863, 895, 927, 959, 991,
1055, 1119, 1183, 1247, 1311, 1375, 1439, 1503, 1567, 1631, 1695, 1759, 1823,
1887, 1951, 2015, 2143, 2271, 2399, 2527, 2655, 2783, 2911, 3039, 3167, 3295,
3423, 3551, 3679, 3807, 3935, 4063, 4319, 4575, 4831, 5087, 5343, 5599, 5855,
6111, 6367, 6623, 6879, 7135, 7391, 7647, 7903, 8159};

static short mu_dec_table[16][8] = {
0, 33, 99, 231, 495, 1023, 2079, 4191,
2, 37, 107, 247, 527, 1087, 2207, 4447,
4, 41, 115, 263, 559, 1151, 2335, 4703,
6, 45, 123, 279, 591, 1215, 2463, 4959,
8, 49, 131, 295, 623, 1279, 2591, 5215,
10,53, 139, 311, 655, 1343, 2719, 5471,
12,57, 147, 327, 687, 1407, 2847, 5727,
14,61, 155, 343, 719, 1471, 2975, 5983,
16,65, 163, 359, 751, 1535, 3103, 6239,
18,69, 171, 375, 783, 1599, 3231, 6495,
20,73, 179, 391, 815, 1663, 3359, 6751,
22,77, 187, 407, 847, 1727, 3487, 7007,
24,81, 195, 423, 879, 1791, 3615, 7263,
26,85, 203, 439, 911, 1855, 3743, 7519,
28,89, 211, 455, 943, 1919, 3871, 7775,
30,93, 219, 471, 975, 1983, 3999, 8031,
};

/*
 * function to convert to mu encoded data
*/

static void get_index(); /*binary search to find mu code value*/

int
linear_to_mu(input, output, size)
short input[];    /*PCM input*/
char output[];    /*mu encoded output*/
long size;        /*number of points in input*/

{
  short P = 0;   /* sign bit: 0 for positive (or zero), 1 for negative*/
  short S;   /* segment identifier: range 0 - 7*/
  short Q;  /* quantized step: range 0 - 15*/

  long i;        /*loop counter*/
  short in;      /* holds input magnitude*/

  for(i = 0; i < size; i++){

    /* initialize output value to zero*/
    output[i] = 0;

    in = input[i];

    /* determine sign of input*/
    if(input[i] < 0){
      P = 1;
      in = -input[i];
    }

    /* find index of input in mu_table*/
    get_index(in, mu_enc_table, &S, &Q);

    
    if(mu_debug > 0){
      (void)fprintf(stderr, "linear2mu: input = %d, init output = %d\n", 
		    input[i], output[i]);
      (void)fprintf(stderr, "linear2mu: P = %d, S = %d, Q = %d\n", P, S, Q);
    }
    /* fold P, S, and Q into a byte */
    output[i] |= Q;
    output[i] |= (S << 4);
    output[i] |= (P << 7);
    /* invert for transmission*/
    output[i] = ~output[i];
    /* reset P*/
    P = 0;
  }
return(0);
}

static void
get_index(input, table, S, Q)
short input, *S, *Q;
short table [];
{

/*assume knowledge about table size - i.e = 128*/

  short high = 127, low = 0, mid;

/* do binary search*/
  while(high > low){
    mid = (high + low)/2;

    if(table[mid] < input)
      low = mid + 1;
    else
      high = mid;
  }
  mid = (high + low)/2;

/*integer divide by 16 - find segment*/

  *S = mid/16;

/*modulo 16 to find quantizer level*/

  *Q = mid % 16;

}

/*
 * Function to decode mu coded values
*/
int
mu_to_linear(in, out, size)
char in[];        /*input mu-coded values (bit inverted)*/
short out[];      /*output linear values*/
long size;        /*number of items in the input array*/
{
  char input;       /* holds input data value*/
  long i;           /*loop counter*/
  short S;          /*segment identifier: range 0 to 7*/
  short Q;          /*quantizer step: range 0 to 15*/

  char maskS = 7;    /*mask for lower 3 bits*/
  char maskQ = 15;   /*mask for lower 4 bits*/
  char maskP = 1;    /* still needs to be shifted to 8th bit*/

  maskP <<= 7;

  for(i = 0; i < size; i++){
    input = ~in[i];
    /*get quantizer step*/
    Q = input & maskQ;

    /*get segment identifier*/
    S = ((input >> 4) & maskS);

    /*get output value*/
    out[i] = mu_dec_table[Q][S];

    /*debug output*/
    if(mu_debug > 0)
      (void)fprintf(stderr, "mu_to_linear: S = %d,   Q = %d\n", S, Q);

    /*get sign*/
    if((input & maskP) != 0)
      out[i] = -out[i];
  }
return (0);
}
