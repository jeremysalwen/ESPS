/* dither.c
 *
 * completely reworked dithering module for xloadimage
 * uses error-diffusion dithering (floyd-steinberg) instead
 * of simple 4x4 ordered-dither that was previously used
 *
 * the previous version of this code was written by steve losen
 * (scl@virginia.edu)
 * 
 * jim frost    07.10.89
 * Steve Losen  11.17.89
 * kirk johnson 06.04.90
 *
 * Copyright 1990 Kirk L. Johnson (see the included file
 * "kljcpyrght.h" for complete copyright information)
 *
 * Copyright 1989, 1990 Jim Frost and Steve Losen.  See included file
 * "copyright.h" for complete copyright information.
 */

#ifdef SCCS
    static char *sccs_id = "@(#)dither.c	1.1  10/13/90";
#endif

#include "copyright.h"
#include "kljcpyrght.h"
#include "image.h"

#define RedPercent  0.299
#define GrnPercent  0.587	/* color -> grey conversion parameters */
#define BluPercent  0.114

#define MaxIntensity  65536	/* maximum possible Intensity */

#define MaxGrey       32768	/* limits on the grey levels used */
#define Threshold     16384	/* in the dithering process */
#define MinGrey           0

static unsigned int tone_scale_adjust();
static void         LeftToRight();
static void         RightToLeft();


/*
 * simple floyd-steinberg dither with serpentine raster processing
 */

Image *dither(cimage, verbose)
     Image        *cimage;
     unsigned int  verbose;
{
  Image          *image;	/* destination image */
  unsigned int   *grey;		/* grey map for source image */
  double          tmp;		/* work space */
  unsigned int    spl;		/* source pixel length in bytes */
  unsigned int    dll;		/* destination line length in bytes */
  unsigned char  *src;		/* source data */
  unsigned char  *dst;		/* destination data */
  int            *curr;		/* current line buffer */
  int            *next;		/* next line buffer */
  int            *swap;		/* for swapping line buffers */
  Pixel           color;	/* pixel color */
  unsigned int    level;	/* grey level */
  unsigned int    i, j;		/* loop counters */

  /*
   * check the source image
   */
  goodImage(cimage, "dither");
  if (! RGBP(cimage))
    return(NULL);

  /*
   * allocate destination image
   */
  if (verbose)
  {
    printf("  Dithering image...");
    fflush(stdout);
  }
  image = newBitImage(cimage->width, cimage->height);
  if (cimage->title)
  {
    image->title = (char *) malloc(strlen(cimage->title) + 12);
    sprintf(image->title, "%s (dithered)", cimage->title);
  }

  /*
   * if the number of entries in the colormap isn't too large, compute
   * the grey level for each entry and store it in grey[]. else the
   * grey levels will be computed on the fly.
   */
  if (cimage->depth <= 16)
  {
    grey = (unsigned int *) malloc(sizeof(unsigned int) * cimage->rgb.used);
    for (i=0; i<cimage->rgb.used; i++)
    {
      tmp  = (cimage->rgb.red[i]   * RedPercent);
      tmp += (cimage->rgb.green[i] * GrnPercent);
      tmp += (cimage->rgb.blue[i]  * BluPercent);

      grey[i] = (tmp / MaxIntensity) * MaxGrey;
    }

    for (i=0; i<cimage->rgb.used; i++)
      grey[i] = tone_scale_adjust(grey[i]);
  }
  else
  {
    grey = NULL;
  }

  /*
   * dither setup
   */
  spl = cimage->pixlen;
  dll = (image->width / 8) + (image->width % 8 ? 1 : 0);
  src = cimage->data;
  dst = image->data;

  curr  = (int *) malloc(sizeof(int) * (cimage->width + 2));
  next  = (int *) malloc(sizeof(int) * (cimage->width + 2));
  curr += 1;
  next += 1;
  for (j=0; j<cimage->width; j++)
  {
    curr[j] = 0;
    next[j] = 0;
  }

  /*
   * primary dither loop
   */
  for (i=0; i<cimage->height; i++)
  {
    /* copy the row into the current line */
    for (j=0; j<cimage->width; j++)
    {
      color = memToVal(src, spl);
      src  += spl;
      
      if (grey == NULL)
      {
	tmp  = (cimage->rgb.red[color]   * RedPercent);
	tmp += (cimage->rgb.green[color] * GrnPercent);
	tmp += (cimage->rgb.blue[color]  * BluPercent);

	level = tone_scale_adjust((tmp / MaxIntensity) * MaxGrey);
      }
      else
      {
	level = grey[color];
      }

      curr[j] += level;
    }

    /* dither the current line */
    if (i & 0x01)
      RightToLeft(curr, next, cimage->width);
    else
      LeftToRight(curr, next, cimage->width);

    /* copy the dithered line to the destination image */
    for (j=0; j<cimage->width; j++)
      if (curr[j] < Threshold)
	dst[j / 8] |= 1 << (7 - (j & 7));
    dst += dll;
    
    /* circulate the line buffers */
    swap = curr;
    curr = next;
    next = swap;
    for (j=0; j<cimage->width; j++)
      next[j] = 0;
  }

  /*
   * clean up
   */
  free(grey);
  free(curr-1);
  free(next-1);
  if (verbose)
    printf("done\n");
  
  return(image);
}


/*
 * a _very_ simple tone scale adjustment routine. provides a piecewise
 * linear mapping according to the following:
 *
 *      input:          output:
 *     0 (MinGrey)    0 (MinGrey)
 *     Threshold      Threshold/2
 *     MaxGrey        MaxGrey
 * 
 * this should help things look a bit better on most displays.
 */
static unsigned int tone_scale_adjust(val)
     unsigned int val;
{
  unsigned int rslt;
  
  if (val < Threshold)
    rslt = val / 2;
  else
    rslt = (((val - Threshold) * (MaxGrey-(Threshold/2))) /
	    (MaxGrey-Threshold)) + (Threshold/2);

  return rslt;
}


/*
 * dither a line from left to right
 */
static void LeftToRight(curr, next, width)
     int *curr;
     int *next;
     int  width;
{
  int idx;
  int error;
  int output;

  for (idx=0; idx<width; idx++)
  {
    output       = (curr[idx] > Threshold) ? MaxGrey : MinGrey;
    error        = curr[idx] - output;
    curr[idx]    = output;
    next[idx-1] += error * 3 / 16;
    next[idx]   += error * 5 / 16;
    next[idx+1] += error * 1 / 16;
    curr[idx+1] += error * 7 / 16;
  }
}


/*
 * dither a line from right to left
 */
static void RightToLeft(curr, next, width)
     int *curr;
     int *next;
     int  width;
{
  int idx;
  int error;
  int output;

  for (idx=(width-1); idx>=0; idx--)
  {
    output       = (curr[idx] > Threshold) ? MaxGrey : MinGrey;
    error        = curr[idx] - output;
    curr[idx]    = output;
    next[idx+1] += error * 3 / 16;
    next[idx]   += error * 5 / 16;
    next[idx-1] += error * 1 / 16;
    curr[idx-1] += error * 7 / 16;
  }
}
