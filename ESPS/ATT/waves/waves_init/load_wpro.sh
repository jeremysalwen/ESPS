#!/bin/sh
# This script resets waves to a profile.  If $1 is present, that's the 
# profile used; otherwise the standard path is used.  We also extract 
# the init file and execute it.  

# @(#)load_wpro.sh	1.5 6/29/93

#set -x

case $# in
0)
	profile=`find_esps_file .wave_pro "$HOME:$ESPS_BASE/lib/waves" WAVES_PROFILE_PATH`
	;;
1)
	profile=$1
	;;
esac

tmp_pro=$HOME/.wvtmp_pro$$

sed '/^#/d' $profile | sed 's/^/set all 1 /' > $tmp_pro

echo "shell rm -f $tmp_pro" >> $tmp_pro

init_file=`egrep init_file $tmp_pro | awk '{print $5}'`

#kill the existing waves panels

kill_wpanels.sh

#now load the new stuff

send_xwaves @$tmp_pro

send_xwaves @$init_file








