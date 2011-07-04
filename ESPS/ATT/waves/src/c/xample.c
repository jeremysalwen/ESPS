/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
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
 * Written by:  David Talkin
 *
 * Brief description: A skeleton attachment.
 *
 */

static char *sccs_id = "@(#)xample.c	1.3	9/26/95	ATT/ERL";

#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <Objects.h>
#include <esps/esps.h>

#define SYNTAX USAGE("xample [-w wave_pro] [-n<waves or other host program>] [-p<program number>] [-v<program version>] [-P<host program number>] [-V<host program version>]");

static  char *wave_pro = ".wave_pro"; /* profile name from command line */

int    debug_level = 0;

/* Referred to in globals.c, but not necessarily used in this program. */
int    command_paused = 0;
extern u_char red[], blue[], green[];
int	use_dsp32; 
double	image_clip = 7.0, image_range = 40.0;

extern char ok[], null[];	/* in message.c */

char *host = "waves", *thisprog = "xample";


/* This prototype structure can be expanded to handle the needs of the
   application.  It's only required fields are name, methods and next. */
typedef struct objects {
  char *name;
  char *signal_name;
  Methods *methods;
  struct objects *next;
} Objects;

Objects *get_receiver();

/* This is the master lists of all objects managed by the program. */
Objects *objlist = NULL;

/* These are some of the "global" attributes.  This list can be
   expanded at will.  Attachments commonly keep track of the xwaves
   signals (signame); the display ensemble name (objectname), and a local
   file associated with these (file). */
static char objectname[NAMELEN], file[NAMELEN], signame[NAMELEN];
static Selector
  a12 = {"signal", "%s", signame, NULL},
  a1 = {"name", "%s", objectname, &a12},
  a0 = {"file", "%s", file, &a1};

/* These are the "prognum" and "versnum" of the host RPC server
   (usually xwaves.  They are usually set automatically when a program is
   "attached" to xwaves. */
extern int  svrnum, svrver;

/* mynum and myver are unique numbers to identify this program's
   "prognum" and "versnum" (see man rpc). */
int  mynum = 5271993, myver = 9876;


/* This function tells xwaves what to do when the attachment's name is
   selected from the menu.  This example issues the "mark" command with
   the signal name and the cursor time as arguments. */
/*********************************************************************/
char *generate_startup_command(mynum, myver)
     int mynum, myver;
{
  static char com[MES_BUF_SIZE];

  sprintf(com,"add_op name %s op #send function %s prognum %d versnum %d command _name mark signal _file time _cursor_time",
	   basename(thisprog), thisprog, mynum, myver);
  return(com);
}


/*********************************************************************/
/*********************************************************************/
main(ac, av)
     int ac;
     char **av;
{
  extern Objects *objlist, *new_objects();
  extern Methods base_methods;
  extern int attachment;
  char mess[MES_BUF_SIZE];
  int i;
  extern int optind;
  extern char *optarg;
  int ch;

  thisprog = av[0];

  while ((ch = getopt(ac, av, "w:n:P:V:p:v:")) != EOF)
    switch (ch)
      {
      case 'n':
	host = optarg; /* receiver name for commands sent FROM this program */
	break;
      case 'w':		/* a profile file; same syntax as .wave_pro */
	wave_pro = optarg;
	break;
      case 'P':		/* prognum and versnum of host and attachment */
	svrnum = atoi(optarg);
	break;
      case 'V':
	svrver = atoi(optarg);
	break;
      case 'p':
	mynum = atoi(optarg);
	break;
      case 'v':
	myver = atoi(optarg);
	break;
      default:
	SYNTAX
	exit(-1);
      }

  /* This reads the profile file grabbing all relevant to the list
     pointed to by a0, and also all of the "standard" xwaves globals
     (perhaps most, or none of which will be used) */
  get_all_globals(wave_pro, &a0);
  

  /* Start the list of active objects.  First item is the program
     itself.  Note its methods list (base_methods).  This list may be
     expanded at will. */
  objlist = new_objects(av[0]);
  objlist->methods = &base_methods;

  /* This program example does not use X graphics (via xview). So, the
     following call is issued. */
  start_communication(svrnum, svrver, myver, mynum,
		      generate_startup_command(mynum,myver));
  /* The above call NEVER RETURNS.  All subsequent operations occur
     asynchronously as RPC requests from xwaves. (Or via SIGNAL handlers in
     this program. start_communication() enters svc_run(). (See man rpc) */

  /* If an X window manager/dispatcher IS to be used, call
     setup_communication(), instead, with exactly the same arguments as
     above in start_communication().  Setup_communication() DOES return.
     You are then expected to eventually enter the "notifier" loop via a
     call to something like window_main_loop() */
}

