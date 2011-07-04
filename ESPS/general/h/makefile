#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for general h files @(#)makefile	1.32 5/1/98 ESI
 	
SINC   = /usr/include/esps
HMOD = 0440

FILES = anafea.h constants.h esps.h \
	fea.h feastat.h filt.h ftypes.h \
	header.h param.h  \
	scbk.h sd.h spec.h spsassert.h \
	vq.h limits.h unix.h window.h feaspec.h locks.h feasd.h \
	mach_codes.h xecolors.h epi.xicon \
	esignal_fea.h pc_wav.h \
	exview.h exv_func.h exv_keys.h exv_icon.h exv_colors.h \
	erlicon_b.h erlicon_nb.h p3dicon.h sinicon.h histicon.h \
	imagicon.h  specicon.h exv_bbox.h exv_olwm.h \
	olwm.h mem.h menu.h feadst.h ana_methods.h ss.h feafilt.h \
	epaths.h func.h ultrix_cpt.h xwaves_ipc.h op.h audio.h \
	alaw.h array.h complex.h fftcepstrum.h get_fft.h linalg.h \
	mulaw.h rand.h range_switch.h strlist.h types.h lpsyn.h


install: $(SINC)/anafea.h $(SINC)/constants.h $(SINC)/esps.h \
	$(SINC)/fea.h $(SINC)/feastat.h $(SINC)/filt.h $(SINC)/ftypes.h \
	$(SINC)/header.h $(SINC)/param.h  \
	$(SINC)/scbk.h $(SINC)/sd.h $(SINC)/spec.h $(SINC)/spsassert.h \
	$(SINC)/vq.h $(SINC)/limits.h $(SINC)/unix.h $(SINC)/window.h \
	$(SINC)/feaspec.h $(SINC)/locks.h $(SINC)/feasd.h $(SINC)/mach_codes.h \
	$(SINC)/xecolors.h $(SINC)/epi.xicon \
	$(SINC)/esignal_fea.h $(SINC)/pc_wav.h \
	$(SINC)/exview.h $(SINC)/exv_func.h $(SINC)/exv_keys.h \
	$(SINC)/exv_icon.h $(SINC)/exv_colors.h \
	$(SINC)/erlicon_b.h $(SINC)/erlicon_nb.h \
	$(SINC)/p3dicon.h $(SINC)/sinicon.h \
	$(SINC)/histicon.h $(SINC)/imagicon.h \
	$(SINC)/specicon.h $(SINC)/exv_bbox.h $(SINC)/exv_olwm.h \
	$(SINC)/olwm.h $(SINC)/mem.h $(SINC)/menu.h $(SINC)/feadst.h \
	$(SINC)/ana_methods.h $(SINC)/ss.h $(SINC)/feafilt.h \
	$(SINC)/epaths.h $(SINC)/func.h $(SINC)/ultrix_cpt.h \
	$(SINC)/xwaves_ipc.h $(SINC)/lpsyn.h $(SINC)/op.h $(SINC)/audio.h \
	$(SINC)/alaw.h $(SINC)/array.h $(SINC)/complex.h \
	$(SINC)/fftcepstrum.h $(SINC)/get_fft.h $(SINC)/linalg.h \
	$(SINC)/mulaw.h $(SINC)/rand.h $(SINC)/range_switch.h \
	$(SINC)/strlist.h $(SINC)/types.h

$(SINC)/anafea.h: anafea.h
	-rm -f $(SINC)/anafea.h
	cp anafea.h $(SINC)
	chmod $(HMOD) $(SINC)/anafea.h

$(SINC)/constants.h: constants.h
	-rm -f $(SINC)/constants.h
	cp constants.h $(SINC)
	chmod $(HMOD) $(SINC)/constants.h

$(SINC)/esps.h:  $(FILES)
	-rm -f $(SINC)/esps.h
	cp esps.h $(SINC)
	chmod $(HMOD) $(SINC)/esps.h

