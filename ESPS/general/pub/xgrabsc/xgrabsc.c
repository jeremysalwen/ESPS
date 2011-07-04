/*========================================================================
 *
 * Name - xgrabsc.c
 *
 * Version:	1.12
 *
 * ccsid:	@(#)xgrabsc.c	1.12 - 7/26/91 15:15:31
 * from: 	ccs/s.xgrabsc.c
 * date: 	7/26/91 15:16:10
 *
 * Copyright (c) 1990 Bruce Schuchardt.
 * Read the file cpyright.h for full copyright information.
 *
 *
 * Description:
 *
 * xgrabsc - grab screen images and store in files
 *
 *========================================================================
 */

#include "cpyright.h"
#include "patchlevel.h"
#include "ps_color.h"

#include <stdio.h>

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/XWDFile.h>
#ifndef CARD32
#include <X11/Xmd.h>
#endif

#define MAX_CELLS  256
#define TRUE  1
#define FALSE 0


typedef enum {
  NO_DITHER=0,
  FS_DITHER,
  MATRIX_DITHER,
  MATRIX_HALFTONE
} ditherType;


typedef unsigned char byte;
typedef unsigned long dw;
typedef unsigned int  word;



typedef struct {
  XImage *ximage;
  word numcells;
  word red[MAX_CELLS], green[MAX_CELLS], blue[MAX_CELLS];
  byte used[MAX_CELLS];
} imageInfo;



#ifdef MEMCPY
char *memcpy();
char *memset();
#endif


#ifdef BCOPY
#ifdef MEMCPY
#undef MEMCPY
#endif
#define memcpy(x,y,c) bcopy(y,x,c)
#define memset(x,c)   bzero(x,c) /* well, I only use it for zeroing out stuff */
#endif



Display *hDisplay;
int      hScreen;
Window   hRoot;
int      displayCells;
char    *programName;
char    *imageName;

char    *version    = XGRABSC_VERSION;
int      patchLevel = XGRABSC_PATCHLEVEL;
int      verbose;

word nr[MAX_CELLS], ng[MAX_CELLS], nb[MAX_CELLS];

char   hexdigits[] = "0123456789abcdef";



/*
 * Alter colors by setting or clearing bits in rgb values.
 * This effectively reduces the depth of the image, causing the
 * number of colors used to be reduced.  Equivalent colors are
 * merged in the image, and the used flags of remapped colors are
 * cleared.
 *
 * The number of eliminated colormap entries is returned.  The colormap
 * is not compressed.
 */
alterPlanes(image, modeIsAnd, bits)
  imageInfo *image;
  int modeIsAnd;      /* if TRUE, combine mask with AND; if FALSE, use OR */
  unsigned int bits;
{
  int nc, cidx, ridx, h, w;
  long p;
  XImage *ximage = image->ximage;
  long map[MAX_CELLS];
  int remapCount;
  word mask;

  if (ximage->depth <= 1)
    return 0;

  mask = 0xFFFF ^ ((1 << (bits+8)) - 1);
  if (!modeIsAnd)
    mask = ~mask & 0xFFFF;

  if (verbose) {
    fprintf(stderr, "%s: %s color with mask %x...", programName,
            modeIsAnd? "ANDing" : "ORing", mask);
    fflush(stderr);
  }

  nc = image->numcells;
  if (modeIsAnd)
    for (cidx=0; cidx<nc; cidx++) {
      nr[cidx] = image->red[cidx]   & mask;
      ng[cidx] = image->green[cidx] & mask;
      nb[cidx] = image->blue[cidx]  & mask;
    }
  else
    for (cidx=0; cidx<nc; cidx++) {
      nr[cidx] = image->red[cidx]   | mask;
      ng[cidx] = image->green[cidx] | mask;
      nb[cidx] = image->blue[cidx]  | mask;
    }

  /* now eliminate redundant colors */
  for (cidx=0; cidx<nc; cidx++)
    map[cidx] = cidx;
  remapCount = 0;
  for (cidx=0; cidx<nc; cidx++)
    if (image->used[cidx])
      for (ridx=cidx+1; ridx<nc; ridx++)
        if (image->used[ridx]  &&
            nr[cidx]==nr[ridx] &&
            ng[cidx]==ng[ridx] &&
            nb[cidx]==nb[ridx]) {
          /* the colors match - remap this pixel to the one we're scanning with */
          map[ridx] = cidx;
          image->used[ridx] = FALSE;
          remapCount++;
        }

  memcpy(image->red,   nr, nc*sizeof(word));
  memcpy(image->green, ng, nc*sizeof(word));
  memcpy(image->blue,  nb, nc*sizeof(word));

  /* remap redundant pixels in the image */
  if (remapCount)
    for (h=0; h<ximage->height; h++)
      for (w=0; w<ximage->width; w++) {
        p = XGetPixel(ximage, w, h);
        if (p != map[p])
          XPutPixel(ximage, w, h, map[p]);
      }

  if (verbose)
    fprintf(stderr, "  %d colors remapped\n", remapCount, nc);
  return remapCount;
}





/* Brighten or darken colors in the image by the given amount ('percent').
 * The amount is an integer that, if less than 100 will darken the image
 * and if greater than 100 will brighten the image.  After modifying
 * colors equivalent colors are merged (as in alterPlanes).  The number
 * of eliminated colors is returned.
 */
brightenColors(image, percent)
  imageInfo *image;
  int percent;
{
  int nc, cidx, ridx, h, w;
  long p;
  XImage *ximage = image->ximage;
  float  adjustment;
  long map[MAX_CELLS];
  int remapCount;
  dw new;

  if (ximage->depth <= 1)
    return 0;

  if (verbose) {
    fprintf(stderr, "%s: adjusting intensity by %d...", programName, percent);
    fflush(stderr);
  }

  adjustment = (float)percent / 100.0;
  nc = image->numcells;
  for (cidx=0; cidx<nc; cidx++) {
    new = image->red[cidx] * adjustment;
    if (new > (dw)0xFFFF) new = (dw)0xFFFF;
    nr[cidx] = new;
    new = image->green[cidx] * adjustment;
    if (new > (dw)0xFFFF) new = (dw)0xFFFF;
    ng[cidx] = new;
    new = image->blue[cidx] * adjustment;
    if (new > (dw)0xFFFF) new = (dw)0xFFFF;
    nb[cidx] = new;
  }

  /* now eliminate redundant colors */
  for (cidx=0; cidx<nc; cidx++)
    map[cidx] = cidx;
  remapCount = 0;
  for (cidx=0; cidx<nc; cidx++)
    if (image->used[cidx])
      for (ridx=cidx+1; ridx<nc; ridx++)
        if (image->used[ridx]  &&
            nr[cidx]==nr[ridx] &&
            ng[cidx]==ng[ridx] &&
            nb[cidx]==nb[ridx]) {
          map[ridx] = cidx;
          image->used[ridx] = FALSE;
          remapCount++;
        }

  memcpy(image->red,   nr, nc*sizeof(word));
  memcpy(image->green, ng, nc*sizeof(word));
  memcpy(image->blue,  nb, nc*sizeof(word));

  /* remap redundant pixels in the image */
  if (remapCount)
    for (h=0; h<ximage->height; h++)
      for (w=0; w<ximage->width; w++) {
        p = XGetPixel(ximage, w, h);
        if (p != map[p])
          XPutPixel(ximage, w, h, map[p]);
      }


  if (verbose)
    fprintf(stderr, "  %d colors remapped\n", remapCount, nc);

  return remapCount;
}





/*
 * Compress the colors used in an XImage so that all pixel values are
 * adjacent.  Alters the rgb color tables and the XImage data values.
 */
compressColormap(image)
  imageInfo *image;
{
  XImage *ximage = image->ximage;
  long map[MAX_CELLS];
  int  ncolors, w, h, m;
  long p;

  if (ximage->depth <= 1  ||  image->numcells > MAX_CELLS)
    return;

  if (verbose) {
    fprintf(stderr, "%s: compressing colormap...", programName);
    fflush(stderr);
  }
  ncolors = 0;
  /* map[] is indexed by old pixel values.  It delivers new, compressed,
   * pixel values. */
  for (m=0; m<MAX_CELLS; m++) map[m] = MAX_CELLS+1;
  /* bludgeon through the whole image and remap each pixel value */
  for (h=0; h<ximage->height; h++) {
    for (w=0; w<ximage->width; w++) {
      /* Get the pixel index and see if it has been used or not.
       * Then remap the pixel */
      p = XGetPixel(ximage, w, h);
      if (map[p] == MAX_CELLS+1) {
        map[p] = ncolors;
        ncolors++;
      }
      if (p != map[p])
        XPutPixel(ximage, w, h, map[p]);
    }
  }
  /* now compress the color table */
  memset(image->used, 0, MAX_CELLS);
  for (m=0; m<MAX_CELLS; m++) {
    if (map[m] != MAX_CELLS+1) {
      p = map[m];
      nr[p] = image->red[m];
      ng[p] = image->green[m];
      nb[p] = image->blue[m];
      image->used[p] = TRUE;
    }
  }
  memcpy(image->red,   nr, ncolors*sizeof(word));
  memcpy(image->green, ng, ncolors*sizeof(word));
  memcpy(image->blue,  nb, ncolors*sizeof(word));
  image->numcells = ncolors;
  if (verbose)
    fprintf(stderr, "  %d colors used\n", ncolors);
}





