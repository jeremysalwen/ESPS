/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 *
 * Brief description: utilities for  keyboard command mapping for xwaves
 *
 */

/* This module implements a means for connecting any xwaves command
   with a key on the keyboard.  The connection is defined by a simple
   lookup table relating the key ASCII values to entries in a
   particular Menuop list. The menuop lists will correspond to the
   waveform, spectrogram or "none" repertoires.  By default, no key mappings
   will be defined.  A set of mappings may be specified via "key_map"
   commands in an init file.  This file may be specified as part of the
   users .wave_pro database.  After startup, additional mappings may be
   established with the key_map command that takes two manditory and one
   optional argument (key value, command name and, optionally, menu
   name).  Key mappings may be deleted with the key_unmap command that
   takes only two arguments: key value and, optionally, menu name.

    Two separate keymap lists are maintained: one for waveform data
    windows and one for spectrogram data windows.  The same ksy is allowed
    to have different semantics in different window contexts.
*/

static char *sccs_id = "@(#)keymapper.c	1.10	11/8/95	ATT/ERL";

#include <Objects.h>

extern char *checking_selectors();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }


typedef struct keycom {
  int keyval;
  Menuop *operation;
  struct keycom *next;
} Keycom;

static Keycom *wave_keys = NULL;
static Keycom *spect_keys = NULL;

/************************************************************************/
static Keycom *new_keycom(val, mo)
     int val;
     Menuop *mo;
{
  Keycom *kp;

  if((kp = (Keycom*)malloc(sizeof(Keycom)))) {
    kp->keyval = val;
    kp->operation = mo;
    kp->next = NULL;
    return(kp);
  }
  fprintf(stderr,"Allocation problems in new_keycom()\n");
  return(NULL);
}

/************************************************************************/
static Keycom *link_new_keycom(next, mo, key)
     Keycom *next;
     Menuop *mo;
     int key;
{
  Keycom *kc = next;

  while(kc) {			/* replace existing map */
    if(kc->keyval == key) {
      kc->operation = mo;
      return(next);
    }
    kc = kc->next;
  }

  kc = next;
  while(kc) {			/* use an empty slot */
    if(kc->keyval < 0) {
      kc->keyval = key;
      kc->operation = mo;
      return(next);
    }
    kc = kc->next;
  }

  if((kc = new_keycom(key, mo))) { /* create a new slot */
    kc->next = next;
    return(kc);
  } else
    return(next);
}

/************************************************************************/
/*
  Add a key-command map entry.  If menu is unspecified, search the
  view-type-specific menuop lists and try to match up key "menu" type
  with the view type.  If this fails, search any remaining lists.
*/
add_keycom(menu,key,command)
     char *menu, *command;
     int key;
{
  Menuop *mo;
  Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();
  int itsok = FALSE;

  while(mol) {
      mo = mol->first_op;
      while(mo) {
	if(mo->name && !strcmp(mo->name,command) && mo->proc) {
	  if(((menu && *menu) && !strcmp(menu,"wave") && strcmp(mol->name, "spect")) ||
	     (!(menu && *menu) && strcmp(mol->name,"spect")))
	    if((wave_keys = link_new_keycom(wave_keys,mo,key)))
	      itsok = TRUE;
	  if(((menu && *menu) && !strcmp(menu,"spect") && strcmp(mol->name, "wave")) ||
	     (!(menu && *menu) && strcmp(mol->name,"wave")))
	    if((spect_keys = link_new_keycom(spect_keys,mo,key)))
	      itsok = TRUE;
	  break;
	}
	mo = mo->next;
      }
    mol = mol->next;
  }
  return(itsok);
}

/************************************************************************/
delete_keycom(menu, key)
     char *menu, key;
{
  Keycom *kc;

  if(!(menu && *menu) || !strcmp("spect",menu)) {
    kc = spect_keys;
    while(kc) {
      if(kc->keyval == key)
	kc->keyval = -1;
      kc = kc->next;
    }
  }

  if(!(menu && *menu) || !strcmp("wave",menu)) {
    kc = wave_keys;
    while(kc) {
      if(kc->keyval == key)
	kc->keyval = -1;
      kc = kc->next;
    }
  }
}

/************************************************************************/
translate_keyval(k)
     char *k;
{

  if(strlen(k) > 1) {
    switch(*k) {
    case '^':
      if(k[1] == '_')
	k[1] = ' ';
      *k = k[1] & 037;
      break;
    default:
      fprintf(stderr,"unrecognized sequence in translate_keyval(%s)\n",k);
      break;
    }
  } else {
    if(*k == '_')
      *k = ' ';
  }
  k[1] = 0;
}

