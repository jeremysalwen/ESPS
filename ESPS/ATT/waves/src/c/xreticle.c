/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * reticle.c
 * procedures to plot reticles and numerics
 */

static char *sccs_id = "@(#)xreticle.c	1.10	9/28/98	ATT/ERL";


/* For spectrograms, scalar signals, Cartesian plots (linear axes):
Ordinate intervals (marker intervals, marker lengths)
    major
    minor 1
    minor 2
    numbering interval
    numbering precision
    numbering location
    marking style
Abscissa intervals (marker intervals, marker lengths)
    major
    minor 1
    minor 2
    numbering interval
    numbering precision
    numbering location
    marking style
Image boundaries (in pixels; 0,0 is upper left)
    top (location of highest ordinate value)
    left (location of lowest abscissa value)
    bottom
    right
Color
Line type
Font
Ordinate range
    start
    end
Abscissa range
    start
    end
Ordinate label
Abscissa label

******************************************************************/

#include <stdio.h>
#include <esps/esps.h>
#include <Objects.h>

#include <xview/font.h>

#define oneormore(x) (((y = (int)(x)) >= 1)? y : 1)



/* char default_font[] = "/usr/lib/fonts/fixedwidthfonts/serif.r.12"; */
char default_x_precision[] = "%6.1lf";
char default_y_precision[] = "%6.0lf";

extern char *savestring();

static double	myfmod();
static void	do_x_numbers(), do_x_label(), do_y_numbers(), central_marks();
static int	*get_ord_marklist(), *get_absc_marklist();
static void	arb_spect_view_int(), lin_spect_view_int();

extern int debug_level;
extern Xv_Font def_font;
extern int     def_font_height, def_font_width;


/******************************************************************/
Bound *
reticle_get_margins(r)
     Reticle *r;
{
  static Bound b;
  int height, width, maxl, i;
  char tmp[100], *p;

  if(r) {
    b.top = b.bottom = b.right = b.left = 1;

    if (!r->font) r->font = def_font;
    height = def_font_height;
    width = def_font_width;

    if (debug_level >= 2)
	fprintf(stderr, "%s: font %x width %d\n", "reticle_get_margins",
		r->font, width);

    if(r->ordinate.num_loc & NUM_BOTH) {
      sprintf(tmp,r->ordinate.precision,r->ordi.start);
      p = tmp;
      while(*p == ' ') p++;
      maxl = strlen(p);
      sprintf(tmp,r->ordinate.precision,r->ordi.end);
      p = tmp;
      while(*p == ' ') p++;
      if(strlen(p) > maxl) maxl = strlen(p);
      if(r->ordinate.num_loc & NUM_LB)
	b.left += ((1 + maxl) * width);
      if(r->ordinate.num_loc & NUM_RT)
	b.right += ((1 + maxl) * width);
    }
    if(r->abscissa.num_loc & NUM_LB) {
      b.bottom += (height + 1);
    } else if(r->ordinate.num_loc & NUM_BOTH) b.bottom += 1 + (height/2);
    if(r->abscissa.num_loc & NUM_RT) {
      b.top += (height + 1);
    } else if(r->ordinate.num_loc & NUM_BOTH) b.top += 1 + (height/2);
    if(r->abs_label) b.bottom += (height + 1);
    return(&b);
  }
  return(NULL);
}
    
