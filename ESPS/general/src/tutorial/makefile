#  This material contains proprietary software of Entropic Research Lab, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Research Lab, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Research Lab, Inc. must bear the notice			
# 								
#      "Copyright (c) 1990 Entropic Research Lab, Inc.; All rights reserved"
# 				
#
# makefile for tutorial directory %W% %G% ERL



install: $(BINDIR)/initwtut $(SPSDIR)/demo/speech.sd $(BINDIR)/initetut \
	 $(BINDIR)/demoetut \
	 $(SPSDIR)/demo/speech.sgram $(SPSDIR)/demo/speech.ratios

$(BINDIR)/initwtut: initwtut
	rm -f $(BINDIR)/initwtut
	sed 's;\$$SPSDIR\$$;$(SPSDIR);' initwtut >$(BINDIR)/initwtut
	chmod $(PROGMOD) $(BINDIR)/initwtut

$(BINDIR)/initetut: initetut
	rm -f $(BINDIR)/initetut
	sed 's;\$$SPSDIR\$$;$(SPSDIR);' initetut >$(BINDIR)/initetut
	chmod $(PROGMOD) $(BINDIR)/initetut

$(BINDIR)/demoetut: demoetut
	rm -f $(BINDIR)/demoetut
	sed 's;\$$SPSDIR\$$;$(SPSDIR);' demoetut >$(BINDIR)/demoetut
	chmod $(PROGMOD) $(BINDIR)/demoetut

$(SPSDIR)/demo/speech.sd: speech.sd
	rm -f $(SPSDIR)/demo/speech.sd
	cp speech.sd $(SPSDIR)/demo
	chmod 0444 $(SPSDIR)/demo/speech.sd

$(SPSDIR)/demo/speech.sgram: speech.sgram
	rm -f $(SPSDIR)/demo/speech.sgram
	cp speech.sgram $(SPSDIR)/demo
	chmod $(LIBMOD) $(SPSDIR)/demo/speech.sd

$(SPSDIR)/demo/speech.ratios: speech.ratios
	rm -f $(SPSDIR)/demo/speech.ratios
	cp speech.ratios $(SPSDIR)/demo
	chmod $(LIBMOD) $(SPSDIR)/demo/speech.ratios

clean:
	@echo nothing here to clean
