# **********************************************************
# Project:		ARG General libraries
# SubTree:		/home/projects/devel_source/general/src/lib_header/sphere2.6/src/lib/ sp
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
LIBRARY_ID = sp
LIBRARY	= lib$(LIBRARY_ID).a
# Source files for LIBRARY
SRC	= alaw.c array.c dif_wavf.c dupfinfo.c exit.c fixio.c h_fix.c license.c lpc.c nat_bord.c poly.c sdatamod.c shorten.c shpack_s.c shpacki2.c shpackio.c sp_ntoh.c sp_utils.c spalloc.c spchksum.c spclose.c speof.c sperror.c sphere.c sphio.c spinput.c sploadf.c spopen.c spoutput.c spread.c sprewind.c sprstat.c spseek.c sptell.c sptemp.c sputils2.c spvers.c spwrite.c stdfield.c ulaw.c vario.c verbose.c wavedit.c wavpack.c 

# Local additions for CFLAG options (eg: -g)
LOCAL_CFLAGS	=  -DNARCH_SUN

# *******************************************************
# THE REST OF THE MAKEFILE SHOULD NOT NEED TO BE MODIFIED
#       (EXCEPT UPON APPROVAL OF PROJECT MANAGER)
# *******************************************************

INCLUDE = $(PROJECT_ROOT)/include
OBJ	= alaw.o array.o dif_wavf.o dupfinfo.o exit.o fixio.o h_fix.o license.o lpc.o nat_bord.o poly.o sdatamod.o shorten.o shpack_s.o shpacki2.o shpackio.o sp_ntoh.o sp_utils.o spalloc.o spchksum.o spclose.o speof.o sperror.o sphere.o sphio.o spinput.o sploadf.o spopen.o spoutput.o spread.o sprewind.o sprstat.o spseek.o sptell.o sptemp.o sputils2.o spvers.o spwrite.o stdfield.o ulaw.o vario.o verbose.o wavedit.o wavpack.o 
# Enable converting the distribution to K&R C
#KRC     = $(SRC:%.c=%.kr.c)
LIBRY	= $(LIBRARY)

CFLAGS	= -I$(INCLUDE) $(PROGCFLAGS) $(LOCAL_CFLAGS)
#CFLAGS	= -I$(INCLUDE) $(LOCAL_CFLAGS)
override CC	= gcc -ansi $(CFLAGS)

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
alaw.o: alaw.c
array.o: array.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
dif_wavf.o: dif_wavf.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
dupfinfo.o: dupfinfo.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
exit.o: exit.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h /usr/include/bits/sigset.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h
fixio.o: fixio.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h \
  bitshift.h
h_fix.o: h_fix.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
  /usr/include/sys/stat.h /usr/include/bits/stat.h
license.o: license.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
lpc.o: lpc.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
nat_bord.o: nat_bord.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
poly.o: poly.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
sdatamod.o: sdatamod.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
shorten.o: shorten.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h /usr/include/stdlib.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h /usr/include/bits/sigset.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
shpack_s.o: shpack_s.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shpack/shpk_sph.h \
  /usr/include/netinet/in.h /usr/include/stdint.h \
  /usr/include/bits/wchar.h /usr/include/sys/socket.h \
  /usr/include/sys/uio.h /usr/include/sys/types.h /usr/include/bits/uio.h \
  /usr/include/bits/socket.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/bits/in.h /usr/include/bits/byteswap.h
shpacki2.o: shpacki2.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/sys/types.h /usr/include/time.h /usr/include/netinet/in.h \
  /usr/include/stdint.h /usr/include/bits/wchar.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/bits/in.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/bits/byteswap.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/string.h /usr/include/errno.h \
  /usr/include/bits/errno.h /usr/include/linux/errno.h \
  /usr/include/asm/errno.h /usr/include/asm-generic/errno.h \
  /usr/include/asm-generic/errno-base.h /usr/include/stdlib.h \
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
  /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  /usr/include/bits/sigset.h
