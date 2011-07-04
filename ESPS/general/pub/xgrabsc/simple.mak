#=============================================================================
#
# Name - simple.mak
#
# Version:	1.5
#
# ccsid:	@(#)simple.mak	1.5 - 7/26/91 12:19:52
# from: 	ccs/s.simple.mak
# date: 	7/26/91 13:22:29
#
# Description: make file for xgrabsc.  Use "make -f simple.mak"
#
#=============================================================================

# CFLAGS = -g -DNO_RLE_CHECKS -DMEMCPY

# change INSTALL_PATH to the directory in which you want xgrabsc installed
INSTALL_PATH    = /usr/bin/X11

# change MAN_PATH to point to your man page top directory
MAN_PATH        = /usr/man
# change MAN_EXT to the section for xgrabsc
MAN_EXT         = 1

xgrabsc:: xgrabsc.o
	rm -f xgrabsc
	$(CC) $(CFLAGS) -o xgrabsc xgrabsc.o -lX11

xgrabsc.o:: xgrabsc.c ps_color.h patchlevel.h cpyright.h

install::
	install -c -s xgrabsc $(INSTALL_PATH)

install.man::
	install -c -m 644 xgrabsc.man \
		$(MAN_PATH)/man$(MAN_EXT)/xgrabsc.$(MAN_EXT)

clean::
	rm -f core *.o xgrabsc *.log

