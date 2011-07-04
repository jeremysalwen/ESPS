# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1997 Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)plot.tcl	1.4	7/24/98	ERL
# 
# Written by:  Alan Parker
# Checked by:
# Revised by: David Burton (as result of code review)
# 
# Brief description:
# 
# This script implements a plot program for the old "Stanford" plotting
# used by ESPS.   The tcl script is actually called from an interpreter
# linked with the ESPS plot C programs, plotsd, plotspec, aplot, and
# scatplot.
#
############################################################################
#
# The following globals are used:
#
# # These are used by the text entry panel
# canvas_text_var - contains text
# canvas_text_x   - bottom left corner
# canvas_text_y   - bottom left corner
#
# # This is set to indicate that the next data is text in the stream
# text_command - boolean to help parse plot stream
#
# # Initial values, can be reset by commands in the stream
# current_color Black - plot color
# current_width 1.0   - plot width
#
# # These are set by the PostScript panel
# ps_name file   - current file to hold postscript
# ps_command     - current command to accept postscript
# ps_rot         - orientation: landscape or portrait
# ps_color_mode  - color, grey scale, or B/W
# ps_dest        - current output state: file or printer command
# # These are used to restore previous values after a cancel
# ps_color_hold 
# ps_rot_hold 
# ps_name_hold 
# ps_dest_hold 
# ps_command_hold 
#
# # These indicate the current line class, and whether that class is
#   enabled or not. These control the elements visible on the plot canvas.
# line_class - holds identifier for tic marks, vertical axis, box, etc.
# data_on   - boolean
# box_on    - boolean
# hgrid_on  - boolean
# vgrid_on  - boolean
# text_on   - boolean
# ticks_on  - boolean
# orig_text - boolean
#
# # Fonts and annotation text
# normal_font -Adobe-Helvetica-Bold-R-Normal--*
# small_font -Adobe-Helvetica-Bold-R-Normal--*
# text_line(0)
#
#
############################################################################
# The following procedures interpret the plot commands in the input stream
# These are the so-called Stanford plot stream commands.
#
# The input data stream is put into an array (input_data) by the main C
# program before this script is started.  These functions are called to
# evaluate each line of input from the script.
#

#
# Add to auto_path for plot programs; include erltcl library - we need XtxtTool
# The default auto_path has the ESPS_BASE/{tcl,tk,tix} script libraries.

lappend auto_path $env(ESPS_BASE)/lib/erltcl

# set current color
proc c {arg} {
	global current_color
	set current_color [lindex {Black Red Blue Green Red Grey} [expr $arg-1]]
	return $arg
}

# unknown function; not really used, but it does appear at times
proc f {arg} {return}

# move current position (like moving the pen on a plotter)
proc m {y x} {
	global current_x current_y
	global xscale yscale
	set current_x  [expr $xscale*$x]
	set current_y  [expr $yscale*$y]
	return
}

# set the class of the line to draw.  This is used to allow us to
# disable certain lines classes, such as box, vertical axis, horiz axis,
# tick marks, etc.
proc set_class {class} {
	global line_class
	set line_class $class
	return
}

# draw line from current position to position specified
# reset current position to end point
proc d {y x} {
	global current_x current_y current_color current_width line_class
	global xscale yscale
	set new_x   [expr $xscale*$x]
	set new_y   [expr $yscale*$y]
	set new [.c create line $current_x $current_y $new_x $new_y \
		-fill $current_color -width $current_width -tags $line_class]
	set current_x $new_x
	set current_y $new_y
	return
}

# draw line between two points. reset current position to y2 x2
proc l {y1 x1 y2 x2} {
	global current_x current_y current_color current_width line_class
	global xscale yscale
	set y1   [expr $yscale*$y1]
	set x1   [expr $xscale*$x1]
	set y2   [expr $yscale*$y2]
	set x2   [expr $xscale*$x2]
	set new [.c create line $x1 $y1 $x2 $y2 \
		-fill $current_color -width $current_width  -tags $line_class]
	set current_x $x2
	set current_y $y2
	return
}