/******************************************************************/
Reticle *
new_spectrogram_reticle()
{
  Reticle *r;

  if(!(r = (Reticle*)malloc(sizeof(Reticle)))) return(NULL);

  r->ordinate.maj.style =  MAJOR | EDGES;
  r->ordinate.maj.length = .020;
  r->ordinate.maj.inter = 1000.0;
  r->ordinate.maj.list = NULL;
  r->ordinate.maj.num = 0;
  r->ordinate.min1.style =  EDGES;
  r->ordinate.min1.length = .010;
  r->ordinate.min1.inter = 500.0;
  r->ordinate.min1.list = NULL;
  r->ordinate.min1.num = 0;
  r->ordinate.min2.style = EDGES;
  r->ordinate.min2.length = .005;
  r->ordinate.min2.inter = 100.0;
  r->ordinate.min2.list = NULL;
  r->ordinate.min2.num = 0;
  r->ordinate.precision = savestring(default_y_precision);
  r->ordinate.num_inter = 1000.0;
  r->ordinate.num_loc = NUM_LB;
  r->abscissa.maj.style = MAJOR | EDGES;
  r->abscissa.maj.length = 200.0;
  r->abscissa.maj.inter = .1;
  r->abscissa.maj.list = NULL;
  r->abscissa.maj.num = 0;
  r->abscissa.min1.style =  EDGES;
  r->abscissa.min1.length = 100.0;
  r->abscissa.min1.inter = .05;
  r->abscissa.min1.list = NULL;
  r->abscissa.min1.num = 0;
  r->abscissa.min2.style = EDGES;
  r->abscissa.min2.length = 50.0;
  r->abscissa.min2.inter = .01;
  r->abscissa.min2.list = NULL;
  r->abscissa.min2.num = 0;
  r->abscissa.precision = savestring(default_x_precision);
  r->abscissa.num_inter = .1;
  r->abscissa.num_loc = NUM_LB;
  r->bounds.top = 1;
  r->bounds.bottom = 384;
  r->bounds.left = 1;
  r->bounds.right = 512;
  r->color = 255;
  r->linetype = 1;
  r->font = XV_NULL;
  r->abs_label = NULL;
  r->ord_label = NULL;
  r->ordi.start = 0.0;
  r->ordi.end = 5000.0;
  r->absc.start = 0.0;
  r->absc.end = 1.0;
  return(r);
}

/******************************************************************/
/* Find a "round" approximation to u of the form c*10^e, where the
   coefficient c is 1, 2, or 5, and the exponent e is an integer.
   Result lies between 0.63*u and 1.6*u. */

double
ret_scale(u, cp, ep)
    double  u;
    int     *cp, *ep;
{
    double  half_log_2 = .1505149978319905976;	/* log10(2.0)/2.0 */
    double  t;
    int	    c, e;

    t = log10(u);
    t -= e = ROUND(t);

    if (t > half_log_2) c = 2;
    else
    if (t >= -half_log_2) c = 1;
    else
    {
	c = 5;
	e -= 1;
    }

    if (cp) *cp = c;
    if (ep) *ep = e;
    return c * pow(10.0, (double) e);
}

/******************************************************************/

int
scale_spect_view_ret(r, v)
    Reticle *r;
    View    *v;
{
    double  x_scl = (r->absc.end - r->absc.start) * PIX_PER_CM
			/ (double)(r->bounds.right - r->bounds.left);
    double  y_scl = (r->ordi.end - r->ordi.start) * PIX_PER_CM
			/ (double)(r->bounds.bottom - r->bounds.top);
    double  maj_len = 0.5;	/* Longest ticks 0.5 cm. */
    double  ab_maj_int = 2.0;	/* Spacing between major ticks roughly */
    double  ord_maj_int = 1.5;	/* 2 cm horizontally and 1.5 cm vertically */
    
    /* Give tick marks absolute length, independent of data units. */
    r->ordinate.maj.length = maj_len*x_scl;
    r->ordinate.min1.length = r->ordinate.maj.length/2.0;
    r->ordinate.min2.length = r->ordinate.maj.length/4.0;
    r->abscissa.maj.length = maj_len*y_scl;
    r->abscissa.min1.length = r->abscissa.maj.length/2.0;
    r->abscissa.min2.length = r->abscissa.maj.length/4.0;

    /* Insure reasonable tick spacing. */
    
    lin_spect_view_int(&r->abscissa, ab_maj_int*x_scl);

    if (v->sig && v->sig->y_dat)
    {
	ord_maj_int = 0.8;	/* Closer spacing of major ticks since we
				   don't yet do minor ticks for nonlinear
				   y scale. */
	arb_spect_view_int(&r->ordinate, ord_maj_int,
			   r->bounds.bottom, r->bounds.top, 
			   v, v->y_to_yval, v->yval_to_y);
    }
    else
	lin_spect_view_int(&r->ordinate, ord_maj_int*y_scl);

    /* Formats for axis numbering */

    reticle_set_ord_precision(r,"%g");
    reticle_set_absc_precision(r,"%g");

    return TRUE;
}

/******************************************************************/

