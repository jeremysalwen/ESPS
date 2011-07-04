/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:  Rodney Johnson, John Shore ERL
 *
 * globals.c
 * definition and initialization of globals used by waves, etc.
 */

static char *sccs_id = "@(#)globals.c	1.33	10/30/96	ATT/ERL";

#include <Objects.h>
#include <esps/esps.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif

char	*full_path();

/*these should be changed to non-external defs*/
extern int  use_dsp32;    /*def is in setrate.c and other independent progs*/
extern int  debug_level;  /*def is in waves.c and other main progs*/
extern int da_location;		/* def is in play_data.c */
int da_stop_pos_view = 1; /* enables scrolling view to interrupted D/A position */
int show_play_pos = 1; /* enables showing cursor playback position diring D/a */
double play_time = 0.0;	/* tracks current playback time in seconds during D/A */
double plot_max = 0;	       /* forced max and min for every trace */
double  plot_min = 0;

int append_extensions = 1; /* Add type-specific extensions to edited files? */
int s_range_offset = 1;		/* match waves convention (0 to n-1) to ESPS */
int c_range_offset = 0;	/* (for sample number and channel number, respectively) */
int reticle_grid = 1;	  /* 1: internal grid marks in reticles; 0: none */
int show_processes = 1;		/* enable external-process monitoring GUI */
int show_error_gui = 1;		/* enable error messages in GUI box */
int major_lines = 0;	  /* 1: draw lines across plot on reticle; 0: none */
int do_central_marks = 1;  /* 1: draw central marks on spectrograms; 0: no */
int spect_height = 0;
int spect_width = 0;
int sig_max_override = 0; /* deliberate override of sig-specific maxmin for D/A scale */
/* added for FBI */
int spect_interp = 1;	  /* 1: interpolate spectrograms; 0: "rectangles" */
int h_spect_rescale = 0;  /* 1: rescale spectrograms horizontally; 0: don't */
int v_spect_rescale = 0;  /* 1: rescale spectrograms vertically; 0: don't */
char spect_rescale_scope[50] = "view"; /* on resize, rescale view or buffer? */

int shorten_header = 0;  /* put only the signal's basename in window frame if true */
int dsp_type;		/* type of DSP board */
int P8574_type=0;	/* 1 = P8574, 0 = P8574A */
int ARIEL_16=0;		/* 16 Mhz crystal on Ariel/surf card, else 24Mhz */
int ARIEL_HK=0;		/* Ariel Pro Port on Heurikon vme board */
int socket_port=DEFAULT_PORT;	/* socket for server mode */
int hold_audio_port=0;	/* for sparc only, if non-zero, then hold audio port 
	        open once it has been opened.  This eliminates the clicking. */
int old_sphere_format=0; /* in older sphere files, the "sample_count" field
			    means total samples, NOT samples per channel (or
			    records).  If this is non-zero, then the old
			    format is used.  If zero, then current rule is
			    used, which is that sample_count means samples
			    per channel (like esps).  For example, the
			    1992 Credit Card CD is old format, but the sample
			    files in the current NIST source directory are
			    in current format */
int play_buff_factor = 0;   /* if non-zero then factor to increase play
			       buffer size */

/* some Ariel boards have the wrong type of P8574.  They should be P8574,
   some are P8574A.  This changes the serial address involved with
   communication between the Surfboard and the Ariel card.    */

extern double   image_clip, image_range;
				/* used by dither() in spect.c */

int 		dsp32_wait = 5;	/* how long to try for board access-- */
				/* used by setup_dsp() in spect.c, */

int   min_frameheight = 100;	/* spgm height - in xspect.c and spect.c */
int   min_framewidth  = 350;   /* spgm width - in xspect.c and spect.c */

/* THE FOLLOWING GLOBALS MAY BE SET FROM <a profile file> */

int  command_step = 0 ; /* 0->no step; 1->step through commands */

