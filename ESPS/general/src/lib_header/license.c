/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 * 
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *     Copyright (c) 1990-1996  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Alan Parker
 * 
 * Brief description:  License manager functions
 * 
 */

static char    *sccs_id = "@(#)license.c	1.28 28 Oct 1999 ERL";

#ifdef NO_LIC
void check_header() {};
void check_header2() {};
void tsm_check() {};
void tsm_free() {};
void ZZZ() {ConnectSock();};
#else


#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <esps/esps.h>
#include <ctype.h>
#include <time.h>
#include <esps/unix.h>
#include <esps/ss.h>
#include <sys/param.h>
#define ANSI_C
#include <elm.h>
#include <elm_erl.h>


/* WARNING:  this was defined in previous versions of Elan license
   software in elmlib.h.  It is now in slm.h, but I'm not including 
   that here since we are using the compat library.
*/
#define ENCRYPT 0


#ifdef DEC_ALPHA
char           *elm_code();
int             gethostname();
#endif

char           *get_esps_base();
void            exec_command();
static int      debug = 0;
static char     server_host[ELM_HOST_LEN + 1], *h;
static char     this_host[ELM_HOST_LEN + 1];
static char     feature[40];
static int      header_checked = 0;

SOCKET         *sp = SS_NULL;
#define SOCK_BUF_LEN 80
char            sock_buffer[SOCK_BUF_LEN];
extern short    erl_ech_port;


#define MSG_ESPS \
"Sorry, but there is no ESPS license checked out for this host.  You must\n\
first run echeckout to get a license.  This program attempted to do so, but\n\
it failed for an unknown reason.  Try running echeckout manually and rerun\n\
this program.  See echeckout(1-ESPS)\n"

#define MSG_WAVES \
"This program requires that this site have a valid waves+ license or an ESPS\n\
license checked out on this host.   Neither of these cases appear to be true.\n\
If this site has an ESPS license, try running echeckout.\n"

#define MSG_ERS \
"This program requires that this site have a valid ERS or waves+ license or\n\
an ESPS license checked out on this host.  None of these cases appear to be\n\
met.   If this site has an ESPS license, try running echeckout.\n"

#define MSG_NOECHECKOUT \
"This is an ESPS program, but you do not seem to have echeckout.  Your\n\
ESPS_BASE is set to %s.   Check to be sure this is correct.\n"


static int
expired(j)
   long            j;
{
   time_t          now;
   time(&now);

   return (j && (now > j));
}

