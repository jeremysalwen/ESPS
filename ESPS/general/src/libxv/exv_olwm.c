/*
 *     "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * see the additional copyright notices below

 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 *       contains slightly revised versions of Sun olwm menu functions
 *       plus some additional ESPS cover functions.  
 *
 */

static char *sccs_id = "@(#)exv_olwm.c	1.6	12/15/95	ERL";

/*
 *      (c) Copyright 1989, 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See XVIEW LEGAL_NOTICE
 *      file for terms of the license.
 *
 *	OPEN LOOK is a trademark of AT&T. 
 *      Used by written permission of the owners.
 *
 *	Written for Sun Microsystems by Crucible, Santa Cruz, CA.
 *
 *     @(#) usermenu.c 25.4 90/05/23 Crucible
 *
 *     @(#) mem.c 25.3 90/05/01 Crucible
 *
 * The original Sun programs here were taken from olwm/usermenu.c and 
 * and olwm/mem.c and modified (slightly) by Entropic Research Laboratory, 
 * Inc. The same terms as in the XView LEGAL_NOTICE apply.  
 *
 */


/*
 * Syntax of the user menu file should be identical to that used by
 *	buildmenu (SunView style rootmenu files).
 *
 *	NOTICE that SunView compatibility has resulted in old-style 
 *	olwm menus no longer being supported.
 *
 *	There are two new reserved keywords: 
 *
 *		DEFAULT tags a default button
 *		TITLE tags a title string for a menu (for titlebar)
 *
 *	One syntax in sunview menus is not supported:
 *		<icon_file> can not be used as a menu item 
 *
 *	Here are the common reserved keywords:
 *		MENU and END are used to delimit a submenu
 *		PIN (appearing after END) indicates the menu is pinnable
 *		EXIT (built-in - olwm service)
 *		REFRESH (built-in - olwm service)
 *		POSTSCRIPT will invoke psh on the named command
 *
 * 	The file is line-oriented, however commands to be executed can
 *	extend to the next line if the newline is escaped (\).
 *
 *	Each line consists of up to three fields:  a label (a string 
 *	corresponding to either the menu label or menu option label),
 *	up to two tags (keywords), and a command to be executed
 *	(or a file from which to read a submenu).  Two tags are allowed
 *	if one of them is "DEFAULT" or "END".  
 *
 *	The tag is used to indicate the start and end of menu definitions,
 *	pinnability, built-in functions, and default options.  
 *	The label indicates the text which appears on the user's menu,
 *	and the command describes what should be done when each item
 *	is selected.
 *
 *	Labels must be enclosed in double quotes if they contain 
 *	whitespace.  Commands may be enclosed in double quotes (but
 *	do not have to be).
 *
 *	Comments can be embedded in a file by starting a line with a
 *	pound sign (#).  Comments may not be preserved as the file is 
 *	used.
 *
 *	There are several functions which aren't invoked as programs;
 *	rather, they are built in to window manager.  These built-in
 *	services are each denoted by a single keyword.  The keywords are
 *	listed in the svctokenlookup[] array initialization.
 *
 *	example (will always have label: "Workspace Menu"):
 *
 *	"Workspace Menu"	TITLE
 *	Programs	MENU
 *		"Helpful Programs"	TITLE
 *		"Command Tool"	cmdtool
 *		"Blue Xterm"	DEFAULT xterm -fg white \
 *				-bg blue
 *	Programs	END	PIN
 *	Utilities	MENU
 *		"Refresh Screen" DEFAULT REFRESH
 *		"Clipboard"	 CLIPBOARD
 *	Utilities	END
 */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#ifdef OS5
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/file.h>
#include <sys/param.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#if !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif
#include <memory.h>
#include <sys/types.h>

extern char *strtok();		/* not defined in strings.h */

#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/icon_load.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>

#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/exview.h>

#define TOKLEN		300

#ifndef NULL
#define NULL 0
#endif

typedef enum { UsrToken, MenuToken, EndToken, DefaultToken, PinToken, 
	TitleToken, ServiceToken, PshToken } TokenType;

