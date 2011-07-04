/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
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
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * selected pw (xv) calls with hook for xprinter
 *
 */

static char *sccs_id = "@(#)xp_pw.c	1.14 9/28/98 ERL";

#ifdef DEC_ALPHA
#include <xview/xview.h>
#endif
#include <xview_private/pw_impl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define _NO_PROTO
#include <Xp.h>
#include <XpMacros.h>

#ifdef OS5
#include <libintl.h>
#endif

/***************************************************************************/
Display *global_display = (Display *)NULL;

void
set_pw_display(display)
Display *display;
{
	global_display = display;
}

/***************************************************************************/
#if defined(SUN4) || defined(HP700) || defined(DS3100) || defined(SG) || defined(DEC_ALPHA) || defined(M5600) || defined(LINUX_OR_MAC)
/* this first part is for XView 3 based systems, xview 2 is below */
#include <xview/window.h>
#include <pixrect/pixfont.h>

Xv_private int Xp_xv_rop_mpr_internal();
Xv_private void Xp_xv_set_gc_op();


Xp_xv_vector(window, x0, y0, x1, y1, op, cms_index)
    Xv_opaque       window;
    int             op;
    register int    x0, y0, x1, y1;
    int             cms_index;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XGCValues	    gcv;

    if(global_display == (Display *)NULL) 
/* call real xv_vector function */
       return(xv_vector(window, x0, y0, x1, y1, op, cms_index));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);

    if (op == PIX_NOT(PIX_DST)) {
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_CMS_FG, XV_DEFAULT_FG_BG);
    } else {
	if (!PIX_OPCOLOR(op)) {
	    op |= PIX_COLOR(cms_index);
	}
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG, XV_DEFAULT_FG_BG);
    }
    if(x0 == x1 && y0 == y1)
	XDrawPoint(display, d, gc, x0, y0);
    else
        XDrawLine(display, d, gc, x0, y0, x1, y1);
    XpFreeGC(display, gc);
}


PIXFONT        *xv_pf_sys;

extern          xv_pf_ttext(), xv_pf_text();
extern struct pr_size xv_pf_textwidth();

Xp_xv_ttext(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;
    XGCValues	    gcv;
    XFontStruct     *Xpxfs=NULL;

    if(global_display == (Display *)NULL) 
/* call real xv_ttext function */
       return(xv_ttext(window, xbasew, ybasew, op, pixfont, str));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);
    Xpxfs = XpLoadQueryFont(display,
            "-adobe-times-medium-r-normal--12-120-300-300-p-150-iso8859-1");

    if(Xpxfs)
             XpSetFont(display,gc,Xpxfs->fid);
     else
             fprintf(stderr,"cannot get font for graphic export\n");

    if ((len = strlen(str)) == 0) {
	return;
    }

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    if (PIX_OP(op) == PIX_NOT(PIX_SRC)) {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_INVERTED_FG_BG);
    } else {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }

    XDrawString(display, d, gc, xbasew, ybasew, str, len);
    XpFreeGC(display, gc);

}

Xp_xv_text(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;
    XGCValues	    gcv;
    XFontStruct     *Xpxfs=NULL;

    if(global_display == (Display *)NULL) 
/* call real xv_text function */
       return(xv_text(window, xbasew, ybasew, op, pixfont, str));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);
    Xpxfs = XpLoadQueryFont(display,
            "-adobe-times-medium-r-normal--12-120-300-300-p-150-iso8859-1");

    if(Xpxfs)
             XpSetFont(display,gc,Xpxfs->fid);
     else
             fprintf(stderr,"cannot get font for graphic export\n");

    if ((len = strlen(str)) == 0) {
	return;
    }

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    if (PIX_OP(op) == PIX_NOT(PIX_SRC)) {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_INVERTED_FG_BG);
    } else {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }

    XDrawString(display, d, gc, xbasew, ybasew, str, len);
    XpFreeGC(display, gc);
}

