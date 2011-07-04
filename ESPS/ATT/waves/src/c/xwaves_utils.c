/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  Rod Johnson, Alan Parker, David Talkin, ERL
 *
 *  xwaves_utils.c
 *  a collection of routines of general use in "xwaves"
 * 
 */

static char    *sccs_id = "@(#)xwaves_utils.c	1.27	28 Oct 1999	ATT/ESI/ERL";

#include <Objects.h>
#include <spectrogram.h>
#include <file_ext.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <sys/param.h>
#include <xview/notice.h>


extern Object   program;
extern int      debug_level;
extern int      do_color;
extern int      h_spect_rescale;

static char     env_str[MAXPATHLEN + 20];
char           *get_esps_base(), *get_output_file_names();

/*************************************************************************/
void 
set_default_header()
{
   extern char     default_header[];

   if (*default_header) {
      static char     tch[NAMELEN];
#if defined(SONY_RISC) || defined(CONVEX)
      setenv("DEF_HEADER", default_header, 1);
#else
      sprintf(tch, "DEF_HEADER=%s", default_header);
      putenv(tch);
#endif
   }
}

void
set_old_sphere_format()
{
  extern int old_sphere_format;
  extern void set_old_sphere_flag();	/* in esps headers.c */

  set_old_sphere_flag(old_sphere_format);
}

/*************************************************************************/
clone_methods(so, si)
   Signal         *so, *si;
{
   if (si && so) {
      /* Clone (possibly specialized) utility methods. */
      so->utils->read_data = si->utils->read_data;
      so->utils->write_data = si->utils->write_data;
      so->utils->free_data = si->utils->free_data;
      so->utils->buf_start = si->utils->buf_start;
      so->utils->buf_end = si->utils->buf_end;
      so->utils->sig_dur = si->utils->sig_dur;
      so->utils->index_to_time = si->utils->index_to_time;
      so->utils->time_to_index = si->utils->time_to_index;
      return (TRUE);
   }
   return (FALSE);
}

/*************************************************************************/
window_wash(handle)
   caddr_t         handle;
{
   if (handle)
      return;
   printf("Xview failed to return a window; sorry...\n");
   quit_proc();
}

static char     cur_oname[NAMELEN] = "";

/*************************************************************************/
char           *
current_object_name()
{
   static char     rv[NAMELEN];
   extern Panel_item newObj_item;

   if (*cur_oname) {
      strcpy(rv, cur_oname);
      *cur_oname = 0;
   } else {
      *rv = 0;
      sscanf((char*)panel_get_value(newObj_item), "%s", rv);
   }
   return (rv);
}

/*************************************************************************/
set_current_obj_name(name)
   char           *name;
{
   if (name && *name)
      strcpy(cur_oname, name);
   else
      *cur_oname = 0;
}

/*************************************************************************/
window_check_return(handle)
   caddr_t         handle;
{
   if (handle)
      return (TRUE);
   printf("Xview can't create a window; sorry...\n");
   return (FALSE);
}

/*************************************************************************/
char           *
receiver_prefixed(str)
   char           *str;
{
   static char     tmp[MES_BUF_SIZE];
   char            junk[MES_BUF_SIZE], cd, *cdp;
   Object         *ob, *find_object();
   extern char     objectname[];

   if (debug_level > 1)
      (void) fprintf(stderr, "receiver_prefixed: function entered\n");

   if (str) {
      while ((*str == ' ') || (*str == '\t'))
	 str++;

      if ((((cd = *str) == '"') || (cd == '\''))) {
	 cdp = ++str;
	 while (*cdp && (*cdp != cd))
	    cdp++;
	 if (*cdp)
	    *cdp = ' ';
      }
      if (*str && sscanf(str, "%s", junk) == 1) {
	 if ((ob = find_object(junk))) {
	    if (debug_level > 1)
	       (void) fprintf(stderr, "receiver_prefixed(1): returning %s\n", str);
	    return (str);
	 }
	 if (*str == '.')	/* means current object */
	    sprintf(tmp, "%s %s", objectname, str + 1);
	 else			/* assume the program is the receiver  */
	    sprintf(tmp, "%s %s", program.name, str);
	 if (debug_level > 1)
	    (void) fprintf(stderr, "receiver_prefixed(2): returning %s\n", tmp);
	 return (tmp);
      }
   }
   if (debug_level > 1)
      (void) fprintf(stderr, "receiver_prefixed(3): returning NULL\n");
   return (NULL);
}