char inputname[MAXPATHLEN] = "",	/* name of signal input file */
     outputname[MAXPATHLEN] = "",	/* name of signal output file */
     objectname[100] = "",	/* user-defined object name */
     commandname[MAXPATHLEN] = "",	/* command name */
     init_file[MAXPATHLEN] = "$ESPS_BASE/lib/waves/commands/xw_init.WC",
 					/* control script for initialization */
     overlayname[MAXPATHLEN] = "",     /* file to be overlaid on a spectrogram */
     funcname[MAXPATHLEN] = "",	/* linkage to external process */
     active_ids[1000] = "",	/* names and numbers of channels to */
     active_numbers[1000] = "",	/* be displayed (all, if these are blank) */
     wb_spect_params[MAXPATHLEN] = "wb_params",
     nb_spect_params[MAXPATHLEN] = "nb_params",
     def_cm[MAXPATHLEN] = "$ESPS_BASE/lib/waves/colormaps/TImap",
     def_left_op[50] = "up/down move",
     def_blowup_op[50] = "xmarks",
     def_move_op[50] = "",
     def_middle_op[50] = "play between marks",
     def_smiddle_op[50] = "modify intensity",
     def_sleft_op[50] = "up/down move",
     mark_reference[50] = "cursor_time",
     time_range_prefix[MAXPATHLEN] = "-r",
     samp_range_prefix[MAXPATHLEN] = "-r",
     attachments[MES_BUF_SIZE] = "xspectrum xlabel",
				/* attachments to make panel buttons for */
     ens_print_atts[MES_BUF_SIZE] = "xspectrum xlabel",
				/* attachments that support ensemble printing */
     f0_plot_specs[50] = "",	/* plotting style, char., and offsets for F0 */
     sgram_program[MAXPATHLEN] = "sgram", /* name of spectrogram program to call 
				     if the dsp32 board isn't present 
				     (i.e., if use_dsp32==0)*/   
     remote_path[MAXPATHLEN] = "",     
                                  /*if defined, prefixed to basename of all 
				    file names in external calls,
				    unless overriden by remote_input_path
				    or remote_output_path; useful
				    for NFS access and rsh*/

     remote_input_path[MAXPATHLEN] = "",  
                                    /*if defined, prefixed to basename of all 
				    output file names in external calls; useful
				    for NFS access and rsh*/

     remote_output_path[MAXPATHLEN] = "",
                                    /*if defined, prefixed to basename of all 
				    output file names in external calls; useful
				    for NFS access and rsh*/
     temp_path[MAXPATHLEN] = ".",     /*directory path to place where esps temp
				  files can be put*/

     output_dir[MAXPATHLEN] = "",  /*directory for all output files; if 
				     strlen(output_dir) == 0, output files
				     go in current directory of same
				     directory as input file */

     play_program[MAXPATHLEN] = ""; /*name of D/A program to call when DSP32
				   board not there (use_dsp32==0)*/

/* globals for Xprinter control */
int print_graphic_printer = 1; /* 1 for printer, 0 for file */
int print_graphic_resolution = 300;
char print_graphic_orientation[25]="Portrait"; /* or Landscape */
char print_graphic_type[25] = "PostScript";    /* or PCL */
char print_graphic_file[MAXPATHLEN] = "waves.prt";
char print_graphic_command[MAXPATHLEN] = "lpr";
double print_graphic_scale = 1.0;
int print_only_plot = 0;
int print_ps_level = 2;  /* level 1 PS results in BW spectrograms */


/* file containing a header for headerless files */

extern char default_header[];	/* in copheader.c */

/*flag indicating that single channel, non-complex, FEA_SD files
should be read in as shorts and treated as traditional waves SD*/

extern int fea_sd_special; /*in header.c*/

int    write_common = 0;  /* should moving markers write ESPS common?*/

int scroll_ganged = 1,
    zoom_ganged = 0,
    edit_ganged = 0,
    line_type = 1,		/* for plotting waveforms  */
    ctlwin_x = 0,
    ctlwin_y = 0,
    ctlwin_iconized = 0,
    options = REPAINT_ON_RELEASE | DONT_SAVE_SPGM | SAVE_AFTER_EDIT;

/* These break out the semantics of (and replace) the "options" variable. */
int dont_save_sgrams = TRUE,
  overlay_as_number = FALSE,
  redraw_on_release = TRUE,
  rewrite_after_edit = TRUE;

/* some display creation globals */
double ref_size = 4.0;		/* size for def. waveform disp.*/
double ref_step = 3.0;		/* amount to step in file */
double ref_start = 0.0;		/* where reference window display begins */
double zoom_ratio = .5;
double cross_level = 0.0;	/* level for crossing-cursor positioning */
int def_w_height = 300,		/* dim. of waveform window */
    def_w_width = 1100,
    scrollbar_height = 20,
    readout_bar_height = 20;	/* XView only */
extern int    max_cmap_size; /* in xcolormap.c */

int invert_dither = 0;		/* dithered spectrograms in inverse video?
				   XView only (xspect.c) */
/* window position parameters */
int  next_x = 10,		/* upper left-hand corner of new window */
     next_y = 120;
int  w_x_incr=10,		/* increment for next window */
     w_y_incr=20;