$(SINC)/fea.h: fea.h
	-rm -f $(SINC)/fea.h
	cp fea.h $(SINC)
	chmod $(HMOD) $(SINC)/fea.h

$(SINC)/feastat.h: feastat.h
	-rm -f $(SINC)/feastat.h
	cp feastat.h $(SINC)
	chmod $(HMOD) $(SINC)/feastat.h

$(SINC)/filt.h: filt.h
	-rm -f $(SINC)/filt.h
	cp filt.h $(SINC)
	chmod $(HMOD) $(SINC)/filt.h

$(SINC)/ftypes.h: ftypes.h
	-rm -f $(SINC)/ftypes.h
	cp ftypes.h $(SINC)
	chmod $(HMOD) $(SINC)/ftypes.h

$(SINC)/header.h: header.h
	-rm -f $(SINC)/header.h
	cp header.h $(SINC)
	chmod $(HMOD) $(SINC)/header.h

$(SINC)/param.h: param.h
	-rm -f $(SINC)/param.h
	cp param.h $(SINC)
	chmod $(HMOD) $(SINC)/param.h

$(SINC)/scbk.h: scbk.h
	-rm -f $(SINC)/scbk.h
	cp scbk.h $(SINC)
	chmod $(HMOD) $(SINC)/scbk.h

$(SINC)/sd.h: sd.h
	-rm -f $(SINC)/sd.h
	cp sd.h $(SINC)
	chmod $(HMOD) $(SINC)/sd.h

$(SINC)/spec.h: spec.h
	-rm -f $(SINC)/spec.h
	cp spec.h $(SINC)
	chmod $(HMOD) $(SINC)/spec.h
	
$(SINC)/spsassert.h: spsassert.h
	-rm -f $(SINC)/spsassert.h
	cp spsassert.h $(SINC)
	chmod $(HMOD) $(SINC)/spsassert.h
	
$(SINC)/vq.h: vq.h
	-rm -f $(SINC)/vq.h
	cp vq.h $(SINC)
	chmod $(HMOD) $(SINC)/vq.h
	
$(SINC)/limits.h: limits.h
	-rm -f $(SINC)/limits.h
	cp limits.h $(SINC)
	chmod $(HMOD) $(SINC)/limits.h
	
$(SINC)/unix.h: unix.h
	-rm -f $(SINC)/unix.h
	cp unix.h $(SINC)
	chmod $(HMOD) $(SINC)/unix.h

$(SINC)/window.h: window.h
	-rm -f $(SINC)/window.h
	cp window.h $(SINC)
	chmod $(HMOD) $(SINC)/window.h

$(SINC)/feaspec.h: feaspec.h
	-rm -f $(SINC)/feaspec.h
	cp feaspec.h $(SINC)
	chmod $(HMOD) $(SINC)/feaspec.h

$(SINC)/locks.h: locks.h
	-rm -f $(SINC)/locks.h
	cp locks.h $(SINC)
	chmod $(HMOD) $(SINC)/locks.h

$(SINC)/feasd.h: feasd.h
	-rm -f $(SINC)/feasd.h
	cp feasd.h $(SINC)
	chmod $(HMOD) $(SINC)/feasd.h

$(SINC)/mach_codes.h: mach_codes.h
	-rm -f $(SINC)/mach_codes.h
	cp mach_codes.h $(SINC)
	chmod $(HMOD) $(SINC)/mach_codes.h

$(SINC)/xecolors.h: xecolors.h
	-rm -f $(SINC)/xecolors.h
	cp xecolors.h $(SINC)
	chmod $(HMOD) $(SINC)/xecolors.h

$(SINC)/epi.xicon: epi.xicon
	-rm -f $(SINC)/epi.xicon
	cp epi.xicon $(SINC)
	chmod $(HMOD) $(SINC)/epi.xicon

$(SINC)/esignal_fea.h: esignal_fea.h
	-rm -f $(SINC)/esignal_fea.h
	cp esignal_fea.h $(SINC)
	chmod $(HMOD) $(SINC)/esignal_fea.h

