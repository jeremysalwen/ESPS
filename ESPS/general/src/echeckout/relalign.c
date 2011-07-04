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

#ifdef NO_LIC
main() {exit(0);}
#else

#include <elm.h>
#include <elm_erl.h>
#include <stdio.h>
#ifdef MACII
#include <sys/types.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <esps/esps.h>
#include <esps/mach_codes.h>
#include <sys/param.h>
#include <ctype.h>
#include <esps/ss.h>
#include <pwd.h>

extern char    *getenv(), *strchr();
extern int      gethostname();
static char     server_host[101];
extern int      getpid();
extern char    *optarg;
extern int      optind;
int             c;
#ifndef NO_LIC
extern long     elm_var;
#else
long		elm_var;
#endif
int		debug_level=0;



/*
#define DEBUG 1
*/

char           *program = "relalign";

struct elm_user user;
int             i;

char            *statfile;
FILE		*statfile_fp;


#define ITIMER_NULL ((struct itimerval *)0)

#define DOWN_MSG1 \
"\nUnable to find an Entropic license server.  You have ELM_HOST defined\n\
to %s.  Be sure that elmd is running on host %s, or \n\
correct ELM_HOST.\n"

#define DOWN_MSG2 \
"\nUnable to find an Entropic license server.  Be sure elmd is running\n\
on the host system for which the license keys were issued for.  If you \n\
know the name of the host running elmd, then try setting the evironment\n\
variable ELM_HOST to that hostname.\n"


char            feature[50];

#define SYNTAX USAGE("relalign")

main(argc, argv)
	int             argc;
	char          **argv;
{
	int             ret,i,j;
	char           *h;
	char            machine_type[100];


	while ((c = getopt(argc, argv, "x")) != EOF) {
		switch (c) {
		case 'x':
			debug_level = 1;
			break;
		default:
			SYNTAX;
			break;
		}
	}

#ifdef NO_LIC
	exit(0);
#endif


	/*
	 * assume that the server is on the current host, unless this
	 * environment variable is set
	 */

	server_host[0] = '\0';
	if ((h = getenv("ELM_HOST")) && h[0])
		strcpy(server_host, h);

	elm_var = 0;

	/*
	 * initialize the connection to the license server
	 */
	ret = elm_init(server_host, (char *) NULL);
	switch (ret) {
		case ELM_OK:
			break;
		case ELM_WAIT:
			fprintf(stderr,
			"\n%s: License manager on server %s initializing.\n",
			program,server_host);
			fprintf(stderr,
			"%s: Please try again in %d seconds\n",
			program,elm_var);
			exit(-1);
		case ELM_NOHOST:
			fprintf(stderr,
			"\n%s: Invalid host name: %s\n", program,server_host);
			fprintf(stderr,
			"%s: Check your ELM_HOST environment variable.\n",
			program);
			exit(-1);
		case ELM_SRV_DOWN:
			if (h && h[0]) 
			  fprintf(stderr,DOWN_MSG1,h,h);
			 else 
			  fprintf(stderr,DOWN_MSG2);
			exit(-1);
		case ELM_AUTH_FAIL:
			fprintf(stderr,
"\n%s: Authorization Failure. Perhaps the elmd is running on the wrong host.",
	program);
			exit(-1);
		default:
			(void)fprintf(stderr,
				"\n%s: Network or system error; code: %d\n",
				program,ret);
			(void)fprintf(stderr,
				"%s: Perhaps your network is not running?\n",
				program);
			fprintf(stderr,
				"%s: Contact Entropic if you cannot resolve this.\n",program);
			exit(-1);
		}
	if(debug_level) fprintf(stderr,"Done init..\n");


	ret = elm_relalign();
	if(debug_level) {
	switch (ret) {
		case ELM_OK:
			break;
		case ELM_FAIL:
			(void)fprintf(stderr,
			"\n%s: No licenses currently are held.\n",program);
			exit(-1);
		case ELM_EXPIRED:
			(void)fprintf(stderr,
			"\n%s: Sorry, but your license has expired.\n",
			program);
			exit(-1);
		case ELM_SRV_DOWN:
			if (server_host[0])
			    fprintf(stderr,"\n%s: License server on %s is down.", 
			    program,server_host);
			else
			    fprintf(stderr,"\n%s: License server is down.",program);
			exit(-1);
		case ELM_AUTH_FAIL:
			fprintf(stderr,
"\n%s: Authorization Failure. Perhaps the elmd is running on the wrong host.",
	program);
			exit(-1);
		case ELM_ERROR:
			fprintf(stderr,
"\n%s: Error; is the key file installed and readable?\n",program);
			exit(-1);
		default:
			(void)fprintf(stderr,
				"\n%s: Network or system error; code: %d\n",
				program,ret);
			(void)fprintf(stderr,
				"%s: Perhaps your network is not running?\n"
				,program);
			fprintf(stderr,
				"%s: Contact Entropic if you cannot resolve this.\n",program);
			exit(-1);
		}
	    fprintf(stderr,
		"\n\n%s: %s license released OK.\n", program, feature);
	}
	elm_bye();
	exit(0);

}

#endif
