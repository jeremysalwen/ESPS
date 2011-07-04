/* window.c:
 *
 * display an image in a window
 *
 * jim frost 10.03.89
 *
 * Copyright 1989, 1990 Jim Frost.  See included file "copyright.h" for
 * complete copyright information.
 */

/*
 * Additions to support  -prog_slideshow  option (advance to next slide
 * under external program control) added by Rod Johnson, Entropic
 * Research Laboratory, Inc., 90/09/25.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)window.c	1.4  11/17/90";
#endif

#include "copyright.h"
#include "xloadimage.h"
#include <ctype.h>
#include <X11/cursorfont.h>
#include "ready.h"

extern void	set_ready();
extern int	is_ready(), get_ready();

static Window ImageWindow= 0;

static void setCursor(disp, window, iw, ih, ww, wh, cursor)
     Display      *disp;
     Window        window;
     unsigned int  iw, ih;
     unsigned int  ww, wh;
     Cursor       *cursor;
{ XSetWindowAttributes swa;

  if ((ww >= iw) && (wh >= ih))
    swa.cursor= XCreateFontCursor(disp, XC_icon);
  else if ((ww < iw) && (wh >= ih))
    swa.cursor= XCreateFontCursor(disp, XC_sb_h_double_arrow);
  else if ((ww >= iw) && (wh < ih))
    swa.cursor= XCreateFontCursor(disp, XC_sb_v_double_arrow);
  else
    swa.cursor= XCreateFontCursor(disp, XC_fleur);
  XChangeWindowAttributes(disp, window, CWCursor, &swa);
  XFreeCursor(disp, *cursor);
  *cursor= swa.cursor;
}

/* place an image
 */

static void placeImage(width, height, winwidth, winheight, rx, ry)
     int width, height, winwidth, winheight;
     int *rx, *ry; /* supplied and returned */
{ int pixx, pixy;

  pixx= *rx;
  pixy= *ry;

  if (winwidth > width)
    pixx= (winwidth - width) / 2;
  else {
    if ((pixx < 0) && (pixx + width < winwidth))
      pixx= winwidth - width;
    if (pixx > 0)
      pixx= 0;
  }
  if (winheight > height)
    pixy= (winheight - height) / 2;
  else {
    if ((pixy < 0) && (pixy + height < winheight))
      pixy= winheight - height;
    if (pixy > 0)
      pixy= 0;
  }
  *rx= pixx;
  *ry= pixy;
}

/* blit an image
 */

static void blitImage(disp, pixmap, window, gc, pixx, pixy, width, height,
		      winwidth, winheight, x, y, w, h)
     Display *disp;
     Pixmap   pixmap;
     Window   window;
     GC       gc;
     int      pixx, pixy, width, height, winwidth, winheight, x, y, w, h;
{
  if (x + w > winwidth)
    w= winwidth - x;
  if (y + h > winheight)
    h= winheight - y;
  if (x < pixx) {
    XClearArea(disp, window, x, y, pixx - x, y + h, False);
    w -= (pixx - x);
    x= pixx;
  }
  if (y < pixy) {
    XClearArea(disp, window, x, y, w, pixy - y, False);
    h -= (pixy - y);
    y= pixy;
  }
  if (x + w > pixx + width) {
    XClearArea(disp, window, pixx + width, y, w - width, h, False);
    w= width;
  }
  if (y + h > pixy + height) {
    XClearArea(disp, window, x, pixy + height, w, h - height, False);
    h= height;
  }
  XCopyArea(disp, pixmap, ImageWindow, gc, x - pixx, y - pixy, w, h,
	    x, y);
}

/* clean up static window if we're through with it
 */

void cleanUpWindow(disp)
     Display *disp;
{
  if (ImageWindow)
    XDestroyWindow(disp, ImageWindow);
  ImageWindow= 0;
}