/*
 * Get the image bounded by the given rectangle.
 * The associated colormap information is also extracted and returned.
 * TRUE is returned if an image was successfully grabbed, and FALSE
 * otherwise.
 */
getImage(xrect, image, window)
  XRectangle *xrect;
  imageInfo *image;
  Window window;
{
  XImage *ximage;
  int depth, ncolors, cmapSize, numCmaps;
  int h, w;
  long i;
  XColor colors[MAX_CELLS];
  Colormap *cmaps, cmap;

  if (xrect->width == 0  || xrect->height == 0)
    return FALSE;

  depth  = DefaultDepth(hDisplay, hScreen);
  ximage = XGetImage(hDisplay, window,
            xrect->x, xrect->y, xrect->width, xrect->height, AllPlanes,
            depth==1 ? XYPixmap : ZPixmap);
  image->ximage = ximage;

  /* get the colormap info too */

  cmaps = XListInstalledColormaps(hDisplay, window, &numCmaps);
  if (numCmaps == 0)
    cmap = DefaultColormap(hDisplay, hScreen);
  else {
    cmap = *cmaps;
    if (numCmaps > 1)
      fprintf(stderr,
        "%s: more than one colormap found - using first encountered",
        programName);
  }
  XFree(cmaps);

  ncolors = DisplayCells(hDisplay, hScreen);
  /* this won't cut the mustard for DirectColor */
  for (i=0; i<ncolors; i++)
    colors[i].pixel = i;

  XQueryColors(hDisplay, cmap, colors, ncolors);
  for (i=0; i<ncolors; i++) {
    image->red[i]   = colors[i].red;
    image->green[i] = colors[i].green;
    image->blue[i]  = colors[i].blue;
  }

  /* figure out which colormap entries are actually used by the image */
  ncolors = cmapSize = 0;
  memset(image->used, 0, MAX_CELLS);
  for (h=0; h<ximage->height; h++)
    for (w=0; w<ximage->width; w++) {
      i = XGetPixel(ximage, w, h);
      if (!image->used[i]) {
        image->used[i] = TRUE;
        if (i+1 > cmapSize)      /* keep track of colormap size */
          cmapSize = i+1;
        ncolors++;
      }
    }
  image->numcells = cmapSize;
  if (verbose)
    fprintf(stderr, "%s: image has %d colors\n", programName, ncolors);

  return TRUE;
}



/*
 * Let the user stretch a rectangle on the screen and return its values.
 * It may be wise to grab the server before calling this routine.  If the
 * screen is allowed to change during XOR drawing video droppings may result.
 */
getRectangle(xrect)
  XRectangle *xrect;
{
  XEvent event;
  unsigned int mask, x, y, rootx, rooty;
  GC gc;
  Cursor pointer1, pointer2;
  int boxDrawn = False;
  int rx, ry, rw, rh;
  Window root, child;
  int discarded;

  /* get some cursors for rectangle formation */
  pointer1 = XCreateFontCursor(hDisplay, XC_ul_angle);
  pointer2 = XCreateFontCursor(hDisplay, XC_lr_angle);

  /* grab the pointer */
  if (GrabSuccess != XGrabPointer(hDisplay, hRoot, False, ButtonPressMask,
        GrabModeAsync, GrabModeAsync, hRoot, pointer1, CurrentTime)) {
    fprintf(stderr,"%s - could not grab pointer!\n", programName);
    exit(3);
  }

  /* create a graphics context to draw with */
  gc = XCreateGC(hDisplay, hRoot, 0, NULL);
  if (!gc) {
    fprintf(stderr,"%s - could not get drawing resources\n", programName);
    exit(3);
  }
  XSetSubwindowMode(hDisplay, gc, IncludeInferiors);
  XSetForeground(hDisplay, gc, 255);
  XSetFunction(hDisplay, gc, GXxor);

  /* get a button-press and pull out the root location */
  XMaskEvent(hDisplay, ButtonPressMask, &event);
  rootx = rx = event.xbutton.x_root;
  rooty = ry = event.xbutton.y_root;

  /* get pointer motion events */
  XChangeActivePointerGrab(hDisplay, ButtonMotionMask | ButtonReleaseMask,
        pointer2, CurrentTime);


  /* MAKE_RECT converts the original root coordinates and the event root
   * coordinates into a rectangle in xrect */
#define MAKE_RECT(etype) \
  x = event.etype.x_root;       \
  y = event.etype.y_root;       \
  rw  = x - rootx;              \
  if (rw  < 0) rw  = -rw;       \
  rh  = y - rooty;              \
  if (rh  < 0) rh  = -rh;       \
  rx = x < rootx ? x : rootx;   \
  ry = y < rooty ? y : rooty

  /* loop to let the user drag a rectangle */
  while (TRUE) {
    XNextEvent(hDisplay, &event);
    switch(event.type) {
      case ButtonRelease:
        if (boxDrawn) {
          XDrawRectangle(hDisplay, hRoot, gc, rx, ry, rw, rh);
          boxDrawn = False;
        }
        XFlush(hDisplay);
        /* record the final location */
        MAKE_RECT(xbutton);
        /* release resources */
        XFreeGC(hDisplay, gc);
        XFreeCursor(hDisplay, pointer1);
        XFreeCursor(hDisplay, pointer2);
        xrect->x      = rx;
        xrect->y      = ry;
        xrect->width  = rw;
        xrect->height = rh;
        XUngrabPointer(hDisplay, CurrentTime);
        if (verbose)
          fprintf(stderr, "%s: rectangle is %d@%d,  %dx%d\n", programName,
              xrect->x, xrect->y, xrect->width, xrect->height);
        return True;
      case MotionNotify:
        if (boxDrawn) {
          XDrawRectangle(hDisplay, hRoot, gc, rx, ry, rw, rh);
          boxDrawn = False;
        }
        /* discard incoming motion notifies while we handle this one */
        discarded = False;
        while (XCheckTypedEvent(hDisplay, MotionNotify, &event))
          {}
        MAKE_RECT(xmotion);
        XDrawRectangle(hDisplay, hRoot, gc, rx, ry, rw, rh);
        boxDrawn = True;
        break;
    }
  }
}






/*
 * choose a window as in xwd
 */

Window getWindow() {
  int status;
  Cursor cursor;
  XEvent event;
  Window result;

  result = None;

  cursor = XCreateFontCursor(hDisplay, XC_target);

  status = XGrabPointer(hDisplay, hRoot, FALSE,
         ButtonPressMask|ButtonReleaseMask, GrabModeSync,
         GrabModeAsync, None, cursor, CurrentTime);
  if (status != GrabSuccess) {
    fprintf(stderr, "%s: can't grab mouse\n", programName);
    exit(3);
  }

  while (TRUE) {
    XAllowEvents(hDisplay, SyncPointer, CurrentTime);
    XWindowEvent(hDisplay, hRoot, ButtonPressMask|ButtonReleaseMask, &event);

    switch (event.type) {
      case ButtonRelease:
        result = event.xbutton.subwindow;
        if (result == None)
          result = hRoot;
        XUngrabPointer(hDisplay, CurrentTime);      /* Done with pointer */
        return result;
        break;
    }
  }
}







#ifdef MEMCPY

/* memcpy and memset routines from C News */


/*
 * memcpy - copy bytes
 */

char *
memcpy(dst, src, size)
char * dst;
 char * src;
int size;
{
        register char *d;
        register  char *s;
        register int n;

        if (size <= 0)
                return(dst);

        s = src;
        d = dst;
        if (s <= d && s + (size-1) >= d) {
                /* Overlap, must copy right-to-left. */
                s += size-1;
                d += size-1;
                for (n = size; n > 0; n--)
                        *d-- = *s--;
        } else
                for (n = size; n > 0; n--)
                        *d++ = *s++;

        return(dst);
}

