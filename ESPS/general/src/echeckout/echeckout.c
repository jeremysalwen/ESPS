/*
 * This material contains proprietary software of Entropic Research Lab, Inc.
 * Any reproduction, distribution, or publication without the prior written
 * permission of Entropic Research Lab, Inc. is strictly prohibited. Any
 * public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 * 
 * "Copyright (c) 1991 Entropic Research Lab, Inc.; All rights reserved"
 * 
 */

static char    *sccs_id = "%W% %G%	ERL";

#include <elm.h>
#include <elm_erl.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <esps/esps.h>
#include <esps/mach_codes.h>
#include <sys/param.h>
#include <ctype.h>
#include <esps/ss.h>
#ifdef OS5
#include <sys/fcntl.h>
#include <sys/termios.h>
#endif

extern char    *getenv(), *strchr();
extern int      gethostname();
static char     server_host[ELM_HOST_LEN + 1];
static char     expired_feature[ELM_FEATURE_LEN + 1];
static void     background();
#ifndef OS5
extern int      getpid();
#endif
extern char    *optarg;
extern int      optind;
int             c;
extern short    erl_ech_port;
int             debug_level = 0;

char           *get_stat_file();

#define		SOCK_BUF_LEN 100
SOCKET         *sp = SS_NULL, *csp = SS_NULL;
char            sock_buffer[SOCK_BUF_LEN];
char            ciphertext[100];
char           *c_ptr;

/*
 * #define DEBUG 1
 */
static void     byebye();

#ifdef ESPS
char           *feature = ESPS_FEATURE;
char           *program = "echeckout";
#endif

#ifdef HTK
char           *feature = HTK_FEATURE;
char           *program = "hcheckout";
#endif

int             time_count = 0;
int             warning_msg = 0;/* 1 hourly reminder that a license is out is
				 * printed; 0 no reminder */
char            this_host[MAXHOSTNAMELEN];
int             proc_id;
struct elm_user user;
int             i;

char           *statfile;
FILE           *statfile_fp;
int             license_code;

int		mit=0;


#define ITIMER_NULL ((struct itimerval *)0)

#ifdef ESPS
#define SYNTAX USAGE("echeckout [-w] -w to enable hourly reminder")
#else
#define SYNTAX USAGE("hcheckout [-w] -w to enable hourly reminder")
#endif

