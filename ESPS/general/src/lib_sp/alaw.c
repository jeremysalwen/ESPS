/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Burton
 * Checked by:
 * Revised by:  Rodney Johnson
 *
 * Functions linear_to_a, a_to_linear, linear_to_a_2, a_to_linear_2
 * convert linear (PCM coded) data to A-law data and back.
 * The "_2" functions invert code bits according to CCITT convention.
 */

static char *sccs_id = "@(#)alaw.c	1.2	7/24/91	ESI/ERL";

/*
 * TABLES FOR CONVERSIONS
 */

static short A_seg_table[128] = {
 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
};

/**//* Decoding table for a_to_linear created by the following. */
/* #include <stdio.h>							*/
/* static short    A_dec_table[256];					*/
/* main() {								*/
/*     int i;								*/
/*     for (i = 0; i < 32; i++) A_dec_table[i] = 2*i + 1;		*/
/*     for ( ; i < 128; i++)    A_dec_table[i] = 2*A_dec_table[i-16];	*/
/*     for ( ; i < 256; i++)    A_dec_table[i] = -A_dec_table[i-128];	*/
/*     for (i = 0; i < 256; i++) {					*/
/*         printf("%6d,", A_dec_table[i]);				*/
/*         if ((i+1)%8 == 0) printf("\n");				*/
/*     }								*/
/*     exit(0);								*/
/* }									*/
/**/

/* Decoding table for a_to_linear. */

static short A_dec_table[256] = {
     1,     3,     5,     7,     9,    11,    13,    15,
    17,    19,    21,    23,    25,    27,    29,    31,
    33,    35,    37,    39,    41,    43,    45,    47,
    49,    51,    53,    55,    57,    59,    61,    63,
    66,    70,    74,    78,    82,    86,    90,    94,
    98,   102,   106,   110,   114,   118,   122,   126,
   132,   140,   148,   156,   164,   172,   180,   188,
   196,   204,   212,   220,   228,   236,   244,   252,
   264,   280,   296,   312,   328,   344,   360,   376,
   392,   408,   424,   440,   456,   472,   488,   504,
   528,   560,   592,   624,   656,   688,   720,   752,
   784,   816,   848,   880,   912,   944,   976,  1008,
  1056,  1120,  1184,  1248,  1312,  1376,  1440,  1504,
  1568,  1632,  1696,  1760,  1824,  1888,  1952,  2016,
  2112,  2240,  2368,  2496,  2624,  2752,  2880,  3008,
  3136,  3264,  3392,  3520,  3648,  3776,  3904,  4032,
    -1,    -3,    -5,    -7,    -9,   -11,   -13,   -15,
   -17,   -19,   -21,   -23,   -25,   -27,   -29,   -31,
   -33,   -35,   -37,   -39,   -41,   -43,   -45,   -47,
   -49,   -51,   -53,   -55,   -57,   -59,   -61,   -63,
   -66,   -70,   -74,   -78,   -82,   -86,   -90,   -94,
   -98,  -102,  -106,  -110,  -114,  -118,  -122,  -126,
  -132,  -140,  -148,  -156,  -164,  -172,  -180,  -188,
  -196,  -204,  -212,  -220,  -228,  -236,  -244,  -252,
  -264,  -280,  -296,  -312,  -328,  -344,  -360,  -376,
  -392,  -408,  -424,  -440,  -456,  -472,  -488,  -504,
  -528,  -560,  -592,  -624,  -656,  -688,  -720,  -752,
  -784,  -816,  -848,  -880,  -912,  -944,  -976, -1008,
 -1056, -1120, -1184, -1248, -1312, -1376, -1440, -1504,
 -1568, -1632, -1696, -1760, -1824, -1888, -1952, -2016,
 -2112, -2240, -2368, -2496, -2624, -2752, -2880, -3008,
 -3136, -3264, -3392, -3520, -3648, -3776, -3904, -4032,
};

/**//* Decoding table for a_to_linear_2 created by the following. */
/* #include <stdio.h>							*/
/* static short    A_dec_tbl_2[256];					*/
/* main() {								*/
/*     int i;								*/
/*     for (i = 0; i < 32; i++) A_dec_tbl_2[i^0x55] = -(2*i + 1);	*/
/*     for ( ; i < 128; i++)    A_dec_tbl_2[i^0x55] =			*/
/* 					    2*A_dec_tbl_2[(i-16)^0x55]; */
/*     for ( ; i < 256; i++)    A_dec_tbl_2[i] = -A_dec_tbl_2[i-128];	*/
/*     for (i = 0; i < 256; i++) {					*/
/*         printf("%6d,", A_dec_tbl_2[i]);				*/
/*         if ((i+1)%8 == 0) printf("\n");				*/
/*     }								*/
/*     exit(0);								*/
/* }									*/
/**/