/*
 * memset - set bytes
 *
 * CHARBITS should be defined only if the compiler lacks "unsigned char".
 * It should be a mask, e.g. 0377 for an 8-bit machine.
 */

#ifndef CHARBITS
#       define  UNSCHAR(c)      ((unsigned char)(c))
#else
#       define  UNSCHAR(c)      ((c)&CHARBITS)
#endif

char *
memset(s, ucharfill, size)
 char * s;
register int ucharfill;
int size;
{
        register  char *scan;
        register int n;
        register int uc;

        scan = s;
        uc = UNSCHAR(ucharfill);
        for (n = size; n > 0; n--)
                *scan++ = uc;

        return(s);
}
#endif /* MEMCPY */





/*
 * convert a pixmap image into a bitmap image
 */
pixmap2bitmap(image)
  imageInfo *image;
{
  XImage *ximage = image->ximage;
  int x, y;
  word v, black, mid;
  dw total, blackrgb, midrgb, lowDelta, l;
  XImage *newImage;
  byte *newBytes;
  int usedCount;
  int blackp, whitep;

  if (ximage->bits_per_pixel == 1  ||  image->numcells < 1)
    return;


  blackp = BlackPixel(hDisplay,hScreen);
  whitep = WhitePixel(hDisplay,hScreen);

  /* get the darkest color */
  blackrgb = 0x2FFFD;  /* 3 * 0xFFFF == white */
  usedCount = total = 0;
  for (x=0; x<image->numcells; x++) {
    if (image->used[x]) {
      l = (unsigned)image->red[x]
          +(unsigned)image->green[x]
          +(unsigned)image->blue[x];
      if (l <= blackrgb) {
        black = x;
        blackrgb = l;
      }
      total += l;
      usedCount++;
    }
  }
  /* now find the mid color and use it as the cut-off for black */
  midrgb = total / usedCount;
  lowDelta = 0x2FFFD;
  for (x=0; x<image->numcells; x++) {
    if (image->used[x]) {
      l = (unsigned)image->red[x]
          +(unsigned)image->green[x]
          +(unsigned)image->blue[x];
      l -= midrgb;
      if (l < lowDelta) {
        mid = x;
        lowDelta = l;
      }
    }
  }
  midrgb = (unsigned)image->red[mid]
           +(unsigned)image->green[mid]
           +(unsigned)image->blue[mid];

  /* create a bitmap image */
  x = (ximage->width + 7) / 8;
  newBytes = (byte *)malloc(x * ximage->height);
  memset(newBytes, 0, x * ximage->height);
  newImage = XCreateImage(hDisplay, DefaultVisual(hDisplay, hScreen),
                1, XYBitmap, 0, newBytes, ximage->width, ximage->height,
                0, x);
  if (!newImage) {
    fprintf(stderr, "%s: unable to create bitmap for conversion\n",
      programName);
    XCloseDisplay(hDisplay);
    exit(3);
  }
  /* pound the pixels into it */
  for (y = 0; y < ximage->height; y++) {
    for (x = 0; x < ximage->width; x++) {
      v = XGetPixel(ximage, x, y);
      l = (dw)image->red[v]+(dw)image->green[v]+(dw)image->blue[v];
      XPutPixel(newImage, x, y, l<midrgb? blackp : whitep);
    }
  }
  free(ximage->data);
  memcpy(ximage, newImage, sizeof(XImage));
  free(newImage);

  memset(image->used, 0, MAX_CELLS);
  image->used[whitep] = 1;
  image->used[blackp] = 1;
  image->numcells = 2;
}







#define GRAYS    17 /* ((4 * 4) + 1) patterns for a good dither */
#define GRAYSTEP ((dw)(65536 / GRAYS))

static byte DitherBits[GRAYS][4] = {
  0xf, 0xf, 0xf, 0xf,
  0xe, 0xf, 0xf, 0xf,
  0xe, 0xf, 0xb, 0xf,
  0xa, 0xf, 0xb, 0xf,
  0xa, 0xf, 0xa, 0xf,
  0xa, 0xd, 0xa, 0xf,
  0xa, 0xd, 0xa, 0x7,
  0xa, 0x5, 0xa, 0x7,
  0xa, 0x5, 0xa, 0x5,
  0x8, 0x5, 0xa, 0x5,
  0x8, 0x5, 0x2, 0x5,
  0x0, 0x5, 0x2, 0x5,
  0x0, 0x5, 0x0, 0x5,
  0x0, 0x4, 0x0, 0x5,
  0x0, 0x4, 0x0, 0x1,
  0x0, 0x0, 0x0, 0x1,
  0x0, 0x0, 0x0, 0x0
};

/* halftone or dither a color image, changing it into a monochrome
 * image
 */
pixmap2halftone(image, dither)
  imageInfo *image;
  ditherType dither;    /* type of dithering to perform */
{
  XImage *ximage = image->ximage;
  XImage *newImage;
  byte   *newBytes, *ditherBits;
  word   dindex;  /* index into dither array */
  dw     color;   /* pixel color */
  word  *index;   /* index into dither array for a given pixel */
  word   x, y;    /* random counters */
  word   x4, y4;
  register word   w, h;
  register byte  bits;
  char  *str;
  dw    intensity;
  int   maxIntensity, threshold;
  word  *fsIndex;
  int   err, i;
  int   *row1, *row2;
  int blackp = BlackPixel(hDisplay,hScreen);
  int whitep = WhitePixel(hDisplay,hScreen);

  if (ximage->depth <= 1  ||  dither == NO_DITHER)
    return;

  if (verbose) {
    switch (dither) {
      case MATRIX_HALFTONE:
        str = "Matrix halfton";
        break;
      case MATRIX_DITHER:
        str = "Matrix dither";
        break;
      case FS_DITHER:
        str = "Floyd-Steinberg dither";
        break;
      default:
        fprintf(stderr, "%s: unknown type of dithering requested.  Exiting...\n",
            programName);
        exit(3);
    }
    fprintf(stderr, "%s: %sing image...", programName, str);
    fflush(stderr);
  }

  /* create a bitmap image */
  x = (dither == MATRIX_HALFTONE)? 4 : 1;
  w = ((ximage->width + 7) / 8) * x;
  h = ximage->height * x;
  newBytes = (byte *)malloc(w * h);
  memset(newBytes, 0, w * h);
  newImage = XCreateImage(hDisplay, DefaultVisual(hDisplay, hScreen),
                1, XYBitmap, 0, newBytes,
                ximage->width * x,
                h,
                0, w);
  if (!newImage) {
    fprintf(stderr, "%s: unable to create bitmap for conversion\n",
      programName);
    XCloseDisplay(hDisplay);
    exit(3);
  }

  /* if the number of possible pixels isn't very large, build an array
   * which we index by the pixel value to find the dither array index
   * by color brightness.  we do this in advance so we don't have to do
   * it for each pixel.  things will break if a pixel value is greater
   * than (1 << depth), which is bogus anyway.  this calculation is done
   * on a per-pixel basis if the colormap is too big.
   */

  if (ximage->depth <= 16) {
    index= (word *)malloc(sizeof(word) * MAX_CELLS);
    fsIndex= (word *)malloc(sizeof(word) * MAX_CELLS);
    if (index)
      for (x= 0; x < image->numcells; x++) {
        fsIndex[x] = (word)(0.30 * image->red[x] +
                          0.59 * image->green[x] +
                          0.11 * image->blue[x]);
        index[x] = fsIndex[x]/GRAYSTEP;
        if (index[x] >= GRAYS)
          index[x] = GRAYS - 1;
      }
  }
  else
    index = fsIndex = NULL;

  if (dither == FS_DITHER) {
    maxIntensity = 65535;
    threshold = maxIntensity/2;
    row1 = (int *)malloc(ximage->width*sizeof(int));
    row2 = (int *)malloc(ximage->width*sizeof(int));
    /* initialize row2 */
    for (x= 0; x < ximage->width; x++) {
      color = XGetPixel(ximage, x, 0);
      row2[x] = fsIndex? fsIndex[color] :
                  (dw)(0.30*image->red[color] +
                   0.59*image->green[color] +
                   0.11*image->blue[color]);
    }
    for (y= 0; y < ximage->height; y++) {
      /* row1 := row2 */
      memcpy(row1, row2, ximage->width*sizeof(int));
      /* Fill in next row */
      if (y != ximage->height-1)
        for (x= 0; x < ximage->width; x++) {
          color = XGetPixel(ximage, x, y+1);
          row2[x] = fsIndex? fsIndex[color] :
                      (dw)(0.30*image->red[color] +
                       0.59*image->green[color] +
                       0.11*image->blue[color]);
        }
      for (x= 0; x < ximage->width; x++) {
        color = XGetPixel(ximage, x, y);
        intensity = fsIndex? fsIndex[color] :
                    (dw)(0.30*image->red[color] +
                     0.59*image->green[color] +
                     0.11*image->blue[color]);
        if ((i = row1[x]) > threshold)
          err = i - maxIntensity;
        else {
          XPutPixel(newImage, x, y, blackp);
          err = i;
        }
        /* Diagonal gets 1/4 of error. */
        row2[x+1] += err/4;

        /* Right and below get 3/8 of error */
        err = err*3/8;
        row2[x] += err;
        row1[x+1] += err;
      }
    }
    if (row1)  free(row1);
    if (row2)  free(row2);
  }


  else {  /* matrix dither or halftone */

    for (y= 0; y < ximage->height; y++) {
      for (x= 0; x < ximage->width; x++) {
        color = XGetPixel(ximage, x, y);
        dindex = index? index[color] :
                    (dw)(0.30*image->red[color] +
                     0.59*image->green[color] +
                     0.11*image->blue[color])/GRAYSTEP;
        if (dindex >= GRAYS)  /* catch rounding errors */
          dindex= GRAYS - 1;
        if (dither == MATRIX_DITHER) {
          if (DitherBits[dindex][y & 3] & (1 << (x & 3)))
             XPutPixel(newImage, x, y, blackp);
        }
        else { /* halftone */
          /* loop for the four Y bits in the dither pattern, putting all
           * four X bits in at once.  if you think this would be hard to
           * change to be an NxN dithering array, you're right, since we're
           * banking on the fact that we need only shift the mask based on
           * whether x is odd or not.  an 8x8 array wouldn't even need that,
           * but blowing an image up by 64x is probably not a feature.
           */
          ditherBits = &(DitherBits[dindex][0]);
          x4 = x * 4;
          y4 = y * 4;
          for (h= 0; h < 4; h++) {
            bits = ditherBits[h];
            for (w=0; w < 4; w++) {
              XPutPixel(newImage, x4+w, y4+h, bits & 1 ? blackp : whitep);
              bits /= 2;
            }
          }
        }
      }
    }
  }

  if (verbose)
    fputc('\n', stderr);

  free(ximage->data);
  memcpy(ximage, newImage, sizeof(XImage));
  free(newImage);
  if (index) free(index);
  if (fsIndex) free(fsIndex);

  image->numcells = 0;
}





