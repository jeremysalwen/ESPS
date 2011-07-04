/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1995 Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Alan Parker Checked by: Revised by:
 * 
 * Brief description: Tk communications routines for non tcl/tk programs
 * 
 * This code is heavily derived from the tkSend.c file in the TK 4.0 source code
 * written by John Ousterhout.
 * 
 */

static char    *sccs_id = "@(#)xsend.c	1.7	8/22/97	ERL";

#if !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
#define memmove memcpy
#endif

#include <stdio.h>
#include <esps/spsassert.h>
#include <esps/xwaves_ipc.h>
#include <sys/time.h>
#if defined(LINUX_OR_MAC)
#include <string.h>
#endif

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>

#define COMM_PROP 	"Comm"
#define REGIS_PROP 	"InterpRegistry"
#define APPNAME_PROP 	"TK_APPLICATION"

#define MAX_PROP_WORDS 100000
#define UCHAR(c) ((unsigned char) (c))

extern int      debug_level;
/*char	       *strdup();*/

static int      SendSerial = 0;
static Atom     commProperty = 0;
static Atom     registryProperty = 0;
static Atom     appNameProperty = 0;
static Window   myWindow = None;
static int      XSend_Error = 0;
static char    *SendXwaves_defname = "default_xwaves_client";

#define DEBUG 0

typedef struct NameRegistry {
   Display        *display;	/* Display from which registry was read */
   int             locked;	/* Non-zero means the display was locked when
				 * the property was read in */
   int             modified;	/* Non-zero means that the property has been
				 * modified, so it needs to be written out
				 * when the NameRegistry is closed */
   unsigned long   propLength;	/* Length of the property in bytes */
   char           *property;	/* The contents of the property.  See format
				 * above; this is not terminated by the first
				 * null character. */
   int             allocedByX;	/* Non-zero  means must free property with
				 * XFree; zero means use free */
}               NameRegistry;

static void     RegAddName();
static void     RegClose();
static int      RegDeleteName();
static Window   RegFindName();
static NameRegistry *RegOpen();
static int      ValidateName();
static int      SplitList();
static void     UpdateCommWindow();
static void     AppendProp();
static void     print_prop();
static void     print_data();
static int      XSend_Handler();
static int      XSend_Update_Handler();
static void     Usleep();
Sxptr          *OpenXwaves();
void            CloseXwaves();

/*
 * This function is called by the client application to setup for X server
 * based communications.   The name passed in is typically the application
 * name, but it might be changed in order to be unique amoung registered
 * applications.   The application is registered by adding its name and XID
 * of a window to the InterpRegistry property of the display's root window.
 * Arguments: name	The name of the application to register under.
 * display The X display. window  The X window to use as a communications
 * window.
 * 
 * Return value: the function returns the actual name used in the registry.  It
 * might have been changed.
 */


char           *
Setup_X_Comm(name, display, window)
   char           *name;
   Display        *display;
   Window          window;
{
   NameRegistry   *regPtr;
   Window          w;
   char           *actualName, *string;
   int             i;

   spsassert(name, "Setup_X_Comm name is NULL");
   spsassert(display, "Setup_X_Comm display is NULL");
   spsassert(window, "Setup_X_Comm w is NULL");

   commProperty = XInternAtom(display, COMM_PROP, FALSE);
   registryProperty = XInternAtom(display, REGIS_PROP, FALSE);
   appNameProperty = XInternAtom(display, APPNAME_PROP, FALSE);

   if (debug_level)
      fprintf(stderr, "Setup_X_Comm: name: %s, display: %x, win: %x\n",
	      name, display, window);

   regPtr = RegOpen(display, 1);
   actualName = name;
   w = RegFindName(regPtr, actualName);

   if (w != None) {		/* name may be already in use */
      /*
       * First, try to determine if the registered name is for real.  It
       * could be that an application died and left its name registered.
       */
      if (debug_level)
	 fprintf(stderr, "found a %s, checking..");

      if (ValidateName(display, actualName, w)) {	/* Name is in use */

	 if (debug_level)
	    fprintf(stderr, "name is in use\n");
	 string = (char *) malloc(strlen(actualName) + 15);
	 for (i = 2;; i++) {
	    sprintf(string, "%s_#%d", actualName, i);
	    w = RegFindName(regPtr, string);
	    if (w == None) {
	       actualName = string;
	       break;
	    }
	    if (!ValidateName(display, string, w)) {
	       if (debug_level)
		  fprintf(stderr, ", bad name, remove it\n");
	       while (RegDeleteName(regPtr, string));;
	       actualName = string;
	       break;
	    }
	 }
	 if (debug_level)
	    fprintf(stderr, " new name is %s\n", actualName);
      } else {
	 if (debug_level)
	    fprintf(stderr, "found stale name, remove it\n");
	 while (RegDeleteName(regPtr, actualName));;
      }

   }
   /*
    * We now have a name to use.  Store it in the name registry.
    */
   RegAddName(regPtr, actualName, window);
   RegClose(regPtr);
   /*
    * The following just updates the APPNAME_PROP with actualName. Note that
    * Tk might have many apps per process, we have only one.
    */
   UpdateCommWindow(display, window, actualName);
   myWindow = window;
   return actualName;
}
Window
Get_X_Comm_Win(display, name)
   Display        *display;
   char           *name;
{
   NameRegistry   *regPtr;
   Window          win;
   spsassert(display && name, "Get_X_Comm_Win display or name NULL");

   regPtr = RegOpen(display, 0);
   win = RegFindName(regPtr, name);
   RegClose(regPtr);
   return win;
}