/* locally useful macro */
#define	APPEND_STRING(buf, str)	( strncat( buf, str, \
					( sizeof(buf) - strlen(buf) - 1 ) ) )
/* Externals */

/* local forward declarations */

static int	parseMenu();
static void	fillMenuStruct();
static TokenType lookupToken();
static void	initMenu();
static void	initButton();
static void	freeButtonData();
static void	freeMenuData();
static int       menuFromFile();

/* 
 * read_olwm_menu - cover function to read menu from file; John Shore
 */

menudata *
read_olwm_menu(menu_file)
char *menu_file;
{
  int ret;

  menudata *menu = (menudata *) malloc(sizeof(menudata));

  /* not all of these are really needed, but it's cleaner */

  menu->title = "ERL Default";
  menu->label = NULL;
  menu->idefault = -1;
  menu->nbuttons = 0;
  menu->pinnable = True;
  menu->bfirst = NULL;

  ret = menuFromFile(menu_file, menu, True);

  if (ret == MENU_OK)
    return (menu);
  else
    return (NULL);
}

/*
 * prints an olwm menu  - John Shore
 */

void
print_olwm_menu(men)
menudata *men;
{
  buttondata *butn;

  if ((men == NULL) || (men->bfirst == NULL)) {
      fprintf(stderr, "print_olwm_menu:  no menu to print.\n");
      return;
    }

  fprintf(stderr, "NEW MENU\nname = %s, nbuttons = %d\n", 
	  men->title, men->nbuttons);

  butn = men->bfirst; 
  
  while (butn != NULL) {
    fprintf(stderr, "name = %s\n", butn->name);
    if (butn->exec != NULL)  {
	fprintf(stderr, "exec = %s\n", butn->exec);
      }
    if (butn->submenu != NULL)  {
	print_olwm_menu((menudata *) butn->submenu);
      }
    butn = butn->next;
  }
}


/*
 * menuFromFile - read a menu description from a file
 *
 *	Return values: same as parseMenu, with the addition of
 *		MENU_NOTFOUND = couldn't read submenu file
 */
static int
menuFromFile(file, menu, messages)
char		*file;
menudata	*menu;
Bool		messages;
{
	FILE       *stream;
	int         lineno = 1;	/* Needed for recursion */
	int         rval;

	stream = fopen(file, "r");
	if (stream == NULL) {
	    if (messages)
	     fprintf(stderr, "menuFromFile: can't open menu file %s\n", file);
	    return(MENU_NOTFOUND);
	}

	rval = parseMenu(file, stream, menu, &lineno);
	fclose(stream);
	if (rval >= MENU_OK)
	    fillMenuStruct(menu);
	else
	    freeMenuData(menu);

	return(rval);
}


/*
 * parseMenu -- read the user menu from the given stream and
 *	parse the stream into the menu structures defined locally.
 *	These structures (which are local to this module) are later
 *	used to build real menu structures.
 *
 *	Note that fillMenuStruct() needs to be called after parseMenu()
 *	is called (to finish filling out the menudata structure).
 *	If parseMenu() returns < 0, then freeMenuData() needs to be 
 *	called instead, to free up unused memory.
 *
 *	Return values:
 *		MENU_OK		= an unpinnable menu was read successfully
 *		MENU_PINNABLE	= a pinnable menu was read successfully
 *		MENU_FATAL	= a fatal error was encountered
 *
 *	This is based heavily on buildmenu's getmenu() parsing routine.
 *
 */