char imageInWindow(disp, scrn, image, winx, winy, winwidth, winheight,
		   fullscreen, install, slideshow, prog_slideshow, verbose)
     Display      *disp;
     int           scrn;
     Image        *image;
     unsigned int  winx, winy, winwidth, winheight;
     unsigned int  fullscreen;
     unsigned int  install;
     unsigned int  slideshow, prog_slideshow;
     unsigned int  verbose;
{ Pixmap               pixmap;
  Colormap             xcmap;
  XSetWindowAttributes swa;
  XSizeHints           sh;
  XWMHints             wmh;
  XGCValues            gcv;
  GC                   gc;
  int                  pixx, pixy;
  int                  lastx, lasty, mousex, mousey;
  int                  paint;
  union {
    XEvent              event;
    XAnyEvent           any;
    XButtonEvent        button;
    XKeyEvent           key;
    XConfigureEvent     configure;
    XExposeEvent        expose;
    XMotionEvent        motion;
    XResizeRequestEvent resize;
  } event;
  static int		first = 1;

  /* figure out the window size.  unless specifically requested to do so,
   * we will not exceed 90% of display real estate.
   */

  if (fullscreen) {
    winwidth= DisplayWidth(disp, scrn);
    winheight= DisplayHeight(disp, scrn);
  }
  else {
    lastx= (winwidth || winheight); /* user set size flag */
    if (!winwidth) {
      winwidth= image->width;
      if (winwidth > DisplayWidth(disp, scrn) * 0.9)
	winwidth= DisplayWidth(disp, scrn) * 0.9;
    }
    if (!winheight) {
      winheight= image->height;
      if (winheight > DisplayHeight(disp, scrn) * 0.9)
	winheight= DisplayHeight(disp, scrn) * 0.9;
    }
  }

  if (! sendImageToX(disp, scrn, DefaultVisual(disp, scrn),
		     image, &pixmap, &xcmap, verbose))
    exit(1);
  swa.background_pixel= 0;
  swa.backing_store= NotUseful;
  swa.bit_gravity= NorthWestGravity;
  swa.cursor= XCreateFontCursor(disp, XC_watch);
  swa.colormap= xcmap;
  swa.event_mask= ButtonPressMask | Button1MotionMask | KeyPressMask |
    ExposureMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask;
  swa.save_under= False;
  swa.override_redirect= (fullscreen ? True : False);
  if (!ImageWindow) {
    static XClassHint classHint;
    ImageWindow= XCreateWindow(disp, RootWindow(disp, scrn), 0, 0,
			       winwidth, winheight, 0,
			       DefaultDepth(disp, scrn),
			       InputOutput, CopyFromParent,
			       CWBackPixel | CWBackingStore |
			       CWBitGravity | CWCursor | CWColormap |
			       CWEventMask | CWSaveUnder, &swa);
    classHint.res_class = "XLoadImage";
    classHint.res_name=NULL;
    (void) XSetClassHint(disp,ImageWindow,&classHint);
    paint= 0;
  }
  else {
    XResizeWindow(disp, ImageWindow, winwidth, winheight);
    XChangeWindowAttributes(disp, ImageWindow, CWColormap, &swa);
    paint= 1;
  }
  XStoreName(disp, ImageWindow, image->title);
  XSetIconName(disp, ImageWindow, image->title);

  sh.width= winwidth;
  sh.height= winheight;
  if (fullscreen) {
    sh.min_width= sh.max_width= winwidth;
    sh.min_height= sh.max_height= winheight;
  }
  else {
    sh.min_width= 1;
    sh.min_height= 1;
    sh.max_width= image->width;
    sh.max_height= image->height;
  }
  sh.width_inc= 1;
  sh.height_inc= 1;
  sh.flags= PMinSize | PMaxSize | PResizeInc;
  if (lastx || fullscreen)
    sh.flags |= USSize;
  else
    sh.flags |= PSize;
  if (fullscreen) {
    sh.x= sh.y= 0;
    sh.flags |= USPosition;
  }
  else if (winx || winy) {
    sh.x= winx;
    sh.y= winy;
    sh.flags |= USPosition;
  }
  XSetNormalHints(disp, ImageWindow, &sh);

  wmh.input= True;
  wmh.flags= InputHint;
  XSetWMHints(disp, ImageWindow, &wmh);

  gcv.function= GXcopy;
  gcv.foreground= 0;
  gc= XCreateGC(disp, ImageWindow, GCFunction | GCForeground, &gcv);
  XMapWindow(disp, ImageWindow);
  placeImage(image->width, image->height, winwidth, winheight, &pixx, &pixy);
  if (paint)
    blitImage(disp, pixmap, ImageWindow, gc,
	      pixx, pixy, image->width, image->height, winwidth, winheight,
	      0, 0, winwidth, winheight);
  setCursor(disp, ImageWindow, image->width, image->height,
	    winwidth, winheight, &(swa.cursor));

  if (prog_slideshow && !first)		/* If first slide, wait for initial */
    set_ready(disp, scrn, RDY_READY);	/* Expose event (see below).        */

  lastx= lasty= -1;
  for (;;) {
    XNextEvent(disp, &event);
    switch (event.any.type) {
    case ButtonPress:
      if (event.button.button == 1) {
	lastx= event.button.x;
	lasty= event.button.y;
	break;
      }
      break;

    case PropertyNotify:
    case KeyPress: {
      char buf[128];
      KeySym ks;
      XComposeStatus status;
      char ret;
      Cursor cursor;

      if (event.any.type == PropertyNotify)
      {
	  if (prog_slideshow && is_ready(disp, &event.event)
				&& get_ready(disp, scrn) == RDY_BUSY)
	      ret = 'n';
	  else
	      break;
      }
      else	/* KeyPress */
      {
	  XLookupString(&event.key,buf,128,&ks,&status);
	  ret= buf[0];
	  if (isupper(ret))
	      ret= tolower(ret);

	  if (prog_slideshow)
	      if (ret == 'q' || ret == '\003')
		  set_ready(disp, scrn, RDY_QUIT);
	      else
	      {
		  if (ret == 'h') /* "hold" */
		      set_ready(disp, scrn, RDY_PAUSE);
		  else
		  if (ret == 'c') /* "continue" */
		      set_ready(disp, scrn, RDY_READY);

		  break;
	      }
      }

      switch (ret) {
      case 'n':
      case 'p':
	cursor= swa.cursor;
	swa.cursor= XCreateFontCursor(disp, XC_watch);
	XChangeWindowAttributes(disp, ImageWindow, CWCursor, &swa);
	XFreeCursor(disp, cursor);
	XFlush(disp);
	/* FALLTHRU */
      case '\003': /* ^C */
      case 'q':
	XFreeCursor(disp, swa.cursor);
	XFreePixmap(disp, pixmap);
	if (xcmap != DefaultColormap(disp, scrn))
          XFreeColormap(disp, xcmap);
	return(ret);
      }
      break;
    }

    case MotionNotify:
      if ((image->width <= winwidth) && (image->height <= winheight))
	break; /* we're AT&T */
      mousex= event.button.x;
      mousey= event.button.y;
      while (XCheckTypedEvent(disp, MotionNotify, &event) == True) {
	mousex= event.button.x;
	mousey= event.button.y;
      }
      pixx -= (lastx - mousex);
      pixy -= (lasty - mousey);
      lastx= mousex;
      lasty= mousey;
      placeImage(image->width, image->height, winwidth, winheight,
		 &pixx, &pixy);
      blitImage(disp, pixmap, ImageWindow, gc,
		pixx, pixy, image->width, image->height, winwidth, winheight,
		0, 0, winwidth, winheight);
      break;

    case ConfigureNotify:
      winwidth= event.configure.width;
      winheight= event.configure.height;

      placeImage(image->width, image->height, winwidth, winheight,
		 &pixx, &pixy);

      /* configure the cursor to indicate which directions we can drag
       */

      setCursor(disp, ImageWindow, image->width, image->height,
		winwidth, winheight, &(swa.cursor));

      /* repaint 
       */

      blitImage(disp, pixmap, ImageWindow, gc,
		pixx, pixy, image->width, image->height, winwidth, winheight,
		0, 0, winwidth, winheight);
      break;

    case DestroyNotify:
      XFreeCursor(disp, swa.cursor);
      XFreePixmap(disp, pixmap);
      if (xcmap != DefaultColormap(disp, scrn))
	XFreeColormap(disp, xcmap);
      set_ready(disp, scrn, RDY_BUSY);
      return('\0');

    case Expose:
      blitImage(disp, pixmap, ImageWindow, gc,
		pixx, pixy, image->width, image->height, winwidth, winheight,
		event.expose.x, event.expose.y,
		event.expose.width, event.expose.height);
      if (prog_slideshow && first) {
	first = 0;
	set_ready(disp, scrn, RDY_READY);
      }
      break;

    case EnterNotify:
      if (install)
	XInstallColormap(disp, xcmap);
      break;

    case LeaveNotify:
      if (install)
	XUninstallColormap(disp, xcmap);
    }
  }
}
