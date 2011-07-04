/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rod Johnson, modifed by Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: plot filter for HP laserjet
 *
 */

static char *sccs_id = "@(#)imdevhp.c	1.4	12/14/92	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define ESC		'\033'
#define FF		'\014'
#define HP_RESET	"%cE", ESC
#define HP_POSITION	"%c*p%dx%dY", ESC
#define HP_INC_Y	"%c*p+%dY", ESC
#define HP_POSITION_X	"%c*p%dX", ESC
#define HP_RESOLUTION	"%c*t%dR", ESC
#define HP_START_GRAPH	"%c*r1A", ESC
#define HP_TRANS_DATA	"%c*b%dW", ESC
#define HP_END_GRAPH	"%c*rB", ESC
#define HP_PAGE_W	2550
#define HP_PAGE_H	3300
#define HP_LMARG	50
#define HP_TMARG	150
#define PI      3.14159265358979323846
#define NCIRC   120             /* Normal circles */
#define NARC    50              /* Number of segments in arc, if it were 360 * deg. */
#define SPACES \
"                                        \
                                        "       /* 80 blanks */



int		abs();

char		*arr_alloc();
void		arr_free();
double		sin(), cos();


static void	 hp_plotline();
void		do_plot();
static int scalex(x);
static int scaley(y);

extern int	debug_level;
long	width=2400, height=2400;
long	lmarg=0, rmarg=0, tmarg=0, bmarg=0;
long	xorig, yorig;
int	oflag=0;
int	mag=1;
static long      xpos = 0, ypos = 0;
static long      xlow = 0, ylow = 0, xhigh = 1000, yhigh = 1000;




static unsigned char
		**bitarray;
static long	bitarrdim[2];
static FILE	*outfile;

int text_offset=0;


void init_device()
{
    int		    u, v;


    bitarrdim[0] = width + lmarg + rmarg;
    bitarrdim[1] = (height + bmarg + tmarg + 7)/8;

    bitarray = (unsigned char **) arr_alloc(2, bitarrdim, CHAR, 0);
    for (v = 0; v < bitarrdim[0]; v++)
	for (u = 0; u < bitarrdim[1]; u++)
	bitarray[v][u] = 0;

    xorig = yorig = 1;
}


void
hp_fin()
{
    int	    v,h,print_it,h1;
    int	    h_offset, v_offset;
    FILE *bar = fopen("bar","w");
	FILE	*outfile = stdout;

    h_offset = (HP_PAGE_H - (lmarg + width + rmarg)*mag)/2 - HP_TMARG;
    v_offset = (HP_PAGE_W - (bmarg + height + tmarg)*mag)/2 - HP_LMARG;

    printf(HP_RESET);
    printf(HP_POSITION, v_offset, h_offset);
    printf(HP_RESOLUTION, 300/mag);

    for (v = 0; v < bitarrdim[0]; v++)
    {
	print_it=0;
	for (h = 0; h < bitarrdim[1]; h++)
	   if(bitarray[v][h]) {
	     print_it=1;
	     break;
           }
	if(print_it)
        {
	   for (h1 = bitarrdim[1]-1; h1 > 0; h1--)
	   {
	      if(bitarray[v][h1])
	      {
               printf(HP_POSITION,v_offset+(h*8),h_offset);
               printf(HP_START_GRAPH);
	       printf(HP_TRANS_DATA, h1-h+1);
	       fwrite((char *) bitarray[v]+h,
		sizeof(bitarray[0][0]), h1-h+1, outfile);
	       break;
	      }
           }
	}
	h_offset++;
    }

    printf(HP_END_GRAPH);
    putchar(FF);
    printf(HP_RESET);
    arr_free((char *) bitarray, 2, CHAR, 0);
}

static void
hp_vector(x1,y1,x2,y2)
    long x1, y1, x2, y2;
{
    long xarray[2], yarray[2];

    x1 = x1 *.9;
    x2 = x2 *.9;
    y1 = y1 *.9;
    y2 = y2 *.9;
    xarray[0]=x1; xarray[1]=x2;
    yarray[0]=y1; yarray[1]=y2;
    hp_plotline(2,yarray,xarray);
}