/*************************************************************************/
esps_initialize()
{
}

Notify_value
sigint_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   if (debug_level > 1)
      fprintf(stderr, "Killed due to signal(%d); cleaning up...\n", sig);
   quit_proc();
}

emergency_quit()
{
   stop_da(NULL);
#ifndef NO_LIC
   lm_quit();
#endif
   exit(-1);
}

Notify_value
sigtrm_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   fprintf(stderr, "Killed -- exiting.\n");
   emergency_quit();
}

Notify_value
sigfpe_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   fprintf(stderr, "Floating point exception.  Exiting.\n");
   quit_proc();
}

Notify_value
sigbus_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   fprintf(stderr, "Bus error exception caught.  Exiting.\n");
   emergency_quit();
}

Notify_value
sigill_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   fprintf(stderr, "Illegal instruction exception caught.  Exiting.\n");
   emergency_quit();
}

Notify_value
sigseg_func(client, sig, when)
   Notify_client   client;
   int             sig, when;
{
   fprintf(stderr, "Seg violation exception caught.  Xwaves exiting.\n");
   emergency_quit();
}

/*************************************************************************/
install_signal_handlers(frame)
   Frame           frame;
{

   notify_set_signal_func(frame, sigint_func, SIGINT, NOTIFY_SYNC);
   notify_set_signal_func(frame, sigint_func, SIGQUIT, NOTIFY_SYNC);
   notify_set_signal_func(frame, sigint_func, SIGHUP, NOTIFY_SYNC);
   if (0 && !debug_level) {
      notify_set_signal_func(frame, sigtrm_func, SIGTERM, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigfpe_func, SIGFPE, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigbus_func, SIGBUS, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigseg_func, SIGSEGV, NOTIFY_ASYNC);
      notify_set_signal_func(frame, sigill_func, SIGILL, NOTIFY_ASYNC);
   }

   /* if(!debug_level) {
    *   signal(SIGTERM,sigtrm_func);
    *   signal(SIGFPE,sigfpe_func);
    *   signal(SIGBUS,sigbus_func);
    *   signal(SIGSEGV,sigseg_func);
    *   signal(SIGILL,sigill_func);
    * }
    */
}

/*************************************************************************/
void 
distribute_options_settings()
{
   extern int      options;
   extern int      dont_save_sgrams, overlay_as_number, redraw_on_release, rewrite_after_edit;

   dont_save_sgrams = overlay_as_number = redraw_on_release =
      rewrite_after_edit = 0;

   if (options & REPAINT_ON_RELEASE)
      redraw_on_release = 1;
   if (options & DONT_SAVE_SPGM)
      dont_save_sgrams = 1;
   if (options & SAVE_AFTER_EDIT)
      rewrite_after_edit = 1;
   if (options & BW_PLOTS)
      overlay_as_number = 1;
}