/*
 * Send a message.  The message sent is sent without the -r option, which
 * means that the message is async and no reply is required. Arguments:
 * display	The X display of the destination application. destname The
 * name of the destination application. msg	The message to send. length
 * The length of the message.
 * 
 * Return value: The return value is 1 for OK, 0 for Error (i.e. message not
 * sent)
 */

int
Send_X_Msg(display, destname, window, req, msg, length)
   Display        *display;
   char           *destname;
   Window          window;
   int             req;		/* like the RPC request code */
   char           *msg;
   int             length;
{
   char            *buffer;
   NameRegistry   *regPtr;
   int             len;
   Window          win;

#define SEND_CODE1 "\0c\0-n "
#define SEND_CODE1_LEN 6
#define SEND_CODE2 "\0-R "
#define SEND_CODE2_LEN 4
#define SEND_CODE3 "\0-r "
#define SEND_CODE3_LEN 4
#define SEND_CODE4 "\0-s "
#define SEND_CODE4_LEN 4

/* cannot use strlen on the above! */

   XSend_Error = 0;
   spsassert(destname || (window != None), "destname and win NULL");

   if (!msg || length < 1)
      return 0;

   if (debug_level)
      fprintf(stderr, "Send_X_Msg: msg: %s, len: %d\n", msg, length);

   if (window == None) {
      regPtr = RegOpen(display, 0);
      win = RegFindName(regPtr, destname);
      RegClose(regPtr);
      if (win == None)
	 return 0;
   } else
      win = window;

   if (req > IMMEDIATE_RESPONSE || req < RPCINFO_CALL)
      req = IMMEDIATE_RESPONSE;
   SendSerial++;
   buffer = (char *)malloc(length+SEND_CODE1_LEN+SEND_CODE2_LEN+
                           SEND_CODE3_LEN+SEND_CODE4_LEN+strlen(destname)+100);
   (void) memcpy(buffer, SEND_CODE1, SEND_CODE1_LEN);
   len = SEND_CODE1_LEN;
   (void) memcpy(buffer + len, destname, strlen(destname));
   len += strlen(destname);
   if (req != IMMEDIATE_RESPONSE) {
      (void) memcpy(buffer + len, SEND_CODE2, SEND_CODE2_LEN);
      len += SEND_CODE2_LEN;
      sprintf(buffer + len, "%d", req);
      len += strlen(buffer + len);
      (void) memcpy(buffer + len, SEND_CODE3, SEND_CODE3_LEN);
      len += SEND_CODE3_LEN;
      sprintf(buffer + len, "%x %d", myWindow, SendSerial);
      len += strlen(buffer + len);
   }
   (void) memcpy(buffer + len, SEND_CODE4, SEND_CODE4_LEN);
   len += SEND_CODE4_LEN;
   (void) memcpy(buffer + len, msg, length);
   len += length;
   buffer[len] = '\0';

   if (debug_level) {
      int             i;
      fprintf(stderr, "Send_X_Msg: message length %d content:\n", len + 1);
      for (i = 0; i < len + 1; i++)
	 if (buffer[i] == '\0')
	    fprintf(stderr, "<null>");
	 else
	    fputc(buffer[i], stderr);
      fprintf(stderr, "\nEnd of message\n");
   }
   AppendProp(display, win, commProperty, buffer, len + 1);
   free(buffer);
   return (XSend_Error == 0);
}

