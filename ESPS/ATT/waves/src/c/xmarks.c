/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution,
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
 * Written by:  Mark Liberman, Mark Beutnagel, AT&T Bell Laboratories
 * Checked by:
 * Revised by:  Rodney Johnson, Entropic Research Laboratory
 *              (convert to XView)
 *              David Talkin (update attachment IPC)
 *              Alan Parker  (ditto)
 *
 * ``xwaves'' attachment for marking speech files
 * This version, based on "nmark" allows insertion of new markers.
 */

static char *sccs_id = "@(#)xmarks.c	1.16 9/28/98 ERL/ATT";

#include <esps/esps.h>
#include <xview/cursor.h>
#include <xview/cms.h>
#include <xview/xview.h>
#ifdef LINUX_OR_MAC
#include <xview/panel.h>
#endif
#include <esps/exview.h>
#include <wave_colors.h>
#include <w_lengths.h>
#include <marker.h>


#define SYNTAX USAGE("xmarks [-w wave_pro] [-n<waves or other host program>] [-c<registry name of host>] ");

int     debug_level = 0;
int     command_paused = 0;
extern char ok[], null[];	/* in message.c */

int     use_dsp32, da_location = 0;              /*referenced in globals.c*/
double  image_clip = 7.0, image_range = 40.0;


Frame	base_frame = XV_NULL;
Frame   daddy = XV_NULL;
Panel	panel = XV_NULL;
Canvas	labelsw = XV_NULL;
Pixwin	*labelpw;
char *host = "waves", *thisprog = "xmarks";
#if !defined(hpux) && !defined(DS3100) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *malloc();
#endif
Xv_Font	fonts[3];
char	*font_styles[] = {FONT_STYLE_BOLD, FONT_STYLE_NORMAL, FONT_STYLE_BOLD};
int	font_sizes[] = {10, 24, 24};
extern char *Version;

/*
char *font_names[] = {
  "/usr/lib/vfont/delegate.b.12",
  "/usr/lib/vfont/delegate.i.12",
  "/usr/lib/vfont/delegate.r.12"
  };
*/
/*
char *font_names[] = {
  "/usr/lib/fonts/fixedwidthfonts/cour.b.10",
  "/usr/lib/fonts/fixedwidthfonts/cour.r.24",
  "/usr/lib/fonts/fixedwidthfonts/cour.b.24"
  };
*/

Sentence *sentence=NULL;
Marker *current_m = NULL;
int insert_before = 1;		/* insert where? 1=before 0=after */

Rect	*label_rect = NULL;

FILE	*infd = NULL;			/* for reading in markers */

/* Panel_setting read_font(), get_string(); */

Marker	*first_free_marker();
void 	prompt_for_label();

extern char *registry_name;

/* Panel_item quit_item; */
void quit_proc();

Objects base_obj, file_obj;	/* program object and file object */

Notify_value iocatcher();  /* gets input while in window loop */

static double sec_cm, start;
static int  width, height, loc_x, loc_y;
static double m_time, rstart, rend;
static char name[NAMELEN], file[NAMELEN], sigfile[NAMELEN];
static int color;
static Selector a9 = {"sec/cm", "%lf", (char*)&sec_cm, NULL},
       a8 = {"start", "%lf", (char*)&start, &a9},
       a7 = {"width", "%d", (char*)&width, &a8},
       a6 = {"height", "%d", (char*)&height, &a7},
       a5 = {"loc_y", "%d", (char*)&loc_y, &a6},
       a4 = {"loc_x", "%d", (char*)&loc_x, &a5},
       b1 = {"time", "%lf", (char*)&m_time, &a4},
       b2 = {"name", "%s", name, &b1},
       b3a = {"file", "%s", file, &b2},
       b3b = {"signal", "%s", sigfile, &b2},
       b3 = {"rstart", "%lf", (char*)&rstart, &b3b},
       b4 = {"rend", "%lf", (char*)&rend, &b3};

/* These are needed by the inter-process communications stuff. */
/*********************************************************************/
char *get_receiver_name(ob)
     Objects *ob;
{
  return(ob->name);
}

