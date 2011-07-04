/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: sends commands to xwaves via a socket connection
 *
 */

static char *sccs_id = "@(#)sendwaves.c	1.3	2/20/96	ERL";


#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/ss.h>

/*
  open_xwaves():
  Trys to open a connection to an xwaves+ server.  First use host and
  port arguments, if they are NULL and/or zero (for port) use environment
  variables WAVES_HOST and/or WAVES_PORT if defined.   If not defined,
  then use defaults from esps/esps.h
*/

extern int debug_level;

SOCKET *
open_xwaves(host, port, verbose)
char *host;
int port,verbose;
{
	int waves_port = DEFAULT_PORT;
        char hostname[MAXHOSTNAMELEN+1];
	SOCKET *sp;

	hostname[0] = '\0';
	if (!port && getenv("WAVES_PORT"))
	  waves_port = atoi(getenv("WAVES_PORT"));
	else if (port > 0)
	  waves_port = port;

	if (!host && getenv("WAVES_HOST"))
	  (void)strcpy(hostname,getenv("WAVES_HOST"));
	else if (host)
	  (void)strcpy(hostname,host);
	else {
	  if(gethostname(hostname,MAXHOSTNAMELEN) == -1) {
	    fprintf(stderr,"open_xwaves: cannot get local hostname.\n");
	    fprintf(stderr,
		"open_xwaves: this indicates a serious network problem.\n");
	    return NULL;
	  }
	}
	
	if (debug_level)
	  fprintf(stderr,"open_xwaves: hostname: %s, port: %d\n",
		hostname,waves_port);

	if ((sp = ConnectSock(hostname, waves_port)) == SS_NULL) {
	  if (verbose) {
		fprintf(stderr,
		 "open_xwaves: Cannot connect to xwaves server at ");
		fprintf(stderr, "host %s, port %d.\n",hostname,waves_port);
	  }
	  return NULL;
	}
	return sp;
}

/* 
   send_xwaves:
   send a message to an open xwaves+ socket, return 0 for OK, -1 for error
*/

int
send_xwaves(sp, str)
SOCKET *sp;
char *str;
{
	spsassert(sp && str,"send_xwaves: sp or str null");
	if (SockWrites(str, sp) == SS_EOF)
	  return -1;
	else
	  return 0;
}

/*
  close_xwaves:
  close an open xwaves+ socket.  
*/
void
close_xwaves(sp)
SOCKET *sp;
{
	spsassert(sp,"close_xwaves: sp is null");
	(void)SockClose(sp);
}

/*
  send_xwaves2:
  open a connection, send str, and then close the connection
*/

int
send_xwaves2(host, port, str, verbose)
char *host, *str;
int port, verbose;
{
	SOCKET *sp;
	int waves_port = DEFAULT_PORT;
        char hostname[MAXHOSTNAMELEN+1];
	spsassert(str,"send_xwaves2: str is null");

	if (!port && getenv("WAVES_PORT"))
	  waves_port = atoi(getenv("WAVES_PORT"));
	else if (port)
	  waves_port = port;

	if (!host && getenv("WAVES_HOST"))
	  (void)strcpy(hostname,getenv("WAVES_HOST"));
	else if (host)
	  (void)strcpy(hostname,host);
	else {
	  if(gethostname(hostname,MAXHOSTNAMELEN) == -1) {
	    fprintf(stderr,"send_xwaves: cannot get local hostname.\n");
	    fprintf(stderr,
		"send_xwaves: this indicates a serious network problem.\n");
	    return -1;
	  }
	}
	
	if (debug_level)
	  fprintf(stderr,"send_xwaves2: hostname: %s, port: %d\n",
		hostname,waves_port);

	if ((sp = ConnectSock(hostname, waves_port)) == SS_NULL) {
	  if (verbose) {
		fprintf(stderr,
		 "open_xwaves: Cannot connect to xwaves server at ");
		fprintf(stderr, "host %s, port %d.\n",hostname,waves_port);
	  }
	  return -1;
	}
	
	if (SockWrites(str, sp) == SS_EOF) return -1;
	(void)SockClose(sp);
}
