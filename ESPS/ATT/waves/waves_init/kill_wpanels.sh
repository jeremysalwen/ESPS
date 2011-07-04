#!/bin/sh
# @(#)kill_wpanels.sh	1.4 7/7/93
#kill the existing waves panels

#set -x

panel_tmpa=$HOME/.wvtmpA$$.WC
panel_tmpb=$HOME/.wvtmpB$$.WC

send_xwaves save_panels output $panel_tmpa

# danger! 
# This is sensitive to the format produced by save_panels...

awk '{print $17}' $panel_tmpa | sed 's/^/kill_panel name /' > $panel_tmpb

rm -f $panel_tmpa
echo "shell rm -f $panel_tmpb" >> $panel_tmpb

send_xwaves @$panel_tmpb


