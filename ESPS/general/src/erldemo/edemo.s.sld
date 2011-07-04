#! /bin/sh
# edemo.s.sld @(#)edemo.s.sld	1.1 9/11/90	
# main script for single loop demo of waves+ and ESPS  slide show
# to be run under X Windows 
# runs demo one time only

# environment setup 

PATH=.:esps/bin:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON
WAVES_MISC=esps/lib/waves; export WAVES_MISC
DSPDEV="NEW,CODEC,FREQ=3072,PATH=./esps/32bin"; export DSPDEV

log_file=/tmp/erldemo.log

stop_file=/tmp/erldemo.stop

rm -f $log_file
sleep 5
#xwaves+ -w esps/lib/waves/.wave_pro single_demo.com >> $log_file 2>&1 
lock_play audio/e_intro1.sd audio/e_intro2.sd & 
#wait_for 0

#if we happen to be in a continuous demo (repeated execution of 
#edemo.single, check here (as well as in edemo.cont) to see if 
#we should stop; this also enables cutting short a single demo

if test -f $stop_file
then
     exit
else
     snaps.s.demo >> $log_file 2>&1 
fi









