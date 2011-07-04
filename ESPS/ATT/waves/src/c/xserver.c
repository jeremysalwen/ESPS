/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   Alan Parker
 * Checked by:
 * Revised by:   David Talkin
 *
 * Brief description: Handles the socket server aspect of xwaves
 *
 */

static char *sccs_id = "@(#)xserver.c	1.5 10/8/96 ERL";

/*
  	NOTE	NOTE  	NOTE	NOTE  	NOTE	NOTE  	NOTE	NOTE

  Send_xwaves has been rewritten to use RPC instead of the ss
   library.  Support for the ss is still here to smooth transition and
   maintain compatibility with older habits, including conventions used
   in play and record programs that use ss (send_xwaves() and send_xwaves2())
   to communicate with xwaves.
*/


#include <stdio.h>
#ifndef LINUX_OR_MAC
#include <sgtty.h>
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/ss.h>
#include <xview/xview.h>

extern int socket_port;
extern Frame daddy;
extern int w_verbose;
extern int debug_level;
extern char ok[], null[];	/* in message.c */

extern char *checking_selectors();
extern char notice_msg[];

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }


struct clients {
  SOCKET *csp;
  struct  clients *next;
  struct  clients *prev;
};

static SOCKET *sp;
static struct clients *client_list = NULL;
static struct clients *addclient();
static struct clients *remove_client();
static SOCKET *find_socket();

static Notify_value server_io(), server_catcher();

/*********************************************************************/
char *meth_enable_server(ob, str)
     void *ob;
     char *str;
{
  if(debug_level)
    fprintf(stderr,"enable_server()\n");

  CHECK_QUERY(str,NULL)
  
  if(socket_port > 0) {
    static char envirp[200], envirh[200];
    char name[200], *c;
    extern int socket_port;
    int tmp = 0;

    if(!sp) {			/* server already running? */
      if((sp = ServerSock(socket_port)) == NULL) { /* can't open socket */
	sprintf(notice_msg,
        "Cannot open a socket on port %d\nContinuing without server mode. ",
		socket_port);
	show_notice(0,notice_msg);
	return(null);
      } 
      notify_set_input_func(daddy,server_catcher,sp->sd);

      gethostname(name,200);
      c = getenv("WAVES_HOST");
      if(!c || strcmp(name,c)) {
	sprintf(envirh,"WAVES_HOST=%s",name);
	putenv(envirh);
      }
      c = (char*)getenv("WAVES_PORT");
      if(!(c && (socket_port == atoi(c)))) {
	sprintf(envirp,"WAVES_PORT=%d",socket_port);
	putenv(envirp);
      }
    }
    return(ok);
  }
  return(null);
}

/*********************************************************************/
char *meth_disable_server(ob, str)
     void *ob;
     char *str;
{
  struct clients *p, *tmp;
  if(debug_level)
    fprintf(stderr,"disable_server()\n");
  CHECK_QUERY(str,NULL)
  if(!sp) return ok;		/* server not running */
  notify_set_input_func(daddy,NOTIFY_FUNC_NULL,sp->sd);

  p=client_list;
  while(p) {
    notify_set_input_func(daddy,NOTIFY_FUNC_NULL,p->csp->sd);
    (void)SockClose(p->csp);
    tmp = p->next;
    free((char *)p);
    p=tmp;
  }
  client_list=NULL;
  (void)SockClose(sp);
  sp = NULL;
  return ok;
}

/*********************************************************************/
static Notify_value  server_catcher(client, fd)
     Notify_client client;
     int fd;
{
  SOCKET *csp;

  csp = AcceptSock(sp);		/* connect to client */
  client_list = addclient(csp,client_list);
  
  if(w_verbose)
    fprintf(stderr,"Client %d just attached to waves server.\n",
	    csp->sd);
  notify_set_input_func(daddy,server_io,csp->sd);
  return NOTIFY_DONE;
}

static char lockfile[200] = "";
static remove_server_lock()
{
  if(*lockfile)
      unlink(lockfile);
  lockfile[0] = 0;
}

/*********************************************************************/
/* This dispatches incoming commands from the socket port.  It also
   has provisions for a crude form of interlock with (the old)
   send_xwaves: If send_xwaves has included a "server_lock" specification
   at the beginning of the incoming data, the indicated lock file will be
   deleted on return from the dispatch, thus letting (the old)
   send_xwaves know that it may exit.
*/
static Notify_value server_io(client, fd)
     Notify_client client;
     int fd;
{
  char buffer[1000], *lockname, *bufp, *get_next_item();
  SOCKET *csp;

  if (debug_level) fprintf(stderr,"server_io: fd: %d\n",fd);
  csp = find_socket(fd);
  if (SockGets(buffer,sizeof(buffer)-1,csp) == SS_NULL) { /* closed */
    client_list = remove_client(fd,client_list);
    (void)SockClose(csp);
    notify_set_input_func(daddy,NOTIFY_FUNC_NULL,fd); 
    if(w_verbose) fprintf(stderr,"Client %d disconnected.\n",fd);
  } else {
    double delay = .002;
    int is_ready;
    do {
      if(!strncmp("server_lock",buffer,strlen("server_lock"))) {
	bufp = get_next_item(lockname = get_next_item(buffer));
	strip_newline_at_etc(lockfile,lockname);
	if(debug_level && lockname)
	  fprintf(stderr,"server lock file:%s\n",lockfile);
      } else {
	bufp = buffer;
	*lockfile = 0;
      }
      if(*bufp) {
	(void)exec_waves(bufp);
      }
      remove_server_lock();
      if(SockSelect(delay, "r"))
	is_ready = IsReadSet(csp);
      else
	is_ready = 0;
    } while(is_ready && SockGets(buffer,sizeof(buffer)-1,csp));
  }
  return NOTIFY_DONE;
}

/*********************************************************************/
static SOCKET *find_socket(fd)
     int fd;
{
  struct clients *p;
		
  for (p=client_list; p; p=p->next)
    if (p->csp->sd == fd)
      break;
  spsassert(p,"find_socket: client not in list");
  return p->csp;
}

/*********************************************************************/
static struct clients *addclient(csp, client_list)
     SOCKET *csp;
     struct clients *client_list;
{
  struct clients *new;
	
  new = (struct clients *)malloc(sizeof(struct clients));
  spsassert(new,"malloc failed");
  new->prev = NULL;
  new->csp = csp;
  new->next = client_list;
  if (client_list)
    client_list->prev = new;
  return new;
}
	
/*********************************************************************/
static struct clients *remove_client(fd,client_list)
     struct clients *client_list;
     int fd;
{
  struct clients *p, *q;
		
  for (p=client_list; p; p=p->next)
    if (p->csp->sd == fd)
      break;
  spsassert(p,"remove_client: client not in list");
  if (p->prev) (p->prev)->next = p->next;
  if (p->next) (p->next)->prev = p->prev;
  q = p->next;
  free((char *)p);
  return p == client_list ? q : client_list;
}
