#intro.com  	@(#)intro.com	1.4 11/8/90
#puts up a few displays and shimmers them while playing introductory text
#color shimmering is done first with greymap and then with Colormap
waves set colormap esps/lib/waves/greymap
set scrollbar_height 10
shell lock_play audio/e_intro1.sd &
make name a file files/speech.sd loc_x 45 loc_y 190 height 280 width 500 start .05 duration .97
sleep seconds .2
set min_spec_height 325
make name b file files/speech.wb.spgm loc_x 10 loc_y  510
sleep seconds 1
shell wait_for 0
make name d file files/mbrot.fspec loc_x 550 loc_y 70 duration 600
sleep seconds .2
make name c file files/speech.lsfn start 0 duration 4 loc_x 550 loc_y 500 height 400 width 575 
sleep seconds 2
shell lock_play audio/e_intro2.sd audio/e_intro3.sd &
call commandname shimmer.com
call commandname shimmer.com
set colormap esps/lib/waves/Colormap
call commandname shimmer.com
call commandname shimmer.com
shell wait_for 0
set scrollbar_height 15
kill
sleep seconds 5
return


