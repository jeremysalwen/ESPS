# %W5 6/18/91 ERL - waves command script for dtw demo
waves make file 100hz.sd loc_x 25 loc_y 160 height 160
waves make file 100swept.sd loc_x 25 loc_y 360 height 160
waves make file 100toswept.sd loc_x 25 loc_y 560 height 160
waves make file dtw_distance loc_x 25 loc_y 760 height 100