/* This is a procedure that can be called by this program when it
   wants to disconnect from xwaves. */
/*********************************************************************/
void quit_proc(item, event)
     Panel_item item;
     Event *event;
{
  cleanup();
  kill_proc();
}

/* This example shows how a command might be sent to xwaves.  The
   syntax of the command generated below could include all of the forms
   described in the xwaves manual.  This particular example is designed
   for telling xwaves to put up marks on its dieplays. */
/*********************************************************************/
waves_send(name,command,color,time)
     char *name, *command;
     int color;
     double time;
{
  char mes[MES_BUF_SIZE];
  
  sprintf(mes,"%s %s time %f color %d\n",name,command,time,color);
  mess_write(mes);
  return(TRUE);
  
}

/*********************************************************************/
char *meth_kill(ob, args)
     Objects *ob;
     char *args;
{
  static char file[NAMELEN],object[NAMELEN], chart[NAMELEN];
  static Selector a0 = {"file","%s",file,NULL},
                  a1 = {"name","%s",object,&a0},
                  a2 = {"chart","%s",chart,&a1};
  Objects *o;
  int got;

  got = get_args(args,&a2);

  /* This would typically take the name, and optionally other
     information like "chart" or "file" and destroy the corresponding
     objects. */

  return(ok);
}

/*********************************************************************/
char *meth_set(ob, args)
     Objects *ob;
     char *args;
{
  fprintf(stderr,"meth_set(%s)\n",args);
  get_args(args, &a0);

  /* This is how items in the Selector list a0 above (or any other
     Selector list you wish to create) may be assigned values.  Note that
     the get_args() call will CREATE new variables if the names specified
     in the args list do not already exist. */

  return(ok);
}

/*********************************************************************/
char *meth_mark(ob, args)
     Objects *ob;
     char *args;
{
  static double time, rstart, rend;
  static int color;
  static char file[NAMELEN];
  static Selector
    a2 = {"time", "%lf", (char*)&time, NULL},
    a1 = {"color", "%d", (char*)&color, &a2},
    a0b = {"signal", "%s", file, &a1},
    a0 = {"file", "%s", file, &a0b};

  time = 0.0;
  *file = 0;
  color = -1;
  fprintf(stderr,"meth_mark(%s)\n",args);
  if(get_args(args, &a0) ) {

    /* This is an example of a routine that might take time-stamp
       information from xwaves and do something to a particular file or
       signal in relation to the timeing info. */

    return(ok);
  }
  return(null);
}

/* This routine might clean up any displays, deallocate storage, or
   whever, in preparation to the program exiting.  After everything else
   is done, the terminate_communication() message must be sent. */
/*********************************************************************/
cleanup()
{

  /* Unregister this program as an RPC server. */
  terminate_communication(mynum, myver);
}

/* This is called by xwaves when the "detach" command is given. */
/*********************************************************************/
char *meth_quit(ob, args)
     Objects *ob;
     char *args;
{
  cleanup();
  exit(0);
}

/* This is a bare bones example of new object creation.  When a "make"
   command is sent from xwaves, a routine like the following does the
   work. */
/**********************************************************************/
char *meth_make_object(ob,args)
     Objects *ob;
     char *args;
{
  Objects *ob2, *new_objects();
  extern Objects *objlist, *new_objects();
  extern Methods base_methods;
  char mes[MES_BUF_SIZE];

  *objectname = *file = *signame = 0;

  if(debug_level)
    fprintf(stderr,"meth_make_object(%s)\n",(args)? args : "NULL");

  if(get_args(args,&a0) && *objectname) {
    if(! (ob2 = get_receiver(objectname))) { /* does it already exist? */
      if((ob2 = new_objects(objectname))) { /* create one */
	ob2->next = objlist;	/* link it into the master list */
	objlist = ob2;

	/* Here you would typically do something profound, like generate a
	   display. */

	return(ok);
      } else {			/* couldn't allocate a new_object() */
	fprintf(stderr,"meth_make_object: Allocation failure in new_object()\n.");
	return(null);
      }
    }				/* completed new Objects creation */
    return(ok);			/* (already existed) */
  }
  return(null);
}

/* This gets called by xwaves whenever an xwaves data display gets
   changed, or redrawn.  Programs that need to keep visually synchronized
   with xwaves handle it here. */
/*********************************************************************/
char *meth_redisplay(ob,str)
     Objects *ob;
     char *str;
{
  if(ob && str) {
    if(debug_level)
      fprintf(stderr,"meth_redisplay(%s)\n",(str)? str : "NULL");

    return(ok);
  } else
    fprintf(stderr,"Null object or arg list to meth_redisplay()\n");
  return(null);
}

