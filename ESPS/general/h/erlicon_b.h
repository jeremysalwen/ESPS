/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 * @(#)erlicon_b.h	1.6	2/20/96	ERL
 *
 * ERL icon with a thin border
 */

#ifndef erlicon_b_H
#define erlicon_b_H

#ifdef __cplusplus
extern "C" {
#endif

#define erlicon_b_width 64
#define erlicon_b_height 64
static unsigned char erlicon_b_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x80, 0xff, 0x07, 0x00, 0x00, 0x80, 0x01, 0x00, 0xe0, 0x0f,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x80, 0x07, 0xf0,
   0x07, 0x00, 0x00, 0x80, 0x01, 0xe0, 0x01, 0xff, 0x07, 0x00, 0x00, 0x80,
   0x01, 0x70, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x80, 0xff, 0x3f, 0xf8, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x01, 0x1c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x86, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0xc3, 0x01, 0x00, 0xc0, 0x7f, 0x00, 0x80,
   0x81, 0xe3, 0x00, 0x00, 0x30, 0xe0, 0x07, 0x80, 0x81, 0x71, 0x00, 0x00,
   0x0c, 0xf0, 0x1f, 0x80, 0xc1, 0x30, 0x00, 0x00, 0x02, 0xf8, 0x7f, 0x80,
   0xc1, 0x38, 0x00, 0x00, 0x01, 0xfc, 0xff, 0x80, 0xc1, 0x18, 0x00, 0xc0,
   0x00, 0xfc, 0xff, 0x81, 0x61, 0x18, 0x00, 0x20, 0x00, 0xfc, 0xff, 0x81,
   0x61, 0xfc, 0xff, 0xff, 0x00, 0xfe, 0x3f, 0x80, 0x61, 0xfc, 0xff, 0xff,
   0x00, 0xfe, 0x1f, 0x80, 0x61, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0x80,
   0x61, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x0f, 0x80, 0x61, 0x00, 0x00, 0x00,
   0x00, 0xfe, 0x0f, 0x80, 0x61, 0xfc, 0xff, 0xff, 0x00, 0xfe, 0x0f, 0x80,
   0x61, 0xf8, 0xff, 0xff, 0x00, 0xfc, 0x0f, 0x80, 0xc1, 0x18, 0x03, 0x00,
   0x00, 0xfc, 0x0f, 0x80, 0xc1, 0xb8, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0x80,
   0xc1, 0x70, 0x00, 0x00, 0x00, 0xf0, 0x1f, 0x80, 0x81, 0x71, 0x00, 0x00,
   0x00, 0xe0, 0x3f, 0x80, 0xf1, 0xe3, 0x00, 0x00, 0x00, 0x80, 0x7f, 0x80,
   0x0f, 0xc3, 0x01, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x01, 0x86, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x1c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x3f, 0xf8, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x01, 0x70, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x80,
   0x01, 0xe0, 0x01, 0xff, 0x07, 0x00, 0x00, 0x80, 0x01, 0x80, 0x07, 0xf0,
   0x07, 0x00, 0x00, 0x80, 0x01, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0xe0, 0x0f,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x80, 0xff, 0x07, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#ifdef __cplusplus
}
#endif

#endif /* erlicon_b_H */
