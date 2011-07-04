This demonstration provides a brief introduction to Entropic's signal
processing software -- the Entropic Signal Processing System (ESPS),
and the waves+ graphics interface.  

The demo has two parts: "waves+ intro" and "slide show". The waves+
intro is a live demo that shows the software in action.  The slide
show is a sequence of fixed ESPS and waves+ images.  The demo has an
optional voice narrative that describes what you see on the screen.
This option may not be supported on all systems (and it can be turned
off even on systems that support it).

STARTING THE DEMO
=================

The demo is started by clicking on the "Start demo" button that is
located in the upper left corner of the lower control panel. The rest
of the buttons and check boxes configure the demo.

CONFIGURING THE DEMO
====================

Single vs. Continuous
---------------------

The mutually-exclusive push-buttons in the uper right of the lower
panel determine whether the demo is run once or continuously.  If a
continuous demo is started, the button label "Single" changes to "Stop
Continuous".  Pressing this button stops the demo automatically at the
first convenient opportunity -- after either the "waves+ intro" or
"slide show" finishes.  After clicking on "Stop Continuous", faster
termination usually can be achieved by clicking on QUIT in the waves+
command window (if the waves+ intro is running), or by typing "q" in
the slide show window.  If a single demo is running, it can be 
terminated by these same actions.  

waves+ Intro and/or Slide Show
------------------------------

Whether the demo comprises the waves+ intro, the slide show, or both, 
is determined by the check boxes at the lower right.  You can check 
one or the other or both (just point and click).  At least one must be 
checked.  

Audio Output
------------

The demos can be run in either silent or narrated mode.  The default
is for silence. To select the desired output option, just push one of 
the mutually-exclusive buttons in the lower left.  Concurrent has
several D/A options, and by default you get support for device daf0,
clock efclk0, channel number 0, and clock number 0. See the script
"play.mc" in the "play_scripts" directory to modify this for your system.
In general, do not select an output option if your hardware is not 
equipped appropriately.

Note that you can change the speech output option at any time, even
when a demo is running.  The change will take effect on the subsequent
output commands. Also, note that the text of the narrative is included
at the end of this file.

PAUSING THE DEMO
================

The "waves+ intro" part of the demo can be paused at any time by left
mouse clicking on the "PAUSE" button in the waves+ control panel. To
restart it, click on the "CONTINUE" button.  It is best to avoid 
attempting any waves+ operations while in the PAUSEd mode.  Some will 
work just fine, but you risk corrupting the demo.  

The "slide show" part of the demo can be paused by typing "p" in the
slide show window.  The demo will pause on the current or the next
slide.  (The uncertainty arises because the demo program does not know
whether or not X windows has actually displayed the image.)  To
continue type "c".

STOPPING THE DEMO
=================

To stop the single demo during the "waves+ intro" part, click on the
"QUIT!" button in the waves+ control panel.

To stop the single demo during the "slide show" part, type "q" with the 
mouse in the slide window.  

To stop the continuous demo, first push the "Stop Continuous" selector
in the ENTROPIC DEMO control panel.   Now either click on "QUIT!" or
type "q" depending on which part of the demo is running.

PERFORMANCE AND RUNNING TIME
============================

This demo has been tested on a Concurrent 6350 with RTU 5.0 and 
SP-X11 version 1.2.  Ideally, the CPU should have 16 MB of memory and
25-30 MB of swap space.  The demo will run on some systems with as
little as 12 MB of memory, but it can be slow.

In general, the performance of the demo varies as a function of the
system configuration and load (CPU speed, disk speed, available
memory, role of NFS, network traffic, swap space, other processes,
etc.).  Since there is no foolproof method of synchronizing speech
output with the actual appearance of particular windows, inadequate
system resources may result not just in a slower demo, but in a
confusing demo.  

If the demo is not smooth, try running it without speech output, as it
is most reliable in this mode (it is also faster).  If there is
considerable trouble, some system adjustments may be necessary; ask
your system administrator to consider the suggestions in the README
file from the main demo directory.

