This archive contains source files from the ESPS toolkit.

These files are provided "as is". Neither the authors nor the distributors
can provide any form of support for this code. The archive does not claim
to be complete in any sense. No attempt has been made to test it in order
to verify whether it can be compiled.

This software is licensed using a BSD-style license, see the file BSD.txt.

Update 09 Feb, 2009
======================

The sources as presented here have been compiled and run on Apple Mac-OS-X
[versions?] and the Fedora Core 3-6, Ubuntu 8.04 and Scientific Linux 5 (clone
of Centos 5, clone of RHEL 5) Linux distributions.  They need a maintainer,
preferably with autoconfigure skills, to make them properly portable.

At the moment, you build a fresh installation (assuming you start in the
present directory) by doing:

cd ESPS/general
./SETUP [-p <install dir>] [-C]
./ESPS_INSTALL

If you leave out the "-p <install dir>", the installation will go in the
directory /usr/esps. <install dir> doesn't need to exist before you run the
scripts, but you need to have permission to create and write to it.  The -C
option performs 'make clean', removing .o files and binaries from source
directories after copying compiled programs to the installation directory
tree.

SETUP creates a script called emake, which runs make on the directories in the
source tree with various environment variables set to values appropriate to
the system at hand.  It also creates the installation directory and some of
its subdirectories.

ESPS_INSTALL runs the emake script and copies the binaries it creates to the
installation directory.  It takes a while and generates a lot of
compiler warnings.

To run XWAVES/ESPS programs after installation, set the environment variable
$ESPS_BASE to the installation directory, e.g.,

ESPS_BASE=/usr/esps
export ESPS_BASE

and put the directory $ESPS_BASE/bin in your path, e.g.,

PATH=$ESPS_BASE/bin:$PATH
export PATH


==========================================================================
===============  Envorinment set-up for ESPS/xwaves on Mac OS-X ==========
==========================================================================

In the following we will use the name 'Ebase' for full path to the
directory into which you have expanded the installation tar file. (If
you installed ESPS at /usr/local/esps, then Ebase == /usr/local/esps.)

To enable ESPS/waves (xwaves, etc.) on your Mac G4 running OS-X 10.3
or later:

(1) Each user of xwaves should execute this command when logged in as themselves:
    defaults write com.apple.x11 wm_ffm -bool true

(2) Use the X11->Preferences->Output panel to set
    Colors: 256

(3) Make a directory to contain temporary files and data.
    mkdir $HOME/waves
    mkdir $HOME/waves/tmp

(4) Edit your .bashrc to contain the following lines:
   # waves/ESPS setup
   export ESPS_BASE=Ebase
   export PATH=$PATH:$ESPS_BASE/bin
   export USE_ESPS_COMMON=off

(5) Optionally copy the main xwaves config file for customization later:
    cp Ebase/esps60.mac/lib/waves/.wave_pro $HOME

(6) Either start a new terminal window, or source your .bashrc file.

(7) All ESPS programs, including xwaves, should now be available to you.

* Note that the scripts Mplay and Mrecord in $ESPS_BASE/bin are
  the crude, but effective play and record programs for the Mac.
  These scripts use ESPS utilities and the Mac-specific library/main
  that can be found in Ebase/src/macAudio.

* To get started, try: xwaves anyNistHeaderedFile.wav

* The html/pdf documentation can be found at
    file://Ebase/doc/index.html
    (You may not even  need to read any to get some use from xwaves, etc.)

* You can also use eman (like Unix man) to get manpage-style docs.

* You might want to edit $HOME/.wave_pro to customize the behavior to
  your taste.  The customization potential is quite large.

* You may want to occasionally clean out $HOME/waves/tmp to save
  space.  This is where xwaves puts intermediate files.

* If you use this setup, you will find that the following key-bindings
  are active when the mouse is over a displayed waveform:
key		  binding
------------------------------------------------------------------------
space	play window contents
p	play entire file
m	play marked region
=	zoom in
-	zoom out
o	zoom all the way out
b	zoom to markers
Q	stop playback
q	kill the window
