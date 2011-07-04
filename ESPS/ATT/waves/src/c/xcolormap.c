/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xcolormap.c	1.17 9/28/98 ERL/ATT";

#include <Objects.h>
#include <esps/epaths.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <ctype.h>
#include <X11/Xutil.h>

extern int debug_level;
extern char def_cm[];
extern Frame daddy;

#if !defined(LINUX_OR_MAC)
static char cms_name[] = "magic_cmap";
#else
char cms_name[] = "magic_cmap";
#endif

int attachment = FALSE;		/* Is this program an attachment? */
					/* (spectrum, label, etc. must
					 * assign TRUE) */

Cms cms = XV_NULL;		/* XView cololmap segment */

int do_color = 1;		/* color/B&W switch */
int cmap_depth = 8;		/* # of bits supported by hardware */

Xv_singlecolor rgb[MAX_CMAP_SIZE];	/* colormap data */

/* colors used throughout waves; possibly modified in get_globals() and cmap() */

int cmap_size = 128, max_cmap_size = 128, FOREGROUND_COLOR, BACKGRND_COLOR,
  RETICLE_COLOR, TEXT_COLOR, WAVEFORM_COLOR, CURSOR_COLOR, MARKER_COLOR,
  YA5_COLOR, YA4_COLOR, YA3_COLOR, YA2_COLOR, YA1_COLOR, WAVE2_COLOR, CMAP_RESERVED,
  BACKGROUND_COLOR, BACK2_COLOR;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

extern int use_static_cmap;
VisualID visual_id;
int visual_class;
Visual *visual_ptr;

#define GetVisual 6
#define DEFAULT_VISUAL 	-1

static char *VisualNames[] =
{"StaticGray", "GrayScale",
 "StaticColor", "PseudoColor",
 "TrueColor", "DirectColor", "GetVisual", NULL};
static int VisualValues[] =
{StaticGray, GrayScale,
 StaticColor, PseudoColor,
 TrueColor, DirectColor, GetVisual};

