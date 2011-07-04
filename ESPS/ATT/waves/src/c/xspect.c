/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1987-1990  AT&T, Inc. "Copyright (c) 1986-1990  Entropic
 * Speech, Inc. "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc.
 * All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  David Talkin (AT&T) Checked by: Revised by:  Rod Johnson, Alan
 * Parker, John Shore (ERL)
 * 
 * xspect.c computation and display of spectrograms
 */

static char *sccs_id = "@(#)xspect.c	1.32 9/28/98 ERL";

#include <xview/cms.h>
#include <xview/font.h>
#include <sys/ioctl.h>

#include <esps/esps.h>
#include <esps/feaspec.h>
#include <esps/unix.h>
#include <esps/window.h>

#include <Objects.h>
#include <spectrogram.h>
#include <dsplock.h>
#include <esps/exview.h>

/*#include <Xp_pw.h> */
#define _NO_PROTO
/*#include <Xp.h>
#include <XpMacros.h>
*/
/*
   #include <X11/Xutil.h>
 */

#define ALWAYS_LOAD     1
#define LOAD_AS_NEEDED  2
#define BW_OVERLAYS 1

#define HI_SPECT_LIM  (127 - 22)
/*
 * This fudge factor of 22 is to translate the image_clip semantics for
 * monochrome dither to a reasonable value for color/greyscale. Without this
 * fudge factor, all image_clip entries in various environments worldwide
 * would need to be changed!)
 */

extern double log ();
extern S_bar *new_scrollbar ();

extern void repaint ();
extern void set_pvd ();
extern int run_esps_prog ();
extern char *mk_esps_temp ();
extern double nonlin_y_to_yval ();
extern int nonlin_yval_to_y ();

extern int debug_level, cmap_depth;
extern Visual *visual_ptr;
extern char sgram_program[];
extern int dsp32_wait;
extern int use_dsp32, dsp_type;
extern int invert_dither;	/* inverse video in dithered spectrograms? */
extern Xv_Font def_font;
extern int def_font_height, def_font_width;
extern int menu_item_key;	/* key for storing last selected menu item as
				 * XV_KEY_DATA for canvas. */
extern int new_width,		/* dimensions of new window made by */
  new_height;			/* make command */

char spect_prog[100] = "";
static int use_spect_prog = 0;	/* 0 = do not use dsp_prog_name from
				 * params file; 1 = means do use it.
				 * This is so that we can wire in a
				 * path for each type of DSP */
extern int use_static_cmap;

extern Frame daddy;

double image_clip = 7.0, image_range = 40.0;

extern char wb_spect_params[], nb_spect_params[];

char *spect_params = wb_spect_params;
extern int next_x, next_y, w_y_incr, w_x_incr, da_done;
extern int scrollbar_height, readout_bar_height;
char loaded[100];
extern short dsp_is_open;
extern char *dspmap ();
char *inc_esps_name ();
char *sh_mem;
char *savestring ();

static void set_spect_scale ();
static int interp ();

Menu make_spect_menu ();
extern Menu spect_menu;

void cmap_spect ();

extern void e_play_between_marks (), e_play_window (), e_play_file (),
  e_play_from_cursor (), e_page_ahead (), e_page_back (), e_align_views (),
  e_position_view (), e_zoom_in (), e_zoom_out (), e_wb_spectrogram (),
  e_nb_spectrogram (), e_output_bitmap (), e_save_segment (), e_delete_segment (),
  e_insert_file (), e_kill (), e_forward_window (), e_backward_window (),
  e_repeat_previous (), e_zoom_full_out (), e_blow_up (), e_blow_up_call_function (),
  e_up_down_move_marks (), e_move_closest_mark (), e_modify_signal (),
  m_play_file (), e_reassign_formant (), e_modify_intensity (), e_move_contour (),
  menu_operate (), c_print_graphic (), e_print_ensemble ();

static Menuop
  srbo16 =
{"print ensemble", e_print_ensemble, NULL, NULL}, srbo15 =
{"print graphic", c_print_graphic, NULL, &srbo16}, srbo14 =
{"kill window", e_kill, NULL, &srbo15}, srbo13 =
{"insert file", e_insert_file, NULL, &srbo14}, srbo12 =
{"delete segment", e_delete_segment, NULL, &srbo13}, srbo11 =
{"save segment in file", e_save_segment, NULL, &srbo12}, srbo8 =
{"zoom full out", e_zoom_full_out, NULL, &srbo11}, srbo7 =
{"zoom out", e_zoom_out, NULL, &srbo8}, srbo6b =
{"zoom in", e_zoom_in, NULL, &srbo7}, srbo6 =
{"bracket markers", e_position_view, NULL, &srbo6b}, srbo5c =
{"align & rescale", e_align_views, NULL, &srbo6}, srbo5b =
{"window back", e_backward_window, NULL, &srbo5c}, srbo5a =
{"window ahead", e_forward_window, NULL, &srbo5b}, srbo5 =
{"page back", e_page_back, NULL, &srbo5a}, srbo4 =
{"page ahead", e_page_ahead, NULL, &srbo5}, srbo3 =
{"play to end of file", e_play_from_cursor, NULL, &srbo4}, srbo2 =
{"play entire file", e_play_file, NULL, &srbo3}, srbo1 =
{"play window contents", e_play_window, NULL, &srbo2};
Menuop
right_sbut_menu =
{
  "play between marks", e_play_between_marks, NULL, &srbo1
}
,
spect_operators =
{
  "repeat previous", e_repeat_previous, NULL, &right_sbut_menu
};
/*
 * The following menu ops. are used by left and middle buttons of spectrogram
 * windows.  The arguments for the called routines are (canvas,event,arg).
 */
Menuop
slbo3 =
{
  "mark formants", e_reassign_formant, NULL, &spect_operators
}
,
slbo2 =
{
  "move closest", e_move_closest_mark, NULL, &slbo3
}
,
def_aux_ops_bws =
{
  "up/down move", e_up_down_move_marks, NULL, &slbo2
}
,
smbo3 =
{
  "move contour", e_move_contour, NULL, &def_aux_ops_bws
}
,
def_aux_sbut_ops =
{
  "modify intensity", e_modify_intensity, NULL, &smbo3
};

Menuop *aux_sbut_ops = &def_aux_sbut_ops;

extern char def_smiddle_op[], def_move_op[];
extern char def_sleft_op[];

void (*right_sbutton_proc) () = menu_operate;

int wind_type = 3;

extern int doing_print_graphic, print_only_plot;
Display *get_xp_display ();
int bits_per_pixel;

/*************************************************************************/

#if defined(SUN4) || defined(HP700) || defined(SG)
#define CLIENT_BYTE_ORDER MSBFirst
#endif

#if defined(LINUX_OR_MAC) || defined(DEC_ALPHA)
#define CLIENT_BYTE_ORDER LSBFirst
#endif

/*************************************************************************/

static int
rev (i, bits)
     int i;
     int bits;
{
  int rev_i = i;
  switch (bits)
    {
    case 8:
      rev_i = ((rev_i & 0xf0) >> 4) | ((rev_i & 0x0f) << 4);
    case 4:
      rev_i = ((rev_i & 0xcc) >> 2) | ((rev_i & 0x33) << 2);
    case 2:
      rev_i = ((rev_i & 0xaa) >> 1) | ((rev_i & 0x55) << 1);
      break;
    default:
      spsassert (0, "error in call to rev (bits)");
    }
  return rev_i;
}

static void
bit_reverse (buffer, num_read)
     char *buffer;
     int num_read;
{
  int i;
  for (i = 0; i < num_read; i++)
    {
      buffer[i] = rev (buffer[i], 8);
    }
}

static int
byte_swap (t)
     int t;
{
  char tmp;
  union
    {
      int l_val;
      char byte[4];
    }
  u2;
  u2.l_val = t;

  tmp = u2.byte[0];
  u2.byte[0] = u2.byte[3];
  u2.byte[3] = tmp;
  tmp = u2.byte[1];
  u2.byte[1] = u2.byte[2];
  u2.byte[2] = tmp;

  return u2.l_val;
}

static void
byte_swap_s (s)
     short *s;
{
  char tmp;
  union
    {
      short s_val;
      char byte[2];
    }
  u2;
  u2.s_val = *s;

  tmp = u2.byte[0];
  u2.byte[0] = u2.byte[1];
  u2.byte[1] = tmp;

  *s = u2.s_val;
}

/*************************************************************************/
Menuop *
spect_get_ops ()
{
  return (&spect_operators);
}

/*************************************************************************/
Spectrogram *
new_spectrogram (sig)
     Signal *sig;
{
  Spectrogram *spect;

  if (!(spect = (Spectrogram *) malloc (sizeof (Spectrogram))))
    {
      return (NULL);
    }
  if (sig)
    {
      spect->start_time = BUF_START_TIME (sig);
      spect->end_time = BUF_END_TIME (sig);
    }
  else
    {
      spect->start_time = 0.0;
      spect->end_time = 0.0;
    }
  spect->sig = sig;		/* original signal */
  if (sig->name && *(sig->name))
    {
      spect->signame = malloc (strlen (sig->name) + 1);
      strcpy (spect->signame, sig->name);
    }
  else
    spect->signame = NULL;
  spect->outname = NULL;
  spect->sigfreq = sig->freq;
  if (!get_spgm_params (spect_params, spect))
    {
      sprintf (notice_msg, "Problems reading spectrogram parameter file %s.",
	       spect_params);
      show_notice (1, notice_msg);
      free ((char *) spect);
      return (NULL);
    }
  return (spect);
}

/*************************************************************************/
get_spgm_params (name, sp)
     char *name;
     Spectrogram *sp;
{
  FILE *fdp, *fopen ();
  char *param_file;
  int i, j;
  char in[501], *full_path (), *apply_waves_input_path ();
  double freq;
  static double wsize = 6.4, wstep = 2.0, qstep = 1.0, thresh = 35.0, agc = .55,
    var = 0.0, preemp = -1.0;
  static int nfft = 128, yinterp = 4, xdith = 0, ydith = 0;
  static int dix = 3, diy = 11;	/* default spatial filter dimensions */
  static double di_filt[] =
  {1., 1.5, 2., 3., 4., 5., 4., 3., 2., 1.5, 1.,
   2., 3., 4., 6., 8., 10., 8., 6., 4., 3., 2.,
   4., 6., 8., 12., 16., 1., 0., 0., 0., 0., 0.};

  static Selector
    a14 =
  {"use_dsp_prog_name", "%d", (char *) &use_spect_prog, NULL}, a13 =
  {"window_type", "%d", (char *) &wind_type, &a14}, a12 =
  {"dsp_prog_name", "%s", spect_prog, &a13}, a11 =
  {"win_size_ms", "%lf", (char *) &wsize, &a12}, a10 =
  {"win_step_ms", "%lf", (char *) &wstep, &a11}, a9 =
  {"ampl_scale", "%lf", (char *) &qstep, &a10}, a8 =
  {"ampl_offset_db", "%lf", (char *) &thresh, &a9}, a7 =
  {"agc_ratio", "%lf", (char *) &agc, &a8}, a6 =
  {"var_norm", "%lf", (char *) &var, &a7}, a5 =
  {"fft_points", "%d", (char *) &nfft, &a6}, a4 =
  {"freq_intrp", "%d", (char *) &yinterp, &a5}, a3 =
  {"preemp", "%lf", (char *) &preemp, &a4}, a2 =
  {"x_dither", "%d", (char *) &xdith, &a3}, a1 =
  {"y_dither", "%d", (char *) &ydith, &a2};

  xdith = ydith = 0;
  param_file = apply_waves_input_path (NULL, name);
  if (!(fdp = fopen (param_file, "r")))
    {
      char *tmp_name;
      tmp_name = find_esps_file (NULL, "wb_params", "$ESPS_BASE/lib/waves/files", NULL);
      fdp = fopen (tmp_name, "r");
      if (fdp)
	{
	  sprintf (notice_msg,
	     "Can't find readable spectrogram parameter file %s.\nUsed %s.",
		   name, tmp_name);
	  show_notice (0, notice_msg);
	}
      free (tmp_name);
    }
  free (param_file);
  if (fdp)
    {
      while (fgets (in, 500, fdp) && (*in != '*'))
	if ((*in != '#') &&
	    (*in != '\n'))	/* ignore empty lines and comments */
	  get_args (in, &a1);
      if (xdith && ydith)
	{
	  sp->dimp = (double *) malloc (sizeof (double) * xdith * ydith);
	  for (i = 0; i < xdith; i++)
	    {
	      for (j = 0; j < ydith; j++)
		fscanf (fdp, "%lf ", &sp->dimp[(i * ydith) + j]);
	    }
	}
      fclose (fdp);
    }
  else
    {
      sprintf (notice_msg, "%s %s\n%s\n%s",
	       "Can't find readable spectrogram parameter file",
	       name,
	       "or default file $ESPS_BASE/lib/waves/files/wb_params.",
	       "Using default (compiled-in) parameters.");
      show_notice (0, notice_msg);
    }
  if (!(xdith && ydith))
    {
      xdith = dix;
      ydith = diy;
      sp->dimp = (double *) malloc (sizeof (double) * (j = xdith * ydith));
      for (i = 0; i < j; i++)
	sp->dimp[i] = di_filt[i];
    }
  sp->xdith = xdith;
  sp->ydith = ydith;
  if ((wind_type < 0) || (wind_type > 9))
    {
      sprintf (notice_msg, "Bad window type (%d) in get_spgm_params.",
	       wind_type);
      show_notice (1, notice_msg);
      return (0);
    }
  sp->window_type = wind_type;
  sp->window_size = wsize;
  sp->window_step = wstep;
  sp->q_step = qstep;
  sp->q_thresh = thresh - 40;	/* (for compatability with old .wave_pro
				 * files) */
  sp->agc_ratio = agc;
  sp->preemp = preemp;
  sp->var_ratio = var;
  if (!yinterp)
    yinterp = 1;
  sp->yinterp = yinterp;
  if (nfft > 1024)
    {
      show_notice (0,
	    "Max fft size available is 1024 points; limiting accordingly.");
      nfft = 1024;
    }
  sp->nfft = nfft;
  sp->bitmap = XV_NULL;

  /* convert ms to seconds and integerize the time step and size */
  if (sp->sig && (sp->sig->freq > 0.0))
    freq = sp->sig->freq;
  else
    freq = 10000.;		/* what else can one do? */
  i = 0.5 + (freq * sp->window_step / 1000.);
  if (!i)
    {
      sprintf (notice_msg, "Window step (%fms) too small in get_spgm_params().",
	       sp->window_step);
      show_notice (1, notice_msg);
      return (FALSE);
    }
  sp->window_step = ((double) i) / freq;	/* install actual step size
						 * implemented */

  if ((sp->yinterp * sp->nfft / 2) > SCREEN_HEIGHT)
    {				/* pixels vertically... */
      sprintf (notice_msg, "%s\n%s %d\n%s",
	       "Specified vertical size too large.",
	       "Limiting to <=",
	       SCREEN_HEIGHT,
	       "(or put the monitor on its side).");
      show_notice (0, notice_msg);

      sp->yinterp = 2 * SCREEN_HEIGHT / sp->nfft;
    }
  i = 0.5 + (freq * sp->window_size / 1000.);
  if (i < 4)
    {
      sprintf (notice_msg, "Window size (%fms) too small in get_spgm_params().",
	       sp->window_size);
      show_notice (1, notice_msg);
      return (FALSE);
    }
  while (i > sp->nfft)
    {
      if (sp->nfft <= 512)
	{
	  sp->nfft *= 2;
	  if (sp->yinterp >= 2)	/* Keep vertical size constant, if possible. */
	    sp->yinterp /= 2;
	}
      else
	{
	  i = 1024;		/* biggest FFT available on DSP board */
	  sp->yinterp = 1;	/* assumes vertical display size <= 900
				 * pixels */
	  sprintf (notice_msg,
		   "Specified window size too big; window limited to %fms.",
		   (1000.0 * i) / freq);
	  show_notice (0, notice_msg);
	  break;
	}
    }
  sp->window_size = ((double) i) / freq;
  return (TRUE);
}

