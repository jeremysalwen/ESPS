.lo
.ce 2
.b
Installation Instructions for the ESPS/Waves+ DSP32 Support Kit
.sp
Document Version: 1.10 6/6/90
.sh 1 "Introduction"
.pp
This document describes how to install the Entropic ESPS/\fIwaves+\fR DSP32
Support Kit in Sun 3 and Sun 4 workstations.   There are two parts to
the kit.  One part is the device driver for the AT&T DSP32 board.  The other
part is a set of DSP32 programs, used by \fIwaves+\fR to do analog output and
spectrogram computation.   Record and play programs using the
DSP32 board are also provided.
.sh 1 "Getting Started"
.pp
If you received this kit as part of full ESPS or \fIwaves+\fR
then install that product before proceeding with these instructions.
.pp
If the driver was shipped to you on the same tape as an ESPS or
\fIwaves+\fR product, then it should already be on your disk, since you
should have already read in and installed ESPS or \fIwaves+\fR.
Otherwise
read the tape into an appropriate location.  If you only received the
DSP32 Support Kit, (you already have ESPS or \fIwaves+\fR) then there is just
one directory on the tape, \fIdsp32.sunX\fR, where sunX is either sun3_OS3,
sun3_OS4, sun3x, or sun4 depending on your system type.   
In this case, you should read
the tape into \fI/usr/esps\fR, which will result in
\fI/usr/esps/dsp32.sunX\fR. 
.sh 2 "Sun 3 Versions"
.pp
For the Sun3 there are three versions of the kit, 
depending on operating system version and the machine version;
\fIdsp32.sun3_OS3\fR, \fIdsp32.sun3_OS4\fR, and \fIdsp32.sun3x\fR. 
The first two of these are for 68020 Sun3s with either SUN OS3 or SUN
OS4.  The third version of the kit is for the 68030 Sun3s (Sun refers to
these as architecture type Sun3x).  These machines must run SUN OS4.
.pp
I will just refer to any
of these as \fIdsp32.sunX\fR in these instructions.
You should use the one that is appropriate for your system.
If you received this kit along with full ESPS or 
\fIwaves+\fR, then that product will be on the tape before this kit.
.sh 1 "Installation of the Device Driver"
.pp
All of the files required for the device driver installation are in the
directory \fIdsp32.sunX/driver\fR.
.pp
Please note that these instructions assume that you are 
familiar with installing device drivers into the Unix kernel.   Your
system configuration might vary and these instructions might not match
exactly what you find on your system.   If you are not sure how to
install a driver please consult the Sun documentation that describes
how to edit kernel configuration files and rebuild  the kernel, or
seek assistance before proceeding.   Be sure to make backup copies of all
kernel files before you edit them.
.pp
Find the correct section below for your system type (\fIeg.\fR Sun4 with
OS4, Sun3 with OS4, etc.).
.pp
What you will be doing is to insert the device driver ds.c and dsp32.h
file into the correct directory, edit some device dispatch tables into
the C file that contains these, and add some entries to the kernel
configuration file.  You will then rebuild the
kernel in the usual way.   This new kernel should recognize the DSP32
board if the board is properly installed.
.pp
In these instructions, I will assume that the DSP support kit is located
at \fI/usr/esps/dsp32.sunX\fR.
.sh 2 "Sun4 with OS4"
.np
Log into your Sun as superuser.  
.np
.ft CW
# cd /usr/sys
.ft R
.np
Copy the two files (ds.c and dsp32.h) from \fIdriver/sundev\fR to
\fI/usr/sys/sundev\fR.
.nf
.ft CW
# cp /usr/esps/dsp32.sun4/driver/sundev/ds.c sundev
# cp /usr/esps/dsp32.sun4/driver/sundev/dsp32.h sundev
.fi
.np
.ft CW
cd /usr/sys/sun4/conf
.ft R
.np
Edit your kernel
configuration file (usually your system name in CAPS) to insert the contents of
\fIdriver/conf/ds_device\fR at the end of the configuration file.
.np
Edit \fI/usr/sys/sun4/conf/files\fR to insert the contents of
\fIdriver/conf/ds_files\fR at the end of this file.
.np
.ft CW
cd /usr/sys/sun
.ft R
.np
Edit \fI/usr/sys/sun/conf.c\fR to insert the table entry contained in
\fIdriver/sun/ds_conf.cdevsw\fR.
Add this entry at the end of the character device switch tables in the
file.   Correct the comment to reflect the correct major device number
(it is usually 63), and make a note of this number.   Also insert 
the contents of \fIdriver/sun/ds_conf.include\fR
into \fI/usr/sys/sun/conf.c\fR.  
Add these lines at the end of the other device include
sections above the character switch table.
.np
.ft CW
cd /usr/sys/sun4/conf
.ft R
.np
Run \fIconfig\fR, on your configuration file.  This prepares some files
for the compilation and linking of the kernel.   Then change to
appropriate kernel directory and make the kernel.   Assuming your kernel
configuration file is named SYS, follow these commands:
.nf
.ft CW
# config SYS
# cd ../SYS
# make vmunix
.fi
.ft R
.np
If this kernel makes ok, then save your current kernel as vmunix.bak (or
something like that) and move the new kernel to \fI/vmunix\fR.  Reboot
and if the board is installed, it should see device \fIds0\fR
when it boots.  (This device will be listed along with other
devices, such as disks and tapes, when the kernel boots.)
.nf
.ft CW
# mv /vmunix /vmunix.bak
# mv vmunix /
# reboot
.fi
.ft R
If the new kernel boots ok, but does not see
the board, check to see that the board is seated properly and that the
jumpers are correct (according to information provided by AT&T).  If the
new kernel fails, then check the edits in \fIconf.c\fR and other files.
.sh 2 "Sun3 with OS4"
.pp
This configuration is the same as the Sun4 case above, except that
everywhere we refer to \fI/usr/sys/sun4\fR, you should use
\fI/usr/sys/sun3\fR.
.sh 2 "Sun3x"
.pp
This configuration is the same as the Sun4 case above, except that
everywhere we refer to \fI/usr/sys/sun4\fR, you should use
\fI/usr/sys/sun3x\fR.
.sh 2 "Sun3 with OS3"
.np
Log into your Sun as superuser.  
.np
.ft CW
cd /usr/sys
.ft R
.np
Copy the two files (ds.c and dsp32.h) from \fIdriver/sundev\fR to
\fI/usr/sys/sundev\fR.
.nf
.ft CW
# cp /usr/esps/dsp32.sun3_OS3/driver/sundev/ds.c sundev
# cp /usr/esps/dsp32.sun3_OS3/driver/sundev/dsp32.h sundev
.fi
.np
.ft CW
cd /usr/sys/conf
.ft R
.np
Edit your kernel
configuration file (usually your system in CAPS) to insert the contents of
\fIdriver/conf/ds_device\fR at the end of the configuration file.
.np
Edit \fI/usr/sys/conf/files.sun3\fR to insert the contents of
\fIdriver/conf/ds_files\fR at the end of this file.
.np
.ft CW
cd /usr/sys/sun
.ft R
.np
Edit \fI/usr/sys/sun/conf.c\fR to insert the table entry contained in
\fIdriver/sun/ds_conf.cdevsw\fR.
Add this entry at the end of the character device switch tables in the
file.   Correct the comment to reflect the correct major device number
(it is usually 63), and make a note of this number.   Also insert 
the contents of \fIdriver/sun/ds_conf.include\fR
into \fI/usr/sys/sun/conf.c\fR.  
Add these lines at the end of the other device include
sections above the character switch table.
.np
.ft CW
cd /usr/sys/conf
.np
Run \fIconfig\fR, on your configuration file.  This prepares some files
for the compilation and linking of the kernel.   Then change to
appropriate kernel directory and make the kernel.   Assuming your kernel
configuration file is named SYS, follow these commands:
.nf
.ft CW
# config SYS
# cd ../SYS
# make vmunix
.fi
.ft R
.np
If this kernel makes ok, then save your current kernel as vmunix.bak (or
something like that) and move the new kernel to \fI/vmunix\fR.  Reboot
and if the board is installed, it should see devices \fIds0\fR through
\fIds3\fR when it boots.  (These devices will be listed along with other
devices, such as disks and tapes, when the kernel boots.)
.nf
.ft CW
# mv /vmunix /vmunix.bak
# mv vmunix /
# reboot
.fi
.ft R
.pp
If the new kernel boots ok, but does not see
the board, check to see that the board is seated properly and that the
jumpers are correct (according to information provided by AT&T).  If the
new kernel fails, then check the edits in \fIconf.c\fR and other files.
.ft R
.sh 1 "Create the Special Files in /dev"
.pp
You need to use the Unix command \fImknod\fR to create four special file
entries in \fI/dev\fR for the DSP board.   To do this, edit the file
\fIMAKEDEV\fR in \fIdriver\fR to make the major device number (63 in
the supplied file) to be the same as the major device on your system
(depends on the position of the table entry in
\fI/usr/sys/sun/conf.c\fR).  After you edit \fIMAKEDEV\fR, you can just
run it.  For example:
.br
.ft CW
# sh MAKEDEV
.ft R
.pp
This will create the device special entries in \fI/dev\fR.
.sh 1 "Installation of the DSP32 Programs"
.pp
After you have installed either full ESPS or \fIwaves+\fR,
then you can install the DSP32 programs.
This part of the kit is installed by simply running the makefile
provided in the kit directory.   To do this, you must use the ESPS
program \fIemake\fR, which is simply a script that calls Unix \fImake\fR
with a number of symbols pre-defined.   You have \fIemake\fR if you have
ESPS or \fIwaves+\fR installed.  
If either of these
products is not yet installed, please do that first.
.pp
To install this kit, do (assuming again that the DSP support kit is
under \fI/usr/esps\fR and X is one of Sun3_OS3, Sun3_OS4, Sun3x, or Sun4):
.nf
.ft CW
% cd /usr/esps/dsp32.sunX
% emake install
.ft R
.fi
.pp
You do not need to be superuser to do the above command, but you do need
to be the userid that owns the \fI/usr/esps\fR files.
.pp
This should result in files going into
/usr/esps/32bin, /usr/esps/bin and /usr/esps/man/man1.   If the device
driver has been installed, then \fIwaves+\fR will use the DSP board for
spectrogram computations and analog output.  The standalone ESPS
programs \fIplay\fR and \fIrecord\fR can be used for speech output and
input.   (Note that these programs are stored under the names
\fIwplay\fR and \fIwrecord\fR also).  Consult the manual pages for details
on these programs.
.sh 1 "/etc/dspinfo"
.pp
This file provides some information about the DSP board to the
software and provides the file path to the DSP binaries.  In most
cases with \fIwaves+\fR, the default paths will be correct, but you
should install this file anyway.   Copy
\fI/usr/esps/lib/waves/dspinfo\fR to \fI/etc\fR.  
.nf
.ft CW
% su
# cp /usr/esps/lib/waves/dspinfo /etc
# chmod 0644 /etc/dspinfo
.ft R
.fi
.pp
If you have a new DSP32 board (marked DSP32/VME-FAB2 on the board),
edit the variable DSPDEV as shown:
.sp
.ft CW
DSPDEV NEW,CODEC,FREQ=3072
.ft R
.sp
Otherwise, for old boards:
.sp
.ft CW
DSPDEV OLD,CODEC,FREQ=2048
.ft R
.pp
If the number following
.ft CW
FREQ=
.ft R
is wrong, then the a/d and d/a will run at the wrong rate.   You can
test this, by recording a test tone of known frequency and then
looking at its spectrum with \fIwaves+\fR.
Note that if you set FREQ to 3072 be sure that Jumper J5 on the DSP
board is set to position 5-6 (shorting pins 5 and 6). (This comment is
for new DSP boards with the 24.576 Mhz oscillator).
.pp
These same values can be passed into the software by an environment
variable \fBDSPDEV\fR.   Use of the this environment variable overrides
the information in the \fI/etc/dspinfo\fR file.    But since the board
is associated with a particular machine, rather than a user, it seems
best to simply correctly set up this file and not bother with the
environment variables in most cases.  If a user wanted to specify an
alternate set of DSP binaries, then the environment variable would be
handy.  To use the environment variable, simply define a variable
\fIDSPDEV\fR, with:
.sp
.ft CW
setenv DSPDEV NEW,CODEC,FREQ=3072
.ft R
.sp
to your \fI.login\fR file.
Otherwise, for old boards add:
.sp
.ft CW
setenv DSPDEV OLD,CODEC,FREQ=2048
.ft R
.sh 1 "Possible Problems"
.pp
This section of this document will list problems that other user's have
had, so that you have a place to start if everything doesn't work
correctly from the start.
.sh 2 "Board Installation"
.pp
According to the AT&T instructions supplied with the DSP board, the VME
bus grant jumpers must be removed for the slot that the DSP board is
installed in.   Our experience is that if you place the board in the
highest, or last slot, then it is not necessary to change any VME
jumpers.   In a Sun [34]/110 machine, this would be the top slot.   
.pp
We also have had difficultly with the seating of the board.  If Unix does
not see the board at boot time, then power the machine off and reseat
the board again.  (This assume that you are certain that you are running
the kernel with the DSP driver in it.)
.pp
Note that in a Sun [34]/110 type machine, it is possible for the machine to
run, with the DSP board seated properly, but the CPU board not seated
all the way.   If you have never had any additional boards in your machine
this could be the problem and you would not have noticed any problems
until now.
.pp
Be sure that Jumper J5 on the board is at 5-6 (unless you clearly
understand the board and are doing something on your own).
.sh 1 "Using the AT&T DSP32 Board D/A and A/D at 16Khz"
.pp
Normally, the D/A and A/D on the AT&T DSP32 board runs at 12Khz and
other sample rates are provided by using the DSP to do a rate conversion.
This is the case when jumper J5 on the board is set to 5-6 and the
\fBFREQ\fR part of the environment variable \fBDSPDEV\fR is set to 3072 for a board
with a 24.576Mhz oscillator or 2048 for a board with a 16.384Mhz
oscillator.   Most recent boards have the 24.576Mhz oscillator.  
.pp
If the environment variable \fBCODEC16\fR is defined (it can have any
value) then the board D/A and A/D will be run at 16Khz when used by
\fIwaves+\fR, \fIwplay\fR, and \fIwrecord\fR.   If this variable is not
defined, then the chip will be run at 12Khz.
.pp
Please note, that with \fIwrecord\fR you can ask for different sample
rates using the \fB-r\fR option (it defaults to 8Khz).   The converter
will always run at either 12Khz or 16Khz (depending on \fBCODEC16\fR
being defined) and the DSP is used to sample rate convert the data if
another sample frequency is requested.  Only certain combinations work
and \fIwrecord\fR will tell you which frequency is used.
.pp
In summary, if you defined \fBCODEC16\fR and use the \fB-r 16000\fR
option on \fIwrecord\fR you will recording data at a 16Khz sample rate.
If \fBCODEC16\fR is not defined, but you still use the \fB-r 16000\fR
option, the data is recorded at 12Khz, but converted to 16Khz by the
DSP.
.pp
Note that this environment variable must be defined for each user that
might use the board so it must go in their .login or .cshrc files.  It
would have been better to add this to the \fBDSPDEV\fR variable in
\fI/etc/dspinfo\fR and we'll change it to that in the next version.
.sh 1 "For Help"
.pp
Call Entropic at 1 202 547-1420 (Eastern US timezone).
