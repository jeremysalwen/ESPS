/* compress.c:
 *
 * compress a colormap by removing unused RGB colors
 *
 * jim frost 10.05.89
 *
 * Copyright 1989, 1990 Jim Frost.  See included file "copyright.h" for
 * complete copyright information.
 */

/*
 * Hack to speed up the special case  image->pixlen == 1  added by
 * Rod Johnson, Entropic Research Laboratory, Inc., 90/09/25.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)compress.c	1.2  10/14/90";
#endif

#include "copyright.h"
#include "image.h"

void compress(image, verbose)
     Image        *image;
     unsigned int  verbose;
{ Pixel        *index;
  unsigned int *used;  
  RGBMap        rgb;
  byte         *pixptr;
  unsigned int  a, x, y;
  Pixel         color;

  goodImage(image, "compress");
  if (! RGBP(image)) /* we're AT&T */
    return;

  if (verbose) {
    printf("  Compressing colormap...");
    fflush(stdout);
  }

  newRGBMapData(&rgb, image->rgb.size);
  index= (Pixel *)lmalloc(sizeof(Pixel) * image->rgb.used);
  used= (unsigned int *)lmalloc(sizeof(unsigned int) * image->rgb.used);
  for (x= 0; x < image->rgb.used; x++)
    *(used + x)= 0;

#define DO_A_COLOR \
	if (*(used + color) == 0) {					\
	  for (a= 0; a < rgb.used; a++)					\
	    if ((*(rgb.red + a) == *(image->rgb.red + color)) &&	\
	        (*(rgb.green + a) == *(image->rgb.green + color)) &&	\
	        (*(rgb.blue + a) == *(image->rgb.blue + color)))	\
	      break;							\
	  *(index + color)= a;						\
	  *(used + color)= 1;						\
	  if (a == rgb.used) {						\
	    *(rgb.red + a)= *(image->rgb.red + color);			\
	    *(rgb.green + a)= *(image->rgb.green + color);		\
	    *(rgb.blue + a)= *(image->rgb.blue + color);		\
	    rgb.used++;							\
	  }								\
	}

  /*
   * When  image->pixlen == 1  the functions  memToVal  and  valToMem  can
   * be replaces with casts.
   */

  pixptr= image->data;
  if (image->pixlen == 1) {
    for (y= 0; y < image->height; y++)
      for (x= 0; x < image->width; x++) {
	color = (unsigned long) *pixptr;
        DO_A_COLOR
	*pixptr = (byte) *(index + color);
        pixptr += image->pixlen;
      }
  }
  else {
    for (y= 0; y < image->height; y++)
      for (x= 0; x < image->width; x++) {
        color= memToVal(pixptr, image->pixlen);
        DO_A_COLOR
        valToMem(*(index + color), pixptr, image->pixlen);
        pixptr += image->pixlen;
      }
  }

#undef DO_A_COLOR

  if (verbose)
    if (rgb.used < image->rgb.used)
      printf("%d unique colors of %d\n", rgb.used, image->rgb.used);
    else
      printf("no improvement\n");

  freeRGBMapData(&(image->rgb));
  image->rgb= rgb;
}
