/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* xchartprocs.c */
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
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xchartprocs.c	1.7	9/26/95	ATT/ERL";


/*
These routines are designed to manipulate displays of "charts."  A
chart is a collection of segment hypotheses, E.g. word hypotheses.  It
is assumed that the hypotheses for a given symbol are non-overlapping in
time.  Other than this, no assumptions are made about the number or
ordering of the hypotheses in the input file.  The input file is a
regular ASCII file with a header and one or more lines containing
segment hypotheses.  The file format is as follows:

   Optional header elements [default value; explanation]:
type		[1; 1 ==> hypothesis bounds in seconds; 2 ==> samples]
frequency	[8000; specifies PCM sampling frequency when type == 2]
color		[124; specifies color of plotted text in chart]
font		[system default font; specifies font for chart display]

# [# terminates header; must be present with or without other header elements.]

   Any number of segment hypotheses formatted as [ID T1 T2 Likelihood Text]:
12	0.234	0.456	0.77		foobar
12	0.567	1.987	0.97		foobar
15	0.267	1.456	0.27		barfoo
	.
	.
	.

The decimal points are optional and the numbers may have any number of
significant figures.  The ID and "likelihood" values are not currently used
and may have any numerical value.
*/

#include <Objects.h>
#include <labels.h>
#include <charts.h>
#include <xview/font.h>
#define _NO_PROTO
#include <esps/epaths.h>

extern int debug_level;
extern 	double x_to_time();

Wobject *charts=NULL;
void chart_menu_proc();

/*********************************************************************/
Menu make_chart_menu()
{
  char text[256];
  int i, j, nitems, itsok = FALSE;
  int rows, cols;
  Menu m, make_menu();
  extern char chartmenu[];
  
  nitems = 0;
/*
  if(!(m = make_menu(chartmenu, &nitems, chart_menu_proc))) {
    printf("Couldn't find chart menu file (%s); trying default\n",chartmenu);
    m = make_menu(full_path("WAVES_MISC","chartmenu"),&nitems,
		  chart_menu_proc);
   }
*/

    if (!FIND_WAVES_MENU(chartmenu,chartmenu)) {
      (void) fprintf(stderr, "xchart: couldn't find chartmenu %s\n", chartmenu);
      return (FALSE);
    }

    m = make_menu(chartmenu,&nitems, chart_menu_proc);

  if(!(m && nitems))
    printf("Problems getting chart menu (n:%d)\n",nitems);
  return(m);
}
  
/*********************************************************************/
Wobject *new_wobject(ob)
     Objects *ob;
{
  if(ob && ob->name) {
    Wobject *w = (Wobject*)malloc(sizeof(Wobject));
    if(w) {
      w->obj = ob;
      w->label_loc = 0;	/* 0==>top; 1==>bottom; 3==>off */
      w->label_size = 80;	/* pixel height of label area */
      w->charts = NULL;
      w->next = charts;
      w->menu = make_chart_menu();
      charts = w;
      return(w);
    } else
      printf("Allocation failure in new_wobject()\n");
  } else
    printf("Bogus Objects passed to new_wobject()\n");
  return(NULL);
}

/**********************************************************************/
kill_chartfile(name,ob)
     char *name;
     Objects *ob;
{
  Wobject *w;

  if(name && *name && ob && (w = get_wobject(ob))) {
    Chartfile *c = w->charts, *cn;
    if(c) {
      if(!strcmp(c->chart_name,name)) {
	w->charts = c->next;
	free_chartfile(c);
	return(TRUE);
      }
      while(c->next) {
	if(!strcmp(c->next->chart_name,name)) {
	  cn = c->next;
	  c->next = cn->next;
	  free_chartfile(cn);
	  return(TRUE);
	}
	c = c->next;
      }
    }
  }
  return(FALSE);
}

/**********************************************************************/
kill_wobject(w)
     Wobject *w;
{
  if(w) {
    Chartfile *c = w->charts, *cn;
    Wobject *wo = charts;
    
    if(wo) {			/* unlink it from the list */
      if(wo == w)
	charts = w->next;
      else
	while(wo->next) {
	  if(wo->next == w) {
	    wo->next = w->next;
	    break;
	  }
	  wo = wo->next;
	}
    }
    while(c) {			/* kill all its chartfiles */
      cn = c->next;
      free_chartfile(c);
      c = cn;
    }
    if(w->menu) {
      menu_destroy(w->menu);
    }
    free(w);
  }
}