static void
lin_spect_view_int(intv, maj_int)
    Interval	*intv;
    double	maj_int;	/* data units */
{
    int		coef, expt;

    /* maj_int is desired spacing of major ticks.
       Actual spacing may be larger or smaller
       by a factor of sqrt(5.0 / 2.0)
       to allow finding a "round" approximation
       (1, 2, of 5 times pwr of 10). */

    intv->maj.inter =
	intv->num_inter = ret_scale(maj_int, &coef, &expt);
    intv->min1.inter = intv->maj.inter/2.0;
    intv->min2.inter = intv->maj.inter/10.0;

    switch (coef)
    {
    case 1:
	break;
    case 2:
	intv->min1.length = intv->maj.length;
	break;
    case 5:
	intv->min1.inter = intv->maj.inter/5.0;
	break;
    }
}

/******************************************************************/

double
round_num_between(a, b)
    double  a, b;
{
    double  p, x;

    if (a == b) return a;
    if (a > b) {double t = a; a = b; b = t;}

    p = pow(10.0, floor(log10(b - a)));
    if (p > b - a) p /= 10.0;
    if (10*p <= b - a) p *= 10.0;

    x = b - fmod(b, 10*p);
    if (x >= a) return x;
    x = b - fmod(b, 5*p);
    if (x >= a) return x;
    x = b - fmod(b, 2*p);
    if (x >= a) return x;
    return b - fmod(b, p);

/*!*//* Sometimes there are two equally good candidates of the form
	2n.10^e.  Might be good to find a way to return both. */
}

/******************************************************************/

static void
arb_spect_view_int(intv, maj_int, lo, hi, v, to_dat, to_pix)
    Interval	*intv;		/* e.g. ordinate or abscissa of a reticle */
    double	maj_int;	/* desired tick spacing (cm) */
    int		lo, hi;		/* pixel boundaries (bottom & top or
				   left & right) */
    View	*v;		/* view---arg for to_dat & to_pix */
    double	(*to_dat)();	/* e.g. y_to_yval or x_to_time */
    int		(*to_pix)();	/* e.g. yval_to_y or time_to_x */
{
    int		step, inc, dec;
    int		descending = lo > hi;
    int		tgt, min, max, loc;
    double	val;
    int		len;
    LocVal	*list, *tick;

    /* maj_int is desired spacing of major ticks.
       Actual spacing may be larger or smaller
       by about a factor of sqrt(2.5).
       (Limits arbitrarily chosen to agree with lin_spect_view_int). */

    maj_int *= PIX_PER_CM;	
    if (maj_int < 2.0) maj_int = 2.0;
    step = ROUND(maj_int);
    inc = ROUND(1.58*maj_int) - step;
    dec = step - (2*(inc+step))/5;

    if (descending)
    {
	step = -step;
	inc = -inc;
	dec = -dec;
    }

    len = 1 + (hi - lo)/(step - dec);

    list = (intv->maj.list)
	    ? (LocVal *) realloc((char *) intv->maj.list, sizeof(LocVal)*len)
	    : (LocVal *) malloc(sizeof(LocVal)*len);

    if (!list)		/* allocation error */
    {
	intv->maj.inter = 0.0;
	intv->maj.list = NULL;
	intv->maj.num = 0;
    }
    else
    {
	/* If data interval contains 0.0, maybe better
	   to work in both directions from there. */

	tick = list;

	for (min = tgt = lo, max = tgt + inc;
	     (descending)
		? (max >= hi || ((max = hi), tgt >= hi))
		: (max <= hi || ((max = hi), tgt <= hi));
	     tgt = loc + step, min = tgt - dec, max = tgt + inc)
	{
	    val = round_num_between((*to_dat)(v, min), (*to_dat)(v, max));
	    loc = (*to_pix)(v, val);
 
	    tick->loc = loc;
	    tick->val = val;
	    tick++;
	}

	intv->num_inter = 0.0;	/* Use list of major ticks instead. */
	intv->maj.list = list;
	intv->maj.num = tick - list;
    }

/*!*//* Do minor ticks after we see how the major ticks work out.
	(Will have to add to get_*_marklist then.) */    

    intv->min1.list = intv->min2.list = NULL;
    intv->min1.num = intv->min2.num = 0;
    intv->min1.inter = intv->min2.inter = 0.0;
}

/******************************************************************/

