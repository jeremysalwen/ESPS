
/*	
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
*/


#ifdef SCCS
	static char *sccsid = "@(#)hamming.c	1.3 11/19/87 ESI";
#endif



static int hamm_codes[16] = { /* hex representation */
0X00,0Xd2,0X55,0X87,0X99,0X4b,0Xcc,0X1e,0Xe1,0X33,0Xb4,0X66,0X78,0Xaa,0X2d,0Xff
};



/* This function takes an integer in the range [0,15] and returns
an integer that is coded in an (8,4) Hamming code. This code allows the 
correction of all single errors and the detection of all double errors. 
hamm_dec() is the function that decodes the Hamming coded integer.


Written by: David Burton
*/

int
hamm_enc(data, code)
int data;
short *code;
{
if(data > 15 || data < 0)
   return(-1);    /* first check if data is out of range */
else
  {
  *code = hamm_codes[data];   /* code data */
  return (0);
  }
}



/* This function accepts an 8-bit integer (code), decodes it according
to an (8,4) Hamming code, and returns a 4-bit integer (data). 
It is the compliment of hamm_enc(). Hamm_dec corrects single bit errors
and detects double errors. For double errors, a value of -1 is returned
in data.


Written by: David Burton
*/

int
hamm_dec(data,code)
int code;
short *data;
{

static int decode[256] = { /* hex representation - 16 per row */
0X00,0X00,0X00,-0X1,0X00,-0X1,-0X1,0X03,0X00,-0X1,-0X1,0X05,-0X1,0X0e,0X07,-0X1,
0X00,-0X1,-0X1,0X09,-0X1,0X02,0X07,-0X1,-0X1,0X04,0X07,-0X1,0X07,-0X1,0X07,0X07,
0X00,-0X1,-0X1,0X09,-0X1,0X0e,0X0b,-0X1,-0X1,0X0e,0X0d,-0X1,0X0e,0X0e,-0X1,0X0e,
-0X1,0X09,0X09,0X09,0X0a,-0X1,-0X1,0X09,0X0c,-0X1,-0X1,0X09,-0X1,0X0e,0X07,-0X1,
0X00,-0X1,-0X1,0X05,-0X1,0X02,0X0b,-0X1,-0X1,0X05,0X05,0X05,0X06,-0X1,-0X1,0X05,
-0X1,0X02,0X01,-0X1,0X02,0X02,-0X1,0X02,0X0c,-0X1,-0X1,0X05,-0X1,0X02,0X07,-0X1,
-0X1,0X08,0X0b,-0X1,0X0b,-0X1,0X0b,0X0b,0X0c,-0X1,-0X1,0X05,-0X1,0X0e,0X0b,-0X1,
0X0c,-0X1,-0X1,0X09,-0X1,0X02,0X0b,-0X1,0X0c,0X0c,0X0c,-0X1,0X0c,-0X1,-0X1,0X0f,
0X00,-0X1,-0X1,0X03,-0X1,0X03,0X03,0X03,-0X1,0X04,0X0d,-0X1,0X06,-0X1,-0X1,0X03,
-0X1,0X04,0X01,-0X1,0X0a,-0X1,-0X1,0X03,0X04,0X04,-0X1,0X04,-0X1,0X04,0X07,-0X1,
-0X1,0X08,0X0d,-0X1,0X0a,-0X1,-0X1,0X03,0X0d,-0X1,0X0d,0X0d,-0X1,0X0e,0X0d,-0X1,
0X0a,-0X1,-0X1,0X09,0X0a,0X0a,0X0a,-0X1,-0X1,0X04,0X0d,-0X1,0X0a,-0X1,-0X1,0X0f,
-0X1,0X08,0X01,-0X1,0X06,-0X1,-0X1,0X03,0X06,-0X1,-0X1,0X05,0X06,0X06,0X06,-0X1,
0X01,-0X1,0X01,0X01,-0X1,0X02,0X01,-0X1,-0X1,0X04,0X01,-0X1,0X06,-0X1,-0X1,0X0f,
0X08,0X08,-0X1,0X08,-0X1,0X08,0X0b,-0X1,-0X1,0X08,0X0d,-0X1,0X06,-0X1,-0X1,0X0f,
-0X1,0X08,0X01,-0X1,0X0a,-0X1,-0X1,0X0f,0X0c,-0X1,-0X1,0X0f,-0X1,0X0f,0X0f,0X0f
};


if (code < 0 || code > 255)
  return(-1);   /* if code is not an 8-bit integer then return */
else
  *data = decode[code];  /* get decode value */

if(*data == -1)
   return 2; /*double bit error*/
else if (code == hamm_codes[*data])
   return 0; /* no errors*/
else
   return 1; /*single error was corrected*/
}

