/* xwaves_rpc.h */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)xwaves_rpc.h	1.2     6/2/93     ERL
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */
#include <rpc/rpc.h>
#ifdef FOR_XVIEW
#include <xview/xview.h>
#endif
#include <sys/time.h>
#include <sys/socket.h>

/* For RPC identification (program and version numbers) */
#define XWAVES_PNUM  450902
#define XWAVES_VNUM  50

/* Interlocked response vs asynchronous non-blocking response. */
#define IMMEDIATE_RESPONSE 9
#define INTERLOCK_RESPONSE 8
#define INTERLOCK_NOREPLY  7
#define INTERLOCK_DELAY_RESPONSE 6
#define RPCINFO_CALL 0

typedef struct str_list {
  char *str;
  void *data;
  struct str_list *next;
} StrList;

/* Defines the information held about each attachment. */
typedef struct adata {
  char *progname;		/* full name of the attachment program */
  char *menu;			/* the xwaves menu to which it is attached */
  int prognum,			/* the RPC program number of the attachment */
      progvers;			/* the attachment's RPC program version number */
  CLIENT *client;		/* RPC client object for calling purposes */
  StrList *pending;		/* pending requests for attachment */
  struct adata *next;
} Attachment;