Reticle *
new_wave_reticle()
{
  Reticle *r;

  if(!(r = (Reticle*)malloc(sizeof(Reticle)))) return(NULL);

  r->ordinate.maj.style =  MAJOR | EDGES;
  r->ordinate.maj.inter = 32768.0;
  r->ordinate.maj.list = NULL;
  r->ordinate.maj.num = 0;
  r->ordinate.min1.style =  EDGES | MAJOR;
  r->ordinate.min1.length = .01;
  r->ordinate.min1.inter = 4096.0;
  r->ordinate.min1.list = NULL;
  r->ordinate.min1.num = 0;
  r->ordinate.min2.style = EDGES ;
  r->ordinate.min2.length = .005;
  r->ordinate.min2.inter = 512.0;
  r->ordinate.min2.list = NULL;
  r->ordinate.min2.num = 0;
  r->ordinate.precision = savestring(default_y_precision);
  r->ordinate.num_inter = 8192.0;
  r->ordinate.num_loc = NUM_LB;
  r->abscissa.maj.style =  MINOR1 | EDGES;
  r->abscissa.maj.length = 1024.0;
  r->abscissa.maj.inter = .1;
  r->abscissa.maj.list = NULL;
  r->abscissa.maj.num = 0;
  r->ordinate.maj.length = r->abscissa.maj.inter;
  r->abscissa.min1.style =  EDGES;
  r->abscissa.min1.length = 512.0;
  r->abscissa.min1.inter = .05;
  r->abscissa.min1.list = NULL;
  r->abscissa.min1.num = 0;
  r->abscissa.min2.style = EDGES;
  r->abscissa.min2.length = 256.0;
  r->abscissa.min2.inter = .01;
  r->abscissa.min2.list = NULL;
  r->abscissa.min2.num = 0;
  r->abscissa.precision = savestring(default_x_precision);
  r->abscissa.num_inter = .1;
  r->abscissa.num_loc = NUM_LB;
  r->bounds.top = 1;
  r->bounds.bottom = 384;
  r->bounds.left = 1;
  r->bounds.right = 512;
  r->color = 255;
  r->linetype = 1;
  r->font = XV_NULL;
  r->abs_label = NULL;
  r->ord_label = NULL;
  r->ordi.start = -16384.0;
  r->ordi.end = 16384.0;
  r->absc.start = 0.0;
  r->absc.end = 1.5;
  return(r);
}

/******************************************************************/
/* Cf. free_ret in libsig/signal.c. */
void
free_reticle(r)
    Reticle *r;
{
    if (r)
    {
	if (r->abs_label) free(r->abs_label);
	if (r->ord_label) free(r->ord_label);
	if(r->abscissa.precision)  free(r->abscissa.precision);
	if(r->ordinate.precision)  free(r->ordinate.precision);
	if (r->ordinate.maj.list)  free(r->ordinate.maj.list);
	if (r->ordinate.min1.list) free(r->ordinate.min1.list);
	if (r->ordinate.min2.list) free(r->ordinate.min2.list);
	if (r->abscissa.maj.list)  free(r->abscissa.maj.list);
	if (r->abscissa.min1.list) free(r->abscissa.min1.list);
	if (r->abscissa.min2.list) free(r->abscissa.min2.list);
	free(r);
    }
}

/******************************************************************/
static double 
myfmod(num,den)
     register double num, den;
{
  register int q;
  register double dq;

  dq = num/den;
  q = dq;
  if(dq >= 0.0) {
    if((dq - ((double)q)) > .99999) q++;
    return( num - ((double)q)*den);
  } else {
    if((((double)q) - dq) > .99999) q--;
    return((((double)q)*den) - num);
  }
}

/******************************************************************/
/*   here is the code to draw into pixwins (of canvases) */
/******************************************************************/
#ifdef PW_PLOT
static void 
do_x_numbers(x,r,xp,pw)
     Pixwin *pw;
     double x;
     Reticle *r;
     register int xp;
{
  int	xloc, yloc,
	font_height = def_font_height,
    	font_width = def_font_width;

  char	temp[100], *p;

  sprintf(temp,r->abscissa.precision,x);
  p = temp;
  while(*p == ' ') p++;
  xloc = xp - ((strlen(p) * font_width)/2);
  if(r->abscissa.num_loc & NUM_LB) {
    yloc = r->bounds.bottom + font_height;
    pw_text(pw, xloc, yloc,
	    PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ ), def_font, p);
  }
  if(r->abscissa.num_loc & NUM_RT) {
    yloc = r->bounds.top - (font_height/4);
    pw_ttext(pw, xloc, yloc,
	     PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ ), def_font, p);
  }
}

