/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:  John Shore
 *
 * Brief description:
 *
 *   functions for popping up buttons that invoke programs on the named file
 */

static char *sccs_id = "@(#)ebanner.c	1.5	07 Feb 1996	ERL";

/*
 * system include files
 */

#include <stdio.h>
#include <math.h>
#include <sys/file.h>
#if !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif
#include <esps/esps.h>
#include <esps/feasd.h>
#ifdef OS5
#include <sys/fcntl.h>
#endif

/*global declarations for default font*/

float sf = 8000.0, low = .1, hi = .9, pdur = .02, tstep = .01,
  ampl = 500.0;


int height = 20, width = 20, obufl, ind = 0, psize = 0, pstep,
    sam_per_char;

float **pulses = NULL;
short *obuf = NULL;

char **pixmap = NULL, *font_bitmap = NULL;

char *font_name = "cour.r.24";

int  debug_level=0;

init_arrays()
{
  register int i, j;

  if( ! pulses ) {
    register double fincr,  arg, freq;
    float *wind, lowf = low*sf*0.5, hif = hi*sf*0.5;

    pulses = (float**)malloc(sizeof(float*)*height);
    psize = 0.5 + (sf * pdur);
    pstep = 0.5 + (tstep * sf);
    ind = 0;

    /* Create a window for the pulses. */
    wind = (float*)malloc(sizeof(float) * psize);
    for(i=0, arg = 3.1415927 * 2.0/(psize-1); i < psize; i++)
      wind[i] = ampl * (.5 - (0.5 * cos(arg*i)));

    /* Now generate the pulses. */
    for(i=0, freq = hif, fincr = (hif - lowf)/height;
        i < height; i++, freq -= fincr) {
      pulses[i] = (float*)malloc(sizeof(float) * psize);
      arg = (freq * 3.1415927 * 2.0)/sf;
      if(i & 1)
	for(j=0; j < psize; j++)
	  pulses[i][j] = wind[j] * sin(arg*j);
      else
	for(j=0; j < psize; j++)
	  pulses[i][j] = wind[j] * cos(arg*j);
    }
  }
  if( ! obuf ) {
    obufl = (pstep * width) + psize;
    obuf = (short*)calloc(obufl, sizeof(short));
  }
  sam_per_char = obufl;
  reset_out_array();
}

syntax(prog)
     char *prog;
{
  fprintf(stderr,"Usage: %s [-f font] [-a amplitude ] [-l low] [-h high] [-d duration]\n\t[-s step] [-r sample_rate] [-x debug_level] input.text output.sd\n", prog);
}

main(ac, av)
int ac;
char **av;
{
  FILE *in, *out;
  char inarray[500], *cp;
  int ccount, c;
  extern int optind;
  extern char *optarg;

  if(ac < 3) {
    syntax(av[0]);
    exit(-1);
  }

    while((c = getopt(ac,av,"f:l:h:d:s:r:l:x:a:")) != EOF) {
    switch(c) {
    case 'f':
      font_name = optarg;
      break;
    case 'l':
      low = atof(optarg);
      break;
    case 'h':
      hi = atof(optarg);
      break;
    case 'd':
      pdur = atof(optarg);
      break;
    case 's':
      tstep = atof(optarg);
      break;
    case 'r':
      sf = atof(optarg);
      break;
    case 'a':
      ampl = atof(optarg);
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    default:
      syntax(av[0]);
      exit(-1);
    }
  }

  if((in = fopen(av[optind],"r")) && (out = fopen(av[optind+1],"w"))) {
    
    char *lname = 
      find_esps_file(NULL, font_name, ".:$ESPS_BASE/lib/fixedwidthfonts", "EBANNER_FONTS");

    if (lname == NULL) 
    {
      printf("Couldn't find font file (%s).\n",font_name);
      exit(-1);
    }      

    set_font_dims(lname);
    init_arrays();
    setup_esps_file(out);
    while( (cp = fgets(inarray,500,in)) ) {

      while(*cp) {
        get_pixmap((int)(*cp));
        for(ccount=0; ccount < width; ccount++)
          do_a_column(ccount);
        if(write_esps_output(sam_per_char,out) <= 0) {
          fprintf(stderr,"Output problems\n");
          exit(-1);
        }
        reset_out_array();
        cp++ ;
      }
    }
  } else {
    fprintf(stderr,"Problems opening %s for input and/or %s for output\n",
            av[optind],av[optind+1]);
    exit(-1);
  }
  fclose(in);
  fclose(out);
  exit(0);
}

struct header *oh = NULL;
struct feasd *sd_rec = NULL;

setup_esps_file(ostr)
     FILE *ostr;
{
  double start_time = 0.0, maxv;
  maxv = height * ampl;

  oh = new_header(FT_FEA);
  init_feasd_hd(oh, SHORT, 1, &start_time, 0 , (double)sf);
  *add_genhd_d("max_value", (double*)NULL, 1, oh) = maxv;
  write_header(oh, ostr);
  sd_rec = allo_feasd_recs(oh, SHORT, obufl, obuf, YES);

}

write_esps_output(n, str)
     int n;
     FILE *str;
{
  put_feasd_recs(sd_rec, 0L, n, oh, str);
  return(1);
}

reset_out_array()
{
  register int i;
  for(i=0 ; i < obufl; i++)
    obuf[i] = 1;
  ind = 0;
}