$(SINC)/pc_wav.h: pc_wav.h
	-rm -f $(SINC)/pc_wav.h
	cp pc_wav.h $(SINC)
	chmod $(HMOD) $(SINC)/pc_wav.h

$(SINC)/exview.h: exview.h
	-rm -f $(SINC)/exview.h
	cp exview.h $(SINC)
	chmod $(HMOD) $(SINC)/exview.h

$(SINC)/exv_func.h: exv_func.h 
	-rm -f $(SINC)/exv_func.h
	cp exv_func.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_func.h

$(SINC)/exv_keys.h: exv_keys.h
	-rm -f $(SINC)/exv_keys.h
	cp exv_keys.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_keys.h

$(SINC)/exv_icon.h: exv_icon.h
	-rm -f $(SINC)/exv_icon.h
	cp exv_icon.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_icon.h

$(SINC)/exv_colors.h: exv_colors.h
	-rm -f $(SINC)/exv_colors.h
	cp exv_colors.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_colors.h

$(SINC)/erlicon_b.h: erlicon_b.h
	-rm -f $(SINC)/erlicon_b.h
	cp erlicon_b.h $(SINC)
	chmod $(HMOD) $(SINC)/erlicon_b.h

$(SINC)/erlicon_nb.h: erlicon_nb.h
	-rm -f $(SINC)/erlicon_nb.h
	cp erlicon_nb.h $(SINC)
	chmod $(HMOD) $(SINC)/erlicon_nb.h

$(SINC)/p3dicon.h: p3dicon.h
	-rm -f $(SINC)/p3dicon.h
	cp p3dicon.h $(SINC)
	chmod $(HMOD) $(SINC)/p3dicon.h

$(SINC)/sinicon.h: sinicon.h
	-rm -f $(SINC)/sinicon.h
	cp sinicon.h $(SINC)
	chmod $(HMOD) $(SINC)/sinicon.h

$(SINC)/histicon.h: histicon.h
	-rm -f $(SINC)/histicon.h
	cp histicon.h $(SINC)
	chmod $(HMOD) $(SINC)/histicon.h

$(SINC)/imagicon.h: imagicon.h
	-rm -f $(SINC)/imagicon.h
	cp imagicon.h $(SINC)
	chmod $(HMOD) $(SINC)/imagicon.h

$(SINC)/specicon.h: specicon.h
	-rm -f $(SINC)/specicon.h
	cp specicon.h $(SINC)
	chmod $(HMOD) $(SINC)/specicon.h

$(SINC)/exv_bbox.h: exv_bbox.h
	-rm -f $(SINC)/exv_bbox.h
	cp exv_bbox.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_bbox.h

$(SINC)/exv_olwm.h: exv_olwm.h
	-rm -f $(SINC)/exv_olwm.h
	cp exv_olwm.h $(SINC)
	chmod $(HMOD) $(SINC)/exv_olwm.h

$(SINC)/olwm.h: olwm.h
	-rm -f $(SINC)/olwm.h
	cp olwm.h $(SINC)
	chmod $(HMOD) $(SINC)/olwm.h

$(SINC)/mem.h: mem.h
	-rm -f $(SINC)/mem.h
	cp mem.h $(SINC)
	chmod $(HMOD) $(SINC)/mem.h

$(SINC)/menu.h: menu.h
	-rm -f $(SINC)/menu.h
	cp menu.h $(SINC)
	chmod $(HMOD) $(SINC)/menu.h

$(SINC)/feadst.h: feadst.h
	-rm -f $(SINC)/feadst.h
	cp feadst.h $(SINC)
	chmod $(HMOD) $(SINC)/feadst.h

$(SINC)/ana_methods.h: ana_methods.h
	-rm -f $(SINC)/ana_methods.h
	cp ana_methods.h $(SINC)
	chmod $(HMOD) $(SINC)/ana_methods.h