/*********************************************************************/
char *get_methods(ob)
     Objects *ob;
{
  extern Methods base_methods, file_methods;

  if (ob)return((char *)(ob->methods));
  return((char*)&base_methods);
}

/*********************************************************************/
char *get_receiver(str)
     char *str;
{
  Objects *ob;
  static  char name[NAMELEN];
  extern Objects base_obj;


  ob = &base_obj;
  if (str && strlen(str)) {
    sscanf(str, "%s", name);
    while (ob) {
      if (ob->name &&
	 (! strcmp(ob->name, name))) {
	   return((char*)ob);
	 }
      ob = ob->next;
    }
  }
  return(NULL);
}

/*********************************************************************/
init_object(o)
     Objects *o;
{
  if(o) {
    o->name[0] = 0;
    o->canvas = NULL;
    o->methods = NULL;
    o->next = NULL;
    o->pcanvas = NULL;
    o->x_off = 0;
    o->xloc = 0;
    o->yloc = SCREEN_HEIGHT;
    o->width = SCREEN_WIDTH;
    o->height = 100;
    o->color = 0;
    o->sec_cm = .02;
    o->start = 0.0;
    o->cursor = 0.0;
    o->time = 0.0;
  }
}


/*********************************************************************/
char *generate_startup_command(registry_name)
     char *registry_name;
{
  static char com[MES_BUF_SIZE];

  sprintf(com,"add_op name %s op #send function %s registry %s command _name mark signal _file time _cursor_time",
	   basename(thisprog), thisprog, registry_name);
  return(com);
}

/*********************************************************************/

extern int fullscreendebug;	/* an xview global hack to prevent grabs */

/*********************************************************************/
main(ac, av)
  int	ac;
  char	**av;
{
  int i;
  extern Objects base_obj, file_obj;
  extern Methods base_methods;
  char mess[MES_BUF_SIZE];
  extern char *optarg, *wave_pro;
  extern int	attachment,	/* Is this program an attachment? */
                optind;
  extern Methods  file_methods;
  int rem_args; /* no. of args after getopt processing */
  int		ch;		/* option letter read by getopt */

  char *server_name = "xwaves";
  extern Display *X_Display;
  extern Window comm_window;

  init_object(&file_obj);

  fullscreendebug=1;	/* this global inhibits some server grabs that
			   cause problems on the SGI */
  thisprog = av[0];

  while ((ch = getopt(ac, av, "w:n:c:")) != EOF)
    switch (ch)
      {
      case 'n':
	host = optarg;
	break;
      case 'w':
	wave_pro = optarg;
	break;
      case 'c':
	server_name = optarg;
        break;
      default:
	SYNTAX
	  if(debug_level)
	    for(ch = 0; ch < ac; ch++)
	      fprintf(stderr,"%s ",av[ch]);
	fprintf(stderr,"\n");
	exit(-1);
      }

  xv_init(XV_INIT_ARGC_PTR_ARGV, &ac, av, 0);

  attachment = TRUE;
  get_globals();

  get_color_depth();
  setup_colormap();

/* initialize fonts */
  for (i = 0; i < 3 ; i++)
  {
    fonts[i] = (Xv_Font) xv_find(XV_NULL, FONT,
			FONT_FAMILY,	    FONT_FAMILY_DEFAULT_FIXEDWIDTH,
			FONT_STYLE,	    font_styles[i],
			FONT_SIZE,	    font_sizes[i],
			0);
    if (fonts[i] == XV_NULL){
      fprintf(stderr, "can't open font %d\n", i);
      kill_proc();
    }
  }

/* set up window stuff */

  daddy = base_frame = xv_create(XV_NULL, FRAME,
			     XV_X, 160,
			     XV_Y, 0,
			     XV_WIDTH, 980,
			     FRAME_ARGS, ac, av,
			     XV_LABEL, "marker",
			     0);

  /* set up communications with the host (xwaves) */
  if (!setup_attach_comm(base_frame, server_name, "xmarks")) {
      fprintf(stderr, "Failed to setup ipc communications\n");
      exit(0);
  }
  sprintf(mess, "Marker %s (%s)", Version, registry_name);
  xv_set(base_frame, XV_LABEL, mess, NULL);

  send_start_command(generate_startup_command(registry_name));

  set_blowup_op();

  create_panel_subwindow();
  create_label_subwindow();

  exv_attach_icon(base_frame, ERL_NOBORD_ICON, "xmarks", TRANSPARENT);

  window_fit(base_frame);

/* set up object */

  base_obj.next = &file_obj;
  file_obj.next = NULL;
  base_obj.canvas = file_obj.canvas = labelsw;
  base_obj.pcanvas = file_obj.pcanvas = NULL;
  base_obj.methods = &base_methods;
  file_obj.methods = &file_methods;
  strcpy(base_obj.name, av[0]);

  xv_main_loop(base_frame);

  exit(0);
}

