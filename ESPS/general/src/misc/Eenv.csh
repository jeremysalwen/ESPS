# @(#)Eenv.csh	1.3 9/20/91 ESI
# Set ESPS environment variables (csh); a copy of this can be 
# "sourced" in your .login or .cshrc

# Harmless if ESPS_BASE already set:

setenv ESPS_BASE `get_esps_base`

#set this to 0 to eliminate all those "helpful" messages!
setenv ESPS_VERBOSE 2

#Although Common processing is enabled by default (for backward
#compatibility), many users find it advisable to disable it.  This
#makes ESPS usage somewhat less error-prone.  Also, many programs will
#run faster if Common is disabled. 

setenv USE_ESPS_COMMON off

setenv ESPSCOM $HOME/.espscom
setenv FIELD_ORDER off
setenv ESPS_EDR off
setenv ESPS_TEMP_PATH /usr/tmp

# BUNDLE is a MASSCOMP specific variable
#setenv BUNDLE ?????  

# DSPDEV is a DSP32 specific variable
#setenv DSPDEV ????

#ELM_HOST is a network dependent variable
#setenv ELM_HOST ????

#WAVES_HOST should only be set if you want to send display commands
#to an xwaves+ display server on another host
#setenv WAVES_HOST ?????

#WAVES_PORT should only be set if the xwaves+ display server will 
#be listening on a non-default port
#setenv WAVES_PORT ?????

#DEF_HEADER can be set to a default ESPS header for headerless files
#Note that, if you do this, ESPS programs will no longer report 
#"non-ESPS file" and exit, since they will stick the header in front 
#of any non-ESPS input file
#setenv DEF_HEADER \$ESPS_BASE/lib/waves/files/def_head.feasd

setenv ESPS_MENU_PATH .:\$ESPS_BASE/lib/menus
setenv ESPS_PARAMS_PATH  .:\$ESPS_BASE/lib/params
setenv ESPS_FILTERS_PATH  .:\$ESPS_BASE/lib/filters
setenv ESPS_LIB_PATH \$ESPS_BASE/lib
setenv ESPS_INPUT_PATH  .:\$ESPS_BASE/lib/files
setenv WAVES_INPUT_PATH  .:\$ESPS_BASE/lib/waves/files
setenv WAVES_LIB_PATH  \$ESPS_BASE/lib/waves
setenv WAVES_MENU_PATH  .:\$ESPS_BASE/lib/waves/menus
setenv WAVES_COMMAND_PATH  .:\$ESPS_BASE/lib/waves/commands
setenv WAVES_COLORMAP_PATH  .:\$ESPS_BASE/lib/waves/colormaps
setenv WAVES_PROFILE_PATH  "$HOME":\$ESPS_BASE/lib/waves

#define BBOX_QUIT_BUTTON to force a "QUIT" button in every button 
#panel created via exv_bbox (3-ESPS).  This includes button panels
#created by mbuttons (1-ESPS), fbuttons (1-ESPS) and the xwaves+ 
#command make_panel
#
#setenv BBOX_QUIT_BUTTON 1

#define CODEC16 if the AT&T DSP32 ("Fab 2") board is installed
#and you want the codec chip to run at 16,000 samples per second
#rather than 12,000
#setenv CODEC16 1

#On Sun systems that have the AT&T ("Surfboard") DSP32C installed 
#with an Ariel ProPort option for analog I/O, defining ARIEL_16
#indicates to the software that a 16 Mhz crystal is installed
#(rather than the standard 24 MHz. crystal)
#setenv ARIEL_16 1

#this variable is used by the Ariel S-32C card and ProPort support programs
setenv DSPSPEC DEVICE=/dev/s32c0,PATH=$ESPS_BASE/s32cbin

#ESPS programs must be on your path.  Here's one way to do it:
#setenv PATH=$ESPS_BASE/bin:$PATH

#On Suns, the XView shared library must be on your library path.  
#Here's one way to do it:
#setenv LD_LIBRARY_PATH=$ESPS_BASE/lib:$LD_LIBRARY_PATH

