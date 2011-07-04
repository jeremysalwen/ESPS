#!/bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)elpr.sh	1.1	7/29/91	ERL
# 
# Written by:  Ken Nelson
# Checked by:
# Revised by:
# 
# Brief description: Standard way for ERL scripts to print.
# 


if test x$ELPR = x
then
    lpr $*
else
    ${LPR} $*
fi

