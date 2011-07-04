/* g3.h - header file for group 3 FAX compression filters
*/

/* @(#)g3.h	1.1  10/13/90 */

#define MAXCOLS 2550	/* Maximum image size is 8.5"x11" @ 300dpi */
#define MAXROWS 3300

#define TWTABLE		23
#define MWTABLE		24
#define TBTABLE		25
#define MBTABLE		26
#define EXTABLE		27
#define VRTABLE		28

#define WHASHA 3510
#define WHASHB 1178
#define BHASHA 293
#define BHASHB 2695
#define HASHSIZE 1021

#ifndef _G3_H_
#define _G3_H_

typedef unsigned char bit;

int	bmask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

typedef struct tableentry {
    int tabid;
    int code;
    int length;
    int count;
    } tableentry;

struct tableentry twtable[] = {
    { TWTABLE, 0x35, 8, 0 },
    { TWTABLE, 0x7, 6, 1 },
    { TWTABLE, 0x7, 4, 2 },
    { TWTABLE, 0x8, 4, 3 },
    { TWTABLE, 0xb, 4, 4 },
    { TWTABLE, 0xc, 4, 5 },
    { TWTABLE, 0xe, 4, 6 },
    { TWTABLE, 0xf, 4, 7 },
    { TWTABLE, 0x13, 5, 8 },
    { TWTABLE, 0x14, 5, 9 },
    { TWTABLE, 0x7, 5, 10 },
    { TWTABLE, 0x8, 5, 11 },
    { TWTABLE, 0x8, 6, 12 },
    { TWTABLE, 0x3, 6, 13 },
    { TWTABLE, 0x34, 6, 14 },
    { TWTABLE, 0x35, 6, 15 },
    { TWTABLE, 0x2a, 6, 16 },
    { TWTABLE, 0x2b, 6, 17 },
    { TWTABLE, 0x27, 7, 18 },
    { TWTABLE, 0xc, 7, 19 },
    { TWTABLE, 0x8, 7, 20 },
    { TWTABLE, 0x17, 7, 21 },
    { TWTABLE, 0x3, 7, 22 },
    { TWTABLE, 0x4, 7, 23 },
    { TWTABLE, 0x28, 7, 24 },
    { TWTABLE, 0x2b, 7, 25 },
    { TWTABLE, 0x13, 7, 26 },
    { TWTABLE, 0x24, 7, 27 },
    { TWTABLE, 0x18, 7, 28 },
    { TWTABLE, 0x2, 8, 29 },
    { TWTABLE, 0x3, 8, 30 },
    { TWTABLE, 0x1a, 8, 31 },
    { TWTABLE, 0x1b, 8, 32 },
    { TWTABLE, 0x12, 8, 33 },
    { TWTABLE, 0x13, 8, 34 },
    { TWTABLE, 0x14, 8, 35 },
    { TWTABLE, 0x15, 8, 36 },
    { TWTABLE, 0x16, 8, 37 },
    { TWTABLE, 0x17, 8, 38 },
    { TWTABLE, 0x28, 8, 39 },
    { TWTABLE, 0x29, 8, 40 },
    { TWTABLE, 0x2a, 8, 41 },
    { TWTABLE, 0x2b, 8, 42 },
    { TWTABLE, 0x2c, 8, 43 },
    { TWTABLE, 0x2d, 8, 44 },
    { TWTABLE, 0x4, 8, 45 },
    { TWTABLE, 0x5, 8, 46 },
    { TWTABLE, 0xa, 8, 47 },
    { TWTABLE, 0xb, 8, 48 },
    { TWTABLE, 0x52, 8, 49 },
    { TWTABLE, 0x53, 8, 50 },
    { TWTABLE, 0x54, 8, 51 },
    { TWTABLE, 0x55, 8, 52 },
    { TWTABLE, 0x24, 8, 53 },
    { TWTABLE, 0x25, 8, 54 },
    { TWTABLE, 0x58, 8, 55 },
    { TWTABLE, 0x59, 8, 56 },
    { TWTABLE, 0x5a, 8, 57 },
    { TWTABLE, 0x5b, 8, 58 },
    { TWTABLE, 0x4a, 8, 59 },
    { TWTABLE, 0x4b, 8, 60 },
    { TWTABLE, 0x32, 8, 61 },
    { TWTABLE, 0x33, 8, 62 },
    { TWTABLE, 0x34, 8, 63 },
    };

