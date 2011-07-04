#! /bin/sh
# @(#)rundemo.sh	1.5 1/18/93 ERL
# top level script for plot3d demo

# check syntax

case $# in
1)
        # any output files go in directory specified by $1
	EDEMO_TEMP=$1
	;;
*)
	EDEMO_TEMP=$HOME/Edemo$$
	echo "no temporary directory specified, will use $EDEMO_TEMP"
	;;
esac

# create temp directory if necessary, check that it is writable

if test ! -d $EDEMO_TEMP
then
  mkdir $EDEMO_TEMP 2> /dev/null
fi

touch $EDEMO_TEMP/foo.$$ 2> /dev/null

if test -f $EDEMO_TEMP/foo.$$
 then
   rm $EDEMO_TEMP/foo.$$
 else 
   echo "$0: can't create $EDEMO_TEMP or can't write in it; exiting. "
   exit 1
fi

# make sure we have full path for temp directory

cur_dir=`pwd`
cd $demo_temp
demo_temp=`pwd`
cd $cur_dir

# tell everyone where the temp directory is 

export EDEMO_TEMP

# set various paths for ESPS and xwaves+

EBASE=`get_esps_base`

ESPS_MENU_PATH="./menus:$EBASE/lib/waves/menus"
export ESPS_MENU_PATH

ESPS_INPUT_PATH="./files:$EDEMO_TEMP:$EBASE/lib/files"
export ESPS_INPUT_PATH

ESPS_PARAMS_PATH="$EDEMO_TEMP:./params:$EBASE/lib/params"
export ESPS_PARAMS_PATH

WAVES_INPUT_PATH="./files:$EDEMO_TEMP:$EBASE/lib/waves/files"
export WAVES_INPUT_PATH

WAVES_MENU_PATH="./menus:$EBASE/lib/waves/menus"
export WAVES_MENU_PATH

WAVES_COMMAND_PATH="./wcommands:$EBASE/lib/waves/commands"
export WAVES_COMMAND_PATH

WAVES_COLORMAP_PATH="./colormaps:$EBASE/lib/waves/colormaps"
export WAVES_COLORMAP_PATH

WAVES_PROFILE_PATH="$EDEMO_TEMP:$EBASE/lib/waves"
export WAVES_PROFILE_PATH

# note, we put $EDEMO_TEMP on the path since play links and other 
# executables might go there

PATH="$EDEMO_TEMP:./bin:$PATH"
export PATH

USE_ESPS_COMMON=off
export USE_ESPS_COMMON

ESPS_VERBOSE=0
export ESPS_VERBOSE

# define a unique socket number for use by the demo'; from here on 
# in, both xwaves and send_xwaves will use this number.  

WAVES_PORT=`randport -S $$`
export WAVES_PORT

# put the $EDEMO_TEMP file in the .wave_pro so that it will be used for
# output files

sed -e s%@DEMO_TEMP@%$EDEMO_TEMP%  files/wave_pro > $EDEMO_TEMP/wave_pro


# start waves; 
# initialization is done via demoinit.WC as specified 
# in wave_pro

xwaves -WP 0 500 -w wave_pro -s &

# now we put up a demo control panel

mbuttons -q1 -X1 -Y 1 -w"Plot3d Demo" -i "plot3d" -b 1 plot3d.BM

# after the user quits fbuttons, we kill xwaves+

send_xwaves @demoquit.WC

# Here we normally remove demo temp files, but there are none