Xv_public
Xp_xv_rop(window, x, y, width, height, op, pr, xr, yr)
    Xv_opaque       window;
    int             op, x, y, width, height;
    Pixrect        *pr;
    int             xr, yr;
{
    register Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC 		    gc;
    XGCValues       gcv;

    if(global_display == (Display *)NULL) 
/* call real xv_rop function */
       return(xv_rop(window, x, y, width, height, op, pr, xr, yr));



    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);

    if (pr == NULL) {
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG,
		     XV_DEFAULT_FG_BG);
	XFillRectangle(display, d, gc, x, y, width, height);
    } else {
	Xp_xv_set_gc_op(display, info, gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);

	if (Xp_xv_rop_internal(display, d, gc, x, y, width, height,
			    (Xv_opaque) pr, xr, yr, info) == XV_ERROR) {
	    xv_error(XV_NULL,
		     ERROR_STRING, 
		     "xv_rop: xv_rop_internal failed",
		     0);
	}
    }
    XpFreeGC(display, gc);
}


#ifdef __STDC__ 
#ifndef CAT
#define CAT(a,b)        a ## b 
#endif 
#endif
#include <pixrect/memvar.h>

#include <xview_private/cms_impl.h>
#include <xview_private/svrim_impl.h>
#include <xview_private/i18n_impl.h>

int   GC_CHAIN_KEY;

#include <xview/xv_xrect.h>

#define INVALID_XID		0

extern Xv_xrectlist *screen_get_clip_rects();

Xv_private void
Xp_xv_set_gc_op(display, info, gc, op, fg_mode, fg_bg)
    Display        *display;
    Xv_Drawable_info *info;
    GC              gc;
    int             op;
    short           fg_mode;
    int             fg_bg;
{
    unsigned long   	value_mask;
    XGCValues       	val;
    Cms_info         	*cms = CMS_PRIVATE(xv_cms(info));

    val.function = XV_TO_XOP(op);
    value_mask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
    val.plane_mask = xv_plane_mask(info);

    if (info->is_bitmap) {		/* restrict bitmap colors to 1 and 0 */
	val.foreground = (fg_bg == XV_DEFAULT_FG_BG) ? 1 : 0;
	val.background = (fg_bg == XV_DEFAULT_FG_BG) ? 0 : 1;
    } else if (fg_mode == XV_USE_OP_FG) {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.background = xv_bg(info);
	} else {
	    val.background = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.foreground = xv_bg(info);
	}
    } else {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	} else {
	    val.background = xv_fg(info);
	    val.foreground = xv_bg(info);
	}
    }

    switch (val.function) {
      case GXclear:
	val.foreground = val.background;
	val.function = GXcopy;
	break;

      case GXset:
	val.foreground = xv_fg(info);
	val.function = GXcopy;
	break;

      case GXxor:
	val.foreground = val.foreground ^ val.background;
	val.background = 0;
	break;

      case GXinvert:
	if (val.foreground == val.background) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	}
	val.plane_mask = val.foreground ^ val.background;
	break;
    }

    XChangeGC(display, gc, value_mask, &val);
}

