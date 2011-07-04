# @(#)filter.com	1.3 6/21/91 ERL - waves command script for waves+ filters demo
waves set colormap TImap
waves set wave_height 150
waves set y_increment 300

# waves+ command script to show menu customization for a "task"

#delete standard items
waves delete_item name "spectrogram (W.B.)"
waves delete_item name "spectrogram (N.B.)"
waves delete_item name "delete segment"
waves delete_item name "play window contents"
waves delete_item name "play to end of file"
waves delete_item name "kill window"
waves delete_item name "Run psps on segment"
waves delete_item name "zoom out"
waves delete_item name "zoom in"
waves delete_item name "page ahead"
waves delete_item name "page back"
waves delete_item name "save segment in file"
waves delete_item name "insert file"
waves delete_item name "align & rescale"
waves delete_item name "bracket markers"
waves delete_item name "play between marks"
waves delete_item name "play entire file"

# add items needed

# add play commnads
waves add_waves name "play marks" builtin "play between marks"
waves add_waves builtin "play entire file"
waves add_espsn menu wave name "play channel 0" command play0
waves add_espsn menu wave name "play channel 1" command play1
waves add_waves name "zoom to marks" builtin "bracket markers"
waves add_waves builtin "zoom out"
waves add_espsn menu spect name "3D plot" command plot3d -oR -w

#waves add filter commands
waves add_espsf menu wave out_ext fbank name "Filter Bank  (mb2 and mb3)" command filter.bank2
waves add_espsf menu wave name "Demux Filter Bank" outputs 2 command demux 
waves add_espsf menu wave name "FFT Analysis" command xsgram
waves add_espsf menu wave name "Maximum Entropy Analysis" command xmesgram

#make screen buttons
waves make_panel name filters columns 8 title "Filter Support" icon_title filters file wfilters.menu loc_x 18 loc_y 455
