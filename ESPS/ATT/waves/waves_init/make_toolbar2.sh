#!/bin/sh 
# 9/9/93 1.5
# This script creates a toolbar giving buttons for selected menu operations. 
# The script (named make_toolbar.sh) is itself called via a menu selection.  
# The buttons created invoke menups but have arbitrary names.  To have the 
# buttons automatically get the same name name as the menuop, use the 
# alternative make_toolbar.sh.

# Args are: 
# $1 = panel name
# $2 = object name
# $3 = file  name
# $4 = loc_x  
# $5 = loc_y  (loc_x, locy give upper left corner of data view) 
# $6 = toolbar title 
# $* (remaining) = buttons to create (in the form of pairs of 
#	                              names: name menuop)
#
# Here is the waves command to add the toolbar creator to the menus:
#
#  add_op name <name for menu> command make_toolbar2.sh _name _file _loc_x _loc_y <oplist> <zoombar title> <oplist>
#
# where <oplist> is a list of name/menuop pairs. Note that this itself could be 
# "_operators" to get a complete set.  The result (when this item is selected
# from the menu, is a toolbar with one button for every item currently on 
# the menu

#set -x

pname=$1$$
name=$2
file=$3
locx=$4
locy=`echo $5 70 - p q | dc`   #note the hack to move it

title=$6

shift
shift
shift
shift
shift
shift

# create a bbox menu file containing button definitions

bbox_file=/tmp/.wvtmp_bbox$$
rm -f $bbox_file
touch $bbox_file

while test "$1" != ""
do
 echo \"$1\" $name op file $file op $2 >> $bbox_file
 shift
 shift
done

# Create a command file that pops up a panel with the buttons and then
# removes the temporary menu and command files

cmd_file=/tmp/.wvtmp_cmd$$
rm -f $cmd_file
touch $cmd_file

full_title="$title ($file)"

echo "make_panel name $pname loc_x $locx loc_y $locy title \"$full_title\" file $bbox_file" > $cmd_file

# the command file ends by forking shells to remove the menu file and then
# itself

echo "shell rm $bbox_file" >> $cmd_file
echo "shell rm $cmd_file" >> $cmd_file

send_xwaves @$cmd_file
send_xwaves return
