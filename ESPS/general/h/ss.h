/* ss.h */
/* writen by Mat Watson 12/01/90 */
/*

Copyright (c) 1990 Mat Watson and Hubert Bartels. All rights reserved. 

Permission to use, copy, and/or distribute for any purpose and 
without fee is hereby granted, provided that both the above copyright 
notice and this permission notice appear in all copies and derived works. 
Fees for distribution of this software or for derived sources may only 
be charged with the express written permission of the copyright holders. 
This software is provided ``as is'' without express or implied warranty.
*/
/* ERL sccs info: @(#)ss.h	1.7 2/20/96 ERL */

#ifndef ss_H
#define ss_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#ifdef IBM_RS6000
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

/* ss.h */
/* writen by Mat Watson 12/01/90 */
/*
Copyright (c) 1990 Mat Watson and Hubert Bartels. All rights reserved. 

Permission to use, copy, and/or distribute for any purpose and 
without fee is hereby granted, provided that both the above copyright 
notice and this permission notice appear in all copies and derived works. 
Fees for distribution of this software or for derived sources may only 
be charged with the express written permission of the copyright holders. 
This software is provided ``as is'' without express or implied warranty.
*/

#define MaxSockets 30
#define EXIT 0
#define CONT 1

typedef struct m_in_socket SOCKET;
typedef struct m_in_socket_array SOCKARRAY;

struct m_in_socket {
  int sd;
  struct sockaddr_in addr;
  int _rcnt;
  char *_rptr;
  char *_rbase;
  int _rbufsiz;
  int _wcnt;
  char *_wptr;
  char *_wbase;
  int _wbufsiz;
  short _flag;
};

struct m_in_socket_array {
  int n_sockets;
  SOCKET *spa[MaxSockets];
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
};

/* define flags */
/**/
#define SS_IOFBF  0
#define SS_IOREAD 01
#define SS_IOWRT  02
#define SS_IONBF  04
#define SS_IOMYBUF        010
#define SS_IOEOF  020
#define SS_IOERR  040
#define SS_IOSTRG 0100
#define SS_IOLBF  0200
#define SS_IORW   0400
#define SS_NULL    0
#define SS_EOF     (-1)
#define SS_BUFSIZ  1024

#define SockGetc(p)  (--(p)->_rcnt>=0? ((int)*(p)->_rptr++):_SockFilbuf(p))

#define SockPutc(x, p)  (--(p)->_wcnt >= 0 ?\
	    (int)(*(p)->_wptr++ = (char)(x)) :\
	       (((p)->_flag & SS_IOLBF) && -(p)->_wcnt < (p)->_wbufsiz ?\
                ((*(p)->_wptr = (char)(x)) != '\n' ?\
                        (int)(*(p)->_wptr++) :\
                        _SockFlsbuf(*(char *)(p)->_wptr, p)) :\
                _SockFlsbuf((char)(x), p)))

#define SockEof(p)         (((p)->_flag&SS_IOEOF)!=0)
#define SockError(p)       (((p)->_flag&SS_IOERR)!=0)
#define SockFileno(p)       ((p)->sd)
#define SockClearerr(p)     (void) ((p)->_flag &= ~(SS_IOERR|SS_IOEOF))

#ifdef __STDC__
# define	P_(s) s
#else
# define P_(s) ()
#endif


/* ss.c */
SOCKET *AcceptSock P_((SOCKET *ssp ));
void AddSock P_((SOCKET *sp , SOCKARRAY *sap ));
SOCKET *ConnectSock P_((char *hostname , unsigned port_number ));
int IsExceptSet P_((SOCKET *sp ));
int IsReadSet P_((SOCKET *sp ));
int IsWriteSet P_((SOCKET *sp ));
void RemoveSock P_((SOCKET *sp ));
SOCKET *ServerSock P_((unsigned port_number ));
void SockArrayInit P_((SOCKARRAY *sap ));
int SockClose P_((SOCKET *sp ));
int _SockFilbuf P_((SOCKET *sp ));
int _SockFlsbuf P_((int x , SOCKET *sp ));
int SockFlush P_((SOCKET *sp ));
char *SockGets P_((char *s , int n , SOCKET *sp ));
int SockIndex P_((SOCKET *sp , SOCKARRAY *sap ));
void SockInit P_((int c ));
int SockIsRead P_((SOCKET *sp ));
int SockIsWrite P_((SOCKET *sp ));
int SockPuts P_((char *s , SOCKET *sp ));
int SockSelect P_((double timeout , char *flag ));
void SockSignal P_((int sig ));
int SockWrites P_((char *s , SOCKET *sp ));
void sserror P_((char *mesg , int code ));

#undef P_

/* entropic additions */

SOCKET *
open_xwaves ARGS((char *host, int port, int verbose));

int
send_xwaves ARGS((SOCKET *sp, char *str));

void
close_xwaves ARGS((SOCKET *sp));

int
send_xwaves2 ARGS((char *host, int port, char *str, int verbose));

#ifdef __cplusplus
}
#endif

#endif /* ss_H */