/* This method is provided for asynchronous passing of return values
   from xwaves.  In this program example, it would not actually ever get
   called.  The str argument contains a character string, the first word
   of which is a number indicating the ID of the callback.  The remainder
   of the string is passed as an argument to the callback. */
/***********************************************************************/
char *meth_return(ob,str)
     Object *ob;
     char *str;
{
  if(str && *str) {
    int id;
    char *get_next_item();
    
    sscanf(str,"%d",&id);
    do_return_callback(id, get_next_item(str));
    
    if(debug_level)
      fprintf(stderr,"meth_return(%s)\n",str);
    return(ok);
  } else
    fprintf(stderr,"Meth_return: bad args\n");
  return(null);
}

/* Here is an example of how an xwaves command that can result in a call
   to meth_return might be set up.  This example is taken from xlabel.
   When xwaves tells xlabel to refresh the display, xlabel needs to ask
   xwaves about the location, size and scaling of the associated display.
/*
/*********************************************************************/
get_display_attributes(ob)
     Objects *ob;
{
  char mes[MES_BUF_SIZE];
  int n, id;
  
  if(ob && ob->name) {

    id = set_return_value_callback(meth_redisplay,ob);

    /* In this example, when do_return_callback() is eventually
       called, meth_redisplay will be called with two arguments: ob and the
       second argument passed to do_return_callback.  The unique id returned
       by set_return_value_callback(), keys the query and response together.
     */

    sprintf(mes,"%s get attributes display function %s file %s return_id %d\n",
	    ob->name, thisprog, ob->signal_name, id);

    /* This call to xwaves eventually results in xwaves responding with
       a "completion" command, thus calling meth_return above. */

    mess_write(mes);
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/
/* Here are two "methods lists".  These associate a command name with
   a procedure call.  The first list consists of those commands that may
   only be directed at a named display ensemble object. */
/*********************************************************************/
Methods 
  meth2 = {"redisplay", meth_redisplay,NULL},
  meth1 = { "mark", meth_mark, &meth2};

/* This second list defines those commands that may only be directed
   at the program itself (the so-called base methods). */
Methods
  bm8 = {"completion", meth_return, NULL},
  bm5 = {"kill", meth_kill, &bm8},
  bm2 = {"quit", meth_quit, &bm5},
  bm1 = {"set", meth_set, &bm2},
  base_methods = {"make", meth_make_object, &bm1};


/*********************************************************************/
Objects *new_objects(name)
     char *name;
{
  Objects *ob;
  char *c, *savestring();
  
  if((ob = (Objects*)malloc(sizeof(Objects))) &&
     (c = savestring(name))) {
    ob->name = c;
    ob->signal_name = NULL;
    ob->methods = &meth1;
    ob->next = NULL;
    return(ob);
  }
  fprintf(stderr,"Can't allocate space for another object\n");
  return(NULL);
}

/* The only thing that kill_proc MUST do is send the message as shown
   below.  It could also do other tidying up in preparation for the
   program exit. */
/*********************************************************************/
kill_proc()
{
  char mess[MES_BUF_SIZE];
  
  sprintf(mess,"%s disconnect function %s\n",host,thisprog);
  terminal_message( mess );	/* say by by */
  exit(0);
}


/*********************************************************************/
/* The following utility routines must be in all attachments.  They
   should be left as shown below. */
/*********************************************************************/
Objects *make_new_object(str)
     char *str;
{
  char name[NAMELEN], command[MES_BUF_SIZE];
  
  sscanf(str,"%s",name);
  if(debug_level)
    fprintf(stderr,"make_new_object(%s)\n",str);
  if(strlen(name)) {
    sprintf(command,"name %s",name);
    if(!strcmp(ok,meth_make_object(NULL,command))) {
      return(get_receiver(name));
    }
  }
  return(NULL);
}

/*********************************************************************/
char *exec_waves(str)
     char *str;
{
  extern char *dispatch();
  return(dispatch(str));
}

/*********************************************************************/
char *get_receiver_name(ob)
     Objects *ob;
{
  return(ob->name);
}

/*********************************************************************/
char *get_methods(ob)
     Objects *ob;
{
  extern Methods base_methods;
  
  if(ob) return((char*)(ob->methods));
  
  return((char*)&base_methods);
}

/*********************************************************************/
Objects *get_receiver(str)
     char *str;
{
  Objects *ob;
  static  char name[NAMELEN];
  extern Objects *objlist;
  
  ob = objlist;
  if(str && strlen(str)) {
    sscanf(str,"%s",name);
    while(ob) {
      if(ob->name &&
	 (! strcmp(ob->name, name))) {
	   return(ob);
	 }
      ob = ob->next;
    }
  }
  return(NULL);
}
/******* END OF FIXED UTILITY ROUTINES *****************/

