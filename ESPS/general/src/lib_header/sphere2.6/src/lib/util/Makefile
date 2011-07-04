# **********************************************************
# Project:		ARG General libraries
# SubTree:		/home/projects/devel_source/general/src/lib_header/sphere2.6/src/lib/ util
# Filename:		Makefile
# Programmer:		J. Fiscus
# Organization:		NIST/NCSL/DIV 670/Auto Rec. Group
# Host System:		SUN 4 OS/4.1.1 (UNIX)
# Date Created:		05/28/92
# Project:		NIST LIBRARIES
# Revision history:
#
# Apr 04, 1994
#	- fixes the make 'depend' and 'makefile' commands to not fail
# 	  on the SGI
#
# **********************************************************
# Makefile for SRC Library Directory
# **********************************************************
SHELL = /bin/sh

# PROJDIR Passed On Command Line ONLY
PROJECT_ROOT	= $(PROJDIR)
LIBDIR		= $(PROJECT_ROOT)/lib
DOCDIR		= $(PROJECT_ROOT)/doc

RANLIB		= ranlib
AR		= ar ru

# **************************************************
# INDIVIDUAL PROGRAM MODIFICATIONS SHOULD BEGIN HERE
# **************************************************

# Library to make (library name)
LIBRARY_ID = util
LIBRARY	= lib$(LIBRARY_ID).a
# Source files for LIBRARY
SRC	= cfg.c comments.c ds.c err_util.c fobops.c hist.c hsgetopt.c itoa.c maldie.c mtrf.c order.c pad.c rsprintf.c sfgets.c statdist.c str_srch.c string.c t_or_f.c word.c 

# Local additions for CFLAG options (eg: -g)
LOCAL_CFLAGS	=  -DNARCH_SUN

# *******************************************************
# THE REST OF THE MAKEFILE SHOULD NOT NEED TO BE MODIFIED
#       (EXCEPT UPON APPROVAL OF PROJECT MANAGER)
# *******************************************************

INCLUDE = $(PROJECT_ROOT)/include
OBJ	= cfg.o comments.o ds.o err_util.o fobops.o hist.o hsgetopt.o itoa.o maldie.o mtrf.o order.o pad.o rsprintf.o sfgets.o statdist.o str_srch.o string.o t_or_f.o word.o 
# Enable converting the distribution to K&R C
#KRC     = $(SRC:%.c=%.kr.c)
LIBRY	= $(LIBRARY)

CFLAGS	= -I$(INCLUDE) $(LOCAL_CFLAGS)
CC	= gcc -ansi $(CFLAGS)

MAKEFILE	= Makefile
.PRECIOUS: $(MAKEFILE)

it: $(LIBRY)

install: $(MAKEFILE) $(LIBDIR)/$(LIBRARY)
$(LIBDIR)/$(LIBRARY): $(LIBRY)
	mv $(LIBRY) $(LIBDIR)
	chmod 664 $(LIBDIR)/$(LIBRARY)
	$(RANLIB) $(LIBDIR)/$(LIBRARY)

$(LIBRY): $(OBJ)
	$(AR) $(LIBRY) $?
	$(RANLIB) $(LIBRY)

#build_kr: $(KRC)
#$(KRC): $@
#	cp `echo $@ | sed 's/.kr//'` $@
#	unprotoize -c "-I$(INCLUDE)" $@
#	rm -f $@.save

%.o: %.c
	$(CC) -c -o $@ $<

clean :
	rm -f $(OBJ) $(LIBRARY) $(LIBRARY_ID).doc *.BAK dependlist;

bare: clean
	rm -f $(LIBRY)

depend $(MAKEFILE): $(SRC)
	$(CC) -M $(SRC) > dependlist
	@sed -e '1,/^# DO NOT DELETE/!d' $(MAKEFILE) > $(MAKEFILE).tmp.$$$$; \
	cat dependlist >> $(MAKEFILE).tmp.$$$$; \
	cp $(MAKEFILE) $(MAKEFILE).BAK; \
	mv $(MAKEFILE).tmp.$$$$ $(MAKEFILE); \
	rm -f dependlist \
	echo make depend complete

document:
	$(DOCUMENT) -h $(SRC) > $(LIBRARY_ID).doc
	echo "" >> $(LIBRARY_ID).doc
	echo "Procedure Definitions" >> $(LIBRARY_ID).doc
	echo "" >> $(LIBRARY_ID).doc
	$(DOCUMENT) -t $(SRC) >> $(LIBRARY_ID).doc




# DO NOT DELETE THIS LINE - make depend uses it
cfg.o: cfg.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
comments.o: comments.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
ds.o: ds.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
err_util.o: err_util.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
fobops.o: fobops.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
hist.o: hist.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
hsgetopt.o: hsgetopt.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
itoa.o: itoa.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
maldie.o: maldie.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
mtrf.o: mtrf.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
order.o: order.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
pad.o: pad.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
rsprintf.o: rsprintf.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
sfgets.o: sfgets.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
statdist.o: statdist.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
str_srch.o: str_srch.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
string.o: string.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
t_or_f.o: t_or_f.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
word.o: word.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/stdlib.h /usr/include/time.h \
  /usr/include/bits/time.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/chars.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/memory.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/mtrf.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/macros.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/min.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/order.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/cfg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/proto.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/statdist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/diskarr.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/err_util.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/hsgetopt.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h
