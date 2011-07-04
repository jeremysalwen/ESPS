/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: free an HTK or waves license
 *
 */

static char *sccs_id = "%W%	%G%	ERL";

#include <elm.h>
#include <elm_erl.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>

extern char    *getenv();
extern int      gethostname();

#ifdef WAVES
char           *feature_name = WAVES_FEATURE;
char           *program = "freewaves";
#endif

#ifdef HTK
char           *feature_name = HTK_FEATURE;
char           *program = "hfree";
#endif

struct elm_user user;
struct passwd  *pw, *getpwuid();
char           *user_name;
int             gotit = 0, i, ret;
char           *alias;


main(argc, argv)
   int             argc;
   char          **argv;
{

   erl_elm_init();

   user_name = erl_elm_username();
   alias = erl_decode_feature(feature_name);

   fprintf(stderr, "Looking for %s license for user %s...", alias, user_name);
   for (i = 1; (i = elm_getulist(&user, feature_name, i)) > 0; i++) {
      if ((strcmp(user_name, user.user) == 0)) {
	 fprintf(stderr, "found it (cid=%d)...", i);
	 gotit = 1;
	 ret = elm_control(ELMC_BURYCLIENT, i);
	 if (ret < 0)
	    fprintf(stderr, "cannot free it!\n");
	 else
	    fprintf(stderr, "freed it.\n");
	 break;
      }
   }
   if (!gotit)
      fprintf(stderr, "cannot find one.\n");
   elm_bye();
}