/*************************************************************************/
Signal *
spectrogram (sig, type, start, end, output)
     Signal *sig;
     double start, end;
     char *type, *output;
{
  Xv_Cursor cursor;
  Canvas canvas;
  Spectrogram *spect;
  short *p;
  int wasopen;
  double dtmp;
  Signal *ss, *spgm_32 (), *spgm_32C (), *e_spect (), *e_spgm ();
  View *view;
  int is_p_shorts, esps_ok;

  if (!sig)
    return (NULL);

  if (!strcmp (type, "narrowband"))
    spect_params = nb_spect_params;
  else
    spect_params = wb_spect_params;

  if (!(spect = new_spectrogram (sig)))
    {
      show_notice (1, "xwaves couldn't create spectrogram signal.");
      return (NULL);
    }
  if (end > (dtmp = sig->start_time + ((double) sig->file_size) / sig->freq))
    end = dtmp;
  if (start < sig->start_time)
    start = sig->start_time;
  /* don't create absurdly short (spatially speaking) spectrograms */
  if ((end - start) / spect->window_step < 8.0)
    goto free_bailout;
  spect->start_time = start;
  spect->end_time = end;
  spect->outname = savestring (output);

  is_p_shorts = sig->type & (VECTOR_SIGNALS | APERIODIC_SIGNALS | VAR_REC_SIGNALS);

  esps_ok = sig->header
    && sig->header->magic == ESPS_MAGIC
    && sig->file == SIG_CLOSED;

  if ((!use_dsp32 && esps_ok) || (esps_ok && !sig->header->e_scrsd))
    ss = e_spect (spect);	/* No board; up-to-date ESPS file */
  /*
   * or any old ESPS file (including FEA_SD not read as scrsd) (might work)
   */
  else if (!is_p_shorts || (is_p_shorts && sig->dim != 1))
    {
      show_notice (1, "Data type not supported in spectrogram().");
      return (NULL);
    }
  else if ((use_dsp32 && ((dsp_type == DSP32_FAB2) || (dsp_type == DSP32C_VME)))
	   && ((start < BUF_START_TIME (sig)) || (end > BUF_END_TIME (sig))))
    {
      wasopen = (sig->file >= 0);
      if (!sig->utils->read_data (sig, start, end - start))
	{
	  sprintf (notice_msg,
	      "Can't read specified data segment (%f %f) in spectrogram().",
		   start, end - start);
	  show_notice (1, notice_msg);
	  goto free_bailout;
	}
      if (dsp_type == DSP32_FAB2)
	ss = spgm_32 (spect);
      else if (dsp_type == DSP32C_VME)
	ss = spgm_32C (spect);

      if (sig->views)
	{			/* restore displayed segment, if any */
	  start = sig->views->start_time;
	  end = ET (sig->views) - start;	/* i.e. not end, but duration */
	  get_view_segment (sig->views, &start, &end);
	}
      if (!wasopen)
	close_sig_file (sig);
    }
  else
    {				/* use segment already in memory */
      if (use_dsp32 && dsp_type == DSP32_FAB2)
	ss = spgm_32 (spect);
      else if (use_dsp32 && dsp_type == DSP32C_VME)
	ss = spgm_32C (spect);
      else
	ss = e_spgm (spect);
    }

  link_new_signal ((Object *) sig->obj, ss);	/* put at head of signal list */

  if (!(view = new_spect_display (ss)))
    {
      free_signal (ss);
      goto free_bailout;
    }
  return (ss);

free_bailout:
  if (spect)
    free_spectrogram (spect);
  return (NULL);
}

/*************************************************************************/
View *
new_spect_display (ss)
     Signal *ss;
{
  View *view;

  if (!(view = new_spect_view (ss, FIXED_SCALE, NULL)))
    return (NULL);
  if (!new_spect_window (ss))
    return (NULL);
  update_window_titles (ss);
  return (view);
}

#define RETURN(x) {DSP_UNLOCK; return (x);}

/*************************************************************************/
Signal *
spgm_32 (sp)
     Spectrogram *sp;
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(STARDENT_3000) || defined(hpux) ||  defined(SONY) || defined(M5600) || defined(IBM_RS6000) || defined(SG) || defined(OS5) || defined(DEC_ALPHA) || defined(LINUX_OR_MAC)
  return;
#else
  int size, step, nfrm, nsamps, nx, ny, x, init = 1;
  register int i, cnt;
  register short *p, *q, *data;
  static float fwind[1024];
  struct mem_dpr
    {
      short nfft, win, step, ni;
      float pre, qc[5], qbw[5];
      short fnx, fny, fni, fnh;
      float fh[100];
      float window[512];
    }
  dp;
  char **dpp;
  Signal *ss;
  struct header *hdr;
  double sf;
  char comment[300];
  char name[200], *cp, *full_path ();
  static short bitmap[2048];
  Signal *sig;

  void spblit ();

  static int INBUF_CHARS = INBUF_CHARS_32;
  static int INBUF_SHORTS = INBUF_SHORTS_32;
  char *dsp_bin = NULL;

  if (!sp || !(sig = sp->sig))
    {
      show_notice (1, "Bad argument to spgm_32.");
      return NULL;
    }
  switch (DSP_LOCK (dsp32_wait))
    {
    case LCKFIL_OK:
      break;
    case LCKFIL_INUSE:
      show_notice (1, "DSP board in use.");
      return NULL;
      break;
    default:
    case LCKFIL_ERR:
      show_notice (1,
		"Error in trying to secure exclusive access to DSP board.");
      return NULL;
      break;
    }

  if (use_spect_prog && spect_prog)
    dsp_bin = FIND_FAB2_BIN (NULL, spect_prog);
  if (!(use_spect_prog && setup_dsp (dsp_bin, ALWAYS_LOAD) ||
	setup_dsp ((cp = FIND_FAB2_BIN (NULL, "dspgm")), ALWAYS_LOAD)))
    {
      sprintf (notice_msg, "Couldn't load dsp32 program %s or default %s.",
	       spect_prog, cp);
      show_notice (1, notice_msg);
      RETURN (NULL)
    }
  if (dsp_bin)
    free (dsp_bin);
  if (cp)
    free (cp);

  dsprg (0, C_RUN, 0xffff001b);	/* start prog. (it will wait for param load) */

  dp.nfft = sp->nfft;		/* stuff spectrogram parameters to board */
  dp.win = size = (0.5 + (sig->freq * sp->window_size));
  if (size > INBUF_SHORTS / 2)
    {
      sprintf (notice_msg, "%s (%d)\n%s (%d).",
	       "Requested window size for spectrogram",
	       size,
	       "exceeds available buffer size",
	       INBUF_SHORTS);
      show_notice (1, notice_msg);

      RETURN (NULL)
    }
  dp.step = step = (0.5 + (sig->freq * sp->window_step));
  if ((size < 4) || !step)
    {
      sprintf (notice_msg,
	       "Unreasonable step (%d) or window (%d) size in spgm_32.",
	       step, size);
      show_notice (1, notice_msg);
      RETURN (NULL)
    }
  dp.ni = sp->yinterp;
  dp.qc[0] = sp->q_thresh;
  dp.qc[1] = sp->agc_ratio;
  dp.qc[2] = sp->q_step;
  dp.qc[3] = sp->var_ratio;
  dp.qbw[0] = sp->q_thresh + 60;
  dp.qbw[1] = sp->agc_ratio;
  dp.qbw[2] = sp->q_step * 15;
  dp.qbw[3] = sp->var_ratio;
  dp.pre = sp->preemp;
  dp.fnx = sp->xdith;
  dp.fny = sp->ydith;
  if (!do_color)
    dp.fni = sp->yinterp;	/* Also used as a flag to skip dithering */
  else
    dp.fni = 0;			/* when computing color/grayscale
				 * spectrograms. */
  dp.fnh = sp->xdith * sp->ydith;

  for (i = 0; i < dp.fnh; i++)
    dp.fh[i] = sp->dimp[i];
  if (!get_float_window (fwind, size, sp->window_type))
    {
      show_notice (1, "Can't get_float_window() in spgm_32.");
      RETURN (NULL)
    }
  for (i = 0, cnt = (size + 1) / 2; i < cnt; i++)
    dp.window[i] = fwind[i];

  /*
   * Now blast the structure to the dsp32 board as a short array. This is
   * necessary to assure that the actual transfers are shorts, since the
   * 680xx and SPARC architectures differ and the assemblers do different
   * things.
   */
  for (i = sizeof (struct mem_dpr) / 2, p = (short *) &dp, q = (short *) sh_mem; i-- > 0;)
    *q++ = *p++;

  dsprg (0, C_PIR);		/* notify dsp32 that params are loaded */

  /* nsamps is total number of samples to be processed */
  nsamps = (0.5 + ((sp->end_time - sp->start_time) * sig->freq));
  ny = (sp->yinterp * sp->nfft) / 2;	/* number of pixels per spectrum */
  /* number of time-axis pixels forced to be modulo sizeof(int) */
  nx = ((int) (1 + ((nsamps - size) / step)) / sizeof (int)) * sizeof (int);
  if (nx <= 0)
    {
      show_notice (1, "Spectrogram too short to be computed in spgm_32.");
      RETURN (NULL)
    }
  for (i = 0; i < 1000; i++)	/* wait for params acknowledge */
    if (dsprg (0, C_PCR) & PCR_PIF)
      break;
  if (i >= 1000)
    {
      show_notice (1, "DSP32 board/program not responding in spgm_32.");
      RETURN (NULL)		/* dead board? */
    }
  /*
   * get input data address.  Note that this assumes all input data in
   * memory!
   */
  if (!sig->data
      || !(data = ((short **) sig->data)[0]))
    {
      show_notice (1, "NULL signal data in spgm_32.");
      RETURN (NULL)
    }
  else
    data += time_to_index (sig, sp->start_time);

  /* nfrm will be the number of frames computed with each call to the dsp32 */
  /* Need space for dithered version? (need 1 short/row for returned bitmap) */
  nfrm = 2 * (INBUF_CHARS - ((do_color) ? 0 : ny)) / ny;	/* Limit by output buf
								 * size */
  nfrm &= ~1;			/* force an even number */
  if ((step * nfrm) > (INBUF_SHORTS - size))	/* or is it limited by input
						 * buf? */
    nfrm = ((INBUF_SHORTS - size) / step) & ~1;		/* force even number */
  if (!nfrm)
    {
      show_notice (1,
		   "Specified spectrogram parameters exceed available dsp32 buffer space.");
      RETURN (NULL)
    }
  if (!do_color && (nfrm > 16))
    nfrm = 16;			/* output limited by shortint size */

  /* Now prepare the output signal structure. */
  if (sp->outname && *sp->outname)
    strcpy (name, sp->outname);
  else
    sprintf (name, "%s.spgm", sp->signame);	/* this SHOULD be made unique */

  if (!(ss = new_signal (name, SIG_NEW,
			 dup_header (sig->header), NULL, nx,
			 1.0 / sp->window_step, ny)))
    {
      show_notice (1, "Can't make new_signal() in spgm_32.");
      RETURN (NULL)
    }
  if (ss->header && ss->header->magic == ESPS_MAGIC && ss->header->esps_hdr)
    {
      hdr = new_header (FT_FEA);
      sf = sp->sigfreq;
      init_feaspec_hd (hdr, FALSE, SPFMT_SYM_EDGE, SPTYP_DB,
		       TRUE, (long) ny, SPFRM_FIXED, (float *) NULL,
		       sf, (long) ROUND (sp->window_size * sf), BYTE);
      set_pvd (hdr);
      sprintf (comment,
	       "xwaves spectrogram: start_time %g end_time %g signal %s\n",
	       sp->start_time, sp->end_time, sig->name);
      add_comment (hdr, comment);
      ss->header->esps_hdr = hdr;
    }
  ss->band = sp->sigfreq / 2.0;
  ss->band_low = 0.0;
  ss->params = (caddr_t) sp;
  if (!(dpp = (char **) malloc (sizeof (char *) * ny)))
    {
      show_notice (1, "Can't malloc pointer array in spgm_32.");
      RETURN (NULL)
    }
  for (i = 0; i < ny; i++)
    {
      if (!(dpp[i] = malloc (nx)))
	{
	  show_notice (1, "Can't malloc row in spgm_32.");
	}
    }
  ss->data = (caddr_t) dpp;	/* output signal: the spectrogram */

  p = (short *) sh_mem;		/* point to shared dsp32 memory */
  *p++ = cnt = size + 1;	/* one extra sample for preemphasis on board */
  *p++ = 0;			/* 1st call to board just buffers data */
#ifdef SPGM_CENTER_ON_START
  for (i = size / 2; i--;)
    *p++ = 0;			/* Center initial window on data start */
  cnt -= size / 2;		/* by padding with zeros. */
#endif
  do
    {
      *p++ = *data++;
    }
  while (--cnt);

  dsprg (0, C_PIR);		/* let dsp know that data has been sent */

  if (!do_color)
    {				/* create a bitmap array for dithered image */
      sp->bitmap = XV_NULL;
      /* ! *//* Can't handle dithered bitmaps done on the board. */
      /*
       * if((sp->bitmap = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
       * XV_WIDTH,          nx, XV_HEIGHT,          ny, SERVER_IMAGE_DEPTH,
       * 1, 0)) == XV_NULL) show_notice(1, "Can't create a server image in
       * spgm_32.");
       */
    }
  for (i = 0; i < 10000; i++)	/* Wait for dsp32 acknowledge. */
    if (dsprg (0, C_PCR) & PCR_PIF)
      break;

/**************************************************************************/
  for (x = 0; x < nx; x += nfrm)
    {				/* main loop running dsp32 */
      /* now give him data */
      if (nx - x < nfrm)
	nfrm = nx - x;		/* This MUST be even! */
      p = (short *) sh_mem;
      *p++ = cnt = nfrm * step;
      *p++ = nfrm;
      if (nsamps)
	{			/* out of data? */
	  if ((nsamps -= cnt) < 0)
	    cnt += nsamps;
	  do
	    {
	      *p++ = *data++;
	    }
	  while (--cnt);
	  if (nsamps < 0)
	    cnt = -nsamps;
	}
      if (cnt)
	do
	  {
	    *p++ = 0;
	  }
	while (--cnt);

      dsprg (0, C_PIR);		/* let dsp know that data has been sent */

      if (sp->bitmap && x)
	{			/* blit the dithered image into a server
				 * image */
	  spblit ((Server_image) sp->bitmap, bitmap, ny, nfrm, init);
	  /* (from previous batch) */
	  init = 0;
	}
      for (i = 0; i < 100000; i++)	/* wait for computations to finish. */
	if (dsprg (0, C_PCR) & PCR_PIF)
	  break;

      /* upload results */
      {
	register short *q = (short *) sh_mem;
	int nk = nfrm / sizeof (short);

	for (i = 0; i < ny; i++)
	  {
	    p = (short *) (dpp[i] + x);
	    cnt = nk;
	    if (cnt > 0)
	      do
		{
		  *p++ = *q++;
		}
	      while (--cnt);
	  }

	if (sp->bitmap)		/* copy the dithered image into a safe place */
	  for (p = bitmap + ny, i = ny; i-- > 0;)
	    *--p = *q++;
      }
    }
  if (sp->bitmap)		/* blit the final dithered image into a
				 * server image */
    spblit ((Server_image) sp->bitmap, bitmap, ny, nfrm, -1);

  ss->type = P_CHARS | SIG_SPECTROGRAM;
  ss->file_size = ss->buff_size = nx;
