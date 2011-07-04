# @(#)sat.fspec.com	1.2 6/18/91 ERL - waves command script for plot3d demo
waves set ref_size 650
waves set colormap TImap

waves make name saturn file sat.fspec loc_x 490 loc_y 5 height 400 width 650
saturn bracket 