/*********************************************************************/
set_blowup_op()
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"set blowup_op %s",basename(thisprog));
  mess_write(mess);
}

/*********************************************************************/
void
quit_proc()
{
  print_markers(sentence, stdout);
  cleanup();
  kill_proc();
}

/*********************************************************************/
kill_proc()
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"%s disconnect function %s\n",host,thisprog);
  terminal_message(mess);
  terminate_communication(registry_name);
  exit(0);
}

/*********************************************************************/
create_panel_subwindow()
{
  void quit_proc(), next_word_proc(), last_word_proc(), next_mark_proc(),
       last_mark_proc(), insert_where_proc(), change_label_proc(),
       insert_label_proc(), un_set_proc(), delete_proc();

  panel = xv_create(base_frame, PANEL, 0);

  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"Quit",
			PANEL_NOTIFY_PROC,	quit_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"NextWord",
			PANEL_NOTIFY_PROC,	next_word_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"LastWord",
			PANEL_NOTIFY_PROC,	last_word_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"NextMark",
			PANEL_NOTIFY_PROC,	next_mark_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"LastMark",
			   PANEL_NOTIFY_PROC,	last_mark_proc,
			   0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"ChangeMark",
			PANEL_NOTIFY_PROC,	change_label_proc,
			0);
  (void) xv_create(panel, PANEL_CYCLE,
			PANEL_DISPLAY_LEVEL,	PANEL_CURRENT,
			PANEL_LABEL_STRING,	"Where?",
			PANEL_CHOICE_STRINGS,
			    "after",
			    "before",
			    0,
			PANEL_VALUE,		insert_before,
			PANEL_NOTIFY_PROC,	insert_where_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"AddMark",
			PANEL_NOTIFY_PROC,	insert_label_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"UnSet",
			PANEL_NOTIFY_PROC,	un_set_proc,
			0);
  (void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"Delete",
			PANEL_NOTIFY_PROC,	delete_proc,
			0);

    window_fit_height(panel);
}

/*********************************************************************/
create_label_subwindow()
{
  labelsw = xv_create(base_frame, CANVAS,
			 WIN_BELOW, panel,
			 XV_HEIGHT, 120,
			 0);
  window_fit(labelsw);
  labelpw = canvas_pixwin(labelsw);
}

/*********************************************************************/
void
next_word_proc(){
  Word *w;
  if(current_m && (w = current_m->word)) {

    if (w->right != NULL && w->right->first != NULL){
      current_m = w->right->first;
      show_markers(&labelsw, sentence, current_m, fonts);
    }
  }
}

/*********************************************************************/
void
last_word_proc(){
  Word *w;
  if(current_m && (w = current_m->word)) {
    if (w->left != NULL && w->left->first != NULL){
      current_m = w->left->first;
      show_markers(&labelsw, sentence, current_m, fonts);
    }
  }
}

/*********************************************************************/
void
next_mark_proc(){
  Marker *m;
  if(current_m && (m = current_m->right)) {
    current_m = m;
    show_markers(&labelsw, sentence, current_m, fonts);
  }
}

/*********************************************************************/
void
last_mark_proc(){
  Marker *m;
  if(current_m && (m = current_m->left)) {
    current_m = m;
    show_markers(&labelsw, sentence, current_m, fonts);
  }
}