#ifdef SPGM_CENTER_ON_START
  ss->start_time = sp->start_time;
#else
  ss->start_time = sp->start_time + (sp->window_size / 2.0);
#endif
  ss->freq = sig->freq / step;
  ss->end_time = sp->start_time + ((double) nx) / ss->freq;
  ss->version += 1;
  head_printf (ss->header, "version", &(ss->version));
  head_printf (ss->header, "type_code", &(ss->type));
  head_printf (ss->header, "samples", &(ss->buff_size));
  head_printf (ss->header, "frequency", &(ss->freq));
  head_printf (ss->header, "start_time", &(ss->start_time));
  head_printf (ss->header, "dimensions", &(ss->dim));
  head_ident (ss->header, "f");
  if (ss->header && ss->header->magic == ESPS_MAGIC && ss->header->esps_hdr)
    {
      *add_genhd_d ("start_time", (double *) NULL, 1, hdr) = ss->start_time;
      *add_genhd_d ("end_time", (double *) NULL, 1, hdr) = ss->end_time;
      *add_genhd_d ("record_freq", (double *) NULL, 1, hdr) = ss->freq;
    }
  ss->params = (caddr_t) sp;
  sp->sig = NULL;		/* could be freed without sp's knowledge! */

  RETURN (ss)
#endif
}

#undef RETURN

static char e_spect_tname[NAMELEN] = "";
/*************************************************************************/
clobber_temp_signal_file ()
{
  if (*e_spect_tname)
    unlink (e_spect_tname);
  *e_spect_tname = 0;
}

/*************************************************************************/
/* Do ESPS spectrogram of signal in ESPS file on disk. */
Signal *
e_spect (sp)
     Spectrogram *sp;
{
  Signal *sig = sp->sig;
  char sgram_command[500];
  char *data_window;
  int fft_order;
  double preemphasis;
  double step_size;
  double window_len;
  char spec_name[200];
  extern char output_dir[];
  char *e_spec_name;
  Signal *ss;
  Object *o;

  if (debug_level)
    fprintf (stderr, "e_spect: ESPS spectrogram of signal %s.\n", sig->name);

  switch (sp->window_type)
    {
    case 0:
      data_window = "RECT";
      break;
    case 1:
      data_window = "HAMMING";
      break;
    case 2:
      data_window = "COS4";
      break;
    case 3:
      data_window = "HANNING";
      break;
    case 4:
      data_window = "TRIANG";
      break;
    case 5:
      data_window = "NONE";
      break;
    case 6:
      data_window = "KAISER";
      break;
    case 7:
      data_window = "ARB";
      break;
    case 8:
      data_window = "SINC";
      break;
    case 9:
      data_window = "SINC_C4";
      break;
    default:
      sprintf (notice_msg, "Window type %d not recognized.  Using HAMMING.",
	       sp->window_type);
      show_notice (0, notice_msg);
      data_window = "HAMMING";
      break;
    }
  fft_order = ROUND (log ((double) sp->nfft) / log (2.0));
  preemphasis = -sp->preemp;
  step_size = 1000.0 * sp->window_step;
  window_len = 1000.0 * sp->window_size;
  sprintf (sgram_command, "%s -s%.9f:%.9f -d%s -o%d -E%g -S%g -w%g",
	   sgram_program, sp->start_time, sp->end_time,
	   data_window, fft_order, preemphasis, step_size, window_len);

  if (sp->outname && *sp->outname)
    {
      strcpy (spec_name, sp->outname);
      e_spec_name = spec_name;
    }
  else
    {
#if defined(STARDENT_3000) || defined(M5600) || defined(hpux)
      sprintf (spec_name, "%s.f", sp->signame);
#else
      sprintf (spec_name, "%s.fspec", sp->signame);
#endif

      /*
       * By using just the basename here, we can force the use of output_dir,
       * if it is specified.
       */
      if (*output_dir)
	e_spec_name = inc_esps_name (basename (spec_name));
      else
	e_spec_name = inc_esps_name (spec_name);
    }

  if ((o = (Object *) (sig->obj)))
    set_current_obj_name (o->name);

  (void) run_esps_prog (sgram_command,
			sig->name, e_spec_name, 1, TRUE,
			clobber_temp_signal_file);
  return NULL;
}

/*************************************************************************/
/* Do ESPS spectrogram of signal buffered in memory. */
Signal *
e_spgm (sp)
     Spectrogram *sp;
{
  if (sp && sp->sig)
    {
      char *template = "espstmpXXXXXX", *tname;
      int startsamp, nsamps;
      Signal *sig;
      struct header *hdr;
      char comment[300];
      short *data;
      Signal *ss = sp->sig;
      extern int ref_start;

      if (debug_level)
	fprintf (stderr, "e_spgm: ESPS spectrogram of signal %s.\n", sp->sig->name);

      if (ss->header->magic == ESPS_MAGIC)
	return (e_spect (sp));

      if ((type_of_signal (ss) == P_SHORTS) && (ss->dim == 1))
	{

	  if (!sp->sig->data
	      || !(data = ((short **) sp->sig->data)[0]))
	    {
	      show_notice (1, "NULL input data array in e_spgm.");
	      return NULL;
	    }
	  tname = mk_esps_temp (template);
	  *e_spect_tname = 0;
	  if (tname)
	    {
	      strcpy (e_spect_tname, tname);
	      setup_output_dir (e_spect_tname);
	      free (tname);
	    }
	  else
	    {
	      show_notice (1, "can't create temporary output name in e_spgm,");
	      return NULL;
	    }
	  startsamp =
	    0.5 + (sp->start_time - BUF_START_TIME (sp->sig)) * sp->sig->freq;
	  nsamps = 0.5 + (sp->end_time - sp->start_time) * sp->sig->freq;
	  sig = new_signal (e_spect_tname, SIG_NEW, dup_header (sp->sig->header),
			    (caddr_t) NULL, nsamps, sp->sig->freq, 1);
	  if (!sig)
	    {
	      show_notice (1, "e_spgm: Can't make temporary new signal.");
	      return NULL;
	    }
	  sig->start_time = sp->start_time;
	  if (ref_start > sig->start_time)
	    ref_start = sig->start_time;
	  sig->file_size = nsamps;
	  sig->end_time = sp->end_time;

	  sig->header->magic = ESPS_MAGIC;
	  hdr = sig->header->esps_hdr = new_header (FT_SD);
	  set_sd_type (hdr, SHORT);
	  hdr->hd.sd->sf = sp->sig->freq;
	  set_pvd (hdr);
	  sprintf (comment,
		"xwaves spectrogram: start_time %g end_time %g signal %s\n",
		   sp->start_time, sp->end_time, sp->sig->name);
	  add_comment (hdr, comment);
	  *add_genhd_d ("start_time", (double *) NULL, 1, hdr) = sp->start_time;
	  *add_genhd_d ("end_time", (double *) NULL, 1, hdr) = sp->end_time;

	  if (output_header (sig))
	    {
	      put_sd_recs (data + startsamp, nsamps, hdr, sig->header->strm);
	      close_sig_file (sig);

	      sp->sig = sig;
	      ss = e_spect (sp);
	    }
	  else
	    {
	      show_notice (1,
		   "e_spgm:  Can't output header of temporary new signal.");
	      close_sig_file (sig);
	      ss = NULL;
	    }
	  sig->name = NULL;
	  free_signal (sig);

	  return ss;
	}
      else
	{
	  sprintf (notice_msg,
	    "Spectrogram method for signal %s is not available.", ss->name);
	  show_notice (1, notice_msg);
	}
    }
  else
    show_notice (1, "Null signal passed to e_spgm.");

  return NULL;
}

