/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: Stub for IBM version
 *
 */

static char *sccs_id = "@(#)ibm_stub.c	1.2	9/26/95	ATT/ERL";

#ifdef IBM_RS6000
#include <stdio.h>

int dsprg()
{fprintf(stderr,"called dspgrg stub in IBM version.\n");return 0;}

int dspwr()
{fprintf(stderr,"called dspwr stub in IBM version.\n");return 0;}

int dspmap()
{fprintf(stderr,"called dspmap stub in IBM version.\n");return 0;}

int dsp_codec()
{fprintf(stderr,"called dsp_codec stub in IBM version.\n");return 0;}

int dsp_freq()
{fprintf(stderr,"called dsp_freq stub in IBM version.\n");return 0;}

int dsp_old()
{fprintf(stderr,"called dsp_old stub in IBM version.\n");return 0;}

int dsptmr()
{fprintf(stderr,"called dsptmr stub in IBM version.\n");return 0;}

int dspld()
{fprintf(stderr,"called dspold stub in IBM version.\n");return 0;}

int dspopn()
{return -1;}

#endif
