#! /bin/sh
# testslide.sh @(#)testslide.sh	1.2 10/5/90
# script for single slide show

echo "Running slide show"

PATH=.:esps/bin:/home/openwin/bin:/home/openwin/bin/xview:/usr/bin/X11:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON
WAVES_MISC=esps/lib/waves; export WAVES_MISC
DSPDEV="NEW,CODEC,FREQ=3072,PATH=./esps/32bin"

#call next_slide with no semaphore wait to initialize to known state

next_slide 0

xloadimage -fullscreen -prog_slideshow snaps/ewave1 snaps/e3d &

next_slide

#kill xloadimage 
sleep 15
next_slide

exit 0