struct tableentry mwtable[] = {
    { MWTABLE, 0x1b, 5, 64 },
    { MWTABLE, 0x12, 5, 128 },
    { MWTABLE, 0x17, 6, 192 },
    { MWTABLE, 0x37, 7, 256 },
    { MWTABLE, 0x36, 8, 320 },
    { MWTABLE, 0x37, 8, 384 },
    { MWTABLE, 0x64, 8, 448 },
    { MWTABLE, 0x65, 8, 512 },
    { MWTABLE, 0x68, 8, 576 },
    { MWTABLE, 0x67, 8, 640 },
    { MWTABLE, 0xcc, 9, 704 },
    { MWTABLE, 0xcd, 9, 768 },
    { MWTABLE, 0xd2, 9, 832 },
    { MWTABLE, 0xd3, 9, 896 },
    { MWTABLE, 0xd4, 9, 960 },
    { MWTABLE, 0xd5, 9, 1024 },
    { MWTABLE, 0xd6, 9, 1088 },
    { MWTABLE, 0xd7, 9, 1152 },
    { MWTABLE, 0xd8, 9, 1216 },
    { MWTABLE, 0xd9, 9, 1280 },
    { MWTABLE, 0xda, 9, 1344 },
    { MWTABLE, 0xdb, 9, 1408 },
    { MWTABLE, 0x98, 9, 1472 },
    { MWTABLE, 0x99, 9, 1536 },
    { MWTABLE, 0x9a, 9, 1600 },
    { MWTABLE, 0x18, 6, 1664 },
    { MWTABLE, 0x9b, 9, 1728 },
    };

struct tableentry tbtable[] = {
    { TBTABLE, 0x37, 10, 0 },
    { TBTABLE, 0x2, 3, 1 },
    { TBTABLE, 0x3, 2, 2 },
    { TBTABLE, 0x2, 2, 3 },
    { TBTABLE, 0x3, 3, 4 },
    { TBTABLE, 0x3, 4, 5 },
    { TBTABLE, 0x2, 4, 6 },
    { TBTABLE, 0x3, 5, 7 },
    { TBTABLE, 0x5, 6, 8 },
    { TBTABLE, 0x4, 6, 9 },
    { TBTABLE, 0x4, 7, 10 },
    { TBTABLE, 0x5, 7, 11 },
    { TBTABLE, 0x7, 7, 12 },
    { TBTABLE, 0x4, 8, 13 },
    { TBTABLE, 0x7, 8, 14 },
    { TBTABLE, 0x18, 9, 15 },
    { TBTABLE, 0x17, 10, 16 },
    { TBTABLE, 0x18, 10, 17 },
    { TBTABLE, 0x8, 10, 18 },
    { TBTABLE, 0x67, 11, 19 },
    { TBTABLE, 0x68, 11, 20 },
    { TBTABLE, 0x6c, 11, 21 },
    { TBTABLE, 0x37, 11, 22 },
    { TBTABLE, 0x28, 11, 23 },
    { TBTABLE, 0x17, 11, 24 },
    { TBTABLE, 0x18, 11, 25 },
    { TBTABLE, 0xca, 12, 26 },
    { TBTABLE, 0xcb, 12, 27 },
    { TBTABLE, 0xcc, 12, 28 },
    { TBTABLE, 0xcd, 12, 29 },
    { TBTABLE, 0x68, 12, 30 },
    { TBTABLE, 0x69, 12, 31 },
    { TBTABLE, 0x6a, 12, 32 },
    { TBTABLE, 0x6b, 12, 33 },
    { TBTABLE, 0xd2, 12, 34 },
    { TBTABLE, 0xd3, 12, 35 },
    { TBTABLE, 0xd4, 12, 36 },
    { TBTABLE, 0xd5, 12, 37 },
    { TBTABLE, 0xd6, 12, 38 },
    { TBTABLE, 0xd7, 12, 39 },
    { TBTABLE, 0x6c, 12, 40 },
    { TBTABLE, 0x6d, 12, 41 },
    { TBTABLE, 0xda, 12, 42 },
    { TBTABLE, 0xdb, 12, 43 },
    { TBTABLE, 0x54, 12, 44 },
    { TBTABLE, 0x55, 12, 45 },
    { TBTABLE, 0x56, 12, 46 },
    { TBTABLE, 0x57, 12, 47 },
    { TBTABLE, 0x64, 12, 48 },
    { TBTABLE, 0x65, 12, 49 },
    { TBTABLE, 0x52, 12, 50 },
    { TBTABLE, 0x53, 12, 51 },
    { TBTABLE, 0x24, 12, 52 },
    { TBTABLE, 0x37, 12, 53 },
    { TBTABLE, 0x38, 12, 54 },
    { TBTABLE, 0x27, 12, 55 },
    { TBTABLE, 0x28, 12, 56 },
    { TBTABLE, 0x58, 12, 57 },
    { TBTABLE, 0x59, 12, 58 },
    { TBTABLE, 0x2b, 12, 59 },
    { TBTABLE, 0x2c, 12, 60 },
    { TBTABLE, 0x5a, 12, 61 },
    { TBTABLE, 0x66, 12, 62 },
    { TBTABLE, 0x67, 12, 63 },
    };

