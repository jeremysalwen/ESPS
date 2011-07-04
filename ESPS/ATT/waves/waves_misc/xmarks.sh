#!/bin/sh
#
# @(#)xmarks.sh	1.4	25 Sep 1997	ATT/ESI/ERL
# 
# Written by:  David Burton
# Checked by:
# Revised by:  Rodney Johnson, David Talkin
# 
# This script starts xwaves, attaches xmarks, and displays the data file.
# When the xwaves CONTINUE button is pressed, the labels are written
# to $2.mark
#
# NOTE: If this script is run multiple times with the same arguments,
#       the output file will be over-written!
#
# To run this script,
#     copy it to a directory in which you have write permission,
#     make sure it is executable (chmod u+x xmarks.sh), and
#     type its name, followed by
#         an input waveform filename and
#         an input Ascii label-sequence file to be marked (as described
#	           in the xmarks manual page).
# To exit the script,
#     mouse-click on the "Continue" item in the xwaves control panel.
# 

WAVEFORM=$1
LABELSEQ=$2
#
xwaves &
send_xwaves attach function xmarks
send_xwaves set middle_op "blow up; function"
send_xwaves make name $WAVEFORM \
                 file $WAVEFORM loc_y 350 height 200 loc_x 0
send_xwaves send make name $WAVEFORM
send_xwaves send read file $LABELSEQ
send_xwaves pause
send_xwaves send write file $LABELSEQ.mark
send_xwaves quit
#