/* swap the bits in a byte */
swapbits(b)
  byte b;
{
  byte b2;

  b2 = 0;
  b2 |= (b & 0x01) << 7;
  b2 |= (b & 0x02) << 5;
  b2 |= (b & 0x04) << 3;
  b2 |= (b & 0x08) << 1;
  b2 |= (b & 0x10) >> 1;
  b2 |= (b & 0x20) >> 3;
  b2 |= (b & 0x40) >> 5;
  b2 |= (b & 0x80) >> 7;
  return b2;
}




/* swap the bytes in a long int */
swapbytes(pDblw)
  dw *pDblw;
  {
  union {
    dw  dbl;
    byte bytes[4];
    } cnv;
  byte aByte;

  cnv.dbl = *pDblw;
  aByte = cnv.bytes[0];
  cnv.bytes[0] = cnv.bytes[3];
  cnv.bytes[3] = aByte;
  aByte = cnv.bytes[1];
  cnv.bytes[1] = cnv.bytes[2];
  cnv.bytes[2] = aByte;
  *pDblw = cnv.dbl;
  }



/* swap some long ints.  (n is number of BYTES, not number of longs) */
swapdws (bp, n)
  register char *bp;
  register unsigned n;
{
  register char c;
  register char *ep = bp + n;
  register char *sp;

  while (bp < ep) {
    sp = bp + 3;
    c = *sp;
    *sp = *bp;
    *bp++ = c;
    sp = bp + 1;
    c = *sp;
    *sp = *bp;
    *bp++ = c;
    bp += 2;
  }
}



/* swap some short ints */
swapwords (bp, n)
  register char *bp;
  register unsigned n;
{
  register char c;
  register char *ep = bp + n;

  while (bp < ep) {
    c = *bp;
    *bp = *(bp + 1);
    bp++;
    *bp++ = c;
  }
}





writeSimple(image, outfile)
  imageInfo *image;
  FILE *outfile;
{
  dw width, height, hasColormap, colormapSize;
  dw swaptest = 1;
  int i, w, h;

  if (verbose)
    fprintf("%s: writing in simple output format\n", programName);
  if (image->ximage->depth != 8) {
    fprintf("%s: can't write simple image format if depth is not 8\n",
            programName);
    return;
  }
  width        = image->ximage->width;
  height       = image->ximage->height;
  hasColormap  = 1;
  colormapSize = image->numcells;
  if (*(char *)&swaptest==0) {
    swapdws(&width,        1);
    swapdws(&height,       1);
    swapdws(&hasColormap,  1);
    swapdws(&colormapSize, 1);
  }
  fwrite(&width, 4, 1, outfile);
  fwrite(&height, 4, 1, outfile);
  fwrite(&hasColormap, 4, 1, outfile);
  fwrite(&colormapSize, 4, 1, outfile);
  for (i=0; i<image->numcells; i++)
    fputc((byte)(image->red[i]>>8), outfile);
  for (i=0; i<image->numcells; i++)
    fputc((byte)(image->green[i]>>8), outfile);
  for (i=0; i<image->numcells; i++)
    fputc((byte)(image->blue[i]>>8), outfile);
  for (i=0; i<image->numcells; i++)
    fputc((byte)(image->used[i]), outfile);
  for (h=0; h<image->ximage->height; h++)
    for (w=0; w<image->ximage->width; w++)
      fputc(XGetPixel(image->ximage, w, h), outfile);
}









/*
 * Write an image in Postscript format
 */
