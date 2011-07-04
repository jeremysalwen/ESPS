#
# top level makefile for waves+
#
# usually called by the ESPS install script
#
# %W% %G% ESI
#

install: clean xwaves

xwaves:
	(cd sigproc; emake install)
	(cd libsig; emake install)
	(cd waves/src/c; emake  all install)
	(cd formant/src; emake install)
	(cd cmap; emake install)


clean:	
	(cd sigproc; emake clean)
	(cd libsig; emake clean)
	(cd waves/src/c; emake clobber)
	(cd waves/text; emake clean)
	(cd formant/src; emake clean)
	(cd cmap; emake clean)


