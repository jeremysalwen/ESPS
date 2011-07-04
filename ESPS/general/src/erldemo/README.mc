               Entropic Research Laboratory, Inc. 

          Unix Signal Processing Software Demonstration

INTRODUCTION
============

This demonstration software provides a brief introduction to
Entropic's signal processing software -- the Entropic Signal
Processing System (ESPS), and the waves+ graphics interface.

The demo has two parts: "waves+ intro" and "slide show". The waves+
intro is a live demo that shows the software in action. The slide show
is a sequence of fixed ESPS and waves+ images.  The demo has an
optional voice narrative that describes what you see on the screen.  
This option may not be supported on all systems (and it can be turned 
off even on systems that support it).  Note that the text of the
narrative is contained at the end of the file that is displayed 
when the "About this demo..." button is pressed.  (This file usually
is erldemos/edemo.about).  

SETTING UP THE DEMO
===================

The tape contains a single tar image: the directory "entropic".  This
can be loaded anywhere; root privileges are not required.  Extract the
directory with "tar xv".

By setting your umask to 022 ("umask 022") before reading the tar
tape, anyone may execute the demo, but this is not necessary for a
single user.  The demo only requires write permission in the directory
/tmp, so changing the modes on the files to global read-and-execute
permission only ("chmod -R 555 entropic") will not break the demo.

Requirements and recommendations are as follows:

	1) Concurrent 68030 CPU running RTU5.0 or greater 

	2) color (12 planes) display for "slide show" (must).  A color display
	   is highly desirable for the "waves+ intro" portion, as many things 
	   will not work without one.  If the monitor is monochrome, on 
	   some systems the spectrogram displays will come up with 
	   high-energy regions light instead of dark.  In this case, 
	   un-comment "#invert_dither 1" (remove the "#") in the 
	   ".wave_pro" file located in the directory erldemos/esps/lib/waves".

	3) 25 MB disk space (must) 

	4) 16 MB memory highly recommneded. You can try it with only 12 MB,
	   but the audio and slides may loose sync. With less than 12 MB,
	   things become sluggish and sometimes confusing; turn the audio off.

	5) Concurrent's SP-X11 version 1.2 or greater

	6) swap space of at least 20 MB, preferably 30MB

A user must be standing in the "entropic" directory in order to run the demo. 
If you want to make it easy for anyone to run the demo, put a script in
/usr/local or /usr/local/bin that "cd"s to the "entropic" directory and 
runs ENTROPIC. 

RUNNING THE DEMO
=================

To start the demo, change into the entropic directory ("cd entropic")
and run the program ENTROPIC.  This will put up a command window from
which the user can operate the demo. If the control panel does not appear,
check the file "ENTROPIC.log" on /tmp for error messages. 

The top part of the control panel has buttons that present information
about the demo, ESPS, waves+, and the Entropic Research Laboratory.

The bottom part of the control panel allows you to select various demo
and audio options. By using the check boxes, you can select the live
part, the slide show or both.  The "Single" and "Continuous" buttons
determine whether the "Start Demo" button runs the demo once or 
continuously.  The "Audio Output" buttons control the play back 
options.  Once the options have been selected, clicking on the 
"Start demo" button starts the demo.

For more information about the demo, see the file displayed 
when the "About this demo..." button is pressed.  (This file usually
is erldemos/edemo.about).

CONFIGURATION CONSIDERATIONS
============================

This demo has been tested on a 33 mHz, single processor, 68030 based
CPU. It was running Concurrent RTU 5.0 and SP-X11 version 1.2.
The CPU should have at least 16 MB of memory and 25-30 MB of swap space.  
The demo will run on some systems with as little as 12 MB of memory, but 
it can be slow.

On some Masscomp/Concurrent machines the default value for the DISPLAY
environment variable (unix:0.0) might not work.   It may be necessary
to set it to the actual network hostname of the server machine (even if
it is the same machine as is running the client).   If the machine does not
have a running ethernet, set the DISPLAY environment variable to
"localhost:0.0" (or other display number if appropriate).

The OpenLook fonts (supplied here) must be on your path before the demo
will run. The ENTROPIC demo script tries to do this for you, but if the
demo won't run and you see error messages in the ENTROPIC.log file (found
on /tmp) about fonts, move to the "xview" directory and do the following 
before running the ENTROPIC command:

	source setxview.

Note that the command "xset" must be on your path before sourcing the
setxview script, and that on at least some Masscomp/Concurrent machines,
you cannot acess X fonts across NFS.

The demos can be run in either silent or narrated mode.  The default
is for silence. To select the desired output option, just push one of 
the mutually-exclusive buttons in the lower left.  Concurrent has
several D/A options, and by default you get support for device daf0,
clock efclk0, channel number 0, and clock number 0. See the script
"play.mc" in the "erldemos/play_scripts" directory to modify this 
for your system.

The demo has been tested with the Motif window manager. Note that the placement
and positioning of the windows during the demo can be affected by various
parameters in both the ".mwmrc" and ".Xdefaults" files. Versions of files that
worked for us are in the "xconfig" directory. If you are having problems
with placesment or positioning, copy .mwmrc and .Xdefaults to your home 
directory, logout, and log back in. 

The demo should run with any ICCCM compliant window manager, but we have only 
tested it under twm.  Note that under twm, you must have the following line
in your .twmrc file: 	 	

	UsePPosition "on" # use program-specified size hints

(Without this statement, you must manually place windows as they
appear.)

In general, the performance of the demo varies as a function of the
system configuration and load (CPU speed, disk speed, available
memory, role of NFS, network traffic, swap space, other processes,
etc.).  Since there is no foolproof method of synchronizing speech
output with the actual appearance of particular windows; inadequate
system resources may result not just in a slower demo, but in a
confusing demo.

If the demo is not smooth, try running it without speech output, as it
is most reliable in this mode (it is also faster).  If there is
considerable trouble, some system adjustments may be necessary - 
additional swap space, additional memory, additional processors, etc.

For each loop through the demo, a log file is written in /tmp/erldemo.log.
Problems can often be tracked down by looking at this file. Errors detected
during the creation of the demo control panel are logged in /tmp/ENTROPIC.log.

A single run of the demo takes between 5 and 8 minutes, depending
on the system.  

For more information or for help, please contact us:

  Entropic Research Laboratory    Voice:     202-547-1420
  600 Pennsylvania Ave., S.E. 	  FAX:       202-546-6648
  Suite 202                       Internet:  esps@wrl.epi.com
  Washington, D.C. 20003          uucp:	     ...uunet!epiwrl!esps-info
  USA

Particular contacts are:

	David Burton (burton@wrl.epi.com)
	Alan Parker (parker@wrl.epi.com)
	John Shore (shore@wrl.epi.com)         

COPYRIGHT NOTICE
================

The programs (except xloadimage and next_slide), data files, and text
files in this demo directory (and subdirectories) are the property of
Entropic Research Laboratory, Inc.  (Copyright (c) 1986-1990, Entropic
Speech, Inc.  and Entropic Research Laboratory, Inc.; All rights
reserved).  The demo may be freely distributed and used by prospective
customers to evaluate Entropic Software products, or by Sun
Microsystems employees to demonstrate the capabilities of workstations
and windowing software.

Xloadimage was originally written by Jim Frost Copyright (c) 1989,
1990.  Rod Johnson of Entropic modified xloadimage and wrote
next_slide in order to enhance the capabilities for showing a series
of slides.  Both xloadimage and next_slide are in the public domain.
If anyone wants copies of these, please contact us.

---
(@(#)README.mc	1.6 1/10/91)




