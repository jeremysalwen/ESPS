#!/bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1987-1990  AT&T, Inc. 
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)marks.sh	1.3	4/20/91	ATT/ESI/ERL
# 
# Written by:  David Burton
# Checked by:
# Revised by:
# 
# This script starts waves+, attaches marks, and displays the data file.
# To run this script,
#     copy it to a directory in which you have write permission,
#     make sure it is executable (chmod u+x marks.sh), and
#     type its name, followed by
#         an input waveform filename and
#         an input Ascii label-sequence file to be marked.
# To exit the script,
#     mouse-click on the "Continue" item in the waves+ control panel.
# 

PROC=/tmp/marks$$
WAVEFORM=$1
LABELSEQ=$2
#
echo waves attach function marks >${PROC}
echo waves set middle_op "blow up; function" >>${PROC}
#echo waves pause >>${PROC}
echo waves make name $WAVEFORM file $WAVEFORM loc_y 350 height 200 loc_x 0 >>${PROC}
echo waves send make name $WAVEFORM >>${PROC}
echo waves send read file $LABELSEQ >>${PROC}
echo waves pause >>${PROC}
echo waves send write file $LABELSEQ.mark >>${PROC}
echo waves send kill >>${PROC}
echo waves kill >>${PROC}
#
waves ${PROC}
