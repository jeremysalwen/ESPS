/*	Copyright (c) 1987, 1988, 1989 AT&T and Entropic Speech, Inc.	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* cmap.c */
/*

This is the beginnings of a colormap editor.  This version assumes a
colormap size of CM_SIZE (must be power of 2).  Left mouse pics in the
displayed colormap at the top of the frame cause "handles" to appear
below the map.  Left pics in the region of these handles selects them;
middle pics remove the closest handle.  Movement of the mouse with left
button depressed inside the color "triangle" causes color mix to
change.  Movement of the "slider" causes the intensity of the selected
handle to change.  Selected level number and RGB intensity levels are
displayed at bottom of screen.  RGB intensity levels are linearly
interpolated between handles.  A colormap is saved by entering an
"OUTPUT file" name when the desired effect has been achieved.  An
existing colormap may be read and modified by entering its name for
the "INPUT file" item.  These colormaps are in a form acceptable to the
"wave" and "waves" programs.

To make:

cmap: cmap.o
	cc -o cmap cmap.o -lsuntool -lsunwindow -lpixrect -lm

*/

#ifndef lint
static char *sccs_id = "@(#)cmap.c	1.1 12/21/89		AT&T, ESI";
#endif

#include <stdio.h>
#include <math.h>
#include <sys/file.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <suntool/panel.h>
#include <suntool/text.h>


#define CM_SIZE 128
#define RADIUS 350
#define CM_WIDTH 1024
Panel_item file_item, infile_item, quit_item, slider;
char outputname[100] = "colormap", inputname[100] = "";
static u_char red[CM_SIZE], blue[CM_SIZE], green[CM_SIZE];
int bstart = 0; bwidth = CM_WIDTH/CM_SIZE, bheight = 75, cheight=510,
    cwidth = 1100, tox = 200, toy = 197, intensity = 1000;

typedef struct cbreak {
  int where;
  u_char r, g, b;
  struct cbreak *next, *prev;
} Cbreak;

Cbreak cb1 = {CM_SIZE -1, 0, 0, 0, NULL, NULL};
Cbreak cb0 = {0, 255, 255, 255, &cb1, NULL};
Cbreak *cbp = &cb0;
u_char curr=255, curb=255, curg=255;
Canvas canvas;
Panel panel;

Cbreak *
new_cbreak(where,r,g,b)
int where, r, g, b;
{
  Cbreak  *cb, *cb2;

  cb = &cb0;
  while(cb) {
    if(cb->where == where) {
      cb->r = r;
      cb->b = b;
      cb->g = g;
      return(cb);
    }
    if((cb->where < where) && (cb->next) && (cb->next->where > where)) {
      cb2 = (Cbreak*)malloc(sizeof(Cbreak));
      cb2->where = where;
      cb2->r = r;
      cb2->b = b;
      cb2->g = g;
      cb2->next = cb->next;
      cb->next->prev = cb2;
      cb->next = cb2;
      cb2->prev = cb;
      return(cb2);
    }
    cb = cb->next;
  }
  return(NULL);
}

static void newText(), doit(), intensity_proc(), quit_proc();

