/*
|									
|   This material contains proprietary software of Entropic Speech,
|   Inc.  Any reproduction, distribution, or publication without the
|   prior written permission of Entropic Speech, Inc. is strictly
|   prohibited.  Any public distribution of copies of this work	
|   authorized in writing by Entropic Speech, Inc. must bear the
|   notice						
|						
|   "Copyright (c) 1987,1988 Entropic Speech, Inc. All rights reserved."
|								
|						
|   winplt.c - Unix plotting functions for Sun windows
|						
|   Rodney W. Johnson, Entropic Speech, Inc.
|   The window plot code came from Rob Jacob at the Naval Research Lab.
|   Modified for SunViews by Alan Parker.  Range functions added by Parker.
|									
*/
#ifdef SCCS
static char    *sccs_id = "@(#)winplt.c	3.8 2/20/89	ESI";
#endif

#include <sys/file.h>
#include <stdio.h>
#include <math.h>
#include <esps/esps.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>

#define PI	3.14159265358979323846
#define NCIRC	120		/* Normal circles */
#define NARC	50		/* Number of segments in arc, if it were 360
				 * deg. */
#define NONE	0
#define RIGHT	1
#define MIDDLE	2
#define LEFT	4
#define SPACES \
"                                        \
                                        "	/* 80 blanks */

/* WARNING these numbers are from drawbox.c in plotsd */

#define BOX_START 500
#define BOX_END 5000
#define BOX_TOP 500+70
#define BOX_BOTTOM 3000-70


extern char    *ProgName;

static Pixwin  *pw;
static struct pixfont *SunFont;

static int      width, height;
static int      xorig, yorig;

static int      xpos = 0, ypos = 0;
static int      xlow = 0, ylow = 0, xhigh = 1000, yhigh = 1000;

extern	char	*title;
extern	int	R_flag;
extern	int	P_flag;
extern	char	*icon_file;
char		*filename;
extern  char	*play_options;

int             beginplot;	/* read from ESPS Common file */
int             endplot;	/* read from ESPS Common file */
int             points;

int             new_start;	/* new start point selected */
int             new_nan;	/* new nan point selected */

int             box_start;
int             box_end;
int		box_top;
int		box_bottom;

int		mark_x=0;
int		cursor_x=0;

int             range_state = 1;
int             x_start;
int             x;
int		first_paint = 1;

void 		save_common(), get_common();
void 		s_text();
void		play_file(), play_range(), zoom_in(), zoom_out();
void		quit_proc();
void		do_plot();
char		*sprintf(), *strcpy(), *strcat();
extern 	char 	*tmpname;
int		played=0;
char		range_arg[100];
char		buf[512];	/* general purpose buffer */

static	char	*msg1 = "LEFT: select start of range, MIDDLE: play entire plot, RIGHT: zoom out\n";

static	char	*msg1a = "LEFT: select start of range, MIDDLE: <nothing>, RIGHT: zoom out\n";

static	char	*msg2 = "LEFT: zoom in, MIDDLE: play range, RIGHT: reselect range\n";

static	char	*msg3 = "Saving range...\n";

static	char	*msg4 = "LEFT: zoom in, MIDDLE: save range, RIGHT: reselect range\n";
int	text_offset=0;
static	char	*msg1_ptr;

extern  int	window_pos_x;
extern  int	window_pos_y;
extern  int	window_height;
extern	int	window_width;


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



static void     repaint_canvas();
static void     my_event_proc();

Frame           frame;
Canvas          canvas;

