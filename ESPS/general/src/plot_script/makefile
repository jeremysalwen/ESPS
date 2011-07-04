
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)makefile	3.8	6/14/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  plot_script
# 


# ESPS makefile for plotting programs


GETBINDIR  = \`get_esps_base\`/bin
GETPLOTDIR = \`get_esps_base\`/`basename $(PLOTDIR)`
SCRIPTS =  plotsd plotspec aplot scatplot genplot mlplot

all : $(SCRIPTS)

install : all
	-rm -f $(BINDIR)/plotsd
	cp plotsd $(BINDIR)/plotsd
	chmod $(PROGMOD) $(BINDIR)/plotsd
	-diffmk $(MANDIR)/man1/plotsd.1 plotsd.1 plotsd.diff
	@echo diffmked man page left in plotsd.diff
	-rm $(MANDIR)/man1/plotsd.1
	cp plotsd.1 $(MANDIR)/man1/plotsd.1
	chmod $(MANMOD) $(MANDIR)/man1/plotsd.1

	-rm -f $(BINDIR)/plotspec
	cp plotspec $(BINDIR)/plotspec
	chmod $(PROGMOD) $(BINDIR)/plotspec
	-diffmk $(MANDIR)/man1/plotspec.1 plotspec.1 plotspec.diff
	@echo diffmked man page left in plotspec.diff
	-rm $(MANDIR)/man1/plotspec.1
	cp plotspec.1 $(MANDIR)/man1/plotspec.1
	chmod $(MANMOD) $(MANDIR)/man1/plotspec.1

	-rm -f $(BINDIR)/aplot
	cp aplot $(BINDIR)/aplot
	chmod $(PROGMOD) $(BINDIR)/aplot
	-diffmk $(MANDIR)/man1/aplot.1 aplot.1 aplot.diff
	@echo diffmked man page left in aplot.diff
	-rm $(MANDIR)/man1/aplot.1
	cp aplot.1 $(MANDIR)/man1/aplot.1
	chmod $(MANMOD) $(MANDIR)/man1/aplot.1

	-rm -f $(BINDIR)/scatplot
	cp scatplot $(BINDIR)/scatplot
	chmod $(PROGMOD) $(BINDIR)/scatplot
	-diffmk $(MANDIR)/man1/scatplot.1 scatplot.1 scatplot.diff
	@echo diffmked man page left in scatplot.diff
	-rm $(MANDIR)/man1/scatplot.1
	cp scatplot.1 $(MANDIR)/man1/scatplot.1
	chmod $(MANMOD) $(MANDIR)/man1/scatplot.1

	-rm -f $(BINDIR)/genplot
	cp genplot $(BINDIR)/genplot
	chmod $(PROGMOD) $(BINDIR)/genplot
	-diffmk $(MANDIR)/man1/genplot.1 genplot.1 genplot.diff
	@echo diffmked man page left in genplot.diff
	-rm $(MANDIR)/man1/genplot.1
	cp genplot.1 $(MANDIR)/man1/genplot.1
	chmod $(MANMOD) $(MANDIR)/man1/genplot.1

	rm -f $(BINDIR)/mlplot
	cp mlplot $(BINDIR)/mlplot
	chmod $(PROGMOD) $(BINDIR)/mlplot
	-diffmk $(MANDIR)/man1/mlplot.1 mlplot.1 mlplot.diff
	@echo diffmked man page left in mlplot.diff
	-rm $(MANDIR)/man1/mlplot.1
	cp mlplot.1 $(MANDIR)/man1/mlplot.1
	chmod $(MANMOD) $(MANDIR)/man1/mlplot.1


plotsd: plot_script
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script >/tmp/plotsd
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/plotsd >/tmp/plotsd2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/plotsd2 >/tmp/plotsd
	sed 's;\$$MCD\$$;$(MCD);' /tmp/plotsd > /tmp/plotsd2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/plotsd2 > plotsd
	rm /tmp/plotsd /tmp/plotsd2
	chmod $(PROGMOD) plotsd


plotspec: plot_script
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script >/tmp/plotspec
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/plotspec >/tmp/plotspec2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/plotspec2 >/tmp/plotspec
	sed 's;\$$MCD\$$;$(MCD);' /tmp/plotspec > /tmp/plotspec2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/plotspec2 > plotspec
	rm /tmp/plotspec /tmp/plotspec2
	chmod $(PROGMOD) plotspec

aplot: plot_script
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script >/tmp/aplot
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/aplot >/tmp/aplot2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/aplot2 >/tmp/aplot
	sed 's;\$$MCD\$$;$(MCD);' /tmp/aplot > /tmp/aplot2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/aplot2 > aplot
	rm /tmp/aplot /tmp/aplot2
	chmod $(PROGMOD) aplot


scatplot: plot_script3
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script3 >/tmp/scatplot
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/scatplot >/tmp/scatplot2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/scatplot2 >/tmp/scatplot
	sed 's;\$$MCD\$$;$(MCD);' /tmp/scatplot > /tmp/scatplot2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/scatplot2 > scatplot
	rm /tmp/scatplot /tmp/scatplot2
	chmod $(PROGMOD) scatplot


genplot: plot_script2
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script2 >/tmp/genplot
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/genplot >/tmp/genplot2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/genplot2 >/tmp/genplot
	sed 's;\$$MCD\$$;$(MCD);' /tmp/genplot > /tmp/genplot2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/genplot2 > genplot
	rm /tmp/genplot /tmp/genplot2
	chmod $(PROGMOD) genplot


mlplot: plot_script2
	sed 's;\$$PLOTDIR\$$;$(GETPLOTDIR);' plot_script2 >/tmp/mlplot
	sed 's;\$$PLOTCOMMAND\$$;$(PLOTCOMMAND);' /tmp/mlplot >/tmp/mlplot2
	sed 's;\$$XWIN\$$;$(XWIN);' /tmp/mlplot2 >/tmp/mlplot
	sed 's;\$$MCD\$$;$(MCD);' /tmp/mlplot > /tmp/mlplot2
	sed 's;\$$SPSBIN\$$;$(GETBINDIR);' /tmp/mlplot2 > mlplot
	rm /tmp/mlplot /tmp/mlplot2
	chmod $(PROGMOD) mlplot

clean:
	-rm $(SCRIPTS)

lint:
	@echo Nothing here to lint

test: test.plotsd test.plotspec test.aplot \
		test.scatplot \
		test.mlplot test.genplot

test.plotsd:
	-sh plotsd.tes ./

test.plotspec:
	-sh plotspec.tes ./

test.aplot:
	-sh aplot.tes ./

test.scatplot:
	-sh scatplot.tes ./

test.mlplot:
	@echo "Mlplot tested by test of imlplot."

test.genplot:
	@echo "Genplot tested by test of igenplot."