/*********************************************************************/
do_a_column(c)
int c;
{
  register int r, i;
  register float *p1;
    short *p2;
  
  for(r=0 ; r < height; r++) {
    if(pixmap[c][r]) {
      for(p1 = pulses[r], i = psize, p2 = obuf+ind; i--; )
	*p2++ += *p1++;
    }
  }
  ind += pstep;
}

typedef struct fheader {
  short           magic;	/* Magic number VFONT_MAGIC */
  unsigned short  size;		/* Total # bytes of bitmaps */
  short           maxx;		/* Maximum horizontal glyph size */
  short           maxy;		/* Maximum vertical glyph size */
  short           xtend;	/* (unused) */
} VHEADER;

typedef struct dispatch {
  unsigned short  addr;		/* &(glyph) - &(start of bitmaps) */
  short           nbytes;	/* # bytes of glyphs (0 if no glyph) */
  char            up, down, left, right; /* Widths from baseline point */
  short           width;	/* Logical width, used by troff */
} VDISPATCH;

#define NUM_DISPATCH    256
#define VFONT_MAGIC     0436

VHEADER head;
VDISPATCH dblocks[NUM_DISPATCH];

set_font_dims(name)
     char *name;
{
  unsigned char *p, *q, *r, *s, *rep, *addr;
  register unsigned char ibyte, ibit, obyte, obit;
  char *bout;
  int fdi, fdo, hsize, dsize, bits, c;
  register int i, j, k, l, m, n, o, sum;

  
  /* open input font file */
  if((fdi = open(name,O_RDONLY)) >= 0) {

    /* read in the header block and dblocks */
    if(((hsize = read(fdi,(char*)&head,sizeof(VHEADER))) == sizeof(VHEADER)) &&
       ((dsize = read(fdi,(char*)dblocks,sizeof(VDISPATCH)*NUM_DISPATCH)) ==
	sizeof(VDISPATCH)*NUM_DISPATCH)) {
#ifdef INTEL_ONLY
      /* Swap bytes where necessary (when reading the old font files
         on an intel-byte-ordered machine) */
      {
	int cnt = 5, cnt2;
	char *c1=(char*)&head, *c2=c1+1, chold;
	for(; cnt > 0; cnt--, c1 += 2, c2 += 2) {
	  chold = *c1;
	  *c1 = *c2;
	  *c2 = chold;
	}
	for(cnt=0; cnt < NUM_DISPATCH; cnt++) {
	  c1 = ((char*)dblocks) + (cnt * sizeof(VDISPATCH));
	  c2 = c1 + 1;
	  for(cnt2=0; cnt2 < 5; cnt2++, c1 += 2, c2 += 2) {
	    chold = *c1;
	    *c1 = *c2;
	    *c2 = chold;
	  }
	}
      }
#endif
      /* determine the total file size */

      if(debug_level) {
	printf("File size:%d; fheader:%d dblocks:%d\n",k+hsize+dsize,hsize,dsize);
	printf("magic:%d size:%d maxx:%d maxy:%d\n",
	       head.magic,head.size,head.maxx,head.maxy);
	for(i=0;i<NUM_DISPATCH;i++)
	  printf("addr:%d nbytes:%d up:%d down:%d left:%d right:%d width:%d\n",
		 dblocks[i].addr,dblocks[i].nbytes,dblocks[i].up,dblocks[i].down,
		 dblocks[i].left,dblocks[i].right,dblocks[i].width);
      }

      height = head.maxy;
      width = head.maxx;

      for(i=0;i<NUM_DISPATCH;i++) {
	if((k = dblocks[i].up + dblocks[i].down) > height)
	  height = k;
	if((k = dblocks[i].left + dblocks[i].right) > width)
	  width = k;
      }

      /* allocate input font bitmap arrays */
      lseek(fdi,hsize+dsize,SEEK_SET);
      if((font_bitmap = (char*)malloc(head.size))) {
	/* read the input font */
	if(read(fdi,font_bitmap,head.size) != head.size) {
	  printf("Problems reading input font bitmap array\n");
	  exit(-1);
	}
	return(TRUE);
      } else
	printf("Allocation failure for input font\n");
    } else
      printf("Problems reading header information for input font\n");
  } else
    printf("Problems opening font (%s) file\n",name);
  return(FALSE);
}

get_pixmap(c)
     int c;
{
  unsigned char *p;
  register unsigned char ibyte, ibit;
  register int k, l, m;

  if(c) {
    register char *cp;
    register int i, j;

    if( ! pixmap ) {
      pixmap = (char**)malloc(sizeof(char*) * width);
      for(i=0 ; i < width; i++)
        pixmap[i] = malloc(height);
    }

    /* Initialize the whole matrix to zero. */
    for(i=0 ; i < width; i++)
      for(j=height, cp = pixmap[i]; j--; )*cp++ = 0;

    
    i=c;
    p = (unsigned char*)(font_bitmap + dblocks[i].addr);
    if(dblocks[i].nbytes > 0) {
      for(j=dblocks[i].right+dblocks[i].left,
	  j = (j > width)? width : j,
	  k = 0, m=dblocks[i].up + dblocks[i].down,
	  m = (m > height)? height : m; k < m; k++) {
	for(ibyte = *p++, l = 0, ibit = 1<<7;  l < j; l++) {
	  if(ibit&ibyte)
	    pixmap[l][k] = 1;
	  ibit = ibit >> 1;
	  if(!ibit) {
	    ibit = 1<<7;
	    ibyte = *p++;
	  }
	}
      }
    }
  }
}