int
Send_X_Reply(display, destname, window, serial, msg, length)
   Display        *display;
   char           *destname;
   Window          window;
   int             serial;
   char           *msg;
   int             length;
{
   char            *buffer;
   NameRegistry   *regPtr;
   int             len;
   Window          win;
   char            str[100];

#define REPLY_CODE1 "\0r\0-s "
#define REPLY_CODE2 "\0-r "
/*
 Cannot use strlen on these since they contain null!
*/
#define REPLY_CODE1_LEN 6
#define REPLY_CODE2_LEN 4


   XSend_Error = 0;
   spsassert(destname || (window != None), "destname and win NULL");

   if (!msg || length < 1)
      return 0;

   if (debug_level)
      fprintf(stderr, "Send_X_Reply: msg: %s, len: %d\n", msg, length);

   if (destname) {
      regPtr = RegOpen(display, 0);
      win = RegFindName(regPtr, destname);
      RegClose(regPtr);
      if (win == None)
	 return 0;
   } else
      win = window;

   sprintf(str, "%d", serial);
   SendSerial++;
   buffer = (char *)malloc(REPLY_CODE1_LEN+strlen(str)+
			   REPLY_CODE2_LEN+length+1);
   (void) memcpy(buffer, REPLY_CODE1, REPLY_CODE1_LEN);
   len = 6;
   (void) memcpy(buffer + len, str, strlen(str));
   len += strlen(str);
   (void) memcpy(buffer + len, REPLY_CODE2, REPLY_CODE2_LEN);
   len += 4;
   (void) memcpy(buffer + len, msg, length);
   len += length;
   buffer[len] = '\0';

   if (debug_level) {
      int             i;
      fprintf(stderr, "Send_X_Reply: message length %d content:\n", len + 1);
      for (i = 0; i < len + 1; i++)
	 if (buffer[i] == '\0')
	    fprintf(stderr, "<null>");
	 else
	    fputc(buffer[i], stderr);
      fprintf(stderr, "\nEnd of message\n");
   }
   AppendProp(display, win, commProperty, buffer, len + 1);
   free(buffer);
   return (XSend_Error == 0);
}

/*
 * Unregister the application.
 * 
 * Arguments: display	The X display. name	The name of the application
 * to unregister.
 */

void
Close_X_Comm(display, name)
   Display        *display;
   char           *name;
{
   NameRegistry   *regPtr;
   Window          window;

   spsassert(display, "Close_X_Comm: display is NULL");
   spsassert(name, "Close_X_Comm: name is NULL");

   regPtr = RegOpen(display, 1);
   window = RegFindName(regPtr, name);
   RegDeleteName(regPtr, name);
   RegClose(regPtr);

   if (window != None)
      UpdateCommWindow(display, window, " ");
}

/*
 * Read the message from the designated communications window.  The
 * assumption is that the calling program has been notified of the
 * XPropertyChanged event.   The message can be returned in one of two
 * formats.  One is "raw", which means that the entire contents of the
 * property is returned, including Tk options.   The other format is non-raw,
 * and this means that just the "script" part (the command itself) is
 * returned.   Note that in either format it is possible that multiple
 * commands are returned.  They will be separated by NULLs.
 * 
 * Arguments: display	The X display. window  The X window to read from.
 * xevent  The X event structuure this event. raw	1 means return raw
 * data, 0 means return command only buffer	Pointer to space for the
 * return data. max_length The maximum number of bytes to copy to buffer. If
 * non raw mode, then a new command will not be added to buffer unless it
 * will all fit.
 * 
 * Return value: Number of commands read.  For raw mode, it returns 1;
 * regardless of the number of raw commands in the buffer.  For non raw, it
 * is the number of null separated commands in the buffer. If no data is
 * read, then zero is returned.
 */