writePostscript(image, outfile, encode, encapsulate)
  imageInfo *image;
  FILE *outfile;
  int encode;       /* TRUE if we're to encode the Postscript output */
  int encapsulate;  /* TRUE if encapsulated Postscript output is wanted */
{
  register byte b, *ptr;
  register int x, y;
  register int i;
  XImage *ximage = image->ximage;
  XImage *psimage;
  double xdpi, ydpi, xscale, yscale, f;
  int lshift, lmask;
  int depth, bpl, spb;
  int reverse;
  long p;
  /* rle variables */
  int rlecount;
  byte rlesample;
  dw  rletotal;
  int widthcount;
  int firstSample;

  if (verbose)
    fprintf(stderr, "%s: formatting Postscript output\n", programName);

  /* use depth as the number of bits in output samples */
  depth = ximage->depth;
  /* postscript only supports 1, 2, 4, or 8 */
  if (depth > 8) depth = 8;     /* max postscript bits/sample */
  if (depth < 8 && depth > 4) depth = 8;
  if (depth == 3) depth = 4;


  bpl = ((ximage->width * depth) + 7) / 8;

  if (depth == 1)
    psimage = ximage;
  else {
    /* colors have to be changed to luminescence */
    ptr = (byte *)malloc(ximage->height * bpl);
    psimage = XCreateImage(hDisplay, DefaultVisual(hDisplay, hScreen),
                  depth, ZPixmap,
                  0, ptr,
                  ximage->width, ximage->height,
                  0, bpl);
    if (!psimage) {
      fprintf(stderr, "%s: could not create image for Postscript conversion\n",
        programName);
      exit(3);
    }
    /* force the bits_per_pixel to be what is needed */
    psimage->bits_per_pixel = depth;
  }

  spb = 8 / psimage->bits_per_pixel;    /* samples per byte */

  if (depth > 1) {
    /* translate colors into grays */
    lshift = 16 - psimage->bits_per_pixel;
    lmask  = (1 << psimage->bits_per_pixel) - 1;
    for (y = 0; y < ximage->height; y++) {
      for (x = 0; x < ximage->width; x++) {
        p = XGetPixel(ximage, x, y);
        i = (0.30*(double)image->red[p]) +
            (0.59*(double)image->green[p])+
            (0.11*(double)image->blue[p]);
        i = (i >> lshift) & lmask;
        XPutPixel(psimage, x, y, i);
      }
    }
  }


#ifndef NO_RLE_CHECKS
  if (encode) {
    rletotal = 0;
    rlecount = 0;
    firstSample = TRUE;
    for (y=0; y<psimage->height; y++)
      for (x=0, ptr=(byte *)(psimage->data + (y * psimage->bytes_per_line));
           x<psimage->width; x+=spb, ptr++) {
        b = *ptr;
	if (firstSample || b != rlesample || rlecount==254) {
	  if (!firstSample)
	    rletotal += 2;
	  else
	    firstSample = FALSE;
	  rlecount = 0;
	  rlesample = b;
	}
	else
	  rlecount++;
      }
    if (!firstSample)
      rletotal += 2;
    f = (float)(rletotal) / (float)(psimage->height*bpl);
    if (verbose)
      fprintf(stderr, "%s: encoding would change to %5.1f%% of orig size\n",
            programName, f * 100.0);
    encode = f <= 0.95;
  }
#endif



  if (verbose)
    fprintf(stderr, "%s: image will %sbe encoded\n", programName,
            encode? "" : "not ");

  if (encapsulate) {
    fprintf(outfile, "%%!PS-Adobe-2.0 EPSF-1.2\n");
    fprintf(outfile, "%%%%BoundingBox: %d %d %d %d\n",
                0, 0, psimage->width, psimage->height);
  }
  else
    fprintf(outfile, "%%!\n");

  fprintf(outfile, "%%%%Creator: xgrabsc\n");
  fprintf(outfile, "%%%%Title: %s\n", imageName);
  time(&p);
  fprintf(outfile, "%%%%CreationDate: %s", ctime(&p));
  fprintf(outfile, "%%%%EndComments\n");
  fprintf(outfile, "%%\n");
  fprintf(outfile, "%%\n");

  /* standard inch procedure */
  fputs("/inch {72 mul} def\n", outfile);


  /* define a string to hold image bytes */
  if (encode)
    fputs("/rlebuffer 2 string def\n", outfile);
  else
    fprintf(outfile, "/picstr %d string def\n", bpl);

  /* define the image plotting procedure */
  fputs("/plotimage\n", outfile);

    /* parameters for the standard image procedure */
    fprintf(outfile, "  {%d %d %d [%d 0 0 -%d 0 %d]\n",
          psimage->width, psimage->height, psimage->bits_per_pixel,
          psimage->width, psimage->height, psimage->height);

    /* line reading function  */

  if (encode) {
    fputs("% run-length decoding block\n",                                       outfile);
    fputs("  { currentfile rlebuffer readhexstring pop pop\n",                   outfile);
    fputs("    rlebuffer 0 get 1 add       %% number of copies of the sample\n", outfile);
    fputs("    /nsamples exch store        %% save it away\n",                   outfile);
    fputs("    /lum rlebuffer 1 get store  %% the sample itself\n",              outfile);
    fputs("    /samples nsamples string store\n",                                outfile);
    fputs("    0 1 nsamples -1 add { samples exch lum put } for\n",              outfile);
    fputs("    samples                     %% leave the \n",                     outfile);
    fputs("  }\n", outfile);
  }
  else
    fputs("  {currentfile picstr readhexstring pop}\n", outfile);

  fputs("  image\n} def\n", outfile);


  /* save context and move to a nice origin */
  fputs("gsave\n", outfile);

  if (encapsulate) {
    /* for encapsulated postscript, we need a scale factor that is equal
     * to the image width/height in samples */
    fprintf(outfile, "%d %d scale\n", psimage->width, psimage->height);
  }
  else {
    /* For physical output we need a scale factor that will create
     * the same size image, and we need to center it on the page.
     *   -Figure out the physical dimensions on the screen
     *    and make it come out the same on the printer.
     *   -Use inch units and assume the paper is 8.5x11.0.
     *   (future change: allow selection of paper size and margins)        */
    xdpi = (((double)DisplayWidth(hDisplay,hScreen)) * 25.4) /
            ((double)DisplayWidthMM(hDisplay,hScreen));
    ydpi = (((double)DisplayHeight(hDisplay,hScreen)) * 25.4) /
            ((double)DisplayHeightMM(hDisplay,hScreen));
    xscale = ((double)psimage->width) / xdpi;
    yscale = ((double)psimage->height) / ydpi;
    if (xscale > 7.5) {
      yscale *= 7.5 / xscale;
      xscale = 7.5;
    }
    else if (yscale > 10.0) {
      xscale *= 10.0 / yscale;
      yscale = 10.0;
    }
    fprintf(outfile, "%1.2g inch %1.2g inch translate\n",
                  (8.5 - xscale) / 2.0, (11.0 - yscale) / 2.0);
    fprintf(outfile, "%1.2g inch %1.2g inch scale\n", xscale, yscale);
  }

  fputs("plotimage\n", outfile);


  reverse = depth == 1? BlackPixel(hDisplay,hScreen)==1 : FALSE;
  if (encode) {
    rletotal = 0;
    rlecount = 0;
    firstSample = TRUE;
  }
  widthcount = 0;
  for (y=0; y<psimage->height; y++) {
    for (x=0, ptr=(byte *)(psimage->data+(y * psimage->bytes_per_line));
         x<psimage->width;
         x+=spb, ptr++) {
      b = *ptr;
      if (reverse) b = ~b;
      if (depth == 1  &&  psimage->bitmap_bit_order == LSBFirst)
        b = swapbits(b);
      if (encode) {
        if (firstSample || b != rlesample || rlecount==254) {
	  if (!firstSample) {
	    fprintf(outfile, "%02.2x%02.2x", rlecount, rlesample);
	    rletotal += 2;
	    widthcount += 4;
	    if (widthcount >= 60) {
	      fputc('\n', outfile);
	      widthcount = 0;
	    }
	  }
	  else
	    firstSample = FALSE;
	  rlecount = 0;
	  rlesample = b;
	}
	else
	  rlecount++;
      }
      else {
        fprintf(outfile, "%02.2x", b);
	widthcount += 2;
	if (widthcount >= 60) {
	  fputc('\n', outfile);
          widthcount = 0;
        }
      }
    }
  }

  if (encode) {
    if (!firstSample) {
      fprintf(outfile, "%02.2x%02.2x\n", rlecount, rlesample);
      rletotal += 2;
    }
    fputs("%\n", outfile);
    fprintf(outfile, "%% Run-length encoding savings = %5.1f%%\n",
          100.0 - ((float)(rletotal) * 100.0 / (float)(psimage->height * bpl)));
    fputs("%\n", outfile);
  }

  fputs("\n\n\ngrestore\nshowpage\n", outfile);


  if (psimage != ximage) {
    free(psimage->data);
    free(psimage);
  }
}





/*
 * Write an image in Color Postscript format
 * Run-length encoding is not yet implemented but the request is accepted.
 */
