/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)file_ext.h	1.2 9/26/95 ATT/ERL/ESI */
/* file_ext.h */

/*********************************************************************/
/* A list specifying the correspondence between filename extensions and
   file types defined in Signals.h. */
typedef struct tlist {
  int type;
  char *ext;
  struct tlist *next;
} Tlist;

static Tlist ext1 = {VECTOR_SIGNALS, ".d", NULL},
      ext2 = {SIG_LPC_POLES, ".pole", &ext1},
      ext3 = {SIG_F0, ".f0", &ext2},
      ext4 = {SIG_FORMANTS, ".fb", &ext3},
      ext7 = {SIG_LPCA, ".lpc", &ext4},
      ext8 = {SIG_LPCRC, ".rc", &ext7},
      default_extensions = {SIG_SPECTROGRAM, ".spgm", &ext8};

static Tlist *extensions = &default_extensions;