If you suspect that something has been going wrong, some hints about the
problem may be obtained by checking the log file written for each demo
cycle in /tmp/erldemo.log.

A single run of the demo takes between 5 and 8 minutes, depending
on the system.  

NARRATIVE TEXT 
==============

In case you would rather read than listen, here is the text of the 
demo narrative:

Entropic Research Laboratory is pleased to present an integrated unix
software environment for signal processing research, development, and
applications.

This demonstration will show selected features of the Entropic Signal
Processing System (ESPS), an extension of unix for signal processing,
and selected features of waves+, a program that provides interactive
viewing, manipulating, and processing of signals.

ESPS and waves+ can be operated directly, using the keyboard and
mouse, or indirectly, as in this demonstration, using waves+ command
files and unix shell scripts.

The waves+ user-interface takes advantage of standard, familiar
operations supported by the X window manager.  Generic operations,
such as window open, close, resize, and delete, are inherited by waves+. 

Waveform windows can contain a cursor, shown here in red, as well as
left and right segment markers, shown here in green.  Also displayed
are numerous digital readouts related to the cursor and marker
positions.  

Various operations can be performed on the marked waveform segment.  

One example is zooming or magnification.  

Another example is scrolling.  

Waveforms can also be edited by hand.  

Other operations include audio output, spectrogram computation, and
arbitrary computations via calls to external ESPS programs.  Here we
see examples of wideband and narrowband spectrograms.

Note that any vector time series can be displayed in this form.  

waves+ colormaps are determined by ASCII files specified by the user.
Display threshold and dynamic range can be bound to mouse movements,
thereby achieving the dynamic visualization effects shown here.

Arbitrary parameter tracks can be overlaid on spectrogram displays.  

Most waveform operations can also be invoked on multi-parameter
displays from ESPS object-oriented files.

Note that the cursors and markers remain synchronized in all windows
of a display ensemble.

A specialized attachment facility uses two-way UNIX pipes for
inter-process communication between waves+ and other programs.

Here we see a spectral analysis attachment that provides complete
parametric control of several analysis methods.

Another waves+ attachment provides a convenient interface for waveform
segment labeling.

ESPS and waves+ are complementary.  ESPS provides functions not
available in waves+, including filter design, filtering, pattern
classification, quantization, spectrum analysis, specialized plotting,
arbitrary image display, and many others.

Moreover, the ESPS programming environment facilitates the quick
development of new or modified signal processing programs.

Waves+ can be used to display and interact with the output of standard
or user-written ESPS programs.  Calls to these programs can be
associated with waves+ menu items.

Entropic software tools are built on an object-oriented file system.
They are extensible and flexible.  They are engineered for
productivity, with abstract interfaces, automatic record keeping, and
machine-independent file interchange.

Together, ESPS and waves+ provide rich and extensible tools that will
enhance your productivity today, ... and tomorrow.

COPYRIGHT NOTICE
================

The programs (except xloadimage and next_slide), data files, and text files
in this demo directory (and subdirectories) are the property of Entropic
Research Laboratory, Inc.  (Copyright (c) 1986-1990, Entropic Speech, Inc.
and Entropic Research Laboratory, Inc.; All rights reserved).  The demo may 
be freely distributed and used by anyone to evaluate Entropic Software 
products demonstrate the capabilities of workstations and windowing
software.

Xloadimage was originally written by Jim Frost (and others) Copyright (c) 1989,
1990. Next_slide was written by Rod Johnson Copyright (c) 1990 to make 
xloadimage useful in slide shows. Xloadimage and next_slide are in the 
public domain. If anyone wants copies of these, please contact us.

ACKNOWLEDEMENTS
===============

The demo command program, waves+ command files, and scripts were
written by John Shore.  Portions of the waves+ command files were
derived from a demo written originally by David Talkin at AT&T Bell
Laboratories, Murray Hill.  Thanks to David Burton, Rod Johnson, and
Alan Parker for their assistance.

---
(@(#)edemo.ab.mc	1.3 1/10/91)

