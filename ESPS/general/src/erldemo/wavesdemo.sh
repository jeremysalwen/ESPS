#! /bin/sh
# wavesdemo.sh @(#)wavesdemo.sh	1.4 1/14/91
# script for single live waves+ demo

# environment setup 

PATH=.:esps/bin:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON
WAVES_MISC=./esps/lib/waves; export WAVES_MISC
DSPDEV="NEW,CODEC,FREQ=3072,PATH=./esps/32bin"; export DSPDEV

echo running waves+ live intro demo

xwaves+ -w esps/lib/waves/.wave_pro single_demo.com 

exit 0









