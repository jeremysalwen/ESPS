/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1995-1996 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:   Dave Talkin
 * Checked by:
 * Revised by:   Alan Parker
 *
 * Brief description: provides IPC functions for waves and attachments
 *
 */

static char *sccs_id = "@(#)message.c	1.19 1/18/97 ERL";

#include <stdio.h>
#ifdef LINUX_OR_MAC
#include <sys/ioctl.h>
#else
#include <sgtty.h>
#endif
#include <Methods.h>
#include <w_lengths.h>
#include <esps/xwaves_ipc.h>
#include <xview/xview.h>

#define TRUE 1
#define FALSE 0

char            ok[] = "ok", null[] = "null";
char           *get_next_item(), *get_next_line(), *savestring();
char           *get_methods(), *make_new_object();

static char    *builtin_default_execute_proc();
extern char    *exec_waves();
extern char    notice_msg[];
static Notify_value process_d_q();

/* This gets called if a message receiver can not be found. */
static char    *(*default_proc) () = builtin_default_execute_proc;

/*
 * This stores information about what to do after a waves command is
 * executed.  The caller ID, some data and a procedure pointer are stored to
 * accomplish the callback.
 */
typedef struct pending_calls {
   int             id;
   void            (*proc) ();
   void           *data;
   struct pending_calls *next, *prev;
}               PendCalls;

/* The master list of pending callbacks. */
static PendCalls *pring = NULL;
static int      pending_call_id = 1;

/* The master list of items on the incomingd ispatch queue. */
static StrList *dq = NULL;

/* Message buffers for incoming and outgoing IPC messages. */
static char     in[1000], out[1000], *pin = in, *pout = out;

extern int      w_verbose, debug_level;
extern char    *thisprog;	/* points to exact invocation name for this
				 * program. */
/* these are all set in setup_ipc */

Display        *X_Display = NULL;
Window          comm_window = None;
char           *registry_name = NULL;
char           *remote_name = NULL;
Window          remote_comm_window = None;


/***********************************************************************
   get_receiver_name(), get_methods(), and get_receiver() must be defined to
   mesh the application specific structures to (char*).
   ***********************************************************************/


/***********************************************************************/
char           *(*
		 install_default_execute_proc(newproc)) ()
   char           *(*newproc) ();
{
   char           *(*hold) () = default_proc;

   default_proc = newproc;
   return (hold);
}

/***********************************************************************/
static char    *
builtin_default_execute_proc(who, str)
   char           *who, *str;
{
   char            name[500];
   strnscan(str, name, sizeof(name));
   sprintf(notice_msg, "Unknown command name %s; command ignored", name);
   show_notice(1,notice_msg);
   return (null);
}


static void    *command_step_caller = NULL;
static char    *command_step_resp = NULL;

/*********************************************************************/
void
prepare_to_resume_ipc(caller, data)
   void           *caller;
   char           *data;
{
   command_step_caller = caller;
   command_step_resp = data;
}

/***********************************************************************/
resume_ipc_if_stepping()
{
   do_ipc_response_if_any(command_step_caller, command_step_resp);
   command_step_caller = NULL;
   command_step_resp = NULL;
}

