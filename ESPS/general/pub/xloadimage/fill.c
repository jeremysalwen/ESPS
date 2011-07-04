/* fill.c:
 *
 * fill an image area with a particular pixel value
 *
 * jim frost 10.02.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)fill.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "image.h"

void fill(image, fx, fy, fw, fh, pixval)
     Image        *image;
     unsigned int  fx, fy, fw, fh;
     Pixel         pixval;
{ unsigned int  x, y;
  unsigned int  linelen, start;
  byte         *lineptr, *pixptr;
  byte          startmask, mask;

  goodImage(image);
  switch(image->type) {
  case IBITMAP:

    /* this could be made a lot faster
     */

    linelen= (image->width / 8) + (image->width % 8 ? 1 : 0);
    lineptr= image->data + (linelen * fy);
    start= (fx / 8) + (fx % 8 ? 1 : 0);
    startmask= 0x80 >> (fx % 8);
    for (y= fy; y < fy + fh; y++) {
      mask= startmask;
      pixptr= lineptr + start;
      for (x= fx; x < fw; x++) {
	if (pixval)
	  *pixptr |= mask;
	else
	  *pixptr &= ~mask;
	if (mask >>= 1) {
	  mask= 0x80;
	  pixptr++;
	}
      }
      lineptr += linelen;
    }
    break;

  case IRGB:
    linelen= image->width * image->pixlen;
    start= image->pixlen * fx;
    lineptr= image->data + (linelen * fy);
    for (y= fy; y < fy + fh; y++) {
      pixptr= lineptr + start;
      for (x= fx; x < fw; x++) {
	valToMem(pixval, pixptr, image->pixlen);
	pixptr += image->pixlen;
      }
      lineptr += linelen;
    }
    break;
  default:
    printf("fill: Unsupported image type (ignored)\n");
    return;
  }
}