extern char ok[], null[];

static char key[10], menu[20], command[NAMELEN];
static Selector
  s1 = {"key", "%s", key, NULL},
  s2 = {"menu", "%s", menu, &s1},
  s3b = {"op", "#strq", command, &s2},
  s3 = {"command", "#strq", command, &s3b};

/************************************************************************/
encode_keyval(kv, val)
     char *kv;
     int val;
{
  if((val >= 0) && kv) {
    *kv = 0;
    if(val < 32) {
      if(val > 0)
	val |= 64;
      else
	val = ' ';
      *kv++ = '^';
    }
    if(val == ' ')
      val = '_';
    *kv++ = val;
    *kv = 0;
  }
} 

/************************************************************************/
save_keymap(of,kc,menu)
     FILE *of;
     Keycom *kc;
     char *menu;
{
  Menuop *mo;
  if((kc->keyval >= 0) && (mo = kc->operation) && mo->name &&
     mo->name[0]) {
    char kv[10];
    encode_keyval(kv, kc->keyval);
    fprintf(of,"key_map key %s menu %s op \"%s\"\n",kv,menu,mo->name);
  }
}

/************************************************************************/
dump_keymaps(file)
     char *file;
{
  if(file && *file) {
    char scrat[NAMELEN];
    FILE *of;
    
    /* expand any environment variables */
    (void) build_filename(scrat, "", file); 
    if((of = fopen(scrat, "w"))) {
      Keycom *kc = wave_keys;
      
      while(kc) {
	save_keymap(of,kc,"wave");
	kc = kc->next;
      }
      kc = spect_keys;
      while(kc) {
	save_keymap(of,kc,"spect");
	kc = kc->next;
      }
      fprintf(of,"return\n");
      fclose(of);
      return(TRUE);
    } else {
      sprintf(notice_msg,"Can't open %s for output in dump_keymaps.",scrat);
      show_notice(1,notice_msg);
    }
  }
  return(FALSE);
}

/************************************************************************/
char *meth_dump_keymaps(o, str)
     Object *o;
     char *str;
{
  static char file[NAMELEN];
  static Selector s = {"output", "%s", file, NULL};

  CHECK_QUERY(str,&s)
    if(get_args(str,&s)) {
    if(dump_keymaps(file))
      return(ok);
    else {
      sprintf(notice_msg,"Problems dumping keymaps to file %s", file);
      show_notice(1,notice_msg);
    }
  } else
    show_notice(1,"No output file was specified to meth_dump_keymaps.");
  return(null);
}

/************************************************************************/
char *meth_add_keymap(o, str)
     Object *o;
     char *str;
{
  *menu = 0;
  *command = 0;
  *key = 0;
  CHECK_QUERY(str, &s3)
    if(get_args(str, &s3) && *command && *key) {
    translate_keyval(key);
    if(add_keycom(menu, (int)key[0], command))
      return(ok);
    sprintf(notice_msg,"Can't map command (%s) to key (%s)",command,key);
    show_notice(1,notice_msg);
  } else {
    sprintf(notice_msg,"Bad args to meth_add_keymap(%s)",str);
    show_notice(1,notice_msg);
  }
  return(null);
}

/************************************************************************/
char *meth_delete_keymap(o, str)
     Object *o;
     char *str;
{
  *menu = 0;
  *key = 0;
  CHECK_QUERY(str, &s2)
    if(get_args(str, &s2) && *key) {
    translate_keyval(key);
    delete_keycom(menu,(int)key[0]);
    return(ok);
  }
  return(null);
}

/************************************************************************/
do_key_command(canvas, event, kc)
     Canvas canvas;
     Event *event;
     Keycom *kc;
{
  int id = event_id(event);
  Menuop *mo;
  while(kc) {
    if((kc->keyval == id) && (mo = kc->operation) && mo->proc) {
      mo->proc(canvas, event, mo->data);
      return;
    }
    kc = kc->next;
  }
}

/************************************************************************/
keymap_command(canvas, event, arg)
     Canvas canvas;Event *event;
     caddr_t arg;
{
  View *v = (View*)xv_get(canvas, WIN_CLIENT_DATA);
  
  if(v) {
    if(isa_spectrogram_view(v))
      do_key_command(canvas, event, spect_keys);
    else
      do_key_command(canvas, event, wave_keys);
  }
}