/***********************************************************************/
char           *
execute_command(wh, methods, str)
   register char  *wh;
   register Methods *methods;
   register char  *str;
{
   char            name[100], str2[MES_BUF_SIZE];
   register int    n;
   register Methods *m2;
   extern int      command_step;/* whether or not to single-step command
				 * files */
   extern int      command_paused;	/* whether or not to pause after
					 * command */

   if (debug_level)
   {
      if (str)
	 fprintf(stderr, "execute_command: entered with str = \"%s\"\n", str);
      else
	 fprintf(stderr, "execute_command: entered with str = NULL\n");
   }

   if (str && *str)
   {
      if (!wh) {
	 strcpy(str2, str);
	 if ((wh = (char *) make_new_object(str))) {
	    methods = (Methods *) get_methods(wh);
	    str = get_next_item(str2);
	 }
	 if (debug_level)
	    fprintf(stderr, "execute_command: str = %s\n", str);
      }
      if (w_verbose > 1)
	 fprintf(stderr, "Executing command: %s\n", str);

      if (wh && str && *str) {
	 strnscan(str, name, sizeof(name));
	 n = strlen(name);
	 while (methods) {
	    if (!strncmp(name, methods->name, n)) {	/* found a match */
	       if (strlen(methods->name) != n) {	/* an exact match? */
		  m2 = methods->next;	/* check for uniqueness */
		  while (m2) {
		     if (!strncmp(name, m2->name, n)) {
			sprintf(notice_msg,
				"Non-unique command name %s; command ignored",
				name);
			show_notice(1,notice_msg);
			return (null);	/* found a dup. */
		     }
		     m2 = m2->next;
		  }
	       }
	       if (command_step) {
		  void     *in_a_ipc_dispatch(), *hand = in_a_ipc_dispatch();
		  char     *resp;

		  command_paused = 1;
		  (void) fprintf(stderr, "command: %s", str);
		  if (str[strlen(str) - 1] != '\n')
		     (void) fprintf(stderr, "\n");
		  resp = methods->meth(wh, get_next_item(str));
		  prepare_to_resume_ipc(hand, resp);
		  return (resp);
	       } else
		  return (methods->meth(wh, get_next_item(str)));
	    }
	    methods = methods->next;
	 }				/* fall-through to last-resort */

	 if (default_proc)
	    return (default_proc(wh, str));
      }
   }
   return (null);
}

/***********************************************************************/
set_return_value_callback(proc, data)
   void            (*proc) ();
   void           *data;
{
   if (proc) {
      PendCalls      *pc = (PendCalls *) malloc(sizeof(PendCalls));
      if (pc) {
	 if (pring) {
	    pc->next = pring->next;
	    pring->next->prev = pc;
	    pring->next = pc;
	    pc->prev = pring;
	 } else {
	    pring = pc;
	    pc->next = pc;
	    pc->prev = pc;
	 }
	 pc->proc = proc;
	 pc->data = data;
	 pc->id = pending_call_id;
	 pending_call_id++;
	 if (!pending_call_id)
	    pending_call_id = 1;
	 if (debug_level)
	    fprintf(stderr, "Set callback id:%d %x %x\n", pc->id, proc, data);
	 return (pc->id);
      } else
	 fprintf(stderr, "Allocation failure in set_return_callback()\n");
   } else
      fprintf(stderr, "NULL proc passed to set_return_callback()\n");
   return (0);
}

/***********************************************************************/
do_return_callback(id, str)
   int             id;
   char           *str;
{
   if (id) {
      PendCalls      *pc0 = pring, *pc = pc0;
      void            (*pr) ();
      void           *dat;
      if (debug_level)
	 fprintf(stderr, "Do callback id:%d;%s\n", id, (str) ? str : null);
      if (pring) {
	 do {
	    if (pc->id == id) {
	       if (pc->next != pc) {
		  pc->prev->next = pc->next;
		  pc->next->prev = pc->prev;
		  pring = pc->next;
	       } else
		  pring = NULL;
	       dat = pc->data;
	       pr = pc->proc;
	       free(pc);
	       if (debug_level)
		  fprintf(stderr, "Entering callback id:%d %x %x\n", id, pr, dat);
	       pr(dat, str);
	       if (debug_level)
		  fprintf(stderr, "Done with callback id:%d\n", id);
	       return (TRUE);
	    }
	    pc = pc->next;
	 } while (pc != pc0);
      } else {
	 fprintf(stderr, "No more callback entries!\n");
	 fprintf(stderr, "Failed to find callback id:%d!\n", id);
	 return (FALSE);
      }
   } else {
      fprintf(stderr, "Zero ID passed to do_return_callback()\n");
      return (FALSE);
   }
}

/***********************************************************************/
char           *
dispatch(str)
   char           *str;
{
   char           *wh, *get_receiver(), *get_receiver_name(), *get_methods(),
                   cd, *cdp;

   if (debug_level)
   {
      if (str)
	 fprintf(stderr, "dispatch: entered with str = \"%s\"\n", str);
      else
	 fprintf(stderr, "dispatch: entered with str = NULL\n");
   }

   if (!str)
      return null;

   if (str && (((cd = *str) == '"') || (cd == '\''))) {
      cdp = ++str;
      while (*cdp && (*cdp != cd))
	 cdp++;
      if (*cdp)
	 *cdp = ' ';
   }
   if ((wh = get_receiver(str))) {
      if (!*(str = get_next_item(str))) {
	 if (debug_level)
	    fprintf(stderr, "null message to %s\n", get_receiver_name(wh));
	 return (null);
      }
      return (execute_command(wh, get_methods(wh), str));
   }
   if (debug_level)
      fprintf(stderr, "%s was not found; will try to create it.\n", str);
   return (execute_command(NULL, get_methods(NULL), str));

}

#define ITIMER_NULL ((struct itimerval*)0)
static int      under_xview = 1;
static void     (*sigsave) () = NULL;

/*************************************************************************/
set_no_xview()
{
   under_xview = 0;
}

/*************************************************************************/
restart_d_clock(usec, who, proc)
   int             usec, who;
   void            (*proc) ();
{
   struct itimerval tim, otim;

   tim.it_interval.tv_usec = 0;
   tim.it_interval.tv_sec = 0;
   tim.it_value.tv_usec = usec % 1000000;
   tim.it_value.tv_sec = usec / 1000000;
   if (under_xview)
      notify_set_itimer_func(who, proc, ITIMER_REAL, &tim,
			     ITIMER_NULL);
   else {
      void            (*sav) ();

      setitimer(ITIMER_REAL, &tim, &otim);
      sav = signal(SIGALRM, proc);
      if (!sigsave)
	 sigsave = sav;
   }
}

/*************************************************************************/
stop_d_clock(who)
   int             who;
{

   if (under_xview)
      notify_set_itimer_func(who, process_d_q, ITIMER_REAL, ITIMER_NULL,
			     ITIMER_NULL);
   else {
      signal(SIGALRM, sigsave);
      sigsave = NULL;
   }
}

static StrList *this_request = NULL;
/*************************************************************************/
/*
 * The idea here is that xwaves can grab the information required to respond
 * to a caller before launching any asynchronous process needed to satisfy
 * the caller's request.  When the asynchronous process causes its callback
 * to run, the result notice can finally be returned to the original caller.
 */
void           *
in_a_ipc_dispatch()
{
   if (this_request) {
      Win_data       *xs = (void *) this_request->data;
      this_request->data = NULL;
      return ((void *) xs);
   } else
      return (NULL);
}

/*************************************************************************/
/*
 * It is possible that a client that requested some asynchronous processing
 * via xwaves has died before the request was satisfied.  In this case,
 * xwaves will crash and burn if an attempt is made to use the stale handle.
 * This routine attempts to detect corruption is the structure that would
 * indicate abandonment.  Not a sure thing, but better than nothing...
 */
static int
sanity_check(xs)
   Win_data       *xs;
{
   return (xs && ((Window) xs->caller != None));
}

/*************************************************************************/
do_ipc_response_if_any(xs, resp)
   Win_data       *xs;
   char           *resp;
{
   if (sanity_check(xs)) {
      if (resp && *resp)
	 strcpy(out, resp);
      else
	 strcpy(out, null);
      if (!Send_X_Reply(X_Display, NULL, xs->caller, xs->serial, out,
			strlen(out))) {
	 sprintf(notice_msg,
		 "Problems in %s sending completion IPC response(%s).",
		 thisprog, pout);
         show_notice(1,notice_msg);
      }
   }
}

/*************************************************************************/
static          Notify_value
process_d_q()
{
   StrList        *dq2;

   stop_d_clock(5471420);

   while (dq) {
      if (debug_level)
	 fprintf(stderr, "%s dispatching IPC request:%s\n", thisprog, dq->str);
      this_request = dq;
      strcpy(out, exec_waves(dq->str));
      if (dq->data && dq->data->caller) {
	 pout = out;
	 if (dq->data->tk) {	/* if caller is TK, removed "returned" */
	    if (!strncmp("returned", out, 8)) {
	       pout += 9;
	    }
	 }
	 if (!Send_X_Reply(X_Display, NULL, dq->data->caller,
			   dq->data->serial, pout, strlen(pout))) {
	    sprintf(notice_msg,
		    "Problems in %s sending delayed IPC response(%s).",
		    thisprog, out);
            show_notice(1,notice_msg);
         }
      }
      pout = out;		/* just in case somebody assumes that it ==
				 * out */
      this_request = NULL;
      free(dq->str);
      free(dq->data);
      dq2 = dq->next;
      free(dq);
      dq = dq2;
   }
   if (under_xview)
      return (NOTIFY_DONE);
}

/*************************************************************************/
/*
 * Messages may have come in before an "attachment" was fully connected. This
 * is called when the attachment is complete and transfers those pending
 * requests to the xwaves dispatch queue from which they are sent to the
 * relevant attachment.
 */
transfer_to_dispatch_q(l)
   StrList        *l;
{
   if (l) {
      StrList        *dqp = dq;
      if (dqp) {
	 while (dqp->next)
	    dqp = dqp->next;
	 dqp->next = l;
      } else
	 dq = l;
   }
}

/*************************************************************************/
StrList        *
new_disp_ent(str)
   char           *str;
{
   StrList        *q;
   if (str && *str && (q = (StrList *) malloc(sizeof(StrList)))) {
      q->str = str;
      q->next = NULL;
      q->data = NULL;
      return (q);
   }
   return (NULL);
}

/*************************************************************************/
/*
 * In most cases, it is preferable to defer processing of incoming xwaves
 * commands until after the IPC dispatch process is complete. This places
 * commands that can be handled this way on the queue.
 */
static char    *
delay_dispatch(str, data)
   char           *str;
   void           *data;
{
   if (str && *str) {
      StrList        *dqp, *qp2;

      if ((dqp = new_disp_ent(savestring(str)))) {
	 dqp->data = data;
	 if (!dq)
	    dq = dqp;
	 else {
	    qp2 = dq;
	    while (qp2->next)
	       qp2 = qp2->next;
	    qp2->next = dqp;
	 }
	 restart_d_clock(10000, 5471420, process_d_q);
	 return (ok);
      } else
	 fprintf(stderr, "%s: Allocation failure in delay_dispatch()\n", thisprog);
   }
   return (null);
}

/*************************************************************************/
/*
 * Run any program as a background job.  "command_line" holds the full
 * command to be backgrounded.  One use for this is to start "attachments".
 */
background_a_program(command_line)
   char           *command_line;
{
   char           *args[5];
   int             pid;

   /* set execvp args */
   args[0] = "/bin/sh";
   args[1] = "-c";
   args[2] = command_line;
   args[3] = "&";
   args[4] = NULL;

   /* Fork a new process to run the command. */

#if defined(SUN4) || defined(SUN3) || defined(DS3100)
   switch (pid = vfork()) {
#else
   switch (pid = fork()) {
#endif
   case -1:			/* fork failed */
      perror("background_a_program; fork failed");
      return (FALSE);
   case 0:			/* child process */
      setpgrp(0, getpid());
      execvp(args[0], args);
      perror("background_a_program: execvp failed");
      return (FALSE);
   default:			/* parent */
      return (TRUE);
   }
}

/*************************************************************************/
free_list(l)
   StrList        *l;
{
   StrList        *l2;

   while (l) {
      if (l->str) {
	 free(l->str);
      }
      l2 = l->next;
      free(l);
      l = l2;
   }
}

/*************************************************************************/
/*
 * This is the main IPC dispatch procedure.  It returns immediately or after
 * completion of the actual request, depending on the value of "procedure
 * number" used in the clnt_call() issued by the client program.  Unless
 * absolutely necessary (e.g. to obtain data) the client call should specify
 * IMMEDIATE RESPONSE which permits asynchronous processing of the request.
 */
static void
dispatch_ipc(dest, caller, serial, req, in)
   char           *dest;
   Window          caller;
   int             serial;
   int             req;
   char           *in;
{
   int             res;

   if (debug_level)
      fprintf(stderr, "%s got an IPC request(%s)\n", thisprog, in);

   if (in && *in) {
      switch (req) {
      case RPCINFO_CALL:
	 fprintf(stderr, "Program %s Lives!\n", thisprog);
	 strcpy(out, "all's well");
	 break;
      default:
	 fprintf(stderr, "Unrecognized req in %s IPC dispatch\n", thisprog);
      case IMMEDIATE_RESPONSE:
	 strcpy(out, delay_dispatch(in, NULL));
	 break;
      case INTERLOCK_RESPONSE:
      case INTERLOCK_NOREPLY:
	 strcpy(out, exec_waves(in));
	 break;
      case INTERLOCK_DELAY_RESPONSE:
	 {
	    Win_data       *xptr = (Win_data *) (malloc(sizeof(Win_data)));
	    xptr->caller = caller;
	    xptr->serial = serial;
	    xptr->tk = 0;
	    delay_dispatch(in, xptr);
	    return;
	 }
      case INTERLOCK_DELAY_RESPONSE_TK:
	 {
	    Win_data       *xptr = (Win_data *) (malloc(sizeof(Win_data)));
	    xptr->caller = caller;
	    xptr->serial = serial;
	    xptr->tk = 1;	/* this signals that the caller is TK */
	    /* It causes the "returned" to be removed */
	    delay_dispatch(in, xptr);
	    return;
	 }
      }
   } else {
      fprintf(stderr, "Null request in %s IPC dispatch\n", thisprog);
      strcpy(out, null);
   }
   if (req != INTERLOCK_NOREPLY && caller)
      if (!Send_X_Reply(X_Display, NULL, caller, serial, out, strlen(out))) {
	 sprintf(notice_msg, "Problems in %s sending IPC response(%s).",
		 thisprog, pout);
         show_notice(1,notice_msg);
      }
   return;
}

static void
event_proc(window, event, arg)
   Xv_Window       window;
   Event          *event;
   Notify_arg      arg;
{
   XEvent         *p;
   int             ret;
   Window          win;

   p = event->ie_xevent;
   win = (Window) xv_get(window, XV_XID);
   if (win == comm_window)
      Read_X_Comm(X_Display, win, p, 1, NULL, 1024, dispatch_ipc);
}

char           *
setup_ipc(frame, name)
   Frame           frame;
   char           *name;
{
   char           *Setup_X_Comm();

   X_Display = (Display *) xv_get(frame, XV_DISPLAY);
   comm_window = (Window) xv_get(frame, XV_XID);

   xv_set(frame, WIN_X_EVENT_MASK, NoEventMask, NULL);
   xv_set(frame, WIN_EVENT_PROC, event_proc,
	  WIN_X_EVENT_MASK, PropertyChangeMask,
	  NULL);

   return Setup_X_Comm(name, X_Display, comm_window);
}




/*************************************************************************/
int
setup_attach_comm(frame, dest, myname)
   char           *dest, *myname;
   Frame           frame;
{
   Window          Get_X_Comm_Win();
   registry_name = setup_ipc(frame, myname);

   if (registry_name) {
      remote_name = dest;
      remote_comm_window = Get_X_Comm_Win(X_Display, dest);
      if (remote_comm_window == None) {
	 show_notice(1, "setup_attach_comm: can't get remote comm window");
	 return 0;
      }
   } else
      show_notice(1, "setup_attach_comm: setup_ipc failed!");
   return (registry_name != NULL);
}

void
send_start_command(command)
   char           *command;
{
   extern char    *host;
   int             res;

   sprintf(in, "%s start function %s registry %s command %s",
	   host, thisprog, registry_name, command);
   res = Send_X_Msg(X_Display, remote_name, None,
		    IMMEDIATE_RESPONSE, in, strlen(in));

   if (!res) {
      sprintf(notice_msg, "Problem sending startup message:\n%s", in);
      show_notice(1,notice_msg);
   }
}



/*************************************************************************/
terminate_communication(name)
   char           *name;
{
   if (debug_level)
      fprintf(stderr, "terminate_communication()\n");
   if (X_Display && name) {
      Close_X_Comm(X_Display, name);
      remote_name = NULL;
   }
}

/*************************************************************************/
mess_write(str)
   char           *str;
{
   return (mess_send(str, IMMEDIATE_RESPONSE));
}

/*************************************************************************/
terminal_message(str)
   char           *str;
{
   return (mess_send(str, INTERLOCK_NOREPLY));
}

/*************************************************************************/
mess_send(str, type)
   char           *str;
   int             type;
{
   int             res;

   if (remote_comm_window != None && str && *str) {

      res = Send_X_Msg(X_Display, remote_name, remote_comm_window, type, str,
		       strlen(str));

      if (!res) {
	 if (debug_level) {
	    fprintf(stderr, "mess_send failed on: %s\n", str);
	 }
	 return (FALSE);
      }
      return (TRUE);
   } else if (remote_comm_window == None)
      fprintf(stderr,
	      "No attachments are registered; ignoring send request.\n");
   else
      fprintf(stderr, "%s: bad args to mess_send()\n", thisprog);
   return (FALSE);
}