/*************************************************************************/
waves_initialize()
{
   char           *kluge, *getenv();
   extern int      use_dsp32, dsp_type, w_verbose;
   extern int      fea_sd_special;	/* in header.c */
   extern char     default_header[];
   int             coulduse = 0;
   char	          *ptr;

   setup_environ();

   get_globals();		/* read the configuration file */

   if ((kluge = getenv("DEF_HEADER")))
      strcpy(default_header, kluge);
   else
      set_default_header();

   if (was_changed("options"))
      distribute_options_settings();

#if defined(SUN4) || defined(SG) || defined(HP700) || defined(LINUX_OR_MAC)
   if (sg_audio_is_available()) {
      if (!was_changed("use_dsp32") && !was_changed("use_internal_audio"))	/* not forced in profile */
	 use_dsp32 = 1;
      dsp_type = DSP_SGI;
      if (use_dsp32 && w_verbose)
	 printf("Will use built-in D/A for play commands.\n");
   }
#else
   if ((coulduse |= dsp32c_is_available())) {
      if (!was_changed("use_dsp32"))	/* not forced in profile */
	 use_dsp32 = 1;
      dsp_type = DSP32C_VME;
      if (use_dsp32 && w_verbose)
	 printf(
		"DSP32C board present; will use it for spectrogram and play commands.\n");
   } else if ((coulduse |= dsp32_is_available())) {
      if (!was_changed("use_dsp32"))	/* not forced in profile */
	 use_dsp32 = 1;
      dsp_type = DSP32_FAB2;
      if (use_dsp32 && w_verbose)
	 printf(
		"DSP32 board present; will use it for spectrogram and play commands.\n");
   }
   if (!was_changed("use_dsp32"))	/* not forced in profile */
      use_dsp32 = 0;		/* (Board is not available) */

   if (use_dsp32) {
      if (!was_changed("fea_sd_special"))	/* not forced in profile */
	 fea_sd_special = 1;
      if (w_verbose)
	 printf(
		"(FEA_SD files will be read in as shorts; toggle with fea_sd_special)\n");
   } else {
      if (!was_changed("fea_sd_special"))	/* not forced in profile */
	 fea_sd_special = 0;
      if (coulduse && w_verbose)
	 printf("Built-in DSP will not be used.\n");
      if (w_verbose)
	 printf("Will use external ESPS calls for spectrograms and play.\n");
   }
#endif

   get_color_depth();

   if (!was_changed("do_color")) {	/* color not specified in profile */
      extern int      cmap_depth;

      /* check whether color */
      if (cmap_depth < 2)	/* this COULD be cleverer */
	 do_color = 0;		/* MONO hardware */
      else
	 do_color = 1;
   }
   sprintf(env_str, "XPPATH=%s/lib/Xp", (ptr = get_esps_base(NULL)));
   free(ptr);
   putenv(env_str);
   setup_colormap();
   install_dispatch_hook();
}

/*************************************************************************/
install_colormap(obj)
   register Object *obj;
{
   while (obj) {
      Signal         *s = obj->signals;
      while (s) {
	 View           *v = s->views;
	 while (v) {
	    if (v->canvas) {
	       cmap(v->canvas);
	       return;
	    }
	    v = v->next;
	 }
	 s = s->others;
      }
      obj = obj->next;
   }
}

