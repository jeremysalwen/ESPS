# %W5 6/18/91 ERL - waves command script for interactive filter design demo
waves set colormap TImap
waves set wave_height 150
waves set y_increment 300
waves set fea_sd_special 0

#waves add filter commands
waves add_espsf name "filter" command filter -d float -Ffilters/proto.filt

#make screen buttons
waves make_panel name filters columns 4 title "POLE - ZERO" icon_title filters file xpz.menu loc_x 18 loc_y 785
