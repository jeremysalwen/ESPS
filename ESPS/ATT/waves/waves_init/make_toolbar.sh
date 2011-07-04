#!/bin/sh
# @(#)make_toolbar.sh	1.3 9/9/93
# This script creates a toolbar giving buttons for selected menu operations. 
# The script (named make_toolbar.sh) is itself called via a menu selection.  
# The buttons created have the same names as the menups.  To give different
# names, use make_toolbar2.sh

# Args are: 
# $1 = panel name
# $2 = object name
# $3 = file  name
# $4 = loc_x  
# $5 = loc_y  (loc_x, locy give upper left corner of data view) 
# $6 = toolbar title 
# $* (remaining) = buttons to create (in the form of menuop names) 

# Here is the waves command to add the toolbar creator to the menus:
#
#  add_op name <name for menu> command make_toolbar.sh _name _file _loc_x _loc_y <oplist> <zoombar title> <oplist>
#
# where <oplist> is a list of menuops. Note that this itself could be 
# "_operators" to get a complete set.  The result (when this item is selected
# from the menu, is a toolbar with one button for every item currently on 
# the menu

#set -x

pname=$1$$
name=$2
file=$3
locx=$4
locy=`echo $5 70 - p q | dc`   #try to position above view window

title=$6

shift
shift
shift
shift
shift
shift

# create a bbox menu file containing button definitions

bbox_file=.wvtmp_bbox$$
rm -f $bbox_file
touch $bbox_file

while test "$1" != ""
do
 echo \"$1\" $name op file $file op $1 >> $bbox_file
 shift
done

# Create a command file that pops up a panel with the buttons and then
# removes the temporary menu and command files

cmd_file=.wvtmp_cmd$$
rm -f $cmd_file
touch $cmd_file

full_title="$title ($file)"

echo "make_panel name $pname columns 8 loc_x $locx loc_y $locy title \"$full_title\" file `pwd`/$bbox_file" > $cmd_file

# the command file ends by forking shells to remove the menu file and then
# itself

echo "shell rm $bbox_file" >> $cmd_file
echo "shell rm $cmd_file" >> $cmd_file

send_xwaves @$`pwd`/$cmd_file


