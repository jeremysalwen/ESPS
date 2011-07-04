#! /bin/sh 
# top level script for the wave+ INTRO demo
#@(#)rundemo.sh	1.6 12/1/92 ERL

# check syntax

case $# in
1)
        # any output files go in directory specified by $1
        demo_temp=$1
        ;;
0)
        demo_temp=/usr/tmp/Edemo$$
        echo "No temporary directory specified, will use $demo_temp"
        ;;
*)
	echo "Specify only one writable directory for output files."
	echo "rundemo exiting."
	exit 1
	;;
esac


# create temp directory if necessary, check that it is writable

if test ! -d $demo_temp
then
  mkdir $demo_temp 2> /dev/null
fi

touch $demo_temp/foo.$$ 2> /dev/null

if test -f $demo_temp/foo.$$
 then
   rm $demo_temp/foo.$$
 else
   echo "$0: can't create $demo_temp or can't write in it; exiting. "
   exit 1
fi


# Put demo_temp in the environment

export demo_temp

# make sure we have full path for temp directory

cur_dir=`pwd`
cd $demo_temp
demo_temp=`pwd`
cd $cur_dir


# first, set various paths for ESPS and xwaves+

EBASE=`get_esps_base`

ESPS_MENU_PATH="./menus:$EBASE/lib/waves/menus"
export ESPS_MENU_PATH

ESPS_INPUT_PATH="./files:$demo_temp:$EBASE/lib/files"
export ESPS_INPUT_PATH

ESPS_PARAMS_PATH="$demo_temp:./params:$EBASE/lib/params"
export ESPS_PARAMS_PATH

WAVES_INPUT_PATH="./files:$demo_temp:$EBASE/lib/waves/files"
export WAVES_INPUT_PATH

WAVES_MENU_PATH="./menus:$EBASE/lib/waves/menus"
export WAVES_MENU_PATH

WAVES_COMMAND_PATH="./wcommands:$EBASE/lib/waves/commands"
export WAVES_COMMAND_PATH

WAVES_COLORMAP_PATH="./colormaps:$EBASE/lib/waves/colormaps"
export WAVES_COLORMAP_PATH

WAVES_PROFILE_PATH="$demo_temp:$EBASE/lib/waves"
export WAVES_PROFILE_PATH

PATH=$demo_temp:./bin:$PATH
export PATH

USE_ESPS_COMMON=off
export USE_ESPS_COMMON

ESPS_VERBOSE=0
export ESPS_VERBOSE

# define a unique socket number for use by the demo'; from here on
# in, both xwaves and send_xwaves will use this number.

WAVES_PORT=`randport -S $$`
export WAVES_PORT

# put the $demo_temp file in the wave_pro so that it will be used for
# output files

sed -e s%@DEMO_TEMP@%$demo_temp%  files/wave_pro > $demo_temp/wave_pro

# start waves 
# initialization for demo is done via xw_init.WC as specified 
# in wave_pro
# wtry.WC puts up initial displays

xwaves -w wave_pro -s wtry.WC &

# next sleep is used to make text come up on top

sleep 4
xtext -Wp 5 540 -F help/inter_waves.doc -t "waves+ INTRO Demo" -i "Intro Help" &


exit 0