main(argc, argv)
   int             argc;
   char          **argv;
{
   int             ret, i, j;
   char           *h;
   char            machine_type[100];
   struct elm_info info;
#ifdef HTK
   char           *user_name;
   int             in_use = 0;
#endif


   while ((c = getopt(argc, argv, "wx")) != EOF) {
      switch (c) {
      case 'w':
	 warning_msg = 1;
	 break;
      case 'x':
	 debug_level = 1;
	 break;
      default:
	 SYNTAX;
	 break;
      }
   }


   signal(1, byebye);
   signal(2, byebye);
   signal(3, byebye);
   signal(15, byebye);

#ifdef NO_LIC
   if (0)              {
#else
   if (!erl_elm_mit()) {
      erl_elm_init();
#endif

#ifdef ESPS
      strcpy(machine_type, machine_codes[MACH_CODE]);
      license_code = mach_license_codes[MACH_CODE].license_code;

      if (ret = elm_getinfo(&info, ESPS_FEATURE, 0) > 0) {
	 if (((int) strtol(erl_get_host_field(info.vendordata), NULL, 8) & license_code) == 0) {
	    fprintf(stderr,
		    "\nSorry, but you do not have a license for ESPS on host type: %s\n",
		    machine_type);
	    elm_bye();
	    exit(1);
	 }
      } else {
	 fprintf(stderr, "\n%s.\n", elm_message(NULL, "ESPS", ret));
	 exit(1);
      }

      (void) gethostname(this_host, MAXHOSTNAMELEN);
      for (i = 1; (i = elm_getulist(&user, ESPS_FEATURE, i)) > 0; i++) {
	 if ((strcmp(this_host, user.host) == 0)) {
	    fprintf(stderr,
		    "\necheckout: There already is an ESPS license checked out for this host.\n");
	    fprintf(stderr,
		 "echeckout: You do not need to checkout more than one.\n");
	    elm_bye();
	    exit(0);
	 }
      }
#endif

#ifdef HTK
      user_name = erl_elm_username();

      for (i = 1; (i = elm_getulist(&user, HTK_FEATURE, i)) > 0; i++) {
	 if ((strcmp(user_name, user.user) == 0)) {
	    fprintf(stderr,
		    "\nhcheckout: There already is an HTK license checked out for %s.\n",
		    user_name);
	    fprintf(stderr,
		 "hcheckout: You do not need to checkout more than one.\n");
	    elm_bye();
	    exit(0);
	 }
      }
#endif

      ret = elm_getlicense(feature, ELM_GETLIC, 0L);

      if (ret < 0) {
#ifdef ESPS
	 fprintf(stderr, "\n%s.\n", elm_message(NULL, "ESPS", ret));
#else
	 fprintf(stderr, "\n%s.\n", elm_message(NULL, "HTK", ret));
#endif
#ifdef HTK
	 if (ret == ELM_NO_LICENSES) {
	    fprintf(stderr, "Licenses currently held by user:\n");
	    for (i = 1; (i = elm_getulist(&user, HTK_FEATURE, i)) > 0; i++) {
	       fprintf(stderr, " %s\n", user.user);
	       in_use++;
	    }
	    if (elm_getinfo(&info, feature, 0) > 0) {
	       fprintf(stderr,
		       "%d licenses in use, %d licenses reserved, out of %d licenses total.\n"
		       ,in_use, info.numresv, info.numlic);
	    }
	 }
#endif
	 elm_bye();
	 exit(1);
      }
      fprintf(stderr,
	      "%s license checked out OK", erl_decode_feature(feature));
   } else {
      fprintf(stderr, "License checked out OK (nl)");
      mit = 1;
   }
#ifdef HTK
   fprintf(stderr, " for user %s", user_name);
#endif
   fprintf(stderr, ".\n");

   if (!debug_level)
      background();

   sleep(1);
   if (debug_level)
      exit(0);


   if (getenv("HOME")) {
#ifdef HTK
      statfile = get_stat_file('h');
#else
      statfile = get_stat_file('e');
#endif
      statfile_fp = fopen(statfile, "w");
      if (statfile_fp) {
	 fprintf(statfile_fp, "%d", proc_id);
	 fclose(statfile_fp);
	 fprintf(stderr,
#ifdef HTK
		 "\n\nDo HFree to release license when no longer needed, or kill process %d.\n", proc_id);
#else
		 "\n\nDo efree to release license when no longer needed, or kill process %d.\n", proc_id);
#endif
      } else {
	 fprintf(stderr,
		 "[pid: %d]: Do kill %d (on this CPU) to release license when no longer needed.\n",
		 proc_id, proc_id);
      }
   } else {
      fprintf(stderr,
	      "[pid: %d]: Do kill %d (on this CPU) to release license when no longer needed.\n",
	      proc_id, proc_id);
   }


#ifdef ESPS
   sp = ServerSock(erl_ech_port);
#endif

   for (;;) {			/* Forever */
      register int    x;

#ifdef ESPS
      if (sp != SS_NULL) {
	 (void) SockSelect(20.0, "r");
      } else
#endif
	 sleep(25);
#ifdef ESPS
      if ((sp != SS_NULL) && IsReadSet(sp)) {	/* connect request */
	 csp = AcceptSock(sp);
	 if (csp != SS_NULL) {
	    sock_buffer[0] = 0;
	    (void) SockGets(sock_buffer, SOCK_BUF_LEN - 1, csp);
	    if (c_ptr = strchr(sock_buffer, '\n'))
	       *c_ptr = 0;
	    strcpy(ciphertext, elm_code(ELM_ENCRYPT, sock_buffer));
	    strcat(ciphertext, "\n");
	    (void) SockWrites(ciphertext, csp);
	    SockClose(csp);
	    free(csp);
	 }
      }
#endif
      expired_feature[0] = '\0';

      if (warning_msg)
	 time_count++;
      if (time_count == 120) {
	 fprintf(stderr, "[pid=%d]: ", proc_id);
	 fprintf(stderr,
#ifdef ESPS
		 "\n>>This is a reminder that you have an ESPS license checked out.<<\n"
#else
		 "\n>>This is a reminder that you have an HTK license checked out.<<\n"
#endif
	    );
	 time_count = 0;
      }
      if (!mit) {
	 ret = elm_alive(expired_feature);
	 if (ret != ELM_OK) {
	    elm_bye();
	    exit(1);
	 }
      }
   }
}


static void
byebye(code)
{
   elm_bye();
#ifdef ESPS
   fprintf(stderr, "\n%s: ESPS license (%s/%o) freed.\n",
	   program, feature, license_code);
   SockClose(sp);
#else
   fprintf(stderr, "\n%s: HTK license (%s) freed.\n",
	   program, feature);
#endif
   exit(0);
}

static void
background()
{
   int             fp, fork_ret;

#ifdef SIGTTOU
   signal(SIGTTOU, SIG_IGN);	/* Don't stop on terminal output */
#endif
#ifdef SIGTTIN
   signal(SIGTTIN, SIG_IGN);	/* Don't stop on terminal input */
#endif
#ifdef SIGTSTP
   signal(SIGTSTP, SIG_IGN);	/* Don't allow terminal stop signal */
#endif

   fork_ret = fork();		/* Background me */

   if (fork_ret == -1)
      byebye(-1, "Unable to fork");

   if (fork_ret != 0)		/* If parent... */
      exit(0);			/* die. */

   proc_id = getpid();

   /* start new process group */

   setsid();


   for (fp = 0; fp != fileno(stderr) && fp < 20; fp++)
      close(fp);

#ifdef DEBUG
   chdir("/tmp");
#else
   chdir("/");
#endif
   umask(0);
}
