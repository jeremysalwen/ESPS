/* Simple Socket Libary
   Written by Mat Watson and Hubert Bartels.  12/01/90
*/
/*
Copyright (c) 1990 Mat Watson and Hubert Bartels. All rights reserved. 

Permission to use, copy, and/or distribute for any purpose and 
without fee is hereby granted, provided that both the above copyright 
notice and this permission notice appear in all copies and derived works. 
Fees for distribution of this software or for derived sources may only 
be charged with the express written permission of the copyright holders. 
This software is provided ``as is'' without express or implied warranty.
*/

#include <esps/ss.h>

#ifdef M5600
#define OLD_BSD_FDSETS
#include <esps/ultrix_cpt.h>
#endif

static char *sccs_id = "%W% %G%	ERL";

/* #include "private_globals.h" */
/* #include <signal.h> */

int ss_server_flag;
int ss_client_flag;
int ss_init_flag=0;
SOCKARRAY ss_sockbuff;
int max_sd;  /* this variable is a private global */

#ifdef APOLLO_68K
int _SockFlsbuf();
void SockInit();
#endif

#ifndef DEC_ALPHA
char *malloc();
char *calloc();
#endif

SOCKET *AcceptSock( ssp )
SOCKET *ssp;               /* Server Socket Pointer */
{
  SOCKET *csp;             /* Client Socket Pointer */
  struct sockaddr *adrp;   /* Address Poiner        */
  int *adrlen;             /* Address Length        */
  int csd;                 /* Client Socket Descriptor */

  if( !(ss_sockbuff.n_sockets < MaxSockets) )
    return (SOCKET *) SS_NULL;

  /* Allocate space for the client's socket structure */
  csp = (SOCKET *)malloc( sizeof( SOCKET ));

  if( !csp )
    sserror("AcceptSock: malloc()",EXIT);

  csp->_rcnt = 0;
  csp->_rptr = SS_NULL;
  csp->_rbase = SS_NULL;
  csp->_wcnt = 0;
  csp->_wptr = SS_NULL;
  csp->_wbase = SS_NULL;
  csp->_flag = 0;

  adrp = (struct sockaddr *)0;
  adrlen = (int *)0;

  csd = accept( ssp->sd, adrp, adrlen );
  if( csd < 0 ) {
#ifdef Verbose_Errors
    sserror("AcceptSock: accept()",CONT);
#endif
    return SS_NULL;
  }

  /* Set the value of the maximum socket descriptor used so far */
  max_sd = csd >= max_sd ? csd : max_sd;
  
  csp->sd = csd;

  AddSock( csp, &ss_sockbuff );

  /* Return a pointer to the client socket */
  return csp;
}

void AddSock( sp, sap )
SOCKET *sp;
SOCKARRAY *sap;
{
  int n;
  n = sap->n_sockets;
  sap->n_sockets++;
  sap->spa[n] = sp;
  return;
}