void
check_header(machine)
   int             machine;
{
   int             i, j, ret, license_code;
   struct elm_user user;
   struct elm_info info;
   char		   path[MAXPATHLEN+1];
   struct stat     stat_buf;

   if (getenv("ELM_DEBUG"))
      debug = 1;

   /*
    * this means that the license has already been checked; just do nothing
    * and return
    */
   if (erl_mit() || header_checked)
      return;

   /*
    * First try and connect to a local echeckout process.  If it is there,
    * and it validates, then just run without talking to the ELM daemon
    */

   erl_init_ech_port(); /* this is in elm_erl.c */

   if (gethostname(this_host, MAXHOSTNAMELEN) == -1) {
      fprintf(stderr,
	"Trouble getting your hostname.  Please have sysadmin check it.\n");
   } else {
      /*
       * try and connect to local echeckout.  if it is running ok, then we
       * will just assume that we can run
       */
      sp = ConnectSock(this_host, erl_ech_port);
      if (sp != SS_NULL) {	/* we connected to somebody at that port */
	 time_t          now;
	 char            cleartext[SOCK_BUF_LEN], ciphertext[SOCK_BUF_LEN];
         if (debug) fprintf(stderr,"Connected to echeckout\n");
	 time(&now);
	 sprintf(cleartext, "%x%x", now, 70);
	 strcpy(ciphertext, (char *) elm_code(ENCRYPT, cleartext));
	 strcat(ciphertext, "\n");
	 strcat(cleartext, "\n");
	 if (SockWrites(cleartext, sp) != 0)
	    fprintf(stderr, "Error writing to echeckout.\n");
	 else {
	    sock_buffer[0] = '\0';
	    (void) SockGets(sock_buffer, SOCK_BUF_LEN - 1, sp);
	    if (strcmp(sock_buffer, ciphertext) == 0) {
	       SockClose(sp);
	       header_checked = 1;
	       return;		/* ok to continue */
	    }
	 }
      }
   }

   /*
    * We didn't talk to a echeckout, so try and talk to the ELM daemon
    */

   if (debug) fprintf(stderr,"didn't connect to echeckout, call ELMD\n");

   erl_elm_init();

   /*
    * if the following code is turned on, then the program will run with
    * either a waves+ license present, or an ESPS license out
    */
#ifdef WAVES_PROG

   if (debug)
      fprintf(stderr, "calling elm_getulist() for waves-only\n");

   ret = elm_getinfo(&info, WAVES_FEATURE, 0);
   if (ret > 0 && info.numlic && !expired(info.expiration)) {
      if (debug)
	 fprintf(stderr, "found waves license, we are done\n");
      header_checked = 1;
      elm_bye();
      return;
   }
#endif

#ifdef ERS_PROG
   if (debug)
      fprintf(stderr, "calling elm_getulist() for ers-only\n");

   ret = elm_getinfo(&info, ERS_FEATURE, 0);
   if (ret > 0 && info.numlic && !expired(info.expiration)) {
      if (debug)
	 fprintf(stderr, "found ers license, we are done\n");
      header_checked = 1;
      elm_bye();
      return;
   }
#endif

   license_code = mach_license_codes[MACH_CODE].license_code;

   if (debug) fprintf(stderr,"calling getinfo for esps\n");
   if (ret = elm_getinfo(&info, ESPS_FEATURE, 0) > 0) {
      char *host_field = erl_get_host_field(info.vendordata,(char**)NULL,8);
      if (debug)
	 fprintf(stderr,"host field: %s, this host: %o\n",
		 host_field,license_code);
      if (((int)strtol(host_field,(char **)NULL, 8) & license_code) == 0) {
	 fprintf(stderr,
	 "Sorry, but you do not have a license for ESPS on host type: %s\n",
		 mach_license_codes[MACH_CODE].host_type);
         elm_bye();
	 exit(1);
      }
   } else {
      fprintf(stderr, "%s.\n", elm_message(NULL, "ESPS", ret));
      elm_bye();
      exit(1);
   }

   if (debug) fprintf(stderr,"calling getulist for esps\n");
   for (i = 1; (i = elm_getulist(&user, ESPS_FEATURE, i)) > 0; i++) {
      if ((strcmp(this_host, user.host) == 0)) {
	 header_checked = 1;
         elm_bye();
	 return;
      }
   }

#ifdef ERS_PROG
   fprintf(stderr, MSG_ERS);
   elm_bye();
   exit(1);
#endif
#ifdef WAVES_PROG
   fprintf(stderr, MSG_WAVES);
   elm_bye();
   exit(1);
#endif

   fprintf(stderr, "No ESPS license checked out. Trying...");
   sprintf(path, "%s/bin/echeckout", get_esps_base(NULL));
   if (!stat(path, &stat_buf) && S_ISREG(stat_buf.st_mode)) {
      exec_command(path);
      (void) sleep(1);
      for (i = 1; (i = elm_getulist(&user, ESPS_FEATURE, i)) > 0; i++) {
	 if ((strcmp(this_host, user.host) == 0)) {
	    header_checked = 1;
            elm_bye();
	    return;
	 }
      }
      fprintf(stderr, "failed\n");
      fprintf(stderr, MSG_ESPS);
      elm_bye();
      exit(1);
   } else {
      fprintf(stderr, "failed\n");
      fprintf(stderr,MSG_NOECHECKOUT,get_esps_base(NULL));
      elm_bye();
      exit(1);
   }
}

void
waves_check()
{
   int             i, ret;

   erl_elm_init();

   ret = elm_getlicense(WAVES_FEATURE, ELM_GETLIC, 0L);

   if (ret < 0) {
      fprintf(stderr, "%s.\n", elm_message(NULL, "waves+", ret));
      elm_bye();
      exit(1);
   } else {			/* license OK */
      header_checked = 1;
      return;
   }
}

int
erl_mit()
{
    if (erl_elm_mit()) {
	header_checked = 1;
	return 1;
    } else
        return 0;
}

void
lm_quit()
{
   elm_bye();
}

void
tsm_check()
{
   int             ret;

   erl_elm_init();

   ret = elm_getlicense(TSM_FEATURE, ELM_GETLIC, 0L);

   if (ret < 0) {
      fprintf(stderr, "%s.\n", elm_message(NULL, "TSM", ret));
      erl_elm_bye();
      exit(1);
   } else {			/* license OK */
      header_checked = 1;
      return;
   }
}

/*
 * This function is used to check to see if a waves+ or esps license exist.
 * If it has already been checked, then just return fast.
 */

static          check_header2_called = 0;

void
check_header2()
{

   int             i, j, ret;
   struct elm_info info;

   if (getenv("ELM_DEBUG"))
      debug = 1;

   if (check_header2_called || header_checked || erl_mit())
      return;

   check_header2_called = 1;

   erl_elm_init();

   ret = elm_getinfo(&info, ESPS_FEATURE, 0);
   erl_elm_bye();
   if (ret > 0 && info.numlic && !expired(info.expiration))
      return;

   ret = elm_getinfo(&info, WAVES_FEATURE, 0);
   if (ret > 0 && info.numlic && !expired(info.expiration))
      return;

   if (ret < 0) {
      fprintf(stderr, "%s.\n", elm_message(NULL, NULL, ret));
      exit(1);
   }
}

void
ers_check(feature)
   char           *feature;
{
   int             ret;

   fprintf(stderr, "ers_check: you must convert over to the new LM\n");
   exit(0);
   erl_elm_init();

   ret = elm_getlicense(ERS_FEATURE, ELM_GETLIC, 0L);

   if (ret < 0) {
      fprintf(stderr, "%s.\n", elm_message(NULL, NULL, ret));
      exit(1);
   }
   header_checked = 1;
   return;
}

void
tsm_free()
{
   erl_elm_bye();
}

#endif /* NO_LIC */
