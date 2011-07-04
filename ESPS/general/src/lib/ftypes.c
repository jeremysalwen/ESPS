/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  ftypes.c								|
|									|
|  Rodney W. Johnson							|
|									|
|  This module defines arrays that contain strings associated with the	|
|  various file types and feature-file subtypes.  It should be updated	|
|  whenever a new FT_ symbol is defined in ftypes.h or a new FEA_ file	|
|  subtype symbol is defined in fea.h.					|
|									|  
+----------------------------------------------------------------------*/
#ifdef SCCS
static char *sccsid = "@(#)ftypes.c	1.7 8/9/91 EPI";
#endif
#include <stdio.h>

char *file_type[] =
   {"NONE", "", "", "", "",					/*0-4*/
    "FT_SPEC", "FT_SPECT_DB", "FT_ANA", "FT_PIT", "FT_SD",	/*5-9*/
    "FT_ROS", "FT_FILT", "FT_SCBK", "FT_FEA",			/*10-13*/
    NULL};
char *fea_file_type[] =
   {"NONE", "FEA_VQ", "FEA_ANA", "FEA_STAT",			/*0-3*/
    "FEA_QHIST", "FEA_DST", "FEA_2KB", "FEA_SPEC",		/*4-7*/
    "FEA_SD", "FEA_FILT",					/*8,9*/
    NULL};
