/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)Xp_pw.h	1.3 9/26/95 ATT/ERL/ESI */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description: remap pw functions for use with Xprinter
 *
 */






#undef pw_vector
#undef pw_write
#undef pw_text
#undef pw_rop
#undef pw_ttext


#define pw_vector Xp_xv_vector
#define pw_write  Xp_xv_rop
#define pw_text Xp_xv_text
#define pw_rop Xp_xv_rop
#define pw_ttext Xp_xv_ttext