Xv_private int
Xp_xv_rop_internal(display, d, gc, x, y, width, height, src, xr, yr, dest_info)
    Display        *display;
    Drawable        d;
    GC              gc;
    int             x, y, width, height;
    Xv_opaque       src;
    int             xr, yr;
    Xv_Drawable_info *dest_info;
{
    Xv_Drawable_info *src_info;
    Drawable        src_d;
    XGCValues       changes;
    unsigned long   changes_mask = 0;

    if (width == 0 || height == 0 || !src) {
	return (XV_ERROR);
    }
    /*
     * If src is not a client pixrect, it can either be a window or a
     * server_image.
     */
    if (PR_NOT_MPR(((Pixrect *) src))) {

	DRAWABLE_INFO_MACRO((Xv_opaque)src, src_info);
	src_d = (Drawable) xv_xid(src_info);

	if (PR_IS_SERVER_IMAGE((Pixrect *) src)) {
	    /*
	     * Since src is a server image, avoid the overhead of NoExpose
	     * events by doing stippling/tiling.
	     */
	    changes.ts_x_origin = x;
	    changes.ts_y_origin = y;
	    changes_mask = GCTileStipXOrigin | GCTileStipYOrigin;
	    
	    /* clip to source dimensions */
	    width = (width > ((Pixrect *) src)->pr_size.x) ?
		((Pixrect *) src)->pr_size.x : width;
	    height = (height > ((Pixrect *) src)->pr_size.y) ?
		((Pixrect *) src)->pr_size.y : height;

	    if (xv_depth(dest_info) == xv_depth(src_info)) {
		if (xv_depth(dest_info) == 1) {
		    changes.stipple = xv_xid(src_info);
		    changes.fill_style = FillOpaqueStippled;
		    changes_mask |= GCFillStyle | GCStipple;
		} else {
		    changes.tile = xv_xid(src_info);
		    changes.fill_style = FillTiled;
		    changes_mask |=  GCTile | GCFillStyle;
		}
	    } else if (xv_depth(dest_info) > xv_depth(src_info)) {
		changes.stipple = xv_xid(src_info);
		changes.fill_style = FillOpaqueStippled;
		changes_mask |= GCStipple | GCFillStyle;
	    } else {
		xv_error(XV_NULL,
			 ERROR_STRING,
			     "xv_rop: can't handle drawables of different depth",
			 0);
		return (XV_ERROR);
	    }		
	    if (changes_mask) 
	      XChangeGC(display, gc, changes_mask, &changes);
	    XFillRectangle(display, d, gc, x, y, width, height);
	} else {
	    /* src is a window */
	    if (xv_depth(dest_info) == xv_depth(src_info)) {
		XCopyArea(display, src_d, d, gc, xr, yr, width, height, x, y);
	    } else {
		xv_error(XV_NULL,
			 ERROR_STRING,
			     "xv_rop: Windows of different depth, can't rop",
			 0);
		return (XV_ERROR);
	    }
	}
    } else {
	if (Xp_xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, 
		xr, yr, dest_info, TRUE) == XV_ERROR)
	    return(XV_ERROR);
    }

    return(XV_OK);
}	


