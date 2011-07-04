/* bright.c
 *
 * alter an image's brightness by a given percentage
 *
 * jim frost 10.10.89
 *
 * Copyright 1989 Jim Frost.  See included file "copyright.h" for complete
 * copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)bright.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "image.h"

void brighten(image, percent, verbose)
     Image        *image;
     unsigned int  percent;
     unsigned int  verbose;
{ int          a;
  unsigned int newrgb;
  float        fperc;

  if (! RGBP(image)) /* we're AT&T */
    return;

  if (verbose) {
    printf("  Brightening colormap by %d%%...", percent);
    fflush(stdout);
  }

  fperc= (float)percent / 100.0;
  for (a= 0; a < image->rgb.used; a++) {
    newrgb= *(image->rgb.red + a) * fperc;
    if (newrgb > 65535)
      newrgb= 65535;
    *(image->rgb.red + a)= newrgb;
    newrgb= *(image->rgb.green + a) * fperc;
    if (newrgb > 65535)
      newrgb= 65535;
    *(image->rgb.green + a)= newrgb;
    newrgb= *(image->rgb.blue + a) * fperc;
    if (newrgb > 65535)
      newrgb= 65535;
    *(image->rgb.blue + a)= newrgb;
  }

  if (verbose)
    printf("done\n");
}