/******************************************************************/
static void 
do_x_label(r,pw)
     Pixwin *pw;
     Reticle *r;
{
  int	xloc, yloc;

  int	font_height = def_font_height,
    	font_width = def_font_width;

  char	temp[100];

  if(r->abs_label) {
    sprintf(temp,"%s",r->abs_label);
    xloc = (r->bounds.right + r->bounds.left  - (strlen(temp) * font_width))/2;
    yloc = r->bounds.bottom + font_height;
    if(r->abscissa.num_loc & NUM_LB) yloc += (font_height + 1);
    pw_text(pw, xloc, yloc,
	    PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ ), def_font, temp);
  }
}
 
/******************************************************************/
static void 
do_y_numbers(y,r,yp,pw)
     Pixwin *pw;
     double y;
     Reticle *r;
     register int yp;
{
  int	xloc, yloc;
  int	font_height = def_font_height,
    	font_width = def_font_width, pop;
  extern major_lines;

  char	temp[100], *p;

  sprintf(temp,r->ordinate.precision,y);
  p = temp;
  while(*p == ' ') p++;
  yloc = yp + (font_height/4);
  if(r->ordinate.num_loc & NUM_LB) {
    xloc = r->bounds.left - (strlen(p) * font_width) - (font_width/2);
    pw_text(pw, xloc, yloc,
	    PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ ), def_font, p);
    if (major_lines) {
    	pop = PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ );
    	pw_vector(pw, r->bounds.left, yp, r->bounds.right, yp, pop, 1);
    }
  }
  if(r->ordinate.num_loc & NUM_RT) {
    xloc = r->bounds.right + (font_width/2);
    pw_text(pw, xloc, yloc,
	    PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ ), def_font, p);
  }
}

/******************************************************************/
reticle_set_absc_precision(r,form)
     Reticle *r;
     char *form;
{
  if(r) {
    if(!(form && *form)) {
      if(r->abscissa.precision)
	free(r->abscissa.precision);
      r->abscissa.precision = NULL;
      return;
    }
    if(! r->abscissa.precision)
      r->abscissa.precision = savestring(form);
    else {
      if(strlen(form) > strlen(r->abscissa.precision)) {
	free(r->abscissa.precision);
	r->abscissa.precision = savestring(form);
      } else
	strcpy(r->abscissa.precision, form);
    }
  }
}

/******************************************************************/
reticle_set_ord_precision(r,form)
     Reticle *r;
     char *form;
{
  if(r) {
    if(!(form && *form)) {
      if(r->ordinate.precision)
	free(r->ordinate.precision);
      r->ordinate.precision = NULL;
      return;
    }
    if(! r->ordinate.precision)
      r->ordinate.precision = savestring(form);
    else {
      if(strlen(form) > strlen(r->ordinate.precision)) {
	free(r->ordinate.precision);
	r->ordinate.precision = savestring(form);
      } else
	strcpy(r->ordinate.precision, form);
    }
  }
}

