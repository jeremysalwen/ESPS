# **********************************************************
# Project:		ARG General libraries
# SubTree:		/home/projects/devel_source/general/src/lib_header/sphere2.6/src/progs/ h_add h_delete h_edit h_read h_strip h_test tsphere w_decode w_diff w_edit w_encode 
# Filename:		Makefile
# Programmer:		J. Fiscus
# Organization:		NIST/NCSL/DIV 670/Auto Rec. Group
# Host System:		SUN 4 OS/4.1.1 (UNIX)
# Date Created:		05/28/92
#
# **********************************************************
# Makefile for SRC Binary Directory
# **********************************************************

PROJECT_ROOT	= $(PROJDIR)
SHELL = /bin/sh

# **************************************************
# INDIVIDUAL PROGRAM MODIFICATIONS SHOULD BEGIN HERE
# **************************************************

# Program to make (binary name)
EXECUTABLE	= h_add h_delete h_edit h_read h_strip h_test tsphere w_decode w_diff w_edit w_encode 
# Supporting functions to make if any (They will be included in every compile)
OBJ		= 
# List all .c source code files
SRC		= h_add.c h_delete.c h_edit.c h_read.c h_strip.c h_test.c tsphere.c w_decode.c w_diff.c w_edit.c w_encode.c 
# Enable converting the distribution to K&R C
# KRC             = $(SRC:%.c=%.kr.c)

# LIBS needed to compile the EXECUTABLE
# (Full paths eg: /usr/local/image/lib/libimage.a)
# use $(LIBDIR) defined above for path extensions if appropriate
LIBS	= $(PROJECT_ROOT)/lib
# How the libraries look when invoked on the compile line (eg: -limage)
LLIBS	= -lsp -lutil  -lm
# Local additions for CFLAG options (eg: -g)
LOCAL_CFLAGS	= -g  -DNARCH_SUN

# *******************************************************
# THE REST OF THE MAKEFILE SHOULD NOT NEED TO BE MODIFIED
#       (EXCEPT UPON APPROVAL OF PROJECT MANAGER)
# *******************************************************

BIN	= $(PROJECT_ROOT)/bin
INCLUDE = $(PROJECT_ROOT)/include
LIBDIR	= $(LIBS)

CFLAGS	= -I$(INCLUDE) -L$(LIBDIR) $(LOCAL_CFLAGS)
CC	= gcc -ansi 
INSTALL	= install -c -s -m 755

MAKEFILE	= Makefile
.PRECIOUS: $(MAKEFILE)

#.c: $(BIN)/$@ stub.o
#	$(CC) $@.c $(FUNCT_O) $(LLIBS) -o $@
#
#.c.o:
#	cc -c $@.c

it: $(EXECUTABLE)

install: $(MAKEFILE) $(EXECUTABLE)
	$(INSTALL) $(EXECUTABLE) $(BIN)

#build_kr: $(KRC)
#$(KRC): $@
#	cp `echo $@ | sed 's/.kr//'` $@
#	unprotoize -c "-I$(INCLUDE)" $@
#	rm -f $@.save

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) $@.c $(OBJ) $(LLIBS) -o $@

# if there are other separate programs to compile, add the name to
#	SRC, OBJ, ... and the redo the last 3 lines EXPLICITLY
#	for each program.
#	Install will also have to be changed.

clean :
	rm -f *.o $(EXECUTABLE) core a.out *.BAK

bare: clean
	rm -f $(EXECUTABLE)

$(MAKEFILE): $(SRC)
	$(CC) $(CFLAGS) -M $(SRC) > dependlist
	@sed -e '1,/^# DO NOT DELETE/!d' $(MAKEFILE) > $(MAKEFILE).tmp.$$$$; \
	cat dependlist >> $(MAKEFILE).tmp.$$$$; \
	cp $(MAKEFILE) $(MAKEFILE).BAK; \
	mv $(MAKEFILE).tmp.$$$$ $(MAKEFILE); \
	rm -f dependlist;

depend: $(SRC)
	$(CC) $(CFLAGS) -M $(SRC) > dependlist
	@sed -e '1,/^# DO NOT DELETE/!d' $(MAKEFILE) > $(MAKEFILE).tmp.$$$$; \
	cat dependlist >> $(MAKEFILE).tmp.$$$$; \
	cp $(MAKEFILE) $(MAKEFILE).BAK; \
	mv $(MAKEFILE).tmp.$$$$ $(MAKEFILE); \
	rm -f dependlist; \
	echo make depend complete

# DO NOT DELETE THIS LINE - make depend uses it
h_add.o: h_add.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
h_delete.o: h_delete.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/types.h \
  /usr/include/time.h /usr/include/sys/stat.h /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
h_edit.o: h_edit.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/types.h \
  /usr/include/time.h /usr/include/sys/stat.h /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
h_read.o: h_read.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/ctype.h /usr/include/endian.h \
  /usr/include/bits/endian.h /usr/include/string.h \
  /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
h_strip.o: h_strip.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
h_test.o: h_test.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
tsphere.o: tsphere.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/string.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h \
  /usr/include/netinet/in.h /usr/include/stdint.h \
  /usr/include/bits/wchar.h /usr/include/sys/socket.h \
  /usr/include/sys/uio.h /usr/include/sys/types.h /usr/include/bits/uio.h \
  /usr/include/bits/socket.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/bits/in.h /usr/include/bits/byteswap.h
w_decode.o: w_decode.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
w_diff.o: w_diff.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
w_edit.o: w_edit.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
w_encode.o: w_encode.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/time.h /usr/include/bits/time.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/rpg.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/header.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysparam.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spchksum.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/spfile.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sysfunc.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sp.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/wavpack/wavpack.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
