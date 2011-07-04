.lo
.ce 2
.b
ESPS 3.2 Release Notes
.sp
11/8/88 1.4
.sh 1 "Introduction"
.lp
This document provides notes relevant to release 3.2 of ESPS.  Please
read this document and the Installation Instructions completely before
installing this release of ESPS.
.sh 1 "Contents of this Kit"
.lp
The ESPS 3.2 installation kit contains magnetic media of the ESPS 3.2
distribution.  This is a complete distribution; it is not an update to 3.1.
For Masscomp systems the magnetic media is either a set of diskettes or one
9 track tape.  For Sun systems the media is either a cartridge tape or one
9 track tape.  In any of these cases, the media contains the source
distribution in Unix \fItar(1)\fR format.
.lp
If you do not already have ESPS the the kit also contains a User's Manual,
containing documents describing the use of the system and manual pages for
all programs, library functions and file types.  If you do have ESPS, then
the kit contains update pages for your User's Manual.  The documents
provided are important, and should be read by all users of the system.
They provide essential information about using and programming with ESPS.
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
.sh 1 "Differences Between 3.1 and 3.2"
.lp
Please duplicate this section and make it available to all users of
ESPS.  This information is needed by others than just the ESPS
installer.
.sh 2 "New Programs"
.lp
There are thirteen new programs in 3.2 and two replacement programs.  Please
refer to the manual pages for details.
.np
\fIaddfea\fR - adds data (in ASCII) to an existing feature file or creates a
new file.
.np
\fIaddfeahd\fR - adds an ESPS header to a user supplied non-ESPS file.
Used to import files into ESPS.
.np
\fIaddgen\fR - adds a generic header item to an existing ESPS feature
file.
.np
\fIEspsenv\fR - prints the value of Unix environment variables used by
ESPS.
.np
\fIhditem\fR - prints the value of a header item.  Useful for use inside
backquotes in a shell command.
.np
\fIiir_filt\fR - design recursive (IIR) filter.
.np
\fIimage\fR - provides a grey-scale display of a feature file (e.g., for 
spectrograms).
.np
\fIme_spec\fR - \fIfanaspec\fP renamed with new capabilities (will
produce maximum entropy power spectrum from vector of parameters in 
any FEA file; used to require FEA_ANA file).  
.np
\fItranspec\fP - this is similar to \fIspectrans\fR.
The main difference is (like the change from \fIfanaspec\fP to 
\fIme_spec\fP, \fItranspec\fP does not require a FEA_ANA file.  It 
permits the transformation of one spectral representation to another 
for a vector of parameters in any FEA field.  
.np
\fItoep_solv\fR - solves real symmetric Toeplitz systems of linear
equations.
.np 
\fIframe\fR - creates FEA records containing windowed sampled data
frames.
.np
\fIsetrange\fR - convert SD sample range (seconds) to samples and leave
in ESPS Common (useful for specifying range in seconds).  
.np
\fIpwr\fR - computes power of sampled data in FEA files.
.np
\fIwindow\fR - windows sampled data in FEA files.
.np
\fIzcross\fR - computes the average zero crossing rate for sampled data in 
FEA files.
.lp
The programs \fIframe\fP, \fIpwr\fP, \fIwindow\fP, \fIzcross\fP together
demonstrate a processing scheme under consideration as a general
practice for ESPS Version 4.0 programs that process a series of
constant-length sampled-data frames.  The program \fIframe\fP accepts
an ESPS SD file and produces a FEA file with one field per record, the
field containing the the sampled-data for one frame.  \fIFrame\fP
supports overlapping frames and windowing.  The idea is to use
\fIframe\fP as a general preprocessor for any program that works on
frames of sampled data, so that the code for framing and windowing
doesn't have to appear in every such program.  \fIPwr\fP is an 
example of such a program; it processes FEA records containing 
sampled-data vectors and adds a field giving the power in the vector.  
Another program is \fIzcross\fP; it processes FEA records containing
sampled-data vectors and computes the average zero-crossing rate for
each frame.
A command line like:
.sp
	%frame -l 100 -S 10 -w hamming test.sd | pwr - - | zcross - foo
.sp
would leave in foo a FEA file containing records with three fields per
frame:  the sampled data vector for that frame, the power, and the average
zero-crossing rate.  Because windowing sometimes is required after certain
other processing, the program \fIwindow\fP needs to work on sampled data
vector fields of arbitrary FEA files.
.lp
.sh 2 "Other Major Changes"
.lp
For details on the following changes, please refer to the appropriate
manual page.
.np
New version of \fIplotspec\fP.  The new version has an option -D that
forces plotting in dB whether the values in the file are in dB or not.
Spectrum types ST_PWR, ST_REAL, and ST_CPLX are converted to dB before
plotting.
.np
Updated \fImcd\fP on Sun, so that it uses retained image mode
except when resizing (yields much faster graphics when icons are
opened or windows uncovered).
.np
Added -s option to \fIbtosps\fP to specify a number of bytes to skip
at the beginning of the file.
.np
Updated \fIrefcof\fP and \fIfft\fP:  For both of these the following were
added:  overlapping frames (including gaps between frames), and windowing
of sampled data before analysis.
.np
Plot scripts (\fIplotsd, plotspec, aplot, genplot, mlplot\fP) take "hardcopy"
as a device type for the -T option.  Was "imagen" before.  The
"hardcopy" option sends the output in Tektronix format to the program
identified in the install script.
.np
Added -f option to \fIpplain\fP to allow naming of FEA fields.  
.np 
\fIMcrecord\fP (Masscomp only) outputs data file to standard out when
filename given is "-".
.np
\fIFea_stats\fP changed to add the number of vector samples as a field, 
to correct problems in proper record keeping with embedded headers, 
and to fix a memory allocation bug.  
.np
\fIFilter\fP has been modified to allow output data type specification on
the command line.  In addition, checks have been added to detect when too
large a value is stuffed into a smaller data type.
Also sampled frequency in filt file is checked against SD in data file.
.np
Changed \fImcplay\fP to save/get gain and shift from Common.
.np
Removed Fortran \fImatinv\fP routines, and added new matinv.c.
No Fortran at all in ESPS now.
.np
Modified \fIselect\fP so that eval with -e option doesn't put out
record number labels, etc.   Useful when you desire to feed the output
of a \fIselect\fP -e directly to another program, such as \fIaddfea\fP.
.np
Changed \fIselect\fP to create new output file as segment labeled if
the input files are tagged.
.np
Added field_order option to headers.c and feasupport.c in the library.
This causes feature files to be written in field order, that is, the order
of the data record fields in the disk file is the order in which fields
were defined in the feature header, rather than ordered by data type.
See FEA(5\-ESPS) for details.  This change was needed for the new program
\fIaddfeahd\fP.  Added a check to gensupport.c to print a message and exit
if it is called on a field_order file.
.np
Added hostname to current path field (headers.c).
.np
Modified plot scripts, install script, and makefile to support
the ability to specify the name of the \fImcd\fP program (needed for
integral graphics on Masscomp with mcdigh).
.np
Changed \fIfea_element\f to add -n option to suppress column headings
(useful when output is to be used by \fIaddfeahd\fP).  
.np
Added new library functions addstr to library (and man page).
.np
\fILpcana\fP updated;  default values for min_pp, lpc_frame_size, and
lpc_order added.
.np
Changed parameter file handling (symbols.c) so that if a parameter is to
be prompted for (by using ?= in parameter file) and standard input is
not the user's terminal (the data file is going in on standard input,
for example), the default value is taken instead.  A warning is printed
in this case.
.np
Added complex sqrt to library; manual page complex.3 updated.
.np
\fIFea_stat\fP has been updated to operate on other than just FEA_ANA files.
.np
Added support for local library functions and local programs.  A
skeleton local directory is provided.
.np
New library function \fIstsolv(3)\fR.
.sh 2 "Other Changes"
.np
Fixed the man page for \fIpspsdiff\fP to mention a problem with
options (a space is required around them).
.np
Fixed the man page for \fIwmse_filt\fP to clarify the prompt mode.
.np
Fixed \fIfea_edit\fP to not drop the refer file.
.np
Changed man page for \fIcopysps\fP to mention segment labeled and
tags.
.np
Fixed problem in install script that was causing header files
to be installed mode 440 regardless of what was specified in
the install script.
.np
Added fldrangesw.c to library and updated man page for rangeswtch.3.
.np
\fISpectrans\fP fixed to save correct file as source file.
.np
\fIvq\fP:  Some header items and history were not being written in
checkpoint file.  \fIvqdes\fP and \fIvqdesasc\fP man page description of -i
option was wrong.  Also fixed SEE ALSO section.
.np
Fixed makefile to \fIcross_cor\fP to install default parameter file.
.np
Fixed fmatalloc.3 
arr_alloc.3, calloc.3, malloc.3, getfeasiz.3, savestring.3, 
skiprec.3, symtype.3 manual pages. (Minor typos or reference problems)
Updated fea.5t man page and copyheader.3 man page.
Updated writeheader.3 man page.
getsdorec.3 manual page updated.
.np
Fixed \fIlpcana\fP so that it fills in "refer" with the input file name.
.np
Fixed \fIwmse_filt\fP. Filter type changed to FILT_ARB and
samp_freq added as generic.
.np
Updated \fIfiltspec\fP to correct an error message.
.np
Fixed \fIpsps\fP so that it displays filter type and method
correctly, instead of just displaying code values.  It also now accepts
a range of tags with the -t option.
.np
Fixed putsym_f (it wasn't updating correctly).
Added putsym_f to lint library.
.np
Fixed get_cmd_line(3\-ESPS):  if called more than once, it would
append the command line to the buffer already containing the
command line.
.np
\fIMake_sd\fP usage statement fixed.
.np
Fixed \fIselect\fP to clear the record before copying from input
to buffer.  This way, if all fields are not being copied, the
others will be zero.
.np
Fixed \fIselect\fP != operator.  Didn't work on coded and strings.
.np
\fIFind_ep\fP man page updated.
.np
\fIHis\fP updated (and man page).  Default number of bins is now 64.
.np
Updated \fIfiltspec\fP and test script. Does computations in doubles
instead of floats.
.np
Added a number of Convex fixes to install script.
.np
Fixed bug in headers.c relating to zapping of reference
headers in write_header.
.np
Fixed parameter processing (symbols.c) so that verbose message doesn't come
out for prompted symbols, even if you get them twice. 
.np
In \fIaplot\fR two axis-label bugs were fixed:  0.0 sometimes
appeared as a small floating-point number with lots of digits;
the last label was sometimes omitted.
.np
Updated \fIfft_filter\fR: The sampling frequency in the input sampled 
data file is checked against the filter's design sampling frequency. 
A warning is issued if they are different.
Also fixed a problem with getting the coefficients from the
parameter file, instead of a file file and
the sample freq wasn't going into the header.
.np
\fIScatplot\fR: The changes were to correct axis-labeling bugs:
0 sometimes appeared as a small nonzero floating-point number,
and the last axis label was sometimes omitted.  The changes in
scatplot.c were a major revision.  The first input file no
longer plays a special role in determining range and scaling.
Automatic scaling is now based on the maximum and minimum over
all records of all files of the x or y coordinate.  Any input
file can be stdin.  The -r option can be repeated to specify
different ranges for different files.  Default is to plot all
records from allfiles.
.np
\fIMlplot\fR and \fIgenplot\fR changed to pay attention to environment
variable BUNDLE, like other plot programs.
.np
Added record number as comment in \fIfea_edit\fR ascii file.
.np
New library function \fIcopy_genhd(3)\fR for copying generic header
items from one header to another.
.np
Various ass-sordid bugs fixed.  
.sh 1 "Installing 3.2"
.lp  
If you already have ESPS 3.1 installed, there are several options
about how to install 3.2.    If you want the target directory to remain
the same as before, simply read the 3.2 sources any any directory you
desire and follow the installation instructions.  Set the target
directory path in the installation script to be the same as you used for
3.1.   Please read the 3.2 sources into a clean directory; if you want
it to be the same as you used for the 3.1 sources, then rename that
directory to something like esps.old first.  Don't read the tape
over top of the 3.1 sources, some names have changed and things might
get confused.  
.lp
If you are short on file space, then you should probably delete the old
3.1 sources and binaries first.  If you reinstall over the old version a
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
longer needed by using the \fBQUIT\fR item on the menu pulled down by
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
To adapt another type of plotter to the plot programs, look at the install
script under \fIesps_plot\fR.   Call us if you have
questions.   In the next major release, we are going to provide a better
way to specify the type of hardcopy plot device in the install script.
.sh 1 "Data Acquisition on Suns"
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
