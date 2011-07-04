#! /bin/sh
# @(#)rundemo.sh	1.5 1/18/93 ERL
# top level script for the testsignal demo

# check syntax

case $# in
1)
        # any output files go in directory specified by $1
	demo_temp=$1
	;;
*)
	demo_temp=/usr/tmp/Edemo$$
	echo "no temporary directory specified, will use $demo_temp"
	;;
esac

# create temp directory if necessary

if test ! -d $demo_temp
then
  mkdir $demo_temp 2> /dev/null
fi

# make sure we have full path for temp directory and put name in $HOME
cur_dir=`pwd`
cd $demo_temp
demo_temp=`pwd`
cd $cur_dir

#Is temp writable
touch $demo_temp/foo.$$ 2> /dev/null

if test -f $demo_temp/foo.$$
 then
   rm $demo_temp/foo.$$
 else 
   echo "$0: can't create $demo_temp or can't write in it; exiting. "
   exit 1
fi

#Put demo_temp into the environment
export demo_temp

# set various paths for ESPS and xwaves+

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

# note, we put $demo_temp on the path since play links and other 
# executables might go there

PATH="$demo_temp:./bin:$PATH"
export PATH

USE_ESPS_COMMON=off
export USE_ESPS_COMMON

ESPS_VERBOSE=0
export ESPS_VERBOSE

# define a unique socket number for use by the demo'; from here on 
# in, both xwaves and send_xwaves will use this number.  

WAVES_PORT=`randport -S $$`
export WAVES_PORT

# try to set up for the right audio output

guess_audio $demo_temp

# put the $demo_temp file in the .wave_pro so that it will be used for
# output files

sed -e s%@DEMO_TEMP@%$demo_temp%  files/wave_pro > $demo_temp/wave_pro

# start waves; 
# initialization is done via demoinit.WC as specified 
# in wave_pro

xwaves -w wave_pro -s &

xtext -Wp 555 350 -F README &

# now we put up a demo control panel

mbuttons -q1 -X555 -Y 210 -w"Test Signal Controls" -i "test" -b 2 tsignal.BM

# after the user quits fbuttons, we kill xwaves+

send_xwaves @demoquit.WC

# and we remove all the test files

rm -f $demo_temp/testsig.sd demo_temp/tsig*


