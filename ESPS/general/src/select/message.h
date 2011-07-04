/*---------------------------------------------------------------------------+
| This material contains proprietary software of Entropic Speech, Inc.   |
| Any reproduction, distribution, or publication without the the prior	     |
| written permission of Entropic Speech, Inc. is strictly prohibited.    |
| Any public distribution of copies of this work authorized in writing by    |
| Entropic Speech, Inc. must bear the notice			     |
|									     |
|              "Copyright 1986 Entropic Speech, Inc."		     |
|  									     |
| Written by:	Joseph T. Buck						     |
|									     |
| Module:	message.h						     |
|									     |
|  These macros are used for messages with arguments. All user messages must |
|  ultimately be written by message or errmsg to go to the log file. msgbuf  |
|  is used to form the error message.					     |
|  									     |
| Revision history:							     |
|  	1.1	created							     |
|	1.2	support USG (Sys III, V): sprintf returns int, not char *    |
|	2.1	frozen for release					     |
----------------------------------------------------------------------------*/
/* %W% %G% ESI */
extern char msgbuf[];
#if defined(M5500) || defined(M5600) || defined(hpux) || defined(SG) ||  defined(IBM_RS6000) || defined(DEC_ALPHA) || defined(OS5) || defined(LINUX_OR_MAC)
#define message1(f,s) sprintf(msgbuf,f,s), message(msgbuf)
#define message2(f,a,b) sprintf(msgbuf,f,a,b), message(msgbuf)
#define message3(f,a,b,c) sprintf(msgbuf,f,a,b,c), message(msgbuf)
#define errmsg1(f,s) sprintf(msgbuf,f,s), errmsg(msgbuf)
#define errmsg2(f,a,b) sprintf(msgbuf,f,a,b), errmsg(msgbuf)
#define errmsg3(f,a,b,c) sprintf(msgbuf,f,a,b,c), errmsg(msgbuf)
#define errmsg4(f,a,b,c,d) sprintf(msgbuf,f,a,b,c,d), errmsg(msgbuf)
#else
char *sprintf ();
#define message1(f,s) message(sprintf(msgbuf,f,s))
#define message2(f,a,b) message(sprintf(msgbuf,f,a,b))
#define message3(f,a,b,c) message(sprintf(msgbuf,f,a,b,c))
#define errmsg1(f,s) errmsg(sprintf(msgbuf,f,s))
#define errmsg2(f,a,b) errmsg(sprintf(msgbuf,f,a,b))
#define errmsg3(f,a,b,c) errmsg(sprintf(msgbuf,f,a,b,c))
#define errmsg4(f,a,b,c,d) errmsg(sprintf(msgbuf,f,a,b,c,d))
#endif
void message(),errmsg();
