#! /bin/sh
#@(#)get_demo.mc	1.9 1/17/91
# Assume that you are standing in the traget "entropic" directory
# This only works for the Concurrent machines
#
# fix the link in esps to point to the masscomp binaries
rm -f $HOME/entropic/erldemos/esps/bin
ln -s $HOME/entropic/erldemos/esps/bin.mc $HOME/entropic/erldemos/esps/bin

rsh masscomp fixowner burton erldemo


# fill out entropic directory
esccs erldemo get ENTROPIC.mc;mv ENTROPIC.mc ENTROPIC; chmod 555 ENTROPIC
esccs erldemo get README.mc;mv README.mc README;  chmod 444 README
esccs erldemo get version.mc;mv version.mc version;  chmod 444 version 
mkdir xview;chmod 755 xview;cd xview;cp -r $HOME/entropic/xview.mc/* .
chmod 444 README; chmod 555 setxview;  chmod 755 fonts; cd fonts 
chmod 755 75dpi misc; chmod 444 75dpi/* misc/*; cd ../.. 
cp -r $HOME/entropic/xconfig.mc xconfig;chmod 755 xconfig;chmod 444 xconfig/*

#make script files
mkdir erldemos;chmod 755 erldemos
cd erldemos
esccs erldemo get attach.com; chmod 444 attach.com
esccs erldemo get cursors.com; chmod 444 cursors.com
esccs erldemo get intro.com; chmod 444 intro.com
esccs erldemo get shimmer.com; chmod 444 shimmer.com
esccs erldemo get single_demo.com; chmod 444 single_demo.com
esccs erldemo get waveforms.com; chmod 444 waveforms.com
esccs erldemo get windows.com; chmod 444 windows.com
esccs erldemo get xslideshow.sh; chmod 555 xslideshow.sh
esccs erldemo get wavesdemo.sh; chmod 555 wavesdemo.sh
esccs erldemo get erldemo.sh; chmod 555 erldemo.sh
esccs erldemo get one_demo.sh; chmod 555 one_demo.sh
esccs erldemo get edemo.ab.mc;mv edemo.ab.mc edemo.about; chmod 444 edemo.about
esccs erldemo get edemo.info; chmod 444 edemo.info

#make audio files
mkdir audio; chmod 755 audio; 
cp $HOME/entropic/erldemos/audio/* audio; chmod 444 audio/*

#make files for displaying data 
mkdir files; chmod 755 files
cp $HOME/entropic/erldemos/files/* files; chmod 444 files/*

#make esps tree
mkdir esps;chmod 755 esps;cd esps

cp -r $HOME/entropic/erldemos/esps/bin bin;chmod 755 bin; chmod 555 bin/*
cp -r $HOME/entropic/erldemos/esps/man man; 
chmod 755 man;cd man;chmod 755 man1; chmod 755 cat1;
chmod 444 whatis;chmod 444 man1/*;chmod 444 cat1/*
cd ..
cp -r $HOME/entropic/erldemos/esps/lib lib; 
chmod 755 lib;chmod 755 lib/waves;chmod 444 lib/waves/*;chmod 444 lib/waves/.wave_pro

cd ..

#make slides
mkdir snaps; chmod 755 snaps
cp $HOME/entropic/erldemos/snaps/eplots1 snaps; chmod 444 snaps/eplots1
cp $HOME/entropic/erldemos/snaps/images1 snaps; chmod 444 snaps/images1
cp $HOME/entropic/erldemos/snaps/ewave2 snaps; chmod 444 snaps/ewave2
cp $HOME/entropic/erldemos/snaps/filter1 snaps; chmod 444 snaps/filter1
cp $HOME/entropic/erldemos/snaps/eplots2 snaps; chmod 444 snaps/eplots2
cp $HOME/entropic/erldemos/snaps/ewave1 snaps; chmod 444 snaps/ewave1
cp $HOME/entropic/erldemos/snaps/title.i snaps; chmod 444 snaps/title.i


#make play scripts
mkdir play_scripts; chmod 755 play_scripts; cd play_scripts
esccs erldemo get play.mc play.none ;chmod 555 play.*


cd ../..
echo " "
echo all done