/*********************************************************************/
void
insert_where_proc(item)		/* insert before or after? */
	Panel_item item;
{
    insert_before = (int) xv_get(item, PANEL_VALUE);
}

/*********************************************************************/
void
change_label(p)
    char    *p;
{
    (void) xv_get(fonts[1], FONT_STRING_DIMS,	current_m->label,
						&current_m->bound);
    show_markers(&labelsw, sentence, current_m, fonts);
}

/*********************************************************************/
void
change_label_proc(item)		/* change label of current marker */
    Panel_item	item;
{
    int		x, y;
    Rect	rect;

    frame_get_rect(base_frame, &rect);
    x = rect.r_left + 250;
    y = rect.r_top - 35;

    prompt_for_label(current_m, x, y, change_label);
}

/*********************************************************************/
void
insert_label(p)
    char    *p;
{
    Marker  *m = (Marker *) p;

    if (insert_before){
	insert_marker(current_m->left, m, current_m);
	m->word = current_m->word;
	if (m->word->first==current_m) m->word->first=m;
    }
    else{
	insert_marker(current_m, m, current_m->right);
	m->word = current_m->word;
	if (m->word->last==current_m) m->word->last=m;
    }

    (void) xv_get(fonts[1], FONT_STRING_DIMS,	m->label,
						&m->bound);
    if (insert_before) current_m=m;
    show_markers(&labelsw, sentence, current_m, fonts);
}

/*********************************************************************/
void
insert_label_proc(item)		/* insert a new marker label */
    Panel_item	item;
{
    Marker	*m = get_marker();
    int		x, y;
    Rect	rect;

    frame_get_rect(base_frame, &rect);
    x = rect.r_left + 250;
    y = rect.r_top - 35;

    if (m)
	prompt_for_label(m, x, y, insert_label);
}

/*********************************************************************/
void
un_set_proc(){
  sendtc(&file_obj, "unmark", current_m->time, MARKER_COLOR);
  current_m->time = -1.;
  replot(&file_obj);
}

/*********************************************************************/
void
delete_proc(){
  Marker *delete_marker();
  sendtc(&file_obj, "unmark", current_m->time, MARKER_COLOR);
  current_m = delete_marker(current_m);
  show_markers(&labelsw, sentence, current_m, fonts);
}

#define MY_SPACE 15

/*********************************************************************/
show_markers(sw, s, m, fonts)
  Canvas	*sw;
  Sentence	*s;
  Marker	*m;
  Xv_Font	*fonts;
{
  Rect *m_rect, n_rect;
  Pixwin *pw;
  register int xval, yval, sum, count, fval, total_width;
  register Word *current_w;
  register Marker *mptr, *start_mark;

  if (s == NULL) return;
  if (m == NULL) return;
  pw = canvas_pixwin(*sw);
  m_rect = (Rect *) xv_get(*sw, WIN_RECT, 0);
  n_rect = *m_rect;
  n_rect.r_top -= 30;
  n_rect.r_height += 30;

  erase(pw, &n_rect);
  xval = m_rect->r_width/2 - s->bound.width/2;
  if (xval < 0) xval = 0;
  yval = m_rect->r_top;
  pw_text(pw, xval, yval, PIX_SRC, fonts[0], s->text);
  current_w = m->word;

  xval = m_rect->r_width/2 - current_w->bound.width/2;
  yval += 30;
  pw_text(pw, xval, yval, PIX_SRC, fonts[1], current_w->spelling);
  sum = count = 0;
  for (mptr = current_w->first; mptr && mptr->left != current_w->last;
	 mptr = mptr->right)
  {
    sum += mptr->bound.width;
    count++;
  }
  yval += 35;
  total_width = sum + count*MY_SPACE;
  start_mark = current_w->first;
  while ((total_width > m_rect->r_width) &&
	(start_mark != m) &&
	(start_mark->right != m)){
    total_width -= (start_mark->bound.width + MY_SPACE);
    start_mark = start_mark->right;
    if (start_mark == NULL)break;
  }

  xval = m_rect->r_width/2 - total_width/2;
  if (xval <0) xval = 0;
  for (mptr=start_mark; mptr && mptr->left != current_w->last;
        mptr = mptr->right)
  {
    if (xval > m_rect->r_width)break;
    if (mptr == m)fval = 2;
/*    else if (mptr->time > 0.)fval = 1; */
    else fval = 1;
    pw_text(pw, xval, yval, PIX_SRC, fonts[fval], mptr->label);
    xval += (mptr->bound.width + MY_SPACE);
  }
  replot(&file_obj);
}