/******************************************************************/
draw_reticle(canvas, r)
    Canvas canvas;
    Reticle *r;
{
  double	pix_x, pix_y, x, y, dx, dy, xmint, ymint, xe, xb, ye, yb,
		xdel, ydel;
  register int	i, j, k, nx, ny;
  int		*ed, *ma, *mi1, *mi2, *maa, *mi2a, *mi1a, maxl, mi1xl, mi2xl,
		mayl, mi1yl, mi2yl,
		*get_absc_marklist(), *get_ord_marklist(), xp, yp, pop, *im[3];
  Pixwin	*pw;
  extern int	reticle_grid;
  LocVal	*list;
  int		num;

  if(! (canvas && r)) return(FALSE);

  xdel = (r->absc.end - r->absc.start);
  ydel = (r->ordi.end - r->ordi.start);
  if(xdel == 0.0) xdel = 1.0;
  if(ydel == 0.0) ydel = 1.0;

  pix_x = ((double)(r->bounds.right - r->bounds.left))/xdel;
  pix_y = ((double)(r->bounds.bottom - r->bounds.top))/ydel;

  maxl = oneormore(.5 + r->abscissa.maj.length * pix_y);
  mi1xl = oneormore(.5 + r->abscissa.min1.length * pix_y);
  mi2xl = oneormore(.5 + r->abscissa.min2.length * pix_y);
  xb = r->absc.start;
  xe = r->absc.end;
  yb = r->ordi.start;
  ye = r->ordi.end;
  
  pw = canvas_pixwin(canvas);
  pop = PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ );
  
  /* draw a box around the exterior of the data display area */
  pw_vector(pw, r->bounds.left - 1, r->bounds.bottom + 1,
	    r->bounds.right + 1, r->bounds.bottom + 1, pop,1);
  pw_vector(pw, r->bounds.right + 1, r->bounds.bottom,
	    r->bounds.right + 1, r->bounds.top - 1, pop,1);
  pw_vector(pw, r->bounds.right, r->bounds.top - 1,
	    r->bounds.left, r->bounds.top - 1, pop,1);
  pw_vector(pw, r->bounds.left - 1, r->bounds.top,
	    r->bounds.left - 1, r->bounds.bottom, pop,1);

  /* ordinate numbering */
  if ((ymint = r->ordinate.num_inter) > 0.0)
  {
      ydel =  myfmod(yb, ymint);
      y = yb;
      if(ydel > (ymint * .0001)) {
        if(yb >= 0.0) y += (ymint - ydel);
        else y += ydel;
      }
      while(y <= ye) {
        yp = r->bounds.bottom - (int) (((y - yb) * pix_y) + .5);
        do_y_numbers(y, r, yp, pw);
        y += ymint;
      }
  }
  else if (list = r->ordinate.maj.list)
  {
      num = r->ordinate.maj.num;
      for (i = 0; i < num; i++)
	  do_y_numbers(list[i].val, r, list[i].loc, pw);
  }

  /* abscissa numbering */
  if (((xmint = r->abscissa.num_inter) > 0.0) &&
      (r->abscissa.num_loc & NUM_BOTH))
  {
    xdel = myfmod(xb, xmint);
    x = xb;
    if(xdel > (xmint * .0001)) {
      if(xb >= 0.0) x += (xmint - xdel);
      else x += xdel;
    }
    while(x < xe) {
      xp = r->bounds.left + (int) (((x - xb) * pix_x) + .5);
      do_x_numbers(x,r,xp,pw);
      x += xmint;
    }
  }

  do_x_label(r,pw);		/* apply abscissa label */
    
  if((ed = get_ord_marklist(r,EDGES))) { /* ordinate marks at ends of plot */
    for(i = 0, j = 1; i < *ed; i++, j += 3) {
      yp = ed[j+2];
      pw_vector(pw, r->bounds.left, yp,
		r->bounds.left + ed[j+1], yp, pop, 1);
      pw_vector(pw, r->bounds.right - ed[j+1], yp,
		r->bounds.right, yp, pop, 1);
    }
  }

  /* get ordinate marks to place on interior of plot */
  ma = get_ord_marklist(r,MAJOR);
  mi1 = get_ord_marklist(r,MINOR1);
  mi2 = get_ord_marklist(r,MINOR2);
  maa = get_absc_marklist(r,MAJOR);
  mi1a = get_absc_marklist(r,MINOR1);
  mi2a = get_absc_marklist(r,MINOR2);
  
  pw = canvas_pixwin(canvas);
  pop = PIX_COLOR(r->color)|(PIX_SRC /*^PIX_DST*/ );

  if ((xmint = r->abscissa.min2.inter) > 0.0
      || (xmint = r->abscissa.min1.inter) > 0.0
      || (xmint = r->abscissa.maj.inter) > 0.0)
  {
    if(xb >= 0.0) x = r->absc.start + xmint - myfmod(r->absc.start,xmint);
    else x = r->absc.start + myfmod(r->absc.start,xmint);
    xdel =  .9999 * xmint;
    while(x < xe) {
      if(myfmod(x,r->abscissa.maj.inter) < xdel) {
	xp = r->bounds.left + (int) (((x - xb) * pix_x) + .5);
	if((r->abscissa.maj.style & EDGES)) {
	  pw_vector(pw, xp, r->bounds.bottom,
		    xp, (int)r->bounds.bottom - maxl, pop, 1);
	  pw_vector(pw, xp, r->bounds.top,
		    xp, (int)r->bounds.top + maxl, pop, 1);
	}
	central_marks( ma, maa, xp, pop, pw);
      } else {
	if(myfmod(x,r->abscissa.min1.inter) < xdel) {
	  xp = r->bounds.left + (int) (((x - xb) * pix_x) + .5);
	  if((r->abscissa.min1.style & EDGES)) {
	    pw_vector(pw, xp, r->bounds.bottom,
		      xp, (int)r->bounds.bottom - mi1xl, pop, 1);
	    pw_vector(pw, xp, r->bounds.top,
		      xp, (int)r->bounds.top + mi1xl, pop, 1);
	  }
	  central_marks( mi1, mi1a, xp, pop, pw);
	} else {
	  xp = r->bounds.left + (int) (((x - xb) * pix_x) + .5);
	  if((r->abscissa.min2.style & EDGES)) {
	    pw_vector(pw, xp, r->bounds.bottom,
		      xp, (int)r->bounds.bottom - mi2xl, pop, 1);
	    pw_vector(pw, xp, r->bounds.top,
		      xp, (int)r->bounds.top + mi2xl, pop, 1);
	  }
	  central_marks( mi2, mi2a, xp, pop, pw);
	}
      }
      x += xmint;
    }
  }

  if(mi1)free(mi1);		/* free up scratch arrays */
  if(mi2)free(mi2);
  if(ma)free(ma);
  if(mi1a)free(mi1a);
  if(mi2a)free(mi2a);
  if(maa)free(maa);
  if(ed)free(ed);
  return(TRUE);
}

