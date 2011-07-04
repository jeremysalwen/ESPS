/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: xecolors.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * pre-defined single colors for XView programs
 */

#ifndef lint
static char *sccs_id = "%W%	%G%	ESI";
#endif

#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/cms.h>

#include <esps/exview.h>

#if !defined(hpux) && !defined(SG) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *calloc();
#endif

static Xv_singlecolor ecolors[] = {
        { 255, 255, 255 }, /* white */
        { 255,   0,   0 }, /* red */
        { 0,   255,   0 }, /* green */
	{ 0,     0, 255 }, /* blue */
	{255,  255,   0 }, /* yellow */
	{188,  143, 143 }, /* brown */
	{220, 220,  220 }, /* gray */
	{  0,   0,    0 }, /* black */
	{250, 130,   80 }, /* orange */
	{ 30, 230,  250 }, /* acqua */
	{230,  30,  250 }, /* pink */
	{216, 216,  191 }, /* wheat */
	{142,  35,   35 }, /* firebrick */
	{219, 147,   12 }, /* tan */
	{143, 188,  143 }, /* pale green */
	{255, 238,  204 }, /* beige */
	{255, 255,  204 }, /* light yellow */
    };

Cms
exv_create_cms()
{

  Cms cms;

  cms = (Cms) xv_create(NULL, CMS, 
			CMS_NAME,      exv_cms_name(),
			CMS_CONTROL_CMS, TRUE,
			CMS_TYPE,     XV_STATIC_CMS,
			CMS_SIZE,     CMS_CONTROL_COLORS + NUM_COLORS,
			CMS_COLORS,   ecolors,
			NULL);
  return(cms);
}



/*
 * Initialize the ESPS colormap segment data.
 */
Cms
exv_init_colors(win)
	Xv_opaque       win;			/* base window */
{
	Cms		cms;
	Xv_Screen	screen = (Xv_Screen) XV_SCREEN_FROM_WINDOW(win);

	cms = (Cms) xv_find(screen, CMS,
		CMS_NAME,	exv_cms_name(),
		XV_AUTO_CREATE,	FALSE,
		0);

	if(!cms)
		cms = exv_create_cms();

	xv_set(win,
		WIN_CMS_NAME,	exv_cms_name(),
		WIN_CMS,	cms,
		0);
	
	return(cms);
}


/*
 * return name of Entropic colormap
 */

char	       *
exv_cms_name()
{
	return "ESPS CMS";
}

int
exv_color_status(frame)
Frame frame;
{
  Display *dpy;

  dpy = (Display *) xv_get(frame, XV_DISPLAY, NULL);
  if (DisplayPlanes(dpy, DefaultScreen(dpy)) == 1)
    return 0;
  else
    return 1;
}