int
Read_X_Comm(display, window, xevent, raw, buffer, max_length, callback)
   Display        *display;
   Window          window;
   char           *buffer;
   int             max_length;
   XEvent         *xevent;
   int             raw;
   void            (*callback) ();	/* dest, calling_win, serial, req,
					 * script */
{
   char           *propInfo, *p;
   int             result, actualFormat;
   unsigned long   numItems, bytesAfter;
   Atom            actualType;
   int             line_count = 0;

   spsassert(display, "Read_X_Comm: display is NULL");
   spsassert(window, "Read_X_Comm: window is NULL");
   spsassert(xevent, "Read_X_Comm: xevent is NULL");

   if ((xevent->xproperty.atom != commProperty)
       || (xevent->xproperty.state != PropertyNewValue)) {
      return 0;
   }
   propInfo = NULL;
   result = XGetWindowProperty(display, window, commProperty, 0,
		MAX_PROP_WORDS, True, XA_STRING, &actualType, &actualFormat,
		      &numItems, &bytesAfter, (unsigned char **) &propInfo);

   /*
    * If the property doesn't exist or is improperly formed then ignore it.
    */

   if ((result != Success) || (actualType != XA_STRING)
       || (actualFormat != 8)) {
      if (propInfo != NULL) {
	 XFree(propInfo);
      }
      return 0;
   }
   if (!callback && raw) {
      if (numItems > max_length - 1)
	 numItems = max_length - 1;
      memcpy(buffer, propInfo, numItems);
      buffer[numItems] = '\0';
      XFree(propInfo);
      return 1;
   } else {
      /*
       * Several commands and results could arrive in the property at one
       * time;  each iteration through the outer loop handles a single
       * command or result.
       */

      for (p = propInfo; (p - propInfo) < numItems;) {
	 /*
	  * Ignore leading NULLs; each command or result starts with a NULL
	  * so that no matter how badly formed a preceding command is, we'll
	  * be able to tell that a new command/result is starting.
	  */

	 if (*p == 0) {
	    p++;
	    continue;
	 }
	 if ((*p == 'c') && (p[1] == 0)) {
	    char           *dest, *script, *end;
	    char           *bptr = buffer;
	    int             req, l, serial;
	    Window          commWindow;


	    /*
	     * -------------------------------------------- --------------
	     * This is an incoming command from some other application.
	     * Iterate over all of its options.  Stop when we reach the end
	     * of the property or something that doesn't look like an option.
	     * ------------------------------------------- ---------------
	     */

	    p += 2;
	    script = NULL;
	    req = IMMEDIATE_RESPONSE;
	    commWindow = None;
	    serial = 0;
	    while (((p - propInfo) < numItems) && (*p == '-')) {
	       switch (p[1]) {
	       case 'R':	/* old rpc request code */
		  if (p[2] == ' ') {
		     req = atoi(p + 3);
		  }
		  break;
	       case 'r':
		  commWindow = (Window) (unsigned long) strtol(p + 2, &end, 16);
		  if ((end == p + 2) || (*end != ' ')) {
		     commWindow = None;
		  } else {
		     p = end + 1;
		     serial = (unsigned long) atoi(p);
		  }
		  break;
	       case 'n':
		  if (p[2] == ' ') {
		     dest = p + 3;
		  }
		  break;
	       case 's':
		  if (p[2] == ' ') {
		     script = p + 3;
		  }
		  break;
	       }
	       while (*p != 0) {
		  p++;
	       }
	       p++;
	    }

	    if (script == NULL) {
	       continue;
	    }
	    if (*script == '-' && strcmp(script + 1, "reply")) {
	       req = INTERLOCK_DELAY_RESPONSE_TK;
	       script += 7;
	    }
	    if (callback) {
	       if (debug_level)
		  fprintf(stderr, "Comm Msg callback:\n%s %x %d %d\n%s\n",
			  dest, commWindow, serial, req, script);
	       callback(dest, commWindow, serial, req, script);
	    } else {
	       l = strlen(script);
	       memcpy(bptr, script, l < max_length ? l : max_length);
	       line_count++;
	       bptr += l;
	       if (bptr - buffer >= max_length)
		  break;
	       *bptr++ = '\0';
	    }
	 } else {
	    /*
	     * Didn't recognize this thing.  Just skip through the next null
	     * character and try again.
	     */

	    while (*p != 0) {
	       p++;
	    }
	    p++;
	 }
      }
      XFree(propInfo);
      return line_count;
   }
}

