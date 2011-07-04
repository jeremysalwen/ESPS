/* @(#)xacf.h	1.1 2/25/91 ERL */
/* include file for xacf.c */

#define PROG "xacf"

#define PARAM_OK      0
#define PARAM_MAX     1		/* parameter was greater than maximum */
#define PARAM_MIN     2		/* parameter was less than minimum */
#define PARAM_CHOICE  3		/* illegal discrete choice */
#define PARAM_FORMAT  4		/* parameter string value has bad format */

Attr_attribute EXVK_FIELD_NAME;

Xv_opaque 
  main_window,      /* main window */
  helpframe,        /* frame with help buttons */
  man,              /* acf man button */
  help,             /* xacf help  button */
  defaults,         /* set defaults button */
  quit,
  frameparams,      /* frame for frame params */
  acfopts,          /* frame for acf options */
  frameparamsmsg,   /* title for parameter frame */
  acfmsg,           /* title for acf frame */

  /* frame parameters */
  sdfieldname,      /* text panel: input field anme  */
  window,           /* choice panel: data window */
  units,            /* choice panel: units */
  framelen,         /* text panel: framelength */
  preemp,           /* text panel: preemphasis parameter */
  start,            /* text panel: start param */
  step,             /* text panel: step param */
  nan,              /* text panel: nan param */
  sd_flag,
  sd_fname,

  /* feature parameters */
  pwr_flag,         /* choice panel: compute power */
  pwr_fname,        /* text panel: power field name */
  zc_flag,
  zc_fname,
  ac_flag,
  ac_fname,
  ac_order,
  rc_flag,
  rc_fname,
  rc_order,
  lpc_flag,
  lpc_fname,
  lpc_order,
  lar_flag,
  lar_fname,
  lar_order,
  lsf_flag,
  lsf_fname,
  lsf_order,
  lsf_freq_res,
  lpccep_flag,
  lpccep_fname,
  lpccep_order,
  lpccep_deriv,
  lpccep_warp,

  fftcep_flag,
  fftcep_fname,
  fftcep_order,
  fftcep_deriv,
  fftcep_warp,
  
  fft_flag,
  fft_order,
  fft_fname;

Cms e_cms;

