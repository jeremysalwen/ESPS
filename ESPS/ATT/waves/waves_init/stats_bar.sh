#!/bin/sh
# @(#)stats_bar.sh	1.3 9/9/93
# This script is an example of creates a toolbar with a button that 
# doesn't correspond to a current menu operation.  
# The script (named stats_bar.sh) is itself called via a menu selection.  

# Args are: 
# $1 = object name
# $2 = file  name
# $3 = loc_x  
# $4 = loc_y  (loc_x, locy give upper left corner of data view) 
#
# Here is the waves command to add the toolbar creator to the menus:
#
#  add_op name "Create output stats toolbar" command stats_bar.sh _name _file _loc_y _loc_x
#
#set -x

pname=statbar$$
name=$1
file=$2
locx=$3
locy=`echo $4 70 - p q | dc`   #note the hack to move it

shift
shift
shift
shift

full_title="Stats ($file)"

# create a bbox menu file containing button definitions

bbox_file=.wvtmp_bbox$$
rm -f $bbox_file
touch $bbox_file

echo \"Output segment stats\" $name op file $file op out_stats >> $bbox_file

# Create a command file that defines the new op, pops up a panel with
# a button for it then removes the temporary menu and command files

cmd_file=.wvtmp_cmd$$
rm -f $cmd_file
touch $cmd_file

# define the operations 

echo "add_op name out_stats menu none command fea_stats _range_samp _file" >> $cmd_file

# pop up the panel 

echo "make_panel loc_x $locx loc_y $locy name $pname title \"$full_title\" file `pwd`/$bbox_file" >> $cmd_file

echo "shell rm $bbox_file" >> $cmd_file
echo "shell rm $cmd_file" >> $cmd_file

send_xwaves @$`pwd`/$cmd_file