static char    *
Read_X_Reply(display, window, xevent, serial_expected)
   Display        *display;
   Window          window;
   XEvent         *xevent;
   int             serial_expected;
{
   char           *propInfo, *p;
   int             result, actualFormat;
   unsigned long   numItems, bytesAfter;
   Atom            actualType;

   spsassert(display, "Read_X_Comm: display is NULL");
   spsassert(window, "Read_X_Comm: window is NULL");
   spsassert(xevent, "Read_X_Comm: xevent is NULL");

   if ((xevent->xproperty.atom != commProperty)
       || (xevent->xproperty.state != PropertyNewValue)) {
      return NULL;
   }
   propInfo = NULL;
   result = XGetWindowProperty(display, window, commProperty, 0,
		MAX_PROP_WORDS, True, XA_STRING, &actualType, &actualFormat,
		      &numItems, &bytesAfter, (unsigned char **) &propInfo);

   /*
    * If the property doesn't exist or is improperly formed then ignore it.
    */

   if ((result != Success) || (actualType != XA_STRING)
       || (actualFormat != 8)) {
      if (propInfo != NULL) {
	 XFree(propInfo);
      }
      return NULL;
   }
   for (p = propInfo; (p - propInfo) < numItems;) {
      /*
       * Ignore leading NULLs; each command or result starts with a NULL so
       * that no matter how badly formed a preceding command is, we'll be
       * able to tell that a new command/result is starting.
       */

      if (*p == 0) {
	 p++;
	 continue;
      }
      if ((*p == 'r') && (p[1] == 0)) {
	 int             serial, gotSerial;
	 char           *resultString, *result;

	 p += 2;
	 gotSerial = 0;
	 resultString = NULL;
	 while (((p - propInfo) < numItems) && (*p == '-')) {
	    switch (p[1]) {
	    case 'r':
	       if (p[2] == ' ') {
		  resultString = p + 3;
	       }
	       break;
	    case 's':
	       if (sscanf(p + 2, " %d", &serial) == 1) {
		  gotSerial = 1;
		  if (serial_expected == -1)
		     serial_expected = serial;
	       }
	       break;
	    }
	    while (*p != 0) {
	       p++;
	    }
	    p++;
	 }
	 if (!gotSerial || (serial != serial_expected)) {
	    continue;
	 }
	 result = strdup(resultString);
	 XFree(propInfo);
	 return result;
      } else {
	 while (*p != 0) {
	    p++;
	 }
	 p++;
      }
   }
   XFree(propInfo);
   return NULL;
}

static void
UpdateCommWindow(display, window, name)
   Display        *display;
   Window          window;
   char           *name;
{
   int             (*defaultHandler) () = NULL;


   spsassert(display, "UpdateCommWindow: display is NULL");
   spsassert(window, "UpdateCommWindow: window is NULL");
   spsassert(name, "UpdateCommWindow: name is NULL");

   defaultHandler = XSetErrorHandler(XSend_Update_Handler);
   XChangeProperty(display, window, appNameProperty, XA_STRING,
		   8, PropModeReplace, name, strlen(name) + 1);
   XSync(display, FALSE);
   (void) XSetErrorHandler(defaultHandler);
}


static NameRegistry *
RegOpen(display, lock)
   Display        *display;
   int             lock;
{
   NameRegistry   *regPtr;
   int             result, actualFormat;
   unsigned long   bytesAfter;
   Atom            actualType;

   spsassert(display, "RegOpen: display is NULL");

   regPtr = (NameRegistry *) malloc(sizeof(NameRegistry));
   regPtr->display = display;
   regPtr->locked = 0;
   regPtr->modified = 0;
   regPtr->allocedByX = 1;

   if (lock && !debug_level) {
      XGrabServer(display);
      regPtr->locked = 1;
   }
   /*
    * Read the registry property
    */

   result = XGetWindowProperty(display,
			       RootWindow(display, 0),
			       registryProperty, 0, MAX_PROP_WORDS,
			       False, XA_STRING, &actualType, &actualFormat,
			       &regPtr->propLength, &bytesAfter,
			       (unsigned char **) &regPtr->property);

   if (actualType == None) {
      regPtr->propLength = 0;
      regPtr->property = NULL;
   } else if ((result != Success) || (actualFormat != 8)
	      || (actualType != XA_STRING)) {
      /*
       * The property is improperly formed; delete it.
       */

      if (regPtr->property != NULL) {
	 XFree(regPtr->property);
	 regPtr->propLength = 0;
	 regPtr->property = NULL;
      }
      XDeleteProperty(display, RootWindow(display, 0), registryProperty);
   }
   /*
    * Xlib placed an extra null byte after the end of the property, just to
    * make sure that it is always NULL-terminated.  Be sure to include this
    * byte in our count if it's needed to ensure null termination
    */

   if ((regPtr->propLength > 0)
       && (regPtr->property[regPtr->propLength - 1] != 0)) {
      regPtr->propLength++;
   }
   if (debug_level) {
      fprintf(stderr, "RegOpen: Property:\n");
      print_prop(regPtr);
      fprintf(stderr, "\n");
   }
   return regPtr;
}