# text command, next input is text.
proc t {size dummy} {
	global text_command font small_font normal_font
        #
        # The plot language support a variety of font sizes, but here we just
        # support large and small.  Less than 5 is small. 
        #
        if {$size < 5} {
		set font $small_font
	} else {
		set font $normal_font
	}

	#
	# setting this global causes the next input to be interpreted as text
	#
	set text_command 1
	return
}

# set line width
proc s {width} {
	global current_width
	set current_width $width
	return
}


############ The remaining procs are used by the rest of this script #####

# update the scale globals.  The 5500 and 3500 are artifacts of the 
# ESPS plot coordinate universe (which are actually artifacts of the old
# Masscomp plot programs!)   The width and height are scaled down by .9
# to keep the plot off the edges.
#
# Note that xscale and yscale are checked where computed, not where used.

proc update_scales {height width} {
	global xscale yscale
	set xscale [expr $width*.9/5500.0]
	set yscale [expr $height*.9/3500.0]
        if [ expr [expr $xscale <= 0] || [expr $yscale <= 0] ] {
	    tk_dialog .badScale "Invalid Scale Parameters" \
		    "Invalid X and/or Y scale value(s) - exiting" \
		    error 0 OK
	    exit (1)
	}
	return
}

# redraw the original data, even items that have been deleted.  Text
# added is lost.
proc redraw_orig {} {
	global data_on box_on hgrid_on vgrid_on text_on ticks_on
	global temp text_command xscale yscale
	global line_num  input_data
	global text_line orig_text

	.c delete all
        #
        # clear all added text
        #
	foreach i [array names text_line] {
		unset text_line($i)
	}
	set i 0
	set orig_text 1

	#
	# process the array of input data
	#
	while {$i < $line_num} {
		  if {$text_command > 0} {
			do_text $input_data($i)
		  } else {
			eval $input_data($i)
		  }
	incr i 1
	}
	set orig_text 0
	if {$data_on == 0} {.c delete data}
	if {$box_on == 0} {.c delete box}
	if {$hgrid_on == 0} {.c delete h_grid}
	if {$vgrid_on == 0} {.c delete v_grid}
	if {$ticks_on == 0} {.c delete ticks}
	if {$text_on == 0} {.c delete text}
}

# called when the window changes size
proc rescale {} {
	global old_w old_h
	set h [winfo height .c]
	set w [winfo width .c]
        if [ expr [expr $old_w <= 0] || [expr $old_h <= 0] ] {
	    tk_dialog .badGeometry "Invalid Geometry"\
		    "Invalid height or width value sent to \
		    rescale() - exiting" \
		    error 0 OK
	    exit (1)
	}
	set xscale [expr $w/double($old_w)]
	set yscale [expr $h/double($old_h)]
        if [ expr [expr $xscale <= 0] || [expr $yscale <= 0] ] {
	    tk_dialog .badScale "Invalid Scale Parameters"\
		    "Invalid X and/or Y scale value(s) - exiting" \
		    error 0 OK
	    exit (1)
	}
	set old_w $w
	set old_h $h
	.c scale all 0 0 $xscale $yscale
	update_scales $h $w
}

# main drawing function
#
# This function interprets the data in the array input_data
proc redraw {} {
	global text_command xscale yscale
	global line_num  input_data  orig_text font
	global data_on box_on hgrid_on vgrid_on text_on ticks_on
	global text_line text_x text_y text_color current_color text_font
	set i 0
	
	.c delete all
        #
        # process the array of input data
        # 
	while {$i < $line_num} {
		  if {$text_command} {
			do_text $input_data($i) 
		  } else {
			eval $input_data($i)
		  }
	incr i 1
	}

	.c delete text
	#
	# now process all added text
	#
	if {$text_on == 1} {
	    foreach i [array names text_line] {

		set new_x  [expr $xscale*$text_x($i)]
		set new_y  [expr $yscale*$text_y($i)]
		set new [.c create text $new_x $new_y -text $text_line($i)\
			 -font $text_font($i) \
			 -anchor sw -fill $text_color($i) -tags text]
		set text_line($new) $text_line($i)
		set text_x($new) $text_x($i)
		set text_y($new) $text_y($i)
		set text_color($new) $text_color($i)
	        set text_font($new) $text_font($i)
		unset text_line($i) 
	    }
	}
	if {$data_on == 0} {.c delete data}
	if {$box_on == 0} {.c delete box}
	if {$hgrid_on == 0} {.c delete h_grid}
	if {$vgrid_on == 0} {.c delete v_grid}
	if {$ticks_on == 0} {.c delete ticks}
	set orig_text 0
	return
}