static	short	icon_image[] = {
/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16 */
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x7E3C,0x7C3C,0x0001,0x8000,0x4042,0x4242,0x0001,
	0x8000,0x4040,0x4240,0x0001,0x8000,0x7C3C,0x423C,0x0001,
	0x8000,0x4002,0x7C02,0x0001,0x8000,0x4002,0x4002,0x0001,
	0x8000,0x4042,0x4042,0x0001,0x8000,0x7E3C,0x403C,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8070,0x0000,0x0000,0x0001,0x8070,0x0000,0x0000,0x0001,
	0x80F8,0x0000,0x0000,0x0001,0x80A8,0x0000,0x0000,0x0001,
	0x8124,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8021,0xC000,0x01C0,0x0001,
	0x8022,0x2000,0x0220,0x0001,0x8024,0x1000,0x0410,0x0001,
	0x8024,0x1000,0x0410,0x0001,0x8028,0x0800,0x0808,0x0001,
	0x8028,0x0800,0x0808,0x0001,0x8028,0x0800,0x0808,0x0001,
	0x8030,0x0400,0x1004,0x0001,0x8030,0x0400,0x1004,0x0201,
	0x8030,0x0400,0x1004,0x0181,0x8020,0x0200,0x2002,0x00E1,
	0x9FFF,0xFFFF,0xFFFF,0xFFF9,0x8020,0x0200,0x2002,0x00E1,
	0x8060,0x0100,0x4001,0x0181,0x8060,0x0100,0x4001,0x0201,
	0x8060,0x0100,0x4001,0x0001,0x80A0,0x0080,0x8000,0x8001,
	0x80A0,0x0080,0x8000,0x8001,0x80A0,0x0080,0x8000,0x8001,
	0x8120,0x0041,0x0000,0x4001,0x8120,0x0041,0x0000,0x4001,
	0x8220,0x0022,0x0000,0x2001,0x8C20,0x001C,0x0000,0x1801,
	0x8020,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8020,0x0000,0x0000,0x0001,
	0x8020,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};
mpr_static(icon_pixrect, 64, 64, 1, icon_image);


int resize_canvas();

extern int Argc;
extern char **Argv;


init_device()
{
	Icon		icon;


	icon = icon_create(ICON_IMAGE, &icon_pixrect, 0);
	if (icon_file) 
		(void)icon_load(icon, icon_file,"Cannot load icon");
	frame = window_create(0, FRAME, 
		/*		FRAME_LABEL, title, */
				FRAME_ICON, icon,
				FRAME_ARGC_PTR_ARGV, &Argc, Argv,
				0);
	if (frame == NULL) {
		fprintf(stderr,"mcd: cannot create frame; be sure you running under Suntools\n");
		exit(1);
	}
	canvas = window_create(frame, CANVAS,
			       CANVAS_RETAINED, TRUE,
			       CANVAS_FIXED_IMAGE, FALSE,
			       CANVAS_REPAINT_PROC, repaint_canvas,
			       CANVAS_RESIZE_PROC, resize_canvas,
			       WIN_EVENT_PROC, my_event_proc,
			       0);
	pw = canvas_pixwin(canvas);
	SunFont = pw_pfsysopen();
	(void)notify_set_destroy_func(frame,quit_proc);

	xorig = yorig = 1;
	width = (int) window_get(canvas, CANVAS_WIDTH);
	height = (int) window_get(canvas, CANVAS_HEIGHT);

	if (P_flag)
		msg1_ptr = msg1;
	else
		msg1_ptr = msg1a;

}

static
resize_canvas(canvas, width, height)
Canvas canvas;
int width, height;
{
	window_set(canvas,	CANVAS_REPAINT_PROC, repaint_canvas,
				0);
}



erase()
{
    pw_writebackground(pw, xorig-1, yorig-1, width+3, height+3, PIX_SRC);
}

label(s)
	char            s[];
{
	struct pr_size  size, pf_textwidth();

	pw_text(pw, xpos, ypos, PIX_SRC, SunFont, s);

	size = pf_textwidth(strlen(s), SunFont, s);
	xpos += size.x;
}

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

	int             i, scaledx, scaledy, scaledr, oldx, oldy, newx, newy;

	if (!tblinited) {	/* cache of NCIRC sines and cosines */
		double          theta, incr;

		tblinited = 1;
		incr = (2 * PI) / NCIRC;
		for (theta = incr, i = 1; i < NCIRC; theta += incr, i++) {
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
		pw_vector(pw, oldx, oldy, newx, newy, PIX_SRC, 1);
		oldx = newx;
		oldy = newy;
	}
	/* close up */
	pw_vector(pw, oldx, oldy, scaledx + scaledr, scaledy, PIX_SRC, 1);
}