static void
hp_plotline(m, hor, ver)
    long    m;
    long    *hor, *ver;
{
    int	    i, j, n;
    int	    d_hor, d_ver;
    double  u, v, du, dv;

    for (i = 1; i < m; i++)
    {
	d_hor = hor[i] - hor[i-1];
	d_ver = ver[i] - ver[i-1];
/*Fprintf(stderr, "d_hor %d, d_ver %d\n", d_hor, d_ver);*/

	v = lmarg + hor[i-1] + 0.5;
	u = bmarg + ver[i-1] + 0.5;
/*Fprintf(stderr, "u %g, v %g\n", u, v);*/

	if (d_hor == 0 && d_ver == 0)
	{
	    du = dv = 0.0;
	    n = 1;
	}
	else if (abs(d_hor) > abs(d_ver))
	{
	    dv = (d_hor > 0) ? 1.0 : -1.0;
	    du = (dv * d_ver) / d_hor;
	    n = 1 + abs(d_hor);
	}
	else
	{
	    du = (d_ver > 0) ? 1.0 : -1.0;
	    dv = (du * d_hor) / d_ver;
	    n = 1 + abs(d_ver);
	}
/*Fprintf(stderr, "du %g, dv %g, n %d\n", du, dv, n);*/
	for (j = 0; j < n; j++)
	{
	    unsigned char   b =	0x80 >> ((int) u)%8;

	    bitarray[(int) v][((int) u)/8] |= b;
/*				(unsigned char) (0x80 >> ((int) u)%8);*/
/*Fprintf(stderr, "(int) u %d, (int) v %d\n", (int) u, (int) v);*/
/*Fprintf(stderr, "((int) u)%%8 %d, b %d\n", ((int) u)%8, b);*/
/*Fprintf(stderr, "bitarray[%d][%d] = %d\n", (int) v, ((int) u)/8, */
/*bitarray[(int) v][((int) u)/8]);*/
	    u += du;
	    v += dv;
	}
    }
}

erase()
{  }

label(s)
char *s;
{  }

line(x1, y1, x2, y2)
        int             x1, y1, x2, y2;
{
        move(x1, y1);
        cont(x2, y2);
}

circle(x, y, r)
        int             x, y, r;
{
        static          tblinited = 0;
        static double   sine[NCIRC], cosine[NCIRC];

        long             i, scaledx, scaledy, scaledr, oldx, oldy, newx, newy;

        if (!tblinited) {       /* cache of NCIRC sines and cosines */
                double          theta, incr;

                tblinited = 1;
                incr = (2 * PI) / NCIRC;
                for (theta = incr, i = 1; i < NCIRC; theta += incr, i++)
{
                        sine[i] = sin(theta);
                        cosine[i] = cos(theta);
                }
        }
        scaledx = scalex(x);
        scaledy = scaley(y);
        scaledr =.5 + scalex(r);
        oldx = scaledx + scaledr;
        oldy = scaledy;

        for (i = 1; i < NCIRC; i++) {
                newx = scaledx + scaledr * cosine[i];
                newy = scaledy + scaledr * sine[i];
                hp_vector(oldx, oldy, newx, newy);
                oldx = newx;
                oldy = newy;
        }
        /* close up */
        hp_vector(oldx, oldy, scaledx + scaledr, scaledy);
}

static double
bear(x0, y0, x1, y1)
        int             x0, y0, x1, y1;
{
        return atan2((double) y1 - (double) y0, (double) x1 - (double)
x0);
}

static double
myhypot(x, y)
        int             x, y;
{
        return sqrt((double) x * (double) x + (double) y * (double) y);
}

arc(x, y, x0, y0, x1, y1)
        int             x, y, x0, y0, x1, y1;
{
        double          theta0, theta1, theta, incr;
        int             r;

        r = (int) myhypot(x0 - x, y0 - y);
        theta0 = bear(x, y, x0, y0);
        theta1 = bear(x, y, x1, y1);
        if (theta0 >= theta1)
                theta1 += 2 * PI;

        incr = (2 * PI) / NARC;
        move((int) (x + r * cos(theta0)), (int) (y + r * sin(theta0)));
        for (theta = theta0; theta <= theta1; theta += incr)
                cont((int) (x + r * cos(theta)), (int) (y + r *
sin(theta)));

/* one extra to close up */
        cont((int) (x + r * cos(theta1)), (int) (y + r * sin(theta1)));
}

move(x, y)
        int             x, y;
{
        xpos = scalex(x);
        ypos = scaley(y);
}

cont(x, y)
        int             x, y;
{
        long             oldx = xpos, oldy = ypos;
        move(x, y);
	hp_vector(oldx,oldy,xpos,ypos);
}


space(x0, y0, x1, y1)
        int             x0, y0, x1, y1;
{

        xlow = x0;
        ylow = y0;
        xhigh = x1;
        yhigh = y1;


        xorig = 1;
        yorig = 1;

}

void
start_plot()
{
	do_plot();
}

static int
scalex(x)
	int             x;
{
	return (xorig + width * (x - xlow) / (xhigh - xlow));
}

static int
scaley(y)
	int             y;
{
	return (yorig + height * (yhigh - y) / (yhigh - ylow)) + text_offset;
}