static int
parseMenu(filename, stream, rootmenu, lineno)
char	 *filename;
FILE	 *stream;
menudata *rootmenu;
int      *lineno;
{
	menudata	*currentMenu, *saveMenu;
	buttondata	*currentButton;
	char		line[TOKLEN];
	char		label[TOKLEN];
	char		prog[TOKLEN];
	char		args[TOKLEN];
static	char		localBuf[1024];
	char		*nqformat = 
				"%[^ \t\n]%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";
	char		*qformat = 
				"\"%[^\"]\"%*[ \t]%[^ \t\n]%*[ \t]%[^\n]\n";
	char		*format;
	register char	*p;
	int		continuation;
	Bool		done;

	currentMenu = rootmenu;
	initButton( (buttondata **)&(currentMenu->bfirst) );
	currentButton = currentMenu->bfirst;
	continuation = 0;

	for ( ; fgets(line, sizeof(line), stream) ; (*lineno)++ )
	{
	    if (line[0] == '#') 
		    continue;

	    for ( p = line ; isspace(*p) ; p++ )
		    ;

	    if ( *p == '\0' )
		    continue;

	    /*
	     * if we're already on a continuation line (the previous 
	     * line ended in '\') then just copy the input through
	     * to the output until we get a line that doesn't end in 
	     * '\' (nuke the vi backslash).
	     */
	    if (continuation) 
	    {
		    /* fgets includes the newline in the string read */
		    while ( line[strlen(line) - 2] == '\\' )
		    {
			    /* get rid of backslash */
			    line[strlen(line) - 2] = '\0'; 
			    APPEND_STRING( localBuf, " " );
			    APPEND_STRING(localBuf, p);
			    if ( !fgets(line, sizeof(line), stream) )
				    break;
			    (*lineno)++;
			    for ( p = line ; isspace(*p) ; p++ )
				    ;
		    }
		    /* last line of continuation - replace \n with \0 */
		    line[strlen(line) - 1] = '\0';
		    APPEND_STRING( localBuf, " " );
		    APPEND_STRING(localBuf, p);
		    /* save it permanently in the buttondata structure */
		    currentButton->exec = MemNewString( localBuf );
		    localBuf[0] = '\0';
		    continuation = 0;
		    initButton( (buttondata **)&(currentButton->next) );
		    currentButton = currentButton->next;
		    continue;
	    }

	    /* if the line ends in '\' remember that continuation 
	     * has started.
	     */
	    if ( line[strlen(line) - 2] == '\\' )
	    {
		    continuation = 1;
		    line[strlen(line) - 2] = '\0';
	    }
	
	    args[0] = '\0';
	    format = ( *p == '"' ) ? qformat : nqformat;

	    if ( sscanf( p, format, label, prog, args ) < 2 )
	    {
		    fprintf(stderr,
			"parseMenu: syntax error in menu file %s, line %d\n", 
			    filename, *lineno);
		    return(MENU_FATAL);
	    }

	    if ( strcmp(prog, "END") == 0 ) 
	    {
		    /* currently allocated button is last for this menu */
		    currentButton->isLast = True;
		    if (currentMenu->label != NULL &&
			strcmp(label, currentMenu->label) != 0) {
			fprintf(stderr,
			    "parseMenu: menu label mismatch in file %s, line %d\n",
			    filename, *lineno);
			return(MENU_FATAL);
		    }
		    if ( strcmp(args, "PIN") == 0 )
			    return(MENU_PINNABLE);
		    else
			    return(MENU_OK);
	    }

	    if ( strcmp(prog, "TITLE") == 0 ) 
	    {
		    currentMenu->title = MemNewString( label );
		    /* we don't need to set up the next button, since
		     * the TITLE line didn't use up a button
		     */
		    continue;
	    }

	    currentButton->name = MemNewString( label );

	    if ( strcmp(prog, "DEFAULT") == 0) {
		char *t;
		char *u;

		currentButton->isDefault = True;

		/*
		 * Pull the first token from args into prog.
		 */
		t = strtok(args, " \t");
		if ( t == NULL ) {
		    fprintf(stderr,
			    "parseMenu: error in menu file %s, line %d\n",
			    filename, *lineno);
		    fputs("missing item after DEFAULT keyword.\n", stderr);
		    return(MENU_FATAL);
		}
		strcpy(prog, t);
		t = strtok(NULL, ""); /* get remainder of args */
		if (t == NULL)
		    args[0] = '\0';
		else {
		    u = args;
		    /* can't use strcpy because they overlap */
		    while ( *u++ = *t++ )
			;
		}
	    }

	    if ( strcmp(prog, "MENU") == 0 ) 
	    {
		    int         rval;

		    initMenu( (menudata **)&(currentButton->submenu) );
		    saveMenu = currentMenu;
		    currentMenu = (menudata *)currentButton->submenu;
		    currentMenu->label = MemNewString(label);

		    if (args[0] == '\0') 
		    {
			    /* we haven't incremented lineno for this
			     * read loop yet, so we need to do it now.
			     * when END is read, parseMenu returns without
			     * incrementing lineno, so the count will be
			     * ok when this loop increments it before
			     * reading the next line of the file.
			     */
			    (*lineno)++;
			    if ( (rval = parseMenu(filename, stream, 
						   currentMenu, lineno)) < 0 )
			    {
				freeMenuData( currentMenu );
				return(MENU_FATAL);
			    }
			    else
				fillMenuStruct( currentMenu );
		    }
		    else {
			rval = menuFromFile(args, currentMenu, True);
			if (rval <= MENU_NOTFOUND)
			    return(MENU_FATAL);
		    }
		    if ( rval == MENU_PINNABLE ) 
			    currentMenu->pinnable = True;

		    currentMenu = saveMenu;
		    /* if submenu not found, reuse button */
		    if ( rval != MENU_NOTFOUND )
		    {
		        initButton( (buttondata **)&(currentButton->next) );
		        currentButton = currentButton->next;
		    }
		    continue;			
	    }

	    done = False;
	    while ( !done )
	    {
		switch ( lookupToken( prog, &(currentButton->func) ) )
		{
		case UsrToken:
		    /* if UsrToken, that means that "prog" was just
		     * the first word of the command to be executed,
		     */
		    strcpy( localBuf, prog );
		    APPEND_STRING( localBuf, " " );
		    APPEND_STRING( localBuf, args );
 		    /* copy current contents of localBuf back into
		     * args array so that PshToken code can be used
		     */
		    strcpy( args, localBuf );
		    localBuf[0] = '\0';
		    /* fall through */
		case PshToken:
		    if (continuation) 
		    	strcpy( localBuf, args );
		    else
		    	currentButton->exec = MemNewString( args );
		    done = True;
		    break;
		case PinToken:
		    fprintf( stderr, 
			"parseMenu: format error in menu file %s, line %d\n", 
			filename, *lineno );
		    fputs("menu title and END required before PIN keyword.\n",
			  stderr);
		    return(MENU_FATAL);
		    break;
		default:
		    /* some other valid token found and returned */
		    done = True;
		    break;
		}
	    }
		    
	    if ( !continuation )
	    {
	        initButton( (buttondata **)&(currentButton->next) );
	        currentButton = currentButton->next;
	    }
	}
	/* never used the last button created */
	currentButton->isLast = True;

	return(MENU_OK);
}