/******************************************************************/
static void 
central_marks( yloco, yloca, xp, pop, pw)
     Pixwin *pw;
     register int *yloco, *yloca, xp, pop;
{
  register int yp, i, j, k, *p;
  extern int do_central_marks;
	  
  if(do_central_marks) {
   if(yloco) {			/* draw ordinate marks at this x-position */
    for(i = 0, j = 1; i < *yloco; i++, j += 3) {
      yp = yloco[j+2];
      pw_vector(pw, xp + yloco[j], yp,
		xp + yloco[j+1], yp, pop, 1);
    }
   }
   if(yloca) {			/* draw abscissa marks for this position */
    for(i = 0, j = 1; i < *yloca; i++, j += 2) {
      pw_vector(pw, xp, yloca[j], xp, yloca[j+1], pop, 1);
    }
   }
  }
}
#endif

/******************************************************************/
static int *
get_ord_marklist(r, where)
    Reticle *r;
    int	    where;
{
    double  pix_x, pix_y, y, ydel, ymint, yb, ye;
    int	    mayhl, mi1yhl, mi2yhl, *p, *q, nmarks, i, length;
    LocVal  *list;

    pix_x = ((double)(r->bounds.right - r->bounds.left))/
		(r->absc.end - r->absc.start);
    pix_x *= 0.5;

    mi2yhl = (r->ordinate.min2.style & where)
	     ? oneormore(.5 + (r->ordinate.min2.length * pix_x))
	     : 0;
    mi1yhl = (r->ordinate.min1.style & where)
	     ? oneormore(.5 + (r->ordinate.min1.length * pix_x))
	     : mi2yhl;
    mayhl = (r->ordinate.maj.style & where)
	    ? oneormore(.5 + (r->ordinate.maj.length * pix_x))
	    : mi1yhl;

    if (mayhl == 0) return NULL;

    if (list = r->ordinate.maj.list)
    {
/*!*//* Elaborate when we do variably spaced minor ticks. */
	nmarks = r->ordinate.maj.num;
	if (nmarks <= 0) return NULL;

	p = (int *) malloc(sizeof(int) * ((nmarks+1) * 3 + 1));
	*p = 0;
	q = p + 1;

	length = mayhl;
	for (i = 0; i < nmarks; i++)
	{
	    *q++ = -length;
	    *q++ = length;
	    *q++ = list[i].loc;
	    *p += 1;
	}

	return p;
    }
    else
    if ((ymint = r->ordinate.min2.inter) > 0.0
	|| (ymint = r->ordinate.min1.inter) > 0.0
	|| (ymint = r->ordinate.maj.inter) > 0.0)
    {
	nmarks = (r->ordi.end - r->ordi.start)/ymint;
	if (nmarks <= 0) return (NULL);
  
        pix_y = ((double)(r->bounds.bottom - r->bounds.top))/
		    (r->ordi.end - r->ordi.start);
        yb = r->ordi.start;
        ye = r->ordi.end;

	p = (int *) malloc(sizeof(int) * ((nmarks+1) * 3 + 1));
	*p = 0;
	q = p + 1;
	ydel =  .9999 * ymint;

	for (y = (yb >= 0.0) ? yb + (ymint - myfmod(yb, ymint))
			     : yb + myfmod(yb, ymint);
	     y < ye; y += ymint)
	{
	    length = (myfmod(y, r->ordinate.maj.inter) < ydel) ? mayhl
		     : (myfmod(y,r->ordinate.min1.inter) < ydel) ? mi1yhl
		     : mi2yhl;
	    if (length)
	    {
		*q++ = -length;
		*q++ = length;
		*q++ = r->bounds.bottom - (int) ((y - yb) * pix_y + .5);
		*p += 1;
	    }
	}
	return p;
    }

    return NULL;
}