/* Decoding table for a_to_linear_2. */

static short A_dec_tbl_2[256] = {
  -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
  -944,  -912, -1008,  -976,  -816,  -784,  -880,  -848,
  -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
  -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
 -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
 -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
 -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
 -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
   -43,   -41,   -47,   -45,   -35,   -33,   -39,   -37,
   -59,   -57,   -63,   -61,   -51,   -49,   -55,   -53,
   -11,    -9,   -15,   -13,    -3,    -1,    -7,    -5,
   -27,   -25,   -31,   -29,   -19,   -17,   -23,   -21,
  -172,  -164,  -188,  -180,  -140,  -132,  -156,  -148,
  -236,  -228,  -252,  -244,  -204,  -196,  -220,  -212,
   -86,   -82,   -94,   -90,   -70,   -66,   -78,   -74,
  -118,  -114,  -126,  -122,  -102,   -98,  -110,  -106,
   688,   656,   752,   720,   560,   528,   624,   592,
   944,   912,  1008,   976,   816,   784,   880,   848,
   344,   328,   376,   360,   280,   264,   312,   296,
   472,   456,   504,   488,   408,   392,   440,   424,
  2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
  3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
  1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
  1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
    43,    41,    47,    45,    35,    33,    39,    37,
    59,    57,    63,    61,    51,    49,    55,    53,
    11,     9,    15,    13,     3,     1,     7,     5,
    27,    25,    31,    29,    19,    17,    23,    21,
   172,   164,   188,   180,   140,   132,   156,   148,
   236,   228,   252,   244,   204,   196,   220,   212,
    86,    82,    94,    90,    70,    66,    78,    74,
   118,   114,   126,   122,   102,    98,   110,   106,
};


/*
 * FUNCTION DEFINITIONS
 */

/* Functions to convert to A-encoded data */

int
linear_to_a(input, output, size)	/* non-CCITT */
    short	*input;		/* PCM input */
    char	*output;	/* A-encoded output */
    long	size;		/* number of points in input */
{
    long	i;		/* loop counter */
    unsigned	in;		/* holds input magnitude */
    unsigned    P;		/* sign bit: 0x80 if negative, else 0x00 */
    unsigned	S;		/* segment identifier: range 0 - 7 */
    unsigned	Q;		/* quantized step: range 0 - 15 */

    for(i = 0; i < size; i++)
    {
	if (input[i] < 0)
	{
	    P = 0x80;
	    in = -1 - input[i];
	}
	else
	{
	    P = 0x00;
	    in = input[i];
	}

	if (in > 4095) in = 4095;

	S = A_seg_table[in >> 5];

	if (S == 0)
	    Q = (in >> 1) & 0x0f;
	else
	    Q = (in >> S) & 0x0f;

	output[i] = P | (S << 4) | Q;
    }

    return 0;
}

int
linear_to_a_2(input, output, size)	/* CCITT */
    short	*input;		/* PCM input */
    char	*output;	/* A-encoded output */
    long	size;		/* number of points in input */
{
    long	i;		/* loop counter */
    unsigned	in;		/* holds input magnitude */
    unsigned    P;		/* sign bit: 0x00 if negative, else 0x80 */
    unsigned	S;		/* segment identifier: range 0 - 7 */
    unsigned	Q;		/* quantized step: range 0 - 15 */

    for(i = 0; i < size; i++)
    {
	if (input[i] < 0)
	{
	    P = 0x00;		/* Opposite sign from linear_to_a. */
	    in = -1 - input[i];
	}
	else
	{
	    P = 0x80;		/* Opposite sign from linear_to_a. */
	    in = input[i];
	}

	if (in > 4095) in = 4095;

	S = A_seg_table[in >> 5];

	if (S == 0)
	    Q = (in >> 1) & 0x0f;
	else
	    Q = (in >> S) & 0x0f;
				/* Alternate magnitude bits inverted. */
	output[i] = (P | (S << 4) | Q) ^ 0x55;
    }

    return 0;
}

/* Functions to decode A-coded values */

int
a_to_linear(in, out, size)		/* non-CCITT */
    char	*in;		/*input A-coded values*/
    short	*out;		/*output linear values*/
    long	size;		/*number of items in the input array*/
{
    long	i;		/*loop counter*/

    for(i = 0; i < size; i++)
	out[i] = A_dec_table[(unsigned char) in[i]];

    return 0;
}


int
a_to_linear_2(in, out, size)		/* CCITT */
    char	*in;		/*input A-coded values*/
    short	*out;		/*output linear values*/
    long	size;		/*number of items in the input array*/
{
    long	i;		/*loop counter*/

    for(i = 0; i < size; i++)
	out[i] = A_dec_tbl_2[(unsigned char) in[i]];

    return 0;
}