static double
bear(x0, y0, x1, y1)
	int             x0, y0, x1, y1;
{
	return atan2((double) y1 - (double) y0, (double) x1 - (double) x0);
}

/*
 * like UNIX hypot, but takes integers as arguments; returns a double
 */
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
		cont((int) (x + r * cos(theta)), (int) (y + r * sin(theta)));

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
	int             oldx = xpos, oldy = ypos;
	move(x, y);
	pw_vector(pw, oldx, oldy, xpos, ypos, PIX_SRC, 1);
}

point(x, y)
	int             x, y;
{
	move(x, y);
	label(".");
}

linemod(s)
	char            s[];
{
}

space(x0, y0, x1, y1)
	int             x0, y0, x1, y1;
{

	width = (int) window_get(canvas, CANVAS_WIDTH);
	height = (int) window_get(canvas, CANVAS_HEIGHT)-text_offset;
	xlow = x0;
	ylow = y0;
	xhigh = x1;
	yhigh = y1;


	xorig = 1;
	yorig = 1;

}


static void
repaint_canvas(canvas, pw, repaint_area)
	Canvas          canvas;
	Pixwin         *pw;
	Rectlist       *repaint_area;
{

	erase();
	do_plot();
	box_start = scalex(BOX_START);
	box_end = box_start + scalex(BOX_END);
	box_top = scaley(-BOX_TOP);
	box_bottom = scaley(-BOX_BOTTOM);
	if(R_flag && first_paint) {
		s_text(msg1_ptr,20,20);	/* display help bar */
		get_common();
		first_paint=0;
	}
 	mark_x=cursor_x = 0;
	range_state = 1;
	window_set(canvas, CANVAS_REPAINT_PROC, (int (*)()) NULL,
			   0);
}

void
start_plot()
{
	window_main_loop(frame);
}

static void
my_event_proc(canvas, event)
	Canvas          canvas;
	Event          *event;
{

	char            s[100];

/* just don't do anything unless the R_flag is on
*/
	if(!R_flag) return;

	switch (event_id(event)) {
	case MS_LEFT:
		if (event_is_up(event))
			return;
		if (range_state == 0) {	/* first time */
			get_common();
			range_state = 1;
		} 
		if (range_state == 1) {	
			range_state = 2;
			if (P_flag)
				s_text(msg2, 20, 20);
			else
				s_text(msg4, 20, 20);
			x_start = event_x(event);
			if(mark_x != 0)
			  pw_vector(pw,mark_x,box_top,mark_x, box_bottom, 
			  PIX_NOT(PIX_DST), 1);
			new_start = beginplot + (points * (x_start - box_start)
						 / (box_end - box_start));
			pw_vector(pw,x_start,box_top,x_start, box_bottom, 
			PIX_NOT(PIX_DST), 1);
			mark_x = x_start;
			
		} else if (range_state == 2)
			zoom_in();
		break;

	case MS_MIDDLE:
		if (event_is_up(event))
			return;
		if (P_flag && range_state == 1) {
			play_file();
		}
		if (range_state == 2) {
			save_common();
			if (!P_flag || played) {
				s_text(msg3,20,40);
				save_common();
				s_text(" ",20,40);
			}
			else {
				play_range();
				s_text(msg4, 20, 20);
				played=1;
			}
		}
		break;

	case MS_RIGHT:
		if (event_is_up(event))
			return;
		if (range_state == 1) 
			zoom_out();
		else if (range_state == 2) {
			range_state = 1;
			s_text(msg1_ptr, 20, 20);
			s_text(" ", 20, 40);
			pw_vector(pw,mark_x,box_top,mark_x, box_bottom, 
			PIX_NOT(PIX_DST), 1);
			pw_vector(pw, cursor_x, box_top, cursor_x, 
			box_bottom, PIX_NOT(PIX_DST), 1);
			mark_x=cursor_x=0;
		}
		break;

	case LOC_MOVE:
		if (range_state == 2) {
			if(played == 1) {
				played=0;
				s_text(msg2,20,20);
			}
			x = event_x(event);
			if ((x < x_start) || (x > box_end))
				s_text("OUT OF RANGE", 20, 40);
			else {
				if(cursor_x != 0)
				  pw_vector(pw, cursor_x, box_top, cursor_x, 
				  box_bottom, PIX_NOT(PIX_DST), 1);
				pw_vector(pw, x, box_top, x, box_bottom, 
			  	PIX_NOT(PIX_DST), 1);
				cursor_x = x;
				
				new_nan = 
				 beginplot + (points * (x - box_start) / 
				 (box_end - box_start));
				(void) sprintf(s, 
				 "Range selected is from %d to %d", 
				 new_start, new_nan);
				s_text(s, 20, 40);
			}
		}
		break;
	}
}