Xv_private int
Xp_xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, xr, yr, 
	dest_info, mpr_bits)
    Display        	*display;
    Drawable        	d;
    GC              	gc;
    int             	x, y, width, height;
    Xv_opaque       	src;
    int             	xr, yr;
    Xv_Drawable_info 	*dest_info;
    short		mpr_bits;
{
    int             		 src_depth;
    XImage         		*ximage;
    Cms_info			*cms = CMS_PRIVATE(xv_cms(dest_info));
    static unsigned char	*data = (unsigned char *)NULL;
    static unsigned int		 last_size = 0;

    src_depth = ((Pixrect *) src)->pr_depth;
    /* 
     * In Sunview, this case is handled by setting all non-zero color values 
     * to 1's. This is currently a NO-OP in XView. This case must be 
     * handled by creating a separate array of data bits of setting non-zero 
     * pixel values to 1's.
     */
    if ((xv_depth(dest_info) == 1) && (src_depth > 1)) {
	return(XV_ERROR);
    }

    if (src_depth == 1) {
	if (!(ximage = xv_image_bitmap(dest_info))) {
	    Screen_visual     *visual;

	    visual = (Screen_visual *)xv_get(xv_screen(dest_info), SCREEN_DEFAULT_VISUAL);
	    xv_image_bitmap(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual->vinfo->visual,
				1, XYBitmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image, 
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else if ((src_depth == 8) && (xv_depth(dest_info) == 8)) {
        if (!(ximage = xv_image_pixmap(dest_info))) {
	    Screen_visual *visual;
	    
	    visual = (Screen_visual *)xv_get(xv_screen(dest_info), SCREEN_DEFAULT_VISUAL);
	    xv_image_pixmap(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual->vinfo->visual,
				8, ZPixmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image,
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else {
	return (XV_ERROR);
    }
					
    ximage->bitmap_unit = MPR_LINEBITPAD;
    ximage->bitmap_pad = MPR_LINEBITPAD;
    ximage->height = ((Pixrect *) src)->pr_height;
    ximage->width = ((Pixrect *) src)->pr_width;
    ximage->bytes_per_line = mpr_d(((Pixrect *) src))->md_linebytes;
    ximage->data = (char *) mpr_d(((Pixrect *) src))->md_image;

    /* 
     * The bitmap data being passed in might be in either of 2 formats:
     *    1. memory pixrect format.
     *    2. Xlib bitmap format.
     */
    if (mpr_bits == TRUE) {
    /* bitmap data is in memory pixrect format */
#ifdef i386
        ximage->byte_order = LSBFirst;
        /*
         * Check to see if the pixrect data was set by mpr_static(), or by
         * actually creating the pixrect with mem_create() and drawing into
         * it.
         */
        if (mpr_d((Pixrect *) src)->md_flags & MP_I386)
	    ximage->bitmap_bit_order = LSBFirst;
        else
	    ximage->bitmap_bit_order = MSBFirst;
#else
#ifdef ultrix
        ximage->byte_order = LSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#else
        ximage->byte_order = MSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#endif				/* ~VAX */
#endif				/* ~i386 */
    } else {
    /* bitmap data is in Xlib bitmap format */
	ximage->byte_order = LSBFirst;
	ximage->bitmap_bit_order = LSBFirst;
        if (src_depth == 1) 
          ximage->bytes_per_line = (width + 7) >> 3;
    }

    if (src_depth == 1) {
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
	    MIN(width, ximage->width), MIN(height, ximage->height));
    } else {
	register int     i, j;
	unsigned long    index;
	unsigned int	 size;
	char		*image_data;
	
	/*
	 * Create any space needed to convert the image data to pixel values
	 */
	size = ximage->height * ximage->bytes_per_line;
	if (size > last_size) {
	    if (data)
	      xv_free(data);
	    data = (unsigned char *)xv_malloc(size);
	    last_size = size;
	}
	
	/* 
	 * convert image from cms indices to X pixel values.
	 */
	image_data = ximage->data;
	for (i = 0; i < ximage->height; i++) {
	    for (j = 0; j < ximage->bytes_per_line; j++) {
		index = j + i * ximage->bytes_per_line;
		data[index] = cms->index_table[(unsigned char)image_data[index]];
	    }
	}
	
	ximage->data = (char *)data;
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
		  MIN(width, ximage->width), MIN(height, ximage->height));
	ximage->data = image_data;
    }
    return (XV_OK);
}


#else	/* xview 2 */

Xv_private int Xp_xv_rop_mpr_internal();
Xv_private void Xp_xv_set_gc_op();

/*
 * Pw_vector.c: Implement the pw_vector functions of the pixwin.h interface.
 */


Xp_xv_vector(window, x0, y0, x1, y1, op, cms_index)
    Xv_opaque       window;
    int             op;
    register int    x0, y0, x1, y1;
    int             cms_index;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XGCValues	    gcv;

    if(global_display == (Display *)NULL) 
/* call real xv_vector function */
       return(xv_vector(window, x0, y0, x1, y1, op, cms_index));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);

    if (op == PIX_NOT(PIX_DST)) {
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_CMS_FG, XV_DEFAULT_FG_BG);
    } else {
	if (!PIX_OPCOLOR(op)) {
	    op |= PIX_COLOR(cms_index);
	}
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG, XV_DEFAULT_FG_BG);
    }
    if(x0 == x1 && y0 == y1)
	XDrawPoint(display, d, gc, x0, y0);
    else
        XDrawLine(display, d, gc, x0, y0, x1, y1);
    XpFreeGC(display, gc);
}

