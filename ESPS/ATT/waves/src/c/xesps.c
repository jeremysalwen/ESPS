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
 * Written by:   John Shore
 * Checked by:
 * Revised by:   Alan Parker, David Talkin
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xesps.c	1.16	9/28/98	ATT/ERL";

#ifdef hpux
#define _BSD
#endif

#ifdef DEC_ALPHA
#include <sys/types.h>
#endif

#include <sys/wait.h>
#include <Objects.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <xview/scrollbar.h>

#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/fea.h>

#include <sys/time.h>
#ifndef DS3100
#include <sys/resource.h>
#endif
#include <esps/exview.h>

#if defined(SUN4) && !defined(OS5)
#include <vfork.h>
#endif

#define SINGLE_HIT if(((event_id(event) == LOC_MOVE)||event_is_down(event)) && (event_id(event) != LOC_DRAG))
/*#define SINGLE_HIT if(event_is_down(event) && (event_id(event) != LOC_DRAG)) */

extern char *checking_selectors();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }


void        setup_output_dir();
char        *mk_esps_temp();
char        *savestring();
char        *inc_esps_name();
void        markers_to_common();
void	    menu_change();
void        call_operator();
char	    *basename2();

extern int  debug_level; 

/* THE FOLLOWING GLOBALS MAY BE SET FROM <a profile file> */

extern char temp_path[],
	    remote_path[],
            remote_input_path[],
            remote_output_path[];

extern int  w_verbose;

#define NAME_TAB 150

struct fname {
  char *name;
  char num;
  struct fname *next;
};

struct fname *names[NAME_TAB];

/*********************************************************************/
/* Called when the add_espst command is issued to waves (via command file
   or panel entry).  This sets up the ESPS command components and adds
   the command name to the view-window menus. */

extern char ok[], null[];	/* in message.c */
static char espst_name[50];
static char espst_command[MES_BUF_SIZE];
static char espst_menu[50];

Selector    et3 = {"menu", "%s", espst_menu, NULL},
	    et2b = {"command", "#strq", espst_command, &et3},
	    et2 = {"op", "#strq", espst_command, &et2b},
            et1 = {"name", "#strq", espst_name, &et2};

char *meth_add_espst(ob, str)
    Object *ob;
    char *str;
{
    extern void	call_operator();
    char command_line[MES_BUF_SIZE];

    *espst_name = '\0';
    *espst_command = '\0';
    *espst_menu = '\0';
    CHECK_QUERY(str, &et1)
      (void) get_args(str, &et1);

    if(debug_level) 
	fprintf(stderr,"meth_add_espst: name = %s, command = %s, menu = %s\n", 
	       espst_name, espst_command, espst_menu);

    sprintf(command_line, "xtext %s _range_samp _file", espst_command);
    
    menu_change(espst_name, espst_name,
		new_menuop(espst_menu, espst_name, call_operator,
			   (caddr_t) savestring(command_line)),
		espst_menu);
    return(ok);
}

/*********************************************************************/

/* Called when the add_espsf command is issued to waves (via command file
   or panel entry).  This sets up the ESPS command components and adds
   the command name to the view-window menus. */

typedef struct {
  int	    num_outputs;
  int	    display;
  char    *out_ext;
  char    *command;
} espsfInfo;

static int  espsf_num_outputs;
static int  espsf_display;
static char espsf_out_ext[50];
static char new_command[10 * MES_BUF_SIZE];
static char espsf_name[50];
static char espsf_menu[50];

Selector
  ef6 = {"menu", "%s", espsf_menu, NULL},
  ef2b = {"command", "#strq", new_command, &ef6},
  ef2 = {"op", "#strq", new_command, &ef2b},
  ef1 = {"name", "#qstr", espsf_name, &ef2},
  ef5 = {"outputs", "%d", (char *) &espsf_num_outputs, &ef1},
  ef4 = {"display", "%d", (char *) &espsf_display, &ef5},
  ef3 = {"out_ext", "%s", espsf_out_ext, &ef4};

/*********************************************************************/
char *meth_add_espsf(ob, str)
     Object	*ob;
     char	*str;
{
  espsfInfo	*espsf_info;
  char	command[5000];
  int		i;

  *espsf_menu = '\0';
  espsf_num_outputs = 1;
  espsf_display = 1;
  strcpy(espsf_out_ext, "out");
  *new_command = '\0';
  *espsf_name = '\0';
  CHECK_QUERY(str, &ef3)
    (void) get_args(str, &ef3);
  
  if (debug_level) {
    fprintf(stderr,"meth_add_espsf:\n");
    fprintf(stderr,"name = %s, out_ext = %s, command = %s, display = %d\n",
	   espsf_name, espsf_out_ext, new_command, espsf_display);
    fprintf(stderr,"outputs = %d, menu = %s\n", espsf_num_outputs, espsf_menu);
  }

  sprintf(command, "%s _range_samp _file", new_command);

  for (i = 0; i < espsf_num_outputs; i++) 
    if(espsf_display)
      sprintf(command+strlen(command)," _out.g.%s ", espsf_out_ext);
    else
      sprintf(command+strlen(command)," _out.n.%s ", espsf_out_ext);
  
  menu_change(espsf_name, espsf_name,
	      new_menuop(espsf_menu, espsf_name, call_operator,
			 (caddr_t)savestring(command)),
	      espsf_menu);
  return(ok);
}