/* variables used by xf0_methods.c */
double  f0_canvas_use = .75, f0_range = 300, f0_min = 60;

int  w_verbose = 1; /*used mostly for feedback from command parsing*/
int  show_labels = 1, show_vals = 1;
int  show_current_chan = 0;
int find_crossing = 0;		/* enable/disable level-crossing cursors */
int max_buff_bytes = 2000000;	/* limit on size of signal data buffers */

int use_static_cmap = 0;
int stop_play_error = 1;    /* if 1 stop play on error, if 0
				   then print message and continue */
				/* 1 is traditional waves action */

  /* END OF GLOBALS SET FROM CONFIGURATION FILE */

Selector
  g88 = {"play_buff_factor","%d", (char *)&play_buff_factor, NULL},
  g87 = {"stop_play_error","%d", (char *)&stop_play_error, &g88},
  g86 = {"old_sphere_format", "%d", (char *)&old_sphere_format, &g87},
  g85 = {"hold_audio_port", "%d", (char *)&hold_audio_port, &g86},
  g84 = {"print_ps_level", "%d", (char *) &print_ps_level, &g85},
  g83 = {"print_only_plot", "%d", (char *) &print_only_plot, &g84},
  g82 = {"print_graphic_scale", "%lf", (char *) &print_graphic_scale, &g83},
  g81 = {"print_graphic_command", "#strq", print_graphic_command, &g82},
  g80 = {"print_graphic_file", "#qstr", print_graphic_file, &g81},
  g79 = {"print_graphic_type", "#qstr", print_graphic_type, &g80},
  g78 = {"print_graphic_orientation", "#qstr", print_graphic_orientation,&g79},
  g77 = {"print_graphic_resolution", "%d", (char *)&print_graphic_resolution, &g78},
  g76 = {"print_graphic_printer", "%d", (char *)&print_graphic_printer, &g77},
  g75d = {"samp_range_prefix", "#strq", samp_range_prefix, &g76},
  g75c = {"time_range_prefix", "#strq", time_range_prefix, &g75d},
  g75b = {"attachments", "#strq", attachments, &g75c},
  g75a = {"ens_print_atts", "#strq", ens_print_atts, &g75b},
  g75 = {"shorten_header", "%d", (char *) &shorten_header, &g75a},
  g74 = {"plot_max", "%lf", (char *) &plot_max, &g75},
  g73b = {"plot_min", "%lf", (char *) &plot_min, &g74},
  g73 = {"sig_max_override", "%d", (char *) &sig_max_override, &g73b},
  g62 = {"spect_rescale_scope", "#qstr", spect_rescale_scope, &g73},
  g61 = {"socket_port", "%d", (char *) &socket_port, &g62},
  g60a = {"v_spect_rescale", "%d", (char *) &v_spect_rescale, &g61},
  g60 = {"h_spect_rescale", "%d", (char *) &h_spect_rescale, &g60a},
  g59 = {"spect_interp", "%d", (char *) &spect_interp, &g60},
  g58b = {"do_central_marks", "%d", (char *) &do_central_marks, &g59},
  g58a = {"major_lines", "%d", (char *) &major_lines, &g58b},
  g58 = {"reticle_grid", "%d", (char *) &reticle_grid, &g58a},
  g57d = {"show_error_gui", "%d", (char *) &show_error_gui, &g58},
  g57c = {"show_processes", "%d", (char *) &show_processes, &g57d},
  g57b = {"ARIEL_HK", "%d", (char *) &ARIEL_HK, &g57c},
  g57a = {"P8574_type", "%d", (char *) &P8574_type, &g57b},
  g57 = {"ARIEL_16", "%d", (char *) &ARIEL_16, &g57a},
  g56 = {"max_buff_size", "%d", (char *) &max_buff_bytes, &g57},
  g55 = {"min_spec_height", "%d", (char *) &min_frameheight, &g56},
  g54 = {"min_spec_width", "%d", (char *) &min_framewidth, &g55},
  g53 = {"write_common", "%d", (char *) &write_common, &g54},
  g52 = {"fea_sd_special", "%d", (char *) &fea_sd_special, &g53},
  g51 = {"temp_path", "#qstr", temp_path, &g52},
  g51b = {"mark_reference", "#qstr", mark_reference, &g51},
  g50 = {"dsp32_wait", "%d", (char *) &dsp32_wait, &g51b},
  g49 = {"command_step", "%d", (char *) &command_step, &g50},
  g48 = {"image_clip", "%lf", (char *) &image_clip, &g49},
  g47 = {"image_range", "%lf", (char *) &image_range, &g48},
  g46a = {"find_crossing", "%d", (char *) &find_crossing, &g47},
  g46 = {"show_labels", "%d", (char *) &show_labels, &g46a},
  g45a = {"show_current_chan", "%d", (char *) &show_current_chan, &g46},
  g45 = {"show_vals", "%d", (char *) &show_vals, &g45a},
  g44b = {"append_extensions", "%d", (char*)&append_extensions, &g45},
  g44a = {"remote_input_path", "#qstr", remote_output_path, &g44b},
  g44 = {"remote_output_path", "#qstr", remote_input_path, &g44a},
  g43 = {"remote_path", "#qstr", remote_path, &g44},
  g42 = {"output_dir", "#qstr", output_dir, &g43},
  g41 = {"play_prog", "#strq", play_program, &g42},
  g40 = {"sgram_prog", "#strq", sgram_program, &g41},
  g39d = {"play_time", "%lf", (char *) &play_time, &g40},
  g39c = {"show_play_position", "%d", (char *) &show_play_pos, &g39d},
  g39b = {"da_stop_pos_view", "%d", (char *) &da_stop_pos_view, &g39c},
  g39a = {"use_internal_audio", "%d", (char *) &use_dsp32, &g39b},
  g39 = {"use_dsp32", "%d", (char *) &use_dsp32, &g39a},
  g38 = {"f0_plot_specs", "#strq", f0_plot_specs, &g39},
  g37 = {"objectname", "#qstr", objectname, &g38},
  g36a = {"line_type", "%d", (char*)&line_type, &g37},
  g36 = {"max_cmap_size", "%d", (char*)&max_cmap_size, &g36a},
  g35a = {"readout_bar_height", "%d", (char *) &readout_bar_height, &g36},
  g35 = {"scrollbar_height", "%d", (char *) &scrollbar_height, &g35a},
  g34 = {"scroll_ganged", "%d", (char*)&scroll_ganged, &g35},
  g33 = {"zoom_ganged", "%d", (char*)&zoom_ganged, &g34},
  g32 = {"edit_ganged", "%d", (char*)&edit_ganged, &g33},
  g31a = {"ctlwin_iconized", "%d", (char*)&ctlwin_iconized, &g32},
  g31 = {"ctlwin_x", "%d", (char*)&ctlwin_x, &g31a},
  g30 = {"ctlwin_y", "%d", (char*)&ctlwin_y, &g31},
  g29 = {"f0_range", "%lf", (char*)&f0_range, &g30},
  g28 = {"f0_min", "%lf", (char*)&f0_min, &g29},
  g27 = {"f0_size", "%lf", (char*)&f0_canvas_use, &g28},
  g26c = {"move_op", "#strq", def_move_op, &g27},
  g26b = {"spec_left_op", "#strq", def_sleft_op, &g26c},
  g26a= {"spec_middle_op", "#strq", def_smiddle_op, &g26b},
  g26 = {"blowup_op", "#strq", def_blowup_op, &g26a},
  g25b = {"left_op", "#strq", def_left_op, &g26},
  g25 = {"middle_op", "#strq", def_middle_op, &g25b},
  g24f = {"dont_save_sgrams", "%d", (char*)&dont_save_sgrams, &g25},
  g24e = {"invert_dither", "%d", (char*)&invert_dither, &g24f},
  g24d = {"overlay_as_number", "%d", (char*)&overlay_as_number, &g24e},
  g24c = {"redraw_on_release", "%d", (char*)&redraw_on_release, &g24d},
  g24b = {"rewrite_after_edit", "%d", (char*)&rewrite_after_edit, &g24c},
  g23 = {"def_header", "#qstr", default_header, &g24b},
  g22 = {"do_color", "%d", (char*)&do_color, &g23},
  g21b = {"s_range_offset", "%d", (char*)&s_range_offset, &g22},
  g20b = {"c_range_offset", "%d", (char*)&c_range_offset, &g21b},
  g21 = {"y_increment", "%d", (char*)&w_y_incr, &g20b},
  g20 = {"x_increment", "%d", (char*)&w_x_incr, &g21},
  g19 = {"wave_height", "%d", (char*)&def_w_height, &g20},
  g18 = {"wave_width", "%d", (char*)&def_w_width, &g19},
  g17 = {"first_x", "%d", (char*)&next_x, &g18},
  g16 = {"first_y", "%d", (char*)&next_y, &g17},
  g15 = {"nb_spect_params","#qstr", nb_spect_params, &g16},
  g14 = {"wb_spect_params","#qstr", wb_spect_params, &g15},
  g13 = {"inputname", "#qstr", inputname, &g14},
  g12b = {"outputname", "#qstr", outputname, &g13},
  g12 = {"commandname", "#qstr", commandname, &g12b},
  g11b = {"init_file", "#qstr", init_file, &g12},
  g10b = {"funcname", "#qstr", funcname, &g11b},
  g10 = {"overlayname", "#qstr", overlayname, &g10b},
  g9 = {"colormap", "#qstr", def_cm, &g10},
  g7 = {"ref_size", "%lf", (char*)&ref_size, &g9},
  g6 = {"ref_step", "%lf", (char*)&ref_step, &g7},
  g5 = {"disp_size", "%d", NULL, &g6},
  g4 = {"ref_start", "%lf", (char*)&ref_start, &g5},
  g3a = {"zoom_ratio", "%lf", (char*)&zoom_ratio, &g4},
  g3 = {"cross_level", "%lf", (char*)&cross_level, &g3a},
  g1a = {"debug_level", "%d", (char *) &debug_level, &g3},
  g1b = {"verbose", "%d", (char *) &w_verbose, &g1a},
  g1 = {"da_location", "%d", (char*)&da_location, &g1b},
  gm1 = {"options", "%lx", (char*)&options, &g1};


