# @(#)params.t	1.4 8/20/91 ERL
# parameter file for testing acf.c
# read by acf.t

#  Framing and Windowing Params
string  sd_field_name   =       "sd";
string  units		=       "samples";
float	start		=	1;
float	nan		= 	1024.0;
float	frame_len	=	512.0;
float	step		=	128.0;
float   preemphasis     =       0.97;
#
string	window_type 	=       "HAMMING";

#  Feature Flags and Parameters 
int	sd_flag		=	1;
string	sd_fname	=	"sd";

int	pwr_flag	=	1;
string	pwr_fname	=	"power";

int	zc_flag		=	1;
string	zc_fname	=	"zero_crossing";

int	ac_flag		=	1;
string	ac_fname	=	"auto_corr";
int 	ac_order	=	20;

int	rc_flag		=	1;
string	rc_fname	=	"refcof";

int	lpc_flag	= 	1;
string	lpc_fname	=	"lpc_coeffs";

int 	lpccep_flag	=	1;
string	lpccep_fname	=	"lpc_cepstrum";
int	lpccep_order	=	20;
string	lpccep_deriv	=	"0:5,15:19";

int	lar_flag	=	1;
string	lar_fname	=	"log_area_ratio";

int	lsf_flag	=	1;
string	lsf_fname	=	"line_spec_freq";
float	lsf_freq_res	=	9.0;

int	fftcep_flag	=	1;
string	fftcep_fname	=	"fft_cepstrum";
int	fftcep_order	=	6;
string	fftcep_deriv	=	"0:7,15:19";

int	fft_flag	=	1;
int	fft_order	=	6;
float   warp_param      =       0.0;