main()
{
Frame frame, frm;

cb1.prev = &cb0;
frm = window_create(NULL, FRAME, WIN_X, 10, WIN_Y, 10, 0);

panel = window_create(frm, PANEL, 0);

infile_item = panel_create_item( panel, PANEL_TEXT,
				PANEL_ITEM_Y, ATTR_ROW(0),
				PANEL_ITEM_X, ATTR_COL(0),
				   PANEL_LABEL_STRING, "INPUT file:",
				PANEL_VALUE_DISPLAY_LENGTH, 40,
				   PANEL_VALUE, inputname,
				   PANEL_NOTIFY_PROC, newText,
				   0);

file_item = panel_create_item( panel, PANEL_TEXT,
				PANEL_ITEM_Y, ATTR_ROW(1),
				PANEL_ITEM_X, ATTR_COL(0),
				   PANEL_LABEL_STRING, "OUTPUT file:",
				PANEL_VALUE_DISPLAY_LENGTH, 40,
				   PANEL_VALUE, outputname,
				   PANEL_NOTIFY_PROC, newText,
				   0);

quit_item = panel_create_item(panel, PANEL_BUTTON,
				PANEL_ITEM_Y, ATTR_ROW(2),
				PANEL_ITEM_X, ATTR_COL(0),
				PANEL_LABEL_STRING, "QUIT!",
				PANEL_NOTIFY_PROC, quit_proc,
			      0);

slider = panel_create_item(panel, PANEL_SLIDER,
				PANEL_ITEM_Y, ATTR_ROW(2),
				PANEL_ITEM_X, ATTR_COL(15),
			       PANEL_NOTIFY_LEVEL, PANEL_DONE,
			       PANEL_NOTIFY_PROC, intensity_proc,
			       PANEL_LABEL_STRING, "Brightness %",
			       PANEL_MIN_VALUE, 0,
			       PANEL_MAX_VALUE, 1000,
			       PANEL_SHOW_RANGE, TRUE,
			       PANEL_SHOW_VALUE, TRUE,
			       PANEL_SLIDER_WIDTH, 300,
			       PANEL_VALUE, 1000,
			       0);

window_fit(panel);
window_fit(frm);

frame = window_create(NULL,FRAME,WIN_X,10,WIN_Y,100,
		      FRAME_LABEL, "Colormap Generator",
		      0);
canvas = window_create(frame,CANVAS,
		      CANVAS_AUTO_SHRINK, FALSE,
		      CANVAS_AUTO_EXPAND, FALSE,
		       WIN_CONSUME_PICK_EVENTS, LOC_DRAG,
		                              WIN_IN_TRANSIT_EVENTS, 0,
		       WIN_EVENT_PROC, doit,
		       WIN_WIDTH, cwidth,
		       WIN_HEIGHT, cheight,
		       CANVAS_WIDTH, cwidth,
		       CANVAS_HEIGHT, cheight,
		       0);
window_fit(frame);
window_set(frame,WIN_SHOW,TRUE,0);
redraw_colormap();
draw_colorboxes();
draw_color_triangle();
window_main_loop(frm);

}

/*************************************************************************/
draw_color_triangle()
{
  double x, y, r, th, pi=3.1415927, dt=.01, lim, pi2;
  int i, j, k, xp, yp, xo, yo;
  Pixwin *pw;

  pw = canvas_pixwin(canvas);
  pi2 = pi*2;
  lim = pi2/6;
  x = .5 + tox;
  y = .5 + toy;
  r = RADIUS;
  /* 0.0 to -2pi/3 */
  pw_text(pw,tox-30,toy,PIX_SRC, 0, "RED");
  for(th=dt, xo = tox+RADIUS, yo = toy; th <= lim; th += dt) {
    xp = x + r * cos(th);
    yp = y + r * sin(th);
    pw_vector(pw,xo,yo,xp,yp,PIX_SRC, 255);
    xo = xp;
    yo = yp;
  }
  x = .5 + tox + RADIUS;
  y = .5 + toy;
  r = RADIUS;
  pw_text(pw,(int)x,toy,PIX_SRC, 0, "GREEN");
  for(th=dt, xo = tox, yo = toy; th <= lim; th += dt) {
    xp = x - r * cos(th);
    yp = y + r * sin(th);
    pw_vector(pw,xo,yo,xp,yp,PIX_SRC, 255);
    xo = xp;
    yo = yp;
  }
  x = .5 + tox + (RADIUS/2);
  y = .5 + toy + (RADIUS * sqrt(3.0)/2.0);
  r = RADIUS;
  /* 0.0 to -2pi/3 */
  pw_text(pw,(int)x+20,(int)y,PIX_SRC, 0, "BLUE");
  for(th=dt+lim, lim *= 2, xo = tox+RADIUS, yo = toy; th <= lim; th += dt) {
    xp = x + r * cos(th);
    yp = y - r * sin(th);
    pw_vector(pw,xo,yo,xp,yp,PIX_SRC, 255);
    xo = xp;
    yo = yp;
  }
}

/*************************************************************************/
void
newText(item, event)
     Panel_item item;
     Event *event;
{
  FILE *fdt, *fopen();
  char *name, next[50], *get_output_file_names();
  int n;
  static int pin, pout;

  if(item == file_item) {
    strcpy(outputname,(char*)panel_get_value(item));
    save_colormap(outputname);
    return;
  }

  if(item == infile_item) {
    strcpy(inputname,(char*)panel_get_value(item));
    read_colormap(inputname);
    return;
  }
}

/*************************************************************************/
save_colormap(name)
     char *name;
{
  FILE *fd, *fopen();
  int i;

  if((fd = fopen(name,"w"))) {
    for(i=0; i < CM_SIZE; i++)
      fprintf(fd,"%3d %3d %3d\n",red[i],green[i],blue[i]);
    fclose(fd);
  }
}