/*************************************************************************/

char *wave_pro = ".wave_pro"; /* reassigned if main program called with -w */

/*************************************************************************/
get_globals()
{
  get_general_globals(wave_pro, &gm1);
}

/*************************************************************************/
Selector *get_waves_selector_list()
{
  return(&g1);
}

/*********************************************************************/
Selector *build_full_list(l1, l2)
     Selector *l1, *l2;
{
  Selector *l3;
  if((l3 = l1)) {
    while(l1->next) l1 = l1->next;
    l1->next = l2;
    return(l3);
  }
  return(l2);
}

/*************************************************************************/
get_all_globals(prof,sl)
     char *prof;
     Selector *sl;
{
  get_general_globals(prof,build_full_list(sl,get_waves_selector_list()));
}

/*************************************************************************/
get_general_globals(name, sl)
     char *name;
     Selector *sl;
{
  char home[MAXPATHLEN], in[MES_BUF_SIZE];
  FILE *fd=NULL, *fopen();
  char *w_pro_name;
  
  if (w_pro_name = FIND_WAVES_PROFILE(NULL,name))
    if(!(fd = fopen(w_pro_name, "r"))) {
     fprintf(stderr,"Couldn't read the profile %s; using compiled-in setup\n",
	w_pro_name);
     return;
    }

  if(fd) {
    reset_changed_items();
    while(fgets(in,500,fd))
      if((*in != '#') &&
	 (*in != '\n'))	  /* ignore empty lines and comments */
	get_args_raw(in,sl);
    if (w_verbose)
	fprintf(stderr,"Read profile file %s\n", w_pro_name);
    fclose(fd);

  }  else {
    if(debug_level)
      fprintf(stderr,
	"get_general_globals: Couldn't find a valid profile file (%s).\n",
	      w_pro_name);
  }
}