static          Window
RegFindName(regPtr, name)
   NameRegistry   *regPtr;	/* Pointer to a registry opened with a
				 * previous call to RegOpen. */
   char           *name;	/* Name of an application. */
{
   char           *p, *entry;
   Window          commWindow;

   commWindow = None;
   for (p = regPtr->property; (p - regPtr->property) < regPtr->propLength;) {
      entry = p;
      while ((*p != 0) && (!isspace(UCHAR(*p)))) {
	 p++;
      }
      if ((*p != 0) && (strcmp(name, p + 1) == 0)) {
	 if (sscanf(entry, "%x", (unsigned int *) &commWindow) == 1) {
	    return commWindow;
	 }
      }
      while (*p != 0) {
	 p++;
      }
      p++;
   }
   return None;
}

static int
RegDeleteName(regPtr, name)
   NameRegistry   *regPtr;	/* Pointer to a registry opened with a
				 * previous call to RegOpen. */
   char           *name;	/* Name of an application. */
{
   char           *p, *entry, *entryName;
   int             count;

   for (p = regPtr->property; (p - regPtr->property) < regPtr->propLength;) {
      entry = p;
      while ((*p != 0) && (!isspace(UCHAR(*p)))) {
	 p++;
      }
      if (*p != 0) {
	 p++;
      }
      entryName = p;
      while (*p != 0) {
	 p++;
      }
      p++;
      if ((strcmp(name, entryName) == 0)) {
	 /*
	  * Found the matching entry.  Copy everything after it down on top
	  * of it.
	  */

	 count = regPtr->propLength - (p - regPtr->property);
	 if (count > 0) {
	    memmove((void *) entry, (void *) p, (size_t) count);
	 }
	 regPtr->propLength -= p - entry;
	 regPtr->modified = 1;
	 return 1;
      }
   }
   return 0;
}

static void
RegAddName(regPtr, name, commWindow)
   NameRegistry   *regPtr;	/* Pointer to a registry opened with a
				 * previous call to RegOpen. */
   char           *name;	/* Name of an application.  The caller must
				 * ensure that this name isn't already
				 * registered. */
   Window          commWindow;	/* X identifier for comm. window of
				 * application.  */
{
   char            id[30];
   char           *newProp;
   int             idLength, newBytes;

   sprintf(id, "%x ", (unsigned int) commWindow);
   idLength = strlen(id);
   newBytes = idLength + strlen(name) + 1;
   newProp = (char *) malloc((unsigned) (regPtr->propLength + newBytes));
   strcpy(newProp, id);
   strcpy(newProp + idLength, name);
   if (regPtr->propLength > 0) {
      memcpy((void *) (newProp + newBytes), (void *) regPtr->property,
	     regPtr->propLength);
      if (regPtr->allocedByX) {
	 XFree(regPtr->property);
      } else {
	 free(regPtr->property);
      }
   }
   regPtr->modified = 1;
   regPtr->propLength += newBytes;
   regPtr->property = newProp;
   regPtr->allocedByX = 0;
}

static void
RegClose(regPtr)
   NameRegistry   *regPtr;	/* Pointer to a registry opened with a
				 * previous call to RegOpen. */
{
   if (regPtr->modified) {
      if (!regPtr->locked && !debug_level) {
	 fprintf(stderr,
		 "The name registry was modified without being locked!");
	 return;
      }
      XChangeProperty(regPtr->display,
		      RootWindow(regPtr->display, 0),
		      registryProperty, XA_STRING, 8,
		      PropModeReplace, (unsigned char *) regPtr->property,
		      (int) regPtr->propLength);
   }
   if (regPtr->locked) {
      XUngrabServer(regPtr->display);
   }
   XFlush(regPtr->display);

   if (regPtr->propLength > 0) {
      if (regPtr->allocedByX) {
	 XFree(regPtr->property);
      } else {
	 free(regPtr->property);
      }
   }
   free((char *) regPtr);
}

static int
SplitList(list, argc, argv)
   char           *list;
   int            *argc;
   char         ***argv;
{
#if !defined(LINUX_OR_MAC)
   char           *p, *s, *strtok(), *strdup();
#else
   char           *p, *s, *strtok();
#endif

   *argc = 0;
   if (!list)
      return 0;

   *argv = (char **) malloc(sizeof(char *));
   p = list;
   while (s = strtok(p, "\0")) {
      p = NULL;
      if (debug_level)
	 fprintf(stderr, "Split: %s\n", s);
      *argv = (char **) realloc(*argv, sizeof(char *) + *argc);
      (*argv)[*argc] = strdup(s);
      *argc += 1;
   }
   return 1;
}