/*************************************************************************/
read_colormap(name)
     char *name;
{
  FILE *fd, *fopen();
  int i,r,g,b;
  char line[120];

  if((fd = fopen(name,"r"))) {
    for(i=0; i < CM_SIZE; i++) {
      if(fgets(line,120,fd)== NULL) break;
      sscanf(line,"%d%d%d",&r,&g,&b);
      red[i] = r;
      green[i] = g;
      blue[i] = b;
    }
    fclose(fd);
    reset_colormap();
    redraw_colormap();
  } else
    printf("Can't open %s for input\n",name);
}

/*************************************************************************/
void
quit_proc(item, val, event)
     Panel_item item;
     int val;
     Event *event;
{
  exit(0);
}

/*************************************************************************/
get_rgb(x,y,r,g,b)
     int x, y;
     u_char *r,*g,*b;
{
  double dr, db, dg, euclid(), rad, amax;
  int rx0, bx0, gx0, ry0, by0, gy0;

  rad = RADIUS;
  rx0 = tox;
  ry0 = toy;
  bx0 = tox + .5 + (rad/2);
  by0 = toy + .5 + ((sqrt(3.0)*rad)/2.0);
  gx0 = tox + rad;
  gy0 = toy;
  dr = euclid(rx0,ry0,x,y);
  dg = euclid(gx0,gy0,x,y);
  db = euclid(bx0,by0,x,y);
  if((dr <= rad) && (db <= rad) && (dg <= rad)) {
    dr = rad - dr;
    dg = rad - dg;
    db = rad - db;
    if(dr > dg) {
      if(dr > db) amax = dr;
      else  amax = db;
    } else {
      if(dg > db) amax = dg;
      else amax = db;
    }
    amax = 255.0/amax;
    *r = .5 + (dr*amax);
    *b = .5 + (db*amax);
    *g = .5 + (dg*amax);
    return(TRUE);
  }
  return(FALSE);
}

/*************************************************************************/
double
euclid(x,y,x1,y1)
     int x,y,x1,y1;
{
  int xd, yd;

  xd = x - x1;
  yd = y - y1;
  return(sqrt((double)((xd*xd) + (yd*yd))));
}

/*************************************************************************/
Cbreak *
get_nearest_cb(x)
     int x;
{
  int xi;
  Cbreak *cb;

  xi = (x-bstart)/bwidth;
  if((xi < 0) || (xi > CM_SIZE)) return(NULL);

  cb = &cb0;
  while(cb->next) {
    if((cb->where <= xi) && (cb->next->where >= xi)) {
      if((xi - cb->where) < (cb->next->where - xi)) return(cb);
      else return(cb->next);
    }
    cb = cb->next;
  }
  return(cb);
}

/*************************************************************************/
kill_cb(cb)
     Cbreak *cb;
{
  int amax;

  if((cb == &cb0) || (cb == &cb1)) return;

  cb->prev->next = cb->next;
  cb->next->prev = cb->prev;
  if(cbp == cb) cbp = cb->next;
  reset_current_color(cbp->r,cbp->g,cbp->b);
  free(cb);

}

/*************************************************************************/
reset_current_color(r,g,b)
     u_char r,g,b;
{
  int amax;

  if(r > g) {
    if(r > b) amax = r;
    else  amax = b;
  } else {
    if(g > b) amax = g;
    else amax = b;
  }
  intensity = (1000 * amax)/255;
  if(amax) {
    curg = ((int)g * 255)/amax;
    curb = ((int)b * 255)/amax;
    curr = ((int)r * 255)/amax;
  } else curg = curb = curr = 255;
/*  printf("r%d g%d b%d cr%d cg%d cb%d amax%d int%d\n",(int)r,(int)g,(int)b,
	 (int)curr, (int)curg, (int)curb, amax, intensity);
	 */
  panel_set(slider, PANEL_VALUE, intensity, 0);
}

