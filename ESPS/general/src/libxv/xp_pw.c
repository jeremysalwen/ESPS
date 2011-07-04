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

static char *sccs_id = "@(#)xp_pw.c	1.1	6/21/93	ERL";


#include <xview_private/pw_impl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define _NO_PROTO
#include <Xp.h>
#include <XpMacros.h>

Display *global_display = (Display *)NULL;
Pkg_private     GC Xp_xv_find_proper_gc();
Xv_private int Xp_xv_rop_mpr_internal();
Xv_private void Xp_xv_set_gc_op();

void
set_pw_display(display)
Display *display;
{
	global_display = display;
}

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

    DRAWABLE_INFO_MACRO(window, info);
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
    XDrawLine(display, d, gc, x0, y0, x1, y1);
    XpFreeGC(display, gc);
}

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
    XGCValues	    gcv;

fprintf(stderr,"called Xp_xv_ttext\n");
exit(1);
    if ((len = strlen(str)) == 0) {
	return;
    }
    DRAWABLE_INFO_MACRO(window, info);
    if(global_display == (Display *)NULL) {
       display = xv_display(info);
       d = xv_xid(info);
       gc = Xp_xv_find_proper_gc(display, info, PW_TEXT);
    } else {
       display = global_display;
       d = 0;
       gc = XpCreateGC(display, 0, 0, &gcv);
    }

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

    DRAWABLE_INFO_MACRO(window, info);
    display = global_display;
    d = 0;
    gc = XpCreateGC(display, 0, 0, &gcv);
    Xpxfs = XpLoadQueryFont(display,
            "-adobe-helvetica-medium-r-normal--*-0-300-300-p-*-iso8859-1");

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



    DRAWABLE_INFO_MACRO(window, info);
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
	    xv_error(NULL,
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

	DRAWABLE_INFO_MACRO(src, src_info);
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
		xv_error(NULL,
			 ERROR_STRING,
			     XV_MSG("xv_rop: can't handle drawables of different depth"),
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
		xv_error(NULL,
			 ERROR_STRING,
			     XV_MSG("xv_rop: Windows of different depth, can't rop"),
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

Pkg_private     GC
Xp_xv_find_proper_gc(display, info, op)
    Display        	*display;
    Xv_Drawable_info 	*info;
    int	        	op;
{
    int             depth = xv_depth(info), i;
    Drawable        xid = xv_xid(info);
    XGCValues       gv;
    Xv_Screen       screen = xv_screen(info);
    Xv_xrectlist   *clip_xrects = screen_get_clip_rects(screen);
    short           xv_in_fullscreen = server_get_fullscreen(xv_server(info));
    struct gc_chain   *gcs, *gc_list, **ops_private_gcs;

    if (!GC_CHAIN_KEY)  
	    GC_CHAIN_KEY = xv_unique_key();
    ops_private_gcs = (struct gc_chain **) xv_get( screen, XV_KEY_DATA, GC_CHAIN_KEY );
    if (!ops_private_gcs) {
		ops_private_gcs = (struct gc_chain **) xv_calloc((PW_NUM_OPS+1),sizeof(struct gc_chain));
	    xv_set( screen, XV_KEY_DATA, GC_CHAIN_KEY, ops_private_gcs, 0 );
    }
    gc_list = ops_private_gcs[op];

    /*
     * If a new clipping rectangle has been set for this drawable since the
     * last invocation of this function, set all xid's in the gc list to an
     * invalid xid.
     */
    if (info->new_clipping) {
#ifndef SVR4
	for (i = 0; i < PW_NUM_OPS; i++)
#else SVR4
	for (i = 0; i <= PW_NUM_OPS; i++)
#endif SVR4
	    for (gcs = ops_private_gcs[i]; gcs != NULL; gcs = gcs->next)
		gcs->xid = INVALID_XID;
	info->new_clipping = FALSE;
    }
    if (!gc_list) {
	gc_list = ops_private_gcs[op] =
	    (struct gc_chain *) xv_calloc(1, sizeof(struct gc_chain));
	if (xv_in_fullscreen) {
	    gv.subwindow_mode = IncludeInferiors;
	    gc_list->gc = XCreateGC(display, xid, GCSubwindowMode, &gv);
	} else {
	    gc_list->gc = XCreateGC(display, xid, 0, 0);
	}
	gc_list->clipping_set = FALSE;
	gc_list->depth = depth;
	gc_list->next = NULL;

	/*
	 * Newly created GC. If clipping is enabled on the drawable, set the
	 * GCClipMask for the GC.
	 */
	if (clip_xrects->count) {
	    XSetClipRectangles(display, gc_list->gc, 0, 0,
		     clip_xrects->rect_array, clip_xrects->count, Unsorted);
	    gc_list->clipping_set = TRUE;
	}
	gc_list->xid = xid;
	return (gc_list->gc);
    } else {
	gcs = gc_list;
	while (gcs) {
	    if (gcs->depth == depth) {
		if (xv_in_fullscreen) {
		    gv.subwindow_mode = IncludeInferiors;
		} else {
		    gv.subwindow_mode = ClipByChildren;
		}

		/*
		 * The clipping_set field is redundant. A bug in XChangeGC
		 * (in Xlib) needs to fixed. If the current clip_mask and the
		 * cached clip_mask are both None, the cache need not be
		 * flushed. Remove clipping_set field when this gets fixed.
		 */
		if (gcs->clipping_set && !clip_xrects->count) {
		    gcs->clipping_set = FALSE;
		    gv.clip_mask = None;
		    XChangeGC(display, gcs->gc, GCSubwindowMode | GCClipMask, &gv);
		} else {
		    XChangeGC(display, gcs->gc, GCSubwindowMode, &gv);
		}

		/*
		 * If this is a different drawable since the last invocation
		 * and it has a clipping rectangle enabled, or, the clipping
		 * for the same drawable has changed since the last
		 * invocation, reset the clipping.
		 */
		if (clip_xrects->count && (gcs->xid != xid)) {
		    XSetClipRectangles(display, gcs->gc, 0, 0,
		     clip_xrects->rect_array, clip_xrects->count, Unsorted);
		    gcs->clipping_set = TRUE;
		}
		gcs->xid = xid;
		return (gcs->gc);
	    } else {
		if (gcs->next) {
		    gcs = gcs->next;
		} else {
		    struct gc_chain *new;
		    /*
		     * no gc of the same depth, need to create a new gc
		     */
		    gcs->next = new = (struct gc_chain *) xv_malloc(sizeof(struct gc_chain));
		    if (xv_in_fullscreen) {
			gv.subwindow_mode = IncludeInferiors;
			new->gc = XCreateGC(display, xid, GCSubwindowMode, &gv);
		    } else {
			new->gc = XCreateGC(display, xid, 0, 0);
		    }
		    new->depth = depth;
		    new->next = NULL;

		    /*
		     * Newly created GC. If clipping is enabled on the
		     * window, set the GC.
		     */
		    if (clip_xrects->count) {
			XSetClipRectangles(display, new->gc, 0, 0,
					   clip_xrects->rect_array, clip_xrects->count, Unsorted);
			new->clipping_set = TRUE;
		    }
		    new->xid = xid;
		    return (new->gc);
		}
	    }
	}
    }

}