static int
ValidateName(display, name, window)
   Display        *display;
   char           *name;
   Window          window;
{
   int             argc, result, actualFormat, i;
   unsigned long   length, bytesAfter;
   Atom            actualType;
   char           *property;
   char          **argv;
   int             (*defaultHandler) () = NULL;


   spsassert(display, "ValidateName: display is NULL");
   spsassert(name, "ValidateName: name is NULL");
   spsassert(window, "ValidateName: window is zero");

   /*
    * Be sure window matches Tk 4.0 commWindow specs.  This version does not
    * allow pre 4.0 windows to work.
    */
   if (debug_level)
      fprintf(stderr, "ValidateName display: %x, window: %x, name: %s\n",
	      display, window, name);

   property = NULL;

   defaultHandler = XSetErrorHandler(XSend_Handler);

   result = XGetWindowProperty(display, window,
			       appNameProperty, 0, MAX_PROP_WORDS,
			       False, XA_STRING, &actualType, &actualFormat,
			&length, &bytesAfter, (unsigned char **) &property);

   (void) XSetErrorHandler(defaultHandler);

   if (debug_level)
      fprintf(stderr, "ValidateName: Got property from X\n");

   if ((result == Success) && (actualFormat == 8)
       && (actualType == XA_STRING)) {
      result = 0;
      if (debug_level)
	 print_data(property, length);
      if (SplitList(property, &argc, &argv) == 1) {
	 for (i = 0; i < argc; i++) {
	    if (strcmp(argv[i], name) == 0) {
	       result = 1;
	       break;
	    }
	 }
	 for (i = 0; i < argc; i++)
	    free(argv[i]);
	 free((char *) argv);
      }
   } else {
      result = 0;
   }
   if (property != NULL) {
      XFree(property);
   }
   return result;
}

static void
AppendProp(display, window, property, data, length)
   Display        *display;
   Window          window;
   Atom            property;
   char           *data;
   int             length;
{
   int             (*defaultHandler) () = NULL;

   if (debug_level) {
      fprintf(stderr, "AppendProp:\n");
      fprintf(stderr, "display: %x, win: %x, len: %d\n",
	      display, window, length);
   }
   defaultHandler = XSetErrorHandler(XSend_Handler);

   XChangeProperty(display, window, property, XA_STRING, 8,
		   PropModeAppend, data, length);
   XSync(display, FALSE);

   (void) XSetErrorHandler(defaultHandler);
}

static void
print_prop(regPtr)
   NameRegistry   *regPtr;	/* Pointer to a registry opened with a
				 * previous call to RegOpen. */
{
   char           *p, *entry;
   Window          commWindow;

   commWindow = None;
   for (p = regPtr->property; (p - regPtr->property) < regPtr->propLength;) {
      entry = p;
      fprintf(stderr, "%s\n", entry);
      while ((*p != 0) && (!isspace(UCHAR(*p)))) {
	 p++;
      }
      while (*p != 0) {
	 p++;
      }
      p++;
   }
   return;
}

static void
print_data(ptr, len)
   char           *ptr;
   int             len;
{
   int             i;
   fprintf(stderr, "print_data: len: %d\n", len);
   for (i = 0; i < len; i++)
      if (ptr[i] == '\0')
	 fprintf(stderr, "<null>");
      else
	 fputc(ptr[i], stderr);
   fprintf(stderr, ":end:\n");
}

static int
XSend_Handler(display, error)
   Display        *display;
   XErrorEvent    *error;
{
   if (debug_level) {
      char            msg[80];
      fprintf(stderr, "XSend_Handler called\n");
      XGetErrorText(display, error->error_code, msg, 80);
      fprintf(stderr, "Error code %s\n", msg);
   }
   XSend_Error = 1;
   return 0;
}

static int
XSend_Update_Handler(display, error)
   Display        *display;
   XErrorEvent    *error;
{
   fprintf(stderr, "Unexpected X error in UpdateCommWindow, ignored.\n");
   fprintf(stderr, "Cound not update Comm window property\n");
   if (debug_level) {
      char            msg[80];
      XGetErrorText(display, error->error_code, msg, 80);
      fprintf(stderr, "Error code %s\n", msg);
   }
   return 0;
}

static void
Usleep(usecs)
   unsigned int    usecs;
{
   struct timeval  tval;

   tval.tv_sec = usecs / 1000000;
   tval.tv_usec = usecs % 1000000;
   select(0, NULL, NULL, NULL, &tval);
}