writeColorPS(image, outfile, encode, encapsulate)
  imageInfo *image;
  FILE *outfile;
  int encode;       /* TRUE if we're to encode the Postscript output    */
  int encapsulate;  /* TRUE if encapsulated Postscript output is wanted */
{
  register byte *ptr, b;
  register int x, y;
  XImage *ximage = image->ximage;
  double xdpi, ydpi, xscale, yscale, f;
  double left, right, top, bottom;
  int depth, bpl, spb;
  long p;
  /* rle variables */
  int rlecount;
  dw  rletotal;
  byte rlesample;
  int firstSample;
  int widthcount;


  if (verbose)
    fprintf(stderr, "%s: formatting Color Postscript output\n", programName);

  depth = 8;                                  /* bits per sample  */
  spb   = 1;                                  /* samples per byte */
  bpl = ((ximage->width * depth) + 7) / 8;    /* bytes per line   */


#ifndef NO_RLE_CHECKS
  if (encode) {
    rletotal = 0;
    rlecount = 0;
    firstSample = TRUE;
    for (y=0; y<ximage->height; y++)
      for (x=0, ptr=(byte *)(ximage->data + (y * ximage->bytes_per_line));
           x<ximage->width; x+=spb, ptr++) {
        b = *ptr;
	if (firstSample || b != rlesample || rlecount==254) {
	  if (!firstSample)
	    rletotal += 2;
	  else
	    firstSample = FALSE;
	  rlecount = 0;
	  rlesample = b;
	}
	else
	  rlecount++;
      }
    rletotal += 2;
    f = (float)(rletotal) / (float)(ximage->height*bpl);
    if (verbose)
      fprintf(stderr, "%s: encoding would change to %5.1f%% of orig size\n",
            programName, f * 100.0);
    encode = f <= 0.95;
  }
#endif

  if (encapsulate) {
    fprintf(outfile, "%%!PS-Adobe-2.0 EPSF-2.0\n");
  }
  else
    fprintf(outfile, "%%!\n");

  fprintf(outfile, "%%%%Creator: xgrabsc\n");
  fprintf(outfile, "%%%%Title: %s\n", imageName);
  if (encapsulate) {
    fprintf(outfile, "%%%%Pages: 1\n");
    fprintf(outfile, "%%%%BoundingBox: %d %d %d %d\n",
                0, 0, ximage->width, ximage->height);
  }
  time(&p);
  fprintf(outfile, "%%%%CreationDate: %s", ctime(&p));
  fprintf(outfile, "%%%%EndComments\n");
  if (encapsulate) {
    fprintf(outfile, "%%%%EndProlog\n");
    fprintf(outfile, "%%%%Page: 1 1\n");
  }
  fprintf(outfile, "\n\ngsave\n\n");


#ifndef NO_COLORIMAGE_PROC
  if (!encapsulate) {
    /* put a colorImage procedure into the output file so a monochrome
    * printer can handle the file as well
    */
    for (x=0; ColorImage[x] != NULL; x++) {
      fputs(ColorImage[x], outfile);
      fputc('\n', outfile);
    }
    fprintf(outfile, "\n\n\n");
  }
#endif

  fputs("/inch {72 mul} def\n", outfile);
  fprintf(outfile, "/rgbstr %d string def\n", (int)ximage->width * 3);
  fputs("/buffer 2 string def\n", outfile);
  fputs("/rgb (000) def\n", outfile);
  fprintf(outfile, "/rgbmap %d string def\n", image->numcells * 3);
  fputs("\n\n", outfile);

  if (encapsulate) {
    /* don't translate the image for encapsulated postscript.  The
     * scale should match the data dimensions of the image in samples.  */
    fprintf(outfile, "%d %d scale\n", ximage->width, ximage->height);
  }
  else {
    /* For physical output we need a scale factor that will create
     * the same size image, and we need to center it on the page.
     *   -Figure out the physical dimensions on the screen
     *    and make it come out the same on the printer.
     *   -Use inch units and assume the paper is 8.5x11.0.
     *   (future change: allow selection of paper size and margins)
     */
    xdpi = (((double)DisplayWidth(hDisplay,hScreen)) * 25.4) /
            ((double)DisplayWidthMM(hDisplay,hScreen));
    ydpi = (((double)DisplayHeight(hDisplay,hScreen)) * 25.4) /
            ((double)DisplayHeightMM(hDisplay,hScreen));
    xscale = ((double)ximage->width) / xdpi;
    yscale = ((double)ximage->height) / ydpi;
    if (xscale > 7.5) {
      yscale *= 7.5 / xscale;
      xscale = 7.5;
    }
    else if (yscale > 10.0) {
      xscale *= 10.0 / yscale;
      yscale = 10.0;
    }

    left = ((8.5 - xscale) / 2.0);
    top  = ((11.0 - yscale) / 2.0);
    right = (left + xscale);
    bottom = (top + yscale);
    fprintf(outfile, "%1.2g inch %1.2g inch translate\n", left, top);
    fprintf(outfile, "%1.2g inch %1.2g inch scale\n", xscale, yscale);
    fprintf(outfile, "\n\n\n");
  }

  if (encode) {
    /* define a drawcolorimage procedure geared to this image
    */
    fputs("/drawcolorimage {\n", outfile);
    fprintf(outfile, "  %d %d %d\n", ximage->width, ximage->height,depth);
    fprintf(outfile, "  [%d 0 0 -%d 0 %d]\n", ximage->width, ximage->height,
                                              ximage->height);
    fputs("  %% define a block of code to read and decode the rle input stream\n", outfile);
    fputs("  { currentfile buffer readhexstring pop pop %% run length and index\n", outfile);
    fputs("    /npixels buffer 0 get 1 add 3 mul store  %% number of bytes\n", outfile);
    fputs("    /color buffer 1 get 3 mul store          %% fix index into rgb map\n", outfile);
    fputs("    /rgb rgbmap color 3 getinterval store    %% and get the colors\n", outfile);
    fputs("    /pixels npixels string store             %% make a string to hold the rgb\n", outfile);
    fputs("    0 3 npixels -1 add {                     %% loop to store the rgb bytes\n", outfile);
    fputs("      pixels exch rgb putinterval\n", outfile);
    fputs("    } for\n", outfile);
    fputs("    pixels\n", outfile);
    fputs("  }\n", outfile);
    fputs("  false 3 colorimage\n", outfile);
    fputs("} bind def\n", outfile);
    fprintf(outfile, "\n\n\n");


    fputs("%% get the rgb map\n", outfile);
    fputs("currentfile rgbmap readhexstring pop pop\n", outfile);
    for (x=0; x<image->numcells; x++)
      fprintf(outfile, "%02.2x%02.2x%02.2x\n",
                       (byte)((image->red[x]   >> 8) & 0xff),
		       (byte)((image->green[x] >> 8) & 0xff),
		       (byte)((image->blue[x]  >> 8) & 0xff) );
    fputs("\n\n", outfile);
    fputs("drawcolorimage\n", outfile);

    rletotal = 0;
    rlecount = 0;
    firstSample = TRUE;
    widthcount = 0;
    for (y=0; y<ximage->height; y++) {
      for (x=0, ptr=(byte *)(ximage->data+(y * ximage->bytes_per_line));
          x<ximage->width;
          x+=spb, ptr++) {
        b = *ptr;
        if (depth == 1  &&  ximage->bitmap_bit_order == LSBFirst)
          b = swapbits(b);
        if (firstSample || b != rlesample || rlecount==254) {
	  if (!firstSample) {
	    fprintf(outfile, "%02.2x%02.2x", rlecount, rlesample);
	    rletotal += 2;
	    widthcount += 4;
	    if (widthcount >= 60) {
	      fputc('\n', outfile);
	      widthcount = 0;
	    }
	  }
	  else
	    firstSample = FALSE;
	  rlecount = 0;
	  rlesample = b;
	}
	else
	  rlecount++;
      }
    }

    if (!firstSample) {
      fprintf(outfile, "%02.2x%02.2x\n", rlecount, rlesample);
      rletotal += 2;
    }
    fputs("%\n", outfile);
    fprintf(outfile, "%% Run-length encoding savings = %5.1f%%\n",
          100.0 - ((float)(rletotal) * 100.0 / (float)(ximage->height * bpl)));
    fputs("%\n", outfile);
  }


  else {
    /* parameters for the standard image procedure */
    fprintf(outfile, "%d %d %d [%d 0 0 -%d 0 %d]\n",
            ximage->width, ximage->height, depth,
            ximage->width, ximage->height, ximage->height);

    fputs("{ currentfile rgbstr readhexstring pop }\n", outfile);
    fputs("false 3\n", outfile);
    fputs("colorimage\n", outfile);

    for (y = 0, widthcount = 0; y < ximage->height; y++) {
      for (x = 0; x < ximage->width; x++) {
        p = XGetPixel(ximage, x, y);
        fprintf(outfile, "%02.2x%02.2x%02.2x",
	        (byte)((image->red[p]   >> 8) & 0xFF),
	        (byte)((image->green[p] >> 8) & 0xFF),
	        (byte)((image->blue[p]  >> 8) & 0xFF) );
        widthcount += 6;
        if (widthcount >= 60) {
	  fputs("\n", outfile);
	  widthcount = 0;
        }
      }
    }
  }

  fputs("\ngrestore\nshowpage\n%%Trailer\n", outfile);
}











/*
 * Write an image in 'puzzle' format, suitable for loading with
 * "puzzle -picture".
 */
writePuzzle(image, outfile)
  imageInfo *image;
  FILE *outfile;
{
  XImage *ximage = image->ximage;
  int nc, width, height, w, h, cidx;
  dw swaptest = 1;

  if (verbose)
    fprintf(stderr, "%s: formatting Puzzle output\n", programName);

  if (ximage->depth > 8) {
    fprintf(stderr, "%s: Puzzle converter can't handle depth > 8 yet\n",
            programName);
    return;
  }

  nc     = image->numcells;
  width  = ximage->width;
  height = ximage->height;
  if (*(char *)&swaptest) {
    swapbytes(&width);
    swapbytes(&height);
  }
  fwrite(&width, 4, 1, outfile);
  fwrite(&height, 4, 1, outfile);
  fputc(nc, outfile);
  for (cidx=0; cidx<nc; cidx++) {
    fputc(image->red[cidx]>>8,   outfile);
    fputc(image->green[cidx]>>8, outfile);
    fputc(image->blue[cidx]>>8,  outfile);
  }
  for (h=0; h<ximage->height; h++)
    if (ximage->bits_per_pixel == 8)
      fwrite(ximage->data+(h*ximage->bytes_per_line),ximage->width,1,outfile);
    else
      /* this won't work if depth > 8 */
      for (w=0; w<ximage->width; w++)
        fputc(XGetPixel(ximage, w, h), outfile);
}







