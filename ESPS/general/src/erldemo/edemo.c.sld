#! /bin/sh
# edemo.c.sld @(#)edemo.c.sld	1.1 9/11/90	
# main script for continuous loop demo of waves+ and ESPS 
# to be run under X Windows 
# runs demo continuously, stopping when a stop file is detected
# note that erldemo.stop is also checked for in the edemo.single script

stop_file=/tmp/erldemo.stop

until (test -f $stop_file) 

do
   edemo.s.sld

done