/* 
 * fillMenuStruct - Once the menu structures have been filled out using 
 * 	information in the menu description file (via parseMenu()), the 
 * 	nbuttons and idefault elements need to be set.
 */
static void
fillMenuStruct( mptr )
menudata	*mptr;
{
	buttondata	*bptr;
	int		buttonIndex = 0;

	bptr = mptr->bfirst;
	if ( bptr->isLast == True )
	{
		MemFree( bptr );
		bptr = mptr->bfirst = NULL;
	}
	for ( ; bptr != NULL && bptr->isLast == False ; bptr = bptr->next )
	{
		if ( bptr->isDefault == True )
			mptr->idefault = buttonIndex;

		if ( (bptr->next)->isLast == True )
		{
			MemFree( bptr->next);
			bptr->next = NULL;
		}

		buttonIndex++;
	}
       /* buttonIndex is one past end, but started at 0, so = number buttons */
	mptr->nbuttons = buttonIndex;
}

/* 
 * Allowed menu keywords ("Token") 
 */

struct _svctoken {
	char *token;
	FuncPtr func;
	TokenType toktype;
} svctokenlookup[] = {
/*	{ "REFRESH", RefreshFunc, ServiceToken },*/
/*	{ "CLIPBOARD", ClipboardFunc, ServiceToken },*/
/*	{ "PRINT_SCREEN", PrintScreenFunc, ServiceToken },*/
/*	{ "EXIT", ExitFunc, ServiceToken },*/
/*	{ "WMEXIT", ExitOLWM, ServiceToken },*/
/*	{ "PROPERTIES", PropertiesFunc, ServiceToken },*/
/*	{ "NOP", NopFunc, ServiceToken },*/
	{ "DEFAULT", NULL, DefaultToken },
	{ "MENU", NULL, MenuToken },
	{ "END", NULL, EndToken },
	{ "PIN", NULL, PinToken },
	{ "TITLE", NULL, TitleToken },
/*	{ "WINDOW_CONTROLS", WindowCtlFunc, ServiceToken },*/
/*	{ "FLIPDRAG", FlipDragFunc, ServiceToken },*/
/*	{ "SAVE_WORKSPACE", SaveWorkspaceFunc, ServiceToken },*/
/*	{ "POSTSCRIPT", PshFunc, PshToken },*/
/*	{ "RESTART", RestartOLWM, ServiceToken },*/
/*	{ "FLIPFOCUS", FlipFocusFunc, ServiceToken },*/
};

