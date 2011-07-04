#!/bin/sh 
# @(#)edit_bar.sh	1.5 9/9/93
# This script creates a toolbar giving buttons for edit operations; it 
# also adds an item to the menu (insert cut buffer).  
# The script (named edit_bar.sh) is itself called via a menu selection.  
# Args are: 
# $1 = object name
# $2 = file  name
# $3 = loc_x  
# $4 = loc_y  (loc_x, locy give upper left corner of data view) 
#
# Here is the waves command to add the toolbar creator to the menus:
#
#  add_op name "Create edit toolbar" command edit_bar.sh _name _file _loc_y _loc_x
#

# The result (when this item is selected from the menu, is a toolbar
# with one button for every item currently on the menu

#set -x

pname=editbar$$
name=$1
file=$2
locx=$3
locy=`echo $4 70 - p q | dc`   #note the hack to move it

shift
shift
shift
shift

# create a bbox menu file containing button definitions

bbox_file=.wvtmp_bbox$$
rm -f $bbox_file
touch $bbox_file

echo \"Delete to buffer\" $name op file $file op del_to_buf >> $bbox_file

echo \"Copy to buffer\" $name op file $file op copy_to_buf >> $bbox_file

echo \"View buffer\" make file wcutbuffer.d >> $bbox_file

echo Help shell xtext -t \"Help for Edit Toolbar\" -F `get_esps_base`/lib/waves/edit.help >> $bbox_file

# Create a command file that pops up a panel with the buttons and then
# removes the temporary menu and command files

cmd_file=.wvtmp_cmd$$
rm -f $cmd_file
touch $cmd_file

full_title="Edit toolbar ($file)"

echo "make_panel loc_x $locx loc_y $locy name $pname title \"$full_title\" file `pwd`/$bbox_file" > $cmd_file

# add ops for the cut and copy commands

echo "add_op name del_to_buf menu none command del_to_buf.sh _name _file " >> $cmd_file

echo "add_op name copy_to_buf menu none command copy_to_buf.sh _name _file " >> $cmd_file

# Add a menu item to insert the edit buffer

echo "add_op name \"Insert contents of edit buffer\" command insert_buf.sh _name _file _cursor_time _out.n. _loc_x _loc_y" >> $cmd_file

echo "shell rm $bbox_file" >> $cmd_file
echo "shell rm $cmd_file" >> $cmd_file

send_xwaves @`pwd`/$cmd_file