writeXWD(image, outfile)
  imageInfo *image;
  FILE *outfile;
{
  XImage   *ximage = image->ximage;
  XWDFileHeader header;
  Visual   *visual = DefaultVisual(hDisplay, hScreen);
  XColor    color;
  dw        visMask = (visual->red_mask
                      | visual->green_mask
                      | visual->blue_mask);
  dw        swaptest = 1;
  int       i;

  if (verbose)
    fprintf(stderr, "%s: formatting xwd output\n", programName);

  header.header_size    = (CARD32)(sizeof(header)+strlen(imageName)+1);
  header.file_version   = (CARD32) XWD_FILE_VERSION;
  header.pixmap_format  = (CARD32)(ximage->depth>1? ZPixmap : XYPixmap);
  header.pixmap_depth   = (CARD32) ximage->depth;
  header.pixmap_width   = (CARD32) ximage->width;
  header.pixmap_height  = (CARD32) ximage->height;
  header.xoffset        = (CARD32) ximage->xoffset;
  header.byte_order     = (CARD32) ximage->byte_order;
  header.bitmap_unit    = (CARD32) ximage->bitmap_unit;
  header.bitmap_bit_order = (CARD32) ximage->bitmap_bit_order;
  header.bitmap_pad     = (CARD32) ximage->bitmap_pad;
  header.bits_per_pixel = (CARD32) ximage->bits_per_pixel;
  header.bytes_per_line = (CARD32) ximage->bytes_per_line;
  header.visual_class   = (CARD32)visual->class;
  header.red_mask       = (CARD32)visual->red_mask;
  header.green_mask     = (CARD32)visual->green_mask;
  header.blue_mask      = (CARD32)visual->blue_mask;
  header.bits_per_rgb   = (CARD32)visual->bits_per_rgb;
  header.colormap_entries = (CARD32)visual->map_entries;
  header.ncolors        = image->numcells;
  header.window_width   = (CARD32)ximage->width;
  header.window_height  = (CARD32)ximage->height;
  header.window_x       = 0;
  header.window_y       = 0;
  header.window_bdrwidth = 0;

  if (*(char *) &swaptest)
    swapdws(&header, sizeof(header));

  fwrite(&header, sizeof(header), 1, outfile);
  fwrite(imageName, 1, strlen(imageName)+1, outfile);

  for (i=0; i<image->numcells; i++) {
    color.pixel = i;
    color.red   = image->red[i];
    color.green = image->green[i];
    color.blue  = image->blue[i];
    color.flags = visMask;
    color.pad   = 0;
    if (*(char *) &swaptest) {
      swapdws(&color.pixel, sizeof(color.pixel));
      swapwords(&color.red, 3 * sizeof(color.red)); /* assume g and b follow r */
    }
    fwrite(&color, sizeof(XColor), 1, outfile);
  }

  fwrite(ximage->data, ximage->height * ximage->bytes_per_line, 1, outfile);
}





/*
 * Write a monochrome image out in Bitmap format.  XWriteBitmapToFile
 * requires a Pixmap as input & we'd have to invent one before we could
 * use it.
 */

writeXYPixmap(image, outfile)
  imageInfo *image;
  FILE *outfile;
{
  XImage *ximage = image->ximage;
  int w, h;
  byte b, *line;
  int lcount;
  int reverse = BlackPixel(hDisplay, hScreen) == 0;
  int swap    = ximage->bitmap_bit_order != LSBFirst;

  if (verbose)
    fprintf(stderr, "%s: formatting Bitmap output\n", programName);

  if (ximage->depth != 1) {
    fprintf(stderr, "%s: can't write polychrome images in XY bitmap format\n",
      programName);
    return;
  }

  fprintf(outfile, "#define %s_width %d\n",  imageName, ximage->width);
  fprintf(outfile, "#define %s_height %d\n", imageName, ximage->height);
  fprintf(outfile, "#define %s_x_hot 0\n",   imageName);
  fprintf(outfile, "#define %s_y_hot 0\n",   imageName);
  fprintf(outfile, "static char %s_bits[] = {\n", imageName);
  lcount = 0;
  fputs("  ", outfile);
  for (h=0; h<ximage->height; h++) {
    line = (byte *)(ximage->data + (h * ximage->bytes_per_line));
    for (w=0; w<ximage->width; w+=8) {
      b = line[w/8];
      if (reverse) b = ~b;
      if (swap)    b = swapbits(b);
      fprintf(outfile, " 0x%02x", b);
      if (h<ximage->height || w+8<ximage->width)
        fputc(',', outfile);
      lcount++;
      if (lcount >= 12) {
        fputs("\n  ", outfile);
        lcount = 0;
      }
    }
  }
  fputs("  };\n", outfile);
}








/*
 * Write a color image out in Pixmap format, suitable for loading with
 * "xpd" or "xloadimage".  Note that "xpd" usually fails miserably if
 * the image is wider than 255 characters in the output file.
 */
writeZPixmap(image, outfile)
  imageInfo *image;
  FILE *outfile;
{
  XImage *ximage = image->ximage;
  int nc, width, height, w, h, cidx, cpp;
  char mne[MAX_CELLS][3];

  if (verbose)
    fprintf(stderr, "%s: formatting Pixmap output\n", programName);

  nc  = image->numcells;
  cpp = image->numcells <= 26? 1 : 2;
  fprintf(outfile, "#define %s_format 1\n",   imageName);
  fprintf(outfile, "#define %s_width %d\n",   imageName, ximage->width);
  fprintf(outfile, "#define %s_height %d\n",  imageName, ximage->height);
  fprintf(outfile, "#define %s_ncolors %d\n", imageName, image->numcells);
  fprintf(outfile, "#define %s_chars_per_pixel %d\n",     imageName, cpp);
  fprintf(outfile, "static char * %s_colors[] = {\n", imageName);
  for (cidx=0; cidx<image->numcells; cidx++) {
    if (cpp > 1) {
      mne[cidx][0] = (char)(cidx / 10) + 'a';
      mne[cidx][1] = (char)(cidx % 10) + '0';
      mne[cidx][2] = '\0';
    }
    else {
      mne[cidx][0] = (char)cidx + (cidx? 'A' : ' ');
      mne[cidx][1] = '\0';
    }
    fprintf(outfile, "\"%s\", \"#%4.4x%4.4x%4.4x\"\n", mne[cidx],
                image->red[cidx], image->green[cidx], image->blue[cidx]);
  }
  fputs("} ;\n", outfile);
  fprintf(outfile, "static char * %s_pixels[] = {\n", imageName);
  for (h=0; h<ximage->height; h++) {
    fputs("\"", outfile);
    for (w=0; w<ximage->width; w++)
      fputs(mne[XGetPixel(ximage, w, h)], outfile);
    fputs("\",\n", outfile);
  }
  fputs("} ;\n", outfile);
}








