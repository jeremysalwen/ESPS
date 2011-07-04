#! /bin/sh
# slideshow.sh @(#)slideshow.sh	1.2 10/3/90
# script for single slide show

echo "Running slide show"

PATH=.:esps/bin:/home/openwin/bin:/home/openwin/bin/xview:/usr/bin/X11:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON
WAVES_MISC=esps/lib/waves; export WAVES_MISC
DSPDEV="NEW,CODEC,FREQ=3072,PATH=./esps/32bin"

lock_play audio/e_snaps1.sd audio/e_snaps2.sd &
screenload snaps/ewave1
sleep 4
screenload snaps/eplots1
sleep 4
screenload snaps/eplots2
sleep 4
screenload snaps/images1
sleep 3
screenload snaps/e3d
wait_for 4
lock_play audio/e_snaps3.sd &
screenload snaps/filter1
sleep 4
screenload snaps/ratio1
wait_for 3
lock_play audio/e_snaps4.sd audio/e_snaps5.sd &
screenload snaps/waves1
sleep 4
screenload snaps/waves2
sleep 4
screenload snaps/ewave2
sleep 4
screenload snaps/waves3
wait_for 4

xrefresh -solid blue
sleep 2

