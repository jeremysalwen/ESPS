#wavestry.com  	@(#)wtry.com	1.8 6/12/90
#command file for starting up waves+ on some test files
#first line must start with waves
waves set colormap esps/lib/waves/Colormap
set show_vals 0
set scrollbar_height 15
make name speech file files/e_demo2.sd loc_x 10 loc_y 190 height 220 
sleep seconds .5
set min_spec_height 325
make name speech file files/e_demo2.fspec loc_x 10 loc_y 520 
sleep seconds 2
attach function esps/bin/xspectrum
sleep seconds 2
make name speech file files/e_demo2.rat start 0 duration 4 loc_x 10 loc_y 420 height 125  
sleep seconds .2
speech marker time 21.5 do_left 1
sleep seconds .2
speech marker time 22 do_left 0
sleep seconds .2
add_espst name compute_stats command fea_stats
shell /tmp/play audio/e_demo1.sd &