main(argc, argv)
  int argc;
  char *argv[];
{
  char        *args[100];
  extern char *optarg;
  FILE        *outfile;
  char        *outfileName;
  XRectangle  xrect;
  imageInfo   image;
  int         argn;
  int         doAnd;
  int         doOr;
  int         depth;
  int         noBell;
  int         pixmapFormat;
  int         psColor;
  int         psFormat;
  int         puzzleFormat;
  int         simpleFormat;
  int         xwdFormat;
  int         brighten;
  int         forceBitmap;
  int         grabServer;
  ditherType  ditherKind;
  int         halftone;
  int         compress;
  int         sleepSeconds;
  int         andBits;
  int         orBits;
  int         encapsulate;
  int         sourceRoot;
  int         sourceId;
  int         sourceWd;
  Window      sourceWindow;
  long        ignored;
  char       *ptr;
  char       *display;
  int         x, y, width, height, border, i;
  int         brightenFactor;
  int         optchar;
  int         bellOn;



  bellOn      = TRUE;
  brighten    = FALSE;
  compress    = TRUE;
  display     = NULL;
  ditherKind  = NO_DITHER;
  doAnd       = FALSE;
  doOr        = FALSE;
  encapsulate = FALSE;
  forceBitmap = FALSE;
  grabServer  = TRUE;
  halftone    = FALSE;
  outfile     = stdout;
  outfileName = NULL;
  programName = argv[0];
  puzzleFormat = FALSE;
  psFormat     = TRUE;
  simpleFormat = FALSE;
  sleepSeconds = 0;
  sourceId    = FALSE;
  sourceWd    = FALSE;
  sourceRoot  = FALSE;
  verbose     = FALSE;
  xwdFormat   = FALSE;

  /* merge environment options and command line options */
  args[0] = programName;
  args[1] = (char *)getenv("XGRABSC");
  if (args[1] != NULL)
    for (argn=2; argn<100  &&
                 (args[argn]=(char *)strchr(args[argn-1], ' ')) != NULL;
	         argn++) {
      /* remove leading white space */
      while (*args[argn] == ' ' || *args[argn] == 9) {
        *(args[argn]) = '\0';
        args[argn]++;
      }
      if (*args[argn] == '|' || *args[argn] == '>') /* dbx leaves these in the cmd line */
        break;
    }
  else
    argn = 1;

  for (i=1; i<argc && argn<100; argn++, i++)
    args[argn] = argv[i];

  while ((optchar = getopt(argn, args, "ed:no:s:vg clri:w b:A:BCDFHO: IPWXZ")) != EOF) {
    switch (optchar) {
      case 'e':
        encapsulate = TRUE;
	goto pslabel;
      case 'd':
        display = optarg;
        break;
      case 'g':
        noBell = TRUE;
        break;
      case 'n':
        grabServer = FALSE;
        break;
      case 'o':
        outfileName = optarg;
        break;
      case 's':
        sleepSeconds = atoi(optarg);
        if (sleepSeconds < 0) sleepSeconds = 0;
        break;
      case 'v':
        verbose = TRUE;
        break;


      case 'c':
        compress = FALSE;
        goto pslabel;
        break;
      case 'r':
        sourceRoot = TRUE;
        sourceWd = sourceId = FALSE;
        break;
      case 'i':
        sourceId = TRUE;
        sourceRoot = sourceWd = FALSE;
        sourceWindow = 0;
        if (!sscanf(optarg, "0x%lx", &sourceWindow))
          if (!sscanf(optarg, "%ld", &sourceWindow)) {
            sprintf(stderr, "%s: invalid window id\n", programName);
            exit(3);
          }
        break;
      case 'w':
        sourceWd = TRUE;
        sourceRoot = sourceId = FALSE;
        break;



      case 'A':
        andBits = atoi(optarg);
        doAnd = TRUE;
        break;
      case 'b':
        brightenFactor = atoi(optarg);
        if (brightenFactor <= 0) {
          fprintf(stderr, "%s: brightening factor must be a positive number\n",
            programName);
          exit(3);
        }
        brighten = TRUE;
        break;
      case 'B':
        halftone = FALSE;
        forceBitmap = TRUE;
        break;
      case 'D':
        ditherKind = MATRIX_DITHER;
        halftone = TRUE;
        forceBitmap = FALSE;
        break;
      case 'F':
        ditherKind = FS_DITHER;
        halftone = TRUE;
        forceBitmap = FALSE;
        break;
      case 'H':
        if (forceBitmap) {
          fprintf(stderr,
            "%s: both bitmap and halftone conversion requested.  Using halftone.\n",
            programName);
          forceBitmap = FALSE;
        }
        if (halftone)
          fprintf(stderr,
            "%s: multiple halftone formats requested. Using Matrix halftoning.\n",
            programName);
        ditherKind = MATRIX_HALFTONE;
        halftone = TRUE;
        break;
      case 'O':
        orBits = atoi(optarg);
        doOr = TRUE;
        break;




      case 'P':
        psColor = FALSE;
pslabel:
        psFormat = TRUE;
        simpleFormat = pixmapFormat = xwdFormat = puzzleFormat = FALSE;
        break;
      case 'C':
        psColor = TRUE;
	goto pslabel;
      case 'I':
        simpleFormat = TRUE;
        puzzleFormat = pixmapFormat = psFormat = xwdFormat = FALSE;
        break;
      case 'W':
        xwdFormat = TRUE;
        simpleFormat = pixmapFormat = puzzleFormat = psFormat = FALSE;
        break;
      case 'X':
        pixmapFormat = TRUE;
        simpleFormat = psFormat = puzzleFormat = xwdFormat = FALSE;
        break;
      case 'Z':
        puzzleFormat = TRUE;
        simpleFormat = pixmapFormat = xwdFormat = psFormat = FALSE;
        break;
    }
  }


  if (verbose) {
    fprintf(stderr, "%s: xgrabsc version %s\n", programName, version);
    fprintf(stderr, "%s:         patchlevel %d\n", programName, patchLevel);
    fprintf(stderr, "%s:         %s\n\n", programName, Copyright);
  }

  if (!display) display = (char *)getenv("DISPLAY");
  hDisplay = XOpenDisplay(display);
  if (!hDisplay) {
    fprintf(stderr, "%s: could not open X display\n", programName);
    exit(3);
  }
  hScreen  = DefaultScreen(hDisplay);
  hRoot    = DefaultRootWindow(hDisplay);

  depth  = DefaultDepth(hDisplay, hScreen);
  if (DisplayCells(hDisplay, hScreen) > MAX_CELLS) {
    fprintf(stderr, "%s: color table is too big for this program\n",
      programName);
    XCloseDisplay(hDisplay);
    exit(3);
  }

  /* sleep if asked to do so */
  if (sleepSeconds)
    sleep(sleepSeconds);

  /* grab the screen if asked to do so */
  if (grabServer)
    XGrabServer(hDisplay);



  /* get the source rectangle */
  if (!sourceId)
    sourceWindow = hRoot;
  if (sourceRoot) {
    xrect.x = xrect.y = 0;
    xrect.width  = DisplayWidth(hDisplay, hScreen);
    xrect.height = DisplayHeight(hDisplay, hScreen);
  }
  else if (sourceId || sourceWd) {
    if (sourceWd && !(sourceWindow=getWindow())) {
      fprintf(stderr, "%s: unable to find source window\n", programName);
      XCloseDisplay(hDisplay);
      exit(3);
    }
    /* get window widthXheight */
    if (!XGetGeometry(hDisplay, sourceWindow, &ignored,
         &ignored, &ignored, &width, &height, &ignored, &border)) {
      fprintf(stderr, "%s: unable to get window coordinates\n", programName);
      XCloseDisplay(hDisplay);
      exit(3);
    }
    xrect.width  = width;
    xrect.height = height;
    xrect.x      = 0;
    xrect.y      = 0;
  }
  else if (!getRectangle(&xrect)) {
    XCloseDisplay(hDisplay);
    exit(3);
  }




  /* get the image bounded by the rectangle */
  if (!noBell)
    XBell(hDisplay, 50);
  if (!getImage(&xrect, &image, sourceWindow)) {
    XCloseDisplay(hDisplay);
    exit(3);
  }

  if (grabServer)
    XUngrabServer(hDisplay);

  if (!noBell) {
    XBell(hDisplay, 20);
    XBell(hDisplay, 30);
  }
  XFlush(hDisplay);

  /* do color image processing/conversions */
  if (depth >= 2) {
    if (brighten)
      brightenColors(&image, brightenFactor);
    if (doAnd)
      alterPlanes(&image, TRUE, andBits);
    if (doOr)
      alterPlanes(&image, FALSE, orBits);

    if (forceBitmap) {
      pixmap2bitmap(&image);
      depth = 1;
    }
    else if (halftone)
      pixmap2halftone(&image, ditherKind);
    else
      compressColormap(&image);
  }


  /* open the output stream */
  if (outfileName) {
    outfile = fopen(outfileName, "w");
    if (!outfile) {
      fprintf(stderr, "%s: ", programName);
      perror(outfileName);
      exit(3);
    }
    ptr = rindex(outfileName, '.');
    if (ptr) *ptr = '\0';
    imageName = rindex(outfileName, '/');
    if (imageName) imageName++;
    else imageName = outfileName;
  }
  else
    imageName = "unnamed";   /* default for image names */


  if (psFormat) {
    if (psColor)
      writeColorPS(&image, outfile, compress, encapsulate);
    else
      writePostscript(&image, outfile, compress, encapsulate);
  }
  else if (xwdFormat)
    writeXWD(&image, outfile);
  else if (simpleFormat)
    writeSimple(&image, outfile);
  else if (puzzleFormat)
    writePuzzle(&image, outfile);
  else if (image.ximage->depth <= 1)
    writeXYPixmap(&image, outfile);
  else
    writeZPixmap(&image, outfile);

  XDestroyImage(image.ximage);
  XCloseDisplay(hDisplay);
  if (outfileName)
    fclose(outfile);

  exit(0);
}

