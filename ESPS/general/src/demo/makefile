# This material contains unpublished, proprietary software of
# Entropic Research Laboratory, Inc. Any reproduction, distribution,
# or publication of this work must be authorized in writing by Entropic
# Research Laboratory, Inc., and must bear the notice:
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc.
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended
# publication of this source code.
#
# %W% %G% ERL
#
# Makefile for: demo directory
#
# Written by: Nelson
# Checked by:
# Revised by: Burton

DIR = $(SPSDIR)/demo


install: $(DIR) $(DIR)/filtdemo $(DIR)/mlplotdemo $(DIR)/pipedemo \
	$(DIR)/vqdemo $(DIR)/clean  $(DIR)/specdemo \
	$(DIR)/vqparams $(DIR)/README $(DIR)/lpcdemo $(DIR)/speech.asc \
	$(DIR)/lpcparams $(DIR)/syntparams  \
	$(DIR)/preemp_params $(DIR)/Xmakeplots  \
	$(DIR)/createfiles $(DIR)/refparams $(DIR)/frm.params \
	$(DIR)/setup_ratios $(DIR)/ratios $(DIR)/logratios $(DIR)/cepdemo \
	$(DIR)/syntdemo $(DIR)/showEnsemble
	
	cp speech12.sd speech.sd speech.ratios $(DIR)
	cp poet.sd poet.phones poet.words canyon.phones canyon.sd \
	canyon.words question.sd question.phones question.words $(DIR)
	chmod $(LIBMOD) $(DIR)/speech12.sd $(DIR)/speech.sd
	chmod $(LIBMOD) $(DIR)/poet.sd $(DIR)/poet.phones $(DIR)/poet.words \
	$(DIR)/canyon.phones $(DIR)/canyon.sd \
	$(DIR)/canyon.words $(DIR)/question.sd $(DIR)/question.phones \
	$(DIR)/question.words $(DIR)/speech.ratios

	rm -f $(DIR)/600lpfilt.params $(DIR)/1750bpfilt.params $(DIR)/3000hpfilt.params
	cp  600lpfilt.params 1750bpfilt.params 3000hpfilt.params $(DIR)
	chmod 0644 $(DIR)/600lpfilt.params
	chmod 0644 $(DIR)/1750bpfilt.params
	chmod 0644 $(DIR)/3000hpfilt.params

$(DIR):
	-mkdir $(DIR)

$(DIR)/showEnsemble: showEnsemble
	-rm -f $(DIR)/showEnsemble
	cp showEnsemble $(DIR)
	chmod $(PROGMOD) $(DIR)/showEnsemble

$(DIR)/filtdemo: filtdemo
	-rm -f $(DIR)/filtdemo
	cp filtdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/filtdemo

$(DIR)/syntdemo: syntdemo
	-rm -f $(DIR)/syntdemo
	cp syntdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/syntdemo

$(DIR)/cepdemo: cepdemo
	-rm -f $(DIR)/cepdemo
	cp cepdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/cepdemo

$(DIR)/mlplotdemo: mlplotdemo
	-rm -f $(DIR)/mlplotdemo
	cp mlplotdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/mlplotdemo

$(DIR)/pipedemo: pipedemo
	-rm -f $(DIR)/pipedemo
	cp pipedemo $(DIR)
	chmod $(PROGMOD) $(DIR)/pipedemo

$(DIR)/vqdemo: vqdemo
	-rm -f $(DIR)/vqdemo
	cp vqdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/vqdemo

$(DIR)/clean: clean
	-rm -f $(DIR)/clean
	cp clean $(DIR)
	chmod $(PROGMOD) $(DIR)/clean

$(DIR)/specdemo: specdemo
	-rm -f $(DIR)/specdemo
	cp specdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/specdemo

$(DIR)/vqparams: vqparams
	-rm -f $(DIR)/vqparams
	cp vqparams $(DIR)
	chmod $(MANMOD) $(DIR)/vqparams

$(DIR)/README: README
	-rm -f $(DIR)/README
	cp README $(DIR)
	chmod $(MANMOD) $(DIR)/README

$(DIR)/lpcdemo: lpcdemo
	-rm -f $(DIR)/lpcdemo
	cp lpcdemo $(DIR)
	chmod $(PROGMOD) $(DIR)/lpcdemo

$(DIR)/Xmakeplots: Xmakeplots
	-rm -f $(DIR)/Xmakeplots
	cp Xmakeplots $(DIR)
	chmod $(PROGMOD) $(DIR)/Xmakeplots

$(DIR)/Sunmakeplots: Sunmakeplots
	-rm -f $(DIR)/Sunmakeplots
	cp Sunmakeplots $(DIR)
	chmod $(PROGMOD) $(DIR)/Sunmakeplots

$(DIR)/createfiles: createfiles
	-rm -f $(DIR)/createfiles
	cp createfiles $(DIR)
	chmod $(PROGMOD) $(DIR)/createfiles

$(DIR)/setup_ratios: setup_ratios
	-rm -f $(DIR)/setup_ratios
	cp setup_ratios $(DIR)
	chmod $(PROGMOD) $(DIR)/setup_ratios

$(DIR)/ratios: ratios
	-rm -f $(DIR)/ratios
	cp ratios $(DIR)
	chmod $(PROGMOD) $(DIR)/ratios

$(DIR)/logratios: logratios
	-rm -f $(DIR)/logratios
	cp logratios $(DIR)
	chmod $(PROGMOD) $(DIR)/logratios

$(DIR)/speech.asc: speech.asc
	-rm -f $(DIR)/speech.asc
	cp speech.asc $(DIR)
	chmod $(MANMOD) $(DIR)/speech.asc

$(DIR)/lpcparams: lpcparams
	-rm -f $(DIR)/lpcparams
	cp lpcparams $(DIR)
	chmod $(MANMOD) $(DIR)/lpcparams

$(DIR)/refparams: refparams
	-rm -f $(DIR)/refparams
	cp refparams $(DIR)
	chmod $(MANMOD) $(DIR)/refparams

$(DIR)/syntparams: syntparams
	-rm -f $(DIR)/syntparams
	cp syntparams $(DIR)
	chmod $(MANMOD) $(DIR)/syntparams

$(DIR)/preemp_params: preemp_params
	-rm -f $(DIR)/preemp_params
	cp preemp_params $(DIR)
	chmod $(MANMOD) $(DIR)/preemp_params

$(DIR)/frm.params: frm.params
	-rm -f $(DIR)/frm.params
	cp frm.params $(DIR)
	chmod $(MANMOD) $(DIR)/frm.params

$(DIR)/specparams: specparams
	-rm -f $(DIR)/specparams
	cp specparams $(DIR)
	chmod $(MANMOD) $(DIR)/specparams



