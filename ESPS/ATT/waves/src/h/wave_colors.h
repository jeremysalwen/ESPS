/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)wave_colors.h	1.3 9/26/95 ATT/ERL/ESI */
/* wave_colors.h */

#define W_FOREGROUND_COLOR (cmap_size - 1)
#define W_BACKGRND_COLOR   (cmap_size - 2)
#define W_RETICLE_COLOR    (cmap_size - 3)
#define W_TEXT_COLOR       (cmap_size - 4)
#define W_WAVEFORM_COLOR   (cmap_size - 5)
#define W_CURSOR_COLOR     (cmap_size - 6)
#define W_MARKER_COLOR     (cmap_size - 7)
#define W_YA5_COLOR        (cmap_size - 8)
#define W_YA4_COLOR        (cmap_size - 9)
#define W_YA3_COLOR        (cmap_size - 10)
#define W_YA2_COLOR        (cmap_size - 11)
#define W_YA1_COLOR        (cmap_size - 12)
#define W_WAVE2_COLOR 	 (cmap_size - 13)
#define W_CMAP_RESERVED     (cmap_size - 13)	/* lowest reserved value */
#define W_BACKGROUND_COLOR 0	/* real background */
#define W_BACK2_COLOR      2	/* 2nd off-white background */

extern int FOREGROUND_COLOR,
 BACKGRND_COLOR,
 RETICLE_COLOR,
 TEXT_COLOR,
 WAVEFORM_COLOR,
 CURSOR_COLOR,
 MARKER_COLOR,
 YA5_COLOR,
 YA4_COLOR,
 YA3_COLOR,
 YA2_COLOR,
 YA1_COLOR,
 WAVE2_COLOR,
 CMAP_RESERVED,
 BACKGROUND_COLOR,
 BACK2_COLOR,
 cmap_size;