/*************************************************************************/
void
spblit (pro, pixel_r, height, width, init)
     Server_image pro;
     unsigned short *pixel_r;
     int height, width, init;
{
  static Display *display;
  static XImage *pr = NULL;
  Pixmap pmap;
  static int xd = 0;
  static GC gc;

  /* ! *//* Not working yet. */
  if (1)
    return;

  if (init == 1)
    {
      XWindowAttributes win_attr;
      Visual *visual;
      XGCValues gc_val;

      xd = 0;

      if (!doing_print_graphic)
	display = (Display *) xv_get (daddy, XV_DISPLAY);
      else
	display = get_xp_display ();

      XGetWindowAttributes (display, (Window) xv_get (daddy, XV_XID),
			    &win_attr);
      visual = win_attr.visual;
      pr = XCreateImage (display, visual,
			 1,	/* depth */
			 XYBitmap,
			 0,
			 (char *) pixel_r,
			 16, height,
			 16, 0);
      gc = XCreateGC (display, (Pixmap) xv_get (pro, SERVER_IMAGE_PIXMAP),
		      (unsigned long) None, &gc_val);
    }
  XPutImage (display, pmap, gc, pr, 0, 0, xd, 0, width, height);
  xd += width;

  if (init == -1)
    {
      /* ! *//* Clean up */
      pr->data = NULL;
      XDestroyImage (pr);
      XFreeGC (display, gc);
    }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void
cmap_spect (canv)
     Canvas canv;
{
  register int i, j, k, l, lo_lim, hi_lim;
  int nreserved;
  double scale, a, b, interp, fac;
  static char cm_file[100] = "no map yet";
  extern char def_cm[];
  extern double image_clip, image_range;
  static u_char rb[MAX_CMAP_SIZE], gb[MAX_CMAP_SIZE], bb[MAX_CMAP_SIZE];
  static int last_lower = -1, last_upper = -1;

  nreserved = cmap_size - CMAP_RESERVED;
  fac = ((double) CMAP_RESERVED) / (MAX_CMAP_SIZE - nreserved);
  hi_lim = 0.5 + ((HI_SPECT_LIM - image_clip) * fac);
  lo_lim = hi_lim - (0.5 + (image_range * fac));

  if (!do_color)
    return;
  /*
   * The thresholds (static globals) define the grayscale ramp for the
   * spectrogram colormap.  The full colormap is divided into four regions:
   * colors [0..lower threshold]   == white colors [lower .. upper]       ==
   * gray (or color) ramp colors [upper .. reserved]    == black (or other
   * color) colors [reserved .. map size] == special use (e.g. cursor color
   * 
   * The reserved colors are assumed to be previously set in an external array
   * "rgb".
   */

  cmap (XV_NULL);		/* ensure an initialized colormap */

  if (strcmp (cm_file, def_cm))
    {
      for (i = 0; i < cmap_size; i++)
	{			/* save local copy or orig. colormap */
	  rb[i] = rgb[i].red;
	  gb[i] = rgb[i].green;
	  bb[i] = rgb[i].blue;
	}
      strcpy (cm_file, def_cm);
    }
  if (last_lower != lo_lim || last_upper != hi_lim)
    {
      last_lower = lo_lim;
      last_upper = hi_lim;

      for (i = 0; i <= lo_lim; ++i)
	{
	  rgb[i].red = rb[1];
	  rgb[i].green = gb[1];
	  rgb[i].blue = bb[1];	/* lower region */
	}

      scale = ((double) CMAP_RESERVED) / (hi_lim - lo_lim);
      for (j = 0; i < hi_lim; i++, j++)
	{
	  interp = 1.0 + ((double) j) * scale;
	  k = interp;
	  l = k + 1;
	  a = interp - (double) k;
	  b = 1.0 - a;
	  rgb[i].red = (unsigned char) (b * rb[k] + a * rb[l]);
	  rgb[i].green = (unsigned char) (b * gb[k] + a * gb[l]);
	  rgb[i].blue = (unsigned char) (b * bb[k] + a * bb[l]);
	}
      for (; i < CMAP_RESERVED; ++i)
	{
	  rgb[i].red = rb[CMAP_RESERVED - 1];
	  rgb[i].green = gb[CMAP_RESERVED - 1];		/* g; */
	  rgb[i].blue = bb[CMAP_RESERVED - 1];	/* b; *//* upper region */
	}
      for (i = 0; i < 3; i++)
	{
	  rgb[i].red = rb[i];
	  rgb[i].green = gb[i];
	  rgb[i].blue = bb[i];	/* lowest 3 are taboo also */
	}
    }
  cmap (canv);
}


/*************************************************************************/
/*
 * This assumes that the view and reticle structures have been created,
 * linked and intelligently initialized...
 */
Canvas
new_spect_window (ss)
     Signal *ss;
{
  Frame frame;
  Canvas canvas;
  Xv_Cursor cursor;

  void rescale_spect ();
  extern void doit ();
  extern int min_framewidth, cmap_depth, min_frameheight;
  int frame_width_adjust,	/* adjustment for frame border width */
    frame_height_adjust;	/* adjustment for frame border height */

  char title[256];
  static int no_spect_icon = -1;

  if (!(ss && ss->views && *ss->views->ret))
    return XV_NULL;

  scale_spect_for_canvas (ss->views);

  /* Limit min. size so some of the info. printed in border will be visible. */
  if (ss->views->width < min_framewidth)
    ss->views->width = min_framewidth;
  if (ss->views->height < min_frameheight)
    ss->views->height = min_frameheight;

  if (debug_level)
    fprintf (stderr, "Creating frame with XV_WIDTH %d,  XV_HEIGHT %d.\n",
	     ss->views->width, ss->views->height);

  if (do_color)
    frame = (Frame) xv_create (XV_NULL, FRAME,
			       XV_VISUAL, visual_ptr,
			       XV_LABEL, title,
			       FRAME_NO_CONFIRM, TRUE,
			       FRAME_INHERIT_COLORS, FALSE,
			       XV_X, next_x,
			       XV_Y, next_y,
			       XV_SHOW, FALSE,
			       XV_WIDTH, ss->views->width + 2 * FRAME_MARGIN,
			       XV_HEIGHT, ss->views->height
			       + FRAME_HEADER + FRAME_MARGIN,
			       WIN_DEPTH, cmap_depth,
			       0);
  else
    frame = (Frame) xv_create (XV_NULL, FRAME,
			       XV_LABEL, title,
			       FRAME_NO_CONFIRM, TRUE,
			       WIN_X, next_x,
			       WIN_Y, next_y,
			       WIN_SHOW, FALSE,
			       XV_WIDTH, ss->views->width + 2 * FRAME_MARGIN,
			       XV_HEIGHT, ss->views->height
			       + FRAME_HEADER + FRAME_MARGIN,
			       0);
  if (!window_check_return (frame))
    return ((Canvas) XV_NULL);

  canvas = (Canvas) xv_create (frame, CANVAS,
			       CANVAS_RETAINED, FALSE,
			       CANVAS_FIXED_IMAGE, FALSE,
			       CANVAS_AUTO_SHRINK, TRUE,
			       CANVAS_AUTO_EXPAND, TRUE,
			       OPENWIN_NO_MARGIN, TRUE,
			       XV_WIDTH, WIN_EXTEND_TO_EDGE,
			       XV_HEIGHT, WIN_EXTEND_TO_EDGE,

  /* Compatibility attribute--going away in another release. */
			       WIN_CLIENT_DATA, ss->views,
			       CANVAS_PAINTWINDOW_ATTRS,
  /* WIN_DYNAMIC_VISUAL,       TRUE,   */
			       XV_VISUAL, visual_ptr,
			       WIN_DEPTH, cmap_depth,
			       0,
			       CANVAS_CMS_REPAINT, FALSE,
			       CANVAS_NO_CLIPPING, TRUE,
			       0);

  xv_set (canvas_paint_window (canvas),
	  WIN_CONSUME_EVENTS,
	  LOC_DRAG,
	  LOC_MOVE,
	  LOC_WINEXIT,
	  WIN_IN_TRANSIT_EVENTS,
	  WIN_ASCII_EVENTS,
	  0,
	  WIN_IGNORE_EVENTS,
	  KBD_USE,
	  KBD_DONE,
	  0,
	  WIN_EVENT_PROC, doit,
	  WIN_BIT_GRAVITY, ForgetGravity,
	  0);

  if (!window_check_return (canvas))
    {
      dt_xv_destroy_safe (11, frame);
      return ((Canvas) 0);
    }
  next_x += w_x_incr;
  next_y += w_y_incr;
  if ((next_y + Max (min_frameheight, (int) xv_get (frame, XV_HEIGHT)))
      > SCREEN_HEIGHT)
    {
      next_y = 120;
      next_x = 0;
    }
  xv_set (canvas, WIN_MENU, spect_menu,
	  XV_KEY_DATA, menu_item_key,
	  find_operator (spect_get_ops (), "play between marks"),
	  0);

  window_fit (frame);

  notify_interpose_destroy_func (frame, kill_signal_view);
  xv_set (frame, WIN_CLIENT_DATA, (caddr_t) canvas,
	  0);

  ss->views->canvas = canvas;

  cmap_spect (canvas);

  /*
   * Set repaint and resize procs here instead of the original xv_create to
   * guard against premature calls within xv_create.
   */
  xv_set (canvas, CANVAS_REPAINT_PROC, repaint,
	  CANVAS_RESIZE_PROC, rescale_spect,
	  0);

  xv_set (frame, WIN_SHOW, TRUE,
	  0);

/*
 * There is a problem somewhere related to this call that causes few 
 * different problems at only a few sites.  For a customer with sunos
 * on a sparc2, waves core dumps on occasion when a spectrogram window
 * is created.  For another user on SGI, he gets a spectrogram window
 * without a title bar about 3 out of 10 times.   I can't reproduce either
 * here, but what I get on our Sparc2 with Sunos is that about 3 out of 10
 * spectrogram windows dissappear after briefly flashing on the screen.
 * Not making the exv_attach_icon() call prevents all of these problems.
 * I've attempted to locate the actual problem, using Purify, etc., but
 * nothing conclusive has turned up.   So I'm putting in this awful hack
 * so that if it isn't fixed in a future release, at least we can tell 
 * users how to avoid the more serious problem.   They'll get a window that
 * has a blank icon instead.
 * 
 * So tell them to set the environment variable NO_SPECT_ICON to anything.
*/

  if (no_spect_icon == -1)
	no_spect_icon = (getenv("NO_SPECT_ICON") && 1); 

  if (!no_spect_icon)
  	exv_attach_icon (frame, IMAGE_ICON, title, TRANSPARENT);

  return (canvas);
}


/*************************************************************************/
print_spect_x (v)
     View *v;
{
  Pixwin *pw;
  double time, freq;
  char title[100];

  if (!v || !(v->canvas) || (doing_print_graphic && print_only_plot))
    return;

  time = v->cursor_time;
  if (time < v->start_time)
    time = v->start_time;
  if (time > (freq = ET (v)))
    time = freq;
  freq = v->cursor_yval;
  if (freq < v->start_yval)
    freq = v->start_yval;
  if (freq > v->end_yval)
    freq = v->end_yval;

  sprintf (title, "Time: %8.4f   Freq: %8.2lf   ", time, freq);
  /* If format changes, fix print_spect_y to match. */
  pw = (Pixwin *) canvas_paint_window (v->canvas);

  pw_text (pw, *v->x_offset + 1, 14,
	   PIX_COLOR (FOREGROUND_COLOR) | PIX_SRC, def_font, title);
}

/*********************************************************************/
double
spect_y_to_yval (v, y)
     register View *v;
     int y;
{
  register double freq;

  freq = v->start_yval + (*(v->y_offset) - y) * *(v->y_scale) / PIX_PER_CM;
  if (freq < v->start_yval)
    return (v->start_yval);
  if (freq > v->end_yval)
    return (v->end_yval);
  return (freq);
}

/*************************************************************************/
spect_get_cursor_channel (v)
     View *v;
{
  double freq;
  int i;

  freq = v->cursor_yval;
  if (freq < v->start_yval)
    freq = v->start_yval;
  if (freq > v->end_yval)
    freq = v->end_yval;

  /*
   * ! *//* Hasty hack to get value printout working for nonlinear y scale
     * (ARB_FIXED).  There's got to be a better way.
   */
  if (v->sig->y_dat)
    freq = spect_y_to_yval (v, v->yval_to_y (v, freq));

  i = 0.5 + (v->sig->dim - 1)
    * ((freq - v->start_yval) / (v->end_yval - v->start_yval));

  return (i);
}

/*************************************************************************/
print_spect_y (v)
     View *v;
{
  Signal *sig;
  char **data;
  Canvas canv;
  Pixwin *pw;
  double time, t;
  int i, j;
  char msg[100];

  if (!v || !(sig = v->sig)
      || !(data = (char **) sig->data) || !(canv = v->canvas)
      || (doing_print_graphic && print_only_plot))
    return;

  time = v->cursor_time;
  if (time < v->start_time)
    time = v->start_time;
  if (time > (t = ET (v)))
    time = t;
  j = time_to_index (sig, time);
  if (j < 0 || j >= sig->buff_size)
    return;

  i = spect_get_cursor_channel (v);

  if (!data[i])
    return;

  sprintf (msg, "Value: %3.0f ",
	   v->val_offset[0] + (v->val_scale[0] * data[i][j]));
  pw = (Pixwin *) canvas_paint_window (v->canvas);
  /* 34 = length of title from print_spect_x */
  pw_text (pw, *v->x_offset + 34 * def_font_width, 14,
	   PIX_COLOR (FOREGROUND_COLOR) | PIX_SRC, def_font, msg);
}

/*************************************************************************/
void
rescale_spect (c, w, h)
     Canvas c;
     int w, h;
{
  View *v;

  if (debug_level)
    {
      Frame frame;
      Xv_Window pw;

      fprintf (stderr, "rescale_spect(c, %d, %d)\n", w, h);
      pw = canvas_paint_window (c);
      fprintf (stderr, "Paint window has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	       (int) xv_get (pw, XV_WIDTH), (int) xv_get (pw, XV_HEIGHT));
      fprintf (stderr, "Canvas has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	       (int) xv_get (c, XV_WIDTH), (int) xv_get (c, XV_HEIGHT));
      frame = (Frame) xv_get (c, XV_OWNER);
      fprintf (stderr, "Frame has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	   (int) xv_get (frame, XV_WIDTH), (int) xv_get (frame, XV_HEIGHT));
    }
  if ((v = (View *) xv_get (c, WIN_CLIENT_DATA)) && w != *v->x_offset)
    {
      *v->x_scale *= (double) (v->width - *v->x_offset) / (w - *v->x_offset);
      v->width = w;
      v->height = h;
    }
}

/*********************************************************************/
int
spect_plot_reticles (v)
     View *v;
{
  if (v && v->canvas && v->reticle_on[0] && v->ret && v->ret[0])
    {
      draw_reticle (v->canvas, v->ret[0]);
      return (TRUE);
    }
  return (FALSE);
}

/*************************************************************************/
scale_spect_for_canvas (v)
     View *v;
{
  Bound *b;
  Reticle *ret;
  double endf, endt, dt, df;
  Spectrogram *sp;
  int comb_height;
  extern int h_spect_rescale, v_spect_rescale;

#define EF(v) (v->y_to_yval(v, comb_height + b->top))

  if (!v || !v->sig || !(ret = *v->ret))
    return FALSE;

  b = (Bound *) reticle_get_margins (ret);
  comb_height = v->readout_height + v->scrollbar->height;
  if (!v->h_rescale)		/* Force one pixel per frame. */
    *(v->x_scale) = PIX_PER_CM / v->sig->freq;

  if (*v->v_rescale && v->height > 0)
    {
      *(v->y_scale) =
	(v->sig->band * PIX_PER_CM)
	/ (v->height - (comb_height + b->top + b->bottom));
    }
  else
    {				/* One pixel per frequency bin. */
      *(v->y_scale) = (v->sig->band * PIX_PER_CM) / (v->sig->dim - 1);

      if (v->height <= 0)
	{
	  v->height = comb_height + b->top + b->bottom
	    + ROUND ((v->sig->band_low + v->sig->band - v->start_yval)
		     * PIX_PER_CM / *(v->y_scale));
	  if (v->height > MAX_CANVAS_HEIGHT)
	    v->height = MAX_CANVAS_HEIGHT;
	}
    }

  *(v->y_offset) = v->height - b->bottom;

  ret->absc.start = v->start_time;
  ret->ordi.start = v->start_yval;

  /*
   * The top and right reticle limits are set to the minimum of the data
   * limits or the available plotting area.  This assumes that the plotting
   * function will follow the same conventions...
   */

  if ((ret->absc.end = ET (v)) > BUF_END_TIME (v->sig))
    ret->absc.end = BUF_END_TIME (v->sig);
  if (ret->absc.end < ret->absc.start)
    ret->absc.end = ret->absc.start;
  if ((ret->ordi.end = EF (v)) > v->sig->band_low + v->sig->band)
    ret->ordi.end = v->sig->band_low + v->sig->band;
  if (ret->ordi.end < ret->ordi.start)
    ret->ordi.end = ret->ordi.start;

  ret->bounds.left = *(v->x_offset);
  ret->bounds.bottom = *(v->y_offset);

  /* Set the pixel limits of the edges of the reticle. */
  /* (Somehow this seems like overspecification...) */

  ret->bounds.right = ret->bounds.left
    + ROUND ((ret->absc.end - ret->absc.start)
	     * PIX_PER_CM / *(v->x_scale));
  ret->bounds.top = ret->bounds.bottom
    - ROUND ((ret->ordi.end - ret->ordi.start)
	     * PIX_PER_CM / *(v->y_scale));

  /*
   * Set lengths and spacing of reticle subdivision tick marks. (Disable
   * reticle drawing in case of trouble.)
   */
  v->reticle_plot =
    scale_spect_view_ret (ret, v)
    ? spect_plot_reticles
    : NULL;

  return TRUE;
#undef EF
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
plot_spect_cursors (v, color)
     View *v;
     int color;
{
  Pixwin *pw;
  struct rect *rec;
  int x, y, x1, x2, y1, y2;

  if (!v || !v->canvas || (doing_print_graphic && print_only_plot))
    return (FALSE);

  x = v->time_to_x (v, v->cursor_time);
  y = v->yval_to_y (v, v->cursor_yval);

  x1 = 0;
  x2 = v->width;
  if (v->scrollbar && (v->scrollbar->height > 0) && v->scrollbar->is_on)
    y1 = v->scrollbar->y + v->scrollbar->height;
  else
    y1 = 0;
  y2 = v->height;

  pw = canvas_pixwin (v->canvas);
  pw_vector (pw, x, y1, x, y2,
	     PIX_COLOR (color) | (PIX_SRC ^ PIX_DST),
	     color);
  pw_vector (pw, x1, y, x2, y,
	     PIX_COLOR (color) | (PIX_SRC ^ PIX_DST),
	     color);
  return (TRUE);
}

/*************************************************************************/
kill_srvimage (svim)
     Server_image svim;
{
  if (svim)
    {
      dt_xv_destroy (12, svim);
    }
}

/******************************************************************/
Reticle *
new_spgm_reticle (sig)
     Signal *sig;		/* the spectrogram's SIG structure */
{
  Reticle *r;
  double hz_per_pix, sec_per_pix, lospac, hispac, range, range10, range5,
    range2;
  int right_of_d;
  char abform[20];

  if (!sig || !(r = (Reticle *) calloc (1, sizeof (Reticle))))
    return (NULL);

  r->ordinate.maj.style = MAJOR | EDGES;
  r->ordinate.maj.length = 10.0 / sig->freq;	/* 10 pixels long */
  /* Want at least 30 pixels between freq. numbering and no more than 60. */
  /* Assume total vertical pixels = sig->dim. */
  hz_per_pix = sig->band / sig->dim;
  lospac = 30.0 * hz_per_pix;
  hispac = 60.0 * hz_per_pix;
  for (range = 1.0e-6; range < 1.e6; range *= 10.0)
    {
      range10 = 10.0 * range;
      if ((lospac >= range) && (lospac < range10))
	{
	  range5 = 5.0 * range;
	  range2 = 2.0 * range;
	  if (hispac >= range10)
	    r->ordinate.maj.inter = range10;
	  else if ((lospac <= range5) && (hispac >= range5))
	    r->ordinate.maj.inter = range5;
	  else
	    r->ordinate.maj.inter = range2;
	  break;
	}
    }
  r->ordinate.maj.list = NULL;
  r->ordinate.maj.num = 0;

  /* Want at least 70 pixels between time numbering and no more than 140. */
  /* Assume one pixel per time step. */
  sec_per_pix = 2.0 / sig->freq;
  lospac = 70.0 * sec_per_pix;
  hispac = 140.0 * sec_per_pix;
  range = pow (10.0, floor (log10 (lospac)));
  range10 = 10.0 * range;
  range5 = 5.0 * range;
  range2 = 2.0 * range;
  if (hispac >= range10)
    r->abscissa.maj.inter = range10;
  else if ((lospac <= range5) && (hispac >= range5))
    r->abscissa.maj.inter = range5;
  else
    r->abscissa.maj.inter = range2;

  r->ordinate.min1.style = EDGES;
  r->ordinate.min1.length = 5.0 / sig->freq;
  r->ordinate.min1.inter = r->ordinate.maj.inter / 2.0;
  r->ordinate.min1.list = NULL;
  r->ordinate.min1.num = 0;
  r->ordinate.min2.style = EDGES;
  r->ordinate.min2.length = 3.0 / sig->freq;
  r->ordinate.min2.inter = r->ordinate.maj.inter / 10.0;
  r->ordinate.min2.list = NULL;
  r->ordinate.min2.num = 0;
  reticle_set_ord_precision (r, "%6.0f");
  r->ordinate.num_inter = r->ordinate.maj.inter;
  r->ordinate.num_loc = NUM_LB;
  r->abscissa.maj.style = MAJOR | EDGES;
  r->abscissa.maj.length = 10.0 * hz_per_pix;
  r->abscissa.maj.list = NULL;
  r->abscissa.maj.num = 0;
  r->abscissa.min1.style = EDGES;
  r->abscissa.min1.length = 5.0 * hz_per_pix;
  r->abscissa.min1.inter = r->abscissa.maj.inter / 2.0;
  r->abscissa.min1.list = NULL;
  r->abscissa.min1.num = 0;
  r->abscissa.min2.style = EDGES;
  r->abscissa.min2.length = 3.0 * hz_per_pix;
  r->abscissa.min2.inter = r->abscissa.maj.inter / 10.0;
  r->abscissa.min2.list = NULL;
  r->abscissa.min2.num = 0;
  if (r->abscissa.maj.inter <= 1.0)
    right_of_d = 1 + (int) (0.5 - log10 (r->abscissa.maj.inter));
  else
    right_of_d = 0;
  sprintf (abform, "%s%df", "%8.", right_of_d);
  reticle_set_absc_precision (r, abform);
  r->abscissa.num_inter = r->abscissa.maj.inter;
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
  return (r);
}

/*********************************************************************/
spect_xy_to_chan (v, x, y)
     View *v;
     int x, y;
{
  if (v && v->sig)
    {
      int d, i;

      d = v->sig->dim - 1;

      i = 0.5 + (d * (*v->y_offset - y) * (*v->y_scale)
		 / (PIX_PER_CM * (v->end_yval - v->start_yval)));

      if (i < 0)
	return (0);
      if (i > d)
	return (d);
      return (i);
    }
  return (0);
}

/*************************************************************************/
isa_spectrogram_view (v)
     View *v;
{
  return (v && (v->extra_type == VIEW_BITMAP) &&
	  (v->data_plot == plot_spectrogram));
}

/*********************************************************************/
/* Assumes the view is a spectrogram. */
int
nonlin_yval_to_y (v, yval)
     View *v;
     double yval;
{
  double *y_dat, r;
  int d, i, j, k;

  y_dat = v->sig->y_dat;
  d = v->sig->dim - 1;

  if (d < 1 || yval < y_dat[i = 0])
    r = 0.0;
  else if (yval >= y_dat[j = d])
    r = 1.0;
  else
    {
      while (j - i > 1)
	if (yval < y_dat[k = (i + j) / 2])
	  j = k;
	else
	  i = k;

      r = (i + (yval - y_dat[i]) / (y_dat[j] - y_dat[i])) / d;
    }

  r *= (v->end_yval - v->start_yval) * PIX_PER_CM / (*v->y_scale);
  return *v->y_offset - (int) (r + 0.5);
}

/*********************************************************************/
/* Assumes the view is a spectrogram. */
spect_yval_to_y (v, yval)
     register View *v;
     register double yval;
{
  int i;
  if (yval < v->start_yval)
    yval = v->start_yval;
  if (yval > v->end_yval)
    yval = v->end_yval;
  yval -= v->start_yval;
  i = *v->y_offset - (int) (0.5 + yval * PIX_PER_CM / (*v->y_scale));
  return i;
}

/*********************************************************************/
void
operate_spect_scrollbar (v, event)
     View *v;
     Event *event;
{
  int id = event_id (event);
  switch (id)
    {
    case MS_LEFT:
    case MS_MIDDLE:
    case MS_RIGHT:
      if (event_is_down (event))
	{
	  operate_scrollbar (id, event_x (event), v);
	}
      break;
    default:
      break;
    }
  spect_file_print_x (v, event_x (event));
}

/*************************************************************************/
View *
new_spect_view (sig, type, canvas)
     Signal *sig;
     int type;
     Canvas canvas;
{
  View *view;
  ViewBitmap *vbm;
  Rect *rec;
  Reticle *ret;
  Spectrogram *sp;
  Bound *b;
  extern int plot_hmarkers (), h_spect_rescale, v_spect_rescale;
  extern char mark_reference[], *savestring ();
  extern int spect_interp;
  Menuop *search_all_menus_but ();

  if (!(view = new_waves_view (sig, canvas)))
    return (NULL);
  view->start_time = BUF_START_TIME (sig);
  if (sig->band > 0.0)
    {
      view->start_yval = sig->band_low;
      view->end_yval = sig->band_low + sig->band;
    }
  else
    {
      view->start_yval = 0.0;
      view->end_yval = sig->freq / 2.0;
    }
  view->tmarker_chan = sig->dim - 1;
  view->bmarker_chan = 0;
  view->find_crossing = FALSE;
  view->bmarker_yval = view->start_yval;
  view->tmarker_yval = view->end_yval;
  view->v_rescale[0] = v_spect_rescale;
  view->spect_interp = spect_interp;

  view->ret[0] = ret = new_spgm_reticle (sig);
  if (ret)
    {
      ret->ordi.start = view->start_yval;
      ret->ordi.end = view->end_yval;
      ret->absc.start = view->start_time;
      ret->absc.end = BUF_END_TIME (sig);
      if (view->overlay_as_number)
	ret->color = FOREGROUND_COLOR;
      else
	ret->color = RETICLE_COLOR;
    }
  view->dims = 1;		/* unpleasant HACK, but necessary */
  view->data_plot = plot_spectrogram;
  view->cursor_plot = plot_spect_cursors;
  view->vmarker_plot = plot_markers;
  view->hmarker_plot = plot_hmarkers;
  view->reticle_plot = spect_plot_reticles;
  view->x_print = print_spect_x;
  view->y_print = print_spect_y;
  view->set_scale = scale_spect_for_canvas;
  view->h_rescale = h_spect_rescale;
  view->handle_scrollbar = operate_spect_scrollbar;
  view->left_but_proc = search_all_menus_but ("wave", def_sleft_op);
  view->mid_but_proc = search_all_menus_but ("wave", def_smiddle_op);
  view->move_proc = search_all_menus_but ("wave", def_move_op);
  view->right_but_proc = right_sbutton_proc;
  if (*mark_reference)
    view->mark_reference = savestring (mark_reference);
  else
    view->mark_reference = savestring ("cursor_time");
  view->free_extra = NULL;
  view->time_to_x = generic_time_to_x;
  view->x_to_time = generic_x_to_time;
  view->xy_to_chan = spect_xy_to_chan;
  if (sig->y_dat)
    {
      view->yval_to_y = nonlin_yval_to_y;
      view->y_to_yval = nonlin_y_to_yval;
    }
  else
    {
      view->yval_to_y = spect_yval_to_y;
      view->y_to_yval = spect_y_to_yval;
    }

  b = (Bound *) reticle_get_margins (ret);
  /*
   * If called with zero or negative canvas dimensions, these are to be
   * established:
   */
  view->width = (new_width > 0) ? new_width	/* from make command */
    : -1;			/* from buffer size */
  view->height = (new_height > 0) ? new_height : -1;
  new_height = new_width = 0;	/* kluge to clear after meth_spectrogram() */
  if (view->width <= 0)
    {
      view->width = b->left + b->right
	+ ROUND ((BUF_END_TIME (view->sig) - view->start_time)
		 * view->sig->freq);
      if (view->width > MAX_CANVAS_WIDTH)
	view->width = MAX_CANVAS_WIDTH;
    }
  *(view->x_offset) = b->left;
  if (!view->h_rescale)		/* One pixel per frame. */
    *(view->x_scale) = PIX_PER_CM / view->sig->freq;
  else				/* make buffer fit in window */
    *(view->x_scale) = (((double) view->sig->buff_size) /
			((double) (view->width - b->left - b->right))) *
      (PIX_PER_CM / view->sig->freq);


  if (!(view->extra = (caddr_t) malloc (sizeof (ViewBitmap))))
    {
      show_notice (1, "new_spect_view: cannot malloc ``extra'' space.");
      free ((char *) view);
      return (NULL);
    }
  view->extra_type = VIEW_BITMAP;
  vbm = (ViewBitmap *) view->extra;
  vbm->scale_type = type;	/* FIXED,VARIABLE */
  vbm->depth = vbm->height = vbm->width = 0;	/* currently meaningless */
  vbm->maxval = 0;		/* currently meaningless */
  view->free_extra = kill_srvimage;
  if ((sp = (Spectrogram *) (sig->params)))
    {
      vbm->bitmap = (char *) (sp->bitmap);	/* gets freed in free_view(); */
      sp->bitmap = XV_NULL;
    }
  else
    vbm->bitmap = NULL;

  view->next = view->sig->views;	/* install view in spect signal */
  view->sig->views = view;
  view->scrollbar = new_scrollbar (view);
  return (view);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
spect_file_print_x (v, x)
     register View *v;
     int x;
{
  Xv_Window pw;
  char mes[50];
  double time, sig_dur, fract;

  if (doing_print_graphic && print_only_plot)
    return TRUE;
  if (!v || !v->canvas || !v->sig)
    return FALSE;

  x -= *(v->x_offset);
  if (x < 0)
    x = 0;
  fract = ((double) x) / (v->width - *(v->x_offset));
  sig_dur = SIG_DURATION (v->sig);
  time = v->sig->start_time + (sig_dur * fract);
  pw = canvas_paint_window (v->canvas);

  sprintf (mes, "Time(f):%8.4f", time);

  pw_text (pw, *v->x_offset + 1, 14,
	   PIX_SRC | PIX_COLOR (FOREGROUND_COLOR), def_font, mes);

  return (TRUE);
}

/*************************************************************************/
void
e_move_contour (canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  unsigned char r[MAX_CMAP_SIZE], b = 0, g = 0;
  int range, i, x, y, ewidth, eheight;
  Pixwin *pw;
  View *v;
  Rect *rec;
  Xv_Cursor cursor;
  char title[256];

  if (use_static_cmap)
    {
      show_notice (0, "This operation cannot be done with a static colormap.");
      return;
    }
  v = (View *) xv_get (canvas, WIN_CLIENT_DATA);
  rec = (Rect *) xv_get (canvas, XV_RECT);
  ewidth = rec->r_width;
  eheight = rec->r_height;
  if (ewidth > 400)
    ewidth = 400;
  x = event_x (event);
  y = event_y (event);

  pw = (Pixwin *) canvas_paint_window (canvas);

  y = CMAP_RESERVED * (eheight - y)
    / (eheight - v->readout_height - v->scrollbar->height);
  if (y > CMAP_RESERVED - 1)
    y = CMAP_RESERVED - 1;
  if (y < 1)
    y = 1;			/* y=0 would clobber background colormap
				 * entry */
  range = CMAP_RESERVED * x / ewidth;
  if (range + y > CMAP_RESERVED)
    range = CMAP_RESERVED - y;
  if (range < 1)
    range = 1;
  for (i = 0; i < range; i++)
    r[i] = 255;

  {
    Xv_singlecolor colors[MAX_CMAP_SIZE];
    Cms pwcms;

    pwcms = (Cms) xv_get ((Xv_opaque)pw, WIN_CMS);

    for (i = 0; i < y; i++)
      colors[i] = rgb[i];
    colors[y].red = 255;
    colors[y].green = 0;
    colors[y].blue = 0;
    for (i = y + 1; i < y + range; i++)
      {
	colors[i].red = 255;
	colors[i].green = rgb[i].green;
	colors[i].blue = rgb[i].blue;
      }
    for (; i < cmap_size; i++)
      colors[i] = rgb[i];

    xv_set (pwcms, CMS_COLORS, colors,
	    0);
  }

  sprintf (title, "    Level:%3d         Range:%3d", y, range);

  pw_text (pw, rec->r_width / 4, 16 + v->readout_height,
	   PIX_COLOR (FOREGROUND_COLOR) | PIX_SRC, def_font, title);
}

/*************************************************************************/
void
e_modify_intensity (canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  static int curs_on = 0;
  double threshold;
  int x, y, ewidth, total_range;
  Pixwin *pw;
  Rect *rec;
  /* Frame frm; */
  Xv_Cursor cursor;
  char title[256];
  View *v;

  if (use_static_cmap)
    {
      show_notice (0, "This operation cannot be done with a static colormap.");
      return;
    }
  v = (View *) xv_get (canvas, WIN_CLIENT_DATA);
  rec = (Rect *) xv_get (canvas, WIN_RECT);
  ewidth = rec->r_width;
  if (ewidth > 400)
    ewidth = 400;

  total_range = MAX_CMAP_SIZE - (cmap_size - CMAP_RESERVED);

  if (event_button_is_down (event))
    {
      if (event_is_down (event) && !curs_on)
	{			/* button was pressed */
	  curs_on = 1;
	  threshold = HI_SPECT_LIM - image_clip - image_range;
	  /* warp cursor to location for current colormap */
	  x = (ewidth * image_range) / total_range;
	  y = (rec->r_height * threshold) / total_range;
	  xv_set (canvas, WIN_MOUSE_XY, x, y, 0);
	}
      else
	{			/* LOC_DRAG */
	  x = event_x (event);
	  y = event_y (event);
	  if (x > ewidth)
	    x = ewidth;
	  image_range = (total_range - 1) * ((double) x) / ewidth;
	  threshold = total_range * ((double) y) / rec->r_height;
	  /*
	   * if(image_range < 1) image_range = 1; if((threshold + image_range)
	   * >= total_range) threshold = total_range - image_range - 1; if
	   * (threshold < 0) threshold = 0;
	   */
	  pw = (Pixwin *) canvas_paint_window (canvas);

	  image_clip = HI_SPECT_LIM - threshold - image_range;

	  limit_z_range (total_range);

	  threshold = HI_SPECT_LIM - image_clip - image_range;

	  cmap_spect (canvas);

	  sprintf (title, "Threshold:%3.0f         Range:%3.0f", v->val_offset[0] + (v->val_scale[0] * threshold), image_range);
	  pw_text (pw, rec->r_width / 4, 16 + v->readout_height,
		   PIX_COLOR (FOREGROUND_COLOR) | PIX_SRC, def_font, title);

	  return;
	}
    }
  else
    {				/* button was released (???) */
      if (event_is_up (event) && curs_on)
	{
	  curs_on = 0;
	  /* warp cursor to location of current time,frequency */
	  x = v->time_to_x (v, v->cursor_time);
	  y = v->yval_to_y (v, v->cursor_yval);
	  xv_set (canvas, WIN_MOUSE_XY, x, y,
		  0);
	}
    }
}

/*************************************************************************/
gray_init (display, visual, size, cp, cp1, cp2, dp, prp)
     Display *display;
     Visual *visual;
     int size;
     char **cp, **cp1, **cp2, **dp;
     XImage **prp;
{
  char *c, *c1, *c2, *data;
  XImage *pr;

  c = malloc ((unsigned) size * sizeof (int));
  if (!c)
    return FALSE;

  c1 = malloc ((unsigned) size * sizeof (int));
  if (!c1)
    return FALSE;

  c2 = malloc ((unsigned) size * sizeof (int));
  if (!c2)
    return FALSE;

  data = malloc ((unsigned) size * sizeof (int));
  if (!data)
    return FALSE;

  pr = XCreateImage (display, visual,
		     cmap_depth,
		     ZPixmap,
		     0,
		     data,
		     size, 1,
		     8, 0);

  bits_per_pixel = pr->bits_per_pixel;
  if (debug_level)
    fprintf (stderr, "bits_per_pixel: %d, depth: %d\n",
	     bits_per_pixel, cmap_depth);

  *cp = c;
  *cp1 = c1;
  *cp2 = c2;
  *dp = data;
  *prp = pr;
  return TRUE;
}

/*************************************************************************/
dith_init (display, visual, size, cp, cp1, cp2, bufp, prp)
     Display *display;
     Visual *visual;
     int size;
     char **cp, **cp1, **cp2;
     char **bufp;
     XImage **prp;
{
  char *c, *c1, *c2, *data;
  long bufdim[2];
  long **buf;
  XImage *pr;
  int i, j;
  caddr_t arr_alloc ();

  c = malloc ((unsigned) size * sizeof (char));
  if (!c)
    return FALSE;

  c1 = malloc ((unsigned) size * sizeof (char));
  if (!c1)
    return FALSE;

  c2 = malloc ((unsigned) size * sizeof (char));
  if (!c2)
    return FALSE;

  bufdim[0] = 2;
  bufdim[1] = size + 2;
  buf = (long **) arr_alloc (2, bufdim, LONG, 0);
  /* ! *//* arr_alloc() may exit() on error. */

  data = malloc ((unsigned) (size + 7) / 8);
  if (!data)
    return FALSE;

  pr = XCreateImage (display, visual,
		     1,
		     XYBitmap,
		     0,
		     data,
		     size, 1,
		     8, 0);
  /* if (!pr) return FALSE; */

  if (debug_level >= 2)
    {
      switch (pr->bitmap_bit_order)
	{
	case LSBFirst:
	  fprintf (stderr, "Little-Endian machine.\n");
	  break;
	case MSBFirst:
	  fprintf (stderr, "Big-Endian machine.\n");
	  break;
	default:
	  fprintf (stderr, "Bitmap bit order %d.\n");
	}
    }
  *cp = c;
  *cp1 = c1;
  *cp2 = c2;

  *bufp = (char *) buf;
  for (i = 0; i < 2; i++)
    for (j = 0; j < size + 2; j++)
      buf[i][j] = 0;

  *prp = pr;
  /* ! *//* "Unsupported pixrect operation attempted."  ??? */
  /* pr_rop(pr, 0, 0, size, 1, PIX_SRC, (Pixrect *) NULL, 0, 0); */
  return TRUE;
}

/************************************************************************/
static int
interp (dp, bsiz, ctmp, esiz, off, siz)
     char *dp;
     int *ctmp;
     int bsiz, esiz, off, siz;
{
  int i, j, k;
  int half, beta;

  if (!dp)
    {
      show_notice (1, "interp: NULL data array.");
      return FALSE;
    }
  if ((bsiz > 1) && (esiz > 1))
    {
      bsiz -= 1;
      esiz -= 1;
      half = esiz / 2;		/* For rounding. */

      k = 0;
      j = off;
      i = (j * bsiz) / esiz + (j < esiz);
      beta = i * esiz - j * bsiz;
      for (; i <= bsiz && k < siz; i++, beta += esiz)
	for (; beta >= 0 && k < siz; j++, k++, beta -= bsiz)
	  {
	    /* beta == i*esiz - j*bsiz && j == k + off */
	    if (do_color && (bits_per_pixel > 16))
	      ctmp[k] = ((esiz - beta) * dp[i] + beta * dp[i - 1] + half) / esiz;
	    else if (do_color && (bits_per_pixel > 8))
	      ((short *) ctmp)[k] = ((esiz - beta) * dp[i] + beta * dp[i - 1] + half) / esiz;
	    else
	      ((char *) ctmp)[k] = ((esiz - beta) * dp[i] + beta * dp[i - 1] + half) / esiz;
	  }
    }
  else if (do_color && (bits_per_pixel > 16))
    {
      for (k = 0; k < siz; k++)
	ctmp[k] = dp[off];
    }
  else if (do_color && (bits_per_pixel > 8))
    {
      for (k = 0; k < siz; k++)
	((short *) ctmp)[k] = dp[off];
    }
  else
    {
      for (k = 0; k < siz; k++)
	((char *) ctmp)[k] = dp[off];
    }
  return TRUE;
}

/************************************************************************/
dither (data, lolim, hilim, bufp, pr)
     char *data;
     int lolim, hilim;
     char *bufp;
     XImage *pr;
{
  int alpha = 7, beta = 3, gamma = 5, delta = 1;	/* coefficients of
							 * error-diffusion
							 * filter */
  int coeff_bits = 4;		/* sum of coefficients == 1 <<
				 * coeff_bits */
  int rnd_adj = (alpha + beta + gamma + delta) / 2;
  long **buf = (long **) bufp;
  char *pdata = pr->data;
  int size = pr->width;
  int bit_order = pr->bitmap_bit_order;
  int cval;
  int sval;
  int err;
  int j;
  int frac_bits = 16;		/* use ints as fixed-point */
  /* with this many fraction bits */
  int unit = 1 << frac_bits;
  int half = unit / 2;
  int range = hilim - lolim;
  int half_range = range / 2;

  switch (bit_order)
    {
    case LSBFirst:
    case MSBFirst:
      break;
    default:
      show_notice (1, "dither: unknown bit order.");
      return FALSE;
    }

  for (j = 0; j < size; j++)
    {
      cval = ((char *) data)[j];
      sval = (cval < lolim) ? 0
	: (cval > hilim) ? unit
	: ((cval - lolim << frac_bits) + half_range) / range;
      buf[0][j + 1] = buf[1][j + 1] + sval;
    }

  buf[1][1] = 0;

  /* "Unsupported pixrect operation." */
  /* pr_rop(pr, 0, 0, size, 1, PIX_SRC, (Pixrect *) NULL, 0, 0); */
  for (j = 0; j <= (size - 1) / 8; j++)
    pdata[j] = 0;

  for (j = 0; j < size; j++)
    {
      err = buf[0][j + 1];
      if (err >= half)
	{
	  err -= unit;

	  switch (bit_order)
	    {
	    case LSBFirst:
	      pdata[j / 8] |= 0x01 << j % 8;
	      break;
	    case MSBFirst:
	      pdata[j / 8] |= 0x80 >> j % 8;
	      break;
	    }
	}
      buf[0][j + 2] += err * alpha + rnd_adj >> coeff_bits;
      buf[1][j + 2] = err * delta + rnd_adj >> coeff_bits;
      /* Yes, this one has '=', not '+='. */
      buf[1][j + 1] += err * gamma + rnd_adj >> coeff_bits;
      buf[1][j] += err * beta + rnd_adj >> coeff_bits;
    }

  return TRUE;
}

/*************************************************************************/
void
dith_free (c, c1, c2, buf, pr)
     char **c, **c1, **c2;
     char **buf;
     XImage **pr;
{
  if (*c)
    free (*c);
  *c = NULL;
  if (*c1)
    free (*c1);
  *c1 = NULL;
  if (*c2)
    free (*c2);
  *c2 = NULL;
  if (*buf)
    arr_free (*buf, 2, LONG, 0);
  *buf = NULL;
  if (*pr)
    XDestroyImage (*pr);
  *pr = NULL;
}

/*************************************************************************/
void
gray_free (c, c1, c2, pr)
     char **c, **c1, **c2;
     XImage **pr;
{
  if (*c)
    free (*c);
  *c = NULL;
  if (*c1)
    free (*c1);
  *c1 = NULL;
  if (*c2)
    free (*c2);
  *c2 = NULL;
  if (*pr)
    XDestroyImage (*pr);
  *pr = NULL;
}

/*********************************************************************/
scope_to_number (str)
     char *str;
{
  int scope;

  if (!strcmp (str, "view"))
    scope = SCOPE_VIEW;
  else if (!strcmp (str, "buffer"))
    scope = SCOPE_BUFFER;
  else
    {
      sprintf (notice_msg,
	       "plot_spectrogram: invalid spect_rescale_scope \"%s\".",
	       str);
      show_notice (0, notice_msg);
      return SCOPE_VIEW;
    }
  return (scope);
}

/*************************************************************************/
unsigned int *
get_mapper_array (pw)
     Xv_window pw;
{
  static unsigned int maparray[MAX_CMAP_SIZE];
  unsigned long *mapp;
  float fac;
  register int i, j, k;

  if ((mapp = (unsigned long *) xv_get (pw, WIN_X_COLOR_INDICES)))
    {
      j = cmap_size - CMAP_RESERVED;
      fac = ((double) CMAP_RESERVED) / (MAX_CMAP_SIZE - j);
      if (debug_level > 1)
	fprintf (stderr, "size:%d cmap_reserved:%d fac:%f nspecial:%d\n",
		 cmap_size, CMAP_RESERVED, fac, j);
      for (i = 0, k = MAX_CMAP_SIZE - j; i < k; i++)
	{
	  j = 0.5 + (fac * i);
	  maparray[i] = (int) mapp[j];
	}
      for (; i < MAX_CMAP_SIZE; i++)
	maparray[i] = (int) mapp[j];

      return (maparray);
    }
  else
    show_notice (1, "Problems getting colormap indices in get_mapper_array().");
  return (NULL);
}

/*************************************************************************/
plot_spectrogram (view)
     View *view;
{
  Xv_Window pw;
  Window pwin, real_pwin;
  Signal *sig;
  Canvas canvas;
  struct rect *rect;
  static Pixrect *pr = NULL;
  Server_image prb;
  Pixmap pmap;
  int i, j;
  int height, width, ioffx, ioffy;
  int expdim, expsiz;
  short **dpp;
  unsigned int *ctmp = NULL, *ctmp1 = NULL, *ctmp2 = NULL, *dtmp = NULL;
  Reticle *ret;
  ViewBitmap *vb;
  Display *display, *real_display;
  int screen, real_screen;
  GC pmgc = NULL, pwgc = NULL;
  XGCValues gc_val;
  unsigned int *xpixels;
  extern char spect_rescale_scope[];
  int scope;
  extern Frame daddy;

  if (!view
      || !(sig = view->sig)
      || (sig->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM
      || !(dpp = (short **) sig->data))
    {
      show_notice (1, "plot_spectrogram: invalid view or signal.");
      return FALSE;
    }

  scope = view->rescale_scope;

  /* init variables */

  canvas = view->canvas;
  pw = canvas_paint_window (canvas);
  ret = *(view->ret);
  rect = (struct rect *) xv_get (pw, WIN_RECT);
  width = ret->bounds.right - ret->bounds.left;
  height = 1 + ret->bounds.bottom - ret->bounds.top;
  expsiz = ROUND (BUF_DURATION (sig) * PIX_PER_CM / *view->x_scale);
  ioffx = 0.5 + (ret->absc.start - BUF_START_TIME (sig))
    / (BUF_DURATION (sig) / expsiz);
  expdim = 1 + ROUND (sig->band * PIX_PER_CM / *view->y_scale);
  ioffy = 0.5 + ((sig->band_low + sig->band - ret->ordi.end) / sig->band)
    * (expdim - 1);
  if (ioffy < 0)
    ioffy = 0;

  if (debug_level >= 2)
    {
      fprintf (stderr, "\nscope %s.\n",
	       (scope == SCOPE_VIEW) ? "VIEW"
	       : (scope == SCOPE_BUFFER) ? "BUFFER"
	       : "UNKNOWN");
      fprintf (stderr,
	 "width %d, height %d, expsiz %d, ioffx %d, expdim %d, ioffy %d.\n",
	       width, height, expsiz, ioffx, expdim, ioffy);
    }
  /* clear window */
  if (!doing_print_graphic)
    pw_write (pw, 0, 0, rect->r_width, rect->r_height,
	      PIX_COLOR (BACKGROUND_COLOR) | PIX_SRC, NULL, 0, 0);

  /* copy each freq. channel to screen */

  if (doing_print_graphic)
    {
      display = get_xp_display ();
      screen = DefaultScreen (display);
      pwin = 0;
      real_display = (Display *) xv_get (pw, XV_DISPLAY);
      real_screen = DefaultScreen (display);
      real_pwin = (Window) xv_get (pw, XV_XID);
    }
  else
    {
      real_display = display = (Display *) xv_get (pw, XV_DISPLAY);
      real_screen = screen = DefaultScreen (display);
      real_pwin = pwin = (Window) xv_get (pw, XV_XID);
    }

  if (view->spect_interp
      && view->extra_type == VIEW_BITMAP
      && (vb = (ViewBitmap *) view->extra))
    {
      int xoff;

      prb = (Server_image) vb->bitmap;

      if (prb && doing_print_graphic)
	{
	  dt_xv_destroy (13, prb);
	  vb->bitmap = NULL;
	  prb = XV_NULL;
	}
      if (debug_level)
	{
	  if (prb != XV_NULL)
	    fprintf (stderr, "%s: view bitmap exists.\n", "plot_spectrogram");
	  else
	    fprintf (stderr, "%s: view bitmap empty.\n", "plot_spectrogram");
	}
      if (debug_level >= 2)
	{
	  if (prb != XV_NULL)
	    {
	      fprintf (stderr,
		       "srv image depth %d, height %d.\n",
		       (int) xv_get (prb, SERVER_IMAGE_DEPTH),
		       (int) xv_get (prb, XV_HEIGHT));
	      fprintf (stderr,
		       "bitmap x_scale %g, view x_scale %g, diff %g.\n",
		       vb->x_scale, *view->x_scale,
		       vb->x_scale - *view->x_scale);
	      fprintf (stderr,
		       "absc start %g, bitmap start_time %g, xoff %d.\n",
		       ret->absc.start, vb->start_time,
		       ROUND ((ret->absc.start - vb->start_time) * PIX_PER_CM
			      / vb->x_scale));
	      fprintf (stderr,
		       "xoff + width = %d, srv image width %d.\n",
		       ROUND ((ret->absc.start - vb->start_time) * PIX_PER_CM
			      / vb->x_scale) + width,
		       (int) xv_get (prb, XV_WIDTH));
	    }
	}
      if (prb != XV_NULL
	  && ((int) xv_get (prb, SERVER_IMAGE_DEPTH) != (do_color ? cmap_depth : 1)
	      || (int) xv_get (prb, XV_HEIGHT) != expdim
	      || vb->x_scale != *view->x_scale
	      || 0 > (xoff = ROUND ((ret->absc.start - vb->start_time)
				    * PIX_PER_CM / vb->x_scale))
	      || xoff + width > (int) xv_get (prb, XV_WIDTH)))
	{
	  if (debug_level)
	    fprintf (stderr, "%s: bitmap obsolete.\n", "plot_spectrogram");

	  dt_xv_destroy (14, prb);
	  vb->bitmap = NULL;
	  prb = XV_NULL;
	}
      if (prb != XV_NULL)
	{
	  pmap = (Pixmap) xv_get (prb, SERVER_IMAGE_PIXMAP);
	  if (view->invert_dither)
	    {
	      gc_val.foreground = WhitePixel (display, screen);
	      gc_val.background = BlackPixel (display, screen);
	    }
	  else
	    {
	      gc_val.foreground = BlackPixel (display, screen);
	      gc_val.background = WhitePixel (display, screen);
	    }
	  pwgc = XCreateGC (display, pwin,
			    GCForeground | GCBackground, &gc_val);

	  if (debug_level >= 2)
	    fprintf (stderr, "Use existing bitmap.\n");

	  if (debug_level >= 2)
	    fprintf (stderr, "ioffx %d, x_offset %d bounds.left %d\n",
		     ioffx, *view->x_offset, ret->bounds.left);
	  if (debug_level >= 2)
	    fprintf (stderr,
		     "XCopy(xoff %d, ioffy %d, w %d, h %d, view x_offset %d, ret bds top %d)\n",
	    xoff, ioffy, width, height, *(view->x_offset), ret->bounds.top);

	  if (do_color)
	    XCopyArea (display, pmap, pwin, pwgc, xoff, ioffy,
		       width, height, *(view->x_offset), ret->bounds.top);
	  else
	    XCopyPlane (display, pmap, pwin, pwgc, xoff, ioffy,
			width, height, *(view->x_offset), ret->bounds.top,
			(unsigned long) 0x1);

	  XFreeGC (display, pwgc);
	  pwgc = NULL;
	}
      else
	{			/* prb == XV_NULL */
	  char *buf = NULL;
	  XImage *xim = NULL;
	  double lolim, hilim;
	  XWindowAttributes win_attr;
	  Visual *visual;
	  int siz;

	  if (debug_level)
	    fprintf (stderr, "%s: recomputing bitmap.\n", "plot_spectrogram");

	  XGetWindowAttributes (real_display, real_pwin, &win_attr);
	  visual = win_attr.visual;

	  siz = (scope == SCOPE_VIEW) ? width : expsiz;

	  prb = xv_create (XV_NULL, SERVER_IMAGE,
			   XV_WIDTH, siz,
			   XV_HEIGHT, expdim,
			   SERVER_IMAGE_DEPTH, do_color ? cmap_depth : 1,
			   0);

	  if (prb == XV_NULL)
	    {
	      show_notice (1, "plot_spectrogram: can't create server image.");
	      return (FALSE);
	    }
	  if (do_color && !gray_init (real_display, visual, siz,
				      &ctmp, &ctmp1, &ctmp2, &dtmp, &xim))
	    {
	      sprintf (notice_msg,
		       "%s: can't initialize temp storage to compute image.",
		       "plot_spectrogram");
	      show_notice (1, notice_msg);
	    }
	  else if (!do_color && !dith_init (real_display, visual, siz,
					 &ctmp, &ctmp1, &ctmp2, &buf, &xim))
	    {
	      sprintf (notice_msg,
		       "%s: can't initialize temp storage to dither image.",
		       "plot_spectrogram");
	      show_notice (1, notice_msg);
	    }
	  else
	    {
	      int e = expdim - 1;
	      int rnd_adj = e / 2;

	      if (sig->dim < 1)
		return FALSE;

	      if (view->invert_dither)
		{
		  gc_val.foreground = WhitePixel (display, screen);
		  gc_val.background = BlackPixel (display, screen);
		}
	      else
		{
		  gc_val.foreground = BlackPixel (display, screen);
		  gc_val.background = WhitePixel (display, screen);
		}
	      pwgc = XCreateGC (display, pwin,
				GCForeground | GCBackground, &gc_val);

	      pmap = (Pixmap) xv_get (prb, SERVER_IMAGE_PIXMAP);

	      pmgc = XCreateGC (real_display, pmap, 0, &gc_val);

	      if (debug_level)
		fprintf (stderr,
			 "%s: expansion ratios %d:%d(time), %d:%d(freq).\n",
			 "plot_spectrogram",
			 expsiz, sig->buff_size, expdim, sig->dim);

	      if (!do_color)
		{
		  get_maxmin (sig);

		  hilim = 0.0;
		  for (i = 0; i < sig->dim; i++)
		    if (hilim < sig->smax[i])
		      hilim = sig->smax[i];

		  hilim -= image_clip;
		  if (hilim > 127.0)
		    hilim = 127.0;
		  lolim = hilim - image_range;
		  if (lolim < 0.0)
		    lolim = 0.0;
		}
	      if (do_color)
		xpixels = get_mapper_array (pw);

	      if (do_color && expsiz == sig->buff_size && expdim == sig->dim)
		{		/* No interpolation
				 * needed (1:1 both
				 * ways). */
		  if (debug_level >= 2)
		    fprintf (stderr,
			 "No interpolation either way; ioffx %d, siz %d.\n",
			     ioffx, siz);
		  for (i = 0; i < expdim; i++)
		    {
		      char *dp;

		      dp = (char *) dpp[i];

		      if (!dp)
			{
			  show_notice (1, "NULL data array in plot_spectrogram.");
			  return FALSE;
			}
		      if (scope == SCOPE_VIEW)
			dp += ioffx;

		      if (do_color && (bits_per_pixel > 16))
			{
			  for (j = 0; j < siz; j++)
			    {
			      dtmp[j] = xpixels[dp[j]];
			      if (CLIENT_BYTE_ORDER != ImageByteOrder (real_display))
				dtmp[j] = byte_swap (dtmp[j]);
			    }
			}
		      else if (do_color && (bits_per_pixel > 8))
			{
			  for (j = 0; j < siz; j++)
			    {
			      ((short *) dtmp)[j] = xpixels[dp[j]];
			      if (CLIENT_BYTE_ORDER != ImageByteOrder (real_display))
				byte_swap_s (&(((short *)dtmp)[j]));
			    }
			}
		      else
			for (j = 0; j < siz; j++)
			  ((char *) dtmp)[j] = xpixels[dp[j]];

		      XPutImage (real_display, pmap, pmgc, xim,
				 0, 0, 0, expdim - i - 1, siz, 1);

		      if (i < expdim - ioffy)
			{
			  if (!doing_print_graphic)
			    XCopyArea (display, pmap, pwin, pwgc,
				       (scope == SCOPE_VIEW) ? 0 : ioffx,
				       expdim - i - 1,
				       width, 1,
				       *(view->x_offset),
				  ret->bounds.top + expdim - i - 1 - ioffy);
			  else
			    XPutImage (display, XpRootWindow (display, 0), pwgc, xim,
				       0, 0,
				       *(view->x_offset),
				   ret->bounds.top + expdim - i - 1 - ioffy,
				       width, 1);
			}
		    }
		}
	      else
		{		/* Interpolate both ways. */
		  int i1, i2, a1, a2;

		  xoff = (scope == SCOPE_VIEW) ? ioffx : 0;
		  if (debug_level >= 2)
		    fprintf (stderr,
			     "Interpolation both ways; buff size %d, expsiz %d, xoff %d, siz %d.\n",
			     sig->buff_size, expsiz, xoff, siz);

		  i2 = 0;
		  if (!interp ((char *) dpp[i2], sig->buff_size, ctmp2, expsiz,
			       xoff, siz))
		    return FALSE;

		  for (i = 0; i <= e;)
		    {
		      int lim;

		      i1 = (e == 0) ? 0 : i * (sig->dim - 1) / e;
		      if (i1 == i2)
			{
			  unsigned int *tmp = ctmp1;

			  ctmp1 = ctmp2;
			  ctmp2 = tmp;
			}
		      else if (!interp ((char *) dpp[i1],
					sig->buff_size, ctmp1, expsiz,
					xoff, siz))
			return FALSE;

		      if (e == 0 || sig->dim == 1)
			lim = e;
		      else
			{
			  lim = ((i1 + 1) * e - 1) / (sig->dim - 1);
			  if (lim > e)
			    lim = e;
			  if (lim * (sig->dim - 1) % e != 0)
			    {
			      i2 = i1 + 1;
			      if (!interp ((char *) dpp[i2],
					   sig->buff_size, ctmp2, expsiz,
					   xoff, siz))
				return FALSE;
			    }
			}

		      for (; i <= lim; i++)
			{
			  a2 = (e == 0) ? 0 : i * (sig->dim - 1) % e;

			  if (do_color && (bits_per_pixel > 16))
			    {
			      if (a2 == 0)
				{
				  for (j = 0; j < siz; j++)
				    ctmp[j] = ctmp1[j];
				}
			      else
				{
				  a1 = e - a2;
				  for (j = 0; j < siz; j++)
				    ctmp[j] =
				      (a1 * ctmp1[j] + a2 * ctmp2[j] + rnd_adj) / e;
				}
			    }
			  else if (do_color && (bits_per_pixel > 8))
			    {
			      if (a2 == 0)
				{
				  for (j = 0; j < siz; j++)
				    ((short *) ctmp)[j] = ((short *) ctmp1)[j];
				}
			      else
				{
				  a1 = e - a2;
				  for (j = 0; j < siz; j++)
				    ((short *) ctmp)[j] =
				      (a1 * ((short *) ctmp1)[j] + a2 *
				       ((short *) ctmp2)[j] + rnd_adj) / e;
				}
			    }
			  else
			    {	/* 8 bit or less color map depth */
			      if (a2 == 0)
				{
				  for (j = 0; j < siz; j++)
				    ((char *) ctmp)[j] = ((char *) ctmp1)[j];
				}
			      else
				{
				  a1 = e - a2;
				  for (j = 0; j < siz; j++)
				    ((char *) ctmp)[j] =
				      (a1 * ((char *) ctmp1)[j] + a2 * ((char *) ctmp2)[j] + rnd_adj) / e;
				}
			    }

			  if (do_color)
			    {
			      if (bits_per_pixel > 16)
				{
				  for (j = 0; j < siz; j++)
				    {
				      dtmp[j] = xpixels[ctmp[j]];
				      if (CLIENT_BYTE_ORDER !=
					  ImageByteOrder (real_display))
					dtmp[j] = byte_swap (dtmp[j]);
				    }
				}
			      else if (bits_per_pixel > 8)
				{
				  for (j = 0; j < siz; j++)
				    {
				      ((short *) dtmp)[j] =
xpixels[((short *)ctmp)[j]];
				      if (CLIENT_BYTE_ORDER !=
					  ImageByteOrder (real_display))
				          byte_swap_s (&(((short *)dtmp)[j]));
				    }
				}
			      else
				for (j = 0; j < siz; j++)
				  ((char *) dtmp)[j] = xpixels[((char *) ctmp)[j]];
			    }
			  else
			    {
			      if (!dither (ctmp, ROUND (lolim), ROUND (hilim),
					   buf, xim))
				{
				  sprintf (notice_msg, "%s: dither() failed.",
					   "plot_spectrogram");
				  show_notice (1, notice_msg);
				  return FALSE;
				}
			    }
			  XPutImage (real_display, pmap, pmgc, xim,
				     0, 0, 0, e - i, siz, 1);

			  if (i <= e - ioffy)
			    {
			      if (!doing_print_graphic)
				{
				  if (do_color)
				    XCopyArea (display, pmap, pwin, pwgc,
					       ioffx - xoff, e - i, width, 1,
					       *(view->x_offset),
					   ret->bounds.top + e - i - ioffy);
				  else
				    XCopyPlane (display, pmap, pwin, pwgc,
					      ioffx - xoff, e - i, width, 1,
						*(view->x_offset),
					    ret->bounds.top + e - i - ioffy,
						(unsigned long) 0x1);
				}
			      else
				XPutImage (display, XpRootWindow (display, 0),
					   pwgc, xim,
					   0, 0,
					   *(view->x_offset),
					ret->bounds.top + e - i - 1 - ioffy,
					   width, 1);
			    }
			}
		    }
		}

	      vb->bitmap = (char *) prb;
	      vb->x_scale = *view->x_scale;
	      vb->start_time = (scope == SCOPE_VIEW) ? ret->absc.start
		: BUF_START_TIME (sig);

	      if (debug_level >= 2)
		fprintf (stderr,
			 "New bitmap x_scale %g, start_time %g.\n",
			 vb->x_scale, vb->start_time);
	    }

	  if (do_color)
	    gray_free (&ctmp, &ctmp1, &ctmp2, &xim);
	  else
	    dith_free (&ctmp, &ctmp1, &ctmp2, &buf, &xim);

	  if (pwgc)
	    XFreeGC (display, pwgc);
	  pwgc = NULL;

	  if (pmgc)
	    XFreeGC (real_display, pmgc);
	  pmgc = NULL;

	  if (prb && !vb->bitmap)
	    {
	      dt_xv_destroy (15, prb);
	      prb = XV_NULL;
	      return FALSE;
	    }
	}

      return TRUE;
    }
  /* end (spect_interp && view->extra_type == ...) */
  else if (!view->spect_interp)
    {
      typedef int triple[3];	/* {pixel coord, size, data index} for
				 * rectangle */

      static triple *rowind = NULL, *colind = NULL;
      static int rowalloc = 0, colalloc = 0;
      int numrows, numcols;
      int e, s;
      int top = ret->bounds.top;
      int size;
      int i, j, h, w, d, k, x, y;
      char *dp;

      if (rowalloc < (size = expdim - ioffy))
	{
	  if (rowind)
	    rowind = (triple *) realloc ((char *) rowind, size * sizeof (triple));
	  else
	    rowind = (triple *) malloc (size * sizeof (triple));

	  if (rowind)
	    rowalloc = size;
	  else
	    {
	      sprintf (notice_msg, "plot_spectrogram: Allocation failed.");
	      show_notice (1, notice_msg);
	      rowalloc = 0;
	      return FALSE;
	    }
	}
      if (debug_level)
	fprintf (stderr, "%s: expansion ratio %d:%d(freq).\n",
		 "plot_spectrogram", expdim, sig->dim);

      s = sig->dim - 1;
      e = expdim - 1;

      if (s < e)
	{			/* vertical expansion */
	  j = (e - 1) / 2;
	  i = j / s;
	  h = i + 1;
	  y = top + e - ioffy - i;
	  for (k = 0; y >= top; k++)
	    {
	      rowind[k][0] = y;
	      rowind[k][1] = h;
	      rowind[k][2] = k;
	      j += e;
	      h = j / s - i;
	      i += h;
	      y -= h;
	    }
	  if (y + h > top)
	    {
	      rowind[k][0] = top;
	      rowind[k][1] = y + h - top;
	      rowind[k][2] = k;
	      k++;
	    }
	  numrows = k;
	}
      else
	{			/* vertical compression */
	  j = e / 2;
	  y = top + e - ioffy;
	  for (k = 0; y >= top; k++)
	    {
	      rowind[k][0] = y;
	      rowind[k][1] = 1;
	      rowind[k][2] = j / e;
	      j += s;
	      y--;
	    }
	  numrows = k;
	}

      if (colalloc < (size = width))
	{
	  if (colind)
	    colind = (triple *) realloc ((char *) colind, size * sizeof (triple));
	  else
	    colind = (triple *) malloc (size * sizeof (triple));

	  if (colind)
	    colalloc = size;
	  else
	    {
	      show_notice (1, "plot_spectrogram: Allocation failed.");
	      colalloc = 0;
	      return FALSE;
	    }
	}
      if (debug_level)
	fprintf (stderr, "%s: expansion ratio %d:%d(time).\n",
		 "plot_spectrogram", expsiz, sig->buff_size);

      s = sig->buff_size;
      e = expsiz;

      if (s < e)
	{			/* horizontal expansion */
	  d = (ioffx * s + e / 2) / e;
	  j = s + d * e + (e - 1) / 2;
	  i = j / s;
	  w = i - ioffx;
	  x = *view->x_offset;
	  for (k = 0; i <= ioffx + width; k++)
	    {
	      colind[k][0] = x;
	      colind[k][1] = w;
	      colind[k][2] = d + k;
	      x += w;
	      j += e;
	      w = j / s - i;
	      i += w;
	    }
	  if (i - w < ioffx + width)
	    {
	      colind[k][0] = x;
	      colind[k][1] = w - i + ioffx + width;
	      colind[k][2] = d + k;
	      k++;
	    }
	  numcols = k;
	}
      else
	{			/* horizontal compression */
	  x = *view->x_offset;
	  j = ioffx * s + e / 2;
	  for (k = 0; k < width; k++)
	    {
	      colind[k][0] = x;
	      colind[k][1] = 1;
	      colind[k][2] = j / e;
	      j += s;
	      x++;
	    }
	  numcols = k;
	}

      if (do_color)
	{
	  pwgc = XCreateGC (display, pwin, (unsigned long) None, &gc_val);
	  xpixels = get_mapper_array (pw);
	  for (i = 0; i < numrows; i++)
	    {
	      dp = (char *) dpp[rowind[i][2]];
	      if (!dp)
		{
		  show_notice (1, "NULL data array in plot_spectrogram.");
		  return FALSE;
		}
	      y = rowind[i][0];
	      h = rowind[i][1];
	      for (j = 0; j < numcols; j++)
		{
		  XSetForeground (display, pwgc, xpixels[dp[colind[j][2]]]);
		  XFillRectangle (display, pwin, pwgc,
				  colind[j][0], y, colind[j][1], h);
		}
	    }

	  return TRUE;
	}
    }
  return FALSE;
}

/*********************************************************************/
Menu
make_spect_menu ()
{
  extern Menu make_window_menu ();
  Menu menu;

  aux_sbut_ops = (do_color) ? &def_aux_sbut_ops : &def_aux_ops_bws;

  menu = make_window_menu ("spect", &right_sbut_menu,
			   aux_sbut_ops, aux_sbut_ops);

  return menu;
}

/*************************************************************************/
setup_dsp (prog, ld_stat)
     char *prog;
     int ld_stat;
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(STARDENT_3000) || defined(hpux) || defined(LINUX_OR_MAC) || defined(M5600) || defined(IBM_RS6000) || defined(SG) || defined(OS5) || defined(DEC_ALPHA)
  return;
#else

#define	DA_RUN		1
#define	DA_END		2

  if (!da_done)
    dsprg (0, C_CLRSIG);	/* clear pending interrupt */
  da_done = TRUE;

  if (dsp_is_open < 0)
    {
      dsp_is_open = dspopn ((char *) 0, 0);
      sh_mem = dspmap ();
    }
  dsprg (0, C_WRITE | C_CSR, 0x6a);
  dsprg (0, C_WRITE | C_CSR, 0xea);	/* remove reset; set mem mode bit;
					 * disable interrupts */

  /* Halt all three dsp32's */
  dsprg (0, C_STOP);
  dsprg (1, C_STOP);
  dsprg (2, C_STOP);
  dsprg (0, C_WRITE | C_PCR, 0x18);
  dsprg (1, C_WRITE | C_PCR, 0x18);
  dsprg (2, C_WRITE | C_PCR, 0x18);

  if ((ld_stat == ALWAYS_LOAD) || strcmp (loaded, prog))
    {

      if (dspld (0, prog) < 0)
	{
	  return (FALSE);
	}
      (void) strcpy (loaded, prog);
    }
  else
    dsprg (0, C_WRITE | C_PCR, 0x1a);	/* simply halt the dsp32 */
  return (TRUE);
#endif
}


/****************************************************************************/
/****************************************************************************/
/**************************** NEW 32C VME ***********************************/
/****************************************************************************/
/****************************************************************************/
#if defined(DS3100) || defined(APOLLO_68K) || defined(STARDENT_3000) || defined(hpux) ||  defined(LINUX_OR_MAC) || defined(M5600) || defined(IBM_RS6000) || defined(SG) || defined(OS5) || defined(DEC_ALPHA)
#else

#include <sys/ioctl.h>
#include <dsp32c.h>
#include <vme32c.h>

#define	PARAM_WAIT_CODE	((short int) 2)
#define	DATA_WAIT_CODE	((short int) 3)
#define	DSP_INIT_CODE	((short) 0)
#define	DSP_START_CODE	((short) 1)
#define DSP_CHIP 0
extern int dsp32c[];
extern char *DSP_SharedMemory, *DSP_io;

#endif

/*************************************************************************/
DSP_LoadAndGo (chip, program)
     int chip;
     char *program;
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(STARDENT_3000) || defined(hpux) || defined(SONY) || defined(M5600) || defined(IBM_RS6000) || defined(SG) || defined(OS5) || defined(DEC_ALPHA) || defined(LINUX_OR_MAC)
  return;
#else
  char dsname[200];
  sprintf (dsname, "%s%d", global_dc_device (""), chip);
  if ((dsp32c[chip] < 0) && ((dsp32c[chip] = dcopen (dsname)) < 0))
    {
      sprintf (notice_msg, "Couldn't open %s.", dsname);
      show_notice (1, notice_msg);
      return (-1);
    }
  if (ioctl (dsp32c[chip], DC_RESET, (char *) NULL) != 0)
    {
      perror ("Couldn't reset the board in DSP_LoadAndGo()\n");
      exit (5);
    }
  dcinit (dsp32c[chip], 0);

  dcld (dsp32c[chip], program);
  dcrun (dsp32c[chip]);
  if (dsp_wait (chip, 1000, DSP_START_CODE) >= 1000)
    {
      show_notice (1, "Load and go failed by timeout.");
    }
  return (dsp32c[chip]);
#endif
}


#define RETURN(x) {DSP_UNLOCK; return (x);}

/*************************************************************************/
Signal *
spgm_32C (sp)
     Spectrogram *sp;
{

#if defined(DS3100) || defined(APOLLO_68K) || defined(STARDENT_3000) || defined(hpux) || defined(SONY) || defined(M5600) || defined(IBM_RS6000) || defined(SG) || defined(OS5) || defined(DEC_ALPHA) || defined(LINUX_OR_MAC)
  return;
#else

  int size, step, nfrm, nsamps, nx, ny, x, init = 1, *framep;
  register int i, cnt, SM_Offset, *ip;
  register short *p, *q, *data;
  static float fwind[1024];
  struct mem_dpr
    {
      int nfft, win, step, ni;
      float pre, qc[5], qbw[5];
      int fnx, fny, fni, fnh;
      float fh[100];
      float window[512];
    }
   *dp;
  char **dpp;
  Signal *ss;
  struct header *hdr;
  double sf;
  char comment[300];
  char name[200], *cp, *full_path ();
  static short bitmap[2048];
  Signal *sig;

  void spblit ();

  static int INBUF_CHARS = INBUF_CHARS_32C;
  static int INBUF_SHORTS = INBUF_SHORTS_32C;
  char *dsp_bin = NULL;

  if (!sp || !(sig = sp->sig))
    {
      show_notice (1, "Bad argument to spgm_32C.");
      return NULL;
    }
  switch (DSP_LOCK (dsp32_wait))
    {
    case LCKFIL_OK:
      break;
    case LCKFIL_INUSE:
      show_notice (1, "DSP board in use.");
      return NULL;
      break;
    default:
    case LCKFIL_ERR:
      show_notice (1,
		"Error in trying to secure exclusive access to DSP board.");
      return NULL;
      break;
    }

  if (use_spect_prog && spect_prog)
    dsp_bin = FIND_SURF_BIN (NULL, spect_prog);
  if ((use_spect_prog && DSP_LoadAndGo (DSP_CHIP, dsp_bin) < 0) ||
      DSP_LoadAndGo (DSP_CHIP, cp = FIND_SURF_BIN (NULL, "dspgm")) < 0)
    {
      sprintf (notice_msg, "Couldn't load DSPC32 program %s or default %s.",
	       dsp_bin, cp);
      show_notice (1, notice_msg);
      RETURN (NULL)
    }
  if (dsp_bin)
    free (dsp_bin);
  if (cp)
    free (cp);

  if (!open_32c_sm (global_dc_device ("mem")))
    {				/* this is a null-op if it's
				 * already open */
      show_notice (1, "Problems opening DSP shared memory in spgm_32C()\n");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  dp = (struct mem_dpr *) DSP_SharedMemory;
  framep = (int *) DSP_SharedMemory;
  dp->nfft = sp->nfft;		/* stuff spectrogram parameters to board */
  dp->win = size = (0.5 + (sig->freq * sp->window_size));
  if (size > INBUF_INTS_32C)
    {
      sprintf (notice_msg, "%s (%d)\n%s (%d).",
	       "Requested window size for spectrogram",
	       size,
	       "exceeds available buffer size",
	       INBUF_SHORTS);
      show_notice (1, notice_msg);
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  dp->step = step = (0.5 + (sig->freq * sp->window_step));
  if ((size < 4) || !step)
    {
      sprintf (notice_msg,
	       "Unreasonable step (%d) or window (%d) size in spgm_32C.",
	       step, size);
      show_notice (1, notice_msg);
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  dp->ni = sp->yinterp;
  dp->qc[0] = sp->q_thresh;
  dp->qc[1] = sp->agc_ratio;
  dp->qc[2] = sp->q_step;
  dp->qc[3] = sp->var_ratio;
  dp->qbw[0] = sp->q_thresh + 60;
  dp->qbw[1] = sp->agc_ratio;
  dp->qbw[2] = sp->q_step * 15;
  dp->qbw[3] = sp->var_ratio;
  dp->pre = sp->preemp;
  dp->fnx = sp->xdith;
  dp->fny = sp->ydith;
  if (!do_color)
    dp->fni = sp->yinterp;	/* Also used as a flag to skip dithering */
  else
    dp->fni = 0;		/* when computing color/grayscale
				 * spectrograms. */
  dp->fnh = sp->xdith * sp->ydith;

  for (i = 0; i < dp->fnh; i++)
    dp->fh[i] = sp->dimp[i];
  if (!get_float_window (fwind, size, sp->window_type))
    {
      show_notice (1, "Can't get_float_window() in spgm_32C.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  for (i = 0, cnt = (size + 1) / 2; i < cnt; i++)
    dp->window[i] = fwind[i];

  if (dsp_wait (DSP_CHIP, 1000, PARAM_WAIT_CODE) >= 1000)
    {
      show_notice (1, "Failed to load dsp32 params.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  /* nsamps is total number of samples to be processed */
  nsamps = (0.5 + ((sp->end_time - sp->start_time) * sig->freq));
  ny = (sp->yinterp * sp->nfft) / 2;	/* number of pixels per spectrum */
  /* number of time-axis pixels forced to be modulo sizeof(int) */
  nx = ((int) (1 + ((nsamps - size) / step)) / sizeof (int)) * sizeof (int);
  if (nx <= 0)
    {
      show_notice (1, "Spectrogram too short to be computed in spgm_32C.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  /*
   * get input data address.  Note that this assumes all input data in
   * memory!
   */
  if (!sig->data
      || !(data = ((short **) sig->data)[0]))
    {
      show_notice (1, "NULL signal data in spgm_32C.");
      RETURN (NULL)
    }
  else
    data += time_to_index (sig, sp->start_time);

  /* nfrm will be the number of frames computed with each call to the dsp32 */
  /* Need space for dithered version? (need 1 short/row for returned bitmap) */
  nfrm = 2 * (INBUF_CHARS - ((do_color) ? 0 : ny)) / ny;	/* Limit by output buf
								 * size */
  nfrm &= ~1;			/* force an even number */
  if ((step * nfrm) > (INBUF_INTS_32C - size))	/* or is it limited by input
						 * buf? */
    nfrm = ((INBUF_INTS_32C - size) / step) & ~1;	/* force even number */
  if (!nfrm)
    {
      show_notice (1,
		   "Specified spectrogram parameters exceed available dsp32C buffer space.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  if (!do_color && (nfrm > 16))
    nfrm = 16;			/* output limited by shortint size */

  /* Now prepare the output signal structure. */
  if (sp->outname && *sp->outname)
    strcpy (name, sp->outname);
  else
    sprintf (name, "%s.spgm", sp->signame);	/* this SHOULD be made unique */

  if (!(ss = new_signal (name, SIG_NEW,
			 dup_header (sig->header), NULL, nx,
			 1.0 / sp->window_step, ny)))
    {
      show_notice (1, "Can't make new_signal() in spgm_32C.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  if (ss->header && ss->header->magic == ESPS_MAGIC && ss->header->esps_hdr)
    {
      hdr = new_header (FT_FEA);
      sf = sp->sigfreq;
      init_feaspec_hd (hdr, FALSE, SPFMT_SYM_EDGE, SPTYP_DB,
		       TRUE, (long) ny, SPFRM_FIXED, (float *) NULL,
		       sf, (long) ROUND (sp->window_size * sf), BYTE);
      set_pvd (hdr);
      sprintf (comment,
	       "xwaves spectrogram: start_time %g end_time %g signal %s\n",
	       sp->start_time, sp->end_time, sig->name);
      add_comment (hdr, comment);
      ss->header->esps_hdr = hdr;
    }
  ss->band = sp->sigfreq / 2.0;
  ss->band_low = 0.0;
  ss->params = (caddr_t) sp;
  if (!(dpp = (char **) malloc (sizeof (char *) * ny)))
    {
      show_notice (1, "Can't malloc pointer array in spgm_32C.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  for (i = 0; i < ny; i++)
    {
      if (!(dpp[i] = malloc (nx)))
	{
	  show_notice (1, "Can't malloc row in spgm_32C.");
	}
    }
  ss->data = (caddr_t) dpp;	/* output signal: the spectrogram */

  ip = (int *) (DSP_SharedMemory + 2 * sizeof (int));	/* point to shared
							 * memory */
  framep[0] = cnt = size + 1;	/* one extra sample for preemphasis on board */
  framep[1] = 0;		/* 1st call to board just buffers data */
#ifdef SPGM_CENTER_ON_START
  for (i = size / 2; i--;)
    *ip++ = 0;			/* Center initial window on data start */
  cnt -= size / 2;		/* by padding with zeros. */
#endif
  do
    {
      *ip++ = *data++;
    }
  while (--cnt);

  /* Sent data to the DSP32; now wait for the DSP32 to acknowledge. */
  if (dsp_wait (DSP_CHIP, 100000, DATA_WAIT_CODE) >= 100000)
    {
      show_notice (1, "DSP32C failed to respond to data transfer.");
      close_io_mem ();
      close_dsp32cs ();
      RETURN (NULL);
    }
  if (!do_color)
    {				/* create a bitmap array for dithered image */
      sp->bitmap = XV_NULL;
      show_notice (0, stderr, "Can't create a memory pixrect in spgm_32C.");
      /* ! *//* Can't handle dithered bitmaps done on the board. */
      /*
       * if((sp->bitmap = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
       * XV_WIDTH,           nx, XV_HEIGHT,          ny, SERVER_IMAGE_DEPTH,
       * 1, 0)) == XV_NULL) fprintf(stderr,"Can't create a server image in
       * spgm_32C.");
       */
    }
/***************************************************************************/
  for (x = 0; x < nx; x += nfrm)
    {				/* main loop running dsp32c */
      /* now give him data */
      if (nx - x < nfrm)
	nfrm = nx - x;		/* This MUST be even! */
      ip = (int *) (DSP_SharedMemory + 2 * sizeof (int));
      framep[0] = cnt = nfrm * step;
      framep[1] = nfrm;
      if (nsamps)
	{			/* out of data? */
	  if ((nsamps -= cnt) < 0)
	    cnt += nsamps;
	  do
	    {
	      *ip++ = *data++;
	    }
	  while (--cnt);
	  if (nsamps < 0)
	    cnt = -nsamps;
	}
      if (cnt)
	do
	  {
	    *ip++ = 0;
	  }
	while (--cnt);

      /* Send data to the DSP32, and wait for the DSP32 to acknowledge. */
      if (dsp_wait (DSP_CHIP, 1000000, DATA_WAIT_CODE) >= 1000000)
	{
	  show_notice (1, "DSP32C failed to respond to data transfer.");
	  close_io_mem ();
	  close_dsp32cs ();
	  RETURN (NULL);
	}
      if (sp->bitmap && x)
	{			/* blit the dithered image into a pixrect */
	  spblit (sp->bitmap, bitmap, ny, nfrm, init);	/* (from previous batch) */
	  init = 0;
	}
      /* upload results */
      {
	register char *q = ((char *) DSP_SharedMemory) + 3, *cp;
	register int nk = 8, nl = 4, j, l;

	for (i = 0, l = nl; i < ny; i++)
	  for (j = nfrm, cp = dpp[i] + x; j--;)
	    {
	      *cp++ = *q--;
	      if (!--l)
		{
		  q += nk;
		  l = nl;
		}
	    }

	if (sp->bitmap)
	  {			/* copy the dithered image into a safe place */
	    register short *qp = (short *) (((int) q) & 0xfffffffffffffffe);
	    for (p = bitmap + ny, i = ny; i-- > 0;)
	      *--p = *qp++;
	  }
      }
    }
  if (sp->bitmap)		/* blit the final dithered image into a
				 * pixrect */
    spblit (sp->bitmap, bitmap, ny, nfrm, init);


  ss->type = P_CHARS | SIG_SPECTROGRAM;
  ss->file_size = ss->buff_size = nx;
#ifdef SPGM_CENTER_ON_START
  ss->start_time = sp->start_time;
#else
  ss->start_time = sp->start_time + (sp->window_size / 2.0);
#endif
  ss->freq = sig->freq / step;
  ss->end_time = sp->start_time + ((double) nx) / ss->freq;
  ss->version += 1;
  head_printf (ss->header, "version", &(ss->version));
  head_printf (ss->header, "type_code", &(ss->type));
  head_printf (ss->header, "samples", &(ss->buff_size));
  head_printf (ss->header, "frequency", &(ss->freq));
  head_printf (ss->header, "start_time", &(ss->start_time));
  head_printf (ss->header, "dimensions", &(ss->dim));
  head_ident (ss->header, "f");
  ss->params = (caddr_t) sp;
  sp->sig = NULL;		/* could be freed without sp's knowledge! */

  close_io_mem ();
  close_dsp32cs ();
  RETURN (ss);
#endif
}

#undef RETURN

/* Here ends code specific to the dual-DSP32C-VME board. */