int
SendXwavesNoReply(display_name, dest, sxarg, msg)
   char           *display_name;
   char           *dest;
   Sxptr          *sxarg;
   char           *msg;
{
   int             res;
   Sxptr          *sxptr;

   if (!msg || (!sxarg && !dest))	/* bad args */
      return 0;

   if (!sxarg) {
      if ((sxptr = OpenXwaves(display_name, dest, SendXwaves_defname)) == NULL)
	 return 0;
   } else
      sxptr = sxarg;

   res = Send_X_Msg(sxptr->display, sxptr->dest_name, sxptr->dest_window,
		    IMMEDIATE_RESPONSE, msg, strlen(msg));
   if (!sxarg)
      CloseXwaves(sxptr);

   return res;
}

char           *
SendXwavesReply(display_name, dest, sxarg, msg, timeout)
   char           *display_name;
   char           *dest;
   Sxptr          *sxarg;
   char           *msg;
   unsigned int    timeout;	/* in ms. */
{
   int             res;
   XEvent          event;
   char           *result = NULL;
   int             serial, loop_count = 0;
   Sxptr          *sxptr;

   if (!msg || (!sxarg && !dest))	/* bad args */
      return NULL;

   if (!sxarg) {
      if ((sxptr = OpenXwaves(display_name, dest, SendXwaves_defname)) == NULL)
	 return NULL;
   } else
      sxptr = sxarg;

   if( !ValidateName(sxptr->display, sxptr->dest_name, sxptr->dest_window) )
      return NULL;

   XSelectInput(sxptr->display, sxptr->my_window, PropertyChangeMask);
   res = Send_X_Msg(sxptr->display, sxptr->dest_name, sxptr->dest_window,
		    INTERLOCK_DELAY_RESPONSE, msg, strlen(msg));
   if (!res) {
      XSelectInput(sxptr->display, sxptr->my_window, NoEventMask);
      if (sxarg)
	 CloseXwaves(sxptr);
      return NULL;
   }
   while (1) {
      if (XCheckMaskEvent(sxptr->display, PropertyChangeMask, &event)
	  != False) {
	 if (event.type == PropertyNotify &&
	     event.xproperty.window == sxptr->my_window) {
	    result = Read_X_Reply(sxptr->display, sxptr->my_window,
				  &event, SendSerial);
	    if (result)
	       break;
	 }
      }
      Usleep((unsigned) 1000);
      if (loop_count++ > timeout)
	 break;
   }
   XSelectInput(sxptr->display, sxptr->my_window, NoEventMask);
   if (!sxarg)
      CloseXwaves(sxptr);
   return result;
}


Sxptr          *
OpenXwaves(display_name, dest, myname)
   char           *display_name;
   char           *dest;
   char           *myname;
{
   Sxptr          *sxptr = (Sxptr *) malloc(sizeof(Sxptr));
   int             screen_num;
#if !defined(LINUX_OR_MAC)
   char           *strdup();
#endif

   if (!dest || !sxptr) {
      return NULL;
   }
   if (!myname)
      myname = SendXwaves_defname;

   if ((sxptr->display = XOpenDisplay(display_name)) == NULL) {
      fprintf(stderr, "%s: Cannot connect to X server %s\n",
	      myname, XDisplayName(display_name));
      return NULL;
   }
   screen_num = DefaultScreen(sxptr->display);
   sxptr->my_window = XCreateSimpleWindow(sxptr->display,
				     RootWindow(sxptr->display, screen_num),
					  0, 0, 1, 1, 1,
				     BlackPixel(sxptr->display, screen_num),
				    WhitePixel(sxptr->display, screen_num));
   sxptr->my_name = Setup_X_Comm(myname, sxptr->display, sxptr->my_window);
   if (!sxptr->my_name) {
      free(sxptr);
      return NULL;
   }
   sxptr->my_name = strdup(sxptr->my_name);
   sxptr->dest_window = Get_X_Comm_Win(sxptr->display, dest);
   if (!sxptr->dest_window) {
      free(sxptr->my_name);
      free(sxptr);
      return NULL;
   }
   sxptr->dest_name = strdup(dest);
   return sxptr;
}

void
CloseXwaves(sxptr)
   Sxptr          *sxptr;
{
   if (sxptr) {
      Close_X_Comm(sxptr->display, sxptr->my_name);
      XCloseDisplay(sxptr->display);
      free(sxptr->dest_name);
      free(sxptr->my_name);
      free(sxptr);
   }
}

char           *
StripReturn(msg)
   char           *msg;
{
   char           *ptr = NULL;

   if (msg) {
      ptr = msg;
      if (!strncmp("returned", msg, 8))
	 ptr += 9;
   }
   return ptr;
}
