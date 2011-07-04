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
# Written by:  
# Checked by:
# Revised by:
# 
# Brief description: makefile for waves_init
# 

MENUS = $(WAVESDIR)/menus
COMMANDS = $(WAVESDIR)/commands

all:	waves_init

install: waves_init

# Note that we are installing Pxspecav and xspecav as well

waves_init:
	-rm -f $(WAVESDIR)/edit.help
	cp edit.help $(WAVESDIR)/edit.help
	chmod $(LIBMOD)  $(WAVESDIR)/edit.help
	-rm -f $(WAVESDIR)/record.help
	cp record.help $(WAVESDIR)/record.help
	chmod $(LIBMOD)  $(WAVESDIR)/record.help
	-rm -f $(WAVESDIR)/profile.help
	cp profile.help $(WAVESDIR)/profile.help
	chmod $(LIBMOD)  $(WAVESDIR)/profile.help
	-rm -f $(WAVESDIR)/toolbars.help
	cp toolbars.help $(WAVESDIR)/toolbars.help
	chmod $(LIBMOD)  $(WAVESDIR)/toolbars.help
	-rm -f $(WAVESDIR)/menus.help
	cp menus.help $(WAVESDIR)/menus.help
	chmod $(LIBMOD)  $(WAVESDIR)/menus.help
	-rm -f $(COMMANDS)/ezkeys.WC
	cp ezkeys.WC $(COMMANDS)/ezkeys.WC
	chmod $(LIBMOD) $(COMMANDS)/ezkeys.WC
	-rm -f $(COMMANDS)/xw_init.WC
	cp xw_init.WC $(COMMANDS)/xw_init.WC
	chmod $(LIBMOD)  $(COMMANDS)/xw_init.WC
	-rm -f $(COMMANDS)/defwavmen.WC
	cp defwavmen.WC $(COMMANDS)/defwavmen.WC
	chmod $(LIBMOD) $(COMMANDS)/defwavmen.WC
	-rm -f $(COMMANDS)/wavmen1.WC
	cp wavmen1.WC $(COMMANDS)/wavmen1.WC
	chmod $(LIBMOD) $(COMMANDS)/wavmen1.WC
	-rm -f $(COMMANDS)/wavmen2.WC
	cp wavmen2.WC $(COMMANDS)/wavmen2.WC
	chmod $(LIBMOD) $(COMMANDS)/wavmen2.WC
	-rm -f $(COMMANDS)/defimgmen.WC
	cp defimgmen.WC $(COMMANDS)/defimgmen.WC
	chmod $(LIBMOD) $(COMMANDS)/defimgmen.WC
	-rm -f $(COMMANDS)/imagmen1.WC
	cp imagmen1.WC $(COMMANDS)/imagmen1.WC
	chmod $(LIBMOD) $(COMMANDS)/imagmen1.WC
	-rm -f $(COMMANDS)/imagmen2.WC
	cp imagmen2.WC $(COMMANDS)/imagmen2.WC
	chmod $(LIBMOD) $(COMMANDS)/imagmen2.WC
	-rm -f $(COMMANDS)/rescale_off.WC
	cp rescale_off.WC $(COMMANDS)/rescale_off.WC
	chmod $(LIBMOD) $(COMMANDS)/rescale_off.WC
	-rm -f $(COMMANDS)/rescale_on.WC
	cp rescale_on.WC $(COMMANDS)/rescale_on.WC
	chmod $(LIBMOD) $(COMMANDS)/rescale_on.WC
	-rm -f $(COMMANDS)/plot3d.WC
	cp plot3d.WC $(COMMANDS)/plot3d.WC
	chmod $(LIBMOD) $(COMMANDS)/plot3d.WC
	-rm -f $(COMMANDS)/save_pro.WC
	cp save_pro.WC $(COMMANDS)/save_pro.WC
	chmod $(LIBMOD) $(COMMANDS)/save_pro.WC
	-rm -f $(COMMANDS)/cust_xw_init.WC
	cp cust_xw_init.WC $(COMMANDS)/cust_xw_init.WC
	chmod $(LIBMOD) $(COMMANDS)/cust_xw_init.WC
	-rm -f $(MENUS)/controls.WM
	cp controls.WM $(MENUS)/controls.WM
	chmod $(LIBMOD) $(MENUS)/controls.WM
	-rm -f $(MENUS)/colors.WM
	cp colors.WM $(MENUS)/colors.WM
	chmod $(LIBMOD) $(MENUS)/colors.WM
	-rm -f $(MENUS)/verbose.WM
	cp verbose.WM $(MENUS)/verbose.WM
	chmod $(LIBMOD) $(MENUS)/verbose.WM
	-rm -f $(MENUS)/debug.WM
	cp debug.WM $(MENUS)/debug.WM
	chmod $(LIBMOD) $(MENUS)/debug.WM
	-rm -f $(MENUS)/ganging.WM
	cp ganging.WM $(MENUS)/ganging.WM
	chmod $(LIBMOD) $(MENUS)/ganging.WM
	-rm -f $(MENUS)/annot.WM
	cp annot.WM $(MENUS)/annot.WM
	chmod $(LIBMOD) $(MENUS)/annot.WM
	-rm -f $(MENUS)/mbind.WM
	cp mbind.WM $(MENUS)/mbind.WM
	chmod $(LIBMOD) $(MENUS)/mbind.WM
	-rm -f $(MENUS)/menus.WM
	cp menus.WM $(MENUS)/menus.WM
	chmod $(LIBMOD) $(MENUS)/menus.WM
	-rm -f $(MENUS)/imageglbl.WM
	cp imageglbl.WM $(MENUS)/imageglbl.WM
	chmod $(LIBMOD) $(MENUS)/imageglbl.WM
	-rm -f $(MENUS)/globals.WM
	cp globals.WM $(MENUS)/globals.WM
	chmod $(LIBMOD) $(MENUS)/globals.WM
	-rm -f $(MENUS)/audio.WM
	cp audio.WM $(MENUS)/audio.WM
	chmod $(LIBMOD) $(MENUS)/audio.WM
	-rm -f $(MENUS)/profile.WM
	cp profile.WM $(MENUS)/profile.WM
	chmod $(LIBMOD) $(MENUS)/profile.WM
	-rm -f $(MENUS)/toolbars.WM
	cp toolbars.WM $(MENUS)/toolbars.WM
	chmod $(LIBMOD) $(MENUS)/toolbars.WM
	-rm -f $(BINDIR)/xspecav
	cp xspecav $(BINDIR)/xspecav
	chmod $(PROGMOD) $(BINDIR)/xspecav
	-rm -f $(PARAMDIR)/Pxspecav
	cp Pxspecav $(PARAMDIR)/Pxspecav
	chmod $(LIBMOD) $(PARAMDIR)/Pxspecav
	-rm -f $(BINDIR)/del_to_buf.sh
	cp del_to_buf.sh $(BINDIR)/del_to_buf.sh
	chmod $(PROGMOD) $(BINDIR)/del_to_buf.sh
	-rm -f $(BINDIR)/insert_buf.sh
	cp insert_buf.sh $(BINDIR)/insert_buf.sh
	chmod $(PROGMOD) $(BINDIR)/insert_buf.sh
	-rm -f $(BINDIR)/copy_to_buf.sh
	cp copy_to_buf.sh $(BINDIR)/copy_to_buf.sh
	chmod $(PROGMOD) $(BINDIR)/copy_to_buf.sh
	-rm -f $(BINDIR)/form_overlay.sh
	cp form_overlay.sh $(BINDIR)/form_overlay.sh
	chmod $(PROGMOD) $(BINDIR)/form_overlay.sh
	-rm -f $(BINDIR)/fftzoom
	cp fftzoom $(BINDIR)/fftzoom
	chmod $(PROGMOD) $(BINDIR)/fftzoom
	-rm -f $(BINDIR)/xframerun.sh
	cp xframerun.sh $(BINDIR)/xframerun.sh
	chmod $(PROGMOD) $(BINDIR)/xframerun.sh
	-rm -f $(BINDIR)/kill_wpanels.sh
	cp kill_wpanels.sh $(BINDIR)/kill_wpanels.sh
	chmod $(PROGMOD) $(BINDIR)/kill_wpanels.sh
	-rm -f $(BINDIR)/nvurecord.sh
	cp nvurecord.sh $(BINDIR)/nvurecord.sh
	chmod $(PROGMOD) $(BINDIR)/nvurecord.sh
	-rm -f $(BINDIR)/vurecord.sh
	cp vurecord.sh $(BINDIR)/vurecord.sh
	chmod $(PROGMOD) $(BINDIR)/vurecord.sh
	-rm -f $(BINDIR)/displprof.sh
	cp displprof.sh $(BINDIR)/displprof.sh
	chmod $(PROGMOD) $(BINDIR)/displprof.sh
	-rm -f $(BINDIR)/load_wpro.sh
	cp load_wpro.sh $(BINDIR)/load_wpro.sh
	chmod $(PROGMOD) $(BINDIR)/load_wpro.sh
	-rm -f $(BINDIR)/default_wpro.sh
	cp default_wpro.sh $(BINDIR)/default_wpro.sh
	chmod $(PROGMOD) $(BINDIR)/default_wpro.sh
	-rm -f $(BINDIR)/fix_wavepro.sh
	cp fix_wavepro.sh $(BINDIR)/fix_wavepro.sh
	chmod $(PROGMOD) $(BINDIR)/fix_wavepro.sh
	-rm -f $(BINDIR)/edit_bar.sh
	cp edit_bar.sh $(BINDIR)/edit_bar.sh
	chmod $(PROGMOD) $(BINDIR)/edit_bar.sh
	-rm -f $(BINDIR)/make_toolbar.sh
	cp make_toolbar.sh $(BINDIR)/make_toolbar.sh
	chmod $(PROGMOD) $(BINDIR)/make_toolbar.sh
	-rm -f $(BINDIR)/make_toolbar2.sh
	cp make_toolbar2.sh $(BINDIR)/make_toolbar2.sh
	chmod $(PROGMOD) $(BINDIR)/make_toolbar2.sh
	-rm -f $(BINDIR)/stats_bar.sh
	cp stats_bar.sh $(BINDIR)/stats_bar.sh
	chmod $(PROGMOD) $(BINDIR)/stats_bar.sh
	-rm -f $(BINDIR)/zoomboxin.sh
	cp zoomboxin.sh $(BINDIR)/zoomboxin.sh
	chmod $(PROGMOD) $(BINDIR)/zoomboxin.sh
	-rm -f $(BINDIR)/zoomboxout.sh
	cp zoomboxout.sh $(BINDIR)/zoomboxout.sh
	chmod $(PROGMOD) $(BINDIR)/zoomboxout.sh
	


clean:
	-rm -f *.o   $(OTHER_STUFF)