/*********************************************************************/
get_bounds(s, f, f1)
  Sentence	*s;
  Xv_Font	f, f1;
{
  Word *w;
  Marker *m;


  (void) xv_get(f1,	FONT_STRING_DIMS,	s->text,
						&s->bound);

  for (w = s->first; w && w->left != s->last; w = w->right){

    (void) xv_get(f,	FONT_STRING_DIMS,	w->spelling,
						&w->bound);
    for (m = w->first; m && m->left != w->last; m = m->right){

      (void) xv_get(f,	FONT_STRING_DIMS,	m->label,
						&m->bound);
    }
  }
}

/*********************************************************************/
erase(p, r)
  Pixwin *p;
  Rect *r;
{
  pw_writebackground(p, r->r_left, r->r_top, r->r_width, r->r_height, PIX_CLR);
}

/*********************************************************************/
void
prompt_for_label(m, x, y, continuation)	/* get a marker label from user */
    Marker  *m;
    int	    x, y;
    void    (*continuation)();
{
    extern void	popup_get_string();

    if (m)
	popup_get_string("Label: ", m->label, NLABEL-1, x, y,
			 (char *) m, continuation);
}

/*********************************************************************/
time_to_x(ob, time)
    Objects *ob;
    double time;
{
  return((int)(.5 + (PIX_PER_CM * ((time - ob->start)/ob->sec_cm))) +
	 ob->x_off);
}

/*********************************************************************/
sendtc(ob, cmd, time, color)
	Objects *ob;
	char *cmd;
	double time;
	int color;
{
  char mes[MES_BUF_SIZE];

  if (!ob) return(0);
  sprintf(mes, "%s %s time %f color %d\n", ob->name, cmd, time, color);
  mess_write(mes);
  return(TRUE);
}

/*********************************************************************/
replot(ob)
    Objects	*ob;
{
    Canvas	canvas;
    Marker	*m = current_m;
    Rect	*rec;
    Font_string_dims
		str_dims;
    Pixwin	*pw;
    Xv_Font	pf;
    int		top, bot;
    int		x, xlab, y, str_width, ystep;
    int		oldx[100], line, maxline;
    int		mcolor, tcolor;
    double	end;

    /* window frame has already been repositioned, */
    /* display attributes are up to date */

    if (!ob) return 0;
    if (!m) return 0;
    while (m->left) m = m->left;
    canvas = ob->pcanvas;
    rec = (Rect *) xv_get(canvas, WIN_RECT);
    pw = canvas_pixwin(canvas);
    pf = (Xv_Font) xv_get(ob->canvas, XV_FONT);
    if (!(canvas && rec && pw) || (pf == XV_NULL)) return 0;

    end = ob->start + ((ob->sec_cm * rec->r_width) / PIX_PER_CM);
    top = 15;
    bot = rec->r_height;

    ystep = (int) xv_get(pf, FONT_DEFAULT_CHAR_HEIGHT);
    ystep += (int) (ystep * 0.2);

    maxline=((rec->r_height-top)/ystep);
    for (line = 0; line < maxline; line++)
	oldx[line]= -9999;

    /* now draw labels & marks */

    pw_rop(pw, 0, 0, rec->r_width, rec->r_height,
	   PIX_SRC|PIX_COLOR(0), NULL, 0, 0);

    for ( ; m; m = m->right)
    {
	if (m->time < 0. || m->time < ob->start || m->time > end) continue;
	mcolor = MARKER_COLOR; tcolor = TEXT_COLOR;
	if (m==current_m){
	    mcolor=RETICLE_COLOR; tcolor=CURSOR_COLOR;
	}
	x = time_to_x(ob, m->time);
	pw_vector(pw, x, top, x, bot, PIX_SRC, mcolor);  /* vertical bar */

	(void) xv_get(pf, FONT_STRING_DIMS,	m->label,
						&str_dims);
	str_width = str_dims.width;
	
	if ((xlab = x - str_width) < 1) xlab = 1;
	for (line = 0; line < maxline && oldx[line] > xlab; line++) { }
	oldx[line] = xlab + str_width;
	y = top + line*ystep;
	pw_text(pw, xlab, y, PIX_COLOR(tcolor)|PIX_SRC, 0, m->label);
    }

    return 1;
}