/*******************************************************************/
output_variables_to_file(fp, sl)
     FILE *fp;
     Selector *sl;
{
  if(fp && sl) {
    Selector *gp = get_waves_selector_list();
    char  *cpp = (char*)malloc(MES_BUF_SIZE);
    int nstr = MES_BUF_SIZE;
    if(cpp) {
      while(sl && (sl != gp)) {
	if(arg_to_string(&cpp, &nstr, sl))
	  fprintf(fp,"%s",cpp);
	sl = sl->next;
      }
      free(cpp);
    } else
      fprintf(stderr,"Allocation problems in output_variables_to_file\n");
  }
}

/*******************************************************************/
/* This is designed for use by attachments that have some local variables
   to be saved.  THe presumption is that the attachment does NOT also
   want to save the "globals" inherited trom xwaves.  So, the output
   stops if the beginning of the xwaves Selector list is encountered.*/
dump_local_variables(name, sl)
     char *name;
     Selector *sl;
{
  if(sl) {
    FILE *fp = stdout;

    if(name && *name) {
      if(!(fp = fopen(name, "w"))) {
	sprintf(notice_msg,"Can't open %s for output in dump_local_variables",
		name);
        show_notice(1,notice_msg);
	return(FALSE);
      }
    }
    output_variables_to_file(fp, sl);
    if(fp != stdout)
      fclose(fp);
  } else
    fprintf(stderr, "Bogus args to dump_local_variables\n");
}