static char espsn_name[50];
static char espsn_command[MES_BUF_SIZE];
static char espsn_menu[50];

Selector    e3 = {"menu", "%s", espsn_menu, NULL},
  e2b = {"command", "#strq", espsn_command, &e3},
  e2 = {"op", "#strq", espsn_command, &e2b},
  e1 = {"name", "#qstr", espsn_name, &e2};

/*********************************************************************/
/* Called when the add_espsn command is issued to waves (via command file
   or panel entry).  This sets up the ESPS command components and adds
   the command name to the view-window menus. */
char *meth_add_espsn(ob, str)
     Object	*ob;
     char	*str;
{
  char	command[MES_BUF_SIZE];

  *espsn_name = '\0';
  *espsn_command = '\0';
  *espsn_menu = '\0';
  CHECK_QUERY(str, &e1)
    (void) get_args(str, &e1);
  
  if (debug_level) {
    fprintf(stderr,"meth_add_espsn:\n");
    fprintf(stderr,"name = %s, command = %s\n", espsn_name, espsn_command);
  }

  sprintf(command, "%s _range_samp _file", espsn_command);
  
  menu_change(espsn_name, espsn_name,
	      new_menuop(espsn_menu, espsn_name, call_operator,
			 (caddr_t) savestring(command)),
	      espsn_menu);
  return(ok);
}

/*********************************************************************/
char *meth_add_operator(ob, str)
    Object	*ob;
    char	*str;
{
  *espsf_menu = '\0';
  *new_command = '\0';
  *espsf_name = '\0';

  CHECK_QUERY(str, &ef1)
    if(get_args(str, &ef1) && *new_command && *espsf_name) {

    if (debug_level) {
      fprintf(stderr,"meth_add_operator:\n");
      fprintf(stderr,"name = %s, command = %s\n", espsf_name, new_command);
      fprintf(stderr,"menu = %s\n", espsf_menu);
    }

    menu_change(espsf_name, espsf_name,
		new_menuop(espsf_menu, espsf_name, call_operator,
			   (caddr_t) savestring(new_command)),
		espsf_menu);
    return(ok);
  } else {
    sprintf(notice_msg,
        "Bad args to meth_add_operator(%s)\n",(str)? str : "<null>");
    show_notice(1,notice_msg);
  }
  return(null);
}

/***********************************************************************/
is_an_add_op(mo)
     Menuop *mo;
{
  if(mo && mo->name && mo->name[0] && (mo->proc == call_operator)) {
    char *p = (char*)mo->data;
    return(p && *p);
  } else
    return(FALSE);
}

/***********************************************************************/
is_an_add_waves(mo)
     Menuop *mo;
{
  extern void e_exec_waves();

  if(mo && mo->name && mo->name[0] && (mo->proc == e_exec_waves)) {
    char *p = (char*)mo->data;
    return(p && *p);
  } else
    return(FALSE);
}

/***********************************************************************/
dump_add_ops(file)
     char *file;
{
  if(file && *file) {
    char scrat[NAMELEN];
    FILE *of;
    
    /* expand any environment variables */
    (void) build_filename(scrat, "", file); 
    if((of = fopen(scrat, "w"))) {
      Menuop *mo;
      Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();

      while(mol) {
	mo = mol->first_op;
	while(mo) {
	  if(is_an_add_op(mo))
	    fprintf(of,"add_op name \"%s\" menu %s op %s\n",
		    mo->name, mol->name, mo->data);
	  else
	    if(is_an_add_waves(mo))
	      fprintf(of,"add_waves name \"%s\" menu %s op %s\n",
		      mo->name, mol->name, mo->data);
	  mo = mo->next;
	}
	mol = mol->next;
      }
      fprintf(of,"return\n");
      fclose(of);
      return(TRUE);
    } else {
      sprintf(notice_msg,"Can't open %s for output in dump_add_ops.",scrat);
      show_notice(1,notice_msg);
    }
  }
  return(FALSE);
}


/***********************************************************************/
char *meth_dump_add_ops(o, str)
     Object *o;
     char *str;
{
  static char file[NAMELEN];
  static Selector s = {"output", "%s", file, NULL};
  extern char ok[], null[];

  CHECK_QUERY(str,&s)
    if(get_args(str,&s)) {
    if(dump_add_ops(file))
      return(ok);
    else {
      sprintf(notice_msg,"Problems dumping add_ops to file %s");
      show_notice(1,notice_msg);
    }
  } else
    show_notice(1,"No output file was specified to meth_dump_add_ops");
  return(null);
}

