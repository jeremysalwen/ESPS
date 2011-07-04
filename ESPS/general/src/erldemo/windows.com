# windows.com  	@(#)windows.com	1.4 6/6/90
# command file for display various pop-up waveform windows
# set command_step 1
waves set scrollbar_height 10
set show_labels 0
set show_vals 0
make name e file files/intro1.sd ref_start 2.38 ref_size .2 loc_x 30 loc_y 200 height 150 width 600
sleep seconds 1
shell lock_play audio/e_windows.sd &
make name f file files/intro2.sd ref_size 1 loc_x 10 loc_y 360 height 150 width 900
sleep seconds 1
make name g file files/intro3.sd ref_size .5 loc_x 300 loc_y 100 height 750 width 300
sleep seconds 1
make name h file files/intro4seg.sd ref_size 10 loc_x 0 loc_y 600 height 200 width 1100
sleep seconds 1
make name i file files/e_fe1.sd ref_size 3 loc_x 100 loc_y 300 height 70 width 300
sleep seconds 2
shell wait_for 3
sleep 1
kill name e
sleep seconds .5
kill name f
sleep seconds .5
kill name g
sleep seconds .5
kill name h
sleep seconds .5
kill name i
sleep seconds .5
set scrollbar_height 15
set show_labels 1
set show_vals 1
sleep seconds 2
return

