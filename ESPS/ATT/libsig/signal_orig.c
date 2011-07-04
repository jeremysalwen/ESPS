/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
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

static char *sccs_id = "@(#)signal.c	1.42	9/28/98	ATT/ESI/ERL";

#include <Objects.h>
#include <spectrogram.h>
#include <esps/esps.h>

char           *savestring();
int             free_data(), vec_time_to_index();
double          vec_buf_start(), vec_buf_end(), vec_sig_dur(), vec_index_to_time();
extern int      debug_level;

extern int      valid_header;	/* ugly global defined in copheader.c; used
				 * to ` indicate case where header is ok but
				 * couldn't read data for some reason (e.g.,
				 * requested segment not in file */

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
output_header(sig)
   Signal         *sig;
{
   int             fd, itsok;

#ifdef DEMO
   return FALSE;
#endif

   if ((fd = sig->file) < 0) {
      if (sig->header && sig->header->magic == ESPS_MAGIC) {
	 struct header  *ehead = NULL;

	 if (ehead = sig->header->esps_hdr) {
	    if (sig->freq > 0.0) {
	       if (get_genhd_val("record_freq", ehead, -1.2) < 0.0)
		  *add_genhd_d("record_freq", (double *) NULL, 1, ehead) =
		     sig->freq;
	    }
	    if (get_genhd_val("start_time", ehead, -1.234) == -1.234)
	       *add_genhd_d("start_time", (double *) NULL, 1, ehead) =
		  sig->start_time;
	 }
	 sig->header->strm = fopen(sig->name, "w+");
	 if (sig->header->strm)
	    fd = fileno(sig->header->strm);
      } else
	 fd = open(sig->name, O_TRUNC | O_RDWR | O_CREAT, 0666);

      sig->file = fd;
   }
   if (fd >= 0) {
      if (sig->header && sig->header->header) {
	 itsok = put_header(sig->header, fd);
	 sig->bytes_skip =
	    (sig->header->magic == ESPS_MAGIC)
	    ? sig->header->esps_nbytes
	    : sig->header->nbytes + 12;
      } else {
	 itsok = 1;
	 sig->bytes_skip = 0;
      }
      return (itsok);
   }
   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Close the file descriptor or, for ESPS files, the stream pointer
 * associated with a Signal. 
 */

close_sig_file(sig)
   Signal         *sig;
{
   char *get_sphere_hdr();

   if (debug_level >= 1) {
      if (sig)
	 printf("%s: closing \"%s\", descriptor %d, header->magic 0x%x.\n",
		"close_sig_file",
		(sig->name) ? sig->name: "NULL", sig->file,
		(sig->header) ? sig->header->magic : 0);
      else
	 printf("%s: signal is NULL.\n", "close_sig_file");
   }
   if (is_feasd_sphere(sig)) {
      if(debug_level) fprintf(stderr, "rewinding sphere file.\n");
      sp_rewind(get_sphere_hdr(sig->header->esps_hdr));
      return;
   }

   if (sig && sig->file >= 0) {
      if (sig->header && sig->header->magic == ESPS_MAGIC) {
 	 fclose(sig->header->strm);
 	 sig->header->strm = NULL;
      } else
	 close(sig->file);
   }
   sig->file = SIG_CLOSED;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Write the signal to the file specified by sig->file (a file descriptor),
 * or, if necessary, open a descriptor using sig->name and write to it. The
 * sig->buff_size specifies the amount of data to be written.
 */
put_signal(sig)
   Signal         *sig;
{
   double          size, start;
   struct header  *hdr;

   if (sig && sig->name) {
      size = BUF_DURATION(sig);
      start = BUF_START_TIME(sig);

      if (sig->start_time != start && sig->header) {
	 if (sig->header->header)
	    head_printf(sig->header, "start_time", (char *) &start);
	 if ((hdr = sig->header->esps_hdr)
	     && sig->header->magic == ESPS_MAGIC) {
	    *(genhd_type("start_time", (int *) NULL, hdr) == DOUBLE
	      ? get_genhd_d("start_time", hdr)
	      : add_genhd_d("start_time", (double *) NULL, 1, hdr)
	       ) = start;
	 }
      }
      sig->start_time = start;
      sig->start_samp = 0;
      if (output_header(sig)) {
	 if (sig->utils->write_data(sig, start, size)) {
	    close_sig_file(sig);
	    return (TRUE);
	 } else
	    printf("Problems writing data in put_signal()\n");
      } else
	 printf("Couldn't open an output file (%s) in put_signal()\n",
		sig->name);
   } else
      printf("Bad signal or no name in put_signal()\n");

   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * A file "name" is opened for reading.  Its header is read and the
 * appropriate read method is invoked to read "page_size" (seconds) of data
 * starting at "start" (seconds).  If the signal is not "generic," the caller
 * must specify its own "read_proc" (reading method).
 */
Signal         *
get_signal(name, start, page_size, read_proc)
   char           *name;
   double          start, page_size;
   int             (*read_proc) ();
{
   Signal         *s;
   Header         *h;
   int             fd;

   if (debug_level)
      (void) fprintf(stderr, "get_signal: function entered\n");

   if ((fd = open(name, O_RDONLY)) >= 0) {
      if (h = get_header(fd)) {
	 if ((s = new_signal(name, fd, h, (caddr_t) NULL, 0, 0.0, 0))
#ifdef DEMO
	     && (h->magic == ESPS_MAGIC
		 || ((s->type & SPECIAL_SIGNALS) == SIG_FORMANTS))
#endif				/* DEMO */
	    ) {
	    valid_header = 1;

	    if (read_proc)
	       s->utils->read_data = read_proc;
	    if (s->utils->read_data(s, start, page_size)) {
	       return (s);
	    }			/* else printf("Trouble reading sample
				 * data\n"); */
	 }			/* else printf("Couldn't create a new
				 * Signal\n"); */
      }				/* else printf("get_header failed\n"); */
      close(fd);
   }				/* else printf("File open on %s
				 * failed\n",name); */
   return (NULL);
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Change the name of a signal to new.  If a file descriptor is open for the
 * signal, close it when the signal is renamed.  s->name is reallocated iff
 * more space is required than for the old name.
 */
/*
 * If signal is associated with an ESPS file, make a new header with the old
 * one as source.
 */

rename_signal(s, new)
   Signal         *s;
   char           *new;
{
   char            comment[200];

   if (new && *new && s && s->name) {
      if (!strcmp(s->name, new))
	 return (TRUE);

      if (s->header && s->header->magic == ESPS_MAGIC && s->header->esps_hdr) {
	 struct header  *hdr;

	 sprintf(comment, "waves rename_signal %s %s\n", s->name, new);
	 hdr = copy_header(s->header->esps_hdr);
/*
 * don't do this, until I figure out a better way to free the esps header
 * without freeing open headers that might be embedded
	 add_source_file(hdr, savestring(s->name), s->header->esps_hdr);
*/
	 s->header->esps_hdr = hdr;
	 set_pvd(hdr);
	 add_comment(hdr, comment);
      }
      if (strlen(new) > strlen(s->name)) {
	 free(s->name);
	 if (!(s->name = (char *) malloc(strlen(new) + 1))) {
	    printf("Allocation failure in rename_signal()\n");
	    return (FALSE);
	 }
      }
      strcpy(s->name, new);

      close_sig_file(s);
      s->file = SIG_NEW;
      return (TRUE);
   }
   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Create a new signal structure.  Initialize all attributes assuming the
 * signal is periodic, scalar, shorts. It will be up to the caller to
 * initialize other cases properly.
 */

Signal         *
new_signal(name, fd, header, data, samples, freq, dimensions)
   char           *name;
   caddr_t         data;
   Header         *header;
   int             fd, samples, dimensions;
   double          freq;
{
   Signal         *s;
   char           *str;
   double          maxt, mint;

   if (debug_level)
      (void) fprintf(stderr, "new_signal: function entered\n");

   if ((s = (Signal *) malloc(sizeof(Signal)))) {
      if (name && *name) {
	 if (!(str = malloc(strlen(name) + 1))) {
	    free(s);
	    printf("Can't allocate memory in new_signal()\n");
	    return (NULL);
	 }
	 strcpy(str, name);
	 s->name = str;
      } else
	 s->name = NULL;
      s->file = fd;
      s->file_size = 0;
      s->buff_size = 0;
      s->freq = 10000.0;
      s->dim = 1;
      s->start_samp = 0;
      s->start_time = 0.0;
      s->end_time = 0.0;
      s->band = freq / 2.0;
      s->band_low = 0.0;
      s->y_dat = NULL;
      s->x_dat = NULL;
      s->type = P_SHORTS;
      s->types = NULL;
      s->version = 1;
      s->obj = NULL;
      maxt = mint = 0.0;
      s->smax = &maxt;
      s->smin = &mint;
      s->others = NULL;
      s->header = NULL;
      s->views = NULL;
      s->idents = NULL;
      s->params = NULL;
      s->methods = NULL;

      /* install default utility methods */
      if (s->utils = (Utils *) malloc(sizeof(Utils))) {
	 s->utils->read_data = read_data;
	 s->utils->write_data = write_data;
	 s->utils->free_data = free_data;
	 s->utils->buf_start = vec_buf_start;
	 s->utils->buf_end = vec_buf_end;
	 s->utils->sig_dur = vec_sig_dur;
	 s->utils->index_to_time = vec_index_to_time;
	 s->utils->time_to_index = vec_time_to_index;
      } else {
	 free(s);
	 printf("Can't allocate memory in new_signal()\n");
	 return (NULL);
      }

      s->bytes_skip = 0;
      s->header = header;
      w_read_header(s, header);
      if (samples) {
	 if (!s->file_size)
	    s->file_size = samples;
	 s->buff_size = samples;
      }
      if (dimensions)
	 s->dim = dimensions;
      else if (s->dim > 0)
	 dimensions = s->dim;
      else
	 dimensions = 1;

      if (freq != 0.0)
	 s->freq = freq;
      if (!((s->smax = (double *) malloc(sizeof(double) * dimensions)) &&
	    (s->smin = (double *) malloc(sizeof(double) * dimensions)))) {
	 printf("Can't allocate memory in new_signal()\n");
	 return (NULL);
      }
      s->smax[0] = maxt;
      s->smin[0] = mint;
      s->data = data;
      return (s);
   }
   return (NULL);
}

/******************************************************************/
void
free_ret(r)
   Reticle        *r;
{
   if (r) {
      if (r->abs_label)
	 free(r->abs_label);
      if (r->ord_label)
	 free(r->ord_label);
      if (r->ordinate.maj.list)
	 free(r->ordinate.maj.list);
      if (r->ordinate.min1.list)
	 free(r->ordinate.min1.list);
      if (r->ordinate.min2.list)
	 free(r->ordinate.min2.list);
      if (r->abscissa.maj.list)
	 free(r->abscissa.maj.list);
      if (r->abscissa.min1.list)
	 free(r->abscissa.min1.list);
      if (r->abscissa.min2.list)
	 free(r->abscissa.min2.list);
      /*
       * Note that this doesn't do a pf_close() on any open pixfonts. If this
       * causes problems, change references to free_ret to free_reticle (in
       * reticle.c) and include about 250kbytes of sunview junk!
       */
      free(r);
   }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void
free_view(v)
   View           *v;
{
   int             i;

   if (v) {
      if (v->x_scale)
	 free(v->x_scale);
      if (v->y_scale)
	 free(v->y_scale);
      if (v->z_scale)
	 free(v->z_scale);
      if (v->x_offset)
	 free(v->x_offset);
      if (v->y_offset)
	 free(v->y_offset);
      if (v->val_offset)
	 free(v->val_offset);
      if (v->val_scale)
	 free(v->val_scale);
      if (v->z_offset)
	 free(v->z_offset);
      if (v->elements)
	 free(v->elements);
      if (v->colors)
	 free(v->colors);
      if (v->line_types)
	 free(v->line_types);
      if (v->mark_reference)
	 free(v->mark_reference);

      /*
       * if extra is more complex, need to check types and handle separately.
       */
      if (v->extra_type == VIEW_BITMAP) {
	 ViewBitmap     *vbm = (ViewBitmap *) v->extra;
	 if (v->free_extra && vbm->bitmap)
	    v->free_extra(vbm->bitmap);
	 if (v->extra)		/* remove `extra' once contents are gone */
	    free(v->extra);
      }
      if (v->ret) {
	 for (i = 0; i < v->dims; i++)
	    free_ret(((char **) (v->ret))[i]);
	 free(v->ret);
      }
      free(v);
   }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * The resources used by Signal s and all s->others are returned to the
 * system. All open file descriptors are closed and all signal "views" are
 * freed. Note that free_signal() frees s->name, so that this should be set
 * to NULL before calling if it is not to be freed (the same is true for all
 * Signal elements: if a pointer is NULL, no free() operation is performed).
 */
void
free_signal(si)
   Signal         *si;
{
   Signal         *s, *s2;
   List           *l, *l2;
   View           *v, *v2;

   s = si;
   while (s) {
      if ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM && s->params)
	 free_spectrogram((Spectrogram *) s->params);
      if (s->smax) {
	 free(s->smax);
	 s->smax = NULL;
      }
      if (s->smin) {
	 free(s->smin);
	 s->smin = NULL;
      }
      if (s->x_dat) {
	 free(s->x_dat);
	 s->x_dat = NULL;
      }
      if (s->y_dat) {
	 free(s->y_dat);
	 s->y_dat = NULL;
      }
      close_sig_file(s);   /* should be called before header or name freed */
      if (s->name) {
	 free(s->name);
	 s->name = NULL;
      }
      if (s->header && s->header->magic == ESPS_MAGIC) {
	 if (is_feasd_sphere(s))
	    close_feasd_sphere(s); 

         if (s->header->esps_clean && s->header->esps_hdr) 
	    free_header(s->header->esps_hdr, (long)0, (char*)NULL);
      }

      if (s->header) {
	 if (s->header->header)
	    free(s->header->header);
	 free(s->header);
	 s->header = NULL;
      }
      if (s->utils->free_data)
	 s->utils->free_data(s);
      free(s->utils);		/* after `free_data' used! */

      l = s->idents;
      while (l) {
	 if (l->str) {
	    free(l->str);
	 }
	 l2 = l->next;
	 free(l);
	 l = l2;
      }
      if ((v = s->views))
	 while (v) {
	    v2 = v->next;
	    free_view(v);
	    v = v2;
	 }

      s2 = s->others;
      free(s);
      s = s2;
   }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
unlink_signal(s)
   Signal         *s;
{
   Object         *o;
   Signal         *s2;

   if (s && (o = (Object *) s->obj)) {
      if (s == (s2 = o->signals)) {
	 o->signals = s->others;
	 s->others = NULL;
	 s->obj = NULL;
	 return (TRUE);
      } else {
	 while (s2) {
	    if (s2->others == s) {
	       s2->others = s->others;
	       s->others = NULL;
	       s->obj = NULL;
	       return (TRUE);
	    }
	    s2 = s2->others;
	 }
      }
   }
   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
unlink_view(v, hv)
   View           *v,		/* view to unlink */
                 **hv;		/* head of its view list */
{
   View           *v2;

   if (v && (v2 = *hv)) {
      if (v2 == v) {
	 *hv = v->next;
      } else {
	 while (v2->next && (v2->next != v))
	    v2 = v2->next;
	 if (!v2->next) {
	    printf("Request to unlink_view(%d) failed; not in list !\n", v);
	    return (FALSE);
	 } else
	    v2->next = v->next;
      }
      return (TRUE);
   }
   return (FALSE);
}

/*************************************************************************/
void
free_spectrogram(s)
   Spectrogram    *s;
{
   if (s) {
      if (s->dimp)
	 free(s->dimp);
      if (s->signame)
	 free(s->signame);
      if (s->outname)
	 free(s->outname);
      free(s);
      /*
       * NOTE: Do NOT attempt to free s->bitmap.  It is only used to pass the
       * address of the pixrect structure!
       */
   }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
View           *
new_view(s, c)
   Signal         *s;
   canvas_t        c;		/* Canvas with SUNVIEW or XVIEW; else caddr_t */
{
   View           *v = NULL, *v2 = NULL;
   Signal         *s2 = NULL;
   Object         *ob = NULL;
   int             i;
   int             dim;

   if (!s)
      return (NULL);
   dim = (s) ? s->dim : 1;
   if ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM)
      dim = 1;

   if ((v = (View *) malloc(sizeof(View)))) {
      if ((v->x_scale = (double *) malloc(sizeof(double))) &&
	  (v->x_offset = (int *) malloc(sizeof(int))) &&
	  (v->y_scale = (double *) malloc(sizeof(double) * dim)) &&
	  (v->y_offset = (int *) malloc(sizeof(int) * dim)) &&
	  (v->z_scale = (double *) malloc(sizeof(double) * dim)) &&
	  (v->val_scale = (double *) malloc(sizeof(double) * dim)) &&
	  (v->val_offset = (double *) malloc(sizeof(double) * dim)) &&
	  (v->z_offset = (int *) malloc(sizeof(int) * dim)) &&
	  (v->v_rescale = (int *) malloc(sizeof(int) * dim)) &&
	  (v->show_vals = (int *) malloc(sizeof(int) * dim)) &&
	  (v->show_labels = (int *) malloc(sizeof(int) * dim)) &&
	  (v->reticle_on = (int *) malloc(sizeof(int) * dim)) &&
	  (v->elements = (int *) malloc(sizeof(int) * dim)) &&
	  (v->plot_max = (double *) malloc(sizeof(double) * dim)) &&
	  (v->plot_min = (double *) malloc(sizeof(double) * dim)) &&
	  (v->colors = (int *) malloc(sizeof(int) * dim)) &&
	  (v->line_types = (int *) malloc(sizeof(int) * dim)) &&
	  (v->ret = (Reticle **) malloc(sizeof(Reticle *) * s->dim))) {
	 v->canvas = c;
	 v->sig = s;
	 v->dims = dim;
	 for (i = 0; i < dim; i++) {
	    v->y_scale[i] = 1.0;/* y units per cm */
	    v->y_offset[i] = 0;
	    v->val_offset[i] = 0.0;
	    v->val_scale[i] = 1.0;
	    v->v_rescale[i] = 1;
	    v->show_vals[i] = 1;
	    v->show_labels[i] = 1;
	    v->reticle_on[i] = 1;
	    v->elements[i] = i;
	    v->colors[i] = 1;
	    v->line_types[i] = 1;
	    if (s->smax && s->smin) {
	       v->plot_max[i] = s->smax[i];
	       v->plot_min[i] = s->smin[i];
	    }
	 }
	 for (i = 0; i < s->dim; i++)
	    ((char **) (v->ret))[i] = NULL;
	 v->width = 1000;	/* Saved since resize proc gives no access */
	 v->height = 200;	/* to previous canvas size. */
	 v->start_time = (s) ? BUF_START_TIME(s) : 0.0;
	 *(v->x_offset) = 0;
	 if (s && v->width)	/* seconds per cm */
	    *(v->x_scale) = BUF_DURATION(s) * PIX_PER_CM /
	       (v->width - *v->x_offset);
	 else
	    *(v->x_scale) = .08;
	 v->plotted = FALSE;
	 v->overlay_n = -1;
	 v->zoom_ratio = 0.5;
	 v->cross_level = 0.0;
	 v->page_step = 2.0;
	 v->background = 0;
	 v->invert_dither = FALSE;
	 v->overlay_as_number = FALSE;
	 v->redraw_on_release = TRUE;
	 v->rewrite_after_edit = TRUE;
	 v->spect_interp = TRUE;
	 v->h_rescale = FALSE;
	 v->rescale_scope = SCOPE_VIEW;
	 v->readout_height = 20;
	 v->shorten_header = 0;
	 v->cursor_channel = 0;
	 v->tmarker_chan = 0;
	 v->bmarker_chan = 0;
	 v->find_crossing = 0;
	 v->next = NULL;
	 v->extra = NULL;
	 v->extra_type = VIEW_STANDARD;
	 v->scrollbar = NULL;
	 v->left_but_proc = NULL;
	 v->mid_but_proc = NULL;
	 v->move_proc = NULL;
	 v->mark_reference = NULL;
	 v->right_but_proc = NULL;
	 v->handle_scrollbar = NULL;
	 v->data_plot = NULL;
	 v->cursor_plot = NULL;
	 v->hmarker_plot = NULL;
	 v->vmarker_plot = NULL;
	 v->reticle_plot = NULL;
	 v->x_print = NULL;
	 v->y_print = NULL;
	 v->free_extra = NULL;
	 v->set_scale = NULL;
	 v->time_to_x = NULL;
	 v->x_to_time = NULL;
	 v->xy_to_chan = NULL;
	 v->yval_to_y = NULL;
	 v->y_to_yval = NULL;
	 if (s->band > 0.0) {
	    v->start_yval = s->band_low;
	    v->end_yval = s->band_low + s->band;
	 } else {
	    v->start_yval = 0.0;
	    v->end_yval = s->freq / 2;	/* Nyquist limit */
	 }

	 /*
	  * if a previous view exists in this object, use its current
	  * cursor/marker/etc. values to init this new view
	  */
	 if (s && (ob = (Object *) s->obj)) {	/* find a previous view */
	    for (v2 = NULL, s2 = ob->signals; s2 && !v2; s2 = s2->others)
	       for (v2 = s2->views; v2; v2 = v2->next)
		  if (v != v2)
		     break;	/* found a previous view */

	 }
	 if (v2) {		/* if not first, use current values */
	    v->cursor_time = v2->cursor_time;
	    v->cursor_yval = v2->cursor_yval;
	    v->cursor_channel = v2->cursor_channel;
	    v->tmarker_chan = v2->tmarker_chan;
	    v->bmarker_chan = v2->bmarker_chan;
	    v->lmarker_time = v2->lmarker_time;
	    v->rmarker_time = v2->rmarker_time;
	    v->tmarker_yval = v2->tmarker_yval;
	    v->bmarker_yval = v2->bmarker_yval;
	 } else {		/* if first view, make up values */
	    v->cursor_time = v->start_time;
	    v->cursor_yval = v->start_yval;
	    v->lmarker_time = v->start_time;
	    v->rmarker_time = ET(v);
	    v->tmarker_yval = 0.0;
	    v->bmarker_yval = 0.0;
	 }
	 return (v);
      } else
	 printf("Can't allocate data arrays in new_view()\n");
   } else {
      printf("Can't allocate a View in new_view()\n");
   }
   return (NULL);		/* default if failure */
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#define GET_MAXMIN(the_type) { \
    register the_type	**dpp = (the_type **) sig->data; \
    for (i = 0; i < sig->dim; i++) { \
	register the_type   *p, *q, ima, imi, t; \
	for (p = dpp[i], q = p + sig->buff_size, ima = imi = *p++; p < q; ) { \
	    if ((t = *p++) > ima) ima = t; \
	    else \
		if (t < imi) imi = t; \
	} \
	sig->smax[i] = ima; \
	sig->smin[i] = imi; \
    }}

#define GET_MAXMIN_1(the_type) { \
    register the_type	*dp = (the_type *) s_data; \
    register the_type   *p, *q, ima, imi, t; \
    for (p = dp, q = p + sig->buff_size, ima = imi = *p++; p < q; ) { \
	if ((t = *p++) > ima) ima = t; \
	else \
	    if (t < imi) imi = t; \
    } \
    sig->smax[i] = ima; \
    sig->smin[i] = imi; \
    }

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
get_maxmin(sig)
   Signal         *sig;
{
   if (sig && sig->type && sig->data) {
      if (IS_GENERIC(sig->type)) {
	 register int    i;
	 switch (sig->type & VECTOR_SIGNALS) {
	 case P_CHARS:
	 case P_UCHARS:
	    GET_MAXMIN(char);
	    break;
	 case P_SHORTS:
	    GET_MAXMIN(short);
	    break;
	 case P_FLOATS:
	    GET_MAXMIN(float);
	    break;
	 case P_INTS:
	 case P_UINTS:
	    GET_MAXMIN(int);
	    break;
	 case P_DOUBLES:
	    GET_MAXMIN(double);
	    break;
	 case P_MIXED:
	    for (i = 0; i < sig->dim; i++) {
	       caddr_t         s_data = ((caddr_t *) sig->data)[i];

	       switch (sig->types[i]) {
	       case P_CHARS:
	       case P_UCHARS:
		  GET_MAXMIN_1(char);
		  break;
	       case P_SHORTS:
		  GET_MAXMIN_1(short);
		  break;
	       case P_FLOATS:
		  GET_MAXMIN_1(float);
		  break;
	       case P_INTS:
	       case P_UINTS:
		  GET_MAXMIN_1(int);
		  break;
	       case P_DOUBLES:
		  GET_MAXMIN_1(double);
		  break;
	       default:
		  printf("Unknown element data type in get_maxmin(%x)\n",
			 sig->types[i]);
		  return (FALSE);
	       }
	    }
	    break;
	 default:
	    printf("Unknown data type in get_maxmin(%x)\n", sig->type);
	    return (FALSE);
	 }
	 return (TRUE);
      } else
	 printf("Unknown data type in get_maxmin(%x)\n", sig->type);
   }
   return (FALSE);
}
#undef GET_MAXMIN_1
#undef GET_MAXMIN

/*************************************************************************/

/*
 * These routines compute buffer start/end times in seconds. Routines like
 * these must be pointed to by s->utils->buf_start and s->utils->buf_end.
 * This is set up in new_signal() for generic signals and should be setup in
 * read_<non-generic>_data() for non-generic types.
 */

double
vec_buf_start(s)		/* (buffer) start time for Vector signals */
   register Signal *s;
{
   return (s->start_time + (((double) (s->start_samp)) / s->freq));
}

double
vec_buf_end(s)			/* (buffer) end time for Vector signals */
   register Signal *s;
{
   return (s->start_time + (((double) (s->start_samp + s->buff_size)) / s->freq));
}

/*
 * Signal duration routines.
 */

double
vec_sig_dur(s)
   register Signal *s;
{
   return s->file_size / s->freq;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
double
vec_index_to_time(s, index)
   Signal         *s;
   int             index;
{
   if (index < 0)
      index = 0;
   else if (index >= s->buff_size)
      index = s->buff_size - 1;
   return (s->start_time + (((double) (s->start_samp + index)) / s->freq));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
vec_time_to_index(s, time)
   Signal         *s;
   double          time;
{
   double          rtim;
   int             ind;

   if (time > (rtim = BUF_END_TIME(s)))
      time = rtim;
   if ((rtim = (time - BUF_START_TIME(s))) < 0.0)
      rtim = 0.0;
   ind = (int) (0.5 + (s->freq * rtim));
   if (ind >= s->buff_size)
      ind = s->buff_size - 1;
   return (ind);
}


/*
 * Misc. utility routines.
 */

free_data(s)			/* default fn. works on multi-dim. vectors, */
   register Signal *s;		/* (anything with table of channel vectors) */
{
   register void **dpp;
   register int    n;
   if (s && (n = s->dim) && (dpp = (void **) s->data)) {
      while (n-- > 0)
	 if (dpp[n])
	    free(dpp[n]);
      free(dpp);
   }
   return (TRUE);		/* meaningless return value */
}

/*************************************************************************/
/*
 * Return a pointer to the vector element array corresponding to the "name"
 * argument.  The returned pointer type must be coerced appropriately by the
 * caller.  If sig->idents is null, the "identifiers" in the header are used.
 * If sig->idents is non-null, it is assumed to contain a valid ordered list
 * of vector-element identifiers, in which case sig->header is not reparsed.
 */
char           *
get_signal_element(sig, name)
   Signal         *sig;
   char           *name;
{
   extern Selector head_a0;

   if (sig && sig->data && (sig->header || sig->idents) && name && *name) {
      List           *l = sig->idents;
      int             ind = 0;

      if (!l) {
	 setup_access(NULL);
	 head_a0.dest = (char *) &(sig->idents);
	 get_header_args(sig->header->header, &head_a0);
	 l = sig->idents;
      }
      while (l && (ind < sig->dim)) {
	 if (!strcmp(l->str, name))
	    return (((char **) (sig->data))[ind]);
	 ind++;
	 l = l->next;
      }
   }
   return (NULL);
}