# called on input following the "t" command
proc do_text {line} {
	global current_x current_y current_color text_command
	global text_line text_x text_y input_data text_color text_font
	global xscale yscale orig_text font

	if {$orig_text} {
		set new [.c create text $current_x $current_y -text $line \
		-anchor sw -fill $current_color -tags text -font $font]
		set text_line($new) $line
		set text_x($new) [expr $current_x/$xscale]
		set text_y($new) [expr $current_y/$yscale]
		set text_color($new) $current_color
		set text_font($new) $font
	}
	set text_command 0
}

# called to create a dialog box for PostScript output
proc ps_out {} {
	global ps_color_mode ps_rot ps_name ps_dest ps_command
	global ps_color_hold ps_rot_hold ps_name_hold ps_dest_hold 
	global ps_command_hold filename program_name

	set ps_color_hold $ps_color_mode
	set ps_rot_hold $ps_rot
	set ps_name_hold $ps_name
	set ps_dest_hold $ps_dest
	set ps_command_hold $ps_command

        # If the postscript dialog box exists, pop it to the top and return
	if [winfo exists .d] {
	    if {[wm state .d] == "normal"} {
		wm withdraw .d
	    }
	    wm deiconify .d
	    return
	}

	# Create dialog box
	toplevel .d 
	wm transient .d .
	wm title .d \
		"PostScript Dialog (Program: $program_name; File: $filename)"
	set ypos [winfo y .] 
	set xpos [expr [winfo x .] + 70]
	wm geom .d +$xpos+$ypos
	frame .d.cm -relief raised -bd 1 
	pack .d.cm -fill both
	wm resizable .d 0 0
	label .d.cm.color_label -text "Color mode:" 
	radiobutton .d.cm.cm_full -text Color -variable ps_color_mode \
		-value color -anchor w 
	radiobutton .d.cm.cm_gray -text "Gray Scale" -variable ps_color_mode \
		-value gray -anchor w 
	radiobutton .d.cm.cm_bw -text Monochrome -variable ps_color_mode \
		-value mono -anchor w 
	pack   .d.cm.color_label \
		.d.cm.cm_full .d.cm.cm_gray .d.cm.cm_bw -fill both -side left \
		-padx 1m -pady 2m
	frame .d.rot -relief raised -bd 1 
	pack .d.rot -fill both
	label .d.rot.label -text "Orientation:" 
	radiobutton .d.rot.pos_land -text Landscape -variable ps_rot \
		-value 1 -anchor w 
	radiobutton .d.rot.pos_port -text Portrait -variable ps_rot \
		-value 0 -anchor w 
	pack .d.rot.label .d.rot.pos_land .d.rot.pos_port -fill both \
		-side left -padx 1m -pady 2m
	frame .d.panel -relief raised -bd 1 
	pack .d.panel -fill both
	if {$ps_dest == "file"} {
		set label_text "File name:"
		set text_var ps_name
	} else {
		set label_text "Command:"
		set text_var ps_command
	}
	label .d.panel.name_label -text $label_text
	entry .d.panel.entry -relief sunken -bd 2 -textvariable $text_var \
		 -width 30
	pack .d.panel.name_label .d.panel.entry -side left -padx 1m -pady 2m
	frame .d.file_cmd -relief raised -bd 1 
	pack .d.file_cmd -fill both
	label .d.file_cmd.label -text "Output to:" 
	radiobutton .d.file_cmd.file -text File -variable ps_dest \
		-value file -anchor w  \
		-command {
			.d.panel.name_label configure -text "File:"
			.d.panel.entry configure -textvariable ps_name
		}
	radiobutton .d.file_cmd.cmd -text Command -variable ps_dest \
		-value command -anchor w  \
		-command {
			.d.panel.name_label configure -text "Command:"
			.d.panel.entry configure -textvariable ps_command
		}
	pack .d.file_cmd.label .d.file_cmd.file .d.file_cmd.cmd \
		-fill both -side left -padx 1m -pady 2m
	frame .d.action -relief raised -bd 1 
	pack .d.action -fill both
		button .d.action.doit -text "Print" -command \
			{ do_postscript }
	pack .d.action.doit -side left -padx 1m -pady 2m
	button .d.action.cancel -text "Cancel"  \
		-command {
        		wm withdraw .d
			set ps_rot $ps_rot_hold
			set ps_color_mode $ps_color_hold
			set ps_name $ps_name_hold
			set ps_dest $ps_dest_hold
			set ps_command $ps_command_hold
		}
	pack .d.action.cancel -side right -padx 1m -pady 2m
	tkwait visibility .d
}

