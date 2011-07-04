/*
 *      (c) Copyright 1989, 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See XVIEW LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	OPEN LOOK is a trademark of AT&T. 
 *      Used by written permission of the owners.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 */

/*
 * The original Sun definitions here were taken from olwm/usermenu.c and 
 * modified by Entropic Research Laboratory, Inc. The same terms as 
 * the XView LEGAL_NOTICE apply.  
 *
 *    "Copyright (c) 1991-1995  Entropic Research Laboratory, Inc. 
 *
 * In almost all cases, modifications consist of deletions from usermenu.c 
 * or additions from some other XView files.
 *
 *  @(#)exv_olwm.h	1.4	2/20/96 ERL
 */

/* The following typedefs for buttondata and menudata are taken from 
 * Sun's OLWM code (by Celerity) and are covered by the XView copyright;
 * (see bboxmenu.c). The defines are from olwm/usermenu.c
 */

#ifndef exv_olwm_H
#define exv_olwm_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#include <esps/olwm.h>
#include <esps/mem.h>
#include <esps/menu.h>

/* parseMenu return values */
#define MENU_FATAL     -1
#define MENU_NOTFOUND	0
#define MENU_OK		1
#define MENU_PINNABLE	2

typedef struct _buttondata {
	struct _buttondata *next;
	char *name;
	Bool isDefault;
	Bool isLast;
	FuncPtr func;
	char *exec;    /* string to be executed, like "xterm" */
	void *submenu; /* this really is type menudata, but 
			  can't be since defined below  - js*/
	} buttondata;

typedef struct {
	char *title;
	char *label;
	int idefault;		/* index of default button */
	int nbuttons;
	Bool pinnable;
	buttondata *bfirst;
} menudata;

#ifdef __cplusplus
}
#endif

#endif /* exv_olwm_H */