void cmap(canvas)
     Canvas canvas;
{
   static char cm_file[MAXPATHLEN] = "no map at all";
   int max_reserved = MAX_CMAP_SIZE - (cmap_size - CMAP_RESERVED);
   register int i, j;
   FILE *cm_fd;
   static int cm_changed = 0;

   if (!do_color)
      return;

   if (strcmp (cm_file, def_cm))
     {				/* Only initialize when necessary. */

	cm_changed = 1;
	(void) FIND_WAVES_COLORMAP (def_cm, def_cm);

	/* KLUGE to fix breakage by find_esps_file() */
	if (!strcmp (def_cm, "."))
	   *def_cm = 0;

	if (*def_cm && (cm_fd = fopen (def_cm, "r")))
	  {
	     int r, g, b;
	     char line[120];
	     float fac = ((double) CMAP_RESERVED) / max_reserved;

	     for (i = 0; i < max_reserved; i++)
	       {
		  if (fgets (line, 120, cm_fd) == NULL)
		     break;
		  sscanf (line, "%d%d%d", &r, &g, &b);
		  j = 0.5 + (fac * i);
		  /* there follows gross stuff to avoid colormap probs when
		     running attachments; essentially, we just skip all the
		     entries except for the text, cursor, background, etc., in
		     the case of attachments; otherwise we seem to get lots of
		     interference (often fatal) with the waves dynamic colormaps.
		     The right thing to do probably is to have a separate set of
		     colormap functions for the attachments, but this may suffice
		     for now -- js
		   */

		  if (attachment && j >= 3)
		    {
		       /*for attach, skip the middle */
		       rgb[j].red = 0;
		       rgb[j].green = 0;
		       rgb[j].blue = 0;
		    }
		  else
		    {
		       rgb[j].red = r;
		       rgb[j].green = g;
		       rgb[j].blue = b;
		    }
	       }

	     for (j = CMAP_RESERVED; i < MAX_CMAP_SIZE; i++, j++)
	       {
		  if (fgets (line, 120, cm_fd) == NULL)
		     break;
		  sscanf (line, "%d%d%d", &r, &g, &b);
		  rgb[j].red = r;
		  rgb[j].green = g;
		  rgb[j].blue = b;
	       }
	     fclose (cm_fd);
	  }
	else
	  {			/* make up a colormap */
	     int range = (40 * CMAP_RESERVED) / max_reserved, lim = (45 * CMAP_RESERVED) / max_reserved,
	       j;
	     double steps = 255.0 / range, start = 255;

	     if (*def_cm)
	       {
		  sprintf (notice_msg,
		    "Problems accessing specified colormap (%s)\n", def_cm);
		  show_notice (1, notice_msg);

		  *def_cm = 0;
	       }
	     for (i = 0; i < lim; ++i)	/* default gray ramp */
		rgb[i].red = rgb[i].green = rgb[i].blue = 255;
	     for (j = lim + range; i < j; ++i)	/* default gray ramp */
		rgb[i].red = rgb[i].green = rgb[i].blue = (start -= steps);
	     for (; i < CMAP_RESERVED; ++i)	/* default gray ramp */
		rgb[i].red = rgb[i].green = rgb[i].blue = 0;
	  }
	strcpy (cm_file, def_cm);
     }

   if (!def_cm[0])
     {
	if (debug_level)
	   show_notice (0, "cmap: setting some standard colors");

#define SET_COLOR(i, r, g, b) \
	    rgb[i].red =(r); rgb[i].green = (g); rgb[i].blue = (b);

	SET_COLOR (FOREGROUND_COLOR, 0, 0, 0)
	   SET_COLOR (BACKGRND_COLOR, 255, 255, 255)
	   SET_COLOR (RETICLE_COLOR, 200, 0, 255)
	   SET_COLOR (TEXT_COLOR, 0, 140, 180)
	   SET_COLOR (WAVEFORM_COLOR, 0, 0, 0)
	   SET_COLOR (CURSOR_COLOR, 255, 0, 0)
	   SET_COLOR (MARKER_COLOR, 0, 255, 0)
	   SET_COLOR (YA5_COLOR, 8, 149, 255)
	   SET_COLOR (YA4_COLOR, 74, 255, 1)
	   SET_COLOR (YA3_COLOR, 255, 172, 2)
	   SET_COLOR (YA2_COLOR, 255, 1, 1)
	   SET_COLOR (YA1_COLOR, 211, 70, 255)
	   SET_COLOR (WAVE2_COLOR, 0, 200, 100)
	   SET_COLOR (BACKGROUND_COLOR, 255, 255, 255)
	   SET_COLOR (BACK2_COLOR, 220, 255, 220)
#undef SET_COLOR
     }

   if (cms && cm_changed && use_static_cmap)
	show_notice(0,"The colormap change will take effect on new windows only.");

   if (cms == XV_NULL || (cm_changed && use_static_cmap))
     {
	cm_changed = 0;
	cms = XV_NULL;
	if (canvas != XV_NULL)
	   cms = (Cms) xv_create (XV_NULL, CMS,
				  CMS_NAME, cms_name,
				  CMS_TYPE, (use_static_cmap || attachment)
				  ? XV_STATIC_CMS :
				  XV_DYNAMIC_CMS,
				  CMS_SIZE, cmap_size,
				  CMS_COLORS, rgb,
/*
				  XV_VISUAL, (Visual *) xv_get (
				   canvas_paint_window (canvas), XV_VISUAL),
*/
				  XV_VISUAL, visual_ptr,
				  XV_NULL);
     }
   else if (!use_static_cmap && cms)
      xv_set (cms, CMS_COLORS, rgb, XV_NULL);


   /* set map */
   if (canvas != XV_NULL)
     {
	if (debug_level > 1)
	   (void) fprintf (stderr, "cmap: setting colormap\n");

	xv_set (canvas_paint_window (canvas),
		WIN_CMS, cms,
		0);
     }
}

/*********************************************************************/

void
save_shared_colormap (canvas)
     Canvas canvas;
{
   extern Xv_singlecolor rgb[];

   (void) xv_get (canvas_paint_window (canvas), CMS_COLORS, rgb);
   return;
}

/***********************************************************************/
get_color_depth ()
{
   Frame dummy;
   int i;
   extern int cmap_depth, max_cmap_size;

   dummy = xv_create (XV_NULL, FRAME, NULL);
   cmap_depth = (int) xv_get (dummy, WIN_DEPTH);

   if (debug_level)
      fprintf (stderr, "cmap_depth: %d\n", cmap_depth);

   for (cmap_size = 1, i = cmap_depth; i; i--)
      cmap_size *= 2;
   if (max_cmap_size > MAX_CMAP_SIZE)
      max_cmap_size = MAX_CMAP_SIZE;
   if (cmap_size > max_cmap_size)
      cmap_size = max_cmap_size;
   if (max_cmap_size > cmap_size)	/* don't let it fill up all */
      cmap_size /= 2;		/* unless user specifies */

   if (debug_level)
      fprintf (stderr, "Pixel depth:%d  Cmap size:%d Max cmap size:%d\n",
	       cmap_depth, cmap_size, max_cmap_size);
}