/*********************************************************************/
free_chartfile(c)
     Chartfile *c;
{
  if(c) {
    Word *word, *wn;
    Hit *hit, *hitn;
    
    if(c->sig_name) free(c->sig_name);
    if(c->chart_name) free(c->chart_name);
    if(c->comment) free(c->comment);
    if((word = c->first_word)) {
      do {
	if(word->str) free(word->str);
	hit = word->hits;
	while(hit) {
	  hitn = hit->next;
	  free(hit);
	  hit = hitn;
	}
	wn = word->next;
	free(word);
	word = wn;
      } while(word != c->first_word);
    }
  }
}

/*********************************************************************/
plotting_labels(ob)
     Objects *ob;
{
  Wobject *w = get_wobject(ob);
  if(w && (w->label_loc == 3))
    return(FALSE);
  else
    return(ob && ob->labels);
}

/*********************************************************************/
Chartfile
*get_chart_level(wob,y)
     Wobject *wob;
     register int y;
{
  static Chartfile *cfo = NULL;
  Rect *rec;
  register int n, at, yoffset, height;
  register Chartfile *cf;

  if(wob && (cf = wob->charts)) {
    n=1;
    while ((cf = cf->next)) n++;
    if(n==1) return(wob->charts);
    rec = (Rect*)xv_get(wob->obj->canvas, WIN_RECT);
    if(plotting_labels(wob->obj)) {
      yoffset = (wob->label_loc)? 0 : wob->label_size;
      height =  rec->r_height - wob->label_size;
    } else {
      yoffset = 0;
      height = rec->r_height;
    }
    if(y < yoffset) return(NULL);
    at = (n * (y-yoffset))/height;
    if(at == n) at--;
    cf = wob->charts;
    while(cf && at--) cf = cf->next;
    cfo = cf;
    return(cf);
  }
  return(NULL);
}

/*********************************************************************/
plotting_charts(wob)
     Wobject *wob;
{
  return(wob && wob->charts && wob->charts->first_word);
}

/**********************************************************************/
send_play_seg(ob,ptime,ntime)
     Objects *ob;
     double ptime,ntime;
{
  if(ob) {
    char mes[MES_BUF_SIZE];
    if(ntime > 0.0)
      sprintf(mes,"%s play start %f end %f\n",ob->name,ptime,ntime);
    else
      sprintf(mes,"%s play start %f\n",ob->name, ptime);
    mess_write(mes);
  }
}
     
/**********************************************************************/
double left_bound(word,time)
     Word *word;
     double time;
{
  if(word) {
    Hit *hit = word->hits;
    while(hit) {
      if(hit->start <= time) {
	if(hit->end >= time)
	  return(hit->start);
	else
	  if(!hit->next || (hit->next && (hit->next->start >= time)))
	    return(hit->end);
      }
      hit = hit->next;
    }
   }
  return(-1.0);
}

/**********************************************************************/
double right_bound(word,time)
     Word *word;
     double time;
{
  if(word) {
    Hit *hit = word->hits;
    while(hit) {
      if(hit->start > time)
	return(hit->start);
      else
	if(hit->end > time)
	  return(hit->end);
      hit = hit->next;
    }
   }
  return(-1.0);
}

/**********************************************************************/
Word *get_word_level(cfi,y)
     Chartfile *cfi;
     int y;
{
  Rect *rec;
  register int n, at, me, yspace, ystep,yoffset,height;
  register Chartfile *cf;
  Pixfont* pf;
  Wobject *wob;
  Word *w;

  if(cfi && (wob = cfi->obj) && (cf = wob->charts)) {
    /* Count total charts and find position of cfi. */
    me = n = 1;
    while ((cf = cf->next)) {
      n++;
      if(cf == cfi) me = n;
    }
    if((pf = cfi->font)) {
     yspace = (int)(xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT) * 0.2);
     ystep = xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT) + yspace;
    } else {
      printf("Problems accessing fonts in get_word_level()\n");
      return(NULL);
    }
    rec = (Rect*)xv_get(wob->obj->canvas, WIN_RECT);
    if(plotting_labels(wob->obj)) {
      yoffset = (wob->label_loc)? 0 : wob->label_size;
      height =  rec->r_height - wob->label_size;
    } else {
      yoffset = 0;
      height = rec->r_height;
    }
    at = (y - yoffset - (((me-1)*height)/n))/ystep;
    if(at < 0) return(NULL);
    if((w = cfi->first_word)) {
      do {
	if(w->plotted && (at-- <= 0))
	  return(w);
	w = w->next;
      } while(w != cfi->first_word);
    }
    return(w);
  } else
    printf("Bogus chartfile passed to get_word_level()\n");
  return(NULL);
}