# called to actually do the PostScript output of the drawing canvas
proc do_postscript {} {
	global ps_name ps_rot ps_color_mode ps_dest ps_command
        #
        # send the output PostScript to a file
        #
	if {$ps_dest == "file"} {
	        #
	        # If file is writable, send postscript to it.
	        #
		if [file writable $ps_name] {
			.c postscript -file $ps_name -rotate $ps_rot \
			-colormode $ps_color_mode
		        wm withdraw .d
		} elseif [expr [file writable [file dirname $ps_name]] && \
			![file exist $ps_name] ] {
			.c postscript -file $ps_name -rotate $ps_rot \
			-colormode $ps_color_mode
		        wm withdraw .d
		} else {
		        tk_dialog .badFile "Invalid File Specification"\
				"The file \"$ps_name\" is not writable.\
				Please specify another and try again."\
				warning 0 OK
		}
	} else {
	        #
	        # Send the output PostScript to a shell command.
	        # Try to check that the command can be executed and
	        # that it executes without error.
	        #
	        if [auto_execok [lindex [split $ps_command] 0]] {
		        .c postscript -file /usr/tmp/etkplt[pid].ps -rotate\
				$ps_rot -colormode $ps_color_mode
		        # Need the 'eval' to explode ps_command into
		        # individual elements
		        if {[catch {eval exec cat /usr/tmp/etkplt[pid].ps | \
				$ps_command} msg] != 0} {
			    tk_dialog .postScriptError "PostScript Printer \
				    Error" "Could not print output.\n\n\
				    Error message follows:\n $msg"\
				    error 0 OK
			} else {
			    # remove dialog from display
			    wm withdraw .d
			}
		        exec rm /usr/tmp/etkplt[pid].ps
		 } else {
		        # First argument can not be found on user's PATH
		        tk_dialog .badCmd "Invalid Command Specification"\
				"Cannot execute the plot command \
				\"$ps_command\". \
				Please specify another and try again."\
				warning 0 OK
	        }
	}
	return
}

# turns on all line types
proc enable_all {} {
	global box_on text_on data_on ticks_on hgrid_on vgrid_on
	set box_on  1
	set text_on  1
	set ticks_on  1
	set hgrid_on  1
	set vgrid_on  1
	set data_on 1
}

# create a little dialog for entering arbitrary text on the canvas
#
# I really need to figure out a better way to do this, but ...
#
proc canvas_text {x y} {
	global canvas_text_var canvas_text_x canvas_text_y
	global xscale yscale
	global text_line text_x text_y text_color text_font

        # We only support one text editing dialog at time,
        # so we need to check on the current existence of
        # it, before we create one.
        if [winfo exists .te] {
	    tk_dialog .teExists "Text Entry Dialog Already Exists"\
		    "Please finish current annotation, before starting \
		    a new one" warning 0 OK
	    if {[wm state .te] == "normal"} {
		wm withdraw .te
	    }
	    wm deiconify .te
	    return
	}

        # Create the text editing dialog
	toplevel .te 
	wm transient .te .
	wm title .te "Enter Text"
	wm geom .te +$x+$y
	wm resizable .te 0 0
	frame .te.panel -relief raised -bd 1 
	pack .te.panel -fill both
	label .te.panel.text -text "Text:" 
	entry .te.panel.entry -relief sunken -bd 2 -textvariable \
		canvas_text_var   -width 30
	pack .te.panel.text .te.panel.entry -side left -padx 1m -pady 2m
	frame .te.panel2 -relief raised -bd 1 
	pack .te.panel2 -fill both
	set canvas_text_x $x 
	set canvas_text_y $y
	button .te.panel2.ok -text "OK" -command {
		.c delete temp
		set new [.c create text $canvas_text_x $canvas_text_y \
		  -text $canvas_text_var -anchor sw -tags text \
		  -font $normal_font]
		set text_line($new) $canvas_text_var
		set text_x($new) [expr $canvas_text_x/$xscale]
		set text_y($new) [expr $canvas_text_y/$yscale]
		set text_color($new) Black
		set text_font($new) $normal_font
		destroy .te
		return} 
	button .te.panel2.cancel -text "Cancel" -command {
		.c delete temp
		destroy .te
		return} 
	pack .te.panel2.ok  -side left
	pack .te.panel2.cancel -side right
	bind .te.panel.entry <Return> { .te.panel2.ok invoke }
}
		
