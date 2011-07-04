/* imagetypes.h:
 *
 * supported image types and the imagetypes array declaration.  when you
 * add a new image type, only the makefile and this header need to be
 * changed.
 *
 * jim frost 10.15.89
 */

/* @(#)imagetypes.h	1.1  10/13/90 */

Image *facesLoad();
Image *pbmLoad();
Image *sunRasterLoad();
Image *gifLoad();
Image *xbitmapLoad();
Image *xpixmapLoad();
Image *g3Load();

int facesIdent();
int pbmIdent();
int sunRasterIdent();
int gifIdent();
int xbitmapIdent();
int xpixmapIdent();
int g3Ident();

/* some of these are order-dependent
 */

struct {
  int    (*identifier)(); /* print out image info if this kind of image */
  Image *(*loader)();     /* load image if this kind of image */
  char  *name;            /* name of this image format */
} ImageTypes[] = {
  sunRasterIdent, sunRasterLoad, "Sun Rasterfile",
  pbmIdent,       pbmLoad,       "Portable Bit Map (PBM)",
  facesIdent,     facesLoad,     "Faces Project",
  gifIdent,       gifLoad,       "GIF Image",
  xpixmapIdent,   xpixmapLoad,   "X Pixmap",
  xbitmapIdent,   xbitmapLoad,   "X Bitmap",
  g3Ident,        g3Load,        "G3 FAX Image",
  NULL,           NULL,          NULL
};