/**********************************************************************/
play_chart_entry(wob,time,y)
     Wobject *wob;
     double time;
     int y;
{
  Chartfile *cf = get_chart_level(wob,y);
  double ptime, ntime;
  if(cf) {
    Word *word = (Word*)get_word_level(cf,y);
    double left, right;
    if(word)  {
      send_play_seg(wob->obj,(ptime = left_bound(word,time)),
		    (ntime = right_bound(word,time)));
      mark_window(wob->obj, ptime, ntime);
    }
  }
    
}

/**********************************************************************/
/* redisplay the chart so that the symbol specified by str in chart cf will
   be at the top of the chart display. */
top_word(str,cf)
     char *str;
     Chartfile *cf;
{
  if(str && *str && cf && cf->first_word) {
    Word *w = cf->first_word;
    do {
      if(!strcmp(str,w->str)) {
	cf->first_word = w;
	plot_chart(cf);
	return(TRUE);
      }
      w = w->next;
    } while(w != cf->first_word);
  }
  return(FALSE);
}

/**********************************************************************/
/* scroll() assumes that plot_charts() has been called at least once
   so that the word-plotted flags will all have been set in the Chartfile
   word list. */
scroll(wob,time,y,up)
     Wobject *wob;
     double time;
     int y, up;
{
  Chartfile *cf = get_chart_level(wob,y);
  if(cf) {
    Word *word = (Word*)get_word_level(cf,y);
    
    if(word) {
      if(debug_level)
	fprintf(stderr,"scroll():Target word is %s\n",word->str);
      if(up) {
	cf->first_word = word;
      } else {			/* Scroll down. */
	Word *w = cf->first_word, *wp = word;

	do {
	  if(wp->plotted) {
	    w = w->prev;
	    while(! w->plotted && (w != cf->first_word)) w = w->prev;
	  }
	  wp = wp->prev;
	} while(wp != cf->first_word);
	cf->first_word = w;
      }
      if(debug_level)
	fprintf(stderr,"scroll():First word is now %s\n",word->str);
      plot_chart(cf);
    }
  }
}

/**********************************************************************/
Wobject *get_wobject(ob)
     Objects *ob;
{
  Wobject *wo = charts;

  while(wo) {
    if(wo->obj == ob) return(wo);
    wo = wo->next;
  }
  return(NULL);
}

/**********************************************************************/
Chartfile *new_chartfile(sig_name, filename)
     char *sig_name, *filename;
{
  if(sig_name && *sig_name && filename && *filename) {
    Chartfile *cf = (Chartfile*)malloc(sizeof(Chartfile));
    char *name;
    extern char fontfile[];
    Pixfont *pf, *open_label_font(), *xv_pf_default();
    
    if(cf && (name = (char*)malloc(strlen(sig_name)+1))) {
      strcpy(name,sig_name);
      cf->sig_name = name;
      cf->chart_name = name = malloc(strlen(filename)+1);
      strcpy(name,filename);
      cf->type = 1;
      cf->eventx = 0;
      cf->eventy = 0;
      cf->color = TEXT_COLOR;
      cf->changed = 0;
      if(*fontfile && (pf = open_label_font(fontfile)))
	cf->font = pf;
      else
	cf->font = xv_pf_default();
      cf->first_word = NULL;
      cf->next = NULL;
      return(cf);
    } else
      printf("Allocation failure in new_chartfile()\n");
    return(NULL);
  }
}

/**********************************************************************/
Chartfile *get_chartfile(ob,chart_file_name)
     Wobject *ob;
     char *chart_file_name;
{
  if(ob && chart_file_name && *chart_file_name) {
    Chartfile *cp = ob->charts;

    while(cp) {
      if(!strcmp(chart_file_name,cp->chart_name))
	return(cp);
      cp = cp->next;
    }
  }
  return(NULL);
}

