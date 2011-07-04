/* loadimage.c:
 *
 * generic image loader for X11
 *
 * jim frost 09.27.89
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
    static char *sccs_id = "@(#)xloadimage.c	1.3  11/8/90";
#endif

#include "copyright.h"
#include "xloadimage.h"
#include "patchlevel"

extern void init_ready();

/* options array and definitions.  note that the enum values must be in the
 * same order as the options strings.
 */

char *Options[] = {
  "onroot", /* global options */
  "border",
  "display",
  "fullscreen",
  "geometry",
  "help",
  "identify",
  "list",
  "install",
  "path",
  "quiet",
  "slideshow",
  "prog_slideshow",
  "supported",
  "verbose",
  "version",
  "view",

  "at", /* image options */
  "background",
  "brighten",
  "center",
  "clip",
  "colors",
  "dither",
  "foreground",
  "halftone",
  "name",
  "xzoom",
  "yzoom",
  "zoom",
  NULL
};

enum {

  /* global options
   */

  ONROOT= 0, BORDER, DISPLAY, FULLSCREEN, GEOMETRY, HELP, IDENTIFY, LIST,
  INSTALL, PATH, QUIET, SLIDESHOW, PROG_SLIDESHOW,
  SUPPORTED, VERBOSE, VER_NUM, VIEW,

  /* local options
   */

  AT, BACKGROUND, BRIGHT, CENTER, CLIP, COLORS, DITHER, FOREGROUND,
  HALFTONE, NAME, XZOOM, YZOOM, ZOOM
};

/* if an image loader needs to have our display and screen, it will get
 * them from here.  this is done to keep most of the image routines
 * clean
 */

Display *Disp= NULL;
int      Scrn= 0;

/* the real thing
 */

