#! /bin/sh
# testwtry.sh @(#)testwtry.sh	1.1 10/3/90
# script for testing single live waves+ demo

# environment setup 

PATH=.:esps/bin:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON
WAVES_MISC=esps/lib/waves; export WAVES_MISC
DSPDEV="NEW,CODEC,FREQ=3072,PATH=./esps/32bin"; export DSPDEV

echo running interactive waves+ live demo

xwaves+ -w esps/lib/waves/.wave_pro testwtry.com

exit 0