/***********************************************************************/
setup_colormap ()
{
   int bw = (do_color) ? 0 : 1;	/* set LSB for monochrome */


   FOREGROUND_COLOR = W_FOREGROUND_COLOR | bw;
   BACKGRND_COLOR = W_BACKGRND_COLOR & ~bw;
   RETICLE_COLOR = W_RETICLE_COLOR | bw;
   TEXT_COLOR = W_TEXT_COLOR | bw;
   WAVEFORM_COLOR = W_WAVEFORM_COLOR | bw;
   CURSOR_COLOR = W_CURSOR_COLOR | bw;
   MARKER_COLOR = W_MARKER_COLOR | bw;
   YA5_COLOR = W_YA5_COLOR | bw;
   YA4_COLOR = W_YA4_COLOR | bw;
   YA3_COLOR = W_YA3_COLOR | bw;
   YA2_COLOR = W_YA2_COLOR | bw;
   YA1_COLOR = W_YA1_COLOR | bw;
   WAVE2_COLOR = W_WAVE2_COLOR | bw;
   CMAP_RESERVED = W_CMAP_RESERVED;
   BACKGROUND_COLOR = W_BACKGROUND_COLOR & ~bw;
   BACK2_COLOR = W_BACK2_COLOR & ~bw;
}


void
identify_visual ()
{
   Display *display;
   int screen;
   XVisualInfo visual_info;
   char *s;

   display = (Display *) xv_get (daddy, XV_DISPLAY);
   screen = DefaultScreen (display);

   if (s = getenv ("WAVES_VISUAL_CLASS"))
     {
	if (isdigit (*s))
	   visual_class = atoi (s);
	else
	  {
	     int i;
	     i = lin_search (VisualNames, s);
	     if (i != -1)
		visual_class = VisualValues[i];
	     else
	       {
		  sprintf (notice_msg, "Unknown visual class %s ",
			   s, "using PseudoColor.\n");
		  show_notice(1,notice_msg);
		  visual_class = PseudoColor;
	       }
	  }
	if (s = getenv ("WAVES_DEPTH"))
	  {
	     cmap_depth = atoi (s);
	  }
	fprintf (stderr, "Trying to use visual %d, depth: %d\n",
		 visual_class, cmap_depth);
	if (!XMatchVisualInfo (display, screen, cmap_depth, visual_class,
			       &visual_info))
	  {
	     sprintf (notice_msg,
	       "Cannot use the requested visual class of %d at depth %d\nContinuing without color. ",
		      visual_class, cmap_depth);
             show_notice(0,notice_msg);
	     cmap_depth = 1;
	     do_color = 0;
	  }
     }
   else if (!XMatchVisualInfo (display, screen, 8, PseudoColor,
			       &visual_info))
     {
	if (debug_level)
	   fprintf (stderr, "No 8 plane pseudocolor\n");
	if (!XMatchVisualInfo (display, screen, cmap_depth, PseudoColor,
			       &visual_info))
	  {
	     if (debug_level)
		fprintf (stderr, "No %d plane pseudocolor\n", cmap_depth);
	     if (!XMatchVisualInfo (display, screen, cmap_depth, TrueColor,
				    &visual_info))
	       {
		  sprintf (notice_msg, "No %d plane TrueColor.\n", cmap_depth);
		  sprintf (notice_msg+strlen(notice_msg), "Continuing without color.\nPlease see the information in the file $ESPS_BASE/README.colormap");
		  show_notice(0,notice_msg);

		  cmap_depth = 1;
		  do_color = 0;
	       }
	  }
     }

   visual_class = visual_info.class;
   visual_id = visual_info.visualid;
   visual_ptr = visual_info.visual;
   cmap_depth = visual_info.depth;

   if (visual_class == TrueColor) {
      use_static_cmap = 1;
      if(debug_level)
	show_notice(0,"Cannot allocate a dynamic colormap.\nPlease see the information in the file $ESPS_BASE/README.colormap.");
   }
}