$(SINC)/ss.h: ss.h
	-rm -f $(SINC)/ss.h
	cp ss.h $(SINC)
	chmod $(HMOD) $(SINC)/ss.h

$(SINC)/feafilt.h: feafilt.h
	-rm -f $(SINC)/feafilt.h
	cp feafilt.h $(SINC)
	chmod $(HMOD) $(SINC)/feafilt.h

$(SINC)/epaths.h: epaths.h
	-rm -f $(SINC)/epaths.h
	cp epaths.h $(SINC)
	chmod $(HMOD) $(SINC)/epaths.h
	
$(SINC)/func.h: func.h
	-rm -f $(SINC)/func.h
	cp func.h $(SINC)
	chmod $(HMOD) $(SINC)/func.h
	
$(SINC)/ultrix_cpt.h: ultrix_cpt.h
	-rm -f $(SINC)/ultrix_cpt.h
	cp ultrix_cpt.h $(SINC)
	chmod $(HMOD) $(SINC)/ultrix_cpt.h
	
$(SINC)/xwaves_ipc.h: xwaves_ipc.h
	-rm -f $(SINC)/xwaves_ipc.h
	cp xwaves_ipc.h $(SINC)
	chmod $(HMOD) $(SINC)/xwaves_ipc.h
	
$(SINC)/lpsyn.h: lpsyn.h
	-rm -f $(SINC)/lpsyn.h
	cp lpsyn.h $(SINC)
	chmod $(HMOD) $(SINC)/lpsyn.h
	
$(SINC)/op.h: op.h
	-rm -f $(SINC)/op.h
	cp op.h $(SINC)
	chmod $(HMOD) $(SINC)/op.h

$(SINC)/audio.h: audio.h
	-rm -f $(SINC)/audio.h
	cp audio.h $(SINC)
	chmod $(HMOD) $(SINC)/audio.h

$(SINC)/alaw.h: alaw.h
	-rm -f $(SINC)/alaw.h
	cp alaw.h $(SINC)
	chmod $(HMOD) $(SINC)/alaw.h

$(SINC)/array.h: array.h
	-rm -f $(SINC)/array.h
	cp array.h $(SINC)
	chmod $(HMOD) $(SINC)/array.h

$(SINC)/complex.h: complex.h
	-rm -f $(SINC)/complex.h
	cp complex.h $(SINC)
	chmod $(HMOD) $(SINC)/complex.h

$(SINC)/fftcepstrum.h: fftcepstrum.h
	-rm -f $(SINC)/fftcepstrum.h
	cp fftcepstrum.h $(SINC)
	chmod $(HMOD) $(SINC)/fftcepstrum.h

$(SINC)/get_fft.h: get_fft.h
	-rm -f $(SINC)/get_fft.h
	cp get_fft.h $(SINC)
	chmod $(HMOD) $(SINC)/get_fft.h

$(SINC)/linalg.h: linalg.h
	-rm -f $(SINC)/linalg.h
	cp linalg.h $(SINC)
	chmod $(HMOD) $(SINC)/linalg.h

$(SINC)/mulaw.h: mulaw.h
	-rm -f $(SINC)/mulaw.h
	cp mulaw.h $(SINC)
	chmod $(HMOD) $(SINC)/mulaw.h

$(SINC)/rand.h: rand.h
	-rm -f $(SINC)/rand.h
	cp rand.h $(SINC)
	chmod $(HMOD) $(SINC)/rand.h

$(SINC)/range_switch.h: range_switch.h
	-rm -f $(SINC)/range_switch.h
	cp range_switch.h $(SINC)
	chmod $(HMOD) $(SINC)/range_switch.h

$(SINC)/strlist.h: strlist.h
	-rm -f $(SINC)/strlist.h
	cp strlist.h $(SINC)
	chmod $(HMOD) $(SINC)/strlist.h

$(SINC)/types.h: types.h
	-rm -f $(SINC)/types.h
	cp types.h $(SINC)
	chmod $(HMOD) $(SINC)/types.h

clean:
	@echo nothing here to clean