/*
 * Xv_text.c: Implements pw_char/text functions of the pixwin.h interface for
 * X server.
 */
#include <xview/window.h>
#include <pixrect/pixfont.h>

PIXFONT        *xv_pf_sys;

extern          xv_pf_ttext(), xv_pf_text();
extern struct pr_size xv_pf_textwidth();


Xp_xv_ttext(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;

    if ((len = strlen(str)) == 0) {
	return;
    }
fprintf(stderr,"called Xp_xv_ttext\n");
exit(1);
    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = xv_display(info);
    d = xv_xid(info);

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    /*
     * Since this is transparent text, we always paint it with the background
     * color.
     */
    Xp_xv_set_gc_op(display, info, gc, op, XV_USE_CMS_FG,
		 XV_INVERTED_FG_BG);
    font = (XID) xv_get(pixfont, XV_XID);
    XSetFont(display, gc, font);
    XDrawString(display, d, gc, xbasew, ybasew, str, len);
}

Xp_xv_text(window, xbasew, ybasew, op, pixfont, str)
    Xv_opaque       window;
    int             op;
    register int    xbasew, ybasew;
    Xv_opaque       pixfont;
    char           *str;
{
    Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    GC              gc;
    XID             font;
    int             len;
    XGCValues	    gcv;
    XFontStruct     *Xpxfs=NULL;

    if(global_display == (Display *)NULL)
/* call real xv_text function */
       return(xv_text(window, xbasew, ybasew, op, pixfont, str));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);
    Xpxfs = XpLoadQueryFont(display,
            "-adobe-times-medium-r-normal--12-120-300-300-p-150-iso8859-1");
     if(Xpxfs)
             XpSetFont(display,gc,Xpxfs->fid);
     else
             fprintf(stderr,"cannot get font for graphic export\n");


    if ((len = strlen(str)) == 0) {
	return;
    }

    /* SunView1.X incompatibility: NULL pixfont meant use system_font. */
    if (pixfont == 0) {
	pixfont = xv_get(window, WIN_FONT);
    }
    if (PIX_OP(op) == PIX_NOT(PIX_SRC)) {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_INVERTED_FG_BG);
    } else {
	Xp_xv_set_gc_op(display, info, gc, op, 
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);
    }
    XDrawString(display, d, gc, xbasew, ybasew, str, len);
    XpFreeGC(display, gc);
}

/*
 * xv_rop.c: Implements pw_write functions of the pixwin.h interface for X
 * server.
 */

Xv_public
Xp_xv_rop(window, x, y, width, height, op, pr, xr, yr)
    Xv_opaque       window;
    int             op, x, y, width, height;
    Pixrect        *pr;
    int             xr, yr;
{
    register Xv_Drawable_info *info;
    Display        *display;
    Drawable        d;
    XGCValues	    gcv;
    GC 		    gc;

    if(global_display == (Display *)NULL)
/* call real xv_rop function */
       return(xv_rop(window, x, y, width, height, op, pr, xr, yr));

    DRAWABLE_INFO_MACRO((Xv_opaque)window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);

    if (pr == NULL) {
	Xp_xv_set_gc_op(display, info, gc, op, XV_USE_OP_FG,
		     XV_DEFAULT_FG_BG);
	XFillRectangle(display, d, gc, x, y, width, height);
    } else {
	Xp_xv_set_gc_op(display, info, gc, op,
		     PIX_OPCOLOR(op) ? XV_USE_OP_FG : XV_USE_CMS_FG,
		     XV_DEFAULT_FG_BG);

	if (Xp_xv_rop_internal(display, d, gc, x, y, width, height,
			    (Xv_opaque) pr, xr, yr, info) == XV_ERROR) {
	    xv_error(XV_NULL,
		     ERROR_STRING, "xv_rop: xv_rop_internal failed",
		     0);
	}
    }
    XpFreeGC(display, gc);
}


