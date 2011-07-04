/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
   This module defines character arrays that contain strings
   associated with the different possible values of header and
   parameter values.
*/

#ifndef lint
 static char *sccs_id = "@(#)methods.c	1.26	8/1/89 ESI";
#endif

#include <stdio.h>

char *equip_codes[] = {"NONE", "EF12M", "AD12F", "DSC", "LPA11", 
			"AD12FA", "AD12", "AD16M", "LOCALad1","LOCALad2",
			"RTI-732",NULL};
char *quant_methods[] = {"NONE", "LPC10", "MULAW", "LPC10PA", "LPC10PB", 
		       "ROSA", "AVGROSA", "ROSPA", "ROSPB", "ROSPWRA",
		       "ROSPWRB", "ROSB", "JND", "LPC10PC", "ROS1", 
		       "ROS1B", "ROS2", "ROS1C", "ROS1D", "ROS1E",
			"ROS2B", "ROS2C", "VQ", "SPAREqm1", "SPAREqm2",
			"SPAREqm3", NULL};
char *win_type_codes[] = {"NONE", "HAMMING", NULL};
char *coh_est_methods[] = {"NONE", "STR2_COH", "AUTO_COH", NULL};
char *spec_ana_methods[] = {"NONE", "AUTO_SAM", "BURG_SAM", "COV_SAM", 
			    "MBURG_SAM", "STRCOV_SAM", "VBURG_SAM",NULL};
char *pitch_methods[] = {"NONE", "C_PDM", NULL};
char *synt_methods[] = {"NONE", "PSYNCH", NULL};
char *excit_methods[] = {"NONE", "WHITE", "IMPULSE", NULL};
char *synt_inter_methods[] = {"NONE", "PULSE", "SAMPLE", NULL};
char *synt_ref_methods[] = {"NONE", "ANA", "SINX", NULL};
char *synt_pwr_codes[] = {"NONE", "RAWPULSE", "LPCPULSE", NULL};
char *coh_data_codes[] = {"NONE", "RAW", "RESIDUAL", "BOTH", NULL};
char *frame_methods[] = {"FM_NONE", "FM_FIXED", "FM_VARIABLE", NULL};
char *freq_format_codes[] = {"NONE", "SYM_CTR", "SYM_EDGE", "ASYM_CEN", 
			     "ASYM_EDGE", "ARB_VAR", "ARB_FIXED", NULL};
char *spec_type_codes[] = {"NONE", "ST_PWR", "ST_DB", "ST_REAL", 
			   "ST_CPLX",  NULL};
char *spec_an_methods[] = {"NONE", NULL};
char *post_proc_codes[] = {"NONE", NULL};
char *ros_tf_method[] = {"NONE", "HAM1", NULL};
char *filt_func_spec[] = {"NONE", "BAND", "POINT","IIR",NULL};
char *scbk_distortion[] = {"NONE", "SQUARED_ERROR",NULL};
char *scbk_codebook_type[] = {"NONE", "RC_CBK", "RC_UNVD_CBK", "RC_VCD_CBK", 
	"PP_CBK", "PD_CBK","FL_CBK","EP_CBK","PPR_CBK","PDD_CBK",
	"LAR_VCD_CBK", "LAR_UNVCD_CBK", "LAR_CBK", NULL};
char *filt_type[] = {"NONE", "FILT_LP", "FILT_HP", "FILT_BP", "FILT_BS",
	"FILT_ARB", NULL};
char *filt_method[] = {"NONE", "PZ_PLACE", "PARKS_MC", "WMSE", 
	"BUTTERWORTH", "BESSEL", "CHEBYSHEV1", "CHEBYSHEV2",
	"ELLIPTICAL",NULL};
char *type_codes[] = {"NONE", "DOUBLE", "FLOAT", "LONG", "SHORT",
	"CHAR", "UNDEF", "CODED", "BYTE", "EFILE", "AFILE",
        "DOUBLE_CPLX", "FLOAT_CPLX", "LONG_CPLX", "SHORT_CPLX", "BYTE_CPLX",
	NULL};



int
idx_ok(code,array)
int code;
char *array[];
{
   int i=0;
   if(code < 0) return(0);
   while (i<=code) 
	if(array[i++] == NULL) return(0);
   return(1);
}