/**********************************************************************/
Hit *new_hit(start,end,like)
     double start, end, like;
{
  Hit *h;

  if((h = (Hit*)malloc(sizeof(Hit)))) {
    h->start = start;
    h->end = end;
    h->like = like;
    h->next = NULL;
    return(h);
  } else
    printf("Allocation failure in new_hit()\n");
  return(NULL);
}

/**********************************************************************/
Word *new_word(prev,start,end,like,wrd)
     Word *prev;
     double start, end, like;
     char *wrd;
{
  Word *w;

  if((w  = (Word*)malloc(sizeof(Word))) &&
     (w->hits = new_hit(start,end,like))) {
    if(wrd && *wrd) {
      w->str = (char*)malloc(strlen(wrd)+1);
      strcpy(w->str,wrd);
    } else
      w->str = NULL;
    w->hits->next = NULL;
    w->next = NULL;
    w->plotted = FALSE;
    w->prev = prev;
    if(prev)
      prev->next = w;
    w->color = -1;
    return(w);
  } else
    printf("Allocation problems in new_word()\n");
  return(NULL);
}

/**********************************************************************/
/* If w->str is unique, link w into the Chartfile, else link w->hits
into the duplicate Word entry in Chartfile and then free w.   Occurrences
of a word are stored sorted by increasing time. */
append_word(w,cf)
     Word *w;
     Chartfile *cf;
{
  if(w && cf && w->hits) {
    register Word *wp;
    if((wp = cf->first_word)) {
      register Hit *h, *hn = w->hits;
      do {
	if(!strcmp(wp->str,w->str)) {
	  h = wp->hits;
	  if(h) {
	    if(hn->end <= h->start) {
	      hn->next = h;
	      wp->hits = hn;
	    } else
	      while(h) {
		if((! h->next) || (h->next->start >= hn->end)) {
		  hn->next = h->next;
		  h->next = hn;
		  break;
		}
		h = h->next;
	      }
	  } else
	    wp->hits = hn;
	  free(w);
	  return(TRUE);
	}
	wp = wp->next;
      } while(wp != cf->first_word);

      /* This was a unique new word; add to list. */
      cf->first_word->prev->next = w;
      w->prev = cf->first_word->prev;
      cf->first_word->prev = w;
      w->next = cf->first_word;	/* CIRCULAR LIST */
    } else {			/* it's the first entry */
      cf->first_word = w;
      w->next = w;
      w->prev = w;
    }
    return(TRUE);
  } else
    printf("Bogus arguments to append_word()\n");
  return(FALSE);
}

/**********************************************************************/
/* Create a chartfile structure.  If the named file exists, read it in and
   use its contents to fill a structure. */