#include <pixrect/memvar.h>
#include <xview_private/cms_impl.h>
#include <xview_private/svrim_impl.h>

int   GC_CHAIN_KEY;

#include <xview/xv_xrect.h>

#define INVALID_XID		0

extern Screen_visual *screen_add_visual();
extern Xv_xrectlist *screen_get_clip_rects();


Xv_private void
Xp_xv_set_gc_op(display, info, gc, op, fg_mode, fg_bg)
    Display        *display;
    Xv_Drawable_info *info;
    GC              gc;
    int             op;
    short           fg_mode;
    int             fg_bg;
{
    unsigned long   	value_mask;
    XGCValues       	val;
    Cms_info         	*cms = CMS_PRIVATE(xv_cms(info));

    val.function = XV_TO_XOP(op);
    value_mask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
    val.plane_mask = xv_plane_mask(info);

    if (info->is_bitmap) {		/* restrict bitmap colors to 1 and 0 */
	val.foreground = (fg_bg == XV_DEFAULT_FG_BG) ? 1 : 0;
	val.background = (fg_bg == XV_DEFAULT_FG_BG) ? 0 : 1;
    } else if (fg_mode == XV_USE_OP_FG) {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.background = xv_bg(info);
	} else {
	    val.background = XV_TO_X_PIXEL(PIX_OPCOLOR(op), cms);
	    val.foreground = xv_bg(info);
	}
    } else {
	if (fg_bg == XV_DEFAULT_FG_BG) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	} else {
	    val.background = xv_fg(info);
	    val.foreground = xv_bg(info);
	}
    }

    switch (val.function) {
      case GXclear:
	val.foreground = val.background;
	val.function = GXcopy;
	break;

      case GXset:
	val.foreground = xv_fg(info);
	val.function = GXcopy;
	break;

      case GXxor:
	val.foreground = val.foreground ^ val.background;
	val.background = 0;
	break;

      case GXinvert:
	if (val.foreground == val.background) {
	    val.foreground = xv_fg(info);
	    val.background = xv_bg(info);
	}
	val.plane_mask = val.foreground ^ val.background;
	break;
    }

    XChangeGC(display, gc, value_mask, &val);
}

