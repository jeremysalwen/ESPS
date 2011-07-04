# @(#)Eenv.sh	1.2 9/20/91 ESI
# Set ESPS environment variables (sh); copy this to your home
# directory and insert ". Eenv.sh" in your .login or .profile

# Harmless if ESPS_BASE already set:
ESPS_BASE=`get_esps_base`
export ESPS_BASE 

#set this to 0 to eliminate all those "helpful" messages!
ESPS_VERBOSE=2
export ESPS_VERBOSE

#Although Common processing is enabled by default (for backward
#compatibility), many users find it advisable to disable it.  This
#makes ESPS usage somewhat less error-prone.  Also, many programs will
#run faster if Common is disabled. 

USE_ESPS_COMMON=off
export USE_ESPS_COMMON

ESPSCOM=$HOME/.espscom
export ESPSCOM
FIELD_ORDER=off
export FIELD_ORDER
ESPS_EDR=off
export ESPS_EDR
ESPS_TEMP_PATH=/usr/tmp
export ESPS_TEMP_PATH

#BUNDLE is a MASSCOMP specific variable
#BUNDLE=?????  
#export BUNDLE

#ELM_HOST is a network dependent variable
#ELM_HOST ????
#export ELM_HOST

#WAVES_HOST should only be set if you want to send display commands
#to an xwaves+ display server on another host
#WAVES_HOST=?????
#export WAVES_HOST

#WAVES_PORT should only be set if the xwaves+ display server will 
#be listening on a non-default port
#WAVES_PORT=?????
#export WAVES_PORT

#DEF_HEADER can be set to a default ESPS header for headerless files
#Note that, if you do this, ESPS programs will no longer report 
#"non-ESPS file" and exit, since they will stick the header in front 
#of any non-ESPS input file
#DEF_HEADER=\$ESPS_BASE/lib/waves/files/def_head.feasd
#export DEF_HEADER

ESPS_MENU_PATH=.:\$ESPS_BASE/lib/menus
export ESPS_MENU_PATH
ESPS_PARAMS_PATH=.:\$ESPS_BASE/lib/params
export ESPS_PARAMS_PATH
ESPS_FILTERS_PATH=.:\$ESPS_BASE/lib/filters
export ESPS_FILTERS_PATH
ESPS_LIB_PATH=\$ESPS_BASE/lib
export ESPS_LIB_PATH
ESPS_INPUT_PATH=.:\$ESPS_BASE/lib/files
export ESPS_INPUT_PATH
WAVES_INPUT_PATH=.:\$ESPS_BASE/lib/waves/files
export WAVES_INPUT_PATH
WAVES_LIB_PATH=\$ESPS_BASE/lib/waves
export WAVES_LIB_PATH
WAVES_MENU_PATH=.:\$ESPS_BASE/lib/waves/menus
export WAVES_MENU_PATH
WAVES_COMMAND_PATH=.:\$ESPS_BASE/lib/waves/commands
export WAVES_COMMAND_PATH
WAVES_COLORMAP_PATH=.:\$ESPS_BASE/lib/waves/colormaps
export WAVES_COLORMAP_PATH
WAVES_PROFILE_PATH="$HOME":\$ESPS_BASE/lib/waves
export WAVES_PROFILE_PATH

#define BBOX_QUIT_BUTTON to force a "QUIT" button in every button 
#panel created via exv_bbox (3-ESPS).  This includes button panels
#created by mbuttons (1-ESPS), fbuttons (1-ESPS) and the xwaves+ 
#command make_panel
#
#BBOX_QUIT_BUTTON=1
#export BBOX_QUIT_BUTTON

#define CODEC16 if the AT&T DSP32 ("Fab 2") board is installed
#and you want the codec chip to run at 16,000 samples per second
#rather than 12,000
#CODEC16=1
#export CODEC16

#On Sun systems that have the AT&T ("Surfboard") DSP32C installed 
#with an Ariel ProPort option for analog I/O, defining ARIEL_16
#indicates to the software that a 16 Mhz crystal is installed
#(rather than the standard 24 MHz. crystal)
#ARIEL_16=1
#export ARIEL_16

#this variable is used by the Ariel S-32C card and ProPort support programs
DSPSPEC="DEVICE=/dev/s32c0,PATH=$ESPS_BASE/s32cbin"
export DSPSPEC

#ESPS programs must be on your path.  Here's one way to do it:
#PATH=$ESPS_BASE/bin:$PATH
#export PATH

#On Suns, the XView shared library must be on your library path.  
#Here's one way to do it:
#LD_LIBRARY_PATH=$ESPS_BASE/lib:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH

