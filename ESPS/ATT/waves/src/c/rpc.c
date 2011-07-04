/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc.  Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1995  Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  David Talkin Revised by:  Alan Parker for X communications
 * 
 * Brief description: IPC message handling between waves and attachments
 * 
 */

/* ipc.c */

static char    *sccs_id = "@(#)rpc.c	1.20 2/12/96 ERL";

#include <Objects.h>
#include <esps/xwaves_ipc.h>

/*************************************************************************/
/*
 * The IPC mechanism used is based upon TK 4.  This uses communications via
 * the X server.  See xsend.c for details.   Here after this is refered to
 * just as IPC.
 * 
 * External "functions" are "attached" to xwaves as follows: A program is
 * started in the background with arguments that tell it how to get in touch
 * with xwaves via IPC.  Xwaves then makes an entry in a table indicating
 * that the external program may be reachable at some future time.  Xwaves
 * then goes on about it's business.  If a "send" request comes in for the
 * external program, xwaves goes to the table.  If a named entry is found,
 * but the connection is still pending, the send request is queued and xwaves
 * resumes.
 * 
 * When the external program finally starts running, it asks xwaves to register
 * it as a living function. Its IPC registry name is placed in a structure
 * associated with its func name.  Via add_op commands, the attachment may
 * create appropriate entry or entries in the menus. These are the command(s)
 * it will respond to in ot's specialized way (e.g. xlabel would create an
 * add_op that did "send function xlabel mark time _cursor_time freq
 * _cursor_yval").  If "send" requests for the function are pending, they are
 * processed immediately. Communication in both directions is via IPC.
 * 
 * Note that attachments may also be started without directly involving xwaves.
 * They will try to reach xwaves at its default name.  If they succeed  the
 * remainder of the actions mentioned above are carried out.
 */

extern char    *checking_selectors(), *apply_esps_bin_path(), *basename();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }

char           *savestring(), *dispatch(), *delay_dispatch();


static Attachment *att_list = NULL;

extern char    *registry_name;
extern Display *X_Display;

extern int      w_verbose;
extern int      debug_level;
extern char     ok[], null[];
extern char    *thisprog;

/*************************************************************************/
Attachment     *
new_attachment(name, menu)
   char           *name, *menu;
{
   Attachment     *a;
   if ((a = (Attachment *) malloc(sizeof(Attachment)))) {
      if (!menu || !*menu)
	 menu = "all";
      if (name && *name) {
	 if (!(a->progname = savestring(name))) {
	    fprintf(stderr, "Name allocation problems in new_attachment()\n");
	    free(a);
	    return (NULL);
	 }
      }
      a->ready = 0;
      a->registry_name = NULL;
      a->pending = NULL;
      a->next = NULL;
      return (a);
   } else
      fprintf(stderr, "Can't allocate a new Attachment\n");
   return (NULL);
}


/*************************************************************************/
start_external_function(newfun)
   char           *newfun;
{
   Attachment     *a;
   char            commandline[MES_BUF_SIZE], tmpwavpro[NAMELEN], *newfun_full;
   extern Object   program;
   extern char     funcname[];
   /* extern Panel_item newFunct_item; */

   if (strlen(newfun)) {
      newfun_full = apply_esps_bin_path(NULL, newfun);
      if (newfun_full)
	 strcpy(funcname, newfun_full);
      else {			/* no luck finding the function */
	 fprintf(stderr, "Can't find attachment (%s)\n", newfun);
	 return (FALSE);
      }
   } else {
      funcname[0] = 0;
   }
   /* panel_set_value(newFunct_item, funcname); */
   if (!*funcname)
      return (FALSE);

   terminate(funcname);

   if ((a = new_attachment(newfun, NULL))) {
      /* Write out globals to a temporary file in the user's home directory. */
      build_filename(tmpwavpro, "", "$HOME/.xwave_tmp");
      sprintf(commandline, "attributes global output %s", tmpwavpro);
      meth_get(&program, commandline);

      sprintf(commandline, "%s -w %s -nwaves -c \"%s\"\n",
	      a->progname, tmpwavpro, registry_name);

      if (background_a_program(commandline)) {
	 a->next = att_list;
	 att_list = a;
	 return (TRUE);
      } else {
	 fprintf(stderr, "Can't start the function named %s\n", a->progname);
	 free_attachment(a);
      }
   }
   return (FALSE);
}

/*************************************************************************/
Attachment     *
find_attachment(func)
   char           *func;
{
   Attachment     *a = att_list;

   if (func) {
      while (a) {
	 if (!strcmp(func, a->progname))
	    return (a);
	 a = a->next;
      }
   }
   return (NULL);
}

/*************************************************************************/
static char     func[NAMELEN], command[MES_BUF_SIZE];
static char     registry[NAMELEN];
static Selector s0 = {"command", "#strq", command, NULL},
                s1b = {"op", "#strq", command, &s0},
                s1 = {"function", "%s", func, &s1b},
                s2 = {"registry", "%s", registry, &s1};
/***********************************************************************/
char           *
meth_send(o, str)
   Object         *o;
   char           *str;
{
   Attachment     *a;
   int             result;

   registry[0] = NULL;
   CHECK_QUERY(str, &s2);
   if ((a = att_list) && str && *str) {

      registry[0] = NULL;
      sscanf(str, "%s", func);
      if (strcmp(func, "function")) {	/* old-style send? */
	 strcpy(func, a->progname);
	 sprintf(command, "%s %s", a->progname, str);
      } else
	 get_args(str, &s2);

      if ((a = find_attachment(func))) {
	 if (a->ready) {	/* is attachment complete? */
	    result = Send_X_Msg(X_Display, a->registry_name, None,
			      IMMEDIATE_RESPONSE, command, strlen(command));
	    if (!result) {
	       fprintf(stderr, "meth_send: message not sent to %s (%s)\n",
		       a->registry_name, command);
	       return (null);
	    }
	    if (debug_level) {
	       fprintf(stderr, "%s meth_send(%s) ok\n", thisprog, command);
	       return (ok);
	    }
	    return (ok);
	 } else {		/* queue the request */
	    if (add_to_ipc_send_que(a, str))
	       return (ok);
	    else
	       return (null);
	 }
      }
   } else if (!att_list)
      fprintf(stderr, "No attachments are registered; ignoring send request.\n");
   else
      fprintf(stderr, "Bad args to meth_send()\n");
   return (null);
}

/*************************************************************************/
add_to_ipc_send_que(a, str)
   Attachment     *a;
   char           *str;
{
   static char     mh[] = "waves send ";

   if (a && str && *str) {
      char           *mes;
      StrList        *de, *ap, *new_disp_ent();

      if ((mes = malloc(strlen(mh) + strlen(str) + 1))) {
	 sprintf(mes, "%s%s", mh, str);
	 if ((de = new_disp_ent(mes))) {
	    if ((ap = a->pending)) {
	       while (ap->next)
		  ap = ap->next;
	       ap->next = de;
	    } else
	       a->pending = de;
	    return (TRUE);
	 } else
	    fprintf(stderr, "Allocation problems in add_to_ipc_send_que()\n");
      } else
	 fprintf(stderr, "Allocation problems(2) in add_to_ipc_send_que()\n");
   } else
      fprintf(stderr, "Bad args to add_to_ipc_send_que()\n");
   return (FALSE);
}

/*************************************************************************/
char           *
meth_start_attachment(o, str)
   Object         *o;
   char           *str;
{
   extern char     null[], ok[];

   registry[0] = NULL;
   if (get_args(str, &s2) == 3) {	/* must get: func,registry,command */
      Attachment     *a;

      if (!(a = find_attachment(func))) {
	 if ((a = new_attachment(func, NULL))) {
	    a->next = att_list;
	    att_list = a;
	 } else
	    return (null);
      }
      a->ready = 1;
      a->registry_name = savestring(registry);
      transfer_to_dispatch_q(a->pending);
      a->pending = NULL;
      return (dispatch(receiver_prefixed(command)));
   } else
      fprintf(stderr, "Bad args to meth_start_attachment (%s)\n", str);
   return (null);
}

void
print_att()
{
   Attachment     *a = att_list;
   fprintf(stderr, "Print attach list:\n");
   while (a) {
      fprintf(stderr,
	      "progname: %s, registry: %s\n", a->progname, a->registry_name);
      a = a->next;
   }
}

/***********************************************************************/
char           *
meth_get_attach_list(o, str)
   Object         *o;
   char           *str;
{
   Attachment     *a = att_list;
   static char    *oblock = NULL;
   static int      oblocksize = 0;
   int             used = 0;

   while (a) {

      if (a->progname) {
	 if (!used)
	    block_build(&oblock, &used, &oblocksize, "returned ");
	 else
	    block_build(&oblock, &used, &oblocksize, " ");

	 block_build(&oblock, &used, &oblocksize, a->progname);
      }
      a = a->next;
   }
   if (used)
      return (oblock);
   else
      return (null);
}

/*************************************************************************/
free_attachment(a)
   Attachment     *a;
{

   if (a) {
      if (a->progname) {
	 free(a->progname);
	 a->progname = NULL;
      }
      if (a->registry_name) {
	 free(a->registry_name);
	 a->registry_name = NULL;
      }
      free_list(a->pending);
      free(a);
   }
}