/***********************************************************************/
#define PID_TABSIZE 100
static struct {
	char **out_files;
	char **text_files;
	char *command;
	int (*func)();
	int  pid;
	char oname[NAMELEN];
	void *ipc_client_info;
	caddr_t client_data;
	} pid_table[PID_TABSIZE];

static int	pid_not_initialized = 1;

/***********************************************************************/
static void pid_reset_entry(i)
     int i;
{
  pid_table[i].pid = -1;
  pid_table[i].out_files = NULL; /* should be called "graphic_files" */
  pid_table[i].text_files = NULL;
  pid_table[i].command = NULL;
  pid_table[i].func = NULL;
  pid_table[i].client_data = NULL;
  pid_table[i].ipc_client_info = NULL;
  pid_table[i].oname[0] = 0;
}

/***********************************************************************/
static int init_pid_table()
{
  int i;

  /* If first time in, initialize the pid table entries to -1. */
  if (pid_not_initialized) {
    for (i = 0; i < PID_TABSIZE; i++)
      pid_reset_entry(i);
    pid_not_initialized = 0;
    return(0);
  } else { /* Be sure that there is a free entry. */
    for (i = 0; i < PID_TABSIZE; i++) 
      if (pid_table[i].pid == -1) {
	pid_reset_entry(i);
	return(i);
      }
    return (-1);	/* no free pid slots */
  }
}

/***********************************************************************/
static void pid_clear_out_files(i)
     int i;
{
  char **cp;
  if(( cp = pid_table[i].out_files)) {
    while (*cp)
      free(*cp++);
    free(pid_table[i].out_files);
    pid_table[i].out_files = NULL;
  }
  if(( cp = pid_table[i].text_files)) {
    while (*cp)
      free(*cp++);
    free(pid_table[i].text_files);
    pid_table[i].text_files = NULL;
  }
  if(pid_table[i].command)
    free(pid_table[i].command);
  pid_table[i].command = NULL;
}

/***********************************************************************/
static int pid_get_table_index(pid)
  int pid;
{
  int i;

  for(i=0; i < PID_TABSIZE; i++)
        if(pid == pid_table[i].pid)
          return(i);
  if(w_verbose)
    fprintf(stderr,"Couldn't find table entry for pid %d\n",pid);
  return(-1);
}

/***********************************************************************/
void reset_esps_callback(pid, newcallback)
     int pid;
     int (*newcallback)();   /* if !NULL, then call this function whe n done */
{
  int i = pid_get_table_index(pid);

  if(i >= 0)
    pid_table[i].func = newcallback;
}

/***********************************************************************/
void set_esps_callback_data(pid, data)
     int pid;
     caddr_t data;
{
  int i = pid_get_table_index(pid);

  if(i >= 0)
    pid_table[i].client_data = data;
}

/***********************************************************************/
caddr_t get_esps_callback_data(pid)
     int pid;
{
  int i = pid_get_table_index(pid);

  if(i >= 0)
    return(pid_table[i].client_data);
  else
    return(NULL);
}

/***********************************************************************/
static char *pid_add_graphics_name(i,name)
     char *name;
     int i;
{
  addstr(savestring(name), &(pid_table[i].out_files));
  return((pid_table[i].out_files)[strlistlen(pid_table[i].out_files) - 1]);
}

/***********************************************************************/
static char *pid_add_text_name(i,name)
     char *name;
     int i;
{
  addstr(savestring(name), &(pid_table[i].text_files));
  return((pid_table[i].text_files)[strlistlen(pid_table[i].text_files) - 1]);
}

/***********************************************************************/
static char *pid_add_command(i,command)
     char *command;
     int i;
{
  pid_table[i].command = savestring(command);
  return(pid_table[i].command);
}

/***********************************************************************/
static char *make_output_name(name, ext, type)
     char *name, *ext, *type;
{
  static char newname[NAMELEN];

  if(name) {
    if( ! strncmp(type, "out.", 4)) {
      if(ext && *ext)
      sprintf(newname, "%s.%s", name, ext);
    else
      sprintf(newname, "%s.out", name);

    strcpy(newname, inc_esps_name(newname));
    } else
      strcpy(newname, ext);
    
    if (strlen(remote_output_path) > 0)
      build_filename(newname, basename2(newname), remote_output_path);
    else
      if (strlen(remote_path) > 0) 
	build_filename(newname, basename2(newname), remote_path);
      else
	setup_output_dir(newname);
    return(newname);
  }
  return(NULL);
}

/***********************************************************************/
char *get_only_ascii(out,in, waswhite)
     char *out, *in;
     int *waswhite;
{
  char *c, *get_next_item();
  int i;

  sscanf(in,"%s",out);
  for(i=0, c = out; *c && (((*c >= '0') && (*c <= '9')) ||
		      ((*c >= 'A') && (*c <= 'Z')) ||
		      ((*c >= 'a') && (*c <= 'z')) ||
		      (*c == '_') || (*c == '/') || (*c == '.')); ) {
    c++;
    i++;
  }
  if(*c && i) {
    *waswhite = FALSE;
    while(*in) {
      if(*in == *c) {
	break;
      }
      in++;
    }
  } else {
    in = get_next_item(in);
    *waswhite = TRUE;
  }
  *c = 0;
  return(in);
}