/******************************************************************/
static int *
get_absc_marklist(r, where)
    Reticle *r;
    int	    where;
{
    double  pix_y, y, dy, ymint, yb, ye;
    int	    maxhl, mi1xhl, mi2xhl, majl, min1l, min2l;
    int	    *p, *q, nmarks, i, length, yp;
    LocVal  *list;

    if (r->ordi.end - r->ordi.start == 0)
	pix_y = DBL_MAX;
    else
        pix_y = ((double)(r->bounds.bottom - r->bounds.top))/
    		(r->ordi.end - r->ordi.start);

    maxhl = oneormore(.5 + (r->abscissa.maj.length * pix_y * .5));
    mi1xhl = oneormore(.5 + (r->abscissa.min1.length * pix_y * .5));
    mi2xhl = oneormore(.5 + (r->abscissa.min2.length * pix_y * .5));

    majl = (r->abscissa.maj.style & MAJOR & where) ? maxhl
	   : ((r->abscissa.min1.style & MAJOR)
	      && ((MAJOR | MINOR1) & where)) ? mi1xhl
	   : (r->abscissa.min2.style & MAJOR) ? mi2xhl
	   : 0;
    min1l = ((r->abscissa.maj.style & MINOR1) && (MAJOR & where)) ? maxhl
	    : ((r->abscissa.min1.style & MINOR1)
	       && ((MAJOR | MINOR1) & where)) ? mi1xhl
	    : (r->abscissa.min2.style & MINOR1) ? mi2xhl
	    : 0;
    min2l = ((r->abscissa.maj.style & MINOR2) && (MAJOR & where)) ? maxhl
	    : ((r->abscissa.min1.style & MINOR2)
	       && ((MAJOR | MINOR1) & where)) ? mi1xhl
	    : (r->abscissa.min2.style & MINOR2) ? mi2xhl
	    : 0;

    if (list = r->ordinate.maj.list)
    {
/*!*//* Elaborate when we do variably spaced minor ticks. */
	nmarks = r->ordinate.maj.num;
	if (nmarks <= 0) return NULL;

	p = (int *) malloc(sizeof(int) * ((nmarks+1) * 2 + 1));
	*p = 0;
	q = p + 1;

	length = majl;

	if (length)
	    for (i = 0; i < nmarks; i++)
	    {
		yp = list[i].loc;
		*q++ = yp - length;
		*q++ = yp + length;
		*p += 1;
	    }

	return p;
    }
    else
    if ((ymint = r->ordinate.min2.inter) > 0.0
	|| (ymint = r->ordinate.min1.inter) > 0.0
	|| (ymint = r->ordinate.maj.inter) > 0.0)
    {
      nmarks = (r->ordi.end - r->ordi.start)/ymint;
	if (nmarks <= 0) return (NULL);
  
	yb = r->ordi.start;
	ye = r->ordi.end;

	p = (int *) malloc(sizeof(int) * ((nmarks+1) * 2 + 1));
	*p = 0;
	q = p + 1;

	for (y = (yb >= 0.0) ? yb + (ymint - myfmod(yb, ymint))
			    : yb + myfmod(yb, ymint);
	     y < ye;
	     y += ymint)
	{
	    length = (myfmod(y, r->ordinate.maj.inter) < ymint) ? majl
		     : (myfmod(y, r->ordinate.min1.inter) < ymint) ? min1l
		     : min2l;

	    if (length)
	    {
		yp = r->bounds.bottom - (int) ((y - yb) * pix_y + .5);
		*q++ = yp - length;
		*q++ = yp + length;
		*p += 1;
	    }
	  }
	return p;
    }

    return NULL;
}

