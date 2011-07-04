/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Research Laboratory, Inc.
 *     All rights reserved."
 *
 * Program: xerlicons.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * utility functions for ERL icons
 */

#ifndef lint
static char *sccs_id = "%W%	%G%	ERL";
#endif

/*
 * system include files
 */
#include <stdio.h>

#include <xview/xview.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/icon.h>
#include <xview/cms.h>


/*
 * esps include files
 */

#include <esps/esps.h>
#if !defined(DS3100) && !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/exview.h>

#define EXV_ICON_BITMAPS
#include <esps/exv_icon.h>

/*
 * defines
 */
#define Fprintf (void)fprintf
#define Fflush (void)fflush

#define DEBUG(n) if (debug_level >= n) Fprintf

#ifndef NULL
#define NULL 0
#endif

/*
 * global variable declarations
 */

extern int debug_level;
extern int do_color; 

Icon
  exv_attach_icon(frame, icon_id, label, trans)
Frame frame;			/* frame to which epi icon gets attached */
int  icon_id;			/* indicates which icon is wanted */
char *label;			/* text label for icon*/
int  trans;			/* flag for transparent icon */
{
  Icon icon;			/* icon handle */
  Server_image   icon_image = NULL;	/* icon image */
  int show_f;			/* is XV_SHOW true for frame? */

  switch (icon_id) {

  case ERL_BORD_ICON:
    icon_image = (Server_image) 
      xv_create(XV_NULL, SERVER_IMAGE, 
		XV_WIDTH,            erlicon_b_width,
		XV_HEIGHT,           erlicon_b_height,
		SERVER_IMAGE_X_BITS, erlicon_b_bits,
		NULL);
    break;

  case ERL_NOBORD_ICON:
    icon_image = (Server_image) 
      xv_create(XV_NULL, SERVER_IMAGE, 
		XV_WIDTH,            erlicon_nb_width,
		XV_HEIGHT,           erlicon_nb_height,
		SERVER_IMAGE_X_BITS, erlicon_nb_bits,
		NULL);
    break;

  case HIST_ICON:
    icon_image = (Server_image)
      xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH,	     histicon_width,
		XV_HEIGHT,	     histicon_height,
		SERVER_IMAGE_X_BITS, histicon_bits,
		0);
    break;

  case IMAGE_ICON:
    icon_image = (Server_image)
      xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH,	     imagicon_width,
		XV_HEIGHT,	     imagicon_height,
		SERVER_IMAGE_X_BITS, imagicon_bits,
		0);
    break;

  case SINE_ICON:
    icon_image = (Server_image)
      xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH,	     sinicon_width,
		XV_HEIGHT,	     sinicon_height,
		SERVER_IMAGE_X_BITS, sinicon_bits,
		0);
    break;

  case SPEC_ICON:
    icon_image = (Server_image)
      xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH,	     specicon_width,
		XV_HEIGHT,	     specicon_height,
		SERVER_IMAGE_X_BITS, specicon_bits,
		0);
    break;

  case P3D_ICON:
    icon_image = (Server_image)
      xv_create(XV_NULL, SERVER_IMAGE,
		XV_WIDTH,	     p3dicon_width,
		XV_HEIGHT,	     p3dicon_height,
		SERVER_IMAGE_X_BITS, p3dicon_bits,
		0);
    break;
  }

  if (icon_image == NULL) 
    return(NULL);
  else {

/* We used to create the icon as owned by frame (i.e., 
 * xvcreate(frame,ICON,...).  But this would lead to core dumps 
 * on certain machines when the frame is destroyed.  This way there's 
 * probably a mild memory leak, but at least it seems to work. 
 */

    icon = (Icon) xv_create(NULL, ICON,
			  ICON_IMAGE,       icon_image,
			  ICON_TRANSPARENT, (trans ? TRUE : FALSE),
			  NULL);

    if (icon != NULL) 
    {

      if (label != NULL)
	xv_set(icon, ICON_TRANSPARENT_LABEL, label, NULL); 

      /* for safety, we turn of XV_SHOW while setting icon (seems to
         avoid segmentation violations in user code) */

/*
      show_f = xv_get(frame, XV_SHOW);

      xv_set(frame, XV_SHOW, FALSE, NULL);
*/

      xv_set(frame, FRAME_ICON, icon, NULL);

/*
      if (show_f) 
	xv_set(frame, XV_SHOW, TRUE, NULL);
*/

    }
    return(icon); 
  }
}

