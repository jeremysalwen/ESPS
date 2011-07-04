/*----------------------------------------------------------------------+
|									|
|  Copyright (c) 1990 Entropic Research Laboratory, Inc.		|
|									|
|  Permission to use, copy, modify, distribute, and sell this software	|
|  and its documentation for any purpose is hereby granted without	|
|  fee, provided that the above copyright notice appear in all copies	|
|  and that both that copyright notice and this permission notice	|
|  appear in supporting documentation.  Neither Entropic Research	|
|  Laboratory nor the author makes any representations about the	|
|  suitability of this software for any purpose.  It is provided "as	|
|  is" without express or implied warranty.				|
|									|
|  ENTROPIC RESEARCH LABORATORY AND THE AUTHOR DISCLAIM ALL WARRANTIES	|
|  WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF	|
|  MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ENTROPIC RESEARCH	|
|  LABORATORY OR THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT, OR	|
|  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS	|
|  OF USE, DATA, OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,		|
|  NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN		|
|  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.		|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module:  ready.h							|
|									|
|  This module is include in programs xloadimage and next_slide.  The	|
|  latter program causes xloadimage, if invoked with the option		|
|  -prog_slideshow, to advance to the next image.  The functions in	|
|  ready.c create and access a semaphore that the two programs use	|
|  for communication.  This include file defines symoblic names for	|
|  values of the semaphore.						|
|									|
|  Rod Johnson, Entropic Research Laboratory, Inc.  90/09/25		|
|									|
+----------------------------------------------------------------------*/
/* @(#)ready.h	1.1	11/6/90	ERL */

#define RDY_BUSY	0
#define RDY_READY	1
#define RDY_PAUSE	2
#define RDY_QUIT	3