/*************************************************************************/
void
doit(canvas, event, arg)
Canvas canvas;
Event *event;
caddr_t arg;
{
  Pixwin *pw;
  Rect *rec;
  int x, y, xi;
  u_char r, b, g;
  int e = event_id(event);
  Cbreak *cb;

  if(event_is_up(event)) return;

  x = event_x(event);
  y = event_y(event);
  switch(e) {
  case MS_MIDDLE:
    if((y > bheight) && ( y < (bheight << 1))) {
      if(cb = get_nearest_cb(x)) {
	kill_cb(cb);
	redraw_colormap();
	plot_bp();
      }
    }
    break;

  case MS_LEFT: case LOC_DRAG:
    if(y < bheight) {
      xi = (x - bstart)/bwidth;
      cbp = new_cbreak(xi,red[xi],green[xi],blue[xi]);
      reset_current_color(cbp->r,cbp->g,cbp->b);
      plot_bp();
      return;
    }
    if((y > bheight) && ( y < (bheight << 1))) {
      if(cb = get_nearest_cb(x)) {
	cbp = cb;
	reset_current_color(cb->r,cb->g,cb->b);
	plot_bp();
      }
      return;
    }
    if(get_rgb(x,y,&r,&g,&b)){
      curr = r;
      curb = b;
      curg = g;
      redraw_colormap();
      return;
    }

  case LOC_MOVE:
    if(y < (bheight << 1)) {
      if((x >= bstart) && (x <= CM_WIDTH)) {
	char str[100];
	
	xi = (x-bstart)/bwidth;
	sprintf(str,"Level:%3d  R:%3d  G:%3d  B:%3d",
		xi,red[xi],green[xi],blue[xi]);
	pw = canvas_pixwin(canvas);
	pw_text(pw,cwidth/2,cheight-5,PIX_SRC,0,str);
      }
    }
    break;
	
  default:
    break;
  }
  return;
}

/*************************************************************************/
plot_bp()
{
  Pixwin *pw;
  Rect * rec;
  Cbreak *cb;
  int x, y1;
  char str[100];

  pw = canvas_pixwin(canvas);
  rec = (Rect*)window_get(canvas,WIN_RECT);
  pw_rop(pw,bstart,bheight,CM_WIDTH,bheight,
	   PIX_SRC|PIX_COLOR(0),NULL,0,0);

  cb = &cb0;
  y1 = 2 * bheight/3;
  while(cb) {
    x = (cb->where * bwidth) + bwidth/2;
    pw_rop(pw,x,bheight,bwidth>>2,y1,
	   PIX_SRC|PIX_COLOR(cb->where),NULL,0,0);
    cb = cb->next;
  }
  x = (cbp->where * bwidth);
  pw_rop(pw,x,bheight,bwidth,bheight,
	 PIX_SRC|PIX_COLOR(cbp->where),NULL,0,0);
  print_maplevel(cbp);
}

/*************************************************************************/
void
intensity_proc(item, val, event)
     Panel_item item;
     int val;
     Event *event;
{

  intensity = val;
  redraw_colormap();

}

print_maplevel(cbp)
  Cbreak *cbp;
{
  Pixwin *pw;
  char str[100];

  sprintf(str,"Level:%3d  R:%3d  G:%3d  B:%3d",cbp->where,cbp->r,cbp->g,cbp->b);
  pw = canvas_pixwin(canvas);
  pw_text(pw,cwidth/2,cheight-5,PIX_SRC,0,str);
  return;
}

redraw_colormap()
{
  register int i, j,k, n;
  double r, b, g, rs, bs, gs;
  Cbreak *cb, *nb;
  char str[100];
  Pixwin *pw = canvas_pixwin(canvas);

  cbp->r = (.5 + ((double)curr * intensity)/1000.0);
  cbp->b = (.5 + ((double)curb * intensity)/1000.0);
  cbp->g = (.5 + ((double)curg * intensity)/1000.0);
  print_maplevel(cbp);
  cb = &cb0;
  nb = cb->next;
  while(nb) {
    r = cb->r;
    g = cb->g;
    b = cb->b;
    n = nb->where - cb->where;
    rs = ((double)(nb->r - r))/n;
    bs = ((double)(nb->b - b))/n;
    gs = ((double)(nb->g - g))/n;
    for(i = cb->where, j=nb->where; i < j; i++) {
      red[i] = r + .5;
      green[i] = g + .5;
      blue[i] = b + .5;
      r += rs;
      g += gs;
      b += bs;
    }
    cb = nb;
    nb = cb->next;
  }
  pw_setcmsname(pw, "waves_cmap");
  pw_putcolormap(pw,0,CM_SIZE,red,green,blue);
}


reset_colormap()
{
  register int i, j,k, n;

  for(i=0; i < CM_SIZE; i++) {
    if(! new_cbreak(i,red[i],green[i],blue[i])) {
      printf("Problems setting up new cbreaks in reset_colormap()\n");
      exit(-1);
    }
  }
  plot_bp();
}

draw_colorboxes()
{
  Pixwin *pw;
  int i, loc, k;

  pw = canvas_pixwin(canvas);
  for(i=0, loc=bstart ;i < CM_SIZE; i++, loc += bwidth)
    pw_rop(pw,loc,0,bwidth,bheight,PIX_SRC|PIX_COLOR(i),NULL,0,0);
}