/**********************************************************************/
get_decor_size(height, width)
     int *height, *width;
{
  static Rect  rect, *rec;
  int hp;

  if(labelsw & panel && base_frame) {
    frame_get_rect(base_frame, &rect);
    rec = (Rect*)xv_get(panel, WIN_RECT);
    hp = rec->r_height;
    rec = (Rect*)xv_get(labelsw, WIN_RECT);
    *height = rect.r_height - rec->r_height - hp;
    *width = rect.r_width - rec->r_width;
    /* One sleazy hack deserves another... */
    if((*height < 0) || (*width < 0))
      *height = *width = 0;

    if(debug_level)
      fprintf(stderr,"decor: h:%d w:%d hf:%d hp:%d hl:%d\n",
	      *height, *width,rect.r_height,hp,rec->r_height);
    return(TRUE);
  } else
    return(FALSE);
}


/*********************************************************************/
void
reposition(ob)
    Objects	*ob;
{
    Frame	frm;
    Rect	rect;
    int         height=0, width=0;

    get_decor_size(&height, &width);
    frm = (Frame)xv_get(ob->pcanvas, XV_OWNER);
    frame_get_rect(frm, &rect);
    rect_construct(&rect, ob->xloc + (width/2), ob->yloc + ob->height + height,
		   ob->width, rect.r_height);
    frame_set_rect(frm, &rect);
}

/*********************************************************************/
char *
meth_replot(ob,str)
     Objects *ob;
     char *str;
{
  if(ob && str && ob->canvas) {
    Frame frm;
    Rect	rect;

    initial_bad_values();

    if(get_args(str,&b4) &&
       have_all_display_attributes(ob)) {
      reposition(ob);
      replot(ob);
      return(ok);
    } else
      get_display_attributes(ob);

  } else
    fprintf(stderr,"Null object or arg list to meth_replot()\n");
  return(null);
}

/*********************************************************************/
get_display_attributes(ob)
     Objects *ob;
{
  char mes[MES_BUF_SIZE];
  int n, id;

  if(ob && ob->name && *ob->name) {
    start = sec_cm = -1.0;
    width = height = loc_x = loc_y = -1;
    id = set_return_value_callback(meth_replot, ob);
    sprintf(mes,"%s get attributes display function %s return_id %d\n",ob->name,
	      thisprog, id);
    mess_write(mes);
    return(TRUE);
  }
  return(FALSE);
}

/***********************************************************************/
char *
meth_return(ob,str)
     Objects *ob;
     char *str;
{
  if(str && *str) {
    int id;
    char *get_next_item();

    sscanf(str,"%d",&id);
    do_return_callback(id,get_next_item(str));
    return(ok);
  }
  return(null);
}

/*********************************************************************/
Methods
    fm4 = {"kill", meth_kill, NULL },
    fm3 = {"redisplay", meth_replot, &fm4},
    fm2 = {"mark", meth_mark, &fm3 },
    file_methods = {"quit", meth_quit, &fm2 };

Methods  bm8 = {"completion", meth_return, NULL},
    bm5 = {"read", meth_read, &bm8},
    bm4 = {"kill", meth_kill, &bm5 },
    bm3 = {"write", meth_write, &bm4},
    bm2 = {"make", meth_make, &bm3 },
    base_methods = {"quit", meth_quit, &bm2 };

/*********************************************************************/
int
send_preset_marks(s)
	Sentence *s;
{
    Marker *m;

    if (!s || !s->first || !(m=s->first->first)) return 0;
    if (!(*file_obj.name)) return 0;

    for ( ; m; m = m->right) {
	if (m->time >= 0.)
	    sendtc(&file_obj, "mark", m->time, MARKER_COLOR);
    }
    return 1;
}