/***************************************************************************/
static run_any_prog(command_line, pid_slot)
    char    *command_line;
    int pid_slot;
{
    extern char default_header[];
    int	    i;
    char    *args[4];
    int	    pid;
    int esps_w_done();
    
    set_default_header();

    /* Check to see if remote command rsh and in debug mode, then
       issue warning about -n option on rsh.    */
    if (debug_level &&
	(strlen(command_line) >4) && !strncmp(command_line,"rsh",3)) {
      show_notice(0,"You have used rsh in an external command.\nYou might need the -n option \n on rsh (eg. rsh hostname -n command).");
    }

    pid_add_command(pid_slot,command_line);

    /* set execvp args */
    args[0] = "/bin/sh";
    args[1] = "-c";
    args[2] = command_line;
    args[3] = NULL;
    
    if (w_verbose || debug_level) 
      fprintf(stderr, "Starting shell command:\n%s %s %s", args[0], args[1], args[2]);

/*  Fork a new process to run the command.  */
#if defined(SUN4) && !defined(OS5) || defined(DS3100)
    switch (pid = vfork()) {
#else
    switch (pid = fork()) {
#endif
    case -1:			/* fork failed */
      perror("run_any_prog(); fork failed");	
      return (0);
    case 0:			/* child process */	
      setpgid(0,getpid());
      execvp(args[0], args);
      perror("run_any_prog: execvp failed");
      return (0);
    default:			/* parent */

      /*   Save the pid of the child in the table. */
      {
	extern Frame daddy;
	void *in_a_ipc_dispatch();

	pid_table[pid_slot].pid = pid;
	(void)notify_set_wait3_func(daddy,esps_w_done,pid);
	pid_table[pid_slot].ipc_client_info = in_a_ipc_dispatch();
	indicate_pending_process(pid_slot);
      }
    }

    if (w_verbose)
	fprintf(stderr,"(pid: %d, request: %d)\n",pid,pid_slot);

    return(pid);
}

/* Examine a character string to determine if it has one or more
"arithmetic" operators as a prefix.  If so, return the prefix as a
string and a number indicating how many characters are in the prefix.
***********************************************************************/
char *is_arith_operator(cp, nops)
  char *cp;
    int *nops;
{
  static char aops[] ="*/+-=:%^", opstr[50];

  *nops = 0;
  while(*cp && strchr(aops,(int)(*cp))) {
    opstr[*nops] = *cp++;
    (*nops)++;
  }
  if(*nops) {
    opstr[*nops] = 0;
    return(opstr);
  }  else
    return(NULL);
}

/***********************************************************************/
/* This event routine is called when an external operator is selected
   from view-window menus.  A program is introduced onto the menu
   via a call to meth_add_operator(), which in turn is called by issuing
   the add_op command to xwaves. */
void call_operator(canvas, event, arg)
    Canvas	canvas;
    Event	*event;
    caddr_t	arg;
{
  SINGLE_HIT {
    char	*out_names = NULL;
    int		out_size = 0;
    Signal	*s;
    View	*v;
    static char	command[5000];
    char *cp, *outname, *current_object_name();
    int		i, pidi, internal;
    Object *o;

    v = (View *) xv_get(canvas, WIN_CLIENT_DATA);
    s = v->sig;

    if((pidi = init_pid_table()) < 0) {
      fprintf(stderr," No more free pid table slots\n");
      return;
    }
    
    if((o = (Object*)(s->obj)))
      cp = o->name;
    else
      cp = current_object_name();

    strcpy(pid_table[pidi].oname, cp);

    if (s->file == SIG_NEW) {
	if (debug_level) 
	    fprintf(stderr,"call_operator: signal and file differ; writing signal.\n");
	put_signal(s);
    }

    *command = 0;
    if(!(cp = (char*)arg) || !(*cp) ) {
      if(debug_level)
	fprintf(stderr, "NULL command passed to call_operator()\n");
      return;
    }
    internal = FALSE;
    if(*cp == '#') {
      while((*cp == '#') || (*cp == ' ') || (*cp == '	')) cp++;
      internal = TRUE;
    }
    if(debug_level)
      fprintf(stderr,"call_operator building command from:\n|%s|\n",cp);
    do {
      char item[NAMELEN], *get_next_item(), *aop = NULL;
      int noc = 0;

      if((aop = is_arith_operator(cp,&noc)) && (cp[noc] == '_'))
	cp += noc;
	
      if(*cp == '_') {		/* decode the symbol */
	int waswhite;
	cp = get_only_ascii(item, cp+1, &waswhite);
	if(!strncmp("out.",item,4) || !strncmp("nom.",item,4)) { /* Generate an output file name. */
	  if(!strncmp("g.",&item[4],2)) {
	    outname = pid_add_graphics_name(pidi,make_output_name(view_get_value(v,"file"),&item[6],item));
	  } else
	    if(!strncmp("t.",&item[4],2)) {
	      outname = pid_add_text_name(pidi,make_output_name(view_get_value(v,"file"),&item[6],item));
	    } else
	      if(!strncmp("n.",&item[4],2)) {
		outname = make_output_name(view_get_value(v,"file"),&item[6],item);
	      } else
		outname = make_output_name(view_get_value(v,"file"),&item[4],item);
	  if(!outname) {
	    fprintf(stderr,"Problems parsing output spec %s in call_operator()\n",item);
	    return;
	  }
	  sprintf(command+strlen(command),"%s", outname);
	} else {
	  if(aop) {
	    char *tcp = view_get_value(v,item);
	    noc--;
	    if(*tcp == '-') {
	      if(aop[noc] == '-') {
		tcp++;
		aop[noc] = '+';
	      } else
		if(aop[noc] == '+') {
		  tcp++;
		  aop[noc] = '-';
		}
	    }
	    sprintf(command+strlen(command),"%s%s",aop,tcp);
	  } else
	    sprintf(command+strlen(command),"%s",view_get_value(v,item));
	}
	if(waswhite)
	  strcat(command," ");
      } else {
	sscanf(cp,"%s",item);
	sprintf(command+strlen(command),"%s ",item);
	cp = get_next_item(cp);
      }
    } while(*cp);

    if(internal) {
      pid_clear_out_files(pidi);
      pid_reset_entry(pidi);
      exec_waves(command);
    } else
      if ( ! run_any_prog(command, pidi))
	fprintf(stderr, "call_operator: couldn't run command");
  }
}   

/***********************************************************************/
/* run an external ESPS program, given the command string, 
   input file name, and output file names.  The command can be 
   run either in the foreground or background. If defined, the 
   remote_path string is prepended to the basename of input file 
   and, if num_esps_out > 0, the output files.  If remote_input_path
   is defined, it overrides remote_path for input files.  If
   remote_output_path is defined, it overrides remote_path for 
   output files.  It also overrides output_dir (for external calls) if 
   that global is defined.  This overall behavior is usseful for running 
   in a remote NFS directory. Note that the remote_path globals should not 
   terminate with a /,  which is added here.  The number of file 
   arguments can be reduced to 1 or 0 by calling run_esps_prog with 
   output "" or input and output both "". */
int
run_esps_prog(command, input, output, num_esps_out, display, func)
    char    *command, *input, *output;
    int	    num_esps_out;
    int	    (*func)();	/* if !NULL, then call this function when done */
    int     display;	/* YES, display output files when done */
{
if(run_esps_prog_get_pid(command, input, output, num_esps_out, display, func))
    return(0);
  else
    return(-1);
}

/* Return the PID of the external process on success, zero on failure.  */
int
run_esps_prog_get_pid(command, input, output, num_esps_out, display, func)
    char    *command, *input, *output;
    int     num_esps_out;
    int     (*func)();  /* if !NULL, then call this function when done */
    int     display;    /* YES, display output files when done */
{
    char    command_line[2000];
    char    tmpfile[MES_BUF_SIZE];
    extern char default_header[];
    int	    ret;
    int	    i;
    char    *args[4];
    int	    pid;
    int pid_slot;
    int esps_w_done();
    char *output_wdir;
    int out_size;
    char *output_files = output;
    
    if(*default_header) {
     static char tch[NAMELEN];
#if defined(SONY_RISC) || defined(CONVEX)
      setenv("DEF_HEADER",default_header,1);
#else
      sprintf(tch,"DEF_HEADER=%s",default_header);
      putenv(tch);
#endif
    }


    if((pid_slot = init_pid_table()) < 0)
      return(0);

    /* check to see if remote command rsh and in debug mode, then
       issue warning about -n option on rsh
    */

    if (debug_level && (strlen(command) >4) && !strncmp(command,"rsh",3))
      show_notice(1,
      "You have used rsh in an external command.\nYou might need the -n option \n on rsh (eg. rsh hostname -n command).");


    if (*output) {
	/* Output has one or more output file names with null	*/
	/* chars as separators and terminator.			*/
	/* first, let's fix them up if output_dir or other globals defined */

	output_wdir = savestring(""); 
	out_size = 0;
	for (i = 0; i < num_esps_out; i++)
	{

	  int old_size = out_size;

	  if (strlen(remote_output_path) > 0)
	    build_filename(tmpfile, basename(output), remote_output_path);

	  else if (strlen(remote_path) > 0) 
	    build_filename(tmpfile, basename(output), remote_path);

	  else {
	      strcpy(tmpfile, output); 
	      if(not_explicitly_named(tmpfile))
		setup_output_dir(tmpfile);
	  }
	  if(debug_level > 1)
	    fprintf(stderr, "out #%d:%s\n",i,tmpfile);
	  out_size += strlen(tmpfile) + 1;
	  output_wdir = realloc(output_wdir, out_size);
	  strcpy(output_wdir + old_size, tmpfile);
	  output += strlen(output) + 1;
	}
	output = output_files = output_wdir;
    }
    if(*output && debug_level) {
      int nuts = strlen(output) + 1, kk;
      fprintf(stderr, "outputs: ");
      for(kk=0; kk < num_esps_out; kk++)
	fprintf(stderr,"%s ",output+(kk*nuts));
      fprintf(stderr,"\n");
    }

    if (input && *input) {

	if (strlen(remote_input_path) > 0) 
	  build_filename(tmpfile, basename(input), remote_input_path);
	
	else if (strlen(remote_path) > 0)
	  build_filename(tmpfile, basename(input), remote_path);
		 
	else
	  strcpy(tmpfile, input); 

	sprintf(command_line, "%s %s ", command, tmpfile);	

    }
    else 
      	sprintf(command_line, "%s ", command);	

    if (*output) {
      if (num_esps_out == 0)	{
	  sprintf(tmpfile, " %s", output);
	  strcat(command_line, tmpfile);
	}
      else	/* Output has one or more output file names with null	*/
	/* chars as separators and terminator.			*/
	{

	  for (i = 0; i < num_esps_out; i++)
	    {
	      strcpy(tmpfile, output); 
	      strcat(command_line, tmpfile);
	      strcat(command_line, " ");
	      output += strlen(output) + 1;
	    }
	}
    }
    
    strcat(command_line,  "\n");

    if (w_verbose || debug_level) 
      fprintf(stderr,"Starting shell command:\n%s", command_line);


/* set execvp args */

    args[0] = "/bin/sh";
    args[1] = "-c";
    args[2] = command_line;
    args[3] = NULL;

/* 
 fork a new process to run the command
*/
#if defined(SUN4) || defined(SUN3) || defined(DS3100)
    switch (pid = vfork()) {
#else
    switch (pid = fork()) {
#endif
	case -1:			/* fork failed */
		perror("run_esps_prog_get_pid; fork failed");	
		return (0);
	case 0:				/* child process */	
		setpgid(0,getpid());
		execvp(args[0], args);
		perror("run_esps_prog_get_pid: execvp failed");
		return (0);
	default:			/* parent */
/*
 first save the pid of the child in the table
*/
		{
		  extern Frame daddy;
		  char *current_object_name();
		  void *in_a_ipc_dispatch();

		  pid_table[pid_slot].pid = pid;
		  if(func)pid_table[pid_slot].func = func;
		  strcpy(pid_table[pid_slot].oname, current_object_name());
		  if(display){ 
    		    int j;
	    	    for (j = 0; j < num_esps_out; j++) {
		      addstr(output_files, &(pid_table[pid_slot].out_files));
		      output_files += strlen(output_files) + 1;
		    }	
		  }	
		  else 
		    pid_table[pid_slot].out_files = NULL;

		  (void)notify_set_wait3_func(daddy,esps_w_done,pid);
		  pid_table[pid_slot].ipc_client_info = in_a_ipc_dispatch();
		  pid_add_command(pid_slot,command_line);
		  indicate_pending_process(pid_slot);
		}
    }

    if (w_verbose)
	fprintf(stderr,"(pid: %d, request: %d)\n",pid,pid_slot);

    return(pid);
}

/***********************************************************************/
int esps_w_done(client, pid, status, rusage)
    Notify_client client;
    int pid;
    union wait *status;
    struct rusage *rusage;
{
    int i,j;
#if !defined(DEC_ALPHA) && !defined(OS5)
#if defined(XOS5)
    union wait status_a = *status;
#endif
#if defined(XOS5) 
    if (WIFEXITED(status_a.w_status)) {
#else
    if (WIFEXITED(*status)) {
#endif
#else
      {
#endif
      if (debug_level)
	fprintf(stderr,"esps_w_done: pid %d just finished. \n",pid);

      if((i = pid_get_table_index(pid)) >= 0) {
	extern Panel_item newObj_item;

	panel_set_value(newObj_item, pid_table[i].oname);

	if (pid_table[i].func) {
	  if ((w_verbose > 2) || debug_level) 
	    fprintf(stderr,"pid: %d, calling function %x\n",pid,pid_table[i].func);
	  (*pid_table[i].func)(pid);
	}

	if (pid_table[i].out_files) { /* display signal output files */
	  char **names = pid_table[i].out_files;
	  for (j = 0; names[j]; j++) {
	    if (w_verbose)
	      fprintf(stderr,"Process %d finished, displaying file: %s\n",pid,names[j]);
	    if(create_new_signal_view(names[j]))
	      add_to_new_files_browser(names[j]);
	      
	  }
	}

	if (pid_table[i].text_files) { /* display text output files */
	  char **names = pid_table[i].text_files;
	  Frame help_win;
	  for (j = 0; names[j]; j++) {
	    if (w_verbose)
	      fprintf(stderr,"Process %d finished, displaying file: %s\n",pid,names[j]);
	    help_win = exv_make_text_window(XV_NULL, names[j], basename2(names[j]),
					    names[j], 
					    WITH_FIND_BUTTON, USE_FRAME_INDEP);
	    if(help_win)
	      add_to_new_files_browser(names[j]);
	  }
	}

	do_ipc_response_if_any(pid_table[i].ipc_client_info, ok);

	indicate_process_complete(i);
	pid_clear_out_files(i);
	pid_reset_entry(i);
	return NOTIFY_DONE;
      } else
	if (w_verbose > 2)
	  fprintf(stderr,"esps_w_done: pid %d not in data structure. \n",pid);
      return NOTIFY_IGNORED;
    }
    if (w_verbose > 2)
      fprintf(stderr,"esps_w_done: pid %d not WIFEXITED \n",pid);
    return NOTIFY_IGNORED;
}


/***********************************************************************/
/* routine for making temporary ESPS files */
char *mk_esps_temp(template)
{

  char tpath[NAMELEN];
  sprintf(tpath, "%s/%s", temp_path, template);  
#if !defined(LINUX)
  return(savestring(mktemp(tpath)));
#else
  return(savestring(mkstemp(tpath)));
#endif
/* could also use 
  return(e_temp_name(template)); */
}

/*
 * The following functions (hash, lookup, and inc_esps_name) are 
 * based on code from Alan Parker's genhd.c -- js
 */

/* very simple hash function */
static int
hash (s)
    char    *s;
{
    int     sum = 0;
    while (*s)
	sum += *s++;
    return (sum % NAME_TAB);
}


/* look up an entry in the hash table.   Return NULL if not found, else
   return the address of the node
*/

static struct fname *
lookup(s, tab)
    char	    *s;
    struct fname    *tab[];
{
    struct fname    *np;

    spsassert(tab != NULL && s != NULL,"error in lookup");
    for (np = tab[hash (s)]; np != NULL; np = np->next) 
	if (strcmp(s, np->name) == 0) 
	    return (np);	/* found it */
    return (NULL);		/* not found */
}

/* check to see if file name has been used before; if not, add to 
   list; if so, increment the name
*/

char *
inc_esps_name(name)
    char	    *name;
{
    struct fname    *np,
                    *lookup();
    int		    hashval;
    int		    i;
    char	    *newname[128];
    static int	    first = 1;
    FILE	    *fd;

    if (first) {
	for (i=0; i < NAME_TAB; i++) names[i] = NULL;
	first = 0;
    }

    np = lookup(name, names);

    if (np == NULL) { /* allocate a node if this name is not defined */

	np = (struct fname *) calloc(1, sizeof(*np));
	spsassert(np != NULL,"calloc failed");

	/* save the name */
	np->name = savestring(name);
	spsassert(np->name != NULL,"savestring failed");

	/* insert the node into the hashtable, and set the count */
	hashval = hash(np->name);
	np->next = names[hashval];
	names[hashval] = np;
	/* if the new name is an existing file, we increment the first time*/
	if ((fd = fopen(name, "r")) == NULL)
	    np->num = 0;
	else
	{
	    np->num = 1;
	    fclose(fd);
	}
    }
    else { /* we already saw this name, just increment the count */
	np->num++;
    }

    if (np->num == 0) 
	return (savestring(name));
    else {
	insert_numeric_ext(np->name, np->num, newname);
	if (debug_level) 
	    (void) fprintf(stderr, "inc_esps_name: newname = %s\n", newname);
	return(savestring(newname));
    }
}


/* this function writes the left and right marker information to 
   ESPS Common */
 
void
markers_to_common(v)
    View    *v;			/* view in which markers were changed */
{

  double start, end;		/* relative times for existing markers */
  int startrec, endrec;	/* esps records for existing markers */
  Signal *s;

  s = v->sig;

  if (debug_level)
    (void) fprintf(stderr, 
		   "markers_to_common: function entered\n");

  start = v->lmarker_time - s->start_time;
  end = v->rmarker_time - s->start_time;

  startrec = 1 + (int)(start*s->freq);
  if (startrec < 1) startrec = 1;

  endrec = 1 + (int)(end*s->freq);
  if (endrec > s->file_size) endrec = s->file_size;

  if (debug_level > 1)  {
    (void) fprintf(stderr, 
		   "markers_to_common: start = %g, end = %g\n",
		   start, end);
    (void) fprintf(stderr, 
		   "markers_to_common: startrec = %d, endrec = %d\n",
		   startrec, endrec);
  }

  (void) putsym_s("filename", s->name);
  (void) putsym_s("prog","xwaves");

  (void) putsym_i("start", (int) startrec);
  (void) putsym_i("nan", (int) (endrec - startrec + 1));
}


typedef struct pidisplay {
  int pid;
  char *command;
  struct pidisplay *next, *prev;
};


static Frame pframe = XV_NULL;
static Panel ppanel = XV_NULL;
static Panel_item plist = XV_NULL;
static Frame cframe = XV_NULL;

static void ok_command()
{
  xv_destroy_safe(cframe);
}

static void process_kill_command(item,event)
     Panel_item item;
     Event *event;
{
  int pidi = (int)xv_get(item, PANEL_CLIENT_DATA);

  if(pid_table[pidi].pid > 0) {
    kill(pid_table[pidi].pid,SIGKILL);
    kill(pid_table[pidi].pid+1,SIGKILL);
    do_ipc_response_if_any(pid_table[pidi].ipc_client_info, ok);
    indicate_process_complete(pidi);
    pid_clear_out_files(pidi);
    pid_reset_entry(pidi);
  }
  xv_destroy_safe(cframe);
  cframe = XV_NULL;
}

void show_pending_command(item, string, client_data, op, event, row)
     Panel_item item;
     char *string;
     caddr_t client_data;
     Panel_list_op op;
     Event *event;
     int row;
{
  Panel panel;
  int pidi = (int)client_data;

  cframe = (Frame)xv_create(pframe,FRAME,FRAME_SHOW_HEADER, FALSE,
			    XV_SHOW, FALSE, XV_X, 5, XV_Y, 5, NULL);
  panel = (Panel)xv_create(cframe, PANEL, NULL);
  xv_create(panel, PANEL_MESSAGE,
	    PANEL_LABEL_STRING, pid_table[pidi].command,
	    PANEL_LABEL_BOLD, TRUE,
	    NULL);
  xv_create(panel, PANEL_BUTTON,
	    PANEL_NEXT_ROW, -1,
	    PANEL_LABEL_STRING, "OK",
	    PANEL_NOTIFY_PROC, ok_command,
	    NULL);
  xv_create(panel, PANEL_BUTTON,
	    PANEL_LABEL_STRING, "Kill This Process",
	    PANEL_CLIENT_DATA, pidi,
	    PANEL_NOTIFY_PROC, process_kill_command,
	    NULL);

  window_fit(panel);
  window_fit(cframe);

  xv_set(cframe, XV_SHOW, TRUE, NULL);
}

Notify_value
pframe_destroy_func(client, status)
     Notify_client client;
     Destroy_status status;
{

    if (status == DESTROY_CHECKING) {
	}
    else  if (status == DESTROY_CLEANUP) {
      plist = XV_NULL;
        return notify_next_destroy_func(client, status);
	}
    else if (status == DESTROY_SAVE_YOURSELF) {
	}
    else {
      plist = XV_NULL;
	}
	
    return NOTIFY_DONE;
}

void create_plist(pidi)
     int pidi;
{
  char digits[10];

  sprintf(digits,"%d",pid_table[pidi].pid);
  pframe = (Frame)xv_create(XV_NULL,FRAME, XV_SHOW, FALSE,
			    XV_X, 22, XV_Y, 22, NULL);
  ppanel = (Panel)xv_create(pframe,PANEL,NULL);
  plist = xv_create(ppanel, PANEL_LIST,
                     XV_X, 22,
                     XV_Y, 22,
                    PANEL_LIST_WIDTH, 75,
                     PANEL_LIST_DISPLAY_ROWS, 3,
                     PANEL_LABEL_STRING, "PENDING:",
                     PANEL_LAYOUT, PANEL_VERTICAL,
                     PANEL_READ_ONLY, TRUE,
                     PANEL_CHOOSE_ONE, TRUE,
                     PANEL_CHOOSE_NONE, TRUE,
                     PANEL_NOTIFY_PROC, show_pending_command,
                     NULL);
  xv_set(plist,
       PANEL_LIST_STRING, 0, digits,
       PANEL_LIST_CLIENT_DATA, 0, pidi,
       NULL);

  notify_interpose_destroy_func(pframe, pframe_destroy_func);

  xv_set(pframe, XV_SHOW, TRUE, 0);
 
  window_fit(ppanel);
  window_fit(pframe);
}

void add_to_plist(pidi)
     int pidi;
{
  if(plist) {
    int n = xv_get(plist, PANEL_LIST_NROWS);
    char digits[10];
    
    sprintf(digits,"%d",pid_table[pidi].pid);
    xv_set(plist,PANEL_LIST_INSERT, n,
	   PANEL_LIST_STRING, n, digits,
	   PANEL_LIST_CLIENT_DATA, n, pidi,
	   NULL);
    if(n == 0)
      xv_set(pframe, XV_SHOW, TRUE, 0);
  }
}

indicate_process_complete(pidi)
     int pidi;
{
  extern int show_processes;

  if(show_processes && plist) {
    int n = xv_get(plist, PANEL_LIST_NROWS);
    int i;
    for(i=0; i < n; i++) {
      if((int)xv_get(plist, PANEL_LIST_CLIENT_DATA, i) == pidi) {
	xv_set(plist, PANEL_LIST_DELETE, i, NULL);
	if(n <= 1) {
	  xv_set(pframe, XV_SHOW, FALSE, 0);
	  if(cframe) {
	    xv_destroy_safe(cframe);
	    cframe = XV_NULL;
	  }
	}
	return;
      }
    }
  }
}

indicate_pending_process(pidi)
     int pidi;
{
  extern int show_processes;

  if(show_processes) {
    if(!plist)
      create_plist(pidi);
    else
      add_to_plist(pidi);
  }
}