/*************************************************************************/
clobber_function(funcname)
   char           *funcname;
{
   extern Menuop  *find_operator();
   void            reset_attach_choice();
   Menuop         *mo;
   Moplist        *menu_get_op_lists(), *mol = menu_get_op_lists();

   while (mol) {
      if ((mo = find_operator(mol->first_op, basename(funcname))))
	 mo->proc = NULL;
      mol = mol->next;
   }

   menu_change(basename(funcname), "", XV_NULL, "");
   reset_attach_choice(basename(funcname));
   return;
}

/***********************************************************************/
detach_func(a)
   Attachment     *a;
{

   if (a->ready) {
      int             result;
      char            mess[NAMELEN];

      sprintf(mess, "%s quit", a->progname);

      result = Send_X_Msg(X_Display, a->registry_name, None,
			  IMMEDIATE_RESPONSE, mess, strlen(mess));
      if (!result) {
	 fprintf(stderr, "detach_func: trouble sending message to %s(%s)\n",
		 a->registry_name, mess);
	 return;
      }
      if (debug_level) {
	 fprintf(stderr, "%s detach_func(%s) ok\n", thisprog, mess);
      }
      clobber_function(basename(a->progname));
      a->ready = 0;
   }
}

/***********************************************************************/
destroy_function_client(function)
   char           *function;
{
   if (function && *function) {
      Attachment     *a, *a2, *prev;

      if ((a = att_list)) {
	 while (a) {
	    if (a->progname && !strcmp(a->progname, function)) {
	       clobber_function(basename(a->progname));
	       a->ready = 0;
	       a2 = a->next;
	       if (att_list == a)
		  att_list = a2;
	       else
		  prev->next = a->next;
	       free_attachment(a);
	       a = a2;
	    } else {
	       prev = a;
	       a = a->next;
	    }
	 }
      }
   }
}

/***********************************************************************/
xwaves_ipc_send_attachment(who, what)
   char           *who, *what;
{
   static char    *mb = NULL;
   static int      mbl = 0;
   Attachment     *a;

   if (what && *what && (a = att_list)) {
      while (a) {
	 if (!who || !(*who) || !strcmp(who, "all") ||
	     !strcmp(basename(a->progname), who)) {
	    int             len = strlen(a->progname) + strlen(what) + 2;
	    if (len > mbl) {
	       if (mb)
		  free(mb);
	       mb = NULL;
	       mbl = 0;
	       if (!(mb = malloc(len))) {
		  fprintf(stderr, "Allocation problems in xwaves_ipc_send_attachments()\n");
		  return (FALSE);
	       }
	       mbl = len;
	    }
	    sprintf(mb, "%s %s", a->progname, what);
	    xwaves_ipc_send(a->progname, mb);
	 }
	 a = a->next;
      }
      return (TRUE);
   }
   return (FALSE);
}

/***********************************************************************/
terminate(function)
   char           *function;
{
   if (function && *function) {
      char            str[100];
      int             ok, id;
      extern Panel_item newControl_item;
      char           *meth_sleep();
      Attachment     *a, *a2, *prev;

      function = basename(function);

      if ((a = att_list)) {
	 while (a) {
	    if (!strcmp(function, "all") ||
		(a->progname && !strcmp(basename(a->progname), function))) {
	       detach_func(a);
	       a2 = a->next;
	       if (att_list == a)
		  att_list = a2;
	       else
		  prev->next = a->next;
	       free_attachment(a);
	       a = a->next;
	    } else {
	       prev = a;
	       a = a->next;
	    }
	 }
      }
   }
}

/*************************************************************************/
xwaves_ipc_send(who, str)
   char           *who, *str;
{
   Attachment     *a;
   int             all = !who || !strcmp(who, "all"), itsok = FALSE, result;

   if (str && *str && (a = att_list)) {

      if (debug_level > 1)
	 fprintf(stderr, "xwaves_ipc_send(%s %s)\n", who, str);

      itsok = all;		/* If sending to all, succeed unless some
				 * attempted Send_X_Msg fails. If sending to
				 * an explicit attachment, fail unless (1)
				 * the attachment is found and (2) the
				 * Send_X_Msg attempt succeeds. */
      while (a) {
	 if (a->ready && (all || !strcmp(a->progname, who))) {
	    result = Send_X_Msg(X_Display, a->registry_name, None,
				IMMEDIATE_RESPONSE, str, strlen(str));

	    if (!result) {
	       fprintf(stderr,
		     "xwaves_ipc_send: trouble sending message to %s(%s)\n",
		       a->registry_name, str);
	       itsok = FALSE;
	    } else {
	       if (debug_level)
		  fprintf(stderr, "\nxwaves_ipc_send to %s (%s) ok\n",
			  a->registry_name, str);
	       if (!all)
		  itsok = TRUE;
	    }
	 }
	 a = a->next;
      }
   }
   return (itsok);
}
