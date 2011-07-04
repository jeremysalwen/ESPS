/*
 *      (c) Copyright 1989, 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 *
 *     "@(#) services.c 25.17 90/05/31 Crucible";
 *
 *     "@(#) olwm.c 25.14 90/06/05 Crucible";
 *
 * The original Sun programs here were taken from olwm/services.c and 
 * olwm/olwm.c and modified (in most cases, slightly) by Entropic 
 * Research Laboratory, Inc. The same terms as in the XView 
 * LEGAL_NOTICE apply.  
 *
 *    "Copyright (c) 1991  Entropic Research Laboratory, Inc. 
 *
 */

static char *sccs_id = "@(#)execcom.c	1.3	4/6/93	ERL";

#include <stdio.h>

#ifdef sony
extern int errno;
#endif

#ifdef SUN4
#ifndef OS5
#include <vfork.h>
#endif
#endif

/*
 * Execute a command by handing it to /bin/sh.  This was based on 
 * the olwm routine execCommand; we use execvp instead of execve
 * since we don't want to mess with the unix environment.  
 */

void
exec_command(cmd)
     char *cmd;
{
    char *args[4];
    int pid;

    args[0] = "/bin/sh";
    args[1] = "-c";
    args[2] = cmd;
    args[3] = NULL;

#ifdef SUN4
    pid = vfork();
#else
    pid = fork();
#endif
    if (pid == -1) {
	perror("exec_command: fork");
    } else if (pid == 0) {
	/* child */
	setpgrp(0, getpid());
	execvp(args[0], args);
	perror("exec_command: exec");
	exit(1);
    }
}

