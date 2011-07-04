/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)xwaves_ipc.h	1.3 2/20/96 ERL */
/* xwaves_ipc.h */
/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 *    "Copyright (c) 1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * 
 * Written by:  David Talkin
 * Major Revisions by:  Alan Parker
 * 
 * Brief description:
 * 
 */

#ifndef xwaves_ipc_H
#define xwaves_ipc_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


#include <X11/Xlib.h>

/* Interlocked response vs asynchronous non-blocking response. */
#define IMMEDIATE_RESPONSE 9
#define INTERLOCK_RESPONSE 8
#define INTERLOCK_NOREPLY  7
#define INTERLOCK_DELAY_RESPONSE 6
#define INTERLOCK_DELAY_RESPONSE_TK 5
#define RPCINFO_CALL 0

typedef struct win_data {
   Window          caller;	/* calling window to send reply to */
   int             serial;	/* serial number of reply to TK */
   int             tk;		/* if 1 then caller is TK */
}               Win_data;

typedef struct str_list {
   char           *str;
   Win_data       *data;
   struct str_list *next;
}               StrList;

/* Defines the information held about each attachment. */
typedef struct adata {
   char           *progname;	/* full name of the attachment program */
   char           *menu;	/* the xwaves menu to which it is attached */
   char           *registry_name;	/* actual name the program registered
					 * as */
   int             ready;	/* 1 means attachment is alive */
   StrList        *pending;	/* pending requests for attachment */
   struct adata   *next;
}               Attachment;

typedef struct sxptr {
   Display        *display;
   Window          my_window;
   Window          dest_window;
   char           *my_name;
   char           *dest_name;
}               Sxptr;


char *
Setup_X_Comm ARGS((char *name, Display *display, Window window));

Window
Get_X_Comm_Win ARGS((Display *display, char *name));

int
Send_X_Msg ARGS((Display *display, char *destname,
		 Window window, int req, char *msg, int length));

int
Send_X_Reply ARGS((Display *display, char *destname,
		   Window window, int serial, char *msg, int length));

void
Close_X_Comm ARGS((Display *display, char *name));

int
Read_X_Comm ARGS((Display *display, Window window, XEvent *xevent, int raw,
		  char *buffer, int max_length, void (*callback) ()));

int
SendXwavesNoReply ARGS((char *display_name,
			char *dest, Sxptr *sxarg, char *msg));

char *
SendXwavesReply ARGS((char *display_name, char *dest,
		      Sxptr *sxarg, char *msg, unsigned int timeout));

Sxptr *
OpenXwaves ARGS((char *display_name, char *dest, char *myname));

void
CloseXwaves ARGS((Sxptr *sxptr));

char *
StripReturn ARGS((char *msg));


#ifdef __cplusplus
}
#endif

#endif /* xwaves_ipc_H */