Chartfile *
read_charts(ob, chart_file_name)
     char *chart_file_name;
     Wobject *ob;
{
  static int type, nfields, color;
  static double freq;
  static char signal[150];
  static char comment[MES_BUF_SIZE], font[NAMELEN];
  static Selector  a2 = {"type", "%d", (char*)&type, NULL},
                   a3 = {"nfields", "%d", (char*)&nfields, &a2},
                   a4 = {"signal", "%s", signal, &a3},
                   a5b = {"color","%d", (char*)&color, &a4},
                   a5 = {"frequency","%lf", (char*)&freq, &a5b},
                   a6 = {"font", "%s", font, &a5},
                   a7 = {"comment", "#str", comment, &a6};
  FILE *fopen(), *fp;
  Chartfile *cf;
  Pixfont *pf, *xv_pf_open();

  if(!ob || !ob->obj) return(NULL);
  type = 1;
  nfields = 1;
  freq = 8000.0;
  color = -1;
  *signal = 0;
  *comment = 0;
  *font = 0;

  if((cf = (Chartfile*)get_chartfile(ob,chart_file_name))) {
    printf("Chart file %s exists(%s %s); request to recreate ignored.\n",
	   chart_file_name,ob->obj->name,cf->chart_name);
    return(NULL);
  }
  if((cf = new_chartfile(ob->obj->name, chart_file_name))) {
    if((fp = fopen(cf->chart_name,"r"))) {
      char wrd[200];
      int id, nread;
      double start, end, like;
      Word *w;
      char in[MES_BUF_SIZE];

      while(fgets(in,MES_BUF_SIZE,fp)) {	/* read chart file header */
	if(*in == '#') break;
	get_args(in,&a7);
      }
      if(color >= 0)
	cf->color = color;
      cf->type = type;
      cf->freq = freq;
      if(*font) {
	if(!(pf = xv_pf_open(font)))
	  printf("Can't open fontfile %s; using default\n",font);
	else
	  cf->font = pf;
      }
      if(*comment) {
	cf->comment = malloc(strlen(comment)+1);
	strcpy(cf->comment, comment);
      }
      else cf->comment = NULL;
      /* A typical line in the chart might look like this:
	 11 1.3 1.7 .2 foobar
	 */
      while(fgets(in,MES_BUF_SIZE,fp)) {	/* read all words */
	if((nread =sscanf(in,"%d%lf%lf%lf%s",
			  &id, &start, &end, &like, wrd)) == 5) {
	  if(cf->type == 2) {	/* convert samples to time */
	    start /= cf->freq;
	    end /= cf->freq;
	  }
	  if(! (w = new_word(NULL,start,end,like,wrd))) {
	    printf("Can't create a new word; chart MUST EXIT!\n");
	    fclose(fp);
	    return(NULL);
	  }
	  append_word(w,cf);
	} else {
	  if(nread > 0)
	    fprintf(stderr,"Bad line encountered in chart file %s: %s",
		    chart_file_name, in);
	}
      }
      cf->changed = 0;
      fclose(fp);
    } else {
      printf("Can't open chart file %s for reading.\n",cf->chart_name);
      return(NULL);
    }
    if((!cf->first_word)) {
      free_chartfile(cf);
      printf("Empty chartfile (%s)\n",chart_file_name);
      return(NULL);
    }
    cf->next = ob->charts;	/* link in the new chartfile */
    ob->charts = cf;
    cf->obj = ob;
    return(cf);
  } else
    printf("Can't create new_chartfile(%s,%s)\n",
	   ob->obj->name,chart_file_name);
  return(NULL);
}


/*********************************************************************/
plot_chart(cf)
     Chartfile *cf;
{
  Pixwin *pw;
  Pixfont *pf;
  Rect *rec;
  Objects *ob;
  Word *word;
  Hit *hit;
  Wobject *wob;
  Chartfile *cf2;
  double end;
  int x, y, n, top, bot, height, xl, yl, label_size, yoffset;
  int ystep, ymin, ymax, yspace, width, xc, xw, xh;

  if(!((wob = cf->obj) && (ob = wob->obj) && (cf2 = wob->charts))) {
    printf("Bad Chartfile passed to plot_chart()\n");
    return(FALSE);
  }
  if(ob->labels) {		/* plotting labels too? */
    label_size = wob->label_size;
    yoffset = (wob->label_loc)? 0 : label_size;
  } else
    label_size = yoffset = 0;
  
  /* Find total number of charts to plot. */
  n = 1;
  x = 1;
  while((cf2 = cf2->next)) {
    n++;
    if(cf2 == cf) x = n;
  }  
  pw = canvas_pixwin(ob->canvas);
  rec = (Rect*)xv_get(ob->canvas, WIN_RECT);
  height = (rec->r_height - label_size)/n; /* height allocated to each chart */
  top = (x-1)*height + yoffset;
  bot = top + height;
  end = ob->start + ((ob->sec_cm * rec->r_width) /PIX_PER_CM);

  pw_rop(pw,0,top,rec->r_width,height, PIX_SRC|PIX_COLOR(0),NULL,0,0);

  pw_vector(pw, 0, top, rec->r_width, top, PIX_SRC, cf->color);
  
  if((word = cf->first_word)) {
    if((pf = cf->font)) {
     yspace = (int)(xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT) * 0.2);
     ystep = ymin = xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT) + yspace;
      ymax = bot - ystep;
    }
    else {
      fprintf(stderr,"plot_chart: cannot get font\n");
      return(FALSE);
    }
   width = xv_get(pf,FONT_DEFAULT_CHAR_WIDTH);
   y = top + yspace*0.5 + xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT);
    do {
      hit = word->hits;
      word->plotted = 0;
      while(hit) {
	if((hit->end >= ob->start) && (hit->start <= end)) {
	  if(y < bot) {
	    xl = time_to_x(ob,hit->start); /* left marker */
	    xh = time_to_x(ob,hit->end); /* right marker */
	    xc = (xh+xl)/2;	/* word center */
	    x = xc - ((xw = strlen(word->str) * width)/2); /* text left end */
	    yl = y-ystep;
	    pw_vector(pw, xl, yl, xl, y, PIX_SRC, word->color);
	    pw_vector(pw, xh, yl, xh, y, PIX_SRC, word->color);
	    yl = y - (ystep/2);
	    pw_vector(pw, xl, yl, x, yl, PIX_SRC, word->color);
	    pw_vector(pw, x+xw, yl, xh, yl, PIX_SRC, word->color);
	    pw_text(pw,x,y-yspace,PIX_COLOR(cf->color)|PIX_SRC,pf,word->str);
	    word->plotted = 2;
	  } else
	    word->plotted = 1;	/* plottable, but no room */
	}
	hit = hit->next;
      }
      if(word->plotted)	/* Only inc. y pos. if something was plotted. */
	y += ystep;
      word = word->next;
    } while(word != cf->first_word);
    return(TRUE);
  }
}

