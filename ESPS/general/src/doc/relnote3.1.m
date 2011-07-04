.lo
.ce 2
.b
ESPS 3.1 Release Notes
.sp
4/8/88 1.3
.sh 1 "Introduction"
.lp
This document provides notes relevant to release 3.1 of \s-1ESPS\s+1.  Please
read this document and the Installation Instructions completely before
installing this release of \s-1ESPS\s+1.
.sh 1 "Contents of this Kit"
.lp
The ESPS 3.1 installation kit contains magnetic media of the ESPS 3.1
distribution.
This is a complete distribution; it is not an update to 3.0.
For Masscomp systems the magnetic media is
either a set of diskettes or one 9 track tape.  For Sun systems the
media is either a cartridge tape or one 9 track tape.   In any of these
cases, the media contains the source distribution in Unix \fItar(1)\fR
format.  
.lp
If you do not already have 3.0 the 
the kit also contains a User's Manual, containing documents describing
the use of the system and manual pages for all programs, library
functions and file types.   
If you do have 3.0, then the kit contains update pages for your User's
Manual.
The documents provided are important, and
should be read by all users of the system.   The provide essential
information about using and programming with \s-1ESPS\s+1.
.sh 1 "A Comment About Masscomp Universes"
.lp
Masscomp provides a dual universe environment on their systems.  By
using the \fIuniverse\fR command, a user can set his compilation
environment to be either ATT System V, or Berkeley 4.2.   ESPS is to be
installed under the UCB universe (command: \fIuniverse ucb\fR).  In most
cases, since the library is created under ucb, user programs must also
be compiled and linked under ucb.    You will notice one exception, our
scripts and makefiles use the \fIlint\fR under att.  This is because lint has
some problems under ucb.
.sh 1 "Differences Between 3.0 and 3.1"
.lp
Here is a list of changes in \s-1ESPS\s+1 Version 3.1 from the previous
version.
Please duplicate this section and make it available to all users of
\s-1ESPS\s+1.  This information is needed by others than just the \s-1ESPS\s+1
installer.
.ip 1.
\s-1ESPS\s+1 data files are binary compatible on Sun 3 and Masscomp
computers.  This is particular useful, since Masscomp is now supporting
the Sun Network File System (NFS), which allows Suns and Masscomps to
share file\-systems over a local network.
.ip 2.
Several bugs in mcrecord have been discovered and fixed.  
It would sometimes put the wrong sample frequency into the output file
header, it would fail if the file path from the command line was long,
and it set some types of Masscomp converters up for singled-ended
operation, rather than differential.
.ip 3.
Fixed a bug in fft that prevented the transform size from being less
than the frame_size.
.ip 4.
Added get_sd_orecX functions.  These functions are like the standard
get_sd_rec functions, except that they support overlapped frames,
instead of getting an entirely new buffer load each time.
.ip 5.
Improved the performance of mcplay when playing multiple files.  The file
input is now overlapped with the analog output of the previous file.
.ip 6.
Fixed a bug in copysps, that caused it to fail if the copy was done
across file\-systems.
.ip 7.
A set of macros for malloc and add_genhd were added.  These improve the
readability of programs, and improve lint error checking.
.ip 8.
Added the macro MIN to esps.h
.ip 9.
stats now accepts multiple input files.   Also the spacing of the output
has been improved.
.ip 10.
lwb2esps now processes binary, as well as ascii, LWB files.
.ip 11.
\fIfind_ep\fR has been added.  It is a general end point detector for
sampled data files.  (This was really new to 3.0, but was missing from
some copies of 3.0).
.ip 12.
A new program, cross_cor, for computing cross correlations has been
added. 
.ip 13.
A new program, auto, has been added.  This program is for computing
autocorrelations.
.ip 14.
A new program, make_sd, has been added. This program converts a field of
a feature file into a sampled data file.
.ip 15.
Significant improvements have been made to the \fIrange\fR program on
the Masscomp.   On the Sun, the function of this program is incorporated
into \fImcd\fR.   See the man pages for the plot programs, such as
\fIplotsd\fR.
.sh 1 "Installing 3.1"
.lp
If you already have \s-1ESPS\s+1 3.0 installed, there are several options
about how to install 3.1.    If you want the target directory to remain
the same as before, simply read the 3.1 sources any any directory you
desire and follow the installation instructions.  Set the target
directory path in the installation script to be the same as you used for
3.0.   Please read the 3.1 sources into a clean directory; if you want
it to be the same as you used for the 3.0 sources, then rename that
directory to something like esps.old first.  Don't read the tape
over top of the 3.0 sources, some names have changed and things might
get confused.  
.lp
If you are short on file space, then you should probably delete the old
3.0 sources and binaries first.  If you reinstall over the old version a
copy of all binaries will be kept in the bin/old directory.  This will
take several megabytes.
.sh 1 "Graphics on Sun Systems"
.lp
All of the ESPS plot programs produce an intermediate plot stream
output, which is then processed into Unix GPS format.  If the plot is to
the displayed on the Sun screen, this is then processed by the ESPS
program \fImcd\fR, which runs under Suntools.  \fIMcd\fR will create a
window and display the plot in that window.   The window with the plot
can be moved, resized, or reduced to an icon.   It is removed when no
longer needed by using the \fbQUIT\fR item on the menu pulled down by
clicking the mouse on the title bar (just like all tools under
Suntools).
.lp
Do not write any programs which depend upon the intermediate output of
our plot programs.  This will probably change in a future release.
.sh 1 "Hardcopy of Graphics"
.lp
The ESPS plot programs can produce output for the Imagen laser printer
using its Tektronix emulation mode.   If you have another type of
printer or plotter that can plot tektronix plots streams then you should
be able to use it as we use the Imagen.  If you use a postscript based
printer (such as the Apple LaserWriter) we can supply a public domain
tektronix to postscript emulator.   Call us if you want it.
.lp
To adapt another type of plotter to the plot programs, look at the plot
scripts under \fIgeneral/src/plot_script\fR.   Call us if you have
questions.   In the next major release, we are going to provide a better
way to specify the type of hardcopy plot device in the install script.
.uh "Data Acquisition on Suns"
.lp
This version of ESPS does not directly support data acquisition (speech
input and output) on Suns, since there is no standard speech hardware
used by Suns.   This package does include the source code to the speech
input and output programs used on ESPS Masscomp systems.   If you have
data acquisition hardware, you might be able to modify these programs
for your hardware.  The speech input program is called \fImcplay\fR and
the speech output program is called \fImcrecord\fR.
.lp
If you have another means of data acquisition that results in a binary
file (of 16 bit integer data) you can use the ESPS program \fIbtosps\fR
to turn that file into an ESPS sampled data file.   You can also use
\fItestsd\fR with the -a option to produce an ESPS sampled data file
from an ascii file.
.lp
The ESPS program \fIbhd\fR can be used to remove the ESPS header and
leave behind just the binary data from an ESPS data file.   In the case
of sampled data files, you can convert to the correct data type with
\fIcopysd\fR first.
