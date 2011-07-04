#!/bin/sh

name=$1
file=$2
tmarker=$3
bmarker=$4
plotmax=$5
plotmin=$6
viewstart=$7
viewend=$8
lmarker=$9
shift
rmarker=$9

send_xwaves $name set file $file plot_max $tmarker plot_min $bmarker
send_xwaves $name op file $file op bracket markers

echo $name set file $file plot_max $plotmax plot_min $plotmin > $file.scale
echo $name bracket file $file start $viewstart end $viewend>>  $file.scale
echo $name marker time $lmarker do_left 1 >>  $file.scale
echo $name marker time $rmarker do_left 0 >>  $file.scale
echo shell rm $file.scale >> $file.scale