void
s_text(s, x, y)
	char           *s;
	int             x, y;
{
	pw_text(pw, x, y-10, PIX_SRC, SunFont, SPACES);
	pw_text(pw, x, y-10, PIX_SRC, SunFont, s);
}

void
get_common()
{
	(void) read_params((char *) NULL, SC_CHECK_FILE, (char *) NULL);

	if (symtype("beginplot") == ST_UNDEF || symtype("endplot") == ST_UNDEF){
	  Fprintf(stderr, "mcd: no beginplot or endplot in Common file\n");
	  exit(1);
	}

	filename = getsym_s("filename");
	beginplot = getsym_i("beginplot");
	endplot = getsym_i("endplot");
	points = endplot - beginplot + 1;
}

void
save_common()
{
	if (putsym_s("filename", filename) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_i("start", new_start) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_i("nan", new_nan - new_start + 1) == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
	if (putsym_s("prog", "range") == -1)
	  Fprintf(stderr, "mcd: could not write into ESPS Common file.\n");
}


void
play_file()
{

	(void)sprintf(range_arg,"-p%ld:+%ld ",beginplot,points);
	(void)strcpy(buf,"play ");
	(void)strcat(buf,range_arg);
	if (play_options)
		(void)strcat(buf,play_options);
	(void)strcat(buf," ");
	(void)strcat(buf,filename);
	s_text(buf,20,40);
	(void)system(buf);
	s_text(" ",20,40);
}

void
play_range()
{
	(void)sprintf(range_arg,"-p%ld:+%ld ",new_start, new_nan-new_start+1);
  	(void)strcpy(buf,"play ");
  	(void)strcat(buf,range_arg);
	if (play_options)
		(void)strcat(buf,play_options);
	(void)strcat(buf," ");
	(void)strcat(buf,filename);
	s_text(buf,20,40);
  	(void)system(buf);
	(void) sprintf(buf, "Range selected is from %d to %d", new_start, new_nan);
	s_text(buf, 20, 40);
}

void
zoom_in()
{
	
	(void)sprintf(buf,"Replotting points %ld through %ld ...", 
		new_start,new_nan);
	s_text(buf,20,40);
	(void)sprintf(range_arg,"-p%ld:+%ld",new_start, new_nan-new_start+1);
	(void)strcpy(buf,"plotsd  -E.1 -D ");
	(void)strcat(buf,range_arg);
	(void)strcat(buf," -Tgps ");
	(void)strcat(buf,filename);
	(void)strcat(buf," > ");
	(void)strcat(buf,tmpname);
	(void)system(buf);
	erase();
	do_plot();
	box_start = scalex(500);
	box_end = box_start + scalex(5000);
	s_text("",20,40);
	s_text(msg1_ptr,20,20);
	get_common();
	range_state=1;
	cursor_x=mark_x=0;
}

void
zoom_out()
{
	
	s_text("Zooming out...\n",20,40);
	(void)strcpy(buf,"plotsd -p1: -D ");
	(void)strcat(buf," -Tgps ");
	(void)strcat(buf,filename);
	(void)strcat(buf," > ");
	(void)strcat(buf,tmpname);
	(void)system(buf);
	erase();
	do_plot();
	box_start = scalex(500);
	box_end = box_start + scalex(5000);
	s_text("",20,40);
	s_text(msg1_ptr,20,20);
	get_common();
	range_state=1;
	cursor_x=mark_x=0;
}

static void
quit_proc()
{
	(void)unlink(tmpname);
	exit(0);
}