Xv_private int
Xp_xv_rop_internal(display, d, gc, x, y, width, height, src, xr, yr, dest_info)
    Display        *display;
    Drawable        d;
    GC              gc;
    int             x, y, width, height;
    Xv_opaque       src;
    int             xr, yr;
    Xv_Drawable_info *dest_info;
{
    Xv_Drawable_info *src_info;
    Drawable        src_d;
    XGCValues       changes;

    if (width == 0 || height == 0 || !src) {
	return (XV_ERROR);
    }
    /*
     * If src is not a client pixrect, it can either be a window or a
     * server_image.
     */
    if (PR_NOT_MPR(((Pixrect *) src))) {

	DRAWABLE_INFO_MACRO((Xv_opaque)src, src_info);
	src_d = (Drawable) xv_xid(src_info);

	if (PR_IS_SERVER_IMAGE((Pixrect *) src)) {
	    /*
	     * Since src is a server image, avoid the overhead of NoExpose
	     * events by doing stippling/tiling.
	     */
	    changes.ts_x_origin = x;
	    changes.ts_y_origin = y;

	    /* clip to source dimensions */
	    width = (width > ((Pixrect *) src)->pr_size.x) ?
		((Pixrect *) src)->pr_size.x : width;
	    height = (height > ((Pixrect *) src)->pr_size.y) ?
		((Pixrect *) src)->pr_size.y : height;

	    switch (xv_depth_relation(xv_depth(dest_info), xv_depth(src_info))) {
	      case DEST1SRC1:
		/*
		 * reset foreground/background since fg/bg on color need not
		 * be 1 or 0.
		 */
		changes.foreground = 1;
		changes.background = 0;
		changes.stipple = xv_xid(src_info);
		changes.fill_style = FillOpaqueStippled;
		XChangeGC(display, gc, GCForeground | GCBackground |
		       GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin |
			  GCStipple, &changes);
		XFillRectangle(display, d, gc, x, y, width, height);
		break;
#ifndef SVR4
	      case DEST8SRC1:
#else SVR4
	      case DESTNSRC1:
#endif SVR4
		changes.stipple = xv_xid(src_info);
		changes.fill_style = FillOpaqueStippled;
		XChangeGC(display, gc, GCFillStyle | GCTileStipXOrigin |
			  GCTileStipYOrigin | GCStipple, &changes);
		XFillRectangle(display, d, gc, x, y, width, height);
		break;
#ifndef SVR4
	      case DEST8SRC8:
#endif SVR4
	      case DESTEQUALSRC:
		changes.tile = xv_xid(src_info);
		changes.fill_style = FillTiled;
		XChangeGC(display, gc, GCFillStyle | GCTileStipXOrigin |
			  GCTileStipYOrigin | GCTile, &changes);
		XFillRectangle(display, d, gc, x, y, width, height);
		break;
	      default:
		xv_error(XV_NULL,
			 ERROR_STRING,
			   "xv_rop: can't handle drawables of different depth",
			 0);
		return (XV_ERROR);
	    }
	} else {
	    /* src is a window */
	    switch (xv_depth_relation(xv_depth(dest_info), xv_depth(src_info))) {
	      case DEST1SRC1:
	      case DEST8SRC8:
	      case DESTEQUALSRC:
		XCopyArea(display, src_d, d, gc, xr, yr, width, height, x, y);
		break;
	      default:
		xv_error(XV_NULL,
			 ERROR_STRING,
			     "xv_rop: Windows of different depth, can't rop",
			 0);
		return (XV_ERROR);
	    }
	}
    } else {
	if (Xp_xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, 
		xr, yr, dest_info, TRUE) == XV_ERROR)
	    return(XV_ERROR);
    }

    return(XV_OK);
}	