/*********************************************************************/
void
replot_proc(canvas, pw, repaint_area)
    Canvas	canvas;
    Xv_Window	pw;
    Rectlist	*repaint_area;
{
    Objects	*ob = (Objects *) xv_get(canvas, WIN_CLIENT_DATA);

    if (!get_display_attributes(ob)) return;
    reposition(ob);
    replot(ob);
}

/*********************************************************************/
char *
meth_make(ob, args) /* really "rename" not "make" */
    Objects	    *ob;
    char	    *args;
{
    extern Objects  file_obj;	/* file object */
    static char	    newname[NAMELEN];
    static Selector a0 = {"name", "%s", newname, &b4};
    Frame	    plot_frame;
    Xv_Cursor	    cursor;
    Rect	    rect;
    char	    title[MES_BUF_SIZE];
    extern int      cmap_depth;

    if (!get_args(args, &a0) || !*newname) return(null);
    if (!strcmp(newname, file_obj.name)) return(ok);
    strncpy(file_obj.name, newname, NFILE);
    sprintf(title, "marker display for object %s", file_obj.name);

    plot_frame = xv_create(XV_NULL, FRAME,
			XV_LABEL,	title,
			0);

    rect_construct(&rect, file_obj.xloc, file_obj.yloc + file_obj.height,
			file_obj.width, 50);
    frame_set_rect(plot_frame, &rect);
    xv_set(plot_frame,	XV_HEIGHT,	file_obj.height,
			0);

    exv_attach_icon(plot_frame, ERL_NOBORD_ICON,
			basename(file_obj.name), TRANSPARENT);

    file_obj.pcanvas = xv_create(plot_frame, CANVAS,
			XV_X,			0,
			XV_Y,			0,
/*                        WIN_DEPTH,              cmap_depth, */
			CANVAS_RETAINED,	FALSE,
			CANVAS_FIXED_IMAGE,	FALSE,
			CANVAS_AUTO_SHRINK,	TRUE,
			CANVAS_AUTO_EXPAND,	TRUE,
			XV_WIDTH,		WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		WIN_EXTEND_TO_EDGE,
		       CANVAS_PAINTWINDOW_ATTRS,
		       WIN_DYNAMIC_VISUAL, TRUE,
                        WIN_DEPTH,              cmap_depth,
		       0,
			CANVAS_REPAINT_PROC,	replot_proc,
			WIN_CLIENT_DATA,	&file_obj,
			WIN_SHOW,		TRUE,
			0);

    xv_set(canvas_paint_window(file_obj.pcanvas),
			WIN_BIT_GRAVITY,	ForgetGravity,
			WIN_CONSUME_EVENTS,
			    LOC_DRAG,
			    LOC_MOVE,
			    WIN_IN_TRANSIT_EVENTS,
			    0,
			0);

    cmap(file_obj.pcanvas);

    xv_set(plot_frame,	WIN_SHOW,		TRUE,
			0);

    get_display_attributes(&file_obj);

    return(ok);
}

/**********************************************************************/
initial_bad_values()
{
  m_time = start = sec_cm = -1.0;
  color = -1;
  width = height = loc_x = loc_y = -123;
  *name = 0;
  *file = 0;
}

/*********************************************************************/
have_all_display_attributes(ob)
     Objects *ob;
{
  if(ob && (width > 0) &&
     (height > 0) &&
     (start != -1.0) &&
     (sec_cm > 0) &&
     (loc_x != -123) &&
     (loc_y != -123)) {
    ob->width = width;
    ob->height = height;
    ob->start = start;
    ob->sec_cm = sec_cm;
    ob->xloc = loc_x;
    ob->yloc = loc_y;
    return(TRUE);
  } else
    return(FALSE);
}

