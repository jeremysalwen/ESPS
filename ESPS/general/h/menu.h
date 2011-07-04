/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 */

/* @(#) menu.h 25.1 90/04/10 Crucible */

/* @(#)menu.h	1.3 2/20/96 ERL */

/* 
 * stuff was deleted from original in olwm/menu.h  -- js
 */ 

#ifndef menu_H
#define menu_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

/* a handy typedef for pointers to functions returning int */
typedef int (*FuncPtr)();

/* Button stacks (menus) are implemented as lists of buttons.
 * Each button in a stack may in turn be stacked, this is indicated by
 * the stacked flag of the button. If this flag is True then the buttonAction
 * is a pointer to a new stack. Otherwise it is the function to be called
 * after the menu has been dispatched.
 */
typedef struct {
	FuncPtr		callback;	/* if not stacked; call this */
	struct _menu	*submenu;	/* actually a Menu pointer, but */
					/* Menu hasn't been defined yet */
} ButtonAction;

/* possible button states */
typedef enum {Disabled, Enabled} ButtonState;

typedef struct _button {
	char		*label;		/* displayed text */
	Bool		stacked;	/* True if this is a button stack */
	ButtonState	state;		/* Enabled/Disabled */
	ButtonAction	action;

	/* following are filled in when menu is created */
	int		activeX, activeY, activeW, activeH;
} Button;

/* external functions */
extern void InitMenus(/* dpy */);
extern void MenuCreate(/* dpy, menu */);
extern void MenuShow(/* dpy, WinGeneric, menu, event */);
extern void SetButton(/* dpy, menu, bindex, Bool */);
extern void ExecButtonAction(/* dpy, winInfo, menu, btn, Bool */);
extern void DrawMenu(/* dpy, menu */);
extern int  PointInRect(/* x, y, rx, ry, rw, rh */);

#ifdef __cplusplus
}
#endif

#endif /* menu_H */