/*********************************************************************/
do_chart_menu(cf, event, arg)
     Chartfile *cf;
     Event *event;
     caddr_t arg;
{
  Objects *ob;
  char *str, *c;
  Menu men;
  Wobject *w;

  if(cf && (w = cf->obj) && (ob = w->obj) && (men = w->menu)) {
    cf->eventx = ob->oldcursor_x;
    cf->eventy = ob->oldcursor_y;
    remove_cursor(ob->canvas,ob);
    xv_set(men, MENU_CLIENT_DATA, cf, 0);
    menu_show(men,ob->canvas,event,NULL);
    return(TRUE);
  } else
    printf("Bad Chartfile passed to do_chart_menu()\n");
  return(FALSE);
}

/*********************************************************************/
void
chart_menu_proc(men, item)
     Menu men;
     Menu_item item;
{
  Chartfile *cf;
  Wobject *w;
  Objects *ob;

  if(men && (cf = (Chartfile*)xv_get(men, MENU_CLIENT_DATA)) &&
     (w = cf->obj) && (ob = w->obj)) {
    char *str, *c;
    
    if((str = (char*)xv_get(item,MENU_VALUE))) {
      if(!strcmp(str,"*SHELL*")) { /* Fork a process? */
	Word *wrd;
	char *cp;
	Menu_item item;
	char command[MES_BUF_SIZE], nam[50];
	FILE *ft, *fopen();
	
	/* Whatever process gets forked is called with 4 args:
	   <chartname> <word> <time> <output>,
	   where <output> may optionally be used to store results
	   that may then be read as another chartfile.  If <output>
	   actually gets created, it is read and displayed. */
	if((wrd = get_word_level(cf,cf->eventy)))
	  cp = wrd->str;
	else
	  cp = "XYZZY";
	item = menu_find(men, MENU_VALUE, str, 0);
	c = (char*)xv_get(item, MENU_STRING);
	sprintf(nam,"/tmp/chartXXXXXX");
#if !defined(LINUX)
	mktemp(nam);
#else
	mkstemp(nam);
#endif
	sprintf(command,"%s %s %s %f %s \n",c,cf->chart_name,cp,
		x_to_time(ob,cf->eventx),nam);
	system(command);
	if((ft = fopen(nam,"r"))) { /* Was output created? */
	  extern Panel_item chart_item;
	  extern char chartname[];

	  fclose(ft);
	  strcpy(chartname,nam);
	  
	  xv_set(chart_item, PANEL_VALUE, chartname,0);
	  newFile(chart_item,NULL);

	}
	return;
      }

      if(!strcmp(str,"*UP*"))	/* Scroll up. */
	scroll(w,x_to_time(ob,cf->eventx),cf->eventy,1);

      if(!strcmp(str,"*DOWN*"))	/* Scroll down. */	
	scroll(w,x_to_time(ob,cf->eventx),cf->eventy,0);

      if(!strcmp(str,"*UNLOAD*")) { /* remove the chart file? */
	if(kill_chartfile(cf->chart_name,ob)) {
	  set_display_size(ob);
	  redo_display(ob);
	}
	return;
      }
    }
  } else
    printf("Bad arguments to chart_menu_proc()\n");
  return;
}