# procedure to move text around
proc move_text {item xDist yDist} {
	global text_x text_y xscale yscale
	.c move $item $xDist $yDist
	set x $text_x($item)
	set y $text_y($item)
	set x [expr $x+$xDist/$xscale]
	set y [expr $y+$yDist/$yscale]
	set text_x($item)  $x
	set text_y($item)  $y
}

# show help file,  This uses the proc used by ensig
proc show_help {} {
	global program_name
	global env

	set title "Help for $program_name"
	set helpfile $env(ESPS_BASE)/lib/plot.help
	set X [winfo x .]
	set Y [winfo y .]
	set xpos [expr $X + 20]
	set ypos [expr $Y + 20]
	XtxtTool .help -t $title -T $title -F $helpfile -p . \
		-W 80 -x $xpos -y $ypos
}


# tclPlotExit:
# Routine handles all normal exits and saves print preferences.
# If there is trouble writing preference file, do nothing.
proc tclPlotExit {} {

       global program_name ps_rot ps_color_mode ps_name ps_dest ps_command env
       
       if [catch {open $env(HOME)/.$program_name w 0600} fID] {
	   #
	   # Ignore preference saving, if file cannot be written
	   #
	   close $fID
       } else {
	   puts $fID "set ps_rot $ps_rot" 
	   puts $fID "set ps_color_mode $ps_color_mode" 
	   puts $fID "set ps_dest $ps_dest" 
	   # ps_name could have funny characters
	   puts $fID "set ps_name \{$ps_name\}" 
	   # ps_command can be multi word token
	   puts $fID "set ps_command \{$ps_command\}" 
	   close $fID
       }
       exit
}

# tclPlotGetPrintPrefs:
# Tries to get print preferences from program specific file.
# If file does not exist or is not reable, do nothing.
proc tclPlotGetPrintPrefs {} {

       global env program_name ps_rot ps_color_mode ps_name ps_dest ps_command

       if [file readable $env(HOME)/.$program_name] {
	   source $env(HOME)/.$program_name
       }
}

#################################################################
#
# Control starts here
#
#################################################################


# initialize a bunch of globals

# These are used by the text entry panel
set canvas_text_var "entry1"	
set canvas_text_x 0	
set canvas_text_y 0

# This is set to indicate that the next data is text in the stream
set text_command 0	

# Initial values, can be reset by commands in the stream
set current_color Black		
set current_width 1.0

# These are set by the PostScript panel
set ps_name file.ps
set ps_command lpr
set ps_rot 0			
set ps_color_mode color
set ps_dest file	
set ps_color_hold $ps_color_mode	
set ps_rot_hold $ps_rot
set ps_name_hold $ps_name
set ps_dest_hold $ps_dest
set ps_command_hold $ps_command

# These indicate the current line class, and whether that class is
# enabled or not.
set line_class none
set data_on 1			
set box_on 1
set hgrid_on 1
set vgrid_on 1
set text_on 1
set ticks_on 1
set orig_text 1		

set normal_font -Adobe-Helvetica-Bold-R-Normal--*-120-*
set small_font -Adobe-Helvetica-Bold-R-Normal--*-100-*
set text_line(0) 1

tclPlotGetPrintPrefs

# define the main panels and canvas

