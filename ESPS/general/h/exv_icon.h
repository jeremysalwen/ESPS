/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * @(#)exv_icon.h	1.7	2/20/96	ERL
 * Program: exv_icon.h
 *
 * Written by:  John Shore
 * Checked by:
 *
 * defines for ESPS xview library
 */

/*
 * Ordinary programs will only #include <esps/exview.h>, which contains
 * 	#undef EXV_ICON_BITMAPS
 *	#include <esps/exv_icon.h>
 * and causes only the public stuff below to be include.
 * The module xerlicons.c will in addition have
 *	#define EXV_ICON_BITMAPS
 *	#include <esps/exv_icon.h>
 * to get the icon bitmap definitions in the various esps/*icon*.h files.
 */

#ifdef EXV_ICON_BITMAPS

/* PRIVATE to xerlicons.c */

/*icon definitions*/

#include <esps/erlicon_b.h>	/* ERL icon with square border */
#include <esps/erlicon_nb.h>	/* ERL icon without border */
#include <esps/histicon.h>	/* histogram icon */
#include <esps/imagicon.h>	/* image (gray-scale spectrum) icon */
#include <esps/sinicon.h>	/* sampled-data (sine-wave) icon */
#include <esps/specicon.h>	/* spectrum-plot icon */
#include <esps/p3dicon.h>	/* plot3d icon */

#else

/* PUBLIC */

/* symbolic names for icons---keep this list in 1-to-1 correspondence
 * with the list above.
 */

#define ERL_BORD_ICON     0	/* ERL icon with square border */
#define ERL_NOBORD_ICON   1	/* ERL icon without border */
#define HIST_ICON	  2	/* histogram icon */
#define IMAGE_ICON	  3	/* image (gray-scale spectrum) icon */
#define SINE_ICON	  4	/* sampled-data (sine-wave) icon */
#define SPEC_ICON	  5	/* spectrum-plot icon */
#define P3D_ICON	  6	/* plot3d icon */

/* other icon-related defs */

#define TRANSPARENT   1
#define OPAQUE        0

#endif