Xv_private int
Xp_xv_rop_mpr_internal(display, d, gc, x, y, width, height, src, xr, yr, 
	dest_info, mpr_bits)
    Display        	*display;
    Drawable        	d;
    GC              	gc;
    int             	x, y, width, height;
    Xv_opaque       	src;
    int             	xr, yr;
    Xv_Drawable_info 	*dest_info;
    short		mpr_bits;
{
    int             	src_depth;
    XImage         	*ximage;
    Cms_info		*cms = CMS_PRIVATE(xv_cms(dest_info));
    static char         *inverse_table = (char *)NULL;

    src_depth = ((Pixrect *) src)->pr_depth;
    /* 
     * In Sunview, this case is handled by setting all non-zero color values 
     * to 1's. This is currently a NO-OP in XView. This case must be 
     * handled by creating a separate array of data bits of setting non-zero 
     * pixel values to 1's.
     */
    if ((xv_depth(dest_info) == 1) && (src_depth > 1)) {
	return(XV_ERROR);
    }

    if (src_depth == 1) {
	if (!(ximage = xv_image_1(dest_info))) {
	    Xv_opaque       visual;

	    visual = xv_get(xv_screen(dest_info), SCREEN_STATIC_VISUAL);
	    xv_image_1(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual,
				1, XYBitmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image, 
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else if ((src_depth == 8) && (xv_depth(dest_info) == 8)) {
	if (!(ximage = xv_image_8(dest_info))) {
	    Xv_opaque       visual;

	    visual = xv_get(xv_screen(dest_info), SCREEN_STATIC_VISUAL);
	    xv_image_8(dest_info) = ximage = 
		(XImage *) XCreateImage(display, visual,
				8, ZPixmap, 0,
				(char *) mpr_d(((Pixrect *) src))->md_image,
				0, 0, MPR_LINEBITPAD,
				mpr_d(((Pixrect *) src))->md_linebytes);
	    if (!ximage) {
		return (XV_ERROR);
	    }
	}
    } else {
	return (XV_ERROR);
    }
					
    ximage->bitmap_unit = MPR_LINEBITPAD;
    ximage->bitmap_pad = MPR_LINEBITPAD;
    ximage->height = ((Pixrect *) src)->pr_height;
    ximage->width = ((Pixrect *) src)->pr_width;
    ximage->bytes_per_line = mpr_d(((Pixrect *) src))->md_linebytes;
    ximage->data = (char *) mpr_d(((Pixrect *) src))->md_image;

    /* 
     * The bitmap data being passed in might be in either of 2 formats:
     *    1. memory pixrect format.
     *    2. Xlib bitmap format.
     */
    if (mpr_bits == TRUE) {
    /* bitmap data is in memory pixrect format */
#ifdef i386
        ximage->byte_order = LSBFirst;
        /*
         * Check to see if the pixrect data was set by mpr_static(), or by
         * actually creating the pixrect with mem_create() and drawing into
         * it.
         */
        if (mpr_d((Pixrect *) src)->md_flags & MP_I386)
	    ximage->bitmap_bit_order = LSBFirst;
        else
	    ximage->bitmap_bit_order = MSBFirst;
#else
#ifdef ultrix
        ximage->byte_order = LSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#else
        ximage->byte_order = MSBFirst;
        ximage->bitmap_bit_order = MSBFirst;
#endif				/* ~VAX */
#endif				/* ~i386 */
    } else {
    /* bitmap data is in Xlib bitmap format */
	ximage->byte_order = LSBFirst;
	ximage->bitmap_bit_order = LSBFirst;
    }

    if (src_depth == 1) {
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
	    MIN(width, ximage->width), MIN(height, ximage->height));
    } else {
	register int    i, j;
	unsigned long   index;
	
	/*
	 * build inverse pixel-to-cms_index table
	 */
	if (inverse_table == (char *)NULL)
	  /* malloc only on the first call to this function */
	  inverse_table = (char *)malloc(256 * sizeof(char));
	for (i = 0; i < cms->size; i++)
	  inverse_table[cms->index_table[i]] = i;
	
	/* 
	 * convert image from cms indices to X pixel values.
	 */
	for (i = 0; i < ximage->height; i++) {
	    for (j = 0; j < ximage->bytes_per_line; j++) {
		index = j + i * ximage->bytes_per_line;
		ximage->data[index] = cms->index_table[ximage->data[index]];
	    }
	}
	
	XPutImage(display, d, gc, ximage, xr, yr, x, y,
		  MIN(width, ximage->width), MIN(height, ximage->height));
	
	/*
	 * convert it back to cms indices
	 */
	for (i = 0; i < ximage->height; i++) {
	    for (j = 0; j < ximage->bytes_per_line; j++) {
		index = j + i * ximage->bytes_per_line;
		ximage->data[index] = inverse_table[ximage->data[index]];
	    }
	}
    }
    return (XV_OK);
}


#ifdef NEEDTHIS
Pkg_private
xv_depth_relation(d_depth, s_depth)
    int             d_depth, s_depth;
{
#ifndef SVR4
    if (d_depth == s_depth) {
	if (d_depth == 1) {
	    return DEST1SRC1;
	} else if (d_depth == 8) {
	    return DEST8SRC8;
	} else {
	    return DESTEQUALSRC;
	}
    } else if (d_depth == 8 && s_depth == 1) {
	return DEST8SRC1;
#else SVR4
    if (d_depth == s_depth) {
	if (d_depth == 1) {
	    return DEST1SRC1;
	} else {
	    return DESTEQUALSRC;
	}
    } else if (d_depth != 1 && s_depth == 1) {
	return DESTNSRC1;
#endif SVR4
    } else {
	return OTHER_DEPTHS;
    }
}
#endif

#endif  /* xview 2 */