shpackio.o: shpackio.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/sys/types.h /usr/include/time.h /usr/include/netinet/in.h \
  /usr/include/stdint.h /usr/include/bits/wchar.h \
  /usr/include/sys/socket.h /usr/include/sys/uio.h \
  /usr/include/bits/uio.h /usr/include/bits/socket.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h /usr/include/asm/sockios.h \
  /usr/include/bits/in.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/bits/byteswap.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h /usr/include/fcntl.h /usr/include/bits/fcntl.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/ctype.h /usr/include/string.h /usr/include/errno.h \
  /usr/include/bits/errno.h /usr/include/linux/errno.h \
  /usr/include/asm/errno.h /usr/include/asm-generic/errno.h \
  /usr/include/asm-generic/errno-base.h /usr/include/stdlib.h \
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
  /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  /usr/include/bits/sigset.h
sp_ntoh.o: sp_ntoh.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/string.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
sp_utils.o: sp_utils.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/ctype.h \
  /usr/include/endian.h /usr/include/bits/endian.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
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
spalloc.o: spalloc.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spchksum.o: spchksum.c /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
  /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h \
  /usr/include/gnu/stubs-32.h /usr/include/bits/types.h \
  /usr/include/bits/typesizes.h /usr/include/libio.h \
  /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spclose.o: spclose.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
  /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  /usr/include/bits/sigset.h
speof.o: speof.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
sperror.o: sperror.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
sphere.o: sphere.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  /usr/include/bits/sigset.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shrt_sph.h
sphio.o: sphio.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/string.h /usr/include/errno.h /usr/include/bits/errno.h \
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
spinput.o: spinput.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/ctype.h \
  /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/sys/types.h /usr/include/time.h \
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
sploadf.o: sploadf.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spopen.o: spopen.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spoutput.o: spoutput.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/ctype.h /usr/include/endian.h \
  /usr/include/bits/endian.h /usr/include/string.h \
  /usr/include/sys/types.h /usr/include/time.h \
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
spread.o: spread.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/ulaw.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/alaw.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h \
  /usr/include/setjmp.h /usr/include/bits/setjmp.h \
  /usr/include/bits/sigset.h
sprewind.o: sprewind.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
sprstat.o: sprstat.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spseek.o: spseek.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
sptell.o: sptell.c \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
sptemp.o: sptemp.c /usr/include/stdio.h /usr/include/features.h \
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
sputils2.o: sputils2.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/ctype.h \
  /usr/include/endian.h /usr/include/bits/endian.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
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
spvers.o: spvers.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
spwrite.o: spwrite.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/ulaw.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/alaw.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
stdfield.o: stdfield.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
ulaw.o: ulaw.c
vario.o: vario.c /usr/include/math.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
  /usr/include/bits/mathcalls.h /usr/include/stdio.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/fob.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/shorten/shorten.h
verbose.o: verbose.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
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
wavedit.o: wavedit.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/param.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/limits.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/syslimits.h \
  /usr/include/limits.h /usr/include/linux/limits.h \
  /usr/include/linux/param.h /usr/include/asm/param.h \
  /usr/include/sys/types.h /usr/include/time.h /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/sphere.h \
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/util/utillib.h \
  /usr/include/math.h /usr/include/bits/huge_val.h \
  /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
  /usr/include/ctype.h /usr/include/endian.h /usr/include/bits/endian.h \
  /usr/include/errno.h /usr/include/bits/errno.h \
  /usr/include/linux/errno.h /usr/include/asm/errno.h \
  /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
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
  /home/stepheni/ESPS/general/src/lib_header/sphere2.6/include/sp/version.h
wavpack.o: wavpack.c /usr/include/stdio.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h \
  /usr/include/gnu/stubs.h /usr/include/gnu/stubs-32.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stddef.h \
  /usr/include/bits/types.h /usr/include/bits/typesizes.h \
  /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h \
  /usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h \
  /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h \
  /usr/include/stdlib.h /usr/include/string.h /usr/include/setjmp.h \
  /usr/include/bits/setjmp.h /usr/include/bits/sigset.h \
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
