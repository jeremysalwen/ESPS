/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1988, 1989 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: image.h							|
|									|
|  Include file for program image(1-ESPS).				|
|  Defines device codes and names.					|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
/* @(#)image.h	1.5	8/10/89	ESI */

#define DEV_MCD	    1
#define DEV_IMAGEN  2
#define DEV_TYPE    3
#define DEV_RAS	    4
#define DEV_X11	    5
#define DEV_PS	    6
#define DEV_HP	    7

#define DEV_NAMES   {"NONE", "mcd", "imagen", "type", "ras", \
		     "x11", "postscript", "hp", \
		     NULL}

#define HT_OD1	1
#define HT_OD2	2
#define HT_OD3	3
#define HT_OD4	4
#define HT_FS	5
#define HT_FS2	6
#define HT_DK1	7
#define HT_16LVL    8
#define HT_16OD1    9
#define HT_16OD1_2  10
#define HT_16OD1_3  11

#define HT_NAMES    {"NONE",	"od1",	"od2",	"od3",	"od4", "fs", \
		     "fs2", "dk1", "16lvl", "16od1", "16od1_2", "16od1_3", \
		      NULL}

#define FN_LOG	1
#define FN_EXP	2
#define FN_SQ	3
#define	FN_SQRT 4

#define FN_NAMES    {"NONE", "log", "exp", "sq", "sqrt", \
		     NULL}

#define AX_X 1
#define AX_Y 2
