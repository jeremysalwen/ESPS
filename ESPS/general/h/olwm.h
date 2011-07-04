/*
 *      (c) Copyright 1989, 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 */

/* @(#)  olwm.h 25.11 90/05/20  Crucible */

/* @(#)olwm.h	1.3 2/20/96 ERL */

#ifndef olwm_H
#define olwm_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#ifndef ABS
#define ABS(a)		(((a) < 0) ? -(a) : (a))
#endif

#ifndef MAX
#define	MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif

/* Determine the size of an object type in 32bit multiples.
 * Rounds up to make sure the result is large enough to hold the object. */
#define LONG_LENGTH(a)	((long)(( sizeof(a) + 3 ) / 4))

#define MAX_CHILDREN	10	/* size of child window array */

#define	FOOTLEN	50L

/* protocols bits */
#define		TAKE_FOCUS		(1<<0)
#define		SAVE_YOURSELF		(1<<1)
#define		DELETE_WINDOW		(1<<2)

/* Icon positioning modes */
typedef enum { AlongTop, AlongBottom, AlongRight, AlongLeft,
	       AlongTopRL, AlongBottomRL, AlongRightBT, AlongLeftBT
	     } IconPreference;

/* size of icon window */
#define ICON_WIN_WIDTH 60
#define ICON_WIN_HEIGHT 60
#define ICON_GRID_WIDTH 13
#define ICON_GRID_HEIGHT 13

/* types of button presses */
#define	CLICK		0
#define	DOUBLECLICK	1
#define	DRAG		2

/* Sizes of various graphics bits.  I don't know what will happen
 * if these are not even numbers. */
/* minimum window size */
#define MINSIZE 5

/* offsets within title bar of adornments */
#define	SHINEOFF		11 /* 15 */
#define	PUSHOFF			2  /* 5 */
#define	OKOFF			50

/* mouse button definitions */
#define	MB_MENU		1
#define MB_ADJUST	2
#define MB_SELECT	3

/* adornment pixmaps */
extern	Pixmap	pixIcon;
extern	Pixmap	pixmapGray;
extern	Pixmap	pixGray;

/* various client stuff */
extern struct _List *ActiveClientList;

/* miscellaneous functions */
extern int ExitOLWM();
extern void *GetWindowProperty();

/* state functions */
extern struct _client *StateNew();
extern void ReparentTree();
extern void StateNormIcon();
extern void StateIconNorm();
extern void StateWithdrawn();

/* root window functions */
extern struct _wingeneric *MakeRoot();

/* no-focus window functions */
extern struct _wingeneric *MakeNoFocus();
extern void NoFocusTakeFocus();
extern void NoFocusInit();
extern int NoFocusEventBeep();

/* client functions */
extern struct _client *ClientCreate();
extern Window ClientPane();
typedef struct _clientinboxclose {
	Display *dpy;
	int (*func)();
	short bx, by, bw, bh;
	Time timestamp;
} clientInBoxClosure;
extern void *ClientInBox();
extern void ClientSetFocus();

/* frame functions */
extern struct _winpaneframe *MakeFrame();
extern void FrameSetPosFromPane();
extern void FrameFullSize();
extern void FrameNormSize();
extern void FrameNewFooter();
extern void FrameNewHeader();
extern void FrameSetBusy();
extern void FrameWarpPointer();
extern void FrameUnwarpPointer();

/* generic frame functions */
extern int GFrameFocus();
extern int GFrameSelect();
extern int GFrameSetConfigFunc();
extern void GFrameSetStack();
extern void GFrameSetConfig();
extern int GFrameEventButtonPress();
extern int GFrameEventMotionNotify();
extern int GFrameEventButtonRelease();

/* icon functions */
extern void IconInit();
extern struct _winiconframe *MakeIcon();
extern void IconChangeName();
extern void DrawIconToWindowLines();
extern void IconShow();
extern void IconHide();
extern void IconSetPos();

/* icon pane functions */
extern struct _winiconpane *MakeIconPane();

/* pane functions */
extern struct _winpane *MakePane();

/* pinned menu functions */
extern struct _winmenu *MakeMenu();

/* colormap functions */
extern struct _wingeneric *MakeColormap();
extern void TrackSubwindows();
extern void UnTrackSubwindows();
extern void InstallColormap();
extern void InstallPointerColormap();
extern void UnlockColormap();
extern void ColorWindowCrossing();
extern struct _wingeneric *ColormapUnhook();
extern void ColormapTransmogrify();

/* selection functions */
extern Bool IsSelected();
extern struct _client *EnumSelections();
extern Time TimeFresh();
extern int AddSelection();
extern Bool RemoveSelection();
extern Bool ToggleSelection();
extern void ClearSelections();
extern void SelectionResponse();

/* decoration window functions */
extern struct _winpushpin *MakePushPin();
extern struct _winbutton *MakeButton();

/* general window functions */
extern void WinCallFocus();
extern void WinRedrawAllWindows();

/* general window event functions */
extern int WinEventExpose();
extern int WinNewPosFunc();
extern int WinNewConfigFunc();
extern int WinSetConfigFunc();

/* rubber-banding functions */
extern void UserMoveWindows();
extern void UserResizeWin();
extern Bool TraceBoundingBox();

/* busy windows */
extern struct _winbusy *MakeBusy();

#ifdef __cplusplus
}
#endif

#endif /* olwm_H */