SOCKET *ConnectSock( hostname, port_number )
char *hostname;
unsigned port_number;
{
  SOCKET *sp;
  struct hostent *hep;
  struct sockaddr *adrp;
  int adrlen;
  int result;

  SockInit( 'c' );

  sp = (SOCKET *) malloc( sizeof( SOCKET ) );

  if( !sp ) {
#ifdef Verbose_Errors
    sserror("ConnectSock: malloc()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }

  sp->_rcnt = 0;
  sp->_rptr = SS_NULL;
  sp->_rbase = SS_NULL;
  sp->_wcnt = 0;
  sp->_wptr = SS_NULL;
  sp->_wbase = SS_NULL;
  sp->_flag = 0;

  AddSock( sp, &ss_sockbuff );

  hep = gethostbyname( hostname );
  if( hep == (struct hostent *)0 ) {
#ifdef Verbose_Errors
    perror("ConnectSock: gethostbyname()");
#endif
    return (SOCKET *) SS_NULL;
  }

  bcopy( (char *)(hep->h_addr), (char *)&(sp->addr.sin_addr), hep->h_length );

  sp->addr.sin_family = AF_INET;
  sp->addr.sin_port = htons( (u_short)port_number );
  sp->sd = socket( AF_INET, SOCK_STREAM, 0 );
  if( sp->sd < 0 ) {
#ifdef Verbos_Errors
    sserror("ConnectSock: socket()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }

  adrp = (struct sockaddr *) &(sp->addr);
  adrlen = sizeof( sp->addr );

  result = connect( sp->sd, adrp, adrlen );
  if( result < 0 ) {
#ifdef Verbos_Errors
    sserror("ConnectSock: connect()",CONT);
#endif
    close(sp->sd);
    free((char *) sp );
    return (SOCKET *) SS_NULL;
  }
  max_sd = sp->sd >= max_sd ? sp->sd : max_sd ;
  return sp;
}

int IsExceptSet( sp )
SOCKET *sp;
{
  int result, i;
  SOCKARRAY * sap;
  sap = &ss_sockbuff;
  i = SockIndex( sp, sap );
  if( i == -1 )
    return -1;           /* Socket is not in ss_sockbuff */
  result = FD_ISSET( sap->spa[i]->sd, &(sap->exceptfds) );
  return result;
}

int IsReadSet( sp )
SOCKET *sp;
{
  int result, i;
  SOCKARRAY * sap;
  sap = &ss_sockbuff;
  i = SockIndex( sp, sap );
  if( i == -1 )
    return i;               /* Socket is not in the ss_sockbuff */
  result = FD_ISSET( sap->spa[i]->sd, &(sap->readfds) );
  return result;
}

int IsWriteSet( sp )
SOCKET *sp;
{
  int result, i;
  SOCKARRAY * sap;
  sap = &ss_sockbuff;
  i = SockIndex( sp, sap );
  if( i == -1 )
    return -1;    /* Socket is not in ss_sockbuff */
  result = FD_ISSET( sap->spa[i]->sd, &(sap->writefds) );
  return result;
}

void RemoveSock( sp )
SOCKET *sp;
{
  int i, n;
  SOCKARRAY *sap;
  sap = &ss_sockbuff;

  for( i = 0; i < sap->n_sockets; i++ ) 
    if( sp == sap->spa[i] ) break;

  n = i;

  for( i = n + 1; i < sap->n_sockets; i++ )
    sap->spa[i-1] = sap->spa[i];

  sap->spa[sap->n_sockets - 1] = SS_NULL;
  sap->n_sockets--;
  return;
}


SOCKET *ServerSock( port_number )
unsigned port_number;
{
  int sd;
  int optval;
  unsigned optlen;
  SOCKET *sp;
  int result;

  SockInit( 's' );

  sd = socket( AF_INET, SOCK_STREAM, 0 );
  if( sd < 0 ) {
/* sserror("ServerSock: socket()",EXIT); */
   return (SOCKET *) SS_NULL; 
  }
  
  optval = 1;  /* any nonzero value will cause the flag to be set */
  optlen = sizeof( int );
  result = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, optlen );
  if( result != 0 ) {
#ifdef Verbose_Errors
    sserror("ServerSock: setsockopt()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }

  sp = (SOCKET *)malloc( sizeof(SOCKET));
  if( !sp ) {
#ifdef Verbose_Errors
    sserror("ServerSock: mallock()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }
  
  sp->addr.sin_family = AF_INET;
  sp->addr.sin_addr.s_addr = INADDR_ANY;
  sp->addr.sin_port = htons( (u_short)port_number );

  result = bind( sd, (struct sockaddr *)&(sp->addr), sizeof(struct sockaddr));
  if( result !=0 ) {
#ifdef Verbose_Errors
    sserror("ServerSock: bind()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }

  max_sd = sd >= max_sd ? sd : max_sd;
  sp->sd = sd;
  sp->_rcnt = 0;
  sp->_rptr = SS_NULL;
  sp->_rbase = SS_NULL;
  sp->_wcnt = 0;
  sp->_wptr = SS_NULL;
  sp->_wbase = SS_NULL;
  sp->_flag = 0;
  
  result = listen( sd, 5 );
  if( result != 0 ) {
#ifdef Verbose_Errors
    sserror("ServerSock: listen()",CONT);
#endif
    return (SOCKET *) SS_NULL;
  }

  AddSock( sp, &ss_sockbuff );
  
  return sp;
}

void SockArrayInit( sap )
SOCKARRAY *sap;
{
  int i;
  sap->n_sockets = 0;
  for( i = 0; i < MaxSockets; i++ )
    sap->spa[i] = SS_NULL;
  return;
}

/*	fclose.c
	
	Hubert Bartels
	
	August 8, 1985
	
	fclose closes the file and frees the entry in the _iob block.

        Modified for sockets by Mat Watson 12/1/90

*/
int SockClose(sp)
SOCKET *sp;
{
	int result;

  RemoveSock( sp );

  result = SockFlush(sp);      /* Flush the buffer if needed */
  if( result < 0 ) {
#ifdef Verbose_Errors
    sserror("SockClose(): SockFlush failed",CONT);
#endif
    return SS_EOF;
  }
  result = close((int)sp->sd);		/* Close the file */
  if( result < 0 )
    sserror("SockClose: close() failed",EXIT);
  sp->sd = '\0';
  sp->_rcnt = 0;
  sp->_rptr = SS_NULL;
  if( (sp->_flag & SS_IONBF) == 0 ) 
    if(sp->_rbase)
      free((char *)sp->_rbase);
  sp->_rbase = SS_NULL;

  sp->_wcnt = 0;
  sp->_wptr = SS_NULL;
  if( (sp->_flag & SS_IONBF) == 0 ) 
    if(sp->_wbase)
      free((char *) sp->_wbase );
  sp->_wbase = SS_NULL;
  sp->_flag = 0;
  return 0; /* Success */
}

/* SockFilbuf.c
   Modified for sockets by Mat Watson 12/01/90
*/
/*	filbuf.c
	
	Hubert Bartels
	
	July 8, 1985
	
	filbuf is used to fill the buffer of files being read by the
	polyp. As such, it is part of the file system of the polyp.
*/
int _SockFilbuf(sp)
SOCKET *sp;
{
  static char smallbuf[NOFILE];  /* for unbuffered i/o */

  if( (sp->_flag & (SS_IOEOF | SS_IOERR)) != 0 )
    return(SS_EOF);

  while (sp->_rbase == SS_NULL) /* find buffer space */
    if(sp->_flag & SS_IONBF) /* unbuffered */
      sp->_rbase = &smallbuf[sp->sd];
    else if ((sp->_rbase = calloc(SS_BUFSIZ, 1)) == SS_NULL)
      sp->_flag |= SS_IONBF;	/* Cannot get unbuffered */
    else
      sp->_flag |= SS_IOMYBUF;	/* Got a big buffer */

  sp->_rptr = sp->_rbase;
  sp->_rcnt = read(sp->sd, sp->_rptr,
		  sp->_flag & SS_IONBF ? 1 : SS_BUFSIZ);
  if(--sp->_rcnt < 0) {
    if(sp->_rcnt == -1 || errno == ECONNRESET ) /* ECONNRESET if socket */
      sp->_flag |= SS_IOEOF;                    /* was closed.          */
    else {
      sp->_flag |= SS_IOERR;
#ifdef Verbose_Errors
      sserror("SockFilbuf(): read()",CONT);
#endif
    }
    sp->_rcnt = 0;
    return(SS_EOF);
  }
  return(*sp->_rptr++ & 0377);	/* make character positive */
}
/* SockFlsbuf.c
   Modified for sockets by Mat Watson 12/01/90
*/
/*	flsbuf.c
	
	Hubert Bartels
	
	July 29, 1985
	
	Version 2.0
			Correct the bug, initialization of buffer
	flsbuf is used to flush the buffer of files being written by
	the polyp. As such, it is part of the file system of the polyp.
	_flsbuf may be called with unbuffered or buffered data. There
	may be data still in the buffers when the program completes. 
	either fflush or fclose should be called when closing the program
	or file, to enure that all the data has been read out.
*/

int _SockFlsbuf(x,sp)
char x;
SOCKET *sp;
{
	static char smallbuf[NOFILE];		/* for unbuffered i/o */

	if( (sp->_flag & SS_IOERR) != 0)
		return SS_EOF;
	if (sp->_wbase == SS_NULL) { /* find buffer space */
		if(sp->_flag & SS_IONBF) { /* unbuffered */
			sp->_wbase = &smallbuf[sp->sd];
		}
		else if ((sp->_wbase = malloc(SS_BUFSIZ)) == SS_NULL) {
			sp->_wbase = &smallbuf[sp->sd];
			sp->_flag |= SS_IONBF;	/* Cannot get unbuffered */
		}
		else
			sp->_flag |= SS_IOMYBUF;	/* Got a big buffer */
		sp->_wptr = sp->_wbase;		/* Initialize pointer */
		sp->_wcnt = (sp->_flag & SS_IONBF)?0:SS_BUFSIZ; /* Buffer size */
		if(sp->_flag & SS_IONBF) {		/* If unbuffered */
			write(sp->sd,&x, 1);
			sp->_wcnt = 0;
			}
		else {
		        *(sp)->_wptr++ = x;		/* Store char */
		        sp->_wcnt--;
		        }
		return 0;
	} /* End of sp->_base buffer allocation */
	if(sp->_flag & SS_IONBF) {		/* If unbuffered */
		write(sp->sd,&x, 1);
		sp->_wcnt = 0;
	} else {
		if(write(sp->sd,sp->_wbase,(SS_BUFSIZ-sp->_wcnt)) == -1)
			sp->_flag |= SS_IOERR;
		sp->_wcnt = SS_BUFSIZ;
		sp->_wptr = sp->_wbase;
		*(sp)->_wptr++ = x;
		sp->_wcnt--;
	}
	return 0;
}

/*  SockFlush.c
   Modified for sockets by Mat Watson 12/01/90
*/
/*	fflush.c
	
	Hubert Bartels
	
	July 29, 1985
	
	fflush is called to clear out the buffers to ensure that everything
	has been written. Called in the middle of the programs, it ensures
	that the information will be complete before finishing some long
	calculation. Called at the end, it performs the sync function for
	the program.
*/

int SockFlush(sp)
SOCKET *sp;
{
  int n_writen;
  char *my_base;

  if(
     /* (sp->_flag &SS_IOWRT) == 0  || */ /* Not Appropriate for Sock */
     (sp->_flag & SS_IOERR) != 0
     ) {
#ifdef Verbose_Errors
    sserror("SockFlush: SS_IOERR is set",CONT);
#endif
    return SS_EOF;				/* If errors in file */
  }
  if( sp->_wbase == SS_NULL )
    return 0;                                /* No buffer to flush */
  if((sp->_flag & SS_IONBF) == 0) {		/* If buffered */
    my_base = sp->_wbase;
    while( sp->_wcnt != SS_BUFSIZ ) {
      n_writen = write( sp->sd, my_base, (SS_BUFSIZ - sp->_wcnt) );
      if( n_writen < 0 ) {
        extern int errno;
        if( errno == EPIPE )
          return 0;
#ifdef Verbose_Errors
	sserror("SockFlush: write error",CONT);
	/* When the socket has been set non blocking and the socket
	   would otherwise block, write will return a -1 and errno
	   will be set to EWOULDBLOCK. */
	/* When write tries to send characters to a client or server
	   that has dissconnected errno is set to EPIPE */
#endif
        return SS_EOF;
      }
      sp->_wcnt += n_writen;
      my_base += n_writen;
    }
    sp->_wcnt = SS_BUFSIZ;
    sp->_wptr = sp->_wbase;
  }
  return 0;
}

/* SockGets.c
   Modified for sockets by Mat Watson 12/01/90
*/
/*	fgets.c

	read a newline-terminated string from device (file) dev

	Hubert Bartels
	August 12, 1985
*/

char *SockGets(s, n, sp)
char *s;
int n;
SOCKET *sp;
{
	int c;
	char *cs;

	cs = s;
        while ( --n > 0 && ( c = SockGetc( sp )) >= 0 ) {
		*cs++ = (char)c;
		if ( (char)c =='\n' )
			break;
	}
	if ( c < 0 && cs == s )
		return SS_NULL;
	*cs++ = '\0';
	return s;
}

/* SockIndex.c
   Writen by Mat Watson 12/01/90
*/

int SockIndex( sp, sap )
SOCKET *sp;
SOCKARRAY *sap;
{
  int i;

  for( i = 0; i < sap->n_sockets; i++ ) 
    if( sp == sap->spa[i] ) break;

  if( i == sap->n_sockets )
    i = -1;

  return i;
}

/* SockInit.c
   Writen by Mat Watson 12/01/90
*/
void SockInit( c )
char c;
{
  if( ss_server_flag == 0 && c == 's' )
      ss_server_flag = 1;

  if( ss_client_flag == 0 && c == 'c' )
      ss_client_flag = 1;

  if( ss_init_flag == 0 ) {
    ss_sockbuff.n_sockets = 0;
    ss_init_flag = 1;
    max_sd = 0;
    signal(SIGPIPE,SockSignal);
  }

  return;
}

/* SockIsRead.c
   Writen by Mat Watson 12/01/90
*/

int SockIsRead( sp )
SOCKET *sp;
{
  int result;
  struct timeval to;
  fd_set readfds;

  to.tv_sec = 0;
  to.tv_usec = 0;
  

  FD_ZERO( &readfds );
  FD_SET( sp->sd, &readfds );
  result = select( max_sd+1, &readfds, (fd_set *)0, (fd_set *)0, &to );
  if( result == -1 )
    sserror("SockSelect1(): select() returned -1",CONT);

  return FD_ISSET( sp->sd, &readfds ) ? 1 : 0;
}

int SockIsWrite( sp )
SOCKET *sp;
{
  int result;
  struct timeval to;
  fd_set writefds;

  to.tv_sec = 0;
  to.tv_usec = 0;
  

  FD_ZERO( &writefds );
  FD_SET( sp->sd, &writefds );
  result = select( max_sd+1, (fd_set *)0, &writefds, (fd_set *)0, &to );
  if( result == -1 )
    sserror("SockSelect1(): select() returned -1",CONT);

  return FD_ISSET( sp->sd, &writefds ) ? 1 : 0;
}

/*	SockPuts.c
	Modified for sockets by Mat Watson 12/01/90
*/
/*	fputs.c

	places a string on the output file pointed to by fp.

	Hubert Bartels
	August 12, 1985
*/
int SockPuts(s, sp)
SOCKET *sp;
char *s;
{
	int r;
	char c;

	while (c = *s++)
                r = SockPutc(c, sp);
	return(r);
}

int SockSelect( timeout, flag )
double timeout;
char *flag;
{
  int i;
  int result;
  long seconds;
  long microseconds;
  SOCKARRAY * sap;
  struct timeval to, *tvp;
  fd_set *rfdsp, *wfdsp, *efdsp;
  char *cp;

  sap = &ss_sockbuff;

  if( timeout < 0.0 ) {
    tvp = SS_NULL;
  }
  else {
    seconds = timeout / 1.0;
    microseconds = (timeout - (double)seconds) / 1e-6;
    to.tv_sec = seconds;
    to.tv_usec = microseconds;
    tvp = &to;
  }

  rfdsp = (fd_set *)0;
  wfdsp = (fd_set *)0;
  efdsp = (fd_set *)0;

  for( cp = flag; *cp != '\0'; cp++ ) {
    if( *cp == 'r' ) {
      rfdsp = &(sap->readfds);
      FD_ZERO( rfdsp );
      for( i = 0; i < sap->n_sockets; i ++ )
	FD_SET( sap->spa[i]->sd, rfdsp );
    } 
    else if( *cp == 'w' ) {
      wfdsp = &(sap->writefds);
      FD_ZERO( wfdsp );
      for( i = 0; i < sap->n_sockets; i ++ )
	FD_SET( sap->spa[i]->sd, wfdsp );
    }
    else if( *cp == 'e' ) {
      efdsp = &(sap->exceptfds);
      FD_ZERO( efdsp );
      for( i = 0; i < sap->n_sockets; i ++ )
	FD_SET( sap->spa[i]->sd, efdsp );
    }
    else {
      sserror("SockSelect(): Unknown flag",CONT);
    }
  }

  result = select( max_sd+1, rfdsp, wfdsp, efdsp, tvp );
  if( result == -1 )
    perror("SockSelect(): select() returned -1");

  return result;
}

void SockSignal( sig )
int sig;
{
  switch ( sig )
    {
    case SIGPIPE :
#ifdef Verbose_Signals
      sserror(
        "Caught a SIGPIPE signal, Server/Client probably disconnected",
	 CONT );
#endif
      break;
    }
  return;
}

/* SockWrites
   Writen by Mat Watson 12/01/90
   Based on fputs.c by Hubert Bartels.
*/
int SockWrites(s, sp)
SOCKET *sp;
char *s;
{
	int r;
	char c;

	while (c = *s++)
                r = SockPutc(c, sp);
	r = SockFlush( sp );
	return(r);
}

void sserror( mesg, code )
char * mesg;
int code;
{
  fprintf(stderr,"%s",mesg);
  fputc(' ',stderr);
  fflush(stderr);
  perror(NULL);
  if ( code == EXIT ) exit( 1 );
  return;
}