/*************************************************************************/
iabs(i)
   register int    i;
{
   return ((i >= 0) ? i : -i);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Return the element number whose identification string matches str.  If a
 * channel label can not be found to match str, -1 is returned.
 */
get_labeled_chan(s, str)
   Signal         *s;
   char           *str;
{
   register List  *l, *l2;

   if (s && (l = s->idents)) {
      register int    i, dim;

      for (dim = 0; dim < s->dim; dim++) {	/* for each active
						 * dimension... */
	 if (!strcmp(l->str, str))
	    return (dim);
	 if (l->next)
	    l = l->next;
	 else
	    return (-1);	/* no hope of finding a match */
      }
   }
   return (-1);
}

/*********************************************************************/
Object         *
new_object(name, sig)
   Signal         *sig;
   char           *name;
{
   Object         *ob;
   char           *c;
   extern Methods  meth1;
   extern int      zoom_ganged, edit_ganged, scroll_ganged, dont_save_sgrams;

   if ((ob = (Object *) malloc(sizeof(Object))) &&
       (c = malloc(strlen(name) + 1))) {
      strcpy(c, name);
      ob->name = c;
      ob->methods = &meth1;
      ob->signals = sig;
      ob->marks = NULL;
      ob->zoom_ganged = zoom_ganged;
      ob->edit_ganged = edit_ganged;
      ob->scroll_ganged = scroll_ganged;
      ob->dont_save_sgrams = dont_save_sgrams;
      if (sig)
	 sig->obj = (caddr_t) ob;
      ob->next = NULL;
      return (ob);
   }
   printf("Can't allocate space for another object\n");
   return (NULL);
}

/************************************************************************/
/*
 * This assumes that the object's displays contain links to all signals in
 * the object.  The signals, views, etc. actually get freed up in
 * free_canvas_views() as part of the display cleanup initiated by
 * kill_signal_view().
 */
kill_object(o)
   Object         *o;
{
   Signal         *s, *s2;
   Object         *o2;
   View           *v;
   typedef struct frli {
      Frame           f;
      struct frli    *next;
   }               Flist;
   Flist          *fl = NULL, *fl2, *fl3;
   Frame           frame;
   int             wasthere;
   extern Panel_item newObj_item;
   extern char     objectname[];

   if (o) {
      s = o->signals;
      while (s) {
	 if ((s->file == SIG_NEW) &&
	     ((s->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM))
	    put_waves_signal(s);/* save newly created signals (except
				 * spectrograms) */
	 v = s->views;
	 while (v) {
	    if (v->canvas) {
	       frame = (Frame) xv_get(v->canvas, XV_OWNER);
	       wasthere = 0;
	       if (!(fl2 = fl)) {
		  fl = (Flist *) malloc(sizeof(Flist));
		  fl->f = frame;
		  fl->next = NULL;
	       } else {
		  while (fl2) {
		     if (fl2->f == frame) {
			wasthere = 1;
			break;
		     }
		     fl2 = fl2->next;
		  }
		  if (!wasthere) {
		     fl3 = (Flist *) malloc(sizeof(Flist));
		     fl3->f = frame;
		     fl3->next = fl;
		     fl = fl3;
		  }
	       }
	    }
	    v = v->next;
	 }
	 s = s->others;
      }
      if ((fl2 = fl)) {
	 while (fl2) {
	    xv_set(fl2->f, FRAME_NO_CONFIRM, TRUE, 0);
	    dt_xv_destroy_safe(17, fl2->f);	/* THIS OCCURS
						 * ASYNCHRONOUSLY!! */
	    fl3 = fl2->next;
	    free(fl2);
	    fl2 = fl3;
	 }
      }
      /*
       * The object will actually be freed in free_canvas_views() if it was
       * displayed in a canvas view.
       */
/*      if (!strcmp(objectname, o->name)) { disabled for eddy 3/5/96
	 panel_set_value(newObj_item, "");
      }
*/
      o2 = &program;
      while (o2->next) {
	 if (o2->next == o) {
	    o2->next = o->next;
	    break;
	 }
	 o2 = o2->next;
      }
   }
}

/************************************************************************/
dt_xv_destroy(where, o)
   int             where;
   void           *o;
{
   if (debug_level > 1)
      fprintf(stderr, "Destroying object(%d) %d\n", where, o);
   xv_destroy((Xv_opaque)o);
}

/************************************************************************/
dt_xv_destroy_safe(where, o)
   int             where;
   void           *o;
{
   if (debug_level > 1)
      fprintf(stderr, "Safe destroying object(%d) %d\n", where, o);
   xv_destroy_safe((Xv_opaque)o);
}

/************************************************************************/
put_waves_signal(s)
   Signal         *s;
{
   extern Pending_input new_files;
   extern int      dont_save_sgrams;

   if (s) {
      /* Usually it is not desirable to save the spectrogram signal... */
      if (dont_save_sgrams &&
	  ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM))
	 return (TRUE);

      /* Keep a cumulative list of all new files created. */
      if (put_signal(s)) {
	 if (new_files.list)
	    ((Menu_list *) new_files.list)->active = FALSE;
	 if (add_to_menu_list(&new_files.list, s->name))
	    ((Menu_list *) new_files.list)->active = TRUE;
	 if (new_files.canvas)
	    menu_redoit(new_files.canvas, NULL, NULL);
	 return (TRUE);
      }
   }
   return (FALSE);
}

/*******************************************************************/
char           *
get_extension(stype)
   register int    stype;
{
   Tlist          *tl = extensions;
   int             type;

   if (!(type = (stype & SPECIAL_SIGNALS)))
      type = VECTOR_SIGNALS;
   while (tl) {
      if (tl->type == type)
	 return (tl->ext);
      tl = tl->next;
   }
   return (NULL);
}

/*********************************************************************/
make_edited_filename(s, realname)
   Signal         *s;
   char           *realname;
{
   char           *ext, *name, next[NAMELEN], *cp;
   extern char     outputname[];
   extern int      append_extensions;

   if ((name = get_output_file_names(outputname, next))) {
      if (append_extensions) {
	 /*
	  * ``outputname'' is given, so use it with appropriate extension
	  */
	 if (!(ext = (char *) get_extension(s->type))) {
	    if (debug_level)
	       fprintf(stderr, "Unknown file type (%x) in make_edited_filename()\n", s->type);
	    return (FALSE);
	 }
	 cp = name + strlen(name) - strlen(ext);
	 if (strcmp(cp, ext)) {	/* does it already have the correct
				 * extension? */
	    cp = name + strlen(name) - 2;	/* nope */
	    if (!strcmp(cp, ".."))
	       *cp = 0;
	    sprintf(realname, "%s%s", name, ext);
	 } else
	    strcpy(realname, name);
      } else
	 strcpy(realname, name);
   } else {
      insert_numeric_ext(s->name, ++(s->version), realname);
      setup_output_dir(realname);
   }
   return (TRUE);
}

/*********************************************************************/
char          **
get_new_files_list()
{
   Menu_list      *l;
   Pending_input  *get_new_files(), *p2, *p = get_new_files();
   int             n = 0;
   char          **rv;

   if (p && (l = (Menu_list *) p->list)) {

      while (l) {
	 n++;
	 l = l->next;
      }
      if ((rv = (char **) malloc(sizeof(char *) * (n + 1)))) {
	 char          **sort_a_list();

	 l = (Menu_list *) p->list;
	 n = 0;
	 while (l) {
	    rv[n++] = l->str;
	    l = l->next;
	 }
	 rv[n] = NULL;
	 return (sort_a_list(rv));
      } else
	 fprintf(stderr, "Allocation problems in get_new_files_list()\n");
   }
   return (NULL);
}

/*********************************************************************/
char          **
get_objects_list()
{
   int             n = 0;
   char          **rv;
   extern Object   program;
   Object         *o = program.next;

   if (o) {
      while (o) {
	 n++;
	 o = o->next;
      }
      if ((rv = (char **) malloc(sizeof(char *) * (n + 1)))) {
	 char          **sort_a_list();

	 o = program.next;
	 n = 0;
	 while (o) {
	    rv[n++] = o->name;
	    o = o->next;
	 }
	 rv[n] = NULL;
	 return (sort_a_list(rv));
      } else
	 fprintf(stderr, "Allocation problems in get_objects_list()\n");
   }
   return (NULL);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
time_to_index(s, time)
   register Signal *s;
   register double time;
{
   if (s->utils->time_to_index)
      return (s->utils->time_to_index(s, time));
   else
      return (vec_time_to_index(s, time));
}

/*********************************************************************/
check_file_name(s, cs)
   register Signal *s;
   char           *cs;
{
   char           *cp, *ct;

   if (!is_a_x_file(s->name, cs)) {
      if (!rename_signal(s, make_x_name(s->name, cs))) {
	 printf("Can't create new file name for edited parmeters\n");
	 return;
      }
      update_window_titles(s);
      return (FALSE);
   }
   return (TRUE);
}

/************************************************************************/
/*
 * This assumes that a canvas will only be used by one Object (and its
 * Signals).  All signals which refer solely to this view are deleted.
 */
free_canvas_views(canvas)
   Canvas          canvas;
{
   Object         *o;
   Signal         *s, *s2;
   View           *v, *v2;
   extern Panel_item newObj_item, newFile_item;
   extern char     objectname[];

   if ((v = (View *) xv_get(canvas, WIN_CLIENT_DATA)) &&
       (o = (Object *) v->sig->obj)) {	/* search object for ref. to canvas */
      if ((s = o->signals)) {
	 while (s) {
	    if (debug_level)
	       fprintf(stderr, "Does %s refer to canvas %x?\n", s->name, canvas);
	    v = s->views;
	    while (v) {
	       if (v->canvas == canvas) {
		  if (debug_level)
		     fprintf(stderr, "Yes, found signal %s displayed (its views:%x)\n",
			     v->sig->name, v->sig->views);
		  stop_da_view(v);
		  if (unlink_view(v, &(s->views))) {	/* kill views that use
							 * canvas */
		     v2 = v->next;
		     v->next = NULL;
		     free_view(v);
		  } else {
		     printf("Problems with unlink_view() in free_canvas_views()\n");
		     return (FALSE);
		  }
		  v = v2;
	       } else
		  v = v->next;
	    }
	    s = s->others;
	 }
      }
      /* rescan signal list looking for unreferenced signals */
      s = o->signals;
      while (s) {
	 if (debug_level)
	    fprintf(stderr, "Looking for ref to %s during rescan\n", s->name);
	 if (!s->views) {	/* check for references in other signal view
				 * lists */
	    int             was_referenced = FALSE;
	    if (debug_level)
	       fprintf(stderr, "%s has no directly-referenced views\n", s->name);
	    s2 = o->signals;
	    while (s2) {
	       v = s2->views;
	       while (v) {
		  if (v->sig == s) {
		     was_referenced = TRUE;
		     if (debug_level)
			fprintf(stderr, "Found reference to it from view %d\n", v);
		     break;
		  }
		  v = v->next;
	       }
	       if (was_referenced)
		  break;
	       s2 = s2->others;
	    }
	    if (!was_referenced) {	/* kill signals that have no views */
	       s2 = s->others;
	       
	       if (unlink_signal(s)) {
		  if ((s->file == SIG_NEW) &&	/* save any new or modified
						 * signals */
		      ((s->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM))	/* except spgm. */
		     put_waves_signal(s);
		  send_kill_file(o, s);
		  if (debug_level)
		     fprintf(stderr, "Sent kill file (%s %s)\n", o->name, s->name);
		  /*		  if (!strcmp((char*)panel_get_value(newFile_item), s->name))
				  panel_set_value(newFile_item, ""); */
		  free_signal(s);
	       } else {
		  printf("Problems with unlink_signal() in free_canvas_views()\n");
		  return (FALSE);
	       }
	       s = s2;
	    } else		/* it was referenced in another view */
	       s = s->others;
	 } else {		/* it has views */
	    if (debug_level)
	       fprintf(stderr, "It has a view list\n");
	    s = s->others;
	 }
      }

      if (!o->signals) {	/* if all signals were destroyed... */
	 Object         *o2 = &program;

	 if (debug_level)
	    fprintf(stderr, "Found that all signals in %s were destroyed\n", o->name);
	 while (o2->next) {	/* unlink from object list, if necessary */
	    if (o2->next == o) {
	       o2->next = o->next;
	       break;
	    }
	    o2 = o2->next;
	 }
/*	 if (!strcmp(objectname, o->name)) deleted 3/5/96 for eddy
	    panel_set_value(newObj_item, "");
*/
	 send_kill_object(o);
	 kill_all_marks(o);
	 if (o->name)
	    free(o->name);
	 free(o);
      }
      return (TRUE);
   }
   return (FALSE);
}

/*************************************************************************/
canvas_still_lives(canvas)
   Canvas          canvas;
{
   extern Object   program;
   Object         *o = &program;
   Signal         *s;
   View           *v;

   while (o) {
      s = o->signals;
      while (s) {
	 v = s->views;
	 while (v) {
	    if (v->canvas == canvas)
	       return (TRUE);
	    v = v->next;
	 }
	 s = s->others;
      }
      o = o->next;
   }
   return (FALSE);
}

/*********************************************************************/
static 
really_plot_it_now(canvas)
   Canvas          canvas;
{
   View           *v;
   Signal         *s;

   v = (View *) xv_get(canvas, WIN_CLIENT_DATA);
   s = v->sig;

   for (v = s->views; v; v = v->next)
      plot_view(v);

   if (s && (v = s->views) && v->canvas)
      send_redisplay(s);
}

static List    *dq = NULL;

#define ITIMER_NULL ((struct itimerval*)0)
/*************************************************************************/
static Notify_value 
process_repaint_q()
{
   List           *dq2;
   extern Frame    daddy;

   notify_set_itimer_func(12345, process_repaint_q, ITIMER_REAL, ITIMER_NULL,
			  ITIMER_NULL);
   while (dq) {
      if (dq->str && canvas_still_lives(dq->str))
	 really_plot_it_now(dq->str);
      dq2 = dq->next;
      free(dq);
      dq = dq2;
   }
   return (NOTIFY_DONE);
}

/*************************************************************************/
clobber_repaint_entry(canvas)
   char           *canvas;
{
   List           *dqp = dq;

   while (dqp) {
      if (dqp->str == canvas)
	 dqp->str = NULL;
      dqp = dqp->next;
   }
}

/*************************************************************************/
static List    *
new_repaint_ent(str)
   char           *str;
{
   List           *q;
   if (str && (q = (List *) malloc(sizeof(List)))) {
      q->str = str;
      q->next = NULL;
      return (q);
   }
   return (NULL);
}

/*************************************************************************/
static 
delay_repaint(str)
   char           *str;
{
   if (str) {
      List           *dqp = dq, *qp2;

      restart_d_clock(100000, 12345, process_repaint_q);

      while (dqp) {		/* repaint already requested? */
	 if (dqp->str == str)
	    return (TRUE);
	 dqp = dqp->next;
      }

      if ((dqp = new_repaint_ent(str))) {
	 if (!dq)
	    dq = dqp;
	 else {
	    qp2 = dq;
	    while (qp2->next)
	       qp2 = qp2->next;
	    qp2->next = dqp;
	 }
	 return (TRUE);
      } else
	 fprintf(stderr, "Allocation failure in delay_repaint()\n");
   }
   return (FALSE);
}

/***********************************************************************/
void 
repaint(canvas, pw, ra)
   Canvas          canvas;
   Pixwin         *pw;
   Rectlist       *ra;
{
   delay_repaint(canvas);
}

/*********************************************************************/
void 
redoit(canvas)
   Canvas          canvas;
{
   delay_repaint(canvas);
}

/*********************************************************************/
Object         *
find_object(name)
   char           *name;
{
   Object         *get_receiver();

   return (get_receiver(name));
}

/*********************************************************************/
link_new_object(ob)
   Object         *ob;
{
   if (ob) {
      ob->next = program.next;
      program.next = ob;
   }
}

/*********************************************************************/
link_new_signal(ob, sig)
   Object         *ob;
   Signal         *sig;
{
   if (ob && sig) {
      sig->obj = (caddr_t) ob;
      sig->others = ob->signals;
      ob->signals = sig;
   }
}

/*********************************************************************/
Signal         *
find_signal(ob, name)
   Object         *ob;
   char           *name;
{
   Signal         *s;
   char           *expand_name();

   if (ob && name && (s = ob->signals)) {
      while (s) {
	 if (!strcmp(expand_name(NULL, name), s->name))
	    return (s);
	 s = s->others;
      }
   }
   return (NULL);
}

/*********************************************************************/
Signal         *
get_playable_signal(v)
   View           *v;
{
   Spectrogram    *sp;
   Object         *o;
   Signal         *s0, *s, *get_any_signal();
   struct header  *ehd;

   if (v && (s0 = v->sig)) {
      if ((((s0->type & (VECTOR_SIGNALS | APERIODIC_SIGNALS | VAR_REC_SIGNALS))
	    == P_SHORTS) && playable_dimension(s0->dim)) ||
	  (s0->header
	   && s0->header->magic == ESPS_MAGIC
	   && (ehd = s0->header->esps_hdr)
	   && ehd->common.type == FT_FEA
	   && ehd->hd.fea->fea_type == FEA_SD)
	 )
	 return s0;
      if ((o = (Object *) s0->obj)) {
	 if (((s0->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM)
	     && (sp = (Spectrogram *) (s0->params))
	     && (s = (Signal *) find_signal(o, sp->signame))
	    )
	    return s;
	 if (s0->header
	     && s0->header->magic == ESPS_MAGIC
	     && (ehd = s0->header->esps_hdr)
	     && ehd->common.type == FT_FEA) {
	    if (ehd->variable.refer
		&& (s = find_signal(o, ehd->variable.refer)))
	       return s;
	    else if (ehd->variable.source[0]
                     && strcmp(ehd->variable.source[0],"<stdin>")
		     && (s = find_signal(o, ehd->variable.source[0])))
	       return s;
	    else if (ehd->variable.srchead[0] 
                     && ehd->variable.srchead[0]->variable.source[0]
                     && strcmp(ehd->variable.srchead[0]->variable.source[0],
			       "<stdin>")
		     && (s=find_signal(o, 
                         ehd->variable.srchead[0]->variable.source[0])))
	       return s;
	 }
	 if (s = find_signal(o, new_ext(s0->name, "sd")))
	    return s;
	 if (s = find_signal(o, new_ext(s0->name, "d")))
	    return s;
	 /* search through object for anything playable */
	 for (s = o->signals; s; s = s->others) {
	    if ((((s->type
		   & (VECTOR_SIGNALS | APERIODIC_SIGNALS | VAR_REC_SIGNALS))
		  == P_SHORTS)
		 && playable_dimension(s->dim)))
	       return s;
	 }
      }
      if (s0->header
	  && s0->header->magic == ESPS_MAGIC
	  && (ehd = s0->header->esps_hdr)
	  && ehd->common.type == FT_FEA) {
	 if (ehd->variable.refer
	     && (s = get_any_signal(ehd->variable.refer,
				    0.0, 20.0, (int (*) ()) NULL)))
	    return s;
	 else if (ehd->variable.source[0]
                  && strcmp(ehd->variable.source[0],"<stdin>")
		  && (s = get_any_signal(ehd->variable.source[0],
					 0.0, 20.0, (int (*) ()) NULL)))
	    return s;
	 else if (ehd->variable.srchead[0] 
                  && ehd->variable.srchead[0]->variable.source[0]
                  && strcmp(ehd->variable.srchead[0]->variable.source[0],
                            "<stdin>")
	          && (s=find_signal(o, 
                      ehd->variable.srchead[0]->variable.source[0])))
	    return s;
      }
      /* last resort - return signal being looked for external play */
      return s0;
   }
   return NULL;
}

/*************************************************************************/
update_filename_display(new)
   char           *new;
{
   char            sc[50];
   extern Panel_item outputFile_item;
   extern char     outputname[];

   if (*outputname == '@') {
      sscanf(outputname, "%s", sc);
      if (strlen(new))
	 sprintf(outputname, "%s   %s", sc, new);
      else
	 *outputname = 0;
   } else
      strcpy(outputname, new);
   xv_set(outputFile_item, PANEL_VALUE, outputname, 0);
   return;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
char           *
maybe_basename(name, v)
   char           *name;
   View           *v;
{
   extern          shorten_header;
   extern char    *basename();

   if (name && *name && v->shorten_header)
      return (basename(name));
   else
      return (name);
}

/*********************************************************************/
update_window_titles(s)
   Signal         *s;
{
   View           *v = s->views;

   while (v) {
      if (v->canvas && (v->extra_type != VIEW_OVERLAY)) {
	 char            head[200];
	 Frame           f = (Frame) xv_get(v->canvas, XV_OWNER);
	 sprintf(head, "%s (S.F.:%7.1f) {left:%s  mid:%s  right:menu}",
		 maybe_basename(s->name, v), s->freq, v->left_but_proc->name, v->mid_but_proc->name);
	 xv_set(f, XV_LABEL, head, 0);
      }
      v = v->next;
   }
}

/*********************************************************************/
void 
update_control_panel_names()
{
   extern Panel_item newFile_item, newObj_item, overlay_item, outputFile_item;
   extern char     outputname[], inputname[], objectname[], overlayname[];

   xv_set(outputFile_item, PANEL_VALUE, outputname, 0);
   xv_set(newFile_item, PANEL_VALUE, inputname, 0);
   xv_set(newObj_item, PANEL_VALUE, objectname, 0);
   xv_set(overlay_item, PANEL_VALUE, overlayname, 0);
}

/*************************************************************************/
/*
 * A quick and dirty check to see if the first thing in a file is likely a
 * direct command to the program (such as "xwaves make ...").
 */
is_a_command_file(name)
   char           *name;
{
   FILE           *fd;
   int             n;
   char            stuff[501], pname[201];
   extern Object   program;

   if (!strcmp(name, "stdin"))
      return (TRUE);

   if ((fd = fopen(name, "r"))) {
      *stuff = '\0';
      while ((fgets(stuff, 500, fd) != NULL) && (*stuff == '#'));
      fclose(fd);
      n = strlen(stuff);
      if (n) {
	 stuff[n] = ' ';
	 sscanf(stuff, "%s", pname);
	 return (!strcmp(pname, program.name));
      }
   }
   return (FALSE);
}