/*********************************************************************/
char *
meth_read(ob, args) /* read a new mark list */
     Objects *ob;
     char *args;
{
  static char name[NAMELEN];
  static Selector a1 = {"file", "%s", name, NULL};

  *name = 0;
  if (get_args(args, &a1) && *name){
    if ((infd=fopen(name, "r"))==NULL){
      fprintf(stderr, "can't open marker file %s\n", name);
      return(null);
    }

    init_markers();
    sentence = read_markers(infd);
    fclose(infd);
    send_preset_marks(sentence);

    /* display markers */
    get_bounds(sentence, fonts[1], fonts[0]);
    current_m = first_free_marker(sentence);
    show_markers(&labelsw, sentence, current_m, fonts);
    return(ok);
  }
  else return(null);
}

/*********************************************************************/
char *
meth_write(ob, args) /* write out current mark list */
     Objects *ob;
     char *args;
{
  FILE *outfd;
  static char name[NAMELEN];
  static Selector a1 = {"file", "%s", name, NULL};

  *name = 0;
  if (get_args(args, &a1) && *name){
    outfd = fopen(name, "w");
    if (outfd == NULL){
      printf("meth_write: can't open %s\n", name);
      return(null);
    }
    print_markers(sentence, outfd);
    fclose(outfd);
    return(ok);
  }
  return(null);
}

/*********************************************************************/
/* responds to the "mark" message: set time of current label */
char *
meth_mark(ob, args)
     Objects *ob;
     char *args;
{
  Marker *m;

  m_time = 0.0;
  if (!current_m) {
	fprintf(stderr, "no markers defined yet.\n");
  }
  else if (get_args(args, &b4)) {
    if(current_m->time > 0.)
      sendtc(ob, "unmark", current_m->time, MARKER_COLOR);
    sendtc(ob, "mark", m_time, MARKER_COLOR);
    current_m->time = m_time;
    if((m=current_m->right) != NULL) current_m = m;
    show_markers(&labelsw, sentence, current_m, fonts);
    return(ok);
  }
  return(null);
}

/*********************************************************************/
char *
meth_kill(ob, args)
     Objects *ob;
     char *args;
{
  Rect *m_rect, n_rect;
  Pixwin *pw;
  Frame pfrm;

  *file = 0;
  get_args(args, &b4);
  if (*file)			/* if a particular signal file was specified */
    return(ok);	    /* assume others remain in the display ensemble */
  if (ob==&base_obj) ob=base_obj.next;
  if (!ob) return(null);
  pw = canvas_pixwin(ob->canvas);
  m_rect = (Rect *) xv_get(ob->canvas, WIN_RECT, 0);
  n_rect = *m_rect;
  n_rect.r_top -= 30;
  n_rect.r_height += 30;
  erase(pw, &n_rect);
  if (pfrm=(Frame)xv_get(file_obj.pcanvas, XV_OWNER)){
      xv_set(pfrm, FRAME_NO_CONFIRM, TRUE, 0);
      xv_destroy_safe(pfrm);
  }
  ob->pcanvas = NULL;
  *ob->name = (char)0;
  return(ok);
}

/*********************************************************************/
waves_send(name,command,color,time)
     char *name, *command;
     int color;
     double time;
{
  char mes[MES_BUF_SIZE];

  sprintf(mes,"%s %s time %f color %d\n",name,command,time,color);
  mess_write(mes);
  return(TRUE);

}

/*********************************************************************/
cleanup()
{
  int dumcol=1;
  double dumtime = 0.0;

  print_markers(sentence, stdout);
  if(file_obj.name && file_obj.name[0])
    waves_send(file_obj.name,"unmark all t",dumcol,dumtime);
}

/*********************************************************************/
char *
meth_quit(ob, args)
     Objects *ob;
     char *args;
{
  cleanup();
  terminate_communication(registry_name);
  exit(0);
}

/*********************************************************************/
Objects *
make_new_object(str)
     char *str;
{
    return(NULL);
/*
  char name[NFILE];
  sscanf(str, "%s", name);
  if (strlen(name)>=NFILE) name[NFILE-1]=(char)0;
  strncpy(file_obj.name, name, NFILE);
  return(&file_obj);
*/
}

/*********************************************************************/
char *exec_waves(str)
     char *str;
{
  extern char *dispatch();
  return(dispatch(str));
}
