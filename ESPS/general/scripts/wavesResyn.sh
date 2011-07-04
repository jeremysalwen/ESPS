#!/bin/sh

#Usage: $0 F0_file sample_rate rc_file name
tmp=$HOME/waves/tmp
synfile=$tmp/sig.syn
send_xwaves kill name $4 file $synfile
lp_syn -i 0.01 $3 $1  $synfile
send_xwaves make name $4 file $synfile loc_y 500 height 150
Mplay $synfile