frame .control -relief raised -bd 2 -background lightgrey
frame .dummy -width 10 -height 20
pack .control .dummy -side top  -fill x

menubutton .control.config -text Views -menu .control.config.menu -underline 0
menubutton .control.file -text File -menu .control.file.menu -underline 0 
menubutton .control.help -text Help -menu .control.help.menu -underline 0
		
pack .control.file .control.config -side left
pack .control.help -side right

menu .control.file.menu -tearoff 0
.control.file.menu add command -label "Print..." -command ps_out -underline 0
.control.file.menu add command -label Quit -command tclPlotExit  -underline 0

menu .control.help.menu -tearoff 0
.control.help.menu add command -label "on Plot Tool" -underline 0 -command \
	show_help
.control.help.menu add command -label "About Plot Tool" -underline 0 -command \
	{tk_dialog .foo "About Plot" "ESPS Plot Tool Version 1.0" info 0 OK}

menu .control.config.menu  -tearoff 0
.control.config.menu add checkbutton -label Box  \
	-variable box_on -command {.c delete box}
.control.config.menu add checkbutton -label "Horiz. Grid"  \
	-variable hgrid_on -command {.c delete h_grid}
.control.config.menu add checkbutton -label "Vert. Grid"  \
	-variable vgrid_on -command {.c delete v_grid}
.control.config.menu add checkbutton -label "Text"  \
	-variable text_on -command {.c delete text}
.control.config.menu add checkbutton -label "Tick Marks"  \
	-variable ticks_on -command {.c delete ticks}
.control.config.menu add separator
.control.config.menu add command -label "Enable All"  \
	-command {enable_all}
.control.config.menu add command -label "Redraw Current"  \
	-command {redraw}
.control.config.menu add command -label "Redraw Original"  \
	-command {redraw_orig}

canvas .c -width 570 -height 570 -background white
pack .c -fill both -expand true

set current_x 0
set current_y 0

update_scales  570 570
set old_w 570
set old_h 570

if ([string match none $plot_title]) {
    wm title . "ESPS $program_name, File: $filename"
} else {
    wm title . $plot_title
}
wm iconname . "$program_name: $filename"

wm minsize . 300 300

if {[string length $start_geometry] > 1} {
  	wm geom . $start_geometry
} else {
 	wm geom . +100+0
}

#
# draw for the first time
#
redraw_orig

#
# set up bindings
#

# add q => quit binding, so that it acts like an ensig view
bind . <KeyPress-q>  tclPlotExit 

# button 1 is create text
# button 2 is delete text
# button 3 is move text
#
# The current way that these are handled is not very Motif compliant and
# needs to be redesigned.
#
bind .c <Configure> {rescale}
bind .c <Button-1> {
		.c create text %x %y -text X -tag temp
		canvas_text %x %y
}
.c bind text <Button-3> {
	set curX %x
	set curY %y
}

.c bind text <B3-Motion> {
	move_text [.c find withtag current] [expr %x-$curX] [expr %y-$curY]
	set curX %x
	set curY %y
}

.c bind text <Button-2> {

        # Make sure a delete entry dialog does not
        # already exist.
        if [winfo exists .del] {
	    tk_dialog .delExists "Deletion Dialog Already Exists"\
		    "Please finish current deletion, before starting \
		    a new one" warning 0 OK
	    if {[wm state .del] == "normal"} {
		wm withdraw .del
	    }
	    wm deiconify .del
	    return
	}


        # create the delete entry dialog
	toplevel .del 
	wm transient .del .
	wm resizable .del 0 0
	wm title .del "Delete Text?"
	wm geometry .del +%x+[expr %y +100]
	set obj [.c find withtag current]
	frame .del.panel -relief raised -bd 1 
	pack .del.panel -fill both
	label .del.panel.lab -text "Delete Text?  " 
	button .del.panel.ok -text OK  -command {
		.c delete $obj
		unset text_line($obj) 
		unset text_color($obj) text_x($obj) text_y($obj)
		destroy .del
	}
	button .del.panel.cancel -text Cancel  -command {
		destroy .del
	}
	pack .del.panel.lab .del.panel.ok .del.panel.cancel -side left
}