#define NSERVICES (sizeof(svctokenlookup)/sizeof(struct _svctoken))

/* lookupToken -- look up a token in the list of tokens
 *	given a supposed keyword or service name.  If the name doesn't
 *	match any existing token, return the user-defined token.  
 */
static TokenType
lookupToken(nm,ppf)
char *nm;
FuncPtr *ppf;
{
	int ii;

	for (ii=0; ii<NSERVICES; ii++)
	{
		if (!strcmp(nm,svctokenlookup[ii].token))
		{
			if (ppf != (FuncPtr *)0) 
				*ppf = svctokenlookup[ii].func;
			return svctokenlookup[ii].toktype;
		}
	}


/* deleted - js
	if (ppf != (FuncPtr *)0)
		*ppf = AppMenuFunc;
*/
	return UsrToken;
}


/*
 * initMenu - 
 */
static void
initMenu( newmenu )
menudata	**newmenu;
{
	*newmenu = MemNew(menudata);
	(*newmenu)->title = NULL;
	(*newmenu)->label = NULL;
	(*newmenu)->idefault = -1;
	(*newmenu)->nbuttons = 0;
	(*newmenu)->pinnable = False;
	(*newmenu)->bfirst = (buttondata *)0;
}

/*
 * initButton - 
 */
static void
initButton( newButton )
buttondata	**newButton;
{
	*newButton = MemNew(buttondata);
	(*newButton)->next = NULL;
	(*newButton)->name = NULL;
	(*newButton)->isDefault = False;
	(*newButton)->isLast = False;
	(*newButton)->func = (FuncPtr)0;
	(*newButton)->exec = NULL;
	(*newButton)->submenu = NULL;
}

/*
 * freeMenuData - free any possibly allocated memory for this menudata 
 *	structure (and its buttons), since it's not going to be used
 */
static void
freeMenuData( unusedMenu )
menudata	*unusedMenu;
{
	buttondata	*unusedButton;

	/* isLast probably isn't set, since this menu had an error */
	if ( ( unusedButton = unusedMenu->bfirst ) != (buttondata *)0 )
		freeButtonData( unusedButton );

	MemFree( unusedMenu->title );
	MemFree( unusedMenu->label );
	MemFree( unusedMenu );
	unusedMenu = NULL;
}

/*
 * freeButtonData - free any possibly allocated memory for this buttondata 
 *	structure, since it's not going to be used
 */
static void
freeButtonData( unusedButton )
buttondata	*unusedButton;
{

	if ( unusedButton->next != NULL )
		freeButtonData( unusedButton->next );

	MemFree( unusedButton->name );
	MemFree( unusedButton->exec );
	if ( unusedButton->submenu != NULL )
		freeMenuData( unusedButton->submenu );
	MemFree( unusedButton );
	unusedButton = NULL;
}

/*
 * Safe memory allocation/free routines - front-ends the C library functions
 *
 */

void *
MemAlloc(sz)
unsigned int sz;
{
#ifdef __STDC__
	void *p;
#else
	char *p;
#endif

	if ((p = malloc(sz)) == NULL) {
	    (void) fprintf(stderr, "Memory allocation failure.\n");
	    exit(1);
	  }
	memset((char *)p, 0, (int)sz);
	return p;
}

void
MemFree(p)
void *p;
{
	if (p != NULL)
		free(p);
}

