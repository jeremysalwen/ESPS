#! /bin/sh
#@(#)xslideshow.s	1.4 11/15/90 ERL 
# script for single X-windows slide show

PATH=.:esps/bin:/home/openwin/bin:/home/openwin/bin/xview:/usr/bin/X11:$PATH; export PATH
ESPS_VERBOSE=0; export ESPS_VERBOSE
USE_ESPS_COMMON=off; export USE_ESPS_COMMON

next_slide 0

xloadimage -prog_slideshow -fullscreen  -quiet snaps/title.i snaps/eplots1 snaps/images1 snaps/ewave2 snaps/filter1 snaps/eplots2  snaps/ewave1&

lock_play audio/e_snaps1.sd& 
next_slide || exit 1 # take down title
next_slide || exit 1#slide2

next_slide  || exit 1 #slide 3
wait_for 0 #e_snaps1.sd 

#
lock_play audio/e_snaps2.sd& 

next_slide || exit 1 #slide 4
wait_for 0  #e_snaps2.sd

#
lock_play audio/e_snaps3.sd &

next_slide || exit 1 #slide 5
wait_for 0  #e_snaps3.sd

#
lock_play audio/e_snaps4.sd& 
next_slide || exit 1 #slide 6
wait_for 0 #e_snaps4.sd

#
lock_play audio/e_snaps5.sd &
wait_for 0  #e_snaps5.sd

sleep 15 
next_slide  15 || exit 1 

sleep 2