struct tableentry mbtable[] = {
    { MBTABLE, 0xf, 10, 64 },
    { MBTABLE, 0xc8, 12, 128 },
    { MBTABLE, 0xc9, 12, 192 },
    { MBTABLE, 0x5b, 12, 256 },
    { MBTABLE, 0x33, 12, 320 },
    { MBTABLE, 0x34, 12, 384 },
    { MBTABLE, 0x35, 12, 448 },
    { MBTABLE, 0x6c, 13, 512 },
    { MBTABLE, 0x6d, 13, 576 },
    { MBTABLE, 0x4a, 13, 640 },
    { MBTABLE, 0x4b, 13, 704 },
    { MBTABLE, 0x4c, 13, 768 },
    { MBTABLE, 0x4d, 13, 832 },
    { MBTABLE, 0x72, 13, 896 },
    { MBTABLE, 0x73, 13, 960 },
    { MBTABLE, 0x74, 13, 1024 },
    { MBTABLE, 0x75, 13, 1088 },
    { MBTABLE, 0x76, 13, 1152 },
    { MBTABLE, 0x77, 13, 1216 },
    { MBTABLE, 0x52, 13, 1280 },
    { MBTABLE, 0x53, 13, 1344 },
    { MBTABLE, 0x54, 13, 1408 },
    { MBTABLE, 0x55, 13, 1472 },
    { MBTABLE, 0x5a, 13, 1536 },
    { MBTABLE, 0x5b, 13, 1600 },
    { MBTABLE, 0x64, 13, 1664 },
    { MBTABLE, 0x65, 13, 1728 },
    };

struct tableentry extable[] = {
    { EXTABLE, 0x8, 11, 1792 },
    { EXTABLE, 0xc, 11, 1856 },
    { EXTABLE, 0xd, 11, 1920 },
    { EXTABLE, 0x12, 12, 1984 },
    { EXTABLE, 0x13, 12, 2048 },
    { EXTABLE, 0x14, 12, 2112 },
    { EXTABLE, 0x15, 12, 2176 },
    { EXTABLE, 0x16, 12, 2240 },
    { EXTABLE, 0x17, 12, 2304 },
    { EXTABLE, 0x1c, 12, 2368 },
    { EXTABLE, 0x1d, 12, 2432 },
    { EXTABLE, 0x1e, 12, 2496 },
    { EXTABLE, 0x1f, 12, 2560 },
    };

#endif /*_G3_H_*/