main(argc, argv)
     int argc;
     char *argv[];
{ unsigned int  onroot;
  char         *border;
  char         *dname;
  unsigned int  fullscreen;
  unsigned int  identify;
  unsigned int  install;
  unsigned int  slideshow, prog_slideshow;
  unsigned int  verbose;
  Image        *dispimage;      /* image that will be sent to the display */
  Image        *newimage;       /* new image we're loading */
  Display      *disp;           /* display we're sending to */
  int           scrn;           /* screen we're sending to */
  XColor        xcolor;         /* color for border option */
  ImageOptions  images[MAXIMAGES]; /* list of image w/ options to load */
  int           a;
  unsigned int  imagecount;     /* number of images in ImageName array */
  unsigned int  winx, winy;     /* location of window */
  unsigned int  winwidth, winheight; /* geometry of window */

  if (argc < 2)
    usage(argv[0]);

  /* defaults and other initial settings.  some of these depend on what
   * our name was when invoked.
   */

  loadPathsAndExts();
  onroot= 0;
  verbose= 1;
  if (!strcmp(tail(argv[0]), "xview")) {
    onroot= 0;
    verbose= 1;
  }
  else if (!strcmp(tail(argv[0]), "xsetbg")) {
    onroot= 1;
    verbose= 0;
  }
  border= NULL;
  dname= NULL;
  fullscreen= 0;
  identify= 0;
  install= 0;
  slideshow= prog_slideshow= 0;
  winx= winy= winwidth= winheight= 0;

  imagecount= 0;
  for (a= 0; a < MAXIMAGES; a++) {
    images[a].name= NULL;
    images[a].atx= images[a].aty= 0;
    images[a].bright= 0;
    images[a].center= 0;
    images[a].clipx= images[a].clipy= 0;
    images[a].clipw= images[a].cliph= 0;
    images[a].dither= 0;
    images[a].colors= 0;
    images[a].fg= images[a].bg= NULL;
    images[a].xzoom= images[a].yzoom= 0;
  }
  for (a= 1; a < argc; a++) {
    switch (optionNumber(argv[a], Options)) {
    case OPT_BADOPT:
      printf("%s: Bad option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */

    case OPT_NOTOPT:
      if (imagecount == MAXIMAGES)
	printf("%s: Too many images (ignoring)\n", argv[++a]);
      else
	images[imagecount++].name= argv[a];
      break;

    case OPT_SHORTOPT:
      printf("%s: Not enough characters to identify option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */

    /* process options global to everything
     */

    case ONROOT:
      onroot= 1;
      break;

    case BORDER:
      border= argv[++a];
      break;

    case DISPLAY:
      dname= argv[++a];
      break;

    case FULLSCREEN:
      fullscreen= 1;
      break;

    case GEOMETRY:
      XParseGeometry(argv[++a], &winx, &winy, &winwidth, &winheight);
      break;

    case HELP:
      usage(argv[0]);
      exit(0);

    case IDENTIFY:
      identify= 1;
      break;

    case LIST:
      listImages();
      exit(0);

    case INSTALL:
      install= 1;
      break;

    case PATH:
      showPath();
      break;

    case QUIET:
      verbose= 0;
      break;

    case SLIDESHOW:
      slideshow= 1;
      break;

    case PROG_SLIDESHOW:
      prog_slideshow= slideshow= 1;
      break;

    case SUPPORTED:
      supportedImageTypes();
      break;

    case VERBOSE:
      verbose= 1;
      break;

    case VER_NUM:
      printf("Xloadimage version %s patchlevel %s by Jim Frost\n",
	     VERSION, PATCHLEVEL);
      break;

    case VIEW:
      onroot= 0;
      break;

    /* process options local to an image
     */

    case AT:
      if (sscanf(argv[++a], "%d,%d",
		 &images[imagecount].atx, &images[imagecount].aty) != 2) {
	printf("Bad argument to -at\n");
	usage(argv[0]);
	/* NOTREACHED */
      }
      break;

    case BACKGROUND:
      images[imagecount].bg= argv[++a];
      break;

    case BRIGHT:
      images[imagecount].bright= atoi(argv[++a]);
      break;

    case CENTER:
      images[imagecount].center= 1;
      break;

    case CLIP:
      if (sscanf(argv[++a], "%d,%d,%d,%d",
		 &images[imagecount].clipx, &images[imagecount].clipy,
		 &images[imagecount].clipw, &images[imagecount].cliph) != 4) {
	printf("Bad argument to -clip\n");
	usage(argv[0]);
	/* NOTREACHED */
      }
      break;

    case COLORS:
      images[imagecount].colors= atoi(argv[++a]);
      if (images[imagecount].colors < 2) {
	printf("Argument to -colors is too low (ignored)\n");
	images[imagecount].colors= 0;
      }
      else if (images[imagecount].colors > 65536) {
	printf("Argument to -colors is too high (ignored)\n");
	images[imagecount].colors= 0;
      }
      break;

    case DITHER:
      images[imagecount].dither= 1;
      break;

    case FOREGROUND:
      images[imagecount].fg= argv[++a];
      break;

    case HALFTONE:
      images[imagecount].dither= 2;
      break;

    case NAME:
      if (imagecount == MAXIMAGES)
	printf("%s: Too many images (ignoring)\n", argv[++a]);
      else
	images[imagecount++].name= argv[++a];
      break;

    case XZOOM:
      images[imagecount].xzoom= atoi(argv[++a]);
      break;

    case YZOOM:
      images[imagecount].yzoom= atoi(argv[++a]);
      break;

    case ZOOM:
      images[imagecount].xzoom= images[imagecount].yzoom= atoi(argv[++a]);
      break;

    default:

      /* this should not happen!
       */

      printf("%s: Internal error parsing arguments\n", argv[0]);
      exit(1);
    }
  }

  if (!imagecount) /* NO-OP from here on */
    exit(0);

  if (identify) {                    /* identify the named image(s) */
    for (a= 0; a < imagecount; a++)
      identifyImage(images[a].name);
    exit(0);
  }

  /* filter out mutually exclusive flags
   */

  if (onroot && slideshow) {
    printf("\
%s: -onroot and -slideshow are mutually exclusive (-onroot ignored)\n",
	   argv[0]);
    onroot= 0;
  }

  /* start talking to the display
   */

  if (! (Disp= disp= XOpenDisplay(dname))) {
    printf("%s: Cannot open display\n", XDisplayName(dname));
    exit(1);
  }
  Scrn= scrn= DefaultScreen(disp);
  XSetIOErrorHandler(ioErrorHandler);

  init_ready(disp, scrn);

  dispimage= NULL;
  if (onroot && (winwidth || winheight || images[0].center ||
      images[0].atx || images[0].aty)) {
    if (!winwidth)
	winwidth= DisplayWidth(disp, scrn);
    if (!winheight)
      winheight= DisplayHeight(disp, scrn);
    if (DefaultDepth(disp, scrn) == 1)
      dispimage= newBitImage(winwidth, winheight);
    else {
      dispimage= newRGBImage(winwidth, winheight, DefaultDepth(disp, scrn));
      dispimage->rgb.used= 1;
    }
    *(dispimage->rgb.red)= 65535;   /* default border value is white */
    *(dispimage->rgb.green)= 65535;
    *(dispimage->rgb.blue)= 65535;
    if (border) {
      XParseColor(disp, DefaultColormap(disp, scrn), border, &xcolor);
      *dispimage->rgb.red= xcolor.red;
      *dispimage->rgb.green= xcolor.green;
      *dispimage->rgb.blue= xcolor.blue;
    }

    /* bitmap needs both black and white
     */

    if (DefaultDepth(disp, scrn) == 1) {
	if (*(dispimage->rgb.red)) {
	    *(dispimage->rgb.red + 1)= 0;
	    *(dispimage->rgb.green + 1)= 0;
	    *(dispimage->rgb.blue + 1)= 0;
	}
	else {
	    *(dispimage->rgb.red + 1)= 65535;
	    *(dispimage->rgb.green + 1)= 65535;
	    *(dispimage->rgb.blue + 1)= 65535;
	}
    }
    fill(dispimage, 0, 0, winwidth, winheight, 0);
  }

  if (prog_slideshow)
  {
  /* initial empty frame */
    dispimage = newBitImage((unsigned) 1, (unsigned) 1);
    dispimage->title = "slideshow";
    switch (imageInWindow(disp, scrn, dispimage,
			  0, 0, 1, 1, 1, install, 1, 1, 0))
    {
    case '\0': /* window got nuked by someone */
	XCloseDisplay(disp);
	exit(1);
    case '\003':
    case 'q':  /* quit */
	XCloseDisplay(disp);
	exit(0);
    case 'n':  /* next image */
	break;
    }
    freeImage(dispimage);
    dispimage= NULL;
  }

  /* load in each named image
   */

  for (a= 0; a < imagecount; a++) {
    if (! (newimage= loadImage(images[a].name, verbose)))
      continue;
    if (!images[a].dither &&
	((dispimage && BITMAPP(dispimage)) || (DefaultDepth(disp, scrn) == 1)))
      images[a].dither= 2;
    newimage= processImage(disp, scrn, newimage, &images[a], verbose);
    if (images[a].center && dispimage) {
      images[a].atx= (int)(dispimage->width - newimage->width) / 2;
      images[a].aty= (int)(dispimage->height - newimage->height) / 2;
    }
    if (dispimage) {
      if (! dispimage->title)
	dispimage->title= dupString(newimage->title);
      merge(dispimage, newimage, images[a].atx, images[a].aty, verbose);
      freeImage(newimage);
    }
    else
      dispimage= newimage;
    if (slideshow) {
      switch(imageInWindow(disp, scrn, dispimage, winx, winy,
			   winwidth, winheight,
			   fullscreen, install, slideshow, prog_slideshow,
			   verbose)) {
      case '\0': /* window got nuked by someone */
	XCloseDisplay(disp);
	exit(1);
      case '\003':
      case 'q':  /* user quit */
	XCloseDisplay(disp);
	exit(0);
	
      case 'n':  /* next image */
	break;
      case 'p':  /* previous image */
	if (a > 0)
	  a -= 2;
	else
	  a--;
	break;
      }
      freeImage(dispimage);
      dispimage= NULL;
    }
  }

  if (!slideshow && !dispimage) {
    printf("No images to display\n");
    exit(1);
  }

  if (onroot)
    imageOnRoot(disp, scrn, dispimage, verbose);
  else {
    if (!slideshow)
      imageInWindow(disp, scrn, dispimage, winx, winy, winwidth, winheight,
		    fullscreen, install, slideshow, prog_slideshow, verbose);
    cleanUpWindow(disp);
  }
  XCloseDisplay(disp);
}
